/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@class Document;

@interface COWrapper : NSObject {
}

+ (void)startServer;
+ (void)stopServer;

+ (void)handleHULLOWithDocument:(Document *_Nonnull)document;
+ (void)handleByeWith:(Document *_Nonnull)document NS_SWIFT_NAME(bye(_:));
+ (void)handleMessageWith:(Document *_Nonnull)document message:(NSString *_Nonnull)message;
+ (void)saveAsWith:(Document *_Nonnull)document url:(NSString *_Nonnull)url format:(NSString *_Nonnull)format filterOptions:(NSString *_Nullable)filterOptions;
+ (BOOL)writeClipboardFor:(Document *_Nonnull)document NS_SWIFT_NAME(writeClipboard(for:));
+ (void)setClipboardWith:(Document *_Nonnull)document from:(NSPasteboard *_Nonnull)pasteboard NS_SWIFT_NAME(setClipboard(_:from:));
+ (bool)sendToInternalWith:(Document *_Nonnull)document content:(NSString *_Nonnull)content NS_SWIFT_NAME(sendToInternalClipboard(_:content:));

// Record that we just wrote the system pasteboard ourselves, and ask later
// whether that copy is still the one on the pasteboard. Lets a paste reuse the
// engine's full-fidelity in-memory clipboard instead of a serialized round-trip.
+ (void)noteClipboardWrittenBy:(Document *_Nonnull)document NS_SWIFT_NAME(noteClipboardWritten(by:));
+ (BOOL)pasteboardOwnedBy:(Document *_Nonnull)document NS_SWIFT_NAME(pasteboardOwned(by:));

+ (bool)isBinaryMessage:(const char *_Nonnull)buffer length:(NSInteger)length;

+ (int)generateNewAppDocId;
+ (int)fakeSocketSocket;

+ (void)LOG_DBG:(NSString *_Nonnull)message NS_SWIFT_NAME(LOG_DBG(_:));
+ (void)LOG_ERR:(NSString *_Nonnull)message NS_SWIFT_NAME(LOG_ERR(_:));
+ (void)LOG_TRC:(NSString *_Nonnull)message NS_SWIFT_NAME(LOG_TRC(_:));

+ (bool)isRtlLanguage:(NSString *_Nonnull)language;

+ (bool)darkModeWithSystemDefault:(bool)systemDefault NS_SWIFT_NAME(darkMode(systemDefault:));
+ (void)setDarkMode:(bool)value NS_SWIFT_NAME(setDarkMode(_:));

@end
