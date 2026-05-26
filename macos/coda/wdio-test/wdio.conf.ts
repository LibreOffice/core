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

import { CodaMacOSServiceLauncher } from './lib/coda-macos.service.js';
import { findCodaApp } from './lib/xcode.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const WEBDRIVER_PORT = Number(process.env.WEBDRIVER_PORT || 4567);
const NATIVE_UI_PORT = Number(process.env.NATIVE_UI_PORT || 4568);

// Locate the Xcode Debug build by querying DerivedData (or take an
// override via CODA_APP).
const CODA_APP = process.env.CODA_APP
	|| findCodaApp(join(__dirname, '..', 'coda.xcodeproj'));

// Swift Package directory for coda-driver, the external helper that
// holds the Accessibility permission grant and drives the main app
// via cross-process AXUIElement / CGEvent.
const CODA_DRIVER_PATH = process.env.CODA_DRIVER_PATH
	|| join(__dirname, '..', '..', 'coda-driver');

// Shared test directory: contains both fixtures (simple.odt, scrolling.odt)
// and specs.  Both are referenced by name from the shared specs, so we
// use the Qt fixtures rather than the local codaUITests/TestDocuments.
const QT_TEST_DIR = join(__dirname, '..', '..', '..', 'qt', 'test');
const QT_SPECS_DIR = join(QT_TEST_DIR, 'specs');
const FIXTURES_DIR = process.env.FIXTURES_DIR
	|| join(QT_TEST_DIR, 'fixtures');

export const config = {
	runner: 'local',

	// Specs to run, in order, in the same WebDriver session.
	specs: [[
		join(QT_SPECS_DIR, 'harness.spec.ts'),
		join(QT_SPECS_DIR, 'document-lifecycle.spec.ts'),
		join(QT_SPECS_DIR, 'open-file.spec.ts'),
		join(QT_SPECS_DIR, 'multipage-backstage.spec.ts'),
	]],

	exclude: [],

	maxInstances: 1,

	// Multiremote, matching the Qt setup:
	//   webEngine -> WebDriverServer in the main app (drives the WKWebView)
	//   native    -> NativeUIServer in the external coda-driver (drives
	//                the main app via cross-process AX)
	capabilities: {
		webEngine: {
			port: WEBDRIVER_PORT,
			capabilities: {
				browserName: 'chrome',
			},
		},
		native: {
			port: NATIVE_UI_PORT,
			capabilities: {
				browserName: 'chrome',
			},
		},
	},

	logLevel: 'info',
	outputDir: './logs',
	bail: 1,
	waitforTimeout: 10000,
	waitforInterval: 500,
	connectionRetryTimeout: 30000,
	connectionRetryCount: 3,

	services: [
		[CodaMacOSServiceLauncher, {
			appPath: CODA_APP,
			driverPath: CODA_DRIVER_PATH,
			webDriverPort: WEBDRIVER_PORT,
			nativeUIPort: NATIVE_UI_PORT,
			fixturesDir: FIXTURES_DIR,
		}],
	],

	framework: 'mocha',
	reporters: ['spec'],

	mochaOpts: {
		ui: 'bdd',
		timeout: 150000,
	},

	before: async function () {
		// Add the same waitForCondition command as the Qt tests
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
	},
};
