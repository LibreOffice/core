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

		try {
			const body = await new Promise<string | null>((resolve, reject) => {
				const req = http.get(url, (res) => {
					if (res.statusCode !== 200) {
						resolve(null);
						res.resume();
						return;
					}
					let buf = '';
					res.on('data', (chunk) => (buf += chunk));
					res.on('end', () => resolve(buf));
				});
				req.on('error', reject);
				req.setTimeout(1000, () => req.destroy(new Error('timeout')));
			});
			if (body !== null) {
				console.log(`${label} is ready`);
				try {
					const parsed = JSON.parse(body);
					return parsed?.value && typeof parsed.value === 'object'
						? (parsed.value as Record<string, unknown>)
						: null;
				} catch {
					return null;
				}
			}
		} catch {
			// Not ready yet
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
	#options: CodaMacOSServiceOptions;

	constructor(options: CodaMacOSServiceOptions) {
		this.#options = options;
	}

	async onPrepare(): Promise<void> {
		const { appPath, driverPath, webDriverPort, nativeUIPort, fixturesDir } = this.#options;

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
		await Promise.all([
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

		console.log('coda-macos is ready, tests will now run');
	}

	async onComplete(): Promise<void> {
		// Quit the main app.  The driver observes app termination via
		// NSWorkspace.didTerminateApplicationNotification and exits.
		try {
			execSync(
				'osascript -e \'tell application "Collabora Office" to quit\'',
				{ timeout: 5000 },
			);
		} catch {
			// App may already be gone
		}

		// Give the driver time to notice & exit.  We do not have its
		// pid (launched via `open`), but a quit AppleScript directed
		// at coda-driver would also work if needed.
		await sleep(1500);

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
