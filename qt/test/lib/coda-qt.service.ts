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
import { mkdirSync, mkdtempSync, readFileSync, rmSync, cpSync } from 'fs';
import http from 'http';
import { homedir, tmpdir } from 'os';
import { basename, join } from 'path';
import { promisify } from 'util';

const sleep = promisify(setTimeout);

interface CodaQtServiceOptions {
	atSpiDriverPath: string;
	atSpiPort: number | string;
	webEngineDriver: string;
	webEngineDriverPort: number | string;
	codaQtBinary: string;
	remoteDebuggingPort: number | string;
	fixturesDir: string;
}

function pipeOutput(proc: ChildProcess, label: string): void {
	proc.stdout!.on('data', (d: Buffer) =>
		console.log(`[${label}]: ${d.toString().trim()}`),
	);
	proc.stderr!.on('data', (d: Buffer) => {
		const msg = d.toString().trim();
		if (msg) console.log(`[${label}]: ${msg}`);
	});
	proc.on('exit', (c, s) =>
		console.log(`${label} exited (code=${c}, signal=${s})`),
	);
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
		} catch (e) {
			// Not ready yet
		}
		await sleep(1000);
	}
	throw new Error(
		`${label} not available at ${url} after ${maxAttempts} attempts`,
	);
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
function getXdgDocumentsBasename(): string {
	const configHome = process.env.XDG_CONFIG_HOME || join(homedir(), '.config');
	const userDirsPath = join(configHome, 'user-dirs.dirs');
	try {
		const content = readFileSync(userDirsPath, 'utf8');
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

function killProcess(proc: ChildProcess | null, name: string): void {
	if (proc && proc.pid && !proc.killed) {
		console.log(`Sending SIGTERM to ${name}`);
		proc.kill('SIGTERM');
		const timer = setTimeout(() => {
			if (proc.exitCode === null) {
				console.log(`Forcing SIGKILL on ${name}`);
				proc.kill('SIGKILL');
			}
		}, 2000);
		proc.once('exit', () => clearTimeout(timer));
	}
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
		process.env.CODA_PLATFORM = 'qt';

		console.log('Starting coda-qt...');
		this.#codaQtProcess = spawn(codaQtBinary, [], {
			env: {
				...process.env,
				HOME: this.#testHomeDir,
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
				const timeout = setTimeout(resolve, 5000);
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
