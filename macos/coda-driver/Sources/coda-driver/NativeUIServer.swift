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
import ApplicationServices
import Foundation
import Network

/**
 * W3C WebDriver server that drives the target app via cross-process AX.
 *
 * Tests issue XPath selectors (e.g. accessibility-id="open-panel") and
 * the calls arrive here.  We resolve them against the target's
 * AXUIElement tree, return W3C element references, then perform
 * actions (press, setValue, ...) on the same elements.  Because we
 * query through the system AX service for a different process, XPC
 * remote views (NSOpenPanel's file browser, buttons, etc.) are
 * exposed transparently.
 *
 * Supported endpoints (W3C subset):
 *   GET  /status
 *   POST /session, DELETE /session/{id}
 *   POST /session/{id}/element            -- find by selector
 *   POST /session/{id}/elements           -- find all (returns array)
 *   POST /session/{id}/element/{eid}/click
 *   POST /session/{id}/element/{eid}/value
 *   POST /session/{id}/element/{eid}/clear
 *   GET  /session/{id}/element/{eid}/displayed
 *   GET  /session/{id}/source
 */
final class NativeUIServer: WebDriverHTTPServerBase {

    private let ax: AXTree

    /// Element registry: WebDriver element id -> AXUIElement.
    private var elements: [String: AXUIElement] = [:]
    private let elementsQueue = DispatchQueue(label: "coda-driver.elements")

    init(port: UInt16, targetPid: pid_t) throws {
        self.ax = AXTree(pid: targetPid)
        try super.init(port: port, label: "NativeUIServer")
    }

    override func routeRequest(_ request: HTTPRequest, connection: NWConnection) {
        let segments = request.path.split(separator: "/").map(String.init)

        // GET /status
        if request.method == "GET" && request.path == "/status" {
            sendW3C(connection: connection, value: [
                "ready": true,
                "message": "coda-driver-native",
                "axTrusted": AXIsProcessTrusted(),
            ] as [String: Any])
            return
        }

        // POST /session
        if request.method == "POST" && segments == ["session"] {
            let newId = UUID().uuidString.lowercased()
            sessionIds.insert(newId)
            sendW3C(connection: connection, value: [
                "sessionId": newId,
                "capabilities": [String: Any]()
            ] as [String: Any])
            return
        }

        // DELETE /session/{id}
        if request.method == "DELETE" && segments.count == 2 && segments[0] == "session" {
            sessionIds.remove(segments[1])
            sendW3C(connection: connection, value: NSNull())
            return
        }

        guard segments.count >= 3 && segments[0] == "session" else {
            sendW3CError(connection: connection, error: "unknown command",
                         message: "\(request.method) \(request.path) not implemented")
            return
        }
        guard sessionIds.contains(segments[1]) else {
            sendW3CError(connection: connection, error: "invalid session id",
                         message: "No active session with id '\(segments[1])'")
            return
        }

        let subpath = Array(segments.dropFirst(2))

        if request.method == "POST" && subpath == ["element"] {
            handleFindElement(request, connection: connection)
            return
        }
        if request.method == "POST" && subpath == ["elements"] {
            handleFindElements(request, connection: connection)
            return
        }
        if request.method == "POST" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "click" {
            performOnElement(id: subpath[1], connection: connection) { [ax] elem in
                ax.performPress(on: elem)
                return NSNull()
            }
            return
        }
        if request.method == "POST" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "value" {
            handleSetValue(elementId: subpath[1], request: request, connection: connection)
            return
        }
        if request.method == "POST" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "clear" {
            performOnElement(id: subpath[1], connection: connection) { _ in NSNull() }
            return
        }
        if request.method == "GET" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "displayed" {
            performOnElement(id: subpath[1], connection: connection) { [ax] elem in
                !ax.isHidden(elem)
            }
            return
        }
        if request.method == "GET" && subpath == ["source"] {
            DispatchQueue.main.async { [weak self] in
                self?.sendW3C(connection: connection, value: self?.ax.dumpTree() ?? "")
            }
            return
        }

        sendW3CError(connection: connection, error: "unknown command",
                     message: "\(request.method) \(request.path) not implemented")
    }

    // MARK: - Element handlers

    private func handleFindElement(_ request: HTTPRequest, connection: NWConnection) {
        guard let predicate = parsePredicate(request, connection: connection) else { return }

        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            guard let found = self.ax.findElement(matching: predicate) else {
                let tree = self.ax.dumpTree()
                self.sendW3CError(
                    connection: connection,
                    error: "no such element",
                    message: "No element matching \(predicate.attribute)=\(predicate.value)\n--- accessibility tree ---\n\(tree)"
                )
                return
            }

            let elementId = UUID().uuidString.lowercased()
            self.elementsQueue.sync { self.elements[elementId] = found }
            self.sendW3C(connection: connection, value: [
                "element-6066-11e4-a52e-4f735466cecf": elementId
            ])
        }
    }

    private func handleFindElements(_ request: HTTPRequest, connection: NWConnection) {
        guard let predicate = parsePredicate(request, connection: connection) else { return }

        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            let matches = self.ax.findAllElements(matching: predicate)
            var elements: [[String: String]] = []
            for elem in matches {
                let elementId = UUID().uuidString.lowercased()
                self.elementsQueue.sync { self.elements[elementId] = elem }
                elements.append(["element-6066-11e4-a52e-4f735466cecf": elementId])
            }
            self.sendW3C(connection: connection, value: elements)
        }
    }

    private func parsePredicate(_ request: HTTPRequest,
                                connection: NWConnection) -> SimpleXPath.Predicate? {
        guard let json = try? JSONSerialization.jsonObject(with: request.body) as? [String: Any],
              let using = json["using"] as? String,
              let value = json["value"] as? String else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Missing 'using' or 'value'")
            return nil
        }
        guard using == "xpath" else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Only 'xpath' is supported (got '\(using)')")
            return nil
        }
        guard let predicate = SimpleXPath.parse(value) else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Unsupported XPath: \(value)")
            return nil
        }
        return predicate
    }

    private func performOnElement(id: String,
                                  connection: NWConnection,
                                  action: @escaping (AXUIElement) -> Any) {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            let elem = self.elementsQueue.sync { self.elements[id] }
            guard let elem = elem else {
                self.sendW3CError(connection: connection, error: "stale element reference",
                                  message: "Element '\(id)' no longer exists")
                return
            }
            let result = action(elem)
            if let bool = result as? Bool {
                self.sendW3C(connection: connection, value: bool)
            } else {
                self.sendW3C(connection: connection, value: NSNull())
            }
        }
    }

    private func handleSetValue(elementId: String,
                                request: HTTPRequest,
                                connection: NWConnection) {
        let json = (try? JSONSerialization.jsonObject(with: request.body) as? [String: Any]) ?? [:]
        let text: String
        if let t = json["text"] as? String {
            text = t
        } else if let v = json["value"] as? [String] {
            text = v.joined()
        } else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Missing 'text' or 'value' in body")
            return
        }

        performOnElement(id: elementId, connection: connection) { [ax] elem in
            ax.setValue(text, on: elem)
            return NSNull()
        }
    }
}
