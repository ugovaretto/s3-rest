/*******************************************************************************
* BSD 3-Clause License
* 
* Copyright (c) 2020, Ugo Varetto
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
// Utility functions to parse and manipulate strings and dictionaries.
#include <cassert>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "url_utility.h"
using namespace std;

//------------------------------------------------------------------------------
// Break url into {protocol, hostname, port}
URL ParseURL(const string& s) {
    smatch m;
    // minimal version of a URL parser: no enforcement of having the host
    // section start and end with a word and requiring a '.' character
    // separating the individual words.
    regex e("\\s*(\\w+)://([a-zA-Z-_\\.]+)(:(\\d+))?");
    if(!regex_search(s, m, e, regex_constants::match_any)){
        return {};
    }
    if(m[4].str().empty()) {
        return {-1, m[2], m[1]};
    } else {
        return {int(stoul(m[4])), m[2], m[1]};
    }
}

//------------------------------------------------------------------------------
// urlencode string
string UrlEncode(const std::string& s) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = s.begin(), n = s.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
    }
    return escaped.str();
}
//------------------------------------------------------------------------------
// Return urlencoded url request parameters from {key, value} dictionary
string UrlEncode(const map<string, string>& p) {
    string url;
    for (auto i : p) {
        url += UrlEncode(i.first) + "=" + UrlEncode(i.second);
        url += '&';

    }
    return string(url.begin(), --url.end());
}

//------------------------------------------------------------------------------
// Uppercase
string ToUpper(string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

//------------------------------------------------------------------------------
// Lowercase
string ToLower(string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

//TODO: create StringToDict(const string&, const string& delims) funciton to be
//invoked from both ParseXXX functions.
//------------------------------------------------------------------------------
// From "key1=value1;key2=value2;key3=;key4" to {key, value} dictionary
map<string, string> ParseParams(string s) {
    if (s.empty()) return map<string, string>();
    vector<string> slist;
    split(s, slist, ";");
    map<string, string> params;
    for (auto p : slist) {
        vector<string> kv;
        if(p.find_first_of("=") == string::npos)
            p += "=";
        split(p, kv, "=", 1);
        assert(kv.size() == 1 || kv.size() == 2);
        const string key = kv[0];
        const string value = kv.size() == 2 ? kv[1] : "";
        params.insert({key, value});
    }
    return params;
}

//------------------------------------------------------------------------------
// From "key1:value1;key2:value2 {key, value} dictionary
// Per https://www.w3.org/Protocols/rfc2616/rfc2616.html
// HTTP headers are not case-sensitive and therefore it's better to translate
// all of them to lower-case to avoid problems when they are places is
// lexicographic order as part of the S3v4 signing process.
map<string, string> ParseHeaders(const string& s) {
    if (s.empty()) return map<string, string>();
    vector<string> slist;
    split(s, slist, ";");
    map<string, string> params;
    for (auto p : slist) {
        vector<string> kv;
        split(p, kv, ":", 1);
        assert(kv.size() == 1 || kv.size() == 2);
        const string key = kv[0];
        const string value = kv.size() == 2 ? kv[1] : "";
        params.insert({ToLower(key), value});
    }
    return params;
}