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
#import <CoreServices/CoreServices.h>

#import "coda-Swift.h"
#import "COWrapper.h"
#import "macos.h"

// Include necessary C++ headers
#include <thread>
#include <string>
#include <set>
#include <common/Clipboard.hpp>
#include <common/LangUtil.hpp>
#include <common/AIHttpTransport.hpp>
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

/**
 * Supplies clipboard bytes to the system pasteboard on demand. After a copy we
 * advertise only the list of formats; when something actually pastes one, the
 * pasteboard asks this owner for that format's bytes, which it fetches from the
 * engine then. Mirrors what desktop LibreOffice does in vcl/osx/clipboard.cxx
 * and what the Qt app does with its LazyClipboardMimeData.
 */
@interface COClipboardOwner : NSObject
- (instancetype)initWithDocId:(unsigned)appDocId
                   mimeByType:(NSDictionary<NSPasteboardType, NSString *> *)mimeByType;
/**
 * Force every advertised format onto the pasteboard now. Used before the source
 * document goes away, so a later paste still finds the content.
 */
- (void)flushTo:(NSPasteboard *)pasteboard;
@property (nonatomic, readonly) unsigned appDocId;
@end

/**
 * The pasteboard does not retain the type owner it is given in declareTypes,
 * so we hold the current lazy owner ourselves until the next advertise (or a
 * flush) replaces it.
 */
static COClipboardOwner *sClipboardOwner = nil;

static int closeNotificationPipeForForwardingThread[2];
static std::thread coolwsdThread;

/**
 * Register the AI chat HTTP transport. The desktop apps have no server-side AI
 * proxy and no COOL net stack for outbound requests, so AIChatSession reaches
 * the provider through this platform hook (the macOS counterpart of Qt's
 * registerAIHttpTransport()). onDone may run on any thread; AIChatSession hops
 * back onto its polling thread itself.
 */
static void registerAIHttpTransport()
{
    ai::setHttpPostFn(
        [](const std::string& url, const std::string& authHeader, std::string body,
           int timeoutSeconds, ai::HttpDoneCallback onDone)
        {
            @autoreleasepool {
                NSURL* nsURL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
                if (nsURL == nil)
                {
                    onDone(ai::HttpConnectFailed, std::string());
                    return;
                }

                NSMutableURLRequest* req = [NSMutableURLRequest requestWithURL:nsURL];
                req.HTTPMethod = @"POST";
                [req setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
                if (!authHeader.empty())
                    [req setValue:[NSString stringWithUTF8String:authHeader.c_str()]
                       forHTTPHeaderField:@"Authorization"];
                if (timeoutSeconds > 0)
                    req.timeoutInterval = (NSTimeInterval)timeoutSeconds;
                req.HTTPBody = [NSData dataWithBytes:body.data() length:body.size()];

                NSURLSessionDataTask* task = [[NSURLSession sharedSession]
                    dataTaskWithRequest:req
                      completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
                          int statusCode;
                          if ([response isKindOfClass:[NSHTTPURLResponse class]])
                              statusCode = (int)[(NSHTTPURLResponse*)response statusCode];
                          else if (error != nil && error.code == NSURLErrorTimedOut)
                              statusCode = ai::HttpNoResponse;
                          else
                              statusCode = ai::HttpConnectFailed;

                          std::string responseBody;
                          if (data != nil && data.length > 0)
                              responseBody.assign(reinterpret_cast<const char*>(data.bytes),
                                                  data.length);

                          onDone(statusCode, std::move(responseBody));
                      }];
                [task resume];
            }
        });
}

/**
 * Pull the bytes the engine holds for one clipboard format of an open document.
 * Returns nil when the document is gone or offers nothing for that format.
 */
static NSData *_Nullable copyEngineClipboardData(unsigned appDocId, const char *mime)
{
    // A still-mounted owner can outlive the document it came from; getIfExists
    // looks the document up without aborting on a removed id.
    DocumentData *docData = DocumentData::getIfExists(appDocId);
    if (!docData || !docData->loKitDocument)
        return nil;
    kit::Document *loKitDoc = docData->loKitDocument;

    // getClipboard acts on the kit's current view, so make this document's view
    // current first.
    int nViewId = -1;
    if (!loKitDoc->getViewIds(&nViewId, 1) || nViewId < 0)
        return nil;
    loKitDoc->setView(nViewId);

    const char *filter[] = { mime, nullptr };
    size_t outCount = 0;
    char **outMimeTypes = nullptr;
    size_t *outSizes = nullptr;
    char **outStreams = nullptr;
    if (!loKitDoc->getClipboard(filter, &outCount, &outMimeTypes, &outSizes, &outStreams)
        || outCount == 0)
        return nil;

    NSData *data = nil;
    if (outStreams[0] && outSizes[0] > 0)
        data = [NSData dataWithBytes:outStreams[0] length:outSizes[0]];
    for (size_t i = 0; i < outCount; ++i) {
        free(outMimeTypes[i]);
        free(outStreams[i]);
    }
    free(outMimeTypes);
    free(outSizes);
    free(outStreams);
    return data;
}

@implementation COClipboardOwner
{
    unsigned _appDocId;
    /**
     * Maps an advertised pasteboard type back to the engine mime it stands for,
     * so a request for that type can fetch the matching format.
     */
    NSDictionary<NSPasteboardType, NSString *> *_mimeByType;
}

- (instancetype)initWithDocId:(unsigned)appDocId
                   mimeByType:(NSDictionary<NSPasteboardType, NSString *> *)mimeByType {
    self = [super init];
    if (self) {
        _appDocId = appDocId;
        _mimeByType = mimeByType;
    }
    return self;
}

- (unsigned)appDocId {
    return _appDocId;
}

- (void)provideType:(NSPasteboardType)type to:(NSPasteboard *)pasteboard {
    NSString *mime = _mimeByType[type];
    if (mime == nil)
        return;

    NSData *data = copyEngineClipboardData(_appDocId, [mime UTF8String]);
    if (data != nil)
        [pasteboard setData:data forType:type];
}

/**
 * Sent by the system when an application reads a type we only promised.
 */
- (void)pasteboard:(NSPasteboard *)sender provideDataForType:(NSPasteboardType)type {
    [self provideType:type to:sender];
}

- (void)flushTo:(NSPasteboard *)pasteboard {
    for (NSPasteboardType type in _mimeByType)
        [self provideType:type to:pasteboard];
}

@end

/**
 * Private read helpers used by the clipboard provider callbacks below.
 */
@interface COWrapper ()
+ (void)ensureClipboardProviderFor:(Document *_Nonnull)document;
+ (char *_Nullable *_Nonnull)copyPasteboardMimeTypes;
+ (BOOL)copyPasteboardData:(NSString *_Nonnull)mime
                       out:(char *_Nullable *_Nonnull)pOutData
                      size:(size_t *_Nonnull)pOutSize;
@end

/**
 * The clipboard provider the engine drives. On copy the engine advertises its
 * formats through advertise; on an external paste it reads the pasteboard one
 * format at a time. pUserData is a retained Document so the callbacks can reuse
 * the document's clipboard helpers.
 */

static void clipboardProviderAdvertise(void* pUserData, const char** pMimeTypes)
{
    @autoreleasepool {
        Document* document = (__bridge Document*)pUserData;
        NSMutableArray<NSString*>* mimes = [NSMutableArray array];
        for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i)
            [mimes addObject:[NSString stringWithUTF8String:pMimeTypes[i]]];
        [COWrapper advertiseClipboardFor:document mimeTypes:mimes];
    }
}

static int clipboardProviderOwns(void* pUserData)
{
    Document* document = (__bridge Document*)pUserData;
    return [COWrapper pasteboardOwnedBy:document] ? 1 : 0;
}

static char** clipboardProviderGetMimeTypes(void* /*pUserData*/)
{
    @autoreleasepool {
        return [COWrapper copyPasteboardMimeTypes];
    }
}

static int clipboardProviderGetData(void* /*pUserData*/, const char* pMimeType, char** pOutData,
                                    size_t* pOutSize)
{
    @autoreleasepool {
        return [COWrapper copyPasteboardData:[NSString stringWithUTF8String:pMimeType]
                                         out:pOutData
                                        size:pOutSize]
                   ? 1
                   : 0;
    }
}

static void clipboardProviderRelease(void* pUserData) { CFBridgingRelease(pUserData); }

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

    // Give AIChatSession a native HTTP transport (no server-side AI proxy here).
    registerAIHttpTransport();

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
    // We load a working copy from a temp dir, so pass the original file the user
    // opened as a third token. The Online parts forward it to the engine as a
    // document load option, so the Properties dialog shows and reveals the real
    // location rather than the temp copy. The URL is percent-encoded (no spaces),
    // so it stays a single token.
    if (document.fileURL != nil)
        message += " " + std::string([[document.fileURL absoluteString] UTF8String]);
    fakeSocketWriteQueue(document.fakeClientFd, message.c_str(), message.size());
}

+ (void)handleByeWith:(Document *_Nonnull)document {
    // Pull any clipboard formats this document only promised onto the pasteboard
    // while its engine is still alive, so a later paste elsewhere still works.
    [COWrapper materializeClipboardFor:document];

    // Close one end of the socket pair, that will wake up the forwarding thread
    fakeSocketClose(closeNotificationPipeForForwardingThread[0]);
}

+ (void)handleMessageWith:(Document *)document message:(NSString *)message {
    // The kit document is ready by the time messages flow; register the clipboard
    // provider as soon as it is, well before the first copy.
    [COWrapper ensureClipboardProviderFor:document];

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
 * Advertise the given formats on the system pasteboard without serializing any
 * bytes. The bytes for a format are fetched from the engine only when something
 * actually reads it, through the COClipboardOwner. The mime types are the ones
 * the engine reported it can offer for the current selection.
 */
+ (BOOL)advertiseClipboardFor:(Document *_Nonnull)document
                   mimeTypes:(NSArray<NSString *> *_Nonnull)mimeTypes {
    NSMutableArray<NSPasteboardType> * types = [NSMutableArray array];
    NSMutableDictionary<NSPasteboardType, NSString *> * mimeByType = [NSMutableDictionary dictionary];

    for (NSString * mime in mimeTypes) {
        NSString * type = [COWrapper pasteboardTypeForMime:mime];
        // Several mimes can fold onto one pasteboard type (the plain-text
        // variants), so keep the first, matching the eager write.
        if (mimeByType[type] != nil)
            continue;
        mimeByType[type] = mime;
        [types addObject:type];
    }

    if (types.count == 0)
        return NO;

    COClipboardOwner * owner = [[COClipboardOwner alloc] initWithDocId:document.appDocId
                                                           mimeByType:mimeByType];
    NSPasteboard * pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:types owner:owner];
    sClipboardOwner = owner;
    [COWrapper noteClipboardWrittenBy:document];
    return YES;
}

/**
 * Serialize every still-promised format onto the pasteboard now. Called just
 * before a document closes, so a paste into another app keeps working once the
 * engine that would have provided the bytes is gone.
 */
+ (void)materializeClipboardFor:(Document *_Nonnull)document {
    if (sClipboardOwner == nil
        || sClipboardOwner.appDocId != document.appDocId
        || ![COWrapper pasteboardOwnedBy:document])
        return;

    [sClipboardOwner flushTo:[NSPasteboard generalPasteboard]];
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
 * Map a pasteboard type back to the engine mime type, or nil if it carries no
 * usable mime.
 */
+ (NSString *_Nullable)mimeForPasteboardType:(NSString *_Nonnull)identifier {
    UTType * uti = [UTType typeWithIdentifier:identifier];

    // Not a uniform type identifier at all: a raw type name, use it as is.
    if (uti == nil)
        return identifier;

    if ([uti conformsToType:UTTypePlainText]) {
        // Several plain-text types describe the same text. Keep only the UTF-8 one
        // and tell the engine its charset, so we neither feed duplicates nor
        // mislabel UTF-16 bytes as UTF-8.
        return [identifier isEqualToString:UTTypeUTF8PlainText.identifier]
            ? @"text/plain;charset=utf-8" : nil;
    }

    // A registered type maps to its mime directly (png, html, rtf, pdf, ...).
    if (!uti.dynamic)
        return uti.preferredMIMEType;

    // The internal engine formats (application/x-openoffice-*) have no registered
    // type, so the system stored each as a dynamic type that keeps the original
    // name under the pasteboard tag class. preferredMIMEType is nil for these;
    // recover the name from that tag class, the same value the Pasteboard Viewer
    // shows.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CFStringRef aTag = UTTypeCopyPreferredTagWithClass((__bridge CFStringRef)identifier,
                                                       CFSTR("com.apple.nspboard-type"));
#pragma clang diagnostic pop
    return aTag != NULL ? (__bridge_transfer NSString *)aTag : nil;
}

/**
 * Register the platform clipboard provider with the engine for this document,
 * once its kit document is ready. After this the engine advertises on copy and
 * reads the pasteboard on paste through the provider callbacks. Safe to call on
 * every message; it registers a document only once.
 */
+ (void)ensureClipboardProviderFor:(Document *_Nonnull)document {
    static std::set<unsigned> sRegistered;
    if (sRegistered.count(document.appDocId))
        return;

    kit::Document * loKitDoc = DocumentData::get(document.appDocId).loKitDocument;
    if (!loKitDoc)
        return; // the kit document is not loaded yet; try again on the next message

    COKitClipboardProvider provider{};
    provider.nSize = sizeof(provider);
    provider.pUserData = (void *)CFBridgingRetain(document);
    provider.advertiseToPlatform = clipboardProviderAdvertise;
    provider.ownsClipboard = clipboardProviderOwns;
    provider.getMimeTypes = clipboardProviderGetMimeTypes;
    provider.getDataForMimeType = clipboardProviderGetData;
    provider.release = clipboardProviderRelease;
    loKitDoc->installClipboardProvider(&provider);

    sRegistered.insert(document.appDocId);
}

/**
 * The engine mime types the system pasteboard currently offers, as a
 * nullptr-terminated malloc'd array of malloc'd strings the engine then frees.
 */
+ (char *_Nullable *_Nonnull)copyPasteboardMimeTypes {
    NSPasteboard * pasteboard = [NSPasteboard generalPasteboard];
    NSMutableArray<NSString *> * mimes = [NSMutableArray array];
    for (NSPasteboardItem * item in pasteboard.pasteboardItems) {
        for (NSPasteboardType identifier in item.types) {
            NSString * mime = [COWrapper mimeForPasteboardType:identifier];
            if (mime != nil && ![mimes containsObject:mime])
                [mimes addObject:mime];
        }
    }

    char ** result = static_cast<char **>(malloc((mimes.count + 1) * sizeof(char *)));
    NSUInteger i = 0;
    for (NSString * mime in mimes)
        result[i++] = strdup([mime UTF8String]);
    result[i] = nullptr;
    return result;
}

/**
 * Copy the bytes the pasteboard holds for one engine mime into a malloc'd buffer
 * the engine then frees. Returns NO when the format is not present.
 */
+ (BOOL)copyPasteboardData:(NSString *_Nonnull)mime
                       out:(char *_Nullable *_Nonnull)pOutData
                      size:(size_t *_Nonnull)pOutSize {
    NSPasteboard * pasteboard = [NSPasteboard generalPasteboard];
    for (NSPasteboardItem * item in pasteboard.pasteboardItems) {
        for (NSPasteboardType identifier in item.types) {
            NSString * candidate = [COWrapper mimeForPasteboardType:identifier];
            if (candidate == nil || ![candidate isEqualToString:mime])
                continue;

            NSData * value = [item dataForType:identifier];
            if (value == nil)
                continue;

            *pOutData = static_cast<char *>(malloc(value.length ? value.length : 1));
            memcpy(*pOutData, value.bytes, value.length);
            *pOutSize = value.length;
            return YES;
        }
    }
    return NO;
}

/**
 * Insert data into the internal clipboard. The content's format is mimeType\nlegth\ndata\n[...repeat for more mimetypes...].
 */
+ (bool)sendToInternalWith:(Document *_Nonnull)document content:(NSString *_Nonnull)content {
    // The clipboard provider serves the paste straight from the platform
    // pasteboard on the following .uno:Paste, where it can reach the internal
    // engine formats that the browser's serialized content cannot, or from our
    // own copy when we still own it. So the content the JavaScript handed us is
    // not needed here.
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
    const std::optional<bool> stored = Desktop::getDarkMode();
    return stored.has_value() ? *stored : systemDefault;
}

+ (void)setDarkMode:(bool)value {
    Desktop::setDarkMode(value);
}

+ (NSString *)fetchSettingsConfig {
    return [NSString stringWithUTF8String:Desktop::fetchSettingsConfig().c_str()];
}

+ (NSString *)fetchSettingsFile:(NSString *)relPath {
    const Desktop::FileResult result =
        Desktop::fetchSettingsFile(std::string([relPath UTF8String]));
    return [NSString stringWithUTF8String:result.content.c_str()];
}

+ (void)uploadSettings:(NSString *)payload {
    Desktop::uploadSettings(std::string([payload UTF8String]));
}

+ (void)syncSettingsWith:(Document *)document {
    // syncSettings() reads viewsetting.json and invokes the callback with a
    // "viewsetting: <json>" frame; hand it straight to the document's JS the
    // same way the kit's own messages are delivered. Mirrors the Qt (Bridge.cpp)
    // and Windows (CODA.cpp) SYNCSETTINGS handlers.
    Desktop::syncSettings([document](const std::vector<char>& data) {
        if (!data.empty())
            [document send2JS:data.data() length:(NSInteger)data.size()];
    });
}

@end

// macOS implementations of the platform-specific Desktop:: path hooks that
// common/SettingsStorage.cpp relies on (Windows provides them in CODA.cpp, Qt
// in Application.cpp). The app is sandboxed, so these resolve inside the app
// container: per-user settings live under Application Support, and the app's
// data is the bundle's Resources directory.
namespace Desktop
{
Poco::Path getConfigPath()
{
    NSURL *appSupport = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                               inDomain:NSUserDomainMask
                                                      appropriateForURL:nil
                                                                 create:YES
                                                                  error:nil];
    if (appSupport == nil)
    {
        LOG_ERR("getConfigPath: could not resolve the Application Support directory");
        return Poco::Path(std::string([NSTemporaryDirectory() UTF8String]));
    }

    // Namespace settings under the app's name so they sit beside the engine's
    // user profile (".../Application Support/Collabora Office/") rather than at
    // the Application Support root, matching Qt's AppConfigLocation. CFBundleName
    // follows --with-app-name, so this tracks the product branding.
    NSString *appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    if (appName.length == 0)
        appName = @"Collabora Office";

    return Poco::Path(std::string([appSupport.path UTF8String]) + "/" +
                      std::string([appName UTF8String]));
}

std::string getDataDir()
{
    return std::string([[[NSBundle mainBundle] resourcePath] UTF8String]);
}
}
