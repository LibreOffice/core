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
    if (!sOffice)
        return nil;

    const char *filter[] = { mime, nullptr };
    std::vector<std::string> outMimeTypes;
    std::vector<std::vector<char>> outStreams;
    if (!sOffice->getGlobalClipboard(filter, outMimeTypes, outStreams)
        || outStreams.size() == 0)
        return nil;

    NSData *data = nil;
    if (outStreams[0].size() > 0)
        data = [NSData dataWithBytes:outStreams[0].data() length:outStreams[0].size()];
    return data;
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

// vim:set shiftwidth=4 softtabstop=4 expandtab:
