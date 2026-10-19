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

    /// Press the element.  Returns nil when the press went through, otherwise why it did not.
    func performPress(on elem: AXUIElement) -> String? {
        if (attr(elem, kAXEnabledAttribute as String) as? Bool) == false {
            return "\(describe(elem)) is disabled"
        }
        let err = AXUIElementPerformAction(elem, kAXPressAction as CFString)
        if err != .success {
            return "pressing \(describe(elem)) failed with AXError \(err.rawValue)"
        }
        return nil
    }

    /// Set the element's value.  NSOpenPanel is a special case: the text is a path, and the
    /// panel is navigated to it and the file selected the way a user does it, through the
    /// Go-to-folder sheet.  Returns nil when done, otherwise why it did not work.
    func setValue(_ text: String, on elem: AXUIElement) -> String? {
        if stringAttr(elem, kAXIdentifierAttribute as String) == "open-panel" {
            return navigateOpenPanel(elem, to: text)
        }
        let err = AXUIElementSetAttributeValue(elem, kAXValueAttribute as CFString,
                                               text as CFString)
        if err != .success {
            return "setting the value of \(describe(elem)) failed with AXError \(err.rawValue)"
        }
        return nil
    }

    // MARK: - NSOpenPanel keyboard navigation

    /// Navigate the open panel to the file at path and select it: Cmd+Shift+G opens the
    /// Go-to-folder sheet, the path is pasted into it, Return commits it.  Each step is
    /// checked against the tree before the next one, and the result is that the panel lists
    /// the file's folder with the file selected, ready for the Open button.  The whole
    /// sequence stays well under the 30 seconds the test runner gives one request; a request
    /// that takes longer is sent again, and the sequence would start over on the same panel.
    private func navigateOpenPanel(_ panel: AXUIElement, to path: String) -> String? {
        let started = CFAbsoluteTimeGetCurrent()

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

        // Cmd+Shift+G opens the "Go to folder" sheet on NSOpenPanel.  A sheet
        // that an earlier attempt left up is used as it is.  Look up the
        // physical key that produces 'g' in the current keyboard layout -
        // hardcoding kVK_ANSI_G = 5 sends the wrong shortcut on non-QWERTY
        // layouts (e.g. Dvorak, where the same physical key produces 'i' and
        // so Cmd+Shift+G turns into Cmd+Shift+I = navigate to iCloud Drive
        // sidebar item).
        let pathFieldPredicate = SimpleXPath.Predicate(
            attribute: "accessibility-id", value: "PathTextField")
        var pathField = findElement(matching: pathFieldPredicate,
                                    maxDepth: AXTree.controlDepth).first
        if pathField == nil {
            sendKey(virtualKey: keyCodeFor("g"), flags: [.maskCommand, .maskShift])

            // Wait for the sheet to actually appear before pasting; the
            // sheet animates in and Cmd+V sent too early lands in
            // whatever has focus in the main panel (the file browser
            // AXList), not the path field.
            let sheetDeadline = Deadline(seconds: 3)
            repeat {
                pathField = findElement(matching: pathFieldPredicate, deadline: sheetDeadline,
                                        maxDepth: AXTree.controlDepth).first
                if pathField == nil {
                    Thread.sleep(forTimeInterval: 0.1)
                }
            } while pathField == nil && !sheetDeadline.isPast()
        }
        guard let field = pathField else {
            return openPanelFailure("the Go-to-folder sheet did not appear after Cmd+Shift+G")
        }

        // A field that still holds the text of an earlier attempt is emptied through the
        // accessibility API first, so the paste replaces it.
        if let existing = stringAttr(field, kAXValueAttribute as String), !existing.isEmpty {
            AXUIElementSetAttributeValue(field, kAXValueAttribute as CFString, "" as CFString)
        }

        // Cmd+V pastes the path into the sheet's path field.
        // Same layout caveat as Cmd+Shift+G above.
        sendKey(virtualKey: keyCodeFor("v"), flags: [.maskCommand])
        Thread.sleep(forTimeInterval: 0.2)

        // Read the path field's value back to verify the paste landed.
        let pastedValue = stringAttr(field, kAXValueAttribute as String) ?? ""
        guard pastedValue.contains(path) else {
            return openPanelFailure(
                "the path did not land in the Go-to-folder field, which holds '\(pastedValue)'")
        }

        // Return commits the "Go to folder" sheet: the sheet closes, the
        // panel lists the file's folder and selects the file.  kVK_Return = 36.
        sendKey(virtualKey: 36, flags: [])

        // Listing the folder and selecting the file take a moment, and longer on a slow or
        // busy machine, so wait for the panel to show the result rather than for a fixed
        // time.  The sheet is checked first: while it is up, the panel behind it has not
        // taken the path yet.
        let navigationDeadline = Deadline(seconds: 10)
        var sheetStillUp = true
        var shown = ""
        repeat {
            sheetStillUp = findElement(matching: pathFieldPredicate, deadline: navigationDeadline,
                                       maxDepth: AXTree.controlDepth).first != nil
            if !sheetStillUp {
                shown = panelState(panel, deadline: navigationDeadline)
                if panelShowsFolder(of: path, in: panel, deadline: navigationDeadline) {
                    NSLog("navigateOpenPanel: %@ after %.1f s", shown,
                          CFAbsoluteTimeGetCurrent() - started)
                    return nil
                }
            }
            Thread.sleep(forTimeInterval: 0.2)
        } while !navigationDeadline.isPast()

        if sheetStillUp {
            return openPanelFailure("the Go-to-folder sheet is still showing after Return")
        }
        return openPanelFailure(
            "the open panel does not show the folder of \(path) with a file selected; \(shown)")
    }

    /// Log what went wrong, with the tree as it is now, and hand the message back.
    private func openPanelFailure(_ message: String) -> String {
        NSLog("navigateOpenPanel: %@", message)
        NSLog("navigateOpenPanel: AX tree:\n%@", dumpTree())
        return message
    }

    /// Whether the panel is in the folder of the file at path with a file selected.  Two
    /// controls of the panel itself tell: the "where" popup names the folder the panel
    /// lists, and the Open button is enabled only while something is selected.  Reading
    /// those takes a few levels down from the window, and no walk through the file listing,
    /// which can be thousands of rows.  A panel without the popup counts as in the folder.
    private func panelShowsFolder(of path: String, in panel: AXUIElement,
                                  deadline: Deadline) -> Bool {
        guard let openButton = findElement(matching: openButtonPredicate, roots: [panel],
                                           deadline: deadline, maxDepth: AXTree.controlDepth).first,
              (attr(openButton, kAXEnabledAttribute as String) as? Bool) == true else {
            return false
        }
        guard let popup = findElement(matching: wherePopupPredicate, roots: [panel],
                                      deadline: deadline, maxDepth: AXTree.controlDepth).first,
              let shown = stringAttr(popup, kAXValueAttribute as String) else {
            return true
        }
        // The popup shows the folder as Finder names it, which is the display name, and
        // wraps it in Unicode isolate marks for bidirectional text.
        let folder = (path as NSString).deletingLastPathComponent
        let names: Set<String> = [
            (folder as NSString).lastPathComponent,
            FileManager.default.displayName(atPath: folder),
        ]
        return names.contains(plainName(shown))
    }

    /// The values of the two controls panelShowsFolder reads, for the log.
    private func panelState(_ panel: AXUIElement, deadline: Deadline) -> String {
        let popup = findElement(matching: wherePopupPredicate, roots: [panel],
                                deadline: deadline, maxDepth: AXTree.controlDepth).first
        let openButton = findElement(matching: openButtonPredicate, roots: [panel],
                                     deadline: deadline, maxDepth: AXTree.controlDepth).first
        let shown = popup.flatMap { stringAttr($0, kAXValueAttribute as String) }
        let enabled = openButton.flatMap { attr($0, kAXEnabledAttribute as String) as? Bool }
        return "the panel is in '\(shown.map(plainName) ?? "?")' and its Open button is "
            + (enabled.map { $0 ? "enabled" : "disabled" } ?? "missing")
    }

    /// How many levels below a window its controls sit: the buttons, the "where" popup and
    /// the fields of an attached sheet are all within this many, while the rows of the file
    /// listing are below it.
    private static let controlDepth = 6

    private let openButtonPredicate = SimpleXPath.Predicate(
        attribute: "accessibility-id", value: "OKButton")
    private let wherePopupPredicate = SimpleXPath.Predicate(
        attribute: "accessibility-id", value: "where popup")

    /// The text without the Unicode marks that isolate bidirectional runs, U+2066 to U+2069,
    /// and the left-to-right and right-to-left marks U+200E and U+200F.
    private func plainName(_ text: String) -> String {
        let marks: Set<Character> = ["\u{2066}", "\u{2067}", "\u{2068}", "\u{2069}",
                                     "\u{200E}", "\u{200F}"]
        return String(text.filter { !marks.contains($0) })
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
