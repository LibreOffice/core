/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Runs the specs in qt/test/specs, which CODA-Q and CODA-M also run, against CODA-W.
//
// As on CODA-Q, browser.webEngine is a chromedriver-like driver attached to the running app
// through its remote debugging port: msedgedriver, for WebView2. browser.native is coda-driver,
// over UI Automation, as macos/coda-driver is on CODA-M.

import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

import { CodaWindowsServiceLauncher } from './lib/coda-windows.service.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Spec paths go through glob, which takes a backslash as an escape character.
function forwardSlashes(path: string): string {
	return path.replace(/\\/g, '/');
}

function required(name: string, example: string): string {
	const value = process.env[name];
	if (!value)
		throw new Error(`${name} is not set. Set it to ${example}.`);
	return value;
}

const CODA_APP = required(
	'CODA_APP',
	'the CODA-W executable, e.g. ...\\windows\\coda\\x64\\Debug\\program\\<APP_NAME>.exe',
);
const MSEDGEDRIVER = required(
	'MSEDGEDRIVER',
	'msedgedriver.exe of the same major version as the WebView2 runtime',
);

const REMOTE_DEBUGGING_PORT = Number(process.env.REMOTE_DEBUGGING_PORT || 9222);
const EDGEDRIVER_PORT = Number(process.env.EDGEDRIVER_PORT || 9515);
const NATIVE_UI_PORT = Number(process.env.NATIVE_UI_PORT || 4723);

const QT_TEST_DIR = join(__dirname, '..', '..', '..', 'qt', 'test');
const QT_SPECS_DIR = join(QT_TEST_DIR, 'specs');
const FIXTURES_DIR = process.env.FIXTURES_DIR || join(QT_TEST_DIR, 'fixtures');

export const config = {
	runner: 'local',

	// Specs to run, in order, in the same WebDriver session.
	specs: [[
		join(QT_SPECS_DIR, 'harness.spec.ts'),
		join(QT_SPECS_DIR, 'document-lifecycle.spec.ts'),
		join(QT_SPECS_DIR, 'open-file.spec.ts'),
		join(QT_SPECS_DIR, 'multipage-backstage.spec.ts'),
		join(QT_SPECS_DIR, 'slide-copy-paste.spec.ts'),
	].map(forwardSlashes)],

	exclude: [],

	// One CODA-W instance, so one worker.
	maxInstances: 1,

	capabilities: {
		webEngine: {
			hostname: '127.0.0.1',
			port: EDGEDRIVER_PORT,
			capabilities: {
				browserName: 'webview2',
				'ms:edgeOptions': {
					// msedgedriver wants a browser binary that exists, also when it attaches to
					// one that is already running.
					binary: CODA_APP,
					debuggerAddress: `127.0.0.1:${REMOTE_DEBUGGING_PORT}`,
				},
			},
		},
		native: {
			hostname: '127.0.0.1',
			port: NATIVE_UI_PORT,
			// coda-driver ignores the capabilities. The name is not one that WebdriverIO knows,
			// because for 'chrome' the launcher downloads Chrome and chromedriver: it does not
			// see the hostname and port of a session in a multiremote config.
			capabilities: {
				browserName: 'coda-driver',
			},
		},
	},

	services: [
		[CodaWindowsServiceLauncher, {
			appPath: CODA_APP,
			driverProject: join(__dirname, '..', 'coda-driver'),
			driverExe: process.env.CODA_DRIVER_EXE,
			edgeDriver: MSEDGEDRIVER,
			remoteDebuggingPort: REMOTE_DEBUGGING_PORT,
			edgeDriverPort: EDGEDRIVER_PORT,
			nativeUIPort: NATIVE_UI_PORT,
			fixturesDir: FIXTURES_DIR,
		}],
	],

	logLevel: 'info',
	outputDir: './logs',
	bail: 1,
	// CODA-W is started once per run and quits when the worker ends, so a retried worker would
	// have no application to drive.
	specFileRetries: 0,
	waitforTimeout: 10000,
	waitforInterval: 500,
	connectionRetryTimeout: 30000,
	connectionRetryCount: 3,

	framework: 'mocha',
	reporters: ['spec'],

	mochaOpts: {
		ui: 'bdd',
		timeout: 150000,
	},

	// Runs in the worker process before any spec file.
	before: async function () {
		// The same command as in qt/test/wdio.conf.ts: poll a JS predicate inside the page.
		browser.webEngine.addCommand(
			'waitForCondition',
			async function (this: WebdriverIO.Browser, predicate: () => boolean, opts: { timeout?: number; interval?: number; timeoutMsg?: string } = {}) {
				return this.waitUntil(() => this.execute(predicate), {
					timeout: opts.timeout,
					interval: opts.interval,
					timeoutMsg: opts.timeoutMsg ?? 'waitForCondition: condition not met',
				});
			},
		);

		// The tour for first-time users opens on its own as a modal dialog, which blocks every
		// other dialog. CODA-W keeps the preferences in the local storage of its WebView2
		// profile, which is new for each run, so the tour is marked as seen there, from the
		// starter page, for each document type.
		await browser.webEngine.waitForCondition(
			() => typeof window.prefs !== 'undefined',
			{ timeout: 30000, timeoutMsg: 'The starter page did not load' },
		);
		await browser.webEngine.execute(() => {
			for (const docType of ['text', 'spreadsheet', 'presentation', 'drawing'])
				window.prefs.set(docType + '.onboardingTourSeen', 'true');
		});
	},

	after: async function () {
		// Ask CODA-W to close every document and quit.
		try {
			await browser.webEngine.execute(() => {
				window.postMobileMessage('EXIT_TEST');
			});
		} catch (e) {
			// The page or msedgedriver may already be gone. onComplete finds out from the exit
			// code.
		}
	},
};
