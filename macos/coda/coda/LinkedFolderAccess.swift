/* -*- Mode: Swift; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Cocoa

/// Folders the user allowed the app to read, so that the files a document links to, the chapters
/// of a master document for example, are read where they are. The sandbox lets the app read the
/// file the user opened and nothing next to it; a folder the user chooses is readable, and a
/// security-scoped bookmark keeps it so for later sessions.
enum LinkedFolderAccess {
    private static let defaultsKey = "LinkedFolderBookmarks"

    /// The folders accessed in this session; access lasts until the app quits.
    private static var accessedFolders: [URL] = []

    /// Starts accessing every folder the user allowed in an earlier session.
    static func restore() {
        let bookmarks = UserDefaults.standard.array(forKey: defaultsKey) as? [Data] ?? []
        var kept: [Data] = []
        for data in bookmarks {
            var isStale = false
            guard let url = try? URL(resolvingBookmarkData: data, options: .withSecurityScope,
                                     relativeTo: nil, bookmarkDataIsStale: &isStale) else {
                continue
            }
            if url.startAccessingSecurityScopedResource() {
                accessedFolders.append(url)
            }
            if isStale, let fresh = try? url.bookmarkData(options: .withSecurityScope,
                                                          includingResourceValuesForKeys: nil,
                                                          relativeTo: nil) {
                kept.append(fresh)
            } else {
                kept.append(data)
            }
        }
        UserDefaults.standard.set(kept, forKey: defaultsKey)
    }

    /// The deepest folder that holds all the given files.
    static func commonFolder(of files: [URL]) -> URL? {
        guard var components = files.first?.deletingLastPathComponent().standardizedFileURL.pathComponents else {
            return nil
        }
        for file in files.dropFirst() {
            let other = file.deletingLastPathComponent().standardizedFileURL.pathComponents
            var count = 0
            while count < components.count && count < other.count && components[count] == other[count] {
                count += 1
            }
            components = Array(components.prefix(count))
        }
        return components.isEmpty ? nil : URL(fileURLWithPath: NSString.path(withComponents: components),
                                              isDirectory: true)
    }

    /// Asks the user for the folder that holds the given files, starting in the folder that holds
    /// all of them, and keeps access to it. Calls completion with true when a folder was chosen.
    static func ask(for files: [URL], window: NSWindow?, completion: @escaping (Bool) -> Void) {
        let panel = NSOpenPanel()
        panel.canChooseDirectories = true
        panel.canChooseFiles = false
        panel.allowsMultipleSelection = false
        panel.directoryURL = commonFolder(of: files)
        let names = files.map { $0.lastPathComponent }.joined(separator: ", ")
        panel.message = String(format: NSLocalizedString("This document links to files the app may not read yet: %@. Choose the folder that holds them.", comment: "Folder access panel for the files a document links to"), names)
        panel.prompt = NSLocalizedString("Allow Access", comment: "Folder access panel button")

        let handler: (NSApplication.ModalResponse) -> Void = { response in
            guard response == .OK, let folder = panel.url else {
                completion(false)
                return
            }
            if folder.startAccessingSecurityScopedResource() {
                accessedFolders.append(folder)
            }
            if let data = try? folder.bookmarkData(options: .withSecurityScope,
                                                   includingResourceValuesForKeys: nil,
                                                   relativeTo: nil) {
                // A folder allowed again replaces its bookmark.
                let path = folder.standardizedFileURL.path
                var bookmarks = (UserDefaults.standard.array(forKey: defaultsKey) as? [Data] ?? []).filter {
                    var isStale = false
                    let url = try? URL(resolvingBookmarkData: $0, options: .withSecurityScope,
                                       relativeTo: nil, bookmarkDataIsStale: &isStale)
                    return url?.standardizedFileURL.path != path
                }
                bookmarks.append(data)
                UserDefaults.standard.set(bookmarks, forKey: defaultsKey)
            }
            completion(true)
        }

        if let window {
            panel.beginSheetModal(for: window, completionHandler: handler)
        } else {
            handler(panel.runModal())
        }
    }
}
