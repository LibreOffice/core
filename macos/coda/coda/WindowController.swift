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

    /// Remember the new command state value as provided by JS.
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
