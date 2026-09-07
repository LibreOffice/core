/* -*- typescript-indent-level: 8 -*- */
/* global __dirname */
import { defineConfig } from 'cypress';
import plugin from './plugins/index.js';
import path from 'path';
import process from 'process';
import installLogsPrinter from 'cypress-terminal-report/src/installLogsPrinter';
import { configureVisualRegression } from 'cypress-visual-regression';

const SNAPSHOTS = path.join(__dirname, 'integration_tests/snapshots');

export default defineConfig({
	video: false,
	defaultCommandTimeout: 60000,
	modifyObstructiveCode: false,
	fixturesFolder: 'data',
	chromeWebSecurity: false,
	screenshotOnRunFailure: true,
	// Absolute, so the snapshots are found whatever directory cypress was
	// started from: out of tree the builddir is the working directory while
	// the baselines are versioned next to the specs, in the source tree.
	screenshotsFolder: path.join(SNAPSHOTS, 'actual'),
	// Parallel specs each run their own cypress process against this one
	// screenshotsFolder. The default per-run asset trashing would wipe a
	// concurrent spec's in-flight screenshot before its comparison reads it.
	// The Makefile clears the folder once before the parallel run instead.
	trashAssetsBeforeRuns: false,
	logServerResponse: false,
	env: {
		USER_INTERFACE: process.env.USER_INTERFACE,
		WSD_VERSION_HASH: process.env.COOLWSD_VERSION_HASH,
		// Absolute path of the presets the dev WOPI server offers, in the builddir.
		PRESETS_ROOT: process.env.PRESETS_ROOT,
		visualRegressionType: 'regression',
		visualRegressionBaseDirectory: path.join(SNAPSHOTS, 'base'),
	},
	retries: {
		runMode: 1,
		openMode: 0,
	},
	e2e: {
		baseUrl: 'http://' + process.env.COOLWSD_SERVER + ':' + process.env.FREE_PORT,
		setupNodeEvents(on, config) {
			installLogsPrinter(on, {
				printLogsToConsole: 'onFail', // 'always', 'onFail', 'never'
			});
			plugin(on, config);
			configureVisualRegression(on);
		},
		specPattern: 'integration_tests/**/*_spec.js',
	},
});
