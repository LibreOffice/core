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

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const WEBDRIVER_PORT = Number(process.env.WEBDRIVER_PORT || 4567);

// Default to the Xcode Debug build location
const CODA_APP = process.env.CODA_APP
	|| join(__dirname, '..', '..', '..', 'tmp', 'DerivedData',
		'Build', 'Products', 'Debug', 'Collabora Office.app');

// Test documents
const FIXTURES_DIR = process.env.FIXTURES_DIR
	|| join(__dirname, '..', 'codaUITests', 'TestDocuments');

// Shared specs from the Qt test directory
const QT_SPECS_DIR = join(__dirname, '..', '..', '..', 'qt', 'test', 'specs');

export const config = {
	runner: 'local',

	// Run the harness spec - the most basic shared test
	specs: [[join(QT_SPECS_DIR, 'harness.spec.ts')]],

	exclude: [],

	maxInstances: 1,

	// Multiremote so the shared Qt specs can use browser.webEngine.
	// The webEngine driver points directly at our WebDriverServer.
	// The native driver also points at our server; native-only tests
	// (AT-SPI) will fail but are in separate describe blocks.
	capabilities: {
		webEngine: {
			port: WEBDRIVER_PORT,
			capabilities: {
				browserName: 'chrome',
			},
		},
		native: {
			port: WEBDRIVER_PORT,
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
			webDriverPort: WEBDRIVER_PORT,
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
