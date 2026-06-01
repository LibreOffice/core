/* -*- typescript-indent-level: 8 -*- */
import { defineConfig } from 'cypress';
import plugin from './plugins/index.js';
import process from 'process';
import installLogsPrinter from 'cypress-terminal-report/src/installLogsPrinter';
import { configureVisualRegression } from 'cypress-visual-regression';

export default defineConfig({
	video: false,
	defaultCommandTimeout: 60000,
	modifyObstructiveCode: false,
	fixturesFolder: 'data',
	chromeWebSecurity: false,
	screenshotOnRunFailure: true,
	screenshotsFolder: './integration_tests/snapshots/actual',
	// Parallel specs each run their own cypress process against this one
	// screenshotsFolder. The default per-run asset trashing would wipe a
	// concurrent spec's in-flight screenshot before its comparison reads it.
	// The Makefile clears the folder once before the parallel run instead.
	trashAssetsBeforeRuns: false,
	logServerResponse: false,
	env: {
		USER_INTERFACE: process.env.USER_INTERFACE,
		WSD_VERSION_HASH: process.env.COOLWSD_VERSION_HASH,
		visualRegressionType: 'regression',
		visualRegressionBaseDirectory: './integration_tests/snapshots/base',
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
