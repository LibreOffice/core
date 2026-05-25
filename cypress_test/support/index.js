/* -*- js-indent-level: 8 -*- */
/* global require cy Cypress beforeEach afterEach */

require('cypress-wait-until');
require('cypress-file-upload');
require('cypress-real-events');
import installLogsCollector from 'cypress-terminal-report/src/installLogsCollector';
const { addCompareSnapshotCommand } = require('cypress-visual-regression/dist/command');
const helper = require('../integration_tests/common/helper');

addCompareSnapshotCommand();

beforeEach(function() {
	cy.log('Starting test: ' + getFullTestName());
});

// This afterEach must be before installLogsCollector, otherwise the
// cypress-terminal-report afterEach gets called first, and so this log does
// not get printed
afterEach(function() {
	cy.log('Finishing test: ' + getFullTestName());

	// Enforce the drawSections-per-frame invariant. Only check on tests
	// that otherwise passed - a test that already failed should report
	// the original failure, not a secondary draw-counter complaint.
	// Skip when no active frame exists (tests that don't load a document
	// in an iframe, e.g. idle/ or lighthouse/).
	if (this.currentTest.state !== 'failed' && cy.cActiveFrame) {
		cy.getFrameWindow().then(function(win) {
			helper.assertDrawSectionsPerFrame(win);
		});
	}

	if (this.currentTest.state === 'failed') {
		// Report system load
		cy.exec('cat /proc/loadavg', { log: false, failOnNonZeroExit: false }).then((result) => {
			if (result.code === 0)
				Cypress.log({ name: 'loadavg', message: result.stdout });
		});
		// Dump app.Log on test failure for debugging
		cy.getFrameWindow().then((win) => {
			if (win && win.app && win.app.Log && win.app.Log._logs) {
				var logs = win.app.Log._logs;
				Cypress.log({name: 'app.Log', message: 'Dumping ' + logs.length + ' log entries'});
				for (var i = 0; i < logs.length; i++) {
					var entry = logs[i];
					Cypress.log({name: entry.direction, message: entry.msg});
				}
			}
			// Skip remaining tests in this spec once a test has definitively
			// failed, but only after all retries are exhausted.
			var retries = this.currentTest._retries || 0;
			var currentRetry = this.currentTest._currentRetry || 0;
			if (currentRetry >= retries) {
				Cypress.stop();
				return;
			}
		});
	}
});

installLogsCollector({
	// Filter xhr requests from log
	filterLog: function(log) {
		var type = log[0];
		return type !== 'cy:xhr';
	},
	// Filter assertion values when assertion passes
	processLog: function(log) {
		var type = log[0];
		var message = log[1];
		var severity = log[2];
		if (type == 'cy:command' && message.startsWith('assert') && severity !== 'error') {
			return [type,message.split('\n')[0],severity];
		} else {
			return log;
		}
	}
});

var COMMAND_DELAY = 1000;

// Ignore exceptions coming from nextcloud.
if (Cypress.env('INTEGRATION') === 'nextcloud') {
	Cypress.on('uncaught:exception', function() {
		return false;
	});
} else {
	Cypress.on('window:before:load', function(appWindow) {
		appWindow.addEventListener('error', function(event) {
			Cypress.log({ name:'error:',
				      message: (event.error.message ? event.error.message : 'no message')
				      + '\n' + (event.error.stack ? event.error.stack : 'no stack') });
		});

	});
}

Cypress.on('fail', function(error) {
	var message = '\n';
	message += 'Test failed: ' + getFullTestName() + '\n';
	message += '\n';
	message += error.message + '\n';
	message += '\n';
	if (error.codeFrame) {
		message += error.codeFrame.absoluteFile + ':' + error.codeFrame.line + ':' + error.codeFrame.column + '\n';
		message += error.codeFrame.frame;
	}
	Cypress.log({name: 'fail:', message: message});

	throw error;
});

if (Cypress.browser.isHeaded) {
	const runCommand = cy.queue.runCommand.bind(cy.queue);
	cy.queue.runCommand = function slowRunCommand(cmd) {
		if (cmd != 'get' && cmd != 'contains')
			return runCommand(cmd);
		else
			return Cypress.Promise.delay(COMMAND_DELAY).then(() => runCommand(cmd));
	};
}

// reduce poll interval when waiting.
Cypress.Commands.overwrite('waitUntil', function(originalFn, subject, checkFunction, originalOptions) {
	var options = originalOptions;
	if (!options)
		options = {};
	if (!options.interval)
		options.interval = 100; // ms
	if (!options.timeout)
		options.timeout = Cypress.config('defaultCommandTimeout');
	if (!options.verbose)
		options.verbose = true;
	return originalFn(subject, checkFunction, options);
});

/**
 * Set the current iFrame
 * Example: cy.cSetActiveFrame('#coolframe');
 */
Cypress.Commands.add('cSetActiveFrame', function(frameID) {
	Cypress.log();
	cy.cActiveFrame = frameID;
});

/**
 * Get the current iFrame body to be chained with other queries.
 * Example: cy.cframe().find('#my-item');
 * It is not necessary to chain .should('exist') after this.
 */
Cypress.Commands.add('cframe', function(frameID, options) {
	// Set frameID
	if (!frameID) {
		if (!cy.cActiveFrame) {
			throw new Error('getFrame: Active frame not set');
		}
		frameID = cy.cActiveFrame;
	}

	// Log
	if (options && options.log) {
		Cypress.log({message: frameID});
	}

	// Execute
	return cy.get(frameID, {log: false})
		.its('0.contentDocument', {log: false});
});

/**
 * Get the current iFrame window to be chained with other queries.
 */
Cypress.Commands.add('getFrameWindow', function(frameID) {
	// Set frameID
	if (!frameID) {
		if (!cy.cActiveFrame) {
			throw new Error('getFrame: Active frame not set');
		}
		frameID = cy.cActiveFrame;
	}

	// Log
	Cypress.log({message: frameID});

	// Execute
	return cy.get(frameID, {log: false})
		.its('0.contentWindow', {log: false});
});

/**
 * Find an element within the current iFrame
 * Note: Use cy.cframe().find() instead, which offers better logging on failure
 */
Cypress.Commands.add('cGet', function(selector, options) {
	if (options) {
		if (options.log != false) {
			Cypress.log();
		}
	} else {
		Cypress.log();
	}

	var optionsWithLogFalse;
	if (options) {
		optionsWithLogFalse = options;
		optionsWithLogFalse.log = false;
	} else {
		optionsWithLogFalse = {log: false};
	}

	if (selector) {
		return cy.get(cy.cActiveFrame, {log: false})
			.its('0.contentDocument', {log: false})
			.find(selector, optionsWithLogFalse);
	} else {
		return cy.get(cy.cActiveFrame, optionsWithLogFalse)
			.its('0.contentDocument', {log: false});
	}
});

function getFullTestName() {
	return Cypress.spec.relative + ' / ' + Cypress.currentTest.titlePath.join(' / ');
}
