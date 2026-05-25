/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * coda-driver
 *
 * External, unsandboxed helper that drives a coda app instance through
 * the system accessibility API.  Sits between the wdio test runner and
 * the (sandboxed) main app:
 *
 *   wdio --HTTP-> coda-driver --AX/CGEvent-> coda.app
 *
 * The driver holds the "Accessibility" permission grant.  Because it
 * queries the target via AXUIElementCreateApplication(targetPid), the
 * system AX service bridges into XPC-hosted UI such as NSOpenPanel -
 * the limitation that blocks in-process AX self-introspection in the
 * main app does not apply here.
 *
 * Usage:
 *   coda-driver --target-app <path> --native-port <port>
 *               [-- <args passed to the target app>]
 */

import AppKit
import ApplicationServices
import Foundation

// MARK: - Argument parsing

func argValue(_ name: String, in args: [String]) -> String? {
    guard let idx = args.firstIndex(of: name), idx + 1 < args.count else { return nil }
    return args[idx + 1]
}

let argv = CommandLine.arguments

guard let appPath = argValue("--target-app", in: argv),
      let portString = argValue("--native-port", in: argv),
      let nativePort = UInt16(portString) else {
    let usage = """
    coda-driver: external accessibility driver for coda tests

    Usage:
      coda-driver --target-app <path> --native-port <port> [-- <app args>]
    """
    FileHandle.standardError.write(Data((usage + "\n").utf8))
    exit(2)
}

// Args after "--" go to the target app on launch.
let appArgs: [String] = {
    guard let dashIdx = argv.firstIndex(of: "--") else { return [] }
    return Array(argv.dropFirst(dashIdx + 1))
}()

// MARK: - Log redirection
//
// When launched via `open -a`, stdout/stderr are not piped back to
// the test runner.  --log-file lets the caller specify a file to
// receive our logs (NSLog goes to stderr by default, so redirecting
// stderr captures it).
if let logPath = argValue("--log-file", in: argv) {
    FileManager.default.createFile(atPath: logPath, contents: nil)
    if let fh = FileHandle(forWritingAtPath: logPath) {
        dup2(fh.fileDescriptor, fileno(stdout))
        dup2(fh.fileDescriptor, fileno(stderr))
    }
}

// MARK: - Structured failure reporting
//
// Before exiting on a startup error we write a stable one-line marker
// followed by a human-readable message.  The wdio service matches on
// the marker reason rather than on the prose, so the prose can be
// reworded freely without breaking the fast-fail detection.
//
// Format: CODA-DRIVER-EXIT: <reason>
// Reasons are the public contract; do not rename without updating
// coda-macos.service.ts.
func driverExit(reason: String, message: String, code: Int32) -> Never {
    FileHandle.standardError.write(Data("CODA-DRIVER-EXIT: \(reason)\n".utf8))
    FileHandle.standardError.write(Data(message.utf8))
    exit(code)
}

// MARK: - Accessibility permission

func ensureAccessibilityTrust() {
    let promptKey = kAXTrustedCheckOptionPrompt.takeUnretainedValue() as String
    let options: CFDictionary = [promptKey: true] as CFDictionary
    if !AXIsProcessTrustedWithOptions(options) {
        let msg = """

        ============================================================
        ERROR: Accessibility permission required for coda-driver.

        The driver controls the test app through the system
        accessibility API (AXUIElement / CGEvent).  Open

          System Settings -> Privacy & Security -> Accessibility

        enable coda-driver, then re-run the tests.
        ============================================================

        """
        driverExit(reason: "accessibility-denied", message: msg, code: 1)
    }
}

ensureAccessibilityTrust()

// MARK: - Launch target app

let appURL = URL(fileURLWithPath: appPath)
let config = NSWorkspace.OpenConfiguration()
config.arguments = appArgs
config.activates = true
config.createsNewApplicationInstance = true

var targetPid: pid_t = 0
do {
    let sem = DispatchSemaphore(value: 0)
    var launchError: Error?
    NSWorkspace.shared.openApplication(at: appURL, configuration: config) { app, error in
        if let app = app {
            targetPid = app.processIdentifier
        }
        launchError = error
        sem.signal()
    }
    sem.wait()
    if targetPid == 0 {
        let msg = launchError?.localizedDescription ?? "unknown error"
        driverExit(
            reason: "launch-failed",
            message: "coda-driver: failed to launch \(appPath): \(msg)\n",
            code: 3
        )
    }
}

NSLog("coda-driver: target pid = %d", targetPid)

// MARK: - Start native UI server

// Hold the server in a file-scope binding so it (and its NWListener)
// outlive this block - otherwise the listener is torn down before
// the first connection arrives.
let server: NativeUIServer
do {
    server = try NativeUIServer(port: nativePort, targetPid: targetPid)
    server.start()
} catch {
    FileHandle.standardError.write(Data("coda-driver: failed to start NativeUIServer: \(error.localizedDescription)\n".utf8))
    exit(4)
}
_ = server  // silence "never used" warning - the reference is the point

// Run forever.  The driver is killed by the wdio service on test
// completion (SIGTERM), or when the target app exits.
NSWorkspace.shared.notificationCenter.addObserver(
    forName: NSWorkspace.didTerminateApplicationNotification,
    object: nil,
    queue: nil
) { note in
    guard let app = note.userInfo?[NSWorkspace.applicationUserInfoKey] as? NSRunningApplication else {
        return
    }
    if app.processIdentifier == targetPid {
        NSLog("coda-driver: target app exited, shutting down")
        exit(0)
    }
}

RunLoop.main.run()
