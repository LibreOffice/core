/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import { execSync } from 'child_process';
import { existsSync } from 'fs';

import { CodaQtServiceLauncher } from './lib/coda-qt.service.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const REMOTE_DEBUGGING_PORT = process.env.REMOTE_DEBUGGING_PORT || 9222;
const CODA_QT_BINARY =
	process.env.CODA_QT_BINARY || join(__dirname, '..', 'coda-qt');
const WEBENGINE_DRIVER_PORT = process.env.WEBENGINE_DRIVER_PORT || 9515;
const AT_SPI_PORT = process.env.AT_SPI_PORT || 4723;

const AT_SPI_DRIVER_PATH = process.env.AT_SPI_DRIVER_PATH;
if (!AT_SPI_DRIVER_PATH) {
	throw new Error(
		'AT_SPI_DRIVER_PATH is not set.\n' +
			'Set it to the path of selenium-webdriver-at-spi.py, e.g.:\n' +
			'  export AT_SPI_DRIVER_PATH=/path/to/selenium-webdriver-at-spi/selenium-webdriver-at-spi.py',
	);
}

let QT_WEBENGINE_DRIVER: string;
try {
	const qtLibExecDir = execSync('qtpaths6 --query QT_INSTALL_LIBEXECS')
		.toString()
		.trim();
	QT_WEBENGINE_DRIVER = join(qtLibExecDir, 'webenginedriver');
	// In flatpak, the SDK qtpaths6 points to /usr but webenginedriver is
	// in /app (from the BaseApp). Fall back to the /app prefix path.
	if (!existsSync(QT_WEBENGINE_DRIVER)) {
		const appPath = '/app/lib/libexec/webenginedriver';
		if (!existsSync(appPath)) {
			throw new Error(
				`webenginedriver not found at ${QT_WEBENGINE_DRIVER} or ${appPath}`,
			);
		}
		QT_WEBENGINE_DRIVER = appPath;
	}
} catch (e) {
	console.error(
		'Failed to find Qt installation. Make sure qtpaths6 is in your PATH.',
	);
	throw e;
}

export const config = {
	runner: 'local',

	specs: [['./specs/**/*.spec.ts']],

	suites: {
		harness: ['./specs/harness.spec.ts'],
		functional: [[
			'./specs/document-lifecycle.spec.ts',
			'./specs/open-file.spec.ts',
		]],
	},

	exclude: [],

	// Only one worker - there is a single coda-qt instance, so specs must
	// run sequentially in the same WebDriver session.
	maxInstances: 1,

	// Multiremote: two drivers sharing a single worker.
	capabilities: {
		webEngine: {
			port: WEBENGINE_DRIVER_PORT,
			capabilities: {
				browserName: 'chrome',
				'goog:chromeOptions': {
					binary: CODA_QT_BINARY,
					debuggerAddress: `localhost:${REMOTE_DEBUGGING_PORT}`,
				},
				'wdio:chromedriverOptions': {
					binary: QT_WEBENGINE_DRIVER,
				},
			},
		},
		native: {
			port: AT_SPI_PORT,
			capabilities: {
				'appium:app': 'Root',
				'appium:timeouts': {
					implicit: 10000,
				},
			},
		},
	},

	services: [
		[CodaQtServiceLauncher, {
			atSpiDriverPath: AT_SPI_DRIVER_PATH,
			atSpiPort: AT_SPI_PORT,
			webEngineDriver: QT_WEBENGINE_DRIVER,
			webEngineDriverPort: WEBENGINE_DRIVER_PORT,
			codaQtBinary: CODA_QT_BINARY,
			remoteDebuggingPort: REMOTE_DEBUGGING_PORT,
			fixturesDir: join(__dirname, 'fixtures'),
		}],
	],

	logLevel: 'info',
	outputDir: './logs',
	bail: 1,
	specFileRetries: 1,
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
		// Convenience command: poll a JS predicate inside the WebView.
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

		// The AT-SPI driver does not return the expected handshake fields,
		// so WDIO assumes it is talking to a legacy driver and wraps typed
		// text in an array instead of sending a plain string. The underlying
		// send-keys command expects a plain string, so typing would fail.
		// This override bypasses that formatting step and sends the string
		// directly.
		browser.native.overwriteCommand(
			'addValue',
			async function (this: WebdriverIO.Element, _origFn, value) {
				await this.elementSendKeys(this.elementId, String(value));
			},
			true,
		);
	},

	after: async function () {
		// Ask the app to close the document and quit.
		try {
			await browser.webEngine.execute(() => {
				window.postMobileMessage('EXIT_TEST');
			});
		} catch (e) {
			// Page or webenginedriver may already be gone.
			// onComplete will detect that via the exit code.
		}
	},
};
