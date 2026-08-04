/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { execSync } from 'child_process';
import {
	appendFileSync,
	cpSync,
	createReadStream,
	existsSync,
	mkdirSync,
	mkdtempSync,
	readFileSync,
	rmSync,
	statSync,
	watchFile,
	unwatchFile,
	writeFileSync,
} from 'fs';
import http from 'http';
import { tmpdir } from 'os';
import { join } from 'path';
import { promisify } from 'util';

const sleep = promisify(setTimeout);

interface CodaMacOSServiceOptions {
	appPath: string;
	driverPath: string; // path to the coda-driver Swift package directory
	webDriverPort: number | string;
	nativeUIPort: number | string;
	fixturesDir: string;
}

/**
 * One GET with a short timeout.  Resolves to the parsed 'value' object of
 * a W3C-style JSON body, or null when the endpoint does not answer with
 * HTTP 200 / valid JSON.
 */
function httpGetValue(url: string): Promise<Record<string, unknown> | null> {
	return new Promise((resolve) => {
		const req = http.get(url, (res) => {
			if (res.statusCode !== 200) {
				resolve(null);
				res.resume();
				return;
			}
			let buf = '';
			res.on('data', (chunk) => (buf += chunk));
			res.on('end', () => {
				try {
					const parsed = JSON.parse(buf);
					resolve(
						parsed?.value && typeof parsed.value === 'object'
							? (parsed.value as Record<string, unknown>)
							: null,
					);
				} catch {
					resolve(null);
				}
			});
		});
		req.on('error', () => resolve(null));
		req.setTimeout(1000, () => {
			req.destroy();
			resolve(null);
		});
	});
}

function isProcessAlive(pid: number): boolean {
	try {
		// Signal 0 performs the permission/existence check without
		// delivering anything.
		process.kill(pid, 0);
		return true;
	} catch {
		return false;
	}
}

async function waitForHttp(
	url: string,
	label: string,
	maxAttempts = 30,
	failureCheck?: () => string | null,
): Promise<Record<string, unknown> | null> {
	for (let i = 0; i < maxAttempts; i++) {
		// Fast-fail: bail out as soon as a known startup failure is
		// detected (e.g. the driver wrote an Accessibility-permission
		// error to its log).  Otherwise the user would wait for the
		// full HTTP poll timeout for no reason.
		if (failureCheck) {
			const failure = failureCheck();
			if (failure) throw new Error(failure);
		}

		const value = await httpGetValue(url);
		if (value !== null) {
			console.log(`${label} is ready`);
			return value;
		}
		await sleep(1000);
	}
	throw new Error(
		`${label} not available at ${url} after ${maxAttempts} attempts`,
	);
}

/**
 * User-friendly explanations for each CODA-DRIVER-EXIT reason.  The
 * driver writes the bare reason code; this is the only place where
 * UI text lives, so the wording can evolve without coupling to Swift.
 *
 * Keys MUST stay in sync with the reasons emitted by driverExit() in
 * macos/coda-driver/Sources/coda-driver/main.swift.
 */
const DRIVER_EXIT_MESSAGES: Record<string, string> = {
	'accessibility-denied':
		'coda-driver.app does not have Accessibility permission.\n' +
		'Open System Settings -> Privacy & Security -> Accessibility,\n' +
		'enable coda-driver, then re-run the tests.',
	'launch-failed':
		'coda-driver failed to launch the target app.  See the driver\n' +
		'log above for the underlying NSWorkspace error.',
};

/**
 * Scan the driver log for a structured exit marker emitted by
 * driverExit() in main.swift.  Returns a user-friendly message if
 * one is detected, otherwise null.
 */
function checkDriverLogForFailure(logFile: string | null): string | null {
	if (!logFile || !existsSync(logFile)) return null;
	let content: string;
	try {
		content = readFileSync(logFile, 'utf8');
	} catch {
		return null;
	}
	const m = content.match(/^CODA-DRIVER-EXIT: (\S+)$/m);
	if (!m) return null;
	const reason = m[1];
	return (
		DRIVER_EXIT_MESSAGES[reason] ??
		`coda-driver exited at startup (reason: ${reason}).  See driver log.`
	);
}

/**
 * Build coda-driver and wrap it in a .app bundle.  Returns the absolute
 * path to the .app bundle.  Calls macos/coda-driver/bundle.sh which
 * does swift build + bundle layout + ad-hoc codesign with stable id.
 *
 * The .app bundle is what TCC attributes the Accessibility grant to,
 * scoped by CFBundleIdentifier=com.collabora.coda-driver rather than
 * the launching shell.
 */
function buildDriver(driverPath: string): string {
	console.log('Building coda-driver...');
	execSync('./bundle.sh', {
		cwd: driverPath,
		stdio: ['ignore', 'inherit', 'inherit'],
	});
	const bundle = join(driverPath, 'coda-driver.app');
	if (!existsSync(bundle)) {
		throw new Error(`coda-driver bundle not found at ${bundle}`);
	}
	return bundle;
}

/**
 * Tail a file (`tail -F`-style): forward each line to console with the
 * given prefix.  Returns a function that stops tailing.  Tolerates the
 * file not existing yet.
 */
function tailFile(path: string, prefix: string): () => void {
	let offset = 0;
	let active = true;

	const read = () => {
		if (!active || !existsSync(path)) return;
		const size = statSync(path).size;
		if (size <= offset) {
			offset = Math.min(offset, size); // file rotated
			return;
		}
		const stream = createReadStream(path, { start: offset, end: size - 1 });
		let buf = '';
		stream.on('data', (chunk) => (buf += chunk.toString()));
		stream.on('end', () => {
			offset = size;
			for (const line of buf.split('\n')) {
				if (line.length > 0) console.log(`${prefix} ${line}`);
			}
		});
	};

	// Ensure the file exists so watchFile fires.
	if (!existsSync(path)) {
		writeFileSync(path, '');
	}
	watchFile(path, { interval: 200 }, read);
	read(); // initial read

	return () => {
		active = false;
		unwatchFile(path, read);
	};
}

export class CodaMacOSServiceLauncher {
	#testDocDir: string | null = null;
	#driverLogFile: string | null = null;
	#stopTail: (() => void) | null = null;
	#appLogPoll: ReturnType<typeof setInterval> | null = null;
	#appLogFile: string | null = null;
	#appLogOffset = 0;
	#appLogDraining = false;
	#appPid: number | null = null;
	#appGoneReported = false;
	#options: CodaMacOSServiceOptions;

	/// How many lines of the app's log to show when a run fails.
	static #APP_LOG_TAIL_LINES = 60;

	constructor(options: CodaMacOSServiceOptions) {
		this.#options = options;
	}

	/**
	 * Say once that the app is no longer running, when it went away on its own.
	 *
	 * Without this the first test to touch the app afterwards fails with a bare
	 * connection error, which says nothing about why, and the run before it looks
	 * like it passed cleanly.
	 */
	#reportAppGone(): void {
		if (this.#appGoneReported || this.#appPid === null) return;
		if (isProcessAlive(this.#appPid)) return;

		this.#appGoneReported = true;
		console.error(
			`\nThe app (pid ${this.#appPid}) is gone; it exited on its own, so ` +
				`the tests from here on cannot reach it.\n` +
				`Its log up to that point is in ${this.#appLogFile}, and a crash ` +
				`report, if it crashed, is in ~/Library/Logs/DiagnosticReports.\n`,
		);
	}

	/**
	 * Fetch whatever the app has logged since the last call and append it to the
	 * log file kept for this run.
	 *
	 * The app's own log lives inside its sandbox container, so it hands the
	 * content out over the test driver's connection rather than us reading the
	 * file there.  Calls do not overlap: a slow answer would otherwise let a
	 * second request ask from an offset the first has not accounted for yet, and
	 * the same lines would be written twice.
	 */
	async #drainAppLog(): Promise<void> {
		if (this.#appLogDraining || this.#appLogFile === null) return;
		this.#appLogDraining = true;
		try {
			const { webDriverPort } = this.#options;
			const answer = await httpGetValue(
				`http://localhost:${webDriverPort}/log/${this.#appLogOffset}`,
			);
			if (answer === null) this.#reportAppGone();
			if (typeof answer?.content !== 'string') return;
			if (typeof answer.offset === 'number') {
				this.#appLogOffset = answer.offset;
			}
			if (answer.content.length > 0) {
				appendFileSync(this.#appLogFile, answer.content);
			}
		} finally {
			this.#appLogDraining = false;
		}
	}

	/**
	 * Print the last lines of the collected app log, and where the whole of it
	 * is.  For a failing run, where the app's own account of what happened is
	 * usually what one wants to read first.
	 */
	#printAppLogTail(): void {
		if (this.#appLogFile === null) return;
		let content: string;
		try {
			content = readFileSync(this.#appLogFile, 'utf8');
		} catch (e) {
			console.warn(
				`Could not read the app's log: ${(e as Error).message}`,
			);
			return;
		}

		const lines = content.split('\n').filter((line) => line.length > 0);
		const tail = lines.slice(-CodaMacOSServiceLauncher.#APP_LOG_TAIL_LINES);
		console.log(
			`\nThe last ${tail.length} of ${lines.length} lines the app logged ` +
				`(all of it is in ${this.#appLogFile}):`,
		);
		for (const line of tail) {
			console.log(`[coda-app]: ${line}`);
		}
		console.log('');
	}

	/**
	 * Quit the app and make sure both it and the driver are really gone.
	 *
	 * Ask nicely first (AppleScript quit; in UI testing mode the app
	 * discards unsaved changes, so this returns promptly), then verify
	 * the processes exited using the pids the driver's /status reports,
	 * and SIGKILL whatever is left.  The driver observes the app's
	 * termination and exits on its own, so it only ever needs the kill
	 * when it is itself stuck.
	 */
	async #shutdownApp(): Promise<void> {
		const { nativeUIPort } = this.#options;

		// Grab the pids while the driver still answers.
		const status = await httpGetValue(
			`http://localhost:${nativeUIPort}/status`,
		);
		const targetPid =
			typeof status?.targetPid === 'number' ? status.targetPid : null;
		const driverPid =
			typeof status?.driverPid === 'number' ? status.driverPid : null;

		try {
			execSync(
				'osascript -e \'tell application "Collabora Office" to quit\'',
				{ timeout: 10000 },
			);
		} catch {
			// App may already be gone, or the quit is stuck; the pid
			// checks below sort the two cases out.
		}

		if (targetPid !== null) {
			for (let i = 0; i < 20 && isProcessAlive(targetPid); i++) {
				await sleep(500);
			}
			if (isProcessAlive(targetPid)) {
				console.warn(
					`Collabora Office (pid ${targetPid}) did not quit; killing it`,
				);
				try {
					process.kill(targetPid, 'SIGKILL');
				} catch {
					// Exited in the meantime
				}
			}
		} else {
			// No pid to watch (the driver was already gone); leave the
			// grace period the quit needs to finish.
			await sleep(1500);
		}

		if (driverPid !== null) {
			for (let i = 0; i < 10 && isProcessAlive(driverPid); i++) {
				await sleep(500);
			}
			if (isProcessAlive(driverPid)) {
				console.warn(
					`coda-driver (pid ${driverPid}) did not exit; killing it`,
				);
				try {
					process.kill(driverPid, 'SIGKILL');
				} catch {
					// Exited in the meantime
				}
			}
		}
	}

	async onPrepare(config?: { outputDir?: string }): Promise<void> {
		const { appPath, driverPath, webDriverPort, nativeUIPort, fixturesDir } = this.#options;

		// A leftover instance from an earlier run still owns the test
		// ports: a fresh app instance would fail to bind them and the
		// tests would silently drive the stale instance, old documents
		// and all.  Shut it down before launching.
		const stale = await httpGetValue(
			`http://localhost:${webDriverPort}/status`,
		);
		if (stale !== null) {
			console.warn(
				'A Collabora Office test instance from a previous run is ' +
					'still running; shutting it down first...',
			);
			await this.#shutdownApp();
		}

		// Copy fixtures to a temp directory; tests open files from there
		// via the JS bridge or the native file dialog.
		this.#testDocDir = mkdtempSync(join(tmpdir(), 'coda-macos-test-'));
		mkdirSync(join(this.#testDocDir, 'Documents'));
		cpSync(fixturesDir, join(this.#testDocDir, 'Documents'), {
			recursive: true,
		});
		process.env.CODA_MACOS_TEST_DOCUMENTS_DIR = join(
			this.#testDocDir,
			'Documents',
		);
		// Cross-platform env vars consumed by qt/test/lib/file-dialog.ts.
		process.env.CODA_TEST_DOCUMENTS_DIR = process.env.CODA_MACOS_TEST_DOCUMENTS_DIR;
		process.env.CODA_PLATFORM = 'macos';

		const driverBundle = buildDriver(driverPath);

		// `open -a` does not pipe stdio.  Have the driver redirect its
		// output to a file we tail to forward to console.
		this.#driverLogFile = join(this.#testDocDir, 'coda-driver.log');
		this.#stopTail = tailFile(this.#driverLogFile, '[coda-driver]:');

		console.log('Starting coda-driver via LaunchServices (open -a)...');
		// Launching through `open -a` registers the bundle with
		// LaunchServices and makes it its own TCC responsible code.
		// Without this, the Accessibility grant is attributed to the
		// launching shell (Terminal/Xcode/...).
		execSync(
			[
				'open',
				'-na', // -n: new instance, -a: launch app at the given path
				JSON.stringify(driverBundle),
				'--args',
				'--target-app', JSON.stringify(appPath),
				'--native-port', String(nativeUIPort),
				'--log-file', JSON.stringify(this.#driverLogFile),
				'--',
				// Keep the number of arguments below even.  The app's own
				// defaults handling reads them as -key value pairs, so an odd
				// one out leaves a stray token that AppKit takes for a file to
				// open ("could not open document YES").
				'--uitesting',
				`--testDriverPort=${webDriverPort}`,
				'-ApplePersistenceIgnoreState', 'YES',
			].join(' '),
			{ stdio: ['ignore', 'inherit', 'inherit'] },
		);

		// Wait for both servers.  The main app's WebDriverServer runs
		// in-process; the driver's NativeUIServer runs out-of-process.
		// Both polls also watch the driver log for known startup
		// failures (e.g. missing Accessibility permission) so we fail
		// fast instead of timing out after 30 seconds.
		const failureCheck = () =>
			checkDriverLogForFailure(this.#driverLogFile);
		const [appStatus, driverStatus] = await Promise.all([
			waitForHttp(
				`http://localhost:${webDriverPort}/status`,
				'WebDriverServer',
				30,
				failureCheck,
			),
			waitForHttp(
				`http://localhost:${nativeUIPort}/status`,
				'NativeUIServer',
				30,
				failureCheck,
			),
		]);

		// The driver knows the app's process id; keep it, so that an app which
		// goes away mid-run can be recognized as gone rather than merely
		// unreachable.
		if (typeof driverStatus?.targetPid === 'number') {
			this.#appPid = driverStatus.targetPid;
		}

		// Collect the app's log into a file of our own.  It is far too long to
		// put in the test output, so only its location is announced here, and a
		// failing run gets the tail of it printed.  Collecting starts at offset
		// zero, so the lines written during startup are included, late as we are
		// to it.
		if (typeof appStatus?.logFile === 'string') {
			const logDir = config?.outputDir ?? process.cwd();
			mkdirSync(logDir, { recursive: true });
			this.#appLogFile = join(logDir, 'coda-app.log');
			writeFileSync(this.#appLogFile, '');
			this.#appLogPoll = setInterval(() => {
				void this.#drainAppLog();
			}, 500);
			console.log(`Collecting the app's log in ${this.#appLogFile}`);
		} else {
			console.warn(
				'The app is not logging to a file, so no log will be collected.',
			);
		}

		console.log('coda-macos is ready, tests will now run');
	}

	async onComplete(exitCode?: number): Promise<void> {
		// Collect the rest of the app's log while it can still answer for it.
		if (this.#appLogPoll) {
			clearInterval(this.#appLogPoll);
			this.#appLogPoll = null;
			await this.#drainAppLog();
		}

		// A run that failed gets the end of the app's log printed, which is
		// where whatever went wrong shows up.
		if (exitCode !== 0 && this.#appLogFile !== null) {
			this.#printAppLogTail();
		}

		await this.#shutdownApp();

		if (this.#stopTail) {
			this.#stopTail();
			this.#stopTail = null;
		}

		if (this.#testDocDir) {
			try {
				rmSync(this.#testDocDir, { recursive: true, force: true });
				console.log(`Removed test doc dir: ${this.#testDocDir}`);
			} catch (e) {
				console.warn(
					`Failed to clean up test doc dir: ${(e as Error).message}`,
				);
			}
		}
	}
}
