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
import Foundation
import Network

/**
 * W3C WebDriver server that drives the macOS native UI via NSAccessibility.
 *
 * This is the macOS counterpart of the Qt AT-SPI driver: the same
 * protocol (WebDriver), against a different element tree.  Tests use
 * `browser.native.$('//*[@accessibility-id="..."]')` and the calls
 * arrive here, where we walk NSApp.windows via NSAccessibility and
 * return a matching element.
 *
 * Supported endpoints (W3C subset):
 *   GET /status
 *   POST /session, DELETE /session/{id}
 *   POST /session/{id}/element            -- find by selector
 *   POST /session/{id}/element/{eid}/click
 *   POST /session/{id}/element/{eid}/value -- set value
 *   POST /session/{id}/element/{eid}/displayed
 *
 * Selectors: a tiny XPath subset of the form
 *   //*[@accessibility-id="ID"]
 *   //*[@title="Title"]
 *   //*[@role="AXButton"]
 *
 * [Now "close" XPaths misinterpreted as comments: */*/*/*/]
 */
final class NativeUIServer: WebDriverHTTPServerBase {

    private struct Weak<T: AnyObject> {
        weak var value: T?
    }

    /// Element registry: WebDriver element id -> NSAccessibility object.
    private var elements: [String: Weak<NSObject>] = [:]

    init(port: UInt16) throws {
        try super.init(port: port, label: "NativeUIServer")
    }

    override func routeRequest(_ request: HTTPRequest, connection: NWConnection) {
        let segments = request.path.split(separator: "/").map(String.init)

        // GET /status
        if request.method == "GET" && request.path == "/status" {
            sendW3C(connection: connection, value: ["ready": true, "message": "coda-macos-native"])
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

        // Routes that require a valid session: /session/{id}/...
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

        // POST /session/{id}/element
        if request.method == "POST" && subpath == ["element"] {
            handleFindElement(request, connection: connection)
            return
        }

        // POST /session/{id}/element/{eid}/click
        if request.method == "POST" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "click" {
            performOnElement(id: subpath[1], connection: connection) { obj in
                NSAccessibilityServer.performPress(on: obj)
            }
            return
        }

        // POST /session/{id}/element/{eid}/value
        if request.method == "POST" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "value" {
            handleSetValue(elementId: subpath[1], request: request, connection: connection)
            return
        }

        // GET /session/{id}/element/{eid}/displayed
        if request.method == "GET" && subpath.count == 3
            && subpath[0] == "element" && subpath[2] == "displayed" {
            performOnElement(id: subpath[1], connection: connection) { obj in
                !NSAccessibilityServer.isHidden(obj)
            }
            return
        }

        sendW3CError(connection: connection, error: "unknown command",
                     message: "\(request.method) \(request.path) not implemented")
    }

    // MARK: - Element finding

    private func handleFindElement(_ request: HTTPRequest, connection: NWConnection) {
        guard let json = try? JSONSerialization.jsonObject(with: request.body) as? [String: Any],
              let using = json["using"] as? String,
              let value = json["value"] as? String else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Missing 'using' or 'value'")
            return
        }
        guard using == "xpath" else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Only 'xpath' is supported (got '\(using)')")
            return
        }

        guard let predicate = SimpleXPath.parse(value) else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Unsupported XPath: \(value)")
            return
        }

        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            guard let found = NSAccessibilityServer.findElement(matching: predicate) else {
                self.sendW3CError(connection: connection, error: "no such element",
                                  message: "No element matching \(value)")
                return
            }

            let elementId = UUID().uuidString.lowercased()
            self.elements[elementId] = Weak(value: found)
            // W3C element shape:
            //   {"element-6066-11e4-a52e-4f735466cecf": "<id>"}
            self.sendW3C(connection: connection, value: [
                "element-6066-11e4-a52e-4f735466cecf": elementId
            ])
        }
    }

    private func performOnElement(id: String, connection: NWConnection,
                                  action: @escaping (NSObject) -> Any) {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            guard let weakRef = self.elements[id], let obj = weakRef.value else {
                self.sendW3CError(connection: connection, error: "stale element reference",
                                  message: "Element '\(id)' no longer exists")
                return
            }
            let result = action(obj)
            if let bool = result as? Bool {
                self.sendW3C(connection: connection, value: bool)
            } else {
                self.sendW3C(connection: connection, value: NSNull())
            }
        }
    }

    private func handleSetValue(elementId: String, request: HTTPRequest,
                                connection: NWConnection) {
        let json = (try? JSONSerialization.jsonObject(with: request.body) as? [String: Any]) ?? [:]
        // W3C body: {"text": "..."}, JSON Wire: {"value": ["a","b","c"]}
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

        performOnElement(id: elementId, connection: connection) { obj in
            NSAccessibilityServer.setValue(text, on: obj)
            return NSNull()
        }
    }
}

/**
 * Internal NSAccessibility helpers used by NativeUIServer.
 *
 * The macOS accessibility API exposes element attributes via Cocoa
 * protocol methods (accessibilityChildren, accessibilityIdentifier,
 * accessibilityTitle, ...).  This wrapper walks NSApp.windows and
 * matches against a SimpleXPath predicate.
 */
private enum NSAccessibilityServer {

    static func findElement(matching predicate: SimpleXPath.Predicate) -> NSObject? {
        for window in NSApp.windows {
            if let match = walk(window, predicate: predicate) {
                return match
            }
        }
        return nil
    }

    private static func walk(_ root: NSObject, predicate: SimpleXPath.Predicate) -> NSObject? {
        if matches(root, predicate: predicate) {
            return root
        }
        let ax = root as? NSAccessibilityProtocol
        let children = (ax?.accessibilityChildren() as? [NSObject]) ?? []
        for child in children {
            if let found = walk(child, predicate: predicate) {
                return found
            }
        }
        return nil
    }

    private static func matches(_ obj: NSObject, predicate: SimpleXPath.Predicate) -> Bool {
        guard let ax = obj as? NSAccessibilityProtocol else { return false }
        switch predicate.attribute {
        case "accessibility-id", "identifier":
            return ax.accessibilityIdentifier() == predicate.value
        case "title":
            return (ax.accessibilityTitle() ?? "") == predicate.value
        case "role":
            return (ax.accessibilityRole()?.rawValue ?? "") == predicate.value
        default:
            return false
        }
    }

    static func performPress(on obj: NSObject) -> Any {
        if let button = obj as? NSButton {
            button.performClick(nil)
        } else if let ax = obj as? NSAccessibilityProtocol {
            _ = ax.accessibilityPerformPress()
        }
        return NSNull()
    }

    static func setValue(_ text: String, on obj: NSObject) {
        if let textField = obj as? NSTextField {
            textField.stringValue = text
            return
        }
        if let ax = obj as? NSAccessibilityProtocol {
            ax.setAccessibilityValue(text)
        }
    }

    static func isHidden(_ obj: NSObject) -> Bool {
        if let view = obj as? NSView {
            return view.isHidden || view.window == nil
        }
        return false
    }
}

/**
 * Minimal XPath parser for the subset our tests use:
 *
 *   //*[@accessibility-id="..."]
 *   //*[@title="..."]
 *   //*[@role="..."]
 *
 * [Now "close" XPaths misinterpreted as comments: */*/*/]
 */
private enum SimpleXPath {
    struct Predicate {
        let attribute: String
        let value: String
    }

    static func parse(_ xpath: String) -> Predicate? {
        // Match: //*[@<attr>="<value>"]
        let pattern = #"^//\*\[@([\w-]+)\s*=\s*"([^"]*)"\]$"#
        guard let regex = try? NSRegularExpression(pattern: pattern),
              let m = regex.firstMatch(in: xpath, range: NSRange(xpath.startIndex..., in: xpath)),
              m.numberOfRanges == 3,
              let attrR = Range(m.range(at: 1), in: xpath),
              let valR = Range(m.range(at: 2), in: xpath) else {
            return nil
        }
        return Predicate(attribute: String(xpath[attrR]), value: String(xpath[valR]))
    }
}
