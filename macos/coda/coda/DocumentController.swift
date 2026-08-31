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

/**
 * NSDocumentController instance to modify functionality related to the Open panel.
 */
final class DocumentController: NSDocumentController {

    /// Whether UI testing mode is active.
    static let isUITesting = ProcessInfo.processInfo.arguments.contains("--uitesting")

    /**
     * How long, in seconds, to wait for modified documents to finish saving
     * before opening another document.
     *
     * Saving runs through the engine and completes asynchronously. Opening
     * waits for the saves to land on disk so the work is recoverable if the
     * load of the next document OOMs, but it gives up after this long so that
     * a stalled save never blocks opening for good.
     */
    private static let flushBeforeOpenTimeoutSeconds: TimeInterval = 8

    /**
     * The current, live NSOpenPanel instance that was open during the app startup.
     *
     * We make sure that there is always just one instance, and focus it in case anybody
     * tries to trigger File -> Open... when the Open panel is already presented.
     */
    private weak var liveOpenPanel: NSOpenPanel?

    /**
     * Fired when the live Open panel picks documents; dropped on cancel.
     */
    private var openPanelPickHandler: (() -> Void)?

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
            if result == NSApplication.ModalResponse.OK.rawValue {
                self.openPanelPickHandler?()
            }
            self.openPanelPickHandler = nil
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

        // Write any unsaved documents to disk before loading this one. Loading a document is a relatively dangerous
        // checkpoint which can e.g. OOM, so ensure already-open work survives if loading takes down the process.
        flushModifiedDocuments {
            self.performOpenDocument(withContentsOf: url, display: displayDocument, completionHandler: completionHandler)
        }
    }

    /**
     * Loads the document.
     */
    private func performOpenDocument(withContentsOf url: URL,
                                     display displayDocument: Bool,
                                     completionHandler: @escaping (NSDocument?, Bool, Error?) -> Void) {
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
     * Save every open document that has unsaved edits, then run `next`.
     *
     * Each save runs through the engine and finishes asynchronously, so this
     * waits for all of them to report completion before running `next`. If a
     * save stalls, a timer runs `next` anyway so opening cannot hang for good.
     * `next` runs exactly once, on the main queue.
     */
    private func flushModifiedDocuments(then next: @escaping () -> Void) {
        let modified = documents.filter { $0.hasUnautosavedChanges }
        if modified.isEmpty {
            next()
            return
        }

        var alreadyProceeded = false
        let proceedOnce = {
            if alreadyProceeded { return }
            alreadyProceeded = true
            next()
        }

        let group = DispatchGroup()
        for doc in modified {
            group.enter()
            doc.autosave(withImplicitCancellability: false) { _ in
                group.leave()
            }
        }
        group.notify(queue: .main, execute: proceedOnce)

        DispatchQueue.main.asyncAfter(deadline: .now() + DocumentController.flushBeforeOpenTimeoutSeconds,
                                      execute: proceedOnce)
    }

    /**
     * Opens the Open panel when no other documents or windows are presented.
     * When a panel is already up it is focused and `onPick` is ignored.
     */
    func focusOrPresentOpenPanel(onPick: (() -> Void)? = nil) {
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

        openPanelPickHandler = onPick

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

    enum NewKind { case text, spreadsheet, presentation, drawing }

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
        case .drawing:      return ("Drawing", "odg")
        }
    }

    /**
     * Opens the bundled welcome file (welcome/welcome-slideshow.odp).
     */
    func openWelcome() {
        guard COWrapper.isWelcomeEnabled() else {
            COWrapper.LOG_TRC("The welcome slideshow is off in the configuration")
            return
        }

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

/**
 * Caps the number of document views that hold a web content process, so the memory the app
 * uses does not grow with every document opened. The dropped documents stay loaded in the
 * engine, so a view built for one later rejoins it instead of reading the file again.
 *
 * The count is across the app, because a document on macOS is a window of its own and the
 * renderers add up whether the windows are tabbed together or not. Only a view the user is
 * not looking at is ever dropped.
 */
final class LiveViewLimit {

    static let shared = LiveViewLimit()

    /// How long the limit waits before it runs. A window becoming main is one of a run of
    /// them as often as not, and the one the user stops on is the one that matters.
    private static let defaultDelayMilliseconds = 1000

    /// How many document views keep a web content process. Zero or less turns the limit
    /// off, which is what it is by default. CODA_LIVE_VIEWS sets it, in the environment or
    /// as a defaults key, which is what reaches an app the user launched rather than one
    /// started from a shell.
    private lazy var limit: Int = {
        if let text = ProcessInfo.processInfo.environment["CODA_LIVE_VIEWS"],
           let value = Int(text) {
            return value
        }
        if UserDefaults.standard.object(forKey: "CODA_LIVE_VIEWS") != nil {
            return UserDefaults.standard.integer(forKey: "CODA_LIVE_VIEWS")
        }
        return 0
    }()

    /// Counts activations. Each view records the value it saw when it was last activated.
    private var activationTick: UInt64 = 0

    /// The pending run, and when it comes due.
    private var pendingRun: DispatchWorkItem?
    private var pendingRunDueAt: Date?

    private init() {
        // Main is what a document window gets when the user turns to it. Key is watched too,
        // because a window can take the keyboard in an app that is not the front one.
        for name in [NSWindow.didBecomeMainNotification, NSWindow.didBecomeKeyNotification] {
            NotificationCenter.default.addObserver(forName: name, object: nil,
                                                   queue: .main) { [weak self] note in
                guard let window = note.object as? NSWindow else { return }
                self?.windowBecameMain(window)
            }
        }
    }

    /// Apply the limit after a delay, so a run of window activations settles first and only
    /// the window the user stops on is acted on.
    func schedule(afterMilliseconds delayMilliseconds: Int = LiveViewLimit.defaultDelayMilliseconds) {
        DispatchQueue.main.async {
            let dueAt = Date().addingTimeInterval(Double(delayMilliseconds) / 1000.0)
            // A run that is already due sooner covers this one too.
            if let pendingDueAt = self.pendingRunDueAt, pendingDueAt <= dueAt {
                return
            }

            self.pendingRun?.cancel()
            self.pendingRunDueAt = dueAt
            let run = DispatchWorkItem { [weak self] in
                self?.pendingRun = nil
                self?.pendingRunDueAt = nil
                self?.enforce()
            }
            self.pendingRun = run
            DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(delayMilliseconds),
                                         execute: run)
        }
    }

    /// A document window has just opened. It counts against the limit from now on.
    func noteDocumentOpened() {
        schedule()
    }

    /// The user is on this window now. Its view comes back, and the limit is asked again.
    private func windowBecameMain(_ window: NSWindow) {
        guard let controller = LiveViewLimit.viewController(of: window) else { return }

        activationTick += 1
        controller.lastActiveTick = activationTick
        // The user is on this document again, so the limit may ask it to save once more when
        // it next falls behind. A save that never came back is forgotten with the rest: a
        // live save still refuses a second request on its own.
        controller.dropSaveAsked = false
        controller.dropWaitsForSave = false

        controller.restoreView()
        schedule()
    }

    /// Drop views, least recently used first, until no more than the limit are live.
    private func enforce() {
        if limit <= 0 { return }

        let controllers = documentViewControllers()
        var live = controllers.filter { !$0.isViewDiscarded }.count

        if live <= limit { return }

        // Least recently used first, and only views the user is not looking at.
        let candidates = controllers
            .filter { !$0.isViewDiscarded && $0.mayDiscardView && !$0.isWindowVisible }
            .sorted { $0.lastActiveTick < $1.lastActiveTick }

        var retryInMilliseconds = -1

        for controller in candidates {
            if live <= limit { break }

            let settleMilliseconds = controller.msUntilViewSettled
            if settleMilliseconds > 0 {
                if retryInMilliseconds < 0 || settleMilliseconds < retryInMilliseconds {
                    retryInMilliseconds = settleMilliseconds
                }
                continue
            }

            if !controller.isReadyToDiscardView {
                requestSaveThenDiscard(controller)
                continue
            }

            controller.discardView()
            live -= 1
        }

        // A view that only has to grow older is worth asking about again once it has.
        if retryInMilliseconds > 0 {
            schedule(afterMilliseconds: retryInMilliseconds)
        }
    }

    /// Ask a document to save, and park the drop of its view on the save result. The save
    /// goes through AppKit, so the user's own file is written before the view goes.
    private func requestSaveThenDiscard(_ controller: ViewController) {
        if controller.dropWaitsForSave || controller.dropSaveAsked { return }
        guard let document = controller.document,
              let url = document.fileURL,
              let typeName = document.fileType else { return }

        NSLog("CollaboraOffice: appDocId \(document.appDocId) saves before its view goes")
        controller.dropWaitsForSave = true
        controller.dropSaveAsked = true

        document.save(to: url, ofType: typeName, for: .saveOperation) { [weak controller] error in
            controller?.dropWaitsForSave = false
            if let error {
                // The view stays, and nothing is put in front of the user. One save was
                // asked for, so the next activation is what asks again.
                NSLog("CollaboraOffice: the save before a view drop failed: \(error.localizedDescription)")
            }
            // The save took time, and in that time the user can have come back to this
            // document. Ask the whole policy again rather than dropping this view now.
            LiveViewLimit.shared.schedule()
        }
    }

    /// Every document view the app has open.
    private func documentViewControllers() -> [ViewController] {
        return NSApp.windows.compactMap { LiveViewLimit.viewController(of: $0) }
    }

    private static func viewController(of window: NSWindow) -> ViewController? {
        return window.windowController?.contentViewController as? ViewController
    }
}
