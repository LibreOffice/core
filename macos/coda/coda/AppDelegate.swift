/*
 * Copyright the Collabora Office contributors.
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

        // The folders the user allowed for the files documents link to stay readable.
        LinkedFolderAccess.restore()

        setenv("FONTCONFIG_PATH", Bundle.main.resourcePath! + "/fontconfig", 1)

        // Initialize the COOLWSD
        COWrapper.startServer()

        // The live-view limit watches for windows becoming main, so it has to exist before
        // the first document window does.
        LiveViewLimit.shared.schedule()

        // Start the WebDriver server if requested via --testDriverPort=<port>.
        // This happens early, before any window/webview is created, so the
        // test setup can wait for /status and drive the backstage from there.
        WebDriverManager.shared.startIfRequested()

        // We have to set the product name in the menu entries explicitly, there seems to be no automatic way to do that
        updateProductName()

        // Run after updateProductName so the localized "About <name>" title is
        // already in place when we take over the menu item.
        useCustomAboutPanel()

        // Schedule opening of the Open panel if no document is open in 100ms
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) { [weak self] in
            self?.documentController.presentStartupBackstage(calledFromStartup: true)
        }

        removeInjectedMenuItems()
    }

    func applicationWillFinishLaunching(_ notification: Notification) {
        // AppKit would add the Writing Tools items to the Edit menu; the web editor has its own.
        if #available(macOS 15.2, *),
           let mainMenu = NSApp.mainMenu,
           let editMenu = Self.menu(in: mainMenu, containingAction: NSSelectorFromString("undo:")) {
            editMenu.automaticallyInsertsWritingToolsItems = false
        }
    }

    /**
     * AppKit adds its own items to the File and Edit menus: Share, Writing Tools, AutoFill,
     * Start Dictation and Emoji & Symbols. Sharing is not implemented, and the rest belong
     * to native text views, not to the web editor. Their titles are in the user's language
     * (and so are ours, from Main.strings), so they cannot be found by name. Instead, keep
     * the items the storyboard defines, identified by their action, and drop everything else.
     * A new item in the storyboard's File or Edit menu needs its action added here.
     */
    private func removeInjectedMenuItems() {
        guard let mainMenu = NSApp.mainMenu else { return }

        if let fileMenu = Self.menu(in: mainMenu, containingAction: NSSelectorFromString("openDocument:")) {
            Self.keepOnly(actions: ["newDocument:", "openDocument:", "clearRecentDocuments:",
                                    "performClose:", "saveDocument:", "saveDocumentAs:",
                                    "revertDocumentToSaved:", "runPageLayout:", "printDocument:"],
                          in: fileMenu)
        }
        if let editMenu = Self.menu(in: mainMenu, containingAction: NSSelectorFromString("undo:")) {
            Self.keepOnly(actions: ["undo:", "redo:", "cut:", "copy:", "paste:", "pasteAsPlainText:"],
                          in: editMenu)
        }
    }

    /**
     * Find the (sub)menu that has an item with the given action.
     */
    private static func menu(in menu: NSMenu, containingAction action: Selector) -> NSMenu? {
        for item in menu.items {
            if item.action == action {
                return menu
            }
            if let sub = item.submenu, let found = Self.menu(in: sub, containingAction: action) {
                return found
            }
        }
        return nil
    }

    /**
     * Remove every item of the menu whose action is not listed. An item with a submenu stays
     * when something in that submenu is listed (Open Recent, through Clear Menu). Separators
     * stay, except where removing items left them doubled or at an end of the menu.
     */
    private static func keepOnly(actions: [String], in menu: NSMenu) {
        let keep = Set(actions.map(NSSelectorFromString))
        func isKept(_ item: NSMenuItem) -> Bool {
            if let action = item.action, keep.contains(action) {
                return true
            }
            if let sub = item.submenu {
                return sub.items.contains(where: isKept)
            }
            return false
        }
        for item in menu.items where !item.isSeparatorItem && !isKept(item) {
            menu.removeItem(item)
        }
        var previousWasSeparator = true
        for item in menu.items {
            if item.isSeparatorItem {
                if previousWasSeparator {
                    menu.removeItem(item)
                }
                previousWasSeparator = true
            } else {
                previousWasSeparator = false
            }
        }
        if let last = menu.items.last, last.isSeparatorItem {
            menu.removeItem(last)
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
     * Point the "About" menu item at our own handler. The storyboard wires it
     * to the system selector, whose panel only knows the bundle name and
     * version; we want the same details the web Help -> About dialog shows.
     */
    private func useCustomAboutPanel() {
        guard let mainMenu = NSApp.mainMenu else { return }
        redirectAction(in: mainMenu,
                       from: #selector(NSApplication.orderFrontStandardAboutPanel(_:)),
                       to: #selector(showAboutPanel(_:)),
                       target: self)
    }

    /**
     * Find the first menu item with the given action and make it call a
     * different action on a different target instead.
     */
    private func redirectAction(in menu: NSMenu, from oldAction: Selector, to newAction: Selector, target: AnyObject) {
        for item in menu.items {
            if item.action == oldAction {
                item.action = newAction
                item.target = target
                return
            }
            if let sub = item.submenu {
                redirectAction(in: sub, from: oldAction, to: newAction, target: target)
            }
        }
    }

    /**
     * Show the About panel with the same details as the web Help -> About
     * dialog: the product name, the version with its source revision, a link
     * to the source revision and to the license, and the copyright line.
     */
    @objc private func showAboutPanel(_ sender: Any?) {
        let hash = COWrapper.coolwsdVersionHash()

        // The standard panel reads the copyright line from the raw "Copyright"
        // key, which has no typed constant.
        let copyrightKey = NSApplication.AboutPanelOptionKey(rawValue: "Copyright")
        let options: [NSApplication.AboutPanelOptionKey: Any] = [
            .applicationName: AppDelegate.getAppName(),
            .applicationVersion: COWrapper.coolwsdVersion(),
            .version: hash,
            copyrightKey: COWrapper.copyrightNotice(),
            .credits: AppDelegate.aboutCredits(hash: hash),
        ]

        NSApp.orderFrontStandardAboutPanel(options: options)
    }

    /**
     * The scrollable text shown under the version in the About panel. It links
     * the source revision to the online change browser and offers the license,
     * matching the links the web About dialog shows in the apps.
     */
    private static func aboutCredits(hash: String) -> NSAttributedString {
        let credits = NSMutableAttributedString()

        // Open the source revision in the online change browser.
        if let revisionURL = URL(string: "https://gerrit.collaboraoffice.com/plugins/gitiles/online/+log/\(hash)") {
            credits.append(NSAttributedString(
                string: NSLocalizedString("View source revision", comment: "About panel"),
                attributes: [.link: revisionURL]))
        }

        // Offer the bundled license document when the build includes one.
        if let licenseURL = Bundle.main.url(forResource: "LICENSE", withExtension: "html") {
            credits.append(NSAttributedString(string: "\n"))
            credits.append(NSAttributedString(
                string: NSLocalizedString("License Information", comment: "About panel"),
                attributes: [.link: licenseURL]))
        }

        let centered = NSMutableParagraphStyle()
        centered.alignment = .center
        credits.addAttribute(.paragraphStyle, value: centered,
                             range: NSRange(location: 0, length: credits.length))
        return credits
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

        let accountWindows = CloudAccountWindow.openAccountWindows
        if accountWindows.isEmpty {
            closeAllDocuments()
        } else {
            // As with the close button, each editor page saves first. The quit goes on in
            // lastAccountWindowClosed, or stops in accountWindowKeptOpen.
            for window in accountWindows {
                window.performClose(nil)
            }
        }

        // tell App Kit we’ll decide later
        return .terminateLater
    }

    func lastAccountWindowClosed() {
        if pendingQuitReply != nil {
            closeAllDocuments()
        }
    }

    func accountWindowKeptOpen() {
        finishQuit(.terminateCancel)
    }

    private func closeAllDocuments() {
        // ask every open document to close; the delegate callback will run
        for doc in NSDocumentController.shared.documents {
            // In UI testing mode the quit comes from the test harness and
            // there is no user to answer a save sheet, so discard unsaved
            // changes: clearing the change count makes canClose proceed
            // without asking.
            if DocumentController.isUITesting {
                doc.updateChangeCount(.changeCleared)
            }
            doc.canClose(withDelegate: self,
                         shouldClose: #selector(document(_:shouldClose:context:)),
                         contextInfo: nil)
        }

        // if there were no documents at all, we’re ready to quit
        if NSDocumentController.shared.documents.isEmpty {
            finishQuit(.terminateNow)
        }
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
