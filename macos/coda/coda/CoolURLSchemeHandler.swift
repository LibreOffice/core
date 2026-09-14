/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Foundation
import UniformTypeIdentifiers
import WebKit

/**
 * Serves the cool: URLs that the in-process server gives the page for media embedded in the
 * document, such as a video placed on a slide. The app runs no HTTP listener, so WebKit hands
 * every request for this scheme to this handler, and the handler replies with the media file
 * straight from disk.
 *
 * The one path served is /cool/media. Its Tag query parameter names a media file that the
 * server extracted from the document, and the document's server side resolves the tag to a
 * path on disk. Any other path is answered with 404.
 *
 * Range requests are honoured, because WebKit fetches media in byte ranges and expects a
 * partial response for each of them.
 *
 * This is the port of ios/Mobile/CoolURLSchemeHandler.mm, without its socket transport.
 */
final class CoolURLSchemeHandler: NSObject, WKURLSchemeHandler {

    /// The controller whose document owns the media. The document is assigned to the
    /// controller after the web view exists, so it is looked up at request time.
    private weak var viewController: ViewController?

    /// How many bytes go to WebKit in one piece.
    private static let chunkSize = 1 << 20

    init(viewController: ViewController) {
        self.viewController = viewController
        super.init()
    }

    func webView(_ webView: WKWebView, start urlSchemeTask: WKURLSchemeTask) {
        guard let url = urlSchemeTask.request.url else {
            urlSchemeTask.didFailWithError(URLError(.badURL))
            return
        }

        guard url.path == "/cool/media",
              let tag = CoolURLSchemeHandler.tag(in: url),
              let document = viewController?.document,
              let mediaPath = COWrapper.embeddedMediaPath(document: document, tag: tag) else {
            COWrapper.LOG_ERR("No embedded media for \(url.absoluteString)")
            respond(to: urlSchemeTask, url: url, status: 404, headers: [:])
            return
        }

        do {
            try serve(mediaPath, to: urlSchemeTask, url: url)
        } catch {
            COWrapper.LOG_ERR("Failed to serve embedded media \(mediaPath): \(error.localizedDescription)")
            urlSchemeTask.didFailWithError(error)
        }
    }

    func webView(_ webView: WKWebView, stop urlSchemeTask: WKURLSchemeTask) {
        // Each request is answered completely inside start, on the main thread, so the task is
        // already finished by the time WebKit can ask for a stop.
    }

    /// The value of the Tag query parameter. The server percent-encodes the "&" between the
    /// parameters, so the query is decoded once before it is split.
    private static func tag(in url: URL) -> String? {
        guard let query = URLComponents(url: url, resolvingAgainstBaseURL: false)?
                .percentEncodedQuery?.removingPercentEncoding else {
            return nil
        }
        for parameter in query.split(separator: "&") where parameter.hasPrefix("Tag=") {
            return String(parameter.dropFirst("Tag=".count))
        }
        return nil
    }

    /// The byte range that a Range header asks for, out of a file of the given size:
    /// "bytes=first-last", "bytes=first-" or "bytes=-count", one range only. A last position
    /// past the end of the file is clamped to the end. Nil when the header cannot be parsed or
    /// when no byte of the range lies inside the file.
    static func byteRange(of header: String, size: Int) -> Range<Int>? {
        let prefix = "bytes="
        guard header.hasPrefix(prefix), !header.contains(",") else {
            return nil
        }

        let bounds = header.dropFirst(prefix.count)
            .split(separator: "-", maxSplits: 1, omittingEmptySubsequences: false)
        guard bounds.count == 2 else {
            return nil
        }

        // Either side may be empty; a side that is present has to be a number.
        let firstBound = Int(bounds[0])
        let lastBound = Int(bounds[1])
        guard firstBound != nil || bounds[0].isEmpty, lastBound != nil || bounds[1].isEmpty else {
            return nil
        }

        switch (firstBound, lastBound) {
        case (nil, nil):
            return nil
        case (nil, let count?):
            // The last count bytes of the file.
            guard count > 0, size > 0 else {
                return nil
            }
            return max(size - count, 0)..<size
        case (let first?, nil):
            guard first < size else {
                return nil
            }
            return first..<size
        case (let first?, let last?):
            guard first <= last, first < size else {
                return nil
            }
            return first..<min(last + 1, size)
        }
    }

    /// Sends the media file, or the part of it that the request's Range header asks for.
    private func serve(_ mediaPath: String, to task: WKURLSchemeTask, url: URL) throws {
        let fileURL = URL(fileURLWithPath: mediaPath)
        let file = try FileHandle(forReadingFrom: fileURL)
        defer { try? file.close() }
        let size = try Int(file.seekToEnd())

        var headers = [
            // The slideshow loads its video with crossOrigin set, so WebKit checks this header
            // against the page's origin. The page is a file: URL and the app allows file access
            // from file URLs, so WebKit writes that origin as "file://" rather than "null". The
            // handler serves only this document's own media to this app's own pages, so any
            // origin may read it.
            "Access-Control-Allow-Origin": "*",
            "Accept-Ranges": "bytes",
        ]
        if let mimeType = UTType(filenameExtension: fileURL.pathExtension)?.preferredMIMEType {
            headers["Content-Type"] = mimeType
        }

        var status = 200
        var range = 0..<size
        if let rangeHeader = task.request.value(forHTTPHeaderField: "Range") {
            guard let requested = CoolURLSchemeHandler.byteRange(of: rangeHeader, size: size) else {
                headers["Content-Range"] = "bytes */\(size)"
                respond(to: task, url: url, status: 416, headers: headers)
                return
            }
            status = 206
            range = requested
            headers["Content-Range"] = "bytes \(range.lowerBound)-\(range.upperBound - 1)/\(size)"
        }
        headers["Content-Length"] = String(range.count)

        guard let response = HTTPURLResponse(url: url, statusCode: status, httpVersion: "HTTP/1.1",
                                             headerFields: headers) else {
            throw URLError(.badServerResponse)
        }
        task.didReceive(response)

        try file.seek(toOffset: UInt64(range.lowerBound))
        var remaining = range.count
        while remaining > 0 {
            let wanted = min(remaining, CoolURLSchemeHandler.chunkSize)
            guard let data = try file.read(upToCount: wanted), !data.isEmpty else {
                break
            }
            task.didReceive(data)
            remaining -= data.count
        }
        task.didFinish()
    }

    /// Answers with a status and no body.
    private func respond(to task: WKURLSchemeTask, url: URL, status: Int, headers: [String: String]) {
        var headers = headers
        headers["Access-Control-Allow-Origin"] = "*"
        headers["Content-Length"] = "0"
        guard let response = HTTPURLResponse(url: url, statusCode: status, httpVersion: "HTTP/1.1",
                                             headerFields: headers) else {
            return
        }
        task.didReceive(response)
        task.didFinish()
    }
}
