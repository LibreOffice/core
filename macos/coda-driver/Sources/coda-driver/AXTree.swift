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
import CoreGraphics
import Foundation

/**
 * Cross-process accessibility queries against the target app.
 *
 * Created with the target app's PID; uses
 * AXUIElementCreateApplication(targetPid) so the system AX service
 * bridges into XPC remote views such as NSOpenPanel - the limitation
 * that blocks in-process AX self-introspection does not apply here.
 *
 * Requires the driver process to be trusted for accessibility in
 * System Settings -> Privacy & Security -> Accessibility.
 */
final class AXTree {

    private let pid: pid_t
    private let appElement: AXUIElement

    init(pid: pid_t) {
        self.pid = pid
        self.appElement = AXUIElementCreateApplication(pid)
    }

    /// All top-level windows of the target app, most-recent first so a
    /// modal sheet (NSOpenPanel) wins over the main window when both
    /// contain a match.
    private func orderedWindows() -> [AXUIElement] {
        let wins = arrayAttr(appElement, kAXWindowsAttribute as String) ?? []
        return Array(wins.reversed())
    }

    // MARK: - Find

    func findElement(matching predicate: SimpleXPath.Predicate) -> AXUIElement? {
        // @default-button is resolved by asking each window for its
        // kAXDefaultButton attribute, not by tree-walking.
        if predicate.attribute == "default-button" && predicate.value == "true" {
            for window in orderedWindows() {
                if let btn = elementAttr(window, kAXDefaultButtonAttribute as String) {
                    return btn
                }
            }
            return nil
        }
        for window in orderedWindows() {
            if let m = walk(window, predicate: predicate) { return m }
        }
        return nil
    }

    func findAllElements(matching predicate: SimpleXPath.Predicate) -> [AXUIElement] {
        if predicate.attribute == "default-button" && predicate.value == "true" {
            var out: [AXUIElement] = []
            for window in orderedWindows() {
                if let btn = elementAttr(window, kAXDefaultButtonAttribute as String) {
                    out.append(btn)
                }
            }
            return out
        }
        var out: [AXUIElement] = []
        for window in orderedWindows() {
            walkAll(window, predicate: predicate, into: &out)
        }
        return out
    }

    private func walk(_ root: AXUIElement,
                      predicate: SimpleXPath.Predicate) -> AXUIElement? {
        if matches(root, predicate: predicate) { return root }
        for child in arrayAttr(root, kAXChildrenAttribute as String) ?? [] {
            if let found = walk(child, predicate: predicate) { return found }
        }
        return nil
    }

    private func walkAll(_ root: AXUIElement,
                         predicate: SimpleXPath.Predicate,
                         into out: inout [AXUIElement]) {
        if matches(root, predicate: predicate) { out.append(root) }
        for child in arrayAttr(root, kAXChildrenAttribute as String) ?? [] {
            walkAll(child, predicate: predicate, into: &out)
        }
    }

    private func matches(_ elem: AXUIElement,
                         predicate: SimpleXPath.Predicate) -> Bool {
        switch predicate.attribute {
        case "accessibility-id", "identifier":
            return stringAttr(elem, kAXIdentifierAttribute as String) == predicate.value
        case "title":
            return stringAttr(elem, kAXTitleAttribute as String) == predicate.value
        case "role":
            return stringAttr(elem, kAXRoleAttribute as String) == predicate.value
        default:
            return false
        }
    }

    // MARK: - Actions

    func performPress(on elem: AXUIElement) {
        AXUIElementPerformAction(elem, kAXPressAction as CFString)
    }

    func setValue(_ text: String, on elem: AXUIElement) {
        // Special-case NSOpenPanel: drive it like a user via Cmd+Shift+G
        // ("Go to folder") + typing the path + Return.  This works
        // regardless of locale and exercises the panel's normal
        // navigation/selection path.
        if stringAttr(elem, kAXIdentifierAttribute as String) == "open-panel" {
            navigateOpenPanel(to: text)
            return
        }
        AXUIElementSetAttributeValue(elem, kAXValueAttribute as CFString, text as CFString)
    }

    // MARK: - NSOpenPanel keyboard navigation

    private func navigateOpenPanel(to path: String) {
        // Keyboard events go to the frontmost app, so activate the
        // target first.  The panel is the key window once active.
        if let app = NSRunningApplication(processIdentifier: pid) {
            app.activate(options: [.activateIgnoringOtherApps])
        }
        Thread.sleep(forTimeInterval: 0.2)

        // Cmd+Shift+G opens the "Go to folder" sheet on NSOpenPanel.
        // kVK_ANSI_G = 5.
        sendKey(virtualKey: 5, flags: [.maskCommand, .maskShift])
        Thread.sleep(forTimeInterval: 0.3)

        typeUnicode(path)
        Thread.sleep(forTimeInterval: 0.1)

        // Return commits the "Go to folder" sheet, navigating to the
        // parent directory and selecting the file (if path is a file)
        // or just navigating (if it is a directory).  kVK_Return = 36.
        sendKey(virtualKey: 36, flags: [])
        Thread.sleep(forTimeInterval: 0.3)
    }

    private func sendKey(virtualKey: CGKeyCode, flags: CGEventFlags) {
        let src = CGEventSource(stateID: .combinedSessionState)
        if let down = CGEvent(keyboardEventSource: src,
                              virtualKey: virtualKey, keyDown: true) {
            down.flags = flags
            down.post(tap: .cghidEventTap)
        }
        if let up = CGEvent(keyboardEventSource: src,
                            virtualKey: virtualKey, keyDown: false) {
            up.flags = flags
            up.post(tap: .cghidEventTap)
        }
    }

    private func typeUnicode(_ s: String) {
        let src = CGEventSource(stateID: .combinedSessionState)
        for scalar in s.unicodeScalars {
            // Use a 16-bit code unit.  Scalars outside the BMP would
            // need surrogate pairs, but file paths are ASCII / BMP.
            var unit = UniChar(scalar.value & 0xFFFF)
            if let down = CGEvent(keyboardEventSource: src,
                                  virtualKey: 0, keyDown: true) {
                down.keyboardSetUnicodeString(stringLength: 1,
                                              unicodeString: &unit)
                down.post(tap: .cghidEventTap)
            }
            if let up = CGEvent(keyboardEventSource: src,
                                virtualKey: 0, keyDown: false) {
                up.keyboardSetUnicodeString(stringLength: 1,
                                            unicodeString: &unit)
                up.post(tap: .cghidEventTap)
            }
        }
    }

    func isHidden(_ elem: AXUIElement) -> Bool {
        return (attr(elem, kAXHiddenAttribute as String) as? Bool) ?? false
    }

    // MARK: - Tree dump

    /// Return an indented dump of the AX tree (used in 'no such
    /// element' error messages and by GET /session/{id}/source).
    func dumpTree() -> String {
        var out = ""
        for window in orderedWindows() {
            dump(window, depth: 0, into: &out)
        }
        return out
    }

    private func dump(_ elem: AXUIElement, depth: Int, into out: inout String) {
        let indent = String(repeating: "  ", count: depth)
        let role = stringAttr(elem, kAXRoleAttribute as String) ?? ""
        let id = stringAttr(elem, kAXIdentifierAttribute as String) ?? ""
        let title = stringAttr(elem, kAXTitleAttribute as String) ?? ""
        out += "\(indent)role=\"\(role)\" id=\"\(id)\" title=\"\(title)\"\n"
        for child in arrayAttr(elem, kAXChildrenAttribute as String) ?? [] {
            dump(child, depth: depth + 1, into: &out)
        }
    }

    // MARK: - AXUIElement attribute helpers

    private func attr(_ elem: AXUIElement, _ name: String) -> Any? {
        var value: CFTypeRef?
        let err = AXUIElementCopyAttributeValue(elem, name as CFString, &value)
        return err == .success ? value : nil
    }

    private func stringAttr(_ elem: AXUIElement, _ name: String) -> String? {
        return attr(elem, name) as? String
    }

    private func arrayAttr(_ elem: AXUIElement, _ name: String) -> [AXUIElement]? {
        return attr(elem, name) as? [AXUIElement]
    }

    private func elementAttr(_ elem: AXUIElement, _ name: String) -> AXUIElement? {
        guard let raw = attr(elem, name) else { return nil }
        if CFGetTypeID(raw as CFTypeRef) == AXUIElementGetTypeID() {
            return (raw as! AXUIElement)
        }
        return nil
    }
}
