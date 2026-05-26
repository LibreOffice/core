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
import Carbon.HIToolbox
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

        // Put the path on the general pasteboard so a Cmd+V into the
        // Go-to-folder sheet pastes it as a single operation, rather
        // than typing it character by character via CGEvent (which
        // turned out to be unreliable - the keystrokes were dropped
        // or interleaved unexpectedly).
        let pasteboard = NSPasteboard.general
        pasteboard.clearContents()
        pasteboard.setString(path, forType: .string)

        // Cmd+Shift+G opens the "Go to folder" sheet on NSOpenPanel.
        // Look up the physical key that produces 'g' in the current
        // keyboard layout - hardcoding kVK_ANSI_G = 5 sends the wrong
        // shortcut on non-QWERTY layouts (e.g. Dvorak, where the same
        // physical key produces 'i' and so Cmd+Shift+G turns into
        // Cmd+Shift+I = navigate to iCloud Drive sidebar item).
        sendKey(virtualKey: keyCodeFor("g"), flags: [.maskCommand, .maskShift])

        // Wait for the sheet to actually appear before pasting; the
        // sheet animates in and Cmd+V sent too early lands in
        // whatever has focus in the main panel (the file browser
        // AXList), not the path field.
        let pathFieldPredicate = SimpleXPath.Predicate(
            attribute: "accessibility-id", value: "PathTextField")
        var pathField: AXUIElement?
        for _ in 0..<30 {
            if let f = findElement(matching: pathFieldPredicate) {
                pathField = f
                break
            }
            Thread.sleep(forTimeInterval: 0.1)
        }
        guard let field = pathField else {
            NSLog("navigateOpenPanel: Go-to-folder sheet never appeared after Cmd+Shift+G")
            NSLog("navigateOpenPanel: AX tree:\n%@", dumpTree())
            return
        }

        // Cmd+V pastes the path into the sheet's path field.
        // Same layout caveat as Cmd+Shift+G above.
        sendKey(virtualKey: keyCodeFor("v"), flags: [.maskCommand])
        Thread.sleep(forTimeInterval: 0.2)

        // Read the path field's value back to verify the paste landed.
        let pastedValue = stringAttr(field, kAXValueAttribute as String) ?? ""
        let pasteLanded = pastedValue.contains(path)

        // Return commits the "Go to folder" sheet, navigating to the
        // parent directory and selecting the file (if path is a file)
        // or just navigating (if it is a directory).  kVK_Return = 36.
        sendKey(virtualKey: 36, flags: [])
        Thread.sleep(forTimeInterval: 0.3)

        if !pasteLanded {
            NSLog("navigateOpenPanel: paste did not land in PathTextField")
            NSLog("navigateOpenPanel: AX tree at end:\n%@", dumpTree())
        }
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

    /// Return the physical key code that produces the given character
    /// in the user's current keyboard layout.  CGEvent uses physical
    /// key codes (kVK_ANSI_*), not characters; if we hardcoded
    /// kVK_ANSI_G to mean 'g', we would actually send 'i' on a Dvorak
    /// layout (where the QWERTY G position produces 'i'), so the
    /// shortcut would be interpreted by macOS as Cmd+Shift+I instead
    /// of Cmd+Shift+G.
    ///
    /// We query the active Text Input Source via Carbon's TIS and
    /// UCKeyTranslate APIs, iterating physical key codes until we
    /// find the one that yields the requested character.  Cached
    /// across calls within a single layout.
    private static var keyCodeCache: [Character: CGKeyCode] = [:]

    private func keyCodeFor(_ character: Character) -> CGKeyCode {
        if let cached = AXTree.keyCodeCache[character] {
            return cached
        }

        // Fallback: hardcoded US QWERTY positions for the characters
        // we actually use, so the driver still works if the TIS lookup
        // fails for some reason.
        let fallback: CGKeyCode = {
            switch character {
            case "g": return 5
            case "v": return 9
            default: return 0
            }
        }()

        guard let sourceRef = TISCopyCurrentKeyboardLayoutInputSource()?
                .takeRetainedValue(),
              let dataPtr = TISGetInputSourceProperty(
                  sourceRef, kTISPropertyUnicodeKeyLayoutData) else {
            return fallback
        }
        let layoutData = Unmanaged<CFData>
            .fromOpaque(dataPtr).takeUnretainedValue() as Data
        let kbdType = UInt32(LMGetKbdType())
        let target = String(character)

        for kvk: CGKeyCode in 0..<128 {
            var deadKeyState: UInt32 = 0
            var charCount = 0
            var chars = [UniChar](repeating: 0, count: 4)
            let status = layoutData.withUnsafeBytes { bytes -> OSStatus in
                let layoutPtr = bytes
                    .bindMemory(to: UCKeyboardLayout.self).baseAddress!
                return UCKeyTranslate(
                    layoutPtr,
                    UInt16(kvk),
                    UInt16(kUCKeyActionDown),
                    0,
                    kbdType,
                    OptionBits(kUCKeyTranslateNoDeadKeysBit),
                    &deadKeyState,
                    4,
                    &charCount,
                    &chars)
            }
            if status == noErr && charCount > 0 {
                let produced = String(utf16CodeUnits: chars, count: charCount)
                if produced == target {
                    AXTree.keyCodeCache[character] = kvk
                    return kvk
                }
            }
        }

        AXTree.keyCodeCache[character] = fallback
        return fallback
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
