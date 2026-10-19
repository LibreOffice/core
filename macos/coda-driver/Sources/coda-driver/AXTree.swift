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

    /// What a search of the tree returned.
    struct Found {
        /// The matching elements, in tree order.
        let elements: [AXUIElement]
        /// True when the walk stopped at its time limit, so elements further on in the tree
        /// are missing from the list.
        let timedOut: Bool

        var first: AXUIElement? { elements.first }
    }

    /// How many seconds one walk of the tree may take, for a search, a dump or the page
    /// source alike.  Every attribute read is a round trip into the target process, and on
    /// into the XPC service for a remote view such as NSOpenPanel, so a folder listing of
    /// thousands of rows takes minutes to walk.  A walk that reaches this limit stops and
    /// says so, instead of holding up the test until its own timeout.
    static let walkSeconds: TimeInterval = 5

    /// The time limit of one walk, or of a sequence of them.  Once past, it stays past.
    final class Deadline {
        private let end: CFAbsoluteTime
        private(set) var passed = false

        init(seconds: TimeInterval) {
            end = CFAbsoluteTimeGetCurrent() + seconds
        }

        func isPast() -> Bool {
            if !passed && CFAbsoluteTimeGetCurrent() > end {
                passed = true
            }
            return passed
        }
    }

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

    /// Find the first element matching the predicate.  The walk takes at most walkSeconds,
    /// or stops at the given deadline when the caller has one for a longer sequence of calls.
    func findElement(matching predicate: SimpleXPath.Predicate, roots: [AXUIElement]? = nil,
                     deadline: Deadline? = nil, maxDepth: Int? = nil) -> Found {
        // @default-button is resolved by asking each window for its
        // kAXDefaultButton attribute, not by tree-walking.
        if predicate.attribute == "default-button" && predicate.value == "true" {
            for window in roots ?? orderedWindows() {
                if let btn = elementAttr(window, kAXDefaultButtonAttribute as String) {
                    return Found(elements: [btn], timedOut: false)
                }
            }
            return Found(elements: [], timedOut: false)
        }
        return search(firstOnly: true, roots: roots, deadline: deadline, maxDepth: maxDepth) {
            self.matches($0, predicate: predicate)
        }
    }

    func findAllElements(matching predicate: SimpleXPath.Predicate) -> Found {
        if predicate.attribute == "default-button" && predicate.value == "true" {
            var out: [AXUIElement] = []
            for window in orderedWindows() {
                if let btn = elementAttr(window, kAXDefaultButtonAttribute as String) {
                    out.append(btn)
                }
            }
            return Found(elements: out, timedOut: false)
        }
        return search(firstOnly: false) { self.matches($0, predicate: predicate) }
    }

    /// Walk the roots, by default every window most recent first, collecting the elements
    /// the closure accepts.  With firstOnly the walk ends at the first match, and with
    /// maxDepth it stays within that many levels of each root.
    private func search(firstOnly: Bool, roots: [AXUIElement]? = nil, deadline: Deadline? = nil,
                        maxDepth: Int? = nil, where accepts: (AXUIElement) -> Bool) -> Found {
        let deadline = deadline ?? Deadline(seconds: AXTree.walkSeconds)
        var out: [AXUIElement] = []
        for root in roots ?? orderedWindows() {
            walk(root, deadline: deadline, firstOnly: firstOnly, maxDepth: maxDepth, into: &out,
                 where: accepts)
            if (firstOnly && !out.isEmpty) || deadline.isPast() {
                break
            }
        }
        return Found(elements: out, timedOut: deadline.passed)
    }

    /// Walk one root level by level.  The controls of a window, its buttons, fields and
    /// sheets, sit a few levels down, while the rows of a file listing sit far below them,
    /// so a walk that runs out of time has visited the controls and lost only rows.
    private func walk(_ root: AXUIElement, deadline: Deadline, firstOnly: Bool, maxDepth: Int?,
                      into out: inout [AXUIElement], where accepts: (AXUIElement) -> Bool) {
        var queue = [(elem: root, depth: 0)]
        var next = 0
        while next < queue.count {
            if deadline.isPast() {
                return
            }
            let (elem, depth) = queue[next]
            next += 1
            if accepts(elem) {
                out.append(elem)
                if firstOnly {
                    return
                }
            }
            if let maxDepth = maxDepth, depth >= maxDepth {
                continue
            }
            for child in childElementsToWalk(of: elem) {
                queue.append((elem: child, depth: depth + 1))
            }
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
            if let f = findElement(matching: pathFieldPredicate).first {
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

    /// One line naming an element: its role, identifier and title.
    private func describe(_ elem: AXUIElement) -> String {
        let role = stringAttr(elem, kAXRoleAttribute as String) ?? ""
        let id = stringAttr(elem, kAXIdentifierAttribute as String) ?? ""
        let title = stringAttr(elem, kAXTitleAttribute as String) ?? ""
        return "role=\"\(role)\" id=\"\(id)\" title=\"\(title)\""
    }

    /// Return an indented plain-text dump of the AX tree, one line per
    /// element with its role, identifier and title.
    func dumpTree() -> String {
        let deadline = Deadline(seconds: AXTree.walkSeconds)
        var lines: [String] = []
        for window in orderedWindows() {
            dump(window, depth: 0, deadline: deadline, into: &lines)
        }
        var out = lines.joined(separator: "\n") + "\n"
        if deadline.passed || lines.count >= AXTree.dumpLines {
            out += "(the dump stopped after \(lines.count) lines"
                + " or \(Int(AXTree.walkSeconds)) seconds; the tree goes on)\n"
        }
        return out
    }

    /// How many lines a dump of the tree may have.  A file listing of thousands of rows
    /// says nothing about why a control was not found, and would push the useful lines
    /// out of view.
    private static let dumpLines = 400

    private func dump(_ elem: AXUIElement, depth: Int, deadline: Deadline,
                      into lines: inout [String]) {
        if deadline.isPast() || lines.count >= AXTree.dumpLines {
            return
        }
        lines.append(String(repeating: "  ", count: depth) + describe(elem))
        for child in childElementsToWalk(of: elem) {
            dump(child, depth: depth + 1, deadline: deadline, into: &lines)
        }
    }

    // MARK: - Page source (XML)

    /// Serialize the AX tree as XML in the shape the AT-SPI WebDriver
    /// uses for its page source, so one tree format works across
    /// platforms.  Each element's tag is the AT-SPI role name with
    /// spaces replaced by underscores (push_button, label, frame, ...),
    /// carrying name, role, description, accessibility-id and path
    /// attributes.  The role attribute keeps the raw macOS AX role
    /// string (AXButton, ...).  The path attribute is the list of
    /// child indexes from the root, joined with spaces.  A synthetic
    /// desktop_frame root element wraps the target application's node,
    /// mirroring the AT-SPI desktop that contains the running
    /// applications.  A web view's content is left out, the way every
    /// walk of the tree leaves it out; the page is driven through the
    /// WebDriver server inside the app.
    func pageSourceXML() -> String {
        let deadline = Deadline(seconds: AXTree.walkSeconds)
        var out = "<desktop_frame name=\"main\" role=\"\" description=\"\" path=\"\">\n"
        serialize(appElement, path: [0], depth: 1, deadline: deadline, into: &out)
        if deadline.passed {
            out += "  <!-- the walk stopped after \(Int(AXTree.walkSeconds)) seconds;"
                + " elements are missing -->\n"
        }
        out += "</desktop_frame>\n"
        return out
    }

    private func serialize(_ elem: AXUIElement, path: [Int], depth: Int, deadline: Deadline,
                           into out: inout String) {
        let axRole = stringAttr(elem, kAXRoleAttribute as String) ?? ""
        let subrole = stringAttr(elem, kAXSubroleAttribute as String)
        let tag = AXTree.tagName(forAXRole: axRole, subrole: subrole)
        let name = stringAttr(elem, kAXTitleAttribute as String) ?? ""
        let description = stringAttr(elem, kAXDescriptionAttribute as String) ?? ""
        let id = stringAttr(elem, kAXIdentifierAttribute as String) ?? ""

        let indent = String(repeating: "  ", count: depth)
        out += "\(indent)<\(tag)"
        out += " name=\"\(xmlEscape(name))\""
        out += " role=\"\(xmlEscape(axRole))\""
        out += " description=\"\(xmlEscape(description))\""
        if !id.isEmpty {
            out += " accessibility-id=\"\(xmlEscape(id))\""
        }
        out += " path=\"\(path.map(String.init).joined(separator: " "))\""

        let children = deadline.isPast() ? [] : childElementsToWalk(of: elem)
        if children.isEmpty {
            out += "/>\n"
            return
        }
        out += ">\n"
        for (index, child) in children.enumerated() {
            if deadline.isPast() {
                break
            }
            serialize(child, path: path + [index], depth: depth + 1, deadline: deadline, into: &out)
        }
        out += "\(indent)</\(tag)>\n"
    }

    /// macOS AX roles that translate to a differently-named AT-SPI
    /// role.  Roles not listed here fall back to the generic
    /// camel-case-to-snake-case conversion in tagName.
    private static let axRoleToTag: [String: String] = [
        "AXApplication": "application",
        "AXButton": "push_button",
        "AXCell": "table_cell",
        "AXCheckBox": "check_box",
        "AXComboBox": "combo_box",
        "AXGroup": "panel",
        "AXOutline": "tree_table",
        "AXPopUpButton": "combo_box",
        "AXProgressIndicator": "progress_bar",
        "AXScrollArea": "scroll_pane",
        "AXSecureTextField": "password_text",
        "AXSheet": "dialog",
        "AXSplitGroup": "split_pane",
        "AXStaticText": "label",
        "AXTabGroup": "page_tab_list",
        "AXTextArea": "text",
        "AXTextField": "text",
        "AXToolbar": "tool_bar",
        "AXWebArea": "document_web",
    ]

    private static func tagName(forAXRole axRole: String, subrole: String?) -> String {
        // A window's tag depends on its subrole: plain windows map to
        // the AT-SPI frame role, dialog-like windows to dialog.
        if axRole == "AXWindow" {
            switch subrole {
            case "AXDialog", "AXSystemDialog", "AXSheet":
                return "dialog"
            default:
                return "frame"
            }
        }
        if let mapped = axRoleToTag[axRole] {
            return mapped
        }
        // Generic fallback: drop the AX prefix and convert the
        // remaining camel case to snake case, so for example
        // AXMenuItem becomes menu_item.
        guard axRole.hasPrefix("AX") && axRole.count > 2 else {
            return "accessible"
        }
        var tag = ""
        for character in axRole.dropFirst(2) {
            if character.isUppercase {
                if !tag.isEmpty {
                    tag += "_"
                }
                tag += character.lowercased()
            } else {
                tag.append(character)
            }
        }
        return tag.isEmpty ? "accessible" : tag
    }

    private func xmlEscape(_ text: String) -> String {
        var out = ""
        for character in text {
            switch character {
            case "&": out += "&amp;"
            case "<": out += "&lt;"
            case ">": out += "&gt;"
            case "\"": out += "&quot;"
            default: out.append(character)
            }
        }
        return out
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

    /// Child elements for tree building: the normal children plus any attached
    /// sheets. A document-modal alert (for example the "file was changed by
    /// another application" save error) is a sheet on its window and is not
    /// always listed among the plain children, so include sheets explicitly,
    /// skipping any a window already lists as a child.
    private func childElements(of elem: AXUIElement) -> [AXUIElement] {
        var elements = arrayAttr(elem, kAXChildrenAttribute as String) ?? []
        for sheet in arrayAttr(elem, "AXSheets") ?? [] {
            if !elements.contains(where: { CFEqual($0, sheet) }) {
                elements.append(sheet)
            }
        }
        return elements
    }

    /// The children a walk descends into: the element's children and attached sheets, except
    /// below a web area.  The content of a web view is a tree of thousands of nodes, each
    /// read a round trip into the web content process, and the page is driven through the
    /// WebDriver server inside the app, so the native tree ends at the web area element.
    private func childElementsToWalk(of elem: AXUIElement) -> [AXUIElement] {
        if stringAttr(elem, kAXRoleAttribute as String) == "AXWebArea" {
            return []
        }
        return childElements(of: elem)
    }

    private func elementAttr(_ elem: AXUIElement, _ name: String) -> AXUIElement? {
        guard let raw = attr(elem, name) else { return nil }
        if CFGetTypeID(raw as CFTypeRef) == AXUIElementGetTypeID() {
            return (raw as! AXUIElement)
        }
        return nil
    }
}
