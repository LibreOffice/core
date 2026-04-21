/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { ChildProcess, spawn } from 'child_process';
import { cpSync, mkdirSync, mkdtempSync, rmSync } from 'fs';
import http from 'http';
import { tmpdir } from 'os';
import { join } from 'path';
import { promisify } from 'util';

const sleep = promisify(setTimeout);

interface CodaMacOSServiceOptions {
	appPath: string;
	webDriverPort: number | string;
	fixturesDir: string;
}

async function waitForHttp(
	url: string,
	label: string,
	maxAttempts = 30,
): Promise<void> {
	for (let i = 0; i < maxAttempts; i++) {
		try {
			const ok = await new Promise((resolve, reject) => {
				const req = http.get(url, (res) => resolve(res.statusCode === 200));
				req.on('error', reject);
				req.setTimeout(1000, () => req.destroy(new Error('timeout')));
			});
			if (ok) {
				console.log(`${label} is ready`);
				return;
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

export class CodaMacOSServiceLauncher {
	#appProcess: ChildProcess | null = null;
	#testDocDir: string | null = null;
	#options: CodaMacOSServiceOptions;

	constructor(options: CodaMacOSServiceOptions) {
		this.#options = options;
	}

	async onPrepare(): Promise<void> {
		const { appPath, webDriverPort, fixturesDir } = this.#options;

		// Copy fixtures to a temp directory so the app can write to them
		this.#testDocDir = mkdtempSync(join(tmpdir(), 'coda-macos-test-'));
		mkdirSync(join(this.#testDocDir, 'Documents'));
		cpSync(fixturesDir, join(this.#testDocDir, 'Documents'), {
			recursive: true,
		});
		process.env.CODA_MACOS_TEST_DOCUMENTS_DIR = join(
			this.#testDocDir,
			'Documents',
		);

		// Pick a test file to open initially
		const testFile = join(this.#testDocDir, 'Documents', 'hello.odt');

		console.log('Starting coda-macos...');
		this.#appProcess = spawn('open', [
			'-a', appPath,
			'--args',
			'--uitesting',
			`--testDriverPort=${webDriverPort}`,
			'-ApplePersistenceIgnoreState', 'YES',
			testFile,
		], {
			stdio: ['ignore', 'pipe', 'pipe'],
		});

		this.#appProcess.stdout?.on('data', (d: Buffer) =>
			console.log(`[coda-macos]: ${d.toString().trim()}`),
		);
		this.#appProcess.stderr?.on('data', (d: Buffer) => {
			const msg = d.toString().trim();
			if (msg) console.log(`[coda-macos]: ${msg}`);
		});

		await waitForHttp(
			`http://localhost:${webDriverPort}/status`,
			'WebDriverServer',
		);

		console.log('coda-macos is ready, tests will now run');
	}

	async onComplete(): Promise<void> {
		// Send quit via AppleScript since we launched with `open`
		try {
			const { execSync } = await import('child_process');
			execSync('osascript -e \'tell application "Collabora Office" to quit\'', {
				timeout: 5000,
			});
		} catch {
			// App may already be gone
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
