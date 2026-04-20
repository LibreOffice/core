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
import Network

/**
 * Minimal HTTP server for UI testing.
 *
 * Listens on localhost and accepts POST /execute requests with a JSON body
 * containing a "js" field.  The JavaScript is evaluated in a WKWebView via
 * a caller-supplied closure, and the result (or error) is returned as JSON.
 *
 * Start with ``start()`` and stop with ``stop()``.
 */
final class WebDriverServer {

    private let listener: NWListener
    private let jsExecutor: (String, @escaping (Any?, Error?) -> Void) -> Void
    private let focusHandler: (@escaping () -> Void) -> Void

    /**
     * Create a test HTTP server.
     *
     * - Parameters:
     *   - port: TCP port to listen on.
     *   - jsExecutor: Closure that evaluates JavaScript in the WKWebView.
     *                 Called on an arbitrary queue; the closure must dispatch
     *                 to the main thread internally if needed.  The completion
     *                 handler receives the JS result or error.
     *   - focusHandler: Closure that makes the WKWebView the first responder.
     *                   Must dispatch to the main thread.  Calls the completion
     *                   handler when done.
     */
    init(port: UInt16,
         jsExecutor: @escaping (String, @escaping (Any?, Error?) -> Void) -> Void,
         focusHandler: @escaping (@escaping () -> Void) -> Void) throws {
        let params = NWParameters.tcp
        params.acceptLocalOnly = true
        self.listener = try NWListener(using: params, on: NWEndpoint.Port(rawValue: port)!)
        self.jsExecutor = jsExecutor
        self.focusHandler = focusHandler
    }

    func start() {
        listener.newConnectionHandler = { [weak self] connection in
            self?.handleConnection(connection)
        }
        listener.start(queue: .global(qos: .userInitiated))
        NSLog("WebDriverServer: listening on port %d", listener.port?.rawValue ?? 0)
    }

    func stop() {
        listener.cancel()
    }

    // MARK: - Connection handling

    private func handleConnection(_ connection: NWConnection) {
        connection.start(queue: .global(qos: .userInitiated))
        receiveRequest(connection: connection, accumulated: Data())
    }

    private func receiveRequest(connection: NWConnection, accumulated: Data) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 65536) { [weak self] data, _, isComplete, error in
            guard let self = self else { return }

            if let error = error {
                NSLog("WebDriverServer: receive error: %@", error.localizedDescription)
                connection.cancel()
                return
            }

            var buffer = accumulated
            if let data = data {
                buffer.append(data)
            }

            // Check if we have a complete HTTP request (headers + body)
            if let request = self.parseHTTPRequest(buffer) {
                self.handleRequest(request, connection: connection)
            } else if isComplete {
                // Connection closed before we got a full request
                connection.cancel()
            } else {
                // Need more data
                self.receiveRequest(connection: connection, accumulated: buffer)
            }
        }
    }

    // MARK: - HTTP parsing

    private struct HTTPRequest {
        let method: String
        let path: String
        let body: Data
    }

    /**
     * Try to parse a complete HTTP request from the accumulated data.
     * Returns nil if more data is needed.
     */
    private func parseHTTPRequest(_ data: Data) -> HTTPRequest? {
        guard let headerEnd = data.range(of: Data("\r\n\r\n".utf8)) else {
            return nil
        }

        let headerData = data[data.startIndex..<headerEnd.lowerBound]
        guard let headerString = String(data: headerData, encoding: .utf8) else {
            return nil
        }

        let lines = headerString.components(separatedBy: "\r\n")
        guard let requestLine = lines.first else { return nil }
        let parts = requestLine.split(separator: " ", maxSplits: 2)
        guard parts.count >= 2 else { return nil }

        let method = String(parts[0])
        let path = String(parts[1])

        // Find Content-Length
        var contentLength = 0
        for line in lines.dropFirst() {
            if line.lowercased().hasPrefix("content-length:") {
                let value = line.dropFirst("content-length:".count).trimmingCharacters(in: .whitespaces)
                contentLength = Int(value) ?? 0
            }
        }

        let bodyStart = headerEnd.upperBound
        let available = data.count - data.distance(from: data.startIndex, to: bodyStart)
        if available < contentLength {
            return nil // Need more data
        }

        let body = data[bodyStart..<data.index(bodyStart, offsetBy: contentLength)]
        return HTTPRequest(method: method, path: path, body: Data(body))
    }

    // MARK: - Request handling

    private func handleRequest(_ request: HTTPRequest, connection: NWConnection) {
        guard request.method == "POST" else {
            sendResponse(connection: connection, status: "404 Not Found",
                         body: #"{"error":"Not found"}"#)
            return
        }

        if request.path == "/focus" {
            focusHandler { [weak self] in
                self?.sendResponse(connection: connection, status: "200 OK",
                                   body: #"{"result":"ok"}"#)
            }
            return
        }

        guard request.path == "/execute" else {
            sendResponse(connection: connection, status: "404 Not Found",
                         body: #"{"error":"Not found"}"#)
            return
        }

        guard let json = try? JSONSerialization.jsonObject(with: request.body) as? [String: Any],
              let js = json["js"] as? String else {
            sendResponse(connection: connection, status: "400 Bad Request",
                         body: #"{"error":"Missing 'js' field in JSON body"}"#)
            return
        }

        jsExecutor(js) { [weak self] result, error in
            if let error = error {
                let msg = error.localizedDescription.replacingOccurrences(of: "\\", with: "\\\\")
                    .replacingOccurrences(of: "\"", with: "\\\"")
                self?.sendResponse(connection: connection, status: "200 OK",
                                   body: #"{"error":"\#(msg)"}"#)
            } else {
                let resultJSON: String
                if let result = result {
                    if let data = try? JSONSerialization.data(withJSONObject: result),
                       let str = String(data: data, encoding: .utf8) {
                        resultJSON = str
                    } else {
                        resultJSON = "\"\(result)\""
                    }
                } else {
                    resultJSON = "null"
                }
                self?.sendResponse(connection: connection, status: "200 OK",
                                   body: "{\"result\":\(resultJSON)}")
            }
        }
    }

    private func sendResponse(connection: NWConnection, status: String, body: String) {
        let bodyData = Data(body.utf8)
        let header = "HTTP/1.1 \(status)\r\nContent-Type: application/json\r\nContent-Length: \(bodyData.count)\r\nConnection: close\r\n\r\n"
        var response = Data(header.utf8)
        response.append(bodyData)

        connection.send(content: response, completion: .contentProcessed { _ in
            connection.cancel()
        })
    }
}
