// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ios.h"

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

extern "C" {
#import <native-code.h>
}

#import <cstdlib>
#import <cstring>
#import <string>
#import <vector>

#import "Log.hpp"

const char *user_name = nullptr;

int coolwsd_server_socket_fd = -1;

COKit *lo_kit;

// The engine office handle, captured when the clipboard provider is installed, so a clipboard
// read can go straight to the process-shared clipboard without needing a particular document.
static COKit *sOffice = nullptr;

// Tracks the system pasteboard state right after our own copy. A later paste can then tell
// whether the pasteboard still holds that copy, and if so reuse the engine's full-fidelity
// in-memory transferable instead of the serialized formats read back from the pasteboard. The
// same changeCount check the macOS app does in COWrapper.mm.
static NSInteger sOwnedPasteboardChangeCount = -1;

// True while advertiseToPlatform() serializes the engine's fresh copy onto the pasteboard. The
// serialization pulls each format from the engine, and the engine only serves those bytes from
// its in-memory transferable when it considers the clipboard ours, so ownsClipboard() reports
// ours for the whole write.
static bool sAdvertising = false;

/**
 * Map an engine mime type to the pasteboard type other applications expect for it. The common
 * interchange formats map to their system UTI (text/html becomes public.html, and so on).
 * Everything else - notably the internal engine formats - keeps its raw mime string, the same
 * mapping the macOS app uses.
 */
static NSString *pasteboardTypeForMime(NSString *mime)
{
    // Drop any parameters such as the charset in "text/plain;charset=utf-8" before the UTI
    // lookup, which would otherwise fail to match.
    NSString *baseMime = [[mime componentsSeparatedByString:@";"] firstObject];

    if ([baseMime isEqualToString:@"text/plain"]) {
        // Normalise to the canonical plain-text type so a single representation wins, and so
        // that other plain-text-like flavours do not collide with it.
        return UTTypeUTF8PlainText.identifier;
    }

    UTType *uti = [UTType typeWithMIMEType:baseMime];
    return (uti != nil && !uti.dynamic) ? uti.identifier : mime;
}

/**
 * Map a pasteboard type back to the engine mime type, or nil if it carries no usable mime.
 */
static NSString *_Nullable mimeForPasteboardType(NSString *identifier)
{
    UTType *uti = [UTType typeWithIdentifier:identifier];

    // Not a uniform type identifier at all: a raw type name (the internal engine formats are
    // advertised under their raw mime strings), use it as is.
    if (uti == nil)
        return identifier;

    if ([uti conformsToType:UTTypePlainText]) {
        // Several plain-text types describe the same text. Keep only the UTF-8 one and tell the
        // engine its charset, so we neither feed duplicates nor mislabel other encodings as
        // UTF-8.
        return [identifier isEqualToString:UTTypeUTF8PlainText.identifier]
            ? @"text/plain;charset=utf-8" : nil;
    }

    // A registered type maps to its mime directly (png, html, rtf, pdf, ...).
    if (!uti.dynamic)
        return uti.preferredMIMEType;

    return nil;
}

/**
 * Pull the bytes the engine holds for one clipboard format. The clipboard is process-global (one
 * shared clipboard for the whole app), so read it straight from the office; no document is
 * involved. Returns nil when the engine offers nothing for that format.
 */
static NSData *_Nullable copyEngineClipboardData(const char *mime)
{
    std::vector<char> bytes;
    if (!sOffice || !sOffice->getGlobalClipboardData(mime, bytes))
        return nil;

    return [NSData dataWithBytes:bytes.data() length:bytes.size()];
}

/**
 * The URL a pasteboard that carries one points to, when it looks like an image: its path
 * extension maps to a type that conforms to public.image. Returns nil otherwise. Reading the
 * URL's value shows the system's paste banner.
 */
static NSURL *_Nullable imageURLOnPasteboard()
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    if (!pasteboard.hasURLs)
        return nil;

    NSURL *url = pasteboard.URL;
    if (url == nil)
        return nil;

    NSString *extension = url.pathExtension;
    UTType *uti = extension.length > 0 ? [UTType typeWithFilenameExtension:extension] : nil;
    if (uti == nil || ![uti conformsToType:UTTypeImage]) {
        LOG_DBG("clipboard provider: the pasteboard URL " << [url.absoluteString UTF8String]
                << " does not look like an image, ignoring it");
        return nil;
    }

    return url;
}

/**
 * Download an image and re-encode it as PNG. UIImage decodes every format iOS knows (webp,
 * heic, ...), so the result is a format the engine pastes regardless of what the web serves.
 * Runs synchronously, with a timeout; returns nil when the download or the decode fails.
 */
static NSData *_Nullable downloadImageAsPNG(NSURL *url)
{
    LOG_DBG("clipboard provider: downloading the pasteboard image "
            << [url.absoluteString UTF8String]);

    __block NSData *body = nil;
    dispatch_semaphore_t done = dispatch_semaphore_create(0);
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    request.timeoutInterval = 15;
    NSURLSessionDataTask *task = [[NSURLSession sharedSession]
        dataTaskWithRequest:request
          completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
              if (error != nil)
                  LOG_DBG("clipboard provider: downloading the pasteboard image failed: "
                          << [error.localizedDescription UTF8String]);
              else
                  body = data;
              dispatch_semaphore_signal(done);
          }];
    [task resume];
    dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 20 * NSEC_PER_SEC));

    if (body == nil)
        return nil;

    UIImage *image = [UIImage imageWithData:body];
    if (image == nil) {
        LOG_DBG("clipboard provider: the downloaded data does not decode as an image");
        return nil;
    }

    NSData *png = UIImagePNGRepresentation(image);
    LOG_DBG("clipboard provider: converted the downloaded image to "
            << (size_t)(png != nil ? png.length : 0) << " bytes of PNG");
    return png;
}

/**
 * The clipboard provider the engine drives. On copy the engine advertises its formats through
 * advertise; on an external paste it reads the pasteboard one format at a time. The callbacks
 * act on the process, not one document, so the one shared clipboard is reached from whichever
 * document is current.
 */

static void clipboardProviderAdvertise(const char** pMimeTypes)
{
    @autoreleasepool {
        // UIPasteboard has no lazy owner like the macOS NSPasteboard, so serialize every
        // advertised format now, into a single pasteboard item.
        NSMutableDictionary<NSString *, id> *item = [NSMutableDictionary dictionary];

        sAdvertising = true;
        for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i) {
            NSString *mime = [NSString stringWithUTF8String:pMimeTypes[i]];
            NSString *type = pasteboardTypeForMime(mime);

            // Several mimes can fold onto one pasteboard type (the plain-text variants), so
            // keep the first.
            if (item[type] != nil)
                continue;

            NSData *data = copyEngineClipboardData(pMimeTypes[i]);
            if (data != nil)
                item[type] = data;
        }
        sAdvertising = false;

        if (item.count == 0)
            return;

        UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
        pasteboard.items = @[ item ];
        sOwnedPasteboardChangeCount = pasteboard.changeCount;
    }
}

static bool clipboardProviderOwns()
{
    if (sAdvertising)
        return true;
    return sOwnedPasteboardChangeCount >= 0
           && [UIPasteboard generalPasteboard].changeCount == sOwnedPasteboardChangeCount;
}

static std::vector<std::string> clipboardProviderGetMimeTypes()
{
    @autoreleasepool {
        UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
        // List the types without reading any values. Reading a value (which the items property
        // does for every representation at once) makes the pasteboard materialize everything
        // the source app promised: that can block for many seconds, and the representations
        // that cannot be delivered fall out of the list. The type list alone is available
        // immediately.
        NSArray<NSArray<NSString *> *> *itemTypes = [pasteboard pasteboardTypesForItemSet:nil];
        NSMutableArray<NSString *> *mimes = [NSMutableArray array];
        for (NSArray<NSString *> *types in itemTypes) {
            for (NSString *identifier in types) {
                NSString *mime = mimeForPasteboardType(identifier);
                if (mime != nil && ![mimes containsObject:mime])
                    [mimes addObject:mime];
            }
        }

        // A web image copied in another app often arrives as nothing but its URL, under
        // public.url. When the types alone gave nothing to paste and that URL looks like an
        // image, promise image/png: getDataForMimeType() downloads the image and re-encodes
        // it. The check reads the URL's value, so it stays behind the nothing-to-paste
        // condition and the paste banner only shows for a paste that would otherwise fail.
        if (mimes.count == 0) {
            NSURL *url = imageURLOnPasteboard();
            if (url != nil) {
                LOG_DBG("clipboard provider: getMimeTypes: promising image/png for "
                        << [url.absoluteString UTF8String]);
                [mimes addObject:@"image/png"];
            }
        }

        std::vector<std::string> result;
        result.reserve(mimes.count);
        for (NSString *mime in mimes)
            result.emplace_back([mime UTF8String]);
        return result;
    }
}

static bool clipboardProviderGetData(const char* pMimeType, std::vector<char>* pOutData)
{
    @autoreleasepool {
        NSString *wanted = [NSString stringWithUTF8String:pMimeType];
        UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
        NSArray<NSArray<NSString *> *> *itemTypes = [pasteboard pasteboardTypesForItemSet:nil];
        for (NSUInteger i = 0; i < itemTypes.count; ++i) {
            for (NSString *identifier in itemTypes[i]) {
                NSString *candidate = mimeForPasteboardType(identifier);
                if (candidate == nil || ![candidate isEqualToString:wanted])
                    continue;

                // Fetch just this one representation of this one item; the source app then
                // serializes only the format the engine actually pastes.
                NSIndexSet *index = [NSIndexSet indexSetWithIndex:i];
                id value =
                    [[pasteboard dataForPasteboardType:identifier inItemSet:index] firstObject];

                // A value stored as a plain string arrives as one; the engine expects bytes.
                if ([value isKindOfClass:[NSString class]])
                    value = [(NSString *)value dataUsingEncoding:NSUTF8StringEncoding];

                if (![value isKindOfClass:[NSData class]])
                    continue;

                NSData *data = (NSData *)value;
                pOutData->assign(static_cast<const char *>(data.bytes),
                                 static_cast<const char *>(data.bytes) + data.length);
                return true;
            }
        }

        // No pasteboard type carries the format directly; serve the image/png promised by
        // getMimeTypes() for a pasteboard that holds an image URL.
        if ([wanted isEqualToString:@"image/png"]) {
            NSURL *url = imageURLOnPasteboard();
            NSData *png = url != nil ? downloadImageAsPNG(url) : nil;
            if (png != nil) {
                pOutData->assign(static_cast<const char *>(png.bytes),
                                 static_cast<const char *>(png.bytes) + png.length);
                return true;
            }
        }

        return false;
    }
}

// Install the process-global clipboard provider (declared in ios.h). After this the engine
// advertises formats on copy and reads the pasteboard on paste through the callbacks above,
// using one shared clipboard for every document.
void install_clipboard_provider(COKit &rOffice)
{
    sOffice = &rOffice;

    static COKitClipboardProvider provider{};
    provider.advertiseToPlatform = clipboardProviderAdvertise;
    provider.ownsClipboard = clipboardProviderOwns;
    provider.getMimeTypes = clipboardProviderGetMimeTypes;
    provider.getDataForMimeType = clipboardProviderGetData;
    rOffice.installClipboardProvider(&provider);
}

@class FilePickerDelegate;

// The pick the engine is waiting on, nil when none is open. Only the main queue touches
// these.
static FilePickerDelegate *sOpenPick = nil;

// The copy of the last picked file, handed to the engine and removed when the next pick
// starts.
static NSURL *sPreviousPickedCopy = nil;

static void removePreviousPickedCopy()
{
    if (sPreviousPickedCopy == nil)
        return;
    [NSFileManager.defaultManager removeItemAtURL:sPreviousPickedCopy error:nil];
    sPreviousPickedCopy = nil;
}

// Carries one pick's completion while the document picker is on screen and runs it once,
// with the picked file or with none.
@interface FilePickerDelegate : NSObject <UIDocumentPickerDelegate>
{
@public
    void (*pfnPicked)(void *pContext, const char *pUrl);
    void *pContext;
    UIDocumentPickerViewController *picker;
}
- (void)answerWithURL:(NSURL *)url;
@end

@implementation FilePickerDelegate

- (void)answerWithURL:(NSURL *)url
{
    if (pfnPicked == nullptr)
        return;
    void (*deliver)(void *, const char *) = pfnPicked;
    pfnPicked = nullptr;
    picker = nil;
    if (sOpenPick == self)
        sOpenPick = nil;
    if (url != nil) {
        sPreviousPickedCopy = url;
        deliver(pContext, url.absoluteString.UTF8String);
    } else {
        deliver(pContext, nullptr);
    }
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller
    didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls
{
    [self answerWithURL:urls.firstObject];
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller
{
    [self answerWithURL:nil];
}

@end

// The top of the presentation chain in the foreground scene's key window, or nil when no
// scene is in the foreground.
static UIViewController *_Nullable presentationViewController()
{
    for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if (scene.activationState != UISceneActivationStateForegroundActive
            || ![scene isKindOfClass:[UIWindowScene class]])
            continue;
        UIWindowScene *windowScene = (UIWindowScene *)scene;
        UIWindow *window = windowScene.keyWindow;
        if (window == nil)
            window = windowScene.windows.firstObject;
        UIViewController *controller = window.rootViewController;
        while (controller.presentedViewController != nil
               && !controller.presentedViewController.isBeingDismissed)
            controller = controller.presentedViewController;
        if (controller != nil)
            return controller;
    }
    return nil;
}

// The engine asks the user for a file. The filters are converted on the calling thread, the
// picker is shown on the main queue, and the delegate answers.
static void filePickerProviderPick(const char *pTitle, const COKitFilePickerFilter *pFilters,
                                   size_t nFilters,
                                   void (*pfnPicked)(void *pContext, const char *pUrl),
                                   void *pContext)
{
    @autoreleasepool {
        // The system document picker has no title line.
        (void)pTitle;

        NSMutableArray<UTType *> *types = [NSMutableArray array];
        for (size_t i = 0; i < nFilters; ++i) {
            NSString *wildcards = [NSString stringWithUTF8String:pFilters[i].pWildcards];
            for (NSString *wildcard in [wildcards componentsSeparatedByString:@";"]) {
                // "*.png": the extension follows the last dot. "*.*" names no type and is
                // skipped, so a list of only those stays empty and allows any file.
                NSRange dot = [wildcard rangeOfString:@"." options:NSBackwardsSearch];
                if (dot.location == NSNotFound)
                    continue;
                NSString *extension = [wildcard substringFromIndex:dot.location + 1];
                if (extension.length == 0
                    || [extension rangeOfCharacterFromSet:
                        [NSCharacterSet characterSetWithCharactersInString:@"*?"]].location
                        != NSNotFound)
                    continue;
                UTType *type = [UTType typeWithFilenameExtension:extension];
                if (type != nil && ![types containsObject:type])
                    [types addObject:type];
            }
        }
        if (types.count == 0)
            [types addObject:UTTypeData];

        dispatch_async(dispatch_get_main_queue(), ^{
            // One pick at a time; an earlier one still on screen is cancelled first.
            if (sOpenPick != nil) {
                UIDocumentPickerViewController *stale = sOpenPick->picker;
                [sOpenPick answerWithURL:nil];
                [stale.presentingViewController dismissViewControllerAnimated:NO
                                                                   completion:nil];
            }

            removePreviousPickedCopy();

            UIViewController *presenter = presentationViewController();
            if (presenter == nil) {
                LOG_WRN("No foreground scene to present the document picker from; "
                        "the pick is cancelled");
                pfnPicked(pContext, nullptr);
                return;
            }

            // asCopy gives a copy inside the app sandbox, readable without security-scoped
            // access.
            UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc]
                initForOpeningContentTypes:types asCopy:YES];
            picker.allowsMultipleSelection = NO;

            // The picker holds its delegate weakly; sOpenPick keeps it alive.
            FilePickerDelegate *delegate = [[FilePickerDelegate alloc] init];
            delegate->pfnPicked = pfnPicked;
            delegate->pContext = pContext;
            delegate->picker = picker;
            picker.delegate = delegate;
            sOpenPick = delegate;

            [presenter presentViewController:picker animated:YES completion:nil];
        });
    }
}

// Answer the open pick, if any, as cancelled and dismiss its picker (declared in ios.h). On
// the main thread this is done before the call returns; elsewhere it goes through the main
// queue.
void cancel_filepicker_pick()
{
    void (^cancel)(void) = ^{
        if (sOpenPick == nil)
            return;
        UIDocumentPickerViewController *picker = sOpenPick->picker;
        [sOpenPick answerWithURL:nil];
        removePreviousPickedCopy();
        if (picker.presentingViewController != nil)
            [picker.presentingViewController dismissViewControllerAnimated:NO completion:nil];
    };
    if (NSThread.isMainThread)
        cancel();
    else
        dispatch_async(dispatch_get_main_queue(), cancel);
}

// Install the process-global file picker provider (declared in ios.h). After this a command
// that needs the user to pick a file shows the system document picker instead of the engine's
// own file dialog.
void install_filepicker_provider(COKit &rOffice)
{
    static COKitFilePickerProvider provider{};
    provider.pick = filePickerProviderPick;
    rOffice.installFilePickerProvider(&provider);
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
