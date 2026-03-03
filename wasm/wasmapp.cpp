/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "wasmapp.hpp"

#include <common/Log.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/FakeSocket.hpp>
#include <wsd/COOLWSD.hpp>

#include <emscripten/fetch.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

int coolwsd_server_socket_fd = -1;

static std::optional<std::string> tempFile;
    // empty when operating on a local file in the Emscripten file system
static std::string remoteUrl;
static std::string fileURL;
static int fakeClientFd;
static int closeNotificationPipeForForwardingThread[2] = {-1, -1};

static void send2JS(const std::vector<char>& buffer)
{
    MAIN_THREAD_EM_ASM({
        // Check if the message is binary. We say that any message that isn't just a single line is
        // "binary" even if that strictly speaking isn't the case; for instance the commandvalues:
        // message has a long bunch of non-binary JSON on multiple lines. But _onMessage() in
        // Socket.js handles it fine even if such a message, too, comes in as an ArrayBuffer. (Look
        // for the "textMsg = String.fromCharCode.apply(null, imgBytes);".)

        let newline = false;
        for (let i = 0; i != $1; ++i) {
            if (HEAPU8[$0 + i] === 0x0A) {
                newline = true;
                break;
            }
        }
        let data = HEAPU8.slice($0, $0 + $1);
        if (!newline) {
            data = new TextDecoder().decode(data);
        }

        globalThis.TheFakeWebSocket.onmessage({data});
    }, buffer.data(), buffer.size());
}

extern "C"
void handle_cool_message(const char *string_value)
{
    std::cout << "================ handle_cool_message(): '" << string_value << "'" << std::endl;

    if (string_value == std::string_view("HULLO"))
    {
        // Now we know that the JS has started completely

        // Contact the permanently (during app lifetime) listening COOLWSD server
        // "public" socket
        assert(coolwsd_server_socket_fd != -1);
        int rc = fakeSocketConnect(fakeClientFd, coolwsd_server_socket_fd);
        assert(rc != -1);

        // Create a socket pair to notify the below thread when the document has been closed
        fakeSocketPipe2(closeNotificationPipeForForwardingThread);

        // Start another thread to read responses and forward them to the JavaScript
        std::thread([]
                    {
                        ProcUtil::setThreadName("app2js");
                        while (true)
                        {
                           struct pollfd pollfd[2];
                           pollfd[0].fd = fakeClientFd;
                           pollfd[0].events = POLLIN;
                           pollfd[1].fd = closeNotificationPipeForForwardingThread[1];
                           pollfd[1].events = POLLIN;
                           if (fakeSocketPoll(pollfd, 2, -1) > 0)
                           {
                               if (pollfd[1].revents == POLLIN)
                               {
                                   // The code below handling the "BYE" fake Websocket
                                   // message has closed the other end of the
                                   // closeNotificationPipeForForwardingThread. Let's close
                                   // the other end too just for cleanliness, even if a
                                   // FakeSocket as such is not a system resource so nothing
                                   // is saved by closing it.
                                   fakeSocketClose(closeNotificationPipeForForwardingThread[1]);

                                   // Close our end of the fake socket connection to the
                                   // ClientSession thread, so that it terminates
                                   fakeSocketClose(fakeClientFd);

                                   return;
                               }
                               if (pollfd[0].revents == POLLIN)
                               {
                                   int n = fakeSocketAvailableDataLength(fakeClientFd);
                                   if (n == 0)
                                       return;
                                   std::vector<char> buf(n);
                                   n = fakeSocketRead(fakeClientFd, buf.data(), n);
                                   send2JS(buf);
                               }
                           }
                           else
                               break;
                       }
                       assert(false);
                    }).detach();

        // First we simply send it the URL. This corresponds to the GET request with Upgrade to
        // WebSocket.
        LOG_TRC_NOFILE("Actually sending to Online:" << fileURL);
        std::cout << "Loading file [" << fileURL << "]" << std::endl;

        fakeSocketWriteQueue(fakeClientFd, fileURL.c_str(), fileURL.size());
    }
    else if (string_value == std::string_view("BYE"))
    {
        LOG_TRC_NOFILE("Document window terminating on JavaScript side. Closing our end of the socket.");

        // Close one end of the socket pair, that will wake up the forwarding thread above
        fakeSocketClose(closeNotificationPipeForForwardingThread[0]);
    }
    else
    {
        fakeSocketWriteQueue(fakeClientFd, string_value, strlen(string_value));
    }
}

namespace {
struct FileClose {
    void operator ()(FILE * f) { std::fclose(f); }
};
}

void saveToServer() {
    if (!tempFile) {
        return;
    }
    long n;
    std::unique_ptr<char[]> buf;
    {
        auto const f = std::unique_ptr<FILE, FileClose>(std::fopen(tempFile->c_str(), "r"));
        if (f.get() == nullptr) {
            LOG_WRN("Failed to open " << *tempFile << " for reading"); //TODO
            return;
        }
        int e = std::fseek(f.get(), 0, SEEK_END);
        if (e != 0) {
            LOG_WRN("Failed to seek in " << *tempFile); //TODO
            return;
        }
        n = std::ftell(f.get());
        if (n == -1) {
            LOG_WRN("Failed to get size of " << *tempFile); //TODO
            return;
        }
        buf = std::make_unique<char[]>(n);
        std::rewind(f.get());
        std::size_t n2 = std::fread(buf.get(), 1, n, f.get());
        assert(n >= 0);
        if (n2 != static_cast<unsigned long>(n)) {
            LOG_WRN("Failed to get read " << *tempFile); //TODO
            return;
        }
    }

    // Call back to JS with the file data; JS handles the token exchange and upload
    MAIN_THREAD_EM_ASM({
        if (typeof globalThis.collabSaveToServer === 'function') {
            var bytes = HEAPU8.slice($0, $0 + $1);
            globalThis.collabSaveToServer(bytes);
        } else {
            console.error('collabSaveToServer not available');
        }
    }, buf.get(), n);
    LOG_TRC("Initiated save of " << *tempFile << " (" << n << " bytes) via JS callback");
}

int main(int argc, char* argv_main[])
{
    std::cout << "================ Here is main()" << std::endl;

    assert(argc == 3);

    Log::initialize("WASM", "error");
    ProcUtil::setThreadName("main");

    fakeSocketSetLoggingCallback([](const std::string& line)
                                 {
                                     LOG_TRC_NOFILE(line);
                                 });

    char *argv[2];
    argv[0] = strdup("wasm");
    argv[1] = nullptr;

    fakeClientFd = fakeSocketSocket();

    // We run COOOLWSD::run() in a thread of its own so that main() can return.
    std::thread(
        [&]
        {
            ProcUtil::setThreadName("COOLWSD::run");

            const std::string docKind = std::string(argv_main[1]);
            const std::string docDesc = std::string(argv_main[2]);

            if (docKind == "server" || docKind == "collab")
            {
                // 'server' mode: URL is encoded WOPI source, prepend /wasm/ to fetch
                // 'collab' mode: URL is already a full path (e.g., /co/collab/fetch?token=...)
                if (docKind == "server")
                    remoteUrl = "/wasm/" + docDesc;
                else
                    remoteUrl = docDesc; // collab mode: use URL directly

                printf("Fetching from url %s (mode: %s)\n", remoteUrl.c_str(), docKind.c_str());

                emscripten_fetch_attr_t attr;
                emscripten_fetch_attr_init(&attr);
                strcpy(attr.requestMethod, "GET");
                attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
                emscripten_fetch_t* fetch = emscripten_fetch(
                    &attr, remoteUrl.data()); // Blocks here until the operation is complete.
                if (fetch->status == 200)
                {
                    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes,
                           fetch->url);
                    // Use the original filename from WOPI if available:
                    std::uint32_t n;
                    std::unique_ptr<char, void (*)(char *)> p(
                        static_cast<char *>(
                            MAIN_THREAD_EM_ASM_PTR({
                                if (globalThis.collabFilename === undefined) {
                                    return null;
                                }
                                const n = lengthBytesUTF8(globalThis.collabFilename);
                                const p = _malloc(n + 1);
                                if (!p) { // guard against ABORTING_MALLOC=0
                                    throw new Error("out of memory");
                                }
                                stringToUTF8(globalThis.collabFilename, p, n + 1);
                                HEAP32[$0 >> 2] = n;
                                return p;
                            }, &n)),
                        [](char * p) { std::free(p); });
                    std::string filename;
                    if (p) {
                        filename.assign(p.get(), n);
                    }
                    std::replace_if(
                        filename.begin(), filename.end(),
                        [](char c) { return c == '\0' || c == '/'; }, '_');
                    if (filename.empty()) {
                        filename = "_";
                    }
                    //TODO: Use distinct mktemp-style directories if there can be multiple files
                    // open simultaneously:
                    tempFile = "/tempdoc/" + filename;
                    // With the Emscripten file system, there's no length limit on how long the
                    // tempFile pathname can be (though core sal code likely has some PATH_MAX
                    // limits, would should eventually be addressed one way or another):
                    FILE* f = fopen(tempFile->c_str(), "w");
                    const int wrote = fwrite(fetch->data, 1, fetch->numBytes, f);
                    fclose(f);
                    printf("Wrote %d bytes into %s\n", wrote, tempFile->c_str());
                    fileURL = "file://" + Uri::encode(*tempFile);
                }
                else
                {
                    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url,
                           fetch->status);
                    std::exit(EXIT_FAILURE); //TODO: error handling
                }
                emscripten_fetch_close(fetch);
            }
            else if (docKind == "local")
            {
                fileURL = docDesc;
            }
            else
            {
                assert(false);
            }

            COOLWSD *coolwsd = new COOLWSD();
            coolwsd->run(1, argv);
            delete coolwsd;
        })
        .detach();

    std::cout << "================ main() is returning" << std::endl;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
