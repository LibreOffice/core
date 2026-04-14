/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * NSDocumentController instance to modify functionality related to the Open panel.
 */
final class DocumentController: NSDocumentController {

    /// Whether UI testing mode is active.
    private static let isUITesting = ProcessInfo.processInfo.arguments.contains("--uitesting")

    /**
     * The current, live NSOpenPanel instance that was open during the app startup.
     *
     * We make sure that there is always just one instance, and focus it in case anybody
     * tries to trigger File -> Open... when the Open panel is already presented.
     */
    private weak var liveOpenPanel: NSOpenPanel?

    /**
     * Window for the New document workflow.
     */
    private static var backstageWindowController: NSWindowController?

    /**
     * One place to decide if we should quit the app on losing focus.
     */
    func hasDocsOrWindows() -> Bool {
        let hasDocs = !documents.isEmpty
        let hasVisibleWindows = NSApp.windows.contains { $0.isVisible }
        return hasDocs || hasVisibleWindows
    }

    /**
     * Remember the live/current Open panel instance.
     */
    override func beginOpenPanel(_ openPanel: NSOpenPanel,
                                 forTypes inTypes: [String]?,
                                 completionHandler: @escaping (Int) -> Void) {

        liveOpenPanel = openPanel

        super.beginOpenPanel(openPanel, forTypes: inTypes) { [weak self] result in
            guard let self = self else { return }
            // Panel is going away regardless of result.
            self.liveOpenPanel = nil
            completionHandler(result)
        }
    }

    /**
     * Make the File -> Open… menu also reuse/focus the existing panel (to avoid 2 open panels at the same time).
     */
    override func openDocument(_ sender: Any?) {
        focusOrPresentOpenPanel()
    }

    /**
     * If the Open panel is live, close it.
     */
    private func closeLiveOpenPanel() {
        liveOpenPanel?.cancel(nil)
        liveOpenPanel = nil
    }

    /**
     * Intercept File -> New (and also the Open panel's New Document button)
     */
    override func newDocument(_ sender: Any?) {
        closeLiveOpenPanel()

        // Open the dialog for the new document
        presentStartupBackstage()
    }

    /**
     * Make sure to cancel the Open panel when a document has been open by different means, eg. via File -> Recent files.
     */
    override func openDocument(withContentsOf url: URL,
                               display displayDocument: Bool,
                               completionHandler: @escaping (NSDocument?, Bool, Error?) -> Void) {
        closeLiveOpenPanel()

        // During UI testing the file comes from the test runner's sandbox,
        // which the app cannot write to.  Copy it into the app's own temp
        // directory so NSDocument considers it writable.
        if DocumentController.isUITesting {
            let appTmpDir = FileManager.default.temporaryDirectory
                .appendingPathComponent("codaUITest-\(UUID().uuidString)")
            do {
                try FileManager.default.createDirectory(at: appTmpDir, withIntermediateDirectories: true)
                let destURL = appTmpDir.appendingPathComponent(url.lastPathComponent)
                try FileManager.default.copyItem(at: url, to: destURL)
                super.openDocument(withContentsOf: destURL, display: displayDocument, completionHandler: completionHandler)
                return
            } catch {
                NSLog("DocumentController: failed to copy test file: %@", error.localizedDescription)
            }
        }

        super.openDocument(withContentsOf: url, display: displayDocument, completionHandler: completionHandler)
    }

    /**
     * Opens the Open panel when no other documents or windows are presented.
     */
    func focusOrPresentOpenPanel() {
        if let panel = liveOpenPanel {
            // Focus the existing panel (sheet or app-modal).
            if let parent = panel.sheetParent {
                parent.makeKeyAndOrderFront(nil)
            }
            else {
                panel.makeKeyAndOrderFront(nil)
            }
            NSApp.activate(ignoringOtherApps: true)
            return
        }

        // No panel up -> trigger standard Open panel flow.
        super.openDocument(nil)
    }

    /**
     * Create or focus the New document window.
     */
    func presentStartupBackstage(calledFromStartup: Bool = false) {
        // Don't present the panel during startup if there are any open windows
        // (eg. when the user started the app from Finder with files to open)
        if calledFromStartup && hasDocsOrWindows() {
            return
        }

        if let wc = DocumentController.backstageWindowController { // focus if already open
            wc.window?.makeKeyAndOrderFront(nil)
            NSApp.activate(ignoringOtherApps: true)
            return
        }

        let vc = BackstageViewController()
        vc.onClose = {
            DocumentController.backstageWindowController?.close()
            DocumentController.backstageWindowController = nil
        }

        // Simple titled window for the web view
        let w = NSWindow(contentViewController: vc)
        w.title = AppDelegate.getAppName() // FIXME anything more?
        w.styleMask = [.titled, .closable, .miniaturizable]
        w.setContentSize(NSSize(width: 1000, height: 640))
        w.setAccessibilityIdentifier("CODA.BackstageWindow")
        w.center()

        let wc = NSWindowController(window: w)
        DocumentController.backstageWindowController = wc
        wc.showWindow(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    enum NewKind { case text, spreadsheet, presentation }

    /**
     * Copy content of the template (based on "kind") to NSDocument and trigger its editing.
     */
    func createDocument(fromTemplateFor kind: NewKind, templatePath: String? = nil) {
        // Pick the template file in your bundle
        let templatesURL: URL?

        if let templatePath = templatePath {
            // Use the provided template path from the backstage view
            templatesURL = Bundle.main.resourceURL?.appendingPathComponent(templatePath)
        } else {
            // Use the default blank template
            guard let (resName, ext) = templateNameAndExt(for: kind) else {
                NSApp.presentError(NSError(domain: "NewDoc", code: 1, userInfo: [NSLocalizedDescriptionKey: "Template not found."]))
                return
            }
            templatesURL = Bundle.main.url(forResource: resName, withExtension: ext, subdirectory: "templates")
        }

        guard let templatesURL = templatesURL else {
            NSApp.presentError(NSError(domain: "NewDoc", code: 1, userInfo: [NSLocalizedDescriptionKey: "Template not found."]))
            return
        }

        // Ask AppKit which document type this URL maps to (so your Info.plist drives it)
        let typeName: String
        do {
            typeName = try self.typeForContents(of: templatesURL)
        } catch {
            NSApp.presentError(error)
            return
        }

        // Load the template data
        let data: Data
        do {
            data = try Data(contentsOf: templatesURL)
        } catch {
            NSApp.presentError(error)
            return
        }

        // Create an *untitled* document of the resolved type,
        // then feed it the template data via your existing read(from:data:) path.
        do {
            guard let doc = try makeUntitledDocument(ofType: typeName) as? Document else {
                throw NSError(domain: "NewDoc", code: 2, userInfo: [NSLocalizedDescriptionKey: "Unexpected document class"])
            }

            // mark it as new document (for better UI handling)
            doc.isNewDocument = true
            doc.documentType = typeName  // Store the type explicitly for icon display

            try doc.read(from: data, ofType: typeName) // <- seeds tempDirectoryURL/tempFileURL exactly like a normal open

            // Show it, using the usual controllers
            addDocument(doc)
            doc.makeWindowControllers()
            doc.showWindows()

            // The doc starts unmodified; Document manages that itself.
        } catch {
            NSApp.presentError(error)
        }
    }

    private func templateNameAndExt(for kind: NewKind) -> (String, String)? {
        switch kind {
        case .text:         return ("TextDocument", "odt")
        case .spreadsheet:  return ("Spreadsheet", "ods")
        case .presentation: return ("Presentation", "odp")
        }
    }

    /**
     * Opens the bundled welcome file (welcome/welcome-slideshow.odp).
     */
    func openWelcome() {
        guard let url = Bundle.main.url(forResource: "welcome-slideshow", withExtension: "odp", subdirectory: "welcome") else {
            COWrapper.LOG_ERR("welcome/welcome.odp not found in bundle")
            return
        }

        do {
            let type = try self.typeForContents(of: url)
            guard let doc = try self.makeDocument(withContentsOf: url, ofType: type) as? Document else {
                throw NSError(domain: "Welcome", code: 1, userInfo: [NSLocalizedDescriptionKey: "Unexpected document class"])
            }

            // Flag before windows/controllers are created, so VC will pass welcome=true into the webview
            doc.isWelcome = true

            self.addDocument(doc)
            doc.makeWindowControllers()
            doc.showWindows()
        } catch {
            COWrapper.LOG_ERR("Failed to open welcome-slideshow document: \(error.localizedDescription)")
        }
    }
}
