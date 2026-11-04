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
import {
	copyFileSync,
	cpSync,
	existsSync,
	mkdirSync,
	mkdtempSync,
	readFileSync,
	rmSync,
	writeFileSync,
} from 'fs';
import { homedir, tmpdir } from 'os';
import { basename, join } from 'path';

import { killProcess, pipeOutput, waitForHttp } from './processes.js';

interface CodaQtServiceOptions {
	atSpiDriverPath: string;
	atSpiPort: number | string;
	webEngineDriver: string;
	webEngineDriverPort: number | string;
	codaQtBinary: string;
	// The application name coda-qt was configured with. Its configuration directory is
	// named after it.
	appName: string;
	remoteDebuggingPort: number | string;
	fixturesDir: string;
}

/**
 * Read the basename of the developer's XDG documents directory from
 * the host's user-dirs.dirs (the same file Qt's
 * QStandardPaths::DocumentsLocation looks up).  On a non-English
 * locale this returns the translated name, e.g. "Dokumenty".
 * On a missing or DOCUMENTS-less config it falls back to "Documents".
 *
 * The test setup uses this basename when creating its own documents
 * directory under the test HOME, so that when coda-qt runs with that
 * HOME and reads the same user-dirs.dirs file, Qt expands the $HOME
 * placeholder to our test dir and uses the directory we created.
 */
function hostUserDirsPath(): string {
	const configHome = process.env.XDG_CONFIG_HOME || join(homedir(), '.config');
	return join(configHome, 'user-dirs.dirs');
}

function getXdgDocumentsBasename(): string {
	try {
		const content = readFileSync(hostUserDirsPath(), 'utf8');
		const match = content.match(/^\s*XDG_DOCUMENTS_DIR="([^"]+)"\s*$/m);
		if (match) {
			const name = basename(match[1]);
			if (name && name !== '.' && name !== '$HOME') return name;
		}
	} catch {
		// File missing or unreadable - fall through.
	}
	return 'Documents';
}

export class CodaQtServiceLauncher {
	#codaQtProcess: ChildProcess | null = null;
	#atSpiServerProcess: ChildProcess | null = null;
	#webEngineDriverProcess: ChildProcess | null = null;
	#testHomeDir: string | null = null;
	#options: CodaQtServiceOptions;

	constructor(options: CodaQtServiceOptions) {
		this.#options = options;
	}

	async onPrepare(): Promise<void> {
		const {
			atSpiDriverPath,
			atSpiPort,
			webEngineDriver,
			webEngineDriverPort,
			codaQtBinary,
			appName,
			remoteDebuggingPort,
			fixturesDir,
		} = this.#options;

		console.log('Starting AT-SPI Flask server...');
		this.#atSpiServerProcess = spawn(
			'flask',
			['run', '--port', atSpiPort.toString(), '--no-reload'],
			{
				env: { ...process.env, FLASK_APP: atSpiDriverPath },
				stdio: ['ignore', 'pipe', 'pipe'],
			},
		);
		pipeOutput(this.#atSpiServerProcess, 'at-spi');

		console.log('Starting WebEngineDriver...');
		this.#webEngineDriverProcess = spawn(
			webEngineDriver,
			[`--port=${webEngineDriverPort}`, '--url-base=/'],
			{ stdio: ['ignore', 'pipe', 'pipe'] },
		);
		pipeOutput(this.#webEngineDriverProcess, 'webenginedriver');

		await Promise.all([
			waitForHttp(
				`http://localhost:${atSpiPort}/status`,
				'AT-SPI server',
			),
			waitForHttp(
				`http://localhost:${webEngineDriverPort}/status`,
				'WebEngineDriver',
			),
		]);

		this.#testHomeDir = mkdtempSync(join(tmpdir(), 'coda-qt-test-'));
		// Use the developer's locale-specific name (e.g. "Dokumenty")
		// rather than hardcoding "Documents"
		const documentsDir = join(
			this.#testHomeDir,
			getXdgDocumentsBasename(),
		);
		mkdirSync(documentsDir);
		cpSync(fixturesDir, documentsDir, { recursive: true });
		process.env.CODA_TEST_DOCUMENTS_DIR = documentsDir;

		// coda-qt reads its configuration from the test HOME, with the host's
		// user-dirs.dirs so that the documents directory above is the one it uses.
		const configHome = join(this.#testHomeDir, '.config');
		mkdirSync(join(configHome, appName), { recursive: true });
		if (existsSync(hostUserDirsPath()))
			copyFileSync(hostUserDirsPath(), join(configHome, 'user-dirs.dirs'));
		// The tour for first-time users opens on its own as a modal dialog, which
		// blocks every other dialog, so it is marked as seen for each document type.
		const prefs: Record<string, string> = {};
		for (const docType of ['text', 'spreadsheet', 'presentation', 'drawing'])
			prefs[docType + '.onboardingTourSeen'] = 'true';
		writeFileSync(
			join(configHome, appName, 'coda-prefs.json'),
			JSON.stringify(prefs),
		);
		process.env.CODA_PLATFORM = 'qt';

		console.log('Starting coda-qt...');
		this.#codaQtProcess = spawn(codaQtBinary, [], {
			env: {
				...process.env,
				HOME: this.#testHomeDir,
				XDG_CONFIG_HOME: configHome,
				QTWEBENGINE_REMOTE_DEBUGGING: remoteDebuggingPort.toString(),
				QT_ACCESSIBILITY: '1',
				QT_LINUX_ACCESSIBILITY_ALWAYS_ON: '1',
				// enforce qt dialogs, instead of native ones for consistency in tests
				QT_QPA_PLATFORMTHEME: 'generic',
			},
			stdio: ['ignore', 'pipe', 'pipe'],
		});
		pipeOutput(this.#codaQtProcess, 'coda-qt');

		await waitForHttp(
			`http://localhost:${remoteDebuggingPort}/json/version`,
			'Remote debugging',
		);

		console.log('All services ready, tests will now run');
	}

	async onComplete(): Promise<void> {
		// Give coda-qt a moment to exit after the EXIT_TEST message.
		if (
			this.#codaQtProcess &&
			this.#codaQtProcess.exitCode === null &&
			!this.#codaQtProcess.signalCode
		) {
			await new Promise<void>((resolve) => {
				const timeout = setTimeout(resolve, 20000);
				this.#codaQtProcess!.once('exit', () => {
					clearTimeout(timeout);
					resolve();
				});
			});
		}

		let error: string | null = null;
		const proc = this.#codaQtProcess;
		if (proc) {
			const { exitCode, signalCode } = proc;
			if (exitCode === null && !signalCode) {
				console.error(
					'coda-qt did not exit within timeout after EXIT_TEST',
				);
				killProcess(proc, 'coda-qt');
				error = 'coda-qt did not exit gracefully';
			} else if (signalCode) {
				error = `coda-qt crashed during tests (signal=${signalCode})`;
			} else if (exitCode !== 0) {
				error = `coda-qt crashed during tests (code=${exitCode})`;
			} else {
				console.log('coda-qt exited gracefully');
			}
		}

		killProcess(this.#atSpiServerProcess, 'at-spi');
		killProcess(this.#webEngineDriverProcess, 'webenginedriver');

		if (this.#testHomeDir) {
			try {
				rmSync(this.#testHomeDir, { recursive: true, force: true });
				console.log(`Removed test home dir: ${this.#testHomeDir}`);
			} catch (e) {
				console.warn(
					`Failed to clean up test home dir ${this.#testHomeDir}: ${(e as Error).message}`,
				);
			}
		}

		if (error) throw new Error(error);
	}
}
