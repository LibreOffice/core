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

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    /**
     * Create our document controller early so it becomes the shared instance.
     */
    private let documentController = DocumentController()

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        setUserName();

        setenv("FONTCONFIG_PATH", Bundle.main.resourcePath! + "/fontconfig", 1)

        // Initialize the COOLWSD
        COWrapper.startServer()

        // We have to set the product name in the menu entries explicitly, there seems to be no automatic way to do that
        updateProductName()

        // Schedule opening of the Open panel if no document is open in 100ms
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) { [weak self] in
            self?.documentController.presentStartupBackstage(calledFromStartup: true)
        }

        // Now remove the Share menu item, as we haven't implemented sharing yet.
        if let mainMenu = NSApp.mainMenu,
            let fileMenuItem = mainMenu.item(withTitle: "File"),
            let fileSubmenu = fileMenuItem.submenu,
            let shareItem = fileSubmenu.item(withTitle: "Share") {
            fileSubmenu.removeItem(shareItem)
            if let editMenu = NSApp.mainMenu?.item(withTitle: "Edit")?.submenu {
                let itemsToRemove = ["Writing Tools", "AutoFill", "Start Dictation", "Emoji & Symbols"]
                for title in itemsToRemove {
                    if let item = editMenu.item(withTitle: title) {
                        editMenu.removeItem(item)
                    }
                }
            }
        }
    }

    /**
     * Replaces "ProductName" in a menu item title with the real app name.
     * The menu item is identified by its selector.
     * The title is already localized by AppKit from Main.strings, so we
     * only need to substitute the placeholder.
     */
    private func replaceProductName(in menu: NSMenu, withAction action: Selector, appName: String) {
        for item in menu.items {
            if item.action == action {
                item.title = item.title.replacingOccurrences(of: "ProductName", with: appName)
                return
            }
            if let sub = item.submenu {
                replaceProductName(in: sub, withAction: action, appName: appName)
            }
        }
    }

    /**
     * Return the app's name (with some reasonable fallback).
     */
    static func getAppName() -> String {
        let info = Bundle.main.infoDictionary
        return (info?["CFBundleDisplayName"] as? String)
            ?? (info?["CFBundleName"] as? String)
            ?? ProcessInfo.processInfo.processName
    }

    /**
     * Use the real app name in the menus where it is expected.
     * AppKit has already applied the localized Main.strings to the
     * storyboard, so the titles contain "ProductName" in the correct
     * localized form (e.g. "Über ProductName" in German).  We just
     * swap that placeholder for the real name.
     */
    private func updateProductName() {
        guard let mainMenu = NSApp.mainMenu else { return }

        let name = AppDelegate.getAppName()

        // The app menu item and its submenu title
        if let appMenuItem = mainMenu.items.first {
            appMenuItem.title = appMenuItem.title.replacingOccurrences(of: "ProductName", with: name)
            appMenuItem.submenu?.title = appMenuItem.submenu?.title.replacingOccurrences(of: "ProductName", with: name) ?? name
        }

        replaceProductName(in: mainMenu, withAction: #selector(NSApplication.orderFrontStandardAboutPanel(_:)), appName: name)
        replaceProductName(in: mainMenu, withAction: #selector(NSApplication.hide(_:)), appName: name)
        replaceProductName(in: mainMenu, withAction: #selector(NSApplication.terminate(_:)), appName: name)
    }

    /**
     * Don’t auto-create an empty document at launch.
     */
    func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
        return false
    }

    /**
     * We close the app when there is no document or Open panel, but handle clicking the Dock's icon anyway, just for case.
     */
    func applicationShouldHandleReopen(_ app: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        if !flag { documentController.focusOrPresentOpenPanel() }
        return false
    }

    /// Holds App Kit’s continuation block while we close documents.
    private var pendingQuitReply: ((Bool) -> Void)?

    /**
     * Intercept ⌘Q / Dock-Quit / system shutdown.
     * See https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/AppArchitecture/Tasks/GracefulAppTermination.html
     * for the description how to intercept the document closing and/or delay that until the cleanup is performed.
     */
    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply
    {
        NSLog("CollaboraOffice: App quit requested")

        // save the continuation so we can reply later
        pendingQuitReply = { sender.reply(toApplicationShouldTerminate: $0) }

        // ask every open document to close; the delegate callback will run
        for doc in NSDocumentController.shared.documents {
            doc.canClose(withDelegate: self,
                         shouldClose: #selector(document(_:shouldClose:context:)),
                         contextInfo: nil)
        }

        // if there were no documents at all, we’re ready to quit
        if NSDocumentController.shared.documents.isEmpty {
            finishQuit(.terminateNow)
        }

        // tell App Kit we’ll decide later
        return .terminateLater
    }

    /**
     * Each NSDocument calls this when its asynchronous closing finishes
     */
    @objc func document(_ doc: NSDocument,
                        shouldClose: Bool,
                        context contextInfo: UnsafeMutableRawPointer?)
    {
        NSLog("CollaboraOffice: Checking if we should close the document")
        // user hit “Cancel” in a save dialog => abort quit
        if !shouldClose { return finishQuit(.terminateCancel) }

        // close the document in COOLWSD
        doc.close()

        // when the last doc is gone, we can tear everything down
        if NSDocumentController.shared.documents.isEmpty {
            finishQuit(.terminateNow)
        }
    }

    /**
     * Finalise quit or cancel
     */
    private func finishQuit(_ reply: NSApplication.TerminateReply) {
        guard let replyBlock = pendingQuitReply else { return }
        pendingQuitReply = nil

        if reply == .terminateNow {
            // All docs closed: safe place for global tear-down
            COWrapper.stopServer()
            NSLog("CollaboraOffice: Core shut down, quitting.")
        }
        else {
            NSLog("CollaboraOffice: Quit cancelled.")
        }

        replyBlock(reply == .terminateNow)
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }

    /**
     * Don't use the built-in mechanism to close without windows, it is unreliable in many scenarios.
     *
     * Instead, we'll check for any open document or window when the app loses focus.
     */
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return false
    }

    /**
     * Quit when the app loses focus AND there are no docs and no visible windows/panels.
     */
    func applicationDidResignActive(_ notification: Notification) {
        if !documentController.hasDocsOrWindows() {
            NSApp.terminate(nil)
        }
    }
}
