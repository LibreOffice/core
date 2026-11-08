/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Starts CODA-W and the two drivers before the specs run, and checks how CODA-W exited after.
//
// CODA-W runs with the test environment variables: its own data folder and documents folder in
// a temporary directory, and a remote debugging port. msedgedriver attaches to the WebView2
// browser through that port. coda-driver attaches to the CODA-W process through UI Automation.

import { ChildProcess, execFileSync, spawn, spawnSync } from 'child_process';
import {
	cpSync,
	existsSync,
	mkdirSync,
	mkdtempSync,
	renameSync,
	rmSync,
	writeFileSync,
} from 'fs';
import http from 'http';
import net from 'net';
import { tmpdir } from 'os';
import { join } from 'path';
import { SevereServiceError } from 'webdriverio';

import {
	killProcess,
	pipeOutput,
	waitForHttp,
} from '../../../../qt/test/lib/processes.js';

interface CodaWindowsServiceOptions {
	// The CODA-W executable.
	appPath: string;
	// The directory of the coda-driver project, built before the run.
	driverProject: string;
	// A coda-driver executable to use as it is, instead of building the project.
	driverExe?: string;
	// msedgedriver.exe, of the same major version as the WebView2 runtime. When it is not given,
	// the msedgedriver for the WebView2 runtime is downloaded into edgeDriverCache.
	edgeDriver?: string;
	// The folder of the downloaded msedgedriver executables, one subfolder per version.
	edgeDriverCache: string;
	remoteDebuggingPort: number;
	edgeDriverPort: number;
	nativeUIPort: number;
	fixturesDir: string;
}

function getJson(url: string): Promise<any> {
	return new Promise((resolve, reject) => {
		const req = http.get(url, (res) => {
			let body = '';
			res.on('data', (chunk) => (body += chunk));
			res.on('end', () => {
				try {
					resolve(JSON.parse(body));
				} catch (e) {
					reject(e);
				}
			});
		});
		req.on('error', reject);
		req.setTimeout(2000, () => req.destroy(new Error('timeout')));
	});
}

// The first number of a version string, for example 130 for "Edg/130.0.2849.68" or for
// "Microsoft Edge WebDriver 130.0.2849.68 (...)".
function majorVersion(text: string): string | null {
	const match = text.match(/(\d+)\.\d+\.\d+\.\d+/);
	return match ? match[1] : null;
}

// The full version in a version string, for example 130.0.2849.68 for "Edg/130.0.2849.68".
function fullVersion(text: string): string | null {
	const match = text.match(/\d+\.\d+\.\d+\.\d+/);
	return match ? match[0] : null;
}

const EDGEDRIVER_SITE = 'https://msedgedriver.microsoft.com';

// The architecture in the names of the msedgedriver downloads, by the architecture of Node.js. A
// driver of another architecture than the WebView2 runtime works too, because it talks to the
// runtime over the remote debugging port.
const EDGEDRIVER_ARCHITECTURES: { [arch: string]: string } = {
	arm64: 'arm64',
	x64: 'win64',
	ia32: 'win32',
};

// The body of a download, or null when the site has no such file.
async function download(url: string): Promise<Buffer | null> {
	const response = await fetch(url, { signal: AbortSignal.timeout(120000) });
	if (response.status === 404)
		return null;
	if (!response.ok)
		throw new Error(`Downloading ${url} failed: HTTP ${response.status}`);
	return Buffer.from(await response.arrayBuffer());
}

// A text file of the download site. The LATEST_RELEASE files are in UTF-16 with a byte order mark.
function decodeText(bytes: Buffer): string {
	const encoding = bytes[0] === 0xff && bytes[1] === 0xfe ? 'utf-16le' : 'utf-8';
	return new TextDecoder(encoding).decode(bytes).trim();
}

// Unpack msedgedriver.exe of one version from its zip into the cache, and return its path. The
// tar of Windows reads zip files, and a GNU tar earlier on the PATH, as from Git, does not.
function unpackEdgeDriver(zip: Buffer, cacheDir: string, version: string): string {
	const finalDir = join(cacheDir, version);
	const unpackDir = `${finalDir}.partial-${process.pid}`;
	mkdirSync(unpackDir, { recursive: true });
	try {
		const zipPath = join(unpackDir, 'edgedriver.zip');
		writeFileSync(zipPath, zip);
		const tar = join(process.env.SystemRoot || 'C:\\Windows', 'System32', 'tar.exe');
		execFileSync(tar, ['-xf', zipPath, '-C', unpackDir, 'msedgedriver.exe']);
		rmSync(zipPath);
		// The folder gets its final name only when it is complete, so an interrupted download
		// leaves no driver in the cache.
		renameSync(unpackDir, finalDir);
	} finally {
		rmSync(unpackDir, { recursive: true, force: true });
	}
	return join(finalDir, 'msedgedriver.exe');
}

// The msedgedriver for a WebView2 runtime version: the driver of the same version, or, when
// Microsoft has published none for it, the latest driver of the same major version. A driver
// is downloaded once, and taken from the cache after that.
async function edgeDriverFor(runtimeVersion: string, cacheDir: string): Promise<string> {
	const arch = EDGEDRIVER_ARCHITECTURES[process.arch];
	if (!arch)
		throw new Error(`There is no msedgedriver download for ${process.arch}`);

	const cached = (version: string) => {
		const exe = join(cacheDir, version, 'msedgedriver.exe');
		return existsSync(exe) ? exe : null;
	};
	const zipUrl = (version: string) => `${EDGEDRIVER_SITE}/${version}/edgedriver_${arch}.zip`;

	let version = runtimeVersion;
	let exe = cached(version);
	if (exe)
		return exe;

	console.log(`Downloading msedgedriver ${version}...`);
	let zip = await download(zipUrl(version));
	if (!zip) {
		const major = version.split('.')[0];
		const latestUrl = `${EDGEDRIVER_SITE}/LATEST_RELEASE_${major}_WINDOWS`;
		const latest = await download(latestUrl);
		if (!latest)
			throw new Error(`There is no msedgedriver ${major}.x at ${EDGEDRIVER_SITE}`);
		version = decodeText(latest);
		exe = cached(version);
		if (exe)
			return exe;
		console.log(`There is no msedgedriver ${runtimeVersion}, downloading ${version}...`);
		zip = await download(zipUrl(version));
		if (!zip)
			throw new Error(`${latestUrl} names ${version}, but ${zipUrl(version)} is missing`);
	}
	return unpackEdgeDriver(zip, cacheDir, version);
}

// True when something accepts connections on the port.
function portInUse(port: number): Promise<boolean> {
	return new Promise((resolve) => {
		const socket = net.connect({ host: '127.0.0.1', port });
		socket.setTimeout(1000);
		socket.once('connect', () => {
			socket.destroy();
			resolve(true);
		});
		socket.once('timeout', () => {
			socket.destroy();
			resolve(false);
		});
		socket.once('error', () => resolve(false));
	});
}

// A process left from an interrupted run, such as a CODA-W that Ctrl-C did not reach, would
// answer on a port in place of the process that this run starts, so the run would drive the
// wrong one.
async function checkPortsFree(ports: { port: number; use: string }[]): Promise<void> {
	const taken: string[] = [];
	for (const { port, use } of ports)
		if (await portInUse(port))
			taken.push(`${port} (${use})`);
	if (taken.length > 0)
		throw new Error(
			`Something already listens on port ${taken.join(', ')}. ` +
				'Close what is left from an earlier run, or choose other ports.',
		);
}

function buildDriver(project: string): string {
	console.log('Building coda-driver...');
	const result = spawnSync(
		'dotnet',
		['build', project, '--configuration', 'Debug', '--nologo', '--verbosity', 'quiet'],
		{ stdio: 'inherit' },
	);
	if (result.status !== 0)
		throw new Error(`Building coda-driver in ${project} failed`);
	const exe = join(project, 'bin', 'Debug', 'net8.0-windows', 'coda-driver.exe');
	if (!existsSync(exe))
		throw new Error(`coda-driver was built, but ${exe} is missing`);
	return exe;
}

export class CodaWindowsServiceLauncher {
	#appProcess: ChildProcess | null = null;
	#edgeDriverProcess: ChildProcess | null = null;
	#nativeDriverProcess: ChildProcess | null = null;
	#testDir: string | null = null;
	#options: CodaWindowsServiceOptions;

	constructor(options: CodaWindowsServiceOptions) {
		this.#options = options;
	}

	// WebdriverIO only logs an ordinary error from onPrepare to logs/wdio.log and runs the specs
	// anyway, with no application or drivers to talk to. A SevereServiceError stops the run and
	// prints the message.
	async onPrepare(): Promise<void> {
		try {
			await this.#prepare();
		} catch (e) {
			this.#stopProcesses();
			this.#removeTestDir();
			throw new SevereServiceError((e as Error).message);
		}
	}

	async #prepare(): Promise<void> {
		const {
			appPath,
			driverProject,
			driverExe,
			edgeDriver: givenEdgeDriver,
			edgeDriverCache,
			remoteDebuggingPort,
			edgeDriverPort,
			nativeUIPort,
			fixturesDir,
		} = this.#options;

		await checkPortsFree([
			{ port: remoteDebuggingPort, use: 'REMOTE_DEBUGGING_PORT' },
			{ port: edgeDriverPort, use: 'EDGEDRIVER_PORT' },
			{ port: nativeUIPort, use: 'NATIVE_UI_PORT' },
		]);

		const nativeDriver = driverExe || buildDriver(driverProject);

		// The fixtures are opened from the documents folder, and new documents are made there.
		this.#testDir = mkdtempSync(join(tmpdir(), 'coda-w-test-'));
		const documentsDir = join(this.#testDir, 'Documents');
		mkdirSync(documentsDir);
		cpSync(fixturesDir, documentsDir, { recursive: true });
		process.env.CODA_TEST_DOCUMENTS_DIR = documentsDir;
		process.env.CODA_PLATFORM = 'windows';

		console.log('Starting CODA-W...');
		this.#appProcess = spawn(appPath, [], {
			env: {
				...process.env,
				CODA_TEST_REMOTE_DEBUGGING_PORT: remoteDebuggingPort.toString(),
				CODA_TEST_DATA_DIR: join(this.#testDir, 'data'),
				CODA_TEST_DOCUMENTS_DIR: documentsDir,
			},
			stdio: ['ignore', 'pipe', 'pipe'],
		});
		pipeOutput(this.#appProcess, 'coda-w');

		const versionUrl = `http://127.0.0.1:${remoteDebuggingPort}/json/version`;
		await waitForHttp(versionUrl, 'Remote debugging');

		// msedgedriver attaches only to a WebView2 runtime of its own major version. The runtime
		// on the machine updates itself, about every four weeks to a new major version.
		const browser: string = (await getJson(versionUrl)).Browser || '';
		const runtimeVersion = fullVersion(browser);
		if (!runtimeVersion)
			throw new Error(`The WebView2 runtime gave no version that can be read: "${browser}"`);
		const edgeDriver =
			givenEdgeDriver || (await edgeDriverFor(runtimeVersion, edgeDriverCache));
		const driverVersion = execFileSync(edgeDriver, ['--version']).toString();
		if (majorVersion(runtimeVersion) !== majorVersion(driverVersion))
			throw new Error(
				`msedgedriver is "${driverVersion.trim()}", but the WebView2 runtime is ` +
					`"${browser}". Get msedgedriver ${majorVersion(runtimeVersion)}.x and set ` +
					'MSEDGEDRIVER to it, or leave MSEDGEDRIVER unset to have it downloaded.',
			);

		console.log('Starting msedgedriver...');
		this.#edgeDriverProcess = spawn(edgeDriver, [`--port=${edgeDriverPort}`], {
			stdio: ['ignore', 'pipe', 'pipe'],
		});
		pipeOutput(this.#edgeDriverProcess, 'msedgedriver');

		console.log('Starting coda-driver...');
		this.#nativeDriverProcess = spawn(
			nativeDriver,
			['--target-pid', String(this.#appProcess.pid), '--port', String(nativeUIPort)],
			{ stdio: ['ignore', 'pipe', 'pipe'] },
		);
		pipeOutput(this.#nativeDriverProcess, 'coda-driver');

		await Promise.all([
			waitForHttp(`http://127.0.0.1:${edgeDriverPort}/status`, 'msedgedriver'),
			waitForHttp(`http://127.0.0.1:${nativeUIPort}/status`, 'coda-driver'),
		]);

		console.log('All services ready, tests will now run');
	}

	async onComplete(): Promise<void> {
		// Give CODA-W a moment to exit after the EXIT_TEST message.
		const proc = this.#appProcess;
		if (proc && proc.exitCode === null && !proc.signalCode) {
			await new Promise<void>((resolve) => {
				const timeout = setTimeout(resolve, 20000);
				proc.once('exit', () => {
					clearTimeout(timeout);
					resolve();
				});
			});
		}

		let error: string | null = null;
		if (proc) {
			const { exitCode, signalCode } = proc;
			if (exitCode === null && !signalCode) {
				console.error('CODA-W did not exit within timeout after EXIT_TEST');
				killProcess(proc, 'coda-w');
				error = 'CODA-W did not exit gracefully';
			} else if (signalCode) {
				error = `CODA-W was killed during tests (signal=${signalCode})`;
			} else if (exitCode !== 0) {
				error = `CODA-W crashed during tests (code=${exitCode})`;
			} else {
				console.log('CODA-W exited gracefully');
			}
		}

		killProcess(this.#edgeDriverProcess, 'msedgedriver');
		killProcess(this.#nativeDriverProcess, 'coda-driver');
		this.#removeTestDir();

		if (error) throw new SevereServiceError(error);
	}

	#stopProcesses(): void {
		killProcess(this.#appProcess, 'coda-w');
		killProcess(this.#edgeDriverProcess, 'msedgedriver');
		killProcess(this.#nativeDriverProcess, 'coda-driver');
	}

	#removeTestDir(): void {
		if (!this.#testDir)
			return;
		try {
			// The WebView2 browser processes exit a moment after CODA-W, and keep files in the
			// profile open until then. rmSync retries while they do, for up to about ten seconds.
			rmSync(this.#testDir, { recursive: true, force: true, maxRetries: 9, retryDelay: 200 });
			console.log(`Removed test directory: ${this.#testDir}`);
		} catch (e) {
			console.warn(
				`Failed to clean up test directory ${this.#testDir}: ${(e as Error).message}`,
			);
		}
		this.#testDir = null;
	}
}
