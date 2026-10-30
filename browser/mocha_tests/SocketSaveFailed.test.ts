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

describe('Save errors in the desktop apps', function () {
	let savedErrorMessages: any;
	let savedQtApp: any;
	let warnings: string[];
	let prompts: any[];
	let map: any;
	let socket: any;

	beforeEach(function () {
		savedErrorMessages = (window as any).errorMessages;
		(window as any).errorMessages = {
			storage: { savefailed: 'storage save failed' },
		};
		savedQtApp = (window as any).ThisIsTheQtApp;
		(window as any).ThisIsTheQtApp = true;

		warnings = [];
		prompts = [];
		map = {
			_fatal: false,
			_docLayer: null,
			options: { doc: 'file:///tmp/doc.odt' },
			_saveFailedPrompt: null,
			fire: (name: string, args: any) => {
				if (name === 'warn') warnings.push(args.msg);
			},
			hideBusy: () => {},
			showBusy: () => {},
			_showSaveFailedPrompt: (errorDetail: string, fileName: string) => {
				prompts.push({ errorDetail, fileName });
			},
		};

		socket = new Socket(map);
		socket.connected = () => false;
		socket.close = () => {};
	});

	afterEach(function () {
		(window as any).errorMessages = savedErrorMessages;
		(window as any).ThisIsTheQtApp = savedQtApp;
	});

	function receive(message: string): boolean {
		return socket._onErrorMsg(message, socket.parseServerCmd(message));
	}

	it('prompts with the chosen name for a Save As the engine could not write', function () {
		nodeassert.strictEqual(
			receive('error: cmd=saveas kind=savefailed filename=chosen%20name.odt'),
			true,
		);
		nodeassert.deepStrictEqual(prompts, [
			{ errorDetail: undefined, fileName: 'chosen name.odt' },
		]);
		nodeassert.deepStrictEqual(warnings, []);
	});

	it('keeps the prompt over the engine reports that follow a failed Save As', function () {
		map._saveFailedPrompt = { closing: false };
		nodeassert.strictEqual(
			receive('error: cmd=saveas kind=io errordetail=x'),
			true,
		);
		nodeassert.strictEqual(
			receive('error: cmd=exportas kind=saveasfailed'),
			true,
		);
		nodeassert.deepStrictEqual(prompts, []);
		nodeassert.deepStrictEqual(warnings, []);
	});

	it('keeps the storage error dialog for a Save As the server could not place', function () {
		nodeassert.strictEqual(receive('error: cmd=storage kind=savefailed'), true);
		nodeassert.deepStrictEqual(prompts, []);
		nodeassert.deepStrictEqual(warnings, ['storage save failed']);
	});
});

describe('The alert dialog', function () {
	it('leaves the open dialogs alone for an error that shows nothing', function () {
		let closedAll = 0;
		const map = {
			_fatal: false,
			uiManager: {
				closeAll: () => {
					closedAll++;
				},
				showErrorModal: () => {
					throw new Error('nothing to show');
				},
			},
		};
		const dialog = { _map: map };
		const onError = (globalThis as any).L.Control.AlertDialog.prototype
			._onError;
		// The engine reports one such error after each failed save.
		onError.call(dialog, { cmd: '', kind: '', id: 1, code: -1 });
		nodeassert.strictEqual(closedAll, 0);
	});
});

describe('Save results in the desktop apps', function () {
	let posted: string[];
	let prompts: boolean[];
	let savedPost: any;
	let map: any;

	beforeEach(function () {
		posted = [];
		prompts = [];
		savedPost = (window as any).postMobileMessage;
		(window as any).postMobileMessage = (message: string) => {
			posted.push(message);
		};
		const methods = (globalThis as any).L.Map.included.find(
			(set: any) => set && typeof set._nativeSaveFailed === 'function',
		);
		map = Object.create(methods);
		map._pendingNativeSave = null;
		// The save asked again after one wsd started on its own always starts.
		map._saveForNativeHost = () => true;
		map._showSaveFailedPrompt = (
			errorDetail: string,
			fileName: string,
			closing: boolean,
		) => {
			prompts.push(closing);
		};
	});

	afterEach(function () {
		(window as any).postMobileMessage = savedPost;
	});

	it('leaves the save of the host waiting when a save wsd started on its own fails', function () {
		map._pendingNativeSave = 'quiet';
		map._nativeSaveFailed(true, 'cannot write');
		nodeassert.strictEqual(map._pendingNativeSave, 'quiet');
		nodeassert.deepStrictEqual(posted, []);

		map._nativeSaveSucceeded(false);
		nodeassert.strictEqual(map._pendingNativeSave, null);
		nodeassert.deepStrictEqual(posted, ['SAVECOMPLETED']);
		nodeassert.deepStrictEqual(prompts, []);
	});

	it('does not close on the success of a save wsd started on its own', function () {
		map._pendingNativeSave = 'close';
		map._nativeSaveSucceeded(true);
		nodeassert.strictEqual(map._pendingNativeSave, 'close');
		nodeassert.deepStrictEqual(posted, []);

		map._nativeSaveFailed(false, 'cannot write');
		nodeassert.deepStrictEqual(posted, ['SAVEFAILED prompt']);
		nodeassert.deepStrictEqual(prompts, [true]);
	});

	it('fails a quiet save without a prompt', function () {
		map._pendingNativeSave = 'quiet';
		map._nativeSaveFailed(false, 'cannot write');
		nodeassert.deepStrictEqual(posted, ['SAVEFAILED']);
		nodeassert.deepStrictEqual(prompts, []);
	});
});
