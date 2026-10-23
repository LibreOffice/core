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

// A map with what the dialog control and the builder read from it, and a loaded text document.
// No command has a state yet, no item is locked and the UI is not blocked.
function makeDialogMap(): any {
	const map = makeAIChatMap();
	map.isEditMode = () => true;
	map.sendUnoCommand = () => {};
	map._progressBar = { end: () => {} };
	map._docLayer = { isCalc: () => false, isCalcRTL: () => false };
	map._fatal = false;
	map._docLoaded = true;
	map.stateChangeHandler = { getItemValue: (): any => undefined };
	map.isLockedItem = () => false;
	map.uiManager.isUIBlocked = () => false;
	return map;
}

// The messages sent to the server, in order.
let dialogSentMessages: string[] = [];

// What was logged as an error since setUpDialogPage. The layouting service catches a failing
// task and only logs it.
let dialogErrors: string[] = [];

interface DialogPageState {
	map: any;
	mapThis: any;
	layoutingService: any;
	localeService: any;
	timerRegistry: any;
	socket: any;
	setImage: any;
	idleHandler: any;
	error: any;
}

let dialogSaved: DialogPageState | null = null;

// Puts a fresh map, socket, layout service and an English locale on app, with a dialog control
// on the map. A jsdialog message that reaches the socket opens or closes a dialog, as it does in
// the browser.
function setUpDialogPage(): any {
	dialogSaved = {
		map: app.map,
		mapThis: window.L.Map.THIS,
		layoutingService: app.layoutingService,
		localeService: app.localeService,
		timerRegistry: app.timerRegistry,
		socket: app.socket,
		setImage: app.LOUtil.setImage,
		idleHandler: app.idleHandler,
		error: console.error,
	};
	dialogErrors = [];
	console.error = (message: any) => dialogErrors.push(String(message));

	const map = makeDialogMap();
	app.map = map;
	window.L.Map.THIS = map;
	app.layoutingService = new LayoutingService();
	app.localeService = {
		getDecimalSeparator: () => '.',
		getMinusSign: () => '-',
		initializeNumberFormatting: () => {},
	} as any;
	app.timerRegistry = new TimerRegistry();
	app.LOUtil.setImage = () => {};
	app.idleHandler = { _active: true } as any;
	dialogSentMessages = [];
	app.socket = {
		sendMessage: (message: string) => dialogSentMessages.push(message),
		_onMessage: (message: any) => {
			const prefix = 'jsdialog: ';
			if (!message.textMsg.startsWith(prefix)) return;
			map.fire('jsdialog', {
				data: JSON.parse(message.textMsg.substring(prefix.length)),
				callback: message.callback || (() => {}),
			});
		},
	} as any;

	const control = window.L.control.jsDialog();
	control.onAdd(map);
	map.jsdialog = control;
	return map;
}

// Opens a dialog from its JSON, as core sends it, and lays it out.
function openDialog(json: any): HTMLElement {
	app.map.jsdialog.onJSDialog({ data: json, callback: () => {} });
	drainLayout();
	return document.getElementById(json.id);
}

// Closes every dialog, removes what is left of them from the page, and puts app and the
// console back. Fails if anything was logged as an error since setUpDialogPage.
function closeAllDialogs(): void {
	const control = app.map.jsdialog;
	// The open dialogs are kept on the prototype, so every dialog control shares them.
	const dialogs = window.L.Control.JSDialog.prototype.dialogs;
	if (control) {
		for (const id of Object.keys(dialogs)) control.close(id, false);
		drainLayout();
		control.onRemove();
	}
	for (const id of Object.keys(dialogs)) delete dialogs[id];
	document
		.querySelectorAll('.jsdialog-window, .jsdialog-overlay')
		.forEach((element) => element.remove());
	JSDialog.SnackbarController.clearQueue();
	JSDialog.SnackbarController.showingSnackbar = false;

	if (!dialogSaved) return;
	app.map = dialogSaved.map;
	window.L.Map.THIS = dialogSaved.mapThis;
	app.layoutingService = dialogSaved.layoutingService;
	app.localeService = dialogSaved.localeService;
	app.timerRegistry = dialogSaved.timerRegistry;
	app.socket = dialogSaved.socket;
	app.LOUtil.setImage = dialogSaved.setImage;
	app.idleHandler = dialogSaved.idleHandler;
	console.error = dialogSaved.error;
	dialogSaved = null;
	nodeassert.deepStrictEqual(dialogErrors, []);
}
