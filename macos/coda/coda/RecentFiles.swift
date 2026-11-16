/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import AppKit

class RecentFiles {

    /**
     * Entry representing a recent file.
     */
    private struct RecentFile: Codable {
        let uri: String
        let name: String
        let timestamp: String? // NSDocumentController doesn't reliably expose per-item timestamps
    }

    /**
     * Returns NSDocumentController's recent docs list as JSON, without the documents that are
     * open and without the welcome slideshow:
     * [
     *   { "uri": "...", "name": "...", "timestamp": "..."? },
     *   ...
     * ]
     */
    static func serialize() -> String {
        let controller = NSDocumentController.shared
        let openFiles = Set(controller.documents.compactMap { $0.fileURL.map(comparable) })
        let urls = controller.recentDocumentURLs.filter {
            !openFiles.contains(comparable($0)) && !DocumentController.isWelcomeSlideshow($0)
        }

        let items: [RecentFile] = urls.map { url in
            RecentFile(
                uri: url.absoluteString,
                name: url.lastPathComponent,
                timestamp: bestEffortTimestamp(for: url)
            )
        }

        let encoder = JSONEncoder()
        // DEBUG: encoder.outputFormatting = [.prettyPrinted, .sortedKeys] // pretty printing

        encoder.dateEncodingStrategy = .iso8601

        do {
            let data = try encoder.encode(items)
            return String(data: data, encoding: .utf8) ?? "[]"
        } catch {
            return "[]"
        }
    }

    /**
     * A file URL in a form in which two URLs for the same file are equal: an absolute path, with
     * symbolic links resolved.
     */
    static func comparable(_ url: URL) -> URL {
        return url.standardizedFileURL.resolvingSymlinksInPath()
    }

    /**
     * NSDocumentController doesn't provide "last opened" timestamps per recent item.
     * This tries a reasonable approximation using file metadata.
     */
    private static func bestEffortTimestamp(for url: URL) -> String? {
        guard url.isFileURL else { return nil }

        do {
            let values = try url.resourceValues(forKeys: [
                .contentModificationDateKey,
                .addedToDirectoryDateKey,
                .creationDateKey
            ])

            let date = values.addedToDirectoryDate
                    ?? values.contentModificationDate
                    ?? values.creationDate

            guard let date else { return nil }

            // Format like 2026-01-19T12:34:56Z
            let formatter = ISO8601DateFormatter()
            formatter.formatOptions = [.withInternetDateTime, .withFractionalSeconds]
            return formatter.string(from: date)
        } catch {
            return nil
        }
    }
}
