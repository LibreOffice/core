/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('Dispatcher ext: routing', function () {

	// Enough of app.map for the constructor and the "is a dialog already open"
	// guard at the top of dispatch() to run without touching anything else;
	// _extensions is what the ext: branch actually reads.
	function newDispatcher(): Dispatcher {
		(window as any).mode = { isSmallScreenDevice: () => false };
		app.map = {
			dialog: { hasOpenedDialog: () => false, blinkOpenDialog: () => {} },
			_extensions: {} as { [id: string]: any },
		} as any;
		return new Dispatcher('none');
	}

	it('routes ext:<id>:<command> to that extension\'s invokeCommand', function () {
		const dispatcher = newDispatcher();
		const calls: string[] = [];
		app.map._extensions['demo'] = {
			invokeCommand: (commandId: string) => calls.push(commandId),
		};

		dispatcher.dispatch('ext:demo:insertDate');

		nodeassert.deepStrictEqual(calls, ['insertDate']);
	});

	it('keeps a colon inside the commandId intact instead of truncating it', function () {
		const dispatcher = newDispatcher();
		const calls: string[] = [];
		app.map._extensions['demo'] = {
			invokeCommand: (commandId: string) => calls.push(commandId),
		};

		// A naive action.split(':') would only capture 'my' as the commandId here.
		dispatcher.dispatch('ext:demo:my:weird:command');

		nodeassert.deepStrictEqual(calls, ['my:weird:command']);
	});

	it('warns instead of silently doing nothing when the extension is not found', function () {
		const dispatcher = newDispatcher();
		const warnings: string[] = [];
		const originalWarn = console.warn;
		console.warn = (msg: string) => {
			warnings.push(msg);
		};

		try {
			dispatcher.dispatch('ext:missing:someCommand');
		} finally {
			console.warn = originalWarn;
		}

		nodeassert.strictEqual(warnings.length, 1);
		nodeassert.ok(warnings[0].includes('missing'));
		nodeassert.ok(warnings[0].includes('someCommand'));
	});
});
