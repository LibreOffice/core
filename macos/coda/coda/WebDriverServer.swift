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
 * W3C WebDriver protocol server for UI testing.
 *
 * Listens on localhost and implements a subset of the W3C WebDriver
 * protocol sufficient for WebDriverIO to execute JavaScript, manage
 * sessions, and track window handles.  This allows the same test
 * specs to run on macOS (this server), Linux (WebEngineDriver), and
 * Windows (EdgeDriver).
 *
 * Additionally supports POST /focus as a custom extension to make
 * the WKWebView the macOS first responder for XCUITest typing.
 */
final class WebDriverServer {

    private let listener: NWListener
    private let jsExecutor: (String, @escaping (Any?, Error?) -> Void) -> Void
    private let focusHandler: (@escaping () -> Void) -> Void

    /// The session ID, created on first POST /session.
    private var sessionId: String?

    /**
     * Create a WebDriver server.
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

            if let request = self.parseHTTPRequest(buffer) {
                self.routeRequest(request, connection: connection)
            } else if isComplete {
                connection.cancel()
            } else {
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
            return nil
        }

        let body = data[bodyStart..<data.index(bodyStart, offsetBy: contentLength)]
        return HTTPRequest(method: method, path: path, body: Data(body))
    }

    // MARK: - W3C WebDriver routing

    private func routeRequest(_ request: HTTPRequest, connection: NWConnection) {
        let segments = request.path.split(separator: "/").map(String.init)

        // GET /status
        if request.method == "GET" && request.path == "/status" {
            sendW3C(connection: connection, value: ["ready": true, "message": "coda-macos"])
            return
        }

        // POST /session
        if request.method == "POST" && segments == ["session"] {
            if sessionId == nil {
                sessionId = UUID().uuidString.lowercased()
            }
            sendW3C(connection: connection, value: [
                "sessionId": sessionId!,
                "capabilities": [String: Any]()
            ] as [String: Any])
            return
        }

        // DELETE /session/{id}
        if request.method == "DELETE" && segments.count == 2 && segments[0] == "session" {
            sendW3C(connection: connection, value: NSNull())
            return
        }

        // Routes that require a valid session: /session/{id}/...
        if segments.count >= 3 && segments[0] == "session" {
            guard segments[1] == sessionId else {
                sendW3CError(connection: connection, error: "invalid session id",
                             message: "No active session with id '\(segments[1])'")
                return
            }

            let subpath = Array(segments.dropFirst(2))

            // POST /session/{id}/execute/sync
            if request.method == "POST" && subpath == ["execute", "sync"] {
                handleExecuteSync(request, connection: connection)
                return
            }

            // GET /session/{id}/window/handles
            if request.method == "GET" && subpath == ["window", "handles"] {
                sendW3C(connection: connection, value: ["main"])
                return
            }

            // POST /session/{id}/window
            if request.method == "POST" && subpath == ["window"] {
                sendW3C(connection: connection, value: NSNull())
                return
            }
        }

        // POST /focus (custom extension for XCUITest)
        if request.method == "POST" && request.path == "/focus" {
            focusHandler { [weak self] in
                self?.sendW3C(connection: connection, value: NSNull())
            }
            return
        }

        sendW3CError(connection: connection, error: "unknown command",
                     message: "\(request.method) \(request.path) not implemented")
    }

    // MARK: - Execute sync

    private func handleExecuteSync(_ request: HTTPRequest, connection: NWConnection) {
        guard let json = try? JSONSerialization.jsonObject(with: request.body) as? [String: Any],
              let script = json["script"] as? String else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Missing 'script' field in request body")
            return
        }

        // WKWebView.evaluateJavaScript expects an expression, not a
        // program with "return".  WebDriverIO sends scripts like
        // "return (function(){...}).apply(null,arguments)" so we need
        // to strip the leading "return " and let evaluateJavaScript
        // evaluate the expression directly.
        let expression: String
        if script.hasPrefix("return ") {
            expression = String(script.dropFirst("return ".count))
        } else {
            expression = script
        }

        jsExecutor(expression) { [weak self] result, error in
            if let error = error {
                self?.sendW3CError(connection: connection, error: "javascript error",
                                   message: error.localizedDescription)
            } else {
                self?.sendW3C(connection: connection, value: result ?? NSNull())
            }
        }
    }

    // MARK: - W3C response helpers

    private func sendW3C(connection: NWConnection, value: Any) {
        let wrapper: [String: Any] = ["value": value]
        if let data = try? JSONSerialization.data(withJSONObject: wrapper),
           let body = String(data: data, encoding: .utf8) {
            sendResponse(connection: connection, status: "200 OK", body: body)
        } else {
            sendResponse(connection: connection, status: "200 OK",
                         body: #"{"value":null}"#)
        }
    }

    private func sendW3CError(connection: NWConnection, error: String, message: String) {
        let wrapper: [String: Any] = [
            "value": [
                "error": error,
                "message": message,
                "stacktrace": ""
            ]
        ]
        let status = error == "invalid session id" ? "404 Not Found" : "500 Internal Server Error"
        if let data = try? JSONSerialization.data(withJSONObject: wrapper),
           let body = String(data: data, encoding: .utf8) {
            sendResponse(connection: connection, status: status, body: body)
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
