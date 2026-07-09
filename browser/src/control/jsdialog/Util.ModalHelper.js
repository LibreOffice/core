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

/*
 * JSDialog.ModalHelper - helper functions for manipulating modals built using jsdialog
 */

/* global JSDialog app */

var modalIdPretext = 'modal-dialog-';

/// Returns generated (or to be generated) id for the modal container.
function generateModalId(givenId) {
	return modalIdPretext + givenId;
}

function sendJSON(json) {
	if (!app.socket)
		return;

	app.socket._onMessage({textMsg: 'jsdialog: ' + JSON.stringify(json)});
}

function setMessageInModal(id, msg1) {
	var dialogId = generateModalId(id);

	var json = {
		id: dialogId,
		jsontype: 'dialog',
		type: 'modalpopup',
		action: 'update',
		control: {
			id: 'info-modal-label1',
			type: 'fixedtext',
			text: msg1
		}
	};

	sendJSON(json);
}

function enableButtonInModal(id, buttonId, enable) {
	var dialogId = generateModalId(id);

	var json = {
		id: dialogId,
		jsontype: 'dialog',
		type: 'modalpopup',
		action: 'action',
		data: {
			'control_id': buttonId,
			'action_type': (enable ? 'enable' : 'disable')
		}
	};

	sendJSON(json);
}

// check if user already set 'do not show again' option for a modal
function shouldShowAgain(id) {
    return window.prefs.getBoolean(`UIShowAgain_${id}`, true);
}

function setShowAgain(id, state) {
    window.prefs.set(`UIShowAgain_${id}`, state);
}

// helper to avoid using long list of optional parameters
function showInfoModalWithOptions(id, options) {
	if (app.map) {
		var title = options.title;
		var message1 = options.messages.length ? options.messages[0] : undefined;
		var message2 = options.messages.length > 1 ? options.messages[1] : undefined;
		//TODO: handle dynamic number of options.messages
		var buttonText = options.buttons && options.buttons.length ? options.buttons[0].text : undefined;
		var callback = options.buttons && options.buttons.length ? options.buttons[0].callback : undefined;
		//TODO: handle dynamic number of buttons with callback
		var withCancel = options.withCancel;
		var focusId = options.focusId;

		// TODO: move showInfoModal internals here
		app.map.uiManager.showInfoModal(id, title, message1, message2, buttonText,
			callback, withCancel, focusId);
	}
}

// check the widget is a modalpopup or in a modalpopup parent.
function isWidgetInModalPopup(widgetData) {

	if (widgetData.type == 'modalpopup')
		return true;

	while (widgetData.parent) {
		widgetData = widgetData.parent;
		if (widgetData.type == 'modalpopup')
			return true;
	}

	return false;
}

// A dialog window stays open after a widget inside it is activated, unlike a
// dropdown, snackbar or other popup, which closes immediately and expects
// focus to move back to the document.
function isBuilderInPersistentDialog(builder) {
	if (builder.windowId === undefined)
		return false;

	var dialogs = builder.map.jsdialog && builder.map.jsdialog.dialogs;
	var dialog = dialogs ? dialogs[builder.windowId] : undefined;

	return !!(dialog && dialog.nonModal);
}

// Sends focus back to the document canvas, unless doing so would steal
// keyboard focus away from a dialog window that is still open. Returns
// whether it moved focus to the document, so a caller that only handled
// focus conditionally can leave any other focus-restoration mechanism
// (e.g. returning to the last focused element) in charge otherwise.
function focusMapUnlessInDialog(builder) {
	if (!builder.map)
		return false;

	if (isBuilderInPersistentDialog(builder))
		return false;

	builder.map.focus();
	return true;
}

JSDialog.generateModalId = generateModalId;
JSDialog.sendJSON = sendJSON;
JSDialog.setMessageInModal = setMessageInModal;
JSDialog.enableButtonInModal = enableButtonInModal;
JSDialog.shouldShowAgain = shouldShowAgain;
JSDialog.setShowAgain = setShowAgain;
JSDialog.showInfoModalWithOptions = showInfoModalWithOptions;
JSDialog.isWidgetInModalPopup = isWidgetInModalPopup;
JSDialog.focusMapUnlessInDialog = focusMapUnlessInDialog;
