/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include <webclient.h>

#include <iostream>
#include <algorithm>

using namespace std;
std::atomic<int> WebClient::numInstances_{0};
std::mutex WebClient::cleanupMutex_;

size_t ReadFile(void* ptr, size_t size, size_t nmemb, void* userdata) {
    FILE* f = static_cast<FILE*>(userdata);
    if (ferror(f)) return CURL_READFUNC_ABORT;
    return fread(ptr, size, nmemb, f) * size;
}

size_t ReadFileUnbuffered(void* ptr, size_t size, size_t nmemb,
                          void* userdata) {
    const int fd = *static_cast<int*>(userdata);
    return max(ssize_t(0), read(fd, ptr, size * nmemb));
}

size_t WriteFile(char* ptr, size_t size, size_t nmemb, void* userdata) {
    FILE* writehere = static_cast<FILE*>(userdata);
    size = size * nmemb;
    fwrite(ptr, size, nmemb, writehere);
    return size;
}

size_t WriteFileUnbuffered(char* ptr, size_t size, size_t nmemb,
                           void* userdata) {
    const int fd = *static_cast<int*>(userdata);
    return max(ssize_t(0), write(fd, ptr, size * nmemb));
}
