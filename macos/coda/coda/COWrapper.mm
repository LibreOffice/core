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

#define LIBO_INTERNAL_ONLY
#include <COKit/COKit.hxx>

#import <WebKit/WebKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#import "coda-Swift.h"
#import "COWrapper.h"
#import "macos.h"

// Include necessary C++ headers
#include <thread>
#include <string>
#include <common/Clipboard.hpp>
#include <common/LangUtil.hpp>
#include <common/SettingsStorage.hpp>
#include <common/Log.hpp>
#include <common/ProcUtil.hpp>
#include <common/MobileApp.hpp>
#include <common/Util.hpp>
#include <net/FakeSocket.hpp>
#include <wsd/COOLWSD.hpp>

// Declare the coolwsd pointer at global scope
COOLWSD *coolwsd = nullptr;

// Tracks the system pasteboard state right after our own copy. A later paste can
// then tell whether the pasteboard still holds that copy, and if so reuse the
// engine's full-fidelity in-memory transferable instead of overwriting it with a
// serialized format read back from the pasteboard. This mirrors the ownsClipboard
// check the Qt (Bridge.cpp) and Windows (do_paste_or_read) app variants do.
static NSInteger sOwnedPasteboardChangeCount = -1;
static unsigned sOwnedClipboardDocId = 0;

static int closeNotificationPipeForForwardingThread[2];
static std::thread coolwsdThread;

/**
 * Wrapper to be able to call the C++ code from Swift.
 *
 * The main purpose is to initialize the COOLWSD and interact with it.
 */
@implementation COWrapper

+ (void)startServer {
    // Initialize logging
    // Use "debug" or potentially even "trace" for debugging
#if DEBUG
    Log::initialize("Mobile", "debug");
#else
    Log::initialize("Mobile", "information");
#endif
    ProcUtil::setThreadName("main");

    // Set up the logging callback
    fakeSocketSetLoggingCallback([](const std::string& line) {
        LOG_TRC_NOFILE(line);
    });

    // Start the COOLWSD server in a detached thread
    NSLog(@"CollaboraOffice: Starting the thread");
    coolwsdThread = std::thread([]{
        assert(coolwsd == nullptr);

        // Prepare arguments for COOLWSD
        std::vector<std::string> args = {
            "coda"
        };

        ProcUtil::setThreadName("app");

        coolwsd = new COOLWSD();
        coolwsd->run(args);
        delete coolwsd;
        coolwsd = nullptr; // Reset the pointer after deletion
        NSLog(@"CollaboraOffice: The COOLWSD thread completed");
    });

    // Create a socket pair to notify the thread created in handleHULLOWithDocument:document when the document has been closed
    fakeSocketPipe2(closeNotificationPipeForForwardingThread);
}

+ (void)stopServer {
    NSLog(@"CollaboraOffice: Requesting shutdown");
    SigUtil::requestShutdown();
    fakeSocketClose(closeNotificationPipeForForwardingThread[0]);

    // wait until coolwsdThread is torn down, so that we don't start cleaning up too early
    coolwsdThread.join();
}

+ (void)handleHULLOWithDocument:(Document *)document {
    // Contact the permanently (during app lifetime) listening COOLWSD server
    // "public" socket
    assert(coolwsd_server_socket_fd != -1);
    int rc = fakeSocketConnect(document.fakeClientFd, coolwsd_server_socket_fd);
    assert(rc != -1);

    // Start another thread to read responses and forward them to the JavaScript
    dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       ProcUtil::setThreadName("app2js");
                       while (true) {
                           struct pollfd p[2];
                           p[0].fd = document.fakeClientFd;
                           p[0].events = POLLIN;
                           p[1].fd = closeNotificationPipeForForwardingThread[1];
                           p[1].events = POLLIN;
                           if (fakeSocketPoll(p, 2, -1) > 0) {
                               if (p[1].revents == POLLIN) {
                                   // The code below handling the "BYE" fake Websocket
                                   // message has closed the other end of the
                                   // closeNotificationPipeForForwardingThread. Let's close
                                   // the other end too just for cleanliness, even if a
                                   // FakeSocket as such is not a system resource so nothing
                                   // is saved by closing it.
                                   fakeSocketClose(closeNotificationPipeForForwardingThread[1]);

                                   // Close our end of the fake socket connection to the
                                   // ClientSession thread, so that it terminates
                                   fakeSocketClose(document.fakeClientFd);

                                   return;
                               }
                               if (p[0].revents == POLLIN) {
                                   size_t n = fakeSocketAvailableDataLength(document.fakeClientFd);
                                   // I don't want to check for n being -1 here, even if
                                   // that will lead to a crash (std::length_error from the
                                   // below std::vector constructor), as n being -1 is a
                                   // sign of something being wrong elsewhere anyway, and I
                                   // prefer to fix the root cause. Let's see how well this
                                   // works out. See tdf#122543 for such a case.
                                   if (n == 0)
                                       return;
                                   std::vector<char> buf(n);
                                   n = fakeSocketRead(document.fakeClientFd, buf.data(), n);
                                   [document send2JS:buf.data() length:n];
                               }
                           }
                           else
                               break;
                       }
                       assert(false);
                   });

    // First we simply send the Online C++ parts the URL and the appDocId. This corresponds
    // to the GET request with Upgrade to WebSocket.
    std::string url([[document.tempFileURL absoluteString] UTF8String]);
    // appDocId is read in ClientRequestDispatcher::handleIncomingMessage() in COOLWSD.cpp
    std::string message(url + " " + std::to_string(document.appDocId));
    fakeSocketWriteQueue(document.fakeClientFd, message.c_str(), message.size());
}

+ (void)handleByeWith:(Document *_Nonnull)document {
    // Close one end of the socket pair, that will wake up the forwarding thread
    fakeSocketClose(closeNotificationPipeForForwardingThread[0]);
}

+ (void)handleMessageWith:(Document *)document message:(NSString *)message {
    const char *buf = [message UTF8String];
    fakeSocketWriteQueue(document.fakeClientFd, buf, strlen(buf));
}

+ (void)saveAsWith:(Document *)document url:(NSString *)url format:(NSString *)format filterOptions:(NSString *)filterOptions {
    DocumentData::get(document.appDocId).loKitDocument->saveAs([url UTF8String], [format UTF8String], [filterOptions UTF8String]);
}

/**
 * Map an engine mime type to the pasteboard type other applications expect for
 * it. The common interchange formats map to their system UTI (text/html becomes
 * public.html, and so on). Everything else - notably the internal engine formats
 * - keeps its raw mime string, which is exactly what desktop LibreOffice
 * advertises and reads on the Mac pasteboard, see vcl/osx/DataFlavorMapping.cxx.
 */
+ (NSString *)pasteboardTypeForMime:(NSString *_Nonnull)mime {
    // Drop any parameters such as the charset in "text/plain;charset=utf-8" before
    // the UTI lookup, which would otherwise fail to match.
    NSString * baseMime = [[mime componentsSeparatedByString:@";"] firstObject];

    if ([baseMime isEqualToString:@"text/plain"]) {
        // Normalise to the canonical plain-text type so a single representation
        // wins, and so that other plain-text-like flavours (such as text/markdown)
        // do not collide with it.
        return UTTypeUTF8PlainText.identifier;
    }

    UTType * uti = [UTType typeWithMIMEType:baseMime];
    return (uti != nil && !uti.dynamic) ? uti.identifier : mime;
}

/**
 * Fetch the requested flavours from the engine and put them all on the system
 * pasteboard as raw, unaltered bytes, each under the pasteboard type other apps
 * expect. Returns NO when there was nothing to write.
 *
 * We write through the declareTypes/setData API (the same one desktop LibreOffice
 * uses, see vcl/osx/clipboard.cxx) rather than NSPasteboardItem, because the
 * internal engine formats carry raw mime strings as their type names rather than
 * UTIs. That API accepts them unchanged, which is what lets a paste into desktop
 * LibreOffice keep full fidelity.
 */
+ (BOOL)putOnPasteboard:(const char**)mimeTypes for:(Document *_Nonnull)document {
    size_t outCount = 0;
    char  **outMimeTypes = nullptr;
    size_t *outSizes = nullptr;
    char  **outStreams = nullptr;

    if (!DocumentData::get(document.appDocId).loKitDocument->getClipboard(mimeTypes,
                                                                          &outCount, &outMimeTypes,
                                                                          &outSizes, &outStreams))
    {
        LOG_DBG("failed to fetch mime-types");
        return NO;
    }

    // Collect one representation per pasteboard type, keeping the first we see.
    NSMutableArray<NSPasteboardType> * types = [NSMutableArray array];
    NSMutableDictionary<NSPasteboardType, NSData *> * dataByType = [NSMutableDictionary dictionary];

    for (size_t i = 0; i < outCount; ++i) {
        if (outStreams[i] == nullptr || outSizes[i] == 0)
            continue;

        NSString * type = [COWrapper pasteboardTypeForMime:[NSString stringWithUTF8String:outMimeTypes[i]]];
        if (dataByType[type] != nil)
            continue;

        dataByType[type] = [NSData dataWithBytes:outStreams[i] length:outSizes[i]];
        [types addObject:type];
    }

    if (types.count == 0)
        return NO;

    NSPasteboard * pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:types owner:nil];
    for (NSPasteboardType type in types)
        [pasteboard setData:dataByType[type] forType:type];

    return YES;
}

/**
 * Put the current clipboard content on the system pasteboard. Defaults to text
 * and/or html only when the generic query yields nothing.
 */
+ (BOOL)writeClipboardFor:(Document *_Nonnull)document {
    BOOL written = [COWrapper putOnPasteboard:nullptr for:document];
    if (!written) {
        const char* textMimeTypes[] = {
            "text/plain;charset=utf-8",
            "text/html",
            nullptr
        };
        written = [COWrapper putOnPasteboard:textMimeTypes for:document];
    }

    if (written)
        [COWrapper noteClipboardWrittenBy:document];

    return written;
}

/**
 * Remember the pasteboard state right after we wrote it ourselves.
 */
+ (void)noteClipboardWrittenBy:(Document *_Nonnull)document {
    sOwnedPasteboardChangeCount = [NSPasteboard generalPasteboard].changeCount;
    sOwnedClipboardDocId = document.appDocId;
}

/**
 * Whether the pasteboard still holds the copy this document last wrote. When it
 * does, a paste should use the engine's own clipboard rather than reading the
 * pasteboard back, which both preserves full fidelity and avoids the transfer.
 */
+ (BOOL)pasteboardOwnedBy:(Document *_Nonnull)document {
    return sOwnedPasteboardChangeCount >= 0
        && [NSPasteboard generalPasteboard].changeCount == sOwnedPasteboardChangeCount
        && sOwnedClipboardDocId == document.appDocId;
}

/**
 * Sets the LOKit internal clipboard with the content of NSPasteboard.
 */
+ (void)setClipboardWith:(Document *_Nonnull)document from:(NSPasteboard *_Nonnull)pasteboard {
    // Read every flavour on the pasteboard through the pasteboard-level types API
    // (the same one desktop LibreOffice reads with, see vcl/osx/OSXTransferable.cxx),
    // not just the first item. That way the internal engine formats
    // (application/x-openoffice-*) that another LibreOffice build puts there reach
    // the engine, so a paste from LibreOffice into CODA reconstructs the
    // full-fidelity transferable rather than falling back to RTF or HTML.
    NSMutableArray<NSString *> * orderedMimes = [NSMutableArray array];
    NSMutableDictionary<NSString *, NSData *> * dataByMime = [NSMutableDictionary dictionary];

    for (NSPasteboardType identifier in pasteboard.types) {
        UTType * uti = [UTType typeWithIdentifier:identifier];

        NSString * mime;
        if (uti != nil && [uti conformsToType:UTTypePlainText]) {
            // Several plain-text UTIs (utf8, utf16-external, ...) describe the same
            // text. Keep only the UTF-8 one and tell the engine its charset, so we
            // neither feed duplicates nor mislabel UTF-16 bytes as UTF-8.
            if (![identifier isEqualToString:UTTypeUTF8PlainText.identifier])
                continue;
            mime = @"text/plain;charset=utf-8";
        } else {
            // No system UTI means an internal engine format whose type name is its
            // raw mime string, which we pass through unchanged.
            mime = uti ? uti.preferredMIMEType : identifier;
        }

        if (mime == nil) {
            LOG_WRN("UTI " << [identifier UTF8String] << " did not have associated mime type when deserializing clipboard, skipping...");
            continue;
        }

        // Keep the first representation we see for a given mime.
        if (dataByMime[mime] != nil)
            continue;

        NSData * value = [pasteboard dataForType:identifier];
        if (value == nil)
            continue;

        dataByMime[mime] = value;
        [orderedMimes addObject:mime];
    }

    if (orderedMimes.count == 0)
        return;

    std::vector<const char *> pInMimeTypes(orderedMimes.count);
    std::vector<size_t> pInSizes(orderedMimes.count);
    std::vector<const char *> pInStreams(orderedMimes.count);

    size_t i = 0;
    for (NSString * mime in orderedMimes) {
        pInMimeTypes[i] = [mime UTF8String];
        pInStreams[i] = (const char *)[dataByMime[mime] bytes];
        pInSizes[i] = [dataByMime[mime] length];
        i++;
    }

    DocumentData::get(document.appDocId).loKitDocument->setClipboard(orderedMimes.count,
                                                                     pInMimeTypes.data(),
                                                                     pInSizes.data(),
                                                                     pInStreams.data());
}

/**
 * Insert data into the internal clipboard. The content's format is mimeType\nlegth\ndata\n[...repeat for more mimetypes...].
 */
+ (bool)sendToInternalWith:(Document *_Nonnull)document content:(NSString *_Nonnull)content {
    // If we still own the pasteboard from our own copy, the engine's in-memory
    // transferable is the richer representation, so keep it.
    if ([COWrapper pasteboardOwnedBy:document])
        return true;

    // Otherwise the content came from another app. Ignore the serialized HTML the
    // JavaScript handed us and read every flavour straight off the pasteboard
    // instead. The internal engine formats that desktop LibreOffice puts there
    // never reach the browser's DataTransfer, and they are what a full-fidelity
    // paste needs.
    [COWrapper setClipboardWith:document from:[NSPasteboard generalPasteboard]];
    return true;
}

/**
 * Reuse the common implementation of the check if the message is binary.
 */
+ (bool)isBinaryMessage:(const char *_Nonnull)buffer length:(NSInteger)length {
    return COOLProtocol::isBinaryMessage(buffer, static_cast<size_t>(length));
}

/**
 * We keep a running count of opening documents here. This is not necessarily in sync with the
 * DocBrokerId in DocumentBroker due to potential parallelism when opening multiple documents in
 * quick succession.
 */
static std::atomic<int> appDocIdCounter(1);

+ (int)generateNewAppDocId {
    DocumentData::allocate(appDocIdCounter);
    return appDocIdCounter++;
}

+ (int)fakeSocketSocket {
    return fakeSocketSocket();
}

/**
 * Convert NSString to std::string & call the C++ version of the logging function.
 */
+ (void)LOG_DBG:(NSString *)message {
    std::string stdMessage = [message UTF8String];
    LOG_DBG(stdMessage);
}

+ (void)LOG_ERR:(NSString *)message {
    std::string stdMessage = [message UTF8String];
    LOG_ERR(stdMessage);
}

+ (void)LOG_TRC:(NSString *)message {
    std::string stdMessage = [message UTF8String];
    LOG_TRC(stdMessage);
}

+ (bool)isRtlLanguage:(NSString *)language {
    return LangUtil::isRtlLanguage(std::string([language UTF8String]));
}

+ (bool)darkModeWithSystemDefault:(bool)systemDefault {
    // TODO: persist via Desktop::getDarkMode() once common/SettingsStorage.cpp
    // is added to coda.xcodeproj (it isn't today, so we can't link the
    // Desktop:: layer here). For now follow the system theme on every launch.
    return systemDefault;
}

+ (void)setDarkMode:(bool)value {
    // TODO: persist via Desktop::setDarkMode() once SettingsStorage.cpp is in
    // the Xcode project. Silently no-op for now.
    (void)value;
}

@end
