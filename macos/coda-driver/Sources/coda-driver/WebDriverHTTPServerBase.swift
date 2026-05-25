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
 * Shared HTTP plumbing for the W3C WebDriver-style servers in the app.
 *
 * Subclasses implement `routeRequest(_:connection:)` to dispatch
 * parsed HTTP requests to W3C endpoints.  The base class handles
 * the NWListener, connection lifecycle, request parsing, and W3C
 * JSON response formatting.
 */
class WebDriverHTTPServerBase {

    /// Parsed HTTP request as the subclass needs it.
    struct HTTPRequest {
        let method: String
        let path: String
        let body: Data
    }

    private let listener: NWListener
    private let label: String

    /// All session IDs created via POST /session.
    var sessionIds = Set<String>()

    init(port: UInt16, label: String) throws {
        let params = NWParameters.tcp
        params.acceptLocalOnly = true
        self.listener = try NWListener(using: params, on: NWEndpoint.Port(rawValue: port)!)
        self.label = label
    }

    func start() {
        listener.stateUpdateHandler = { [weak self] state in
            guard let self = self else { return }
            switch state {
            case .ready:            NSLog("%@: ready on port %d", self.label, self.listener.port?.rawValue ?? 0)
            case .failed(let err):  NSLog("%@: listener failed: %@", self.label, err.localizedDescription)
            case .waiting(let err): NSLog("%@: listener waiting: %@", self.label, err.localizedDescription)
            case .cancelled:        NSLog("%@: listener cancelled", self.label)
            default:
                break
            }
        }
        listener.newConnectionHandler = { [weak self] connection in
            self?.handleConnection(connection)
        }
        listener.start(queue: .global(qos: .userInitiated))
        NSLog("%@: starting on requested port %d", label, listener.port?.rawValue ?? 0)
    }

    func stop() {
        listener.cancel()
    }

    /// Subclass entry point - dispatch to a W3C endpoint handler.
    func routeRequest(_ request: HTTPRequest, connection: NWConnection) {
        sendW3CError(connection: connection, error: "unknown command",
                     message: "\(request.method) \(request.path) not implemented")
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
                NSLog("%@: receive error: %@", self.label, error.localizedDescription)
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

    // MARK: - W3C response helpers

    func sendW3C(connection: NWConnection, value: Any) {
        let wrapper: [String: Any] = ["value": value]
        if let data = try? JSONSerialization.data(withJSONObject: wrapper),
           let body = String(data: data, encoding: .utf8) {
            sendResponse(connection: connection, status: "200 OK", body: body)
        } else {
            sendResponse(connection: connection, status: "200 OK",
                         body: #"{"value":null}"#)
        }
    }

    func sendW3CError(connection: NWConnection, error: String, message: String) {
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
