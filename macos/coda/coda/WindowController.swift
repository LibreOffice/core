/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Cocoa
import WebKit

/// The structure we expect in the COMMANDSTATECHANGED payload.
struct CommandStateChange: Decodable {
    let commandName: String
    let state: String
}

/// The "result" member of a command result: a type name, plus the value for the
/// types whose value is a string.
struct CommandResultValue: Decodable {
    let type: String
    let value: String?

    // Writing the initializer below means the compiler no longer derives these.
    private enum CodingKeys: String, CodingKey {
        case type
        case value
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        type = (try? container.decodeIfPresent(String.self, forKey: .type)) ?? ""
        // The value has whatever type the "type" member names, and only string
        // values are of interest here, so anything else reads as no value.
        value = (try? container.decodeIfPresent(String.self, forKey: .value)) ?? nil
    }
}

/// The structure we expect in the COMMANDRESULT payload.
struct CommandResult: Decodable {
    let commandName: String
    let success: Bool?
    let wasModified: Bool?
    let result: CommandResultValue?
}

/// Window controller that manages the document window & menus
class WindowController: NSWindowController {

    /// Track menu states here, to be able to show the state in the menu
    var commandState: [String: String] = [:]

    /// The document window that was the main window last.
    static weak var lastMainDocumentWindow: NSWindow?

    /// Remember the new command state value as reported by the engine.
    func handleCommandStateChange(_ stateChange: CommandStateChange) {
        // extract the command name from the ".uno:CommandName" form
        let unoCommmandName = stateChange.commandName
        guard let colon = unoCommmandName.firstIndex(of: ":") else { return }
        let commandName = unoCommmandName[unoCommmandName.index(after: colon)...]

        // store it
        commandState[String(commandName)] = stateChange.state
    }
}

/**
 * Extend the WKWebView so that we can set the state of Cut/Copy/Paste in the macOS menu too.
 */
extension WKWebView: @retroactive NSMenuItemValidation {

    public func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        guard let commandState = (window?.windowController as? WindowController)?.commandState else {
            return true
        }

        if menuItem.action == #selector(NSText.cut(_:)) {
            return commandState["Cut"] != "disabled"
        }
        else if menuItem.action == #selector(NSText.copy(_:)) {
            return commandState["Copy"] != "disabled"
        }
        else if menuItem.action == #selector(NSText.paste(_:)) {
            return commandState["Paste"] != "disabled"
        }

        return true
    }
}

extension WindowController: NSWindowDelegate {

    // The document window that was main last is the one that a new document joins as a tab.
    func windowDidBecomeMain(_ notification: Notification) {
        guard let window, (document as? Document)?.isWelcome == false else {
            return
        }
        WindowController.lastMainDocumentWindow = window
    }
}

extension WindowController: NSToolbarDelegate {

    private static let newDocumentItem = NSToolbarItem.Identifier("CODANewDocument")

    /// Give the window a toolbar with just a + button at its end, which opens a new document. The
    /// toolbar is part of the title bar, so the button is there whether one document is open or
    /// several.
    func addNewDocumentToolbar() {
        guard let window else { return }

        let toolbar = NSToolbar(identifier: "CODADocumentToolbar")
        toolbar.delegate = self
        toolbar.displayMode = .iconOnly
        toolbar.allowsUserCustomization = false
        window.toolbar = toolbar
        window.toolbarStyle = .unified
    }

    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return [.flexibleSpace, WindowController.newDocumentItem]
    }

    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        return toolbarDefaultItemIdentifiers(toolbar)
    }

    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier,
                 willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        guard itemIdentifier == WindowController.newDocumentItem else {
            return nil
        }

        let label = NSLocalizedString("New", comment: "Toolbar button that opens a new document")
        let item = NSToolbarItem(itemIdentifier: itemIdentifier)
        item.label = label
        item.toolTip = label
        item.image = NSImage(systemSymbolName: "plus", accessibilityDescription: label)
        item.isBordered = true
        // The action goes up the responder chain to the document controller.
        item.action = #selector(NSDocumentController.newDocument(_:))
        return item
    }
}
