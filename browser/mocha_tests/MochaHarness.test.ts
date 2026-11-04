/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Every suite shares one page. A suite adds its own elements and removes only those, so the
// base page is the same for the suites that run after it.
afterEach('the base page still holds #document-container', function () {
	nodeassert.ok(document.getElementById('document-container'));
});

// Tests replace these globals with stand-ins. Each test starts from the values the page loaded
// with.
let sharedGlobals: {
	map: any;
	dispatcher: any;
	file: any;
	mode: any;
	locale: string;
};

beforeEach('save the shared globals', function () {
	sharedGlobals = {
		map: app.map,
		dispatcher: app.dispatcher,
		file: app.file,
		mode: window.mode,
		locale: String.locale,
	};
});

afterEach('put back the shared globals', function () {
	app.map = sharedGlobals.map;
	app.dispatcher = sharedGlobals.dispatcher;
	app.file = sharedGlobals.file;
	window.mode = sharedGlobals.mode;
	String.locale = sharedGlobals.locale;
});

describe('Mocha harness', function () {
	beforeEach(function () {
		setUpAIChatPage();
	});

	afterEach(function () {
		tearDownAIChatPage();
		document.getElementById('x')?.remove();
		document.getElementById('harness-parent')?.remove();
	});

	it('a control subclass keeps the options it is built with', function () {
		const Sub = window.L.Control.extend({});
		const control = new Sub({ a: 1 });
		nodeassert.strictEqual(control.options.a, 1);
	});

	it('jQuery finds an element added to the page after it loaded', function () {
		const div = document.createElement('div');
		div.id = 'x';
		document.body.appendChild(div);
		nodeassert.strictEqual($('#x').length, 1);
	});

	it('a JSON container with two children is built into the page', function () {
		const parent = document.createElement('div');
		parent.id = 'harness-parent';
		document.body.appendChild(parent);

		const builder = window.L.control.jsDialogBuilder({
			windowId: 0,
			mobileWizard: null,
			map: makeAIChatMap(),
			cssClass: 'jsdialog',
		});
		builder.build(parent, [
			{
				id: 'c',
				type: 'container',
				children: [
					{ id: 'a', type: 'fixedtext', text: 'A' },
					{ id: 'b', type: 'fixedtext', text: 'B' },
				],
			},
		]);
		drainLayout();

		nodeassert.strictEqual(document.getElementById('a').textContent, 'A');
		nodeassert.strictEqual(document.getElementById('b').textContent, 'B');
	});
});
