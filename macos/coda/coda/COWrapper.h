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
+ (void)ensureClipboardProviderFor:(Document *_Nonnull)document NS_SWIFT_NAME(ensureClipboardProvider(for:));
+ (void)saveAsWith:(Document *_Nonnull)document url:(NSString *_Nonnull)url format:(NSString *_Nonnull)format filterOptions:(NSString *_Nullable)filterOptions;
+ (BOOL)advertiseClipboardFor:(Document *_Nonnull)document mimeTypes:(NSArray<NSString *> *_Nonnull)mimeTypes NS_SWIFT_NAME(advertiseClipboard(for:mimeTypes:));
+ (void)materializeClipboardFor:(Document *_Nonnull)document NS_SWIFT_NAME(materializeClipboard(for:));
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

// Options dialog (adminIntegratorSettings) native backend. These bridge to the
// shared Desktop:: layer in common/SettingsStorage.cpp, mirroring the Windows
// (CODA.cpp) and Qt (Bridge.cpp) apps.
+ (NSString *_Nonnull)fetchSettingsConfig;
+ (NSString *_Nonnull)fetchSettingsFile:(NSString *_Nonnull)relPath NS_SWIFT_NAME(fetchSettingsFile(_:));
+ (void)uploadSettings:(NSString *_Nonnull)payload NS_SWIFT_NAME(uploadSettings(_:));
// Push the saved view settings (viewsetting.json) into the running document's
// JS so the kit applies them (AI credentials, Zotero, signing cert, ...). Sent
// by the client (main.js) on load via the SYNCSETTINGS message.
+ (void)syncSettingsWith:(Document *_Nonnull)document NS_SWIFT_NAME(syncSettings(with:));

@end
