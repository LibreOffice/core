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
 * window.L.Control.DownloadProgress.
 */
/* global _ JSDialog app */
window.L.Control.DownloadProgress = window.L.Control.extend({
	options: {
		snackbarTimeout: 20000,
		userWarningKey: 'warnedAboutLargeCopy'
	},

	initialize: function (options) {
		window.L.setOptions(this, options);
	},

	onAdd: function (map) {
		this._map = map;
		this._started = false;
		this._complete = false;
		this._closed = false;
		this._isLargeCopy = false;
		this._infinite = true;
	},

	_userAlreadyWarned: function () {
		return window.prefs.getBoolean(this.options.userWarningKey);
	},

	_setUserAlreadyWarned: function () {
		window.prefs.set(this.options.userWarningKey, true);
	},

	_getDialogTitle: function () {
		return _('Download Selection');
	},

	_getLargeCopyPasteMessage: function () {
		return this._map._clip._substProductName(
			_('If you want to share large elements outside of {productname} it\'s necessary to first download them.'));
	},

	_getDownloadProgressDialogId: function () {
		return 'copy_paste_download_progress';
	},

	// Step 1. Large copy paste warning

	_showLargeCopyPasteWarning: function (inSnackbar) {
		var modalId = this._getDownloadProgressDialogId();

		var msg = this._getLargeCopyPasteMessage();
		var buttonText = _('Download') + app.util.replaceCtrlAltInMac(' (Ctrl + C)');

		if (inSnackbar) {
			this._map.uiManager.showSnackbar(
				msg, buttonText, this._onStartDownload.bind(this), this.options.snackbarTimeout);

			this.setupKeyboardShortcutForSnackbar();
		} else {
			this._map.uiManager.showInfoModal(modalId, this._getDialogTitle(), msg, '',
				buttonText, this._onStartDownload.bind(this), true, modalId + '-response');

			this.setupKeyboardShortcutForDialog(modalId);
		}
	},

	// Step 2. Download progress

	_showDownloadProgress: function (inSnackbar) {
		var modalId = this._getDownloadProgressDialogId();
		var msg = _('Downloading clipboard content');
		var buttonText = _('Cancel');

		if (inSnackbar) {
			this._map.uiManager.showProgressBar(msg, buttonText, this._onClose.bind(this), undefined, undefined, this._infinite);
		} else if (this._isLargeCopy) {
			// downloading for copy, next: show download complete dialog
			buttonText = _('Copy') + app.util.replaceCtrlAltInMac(' (Ctrl + C)');

			this._map.uiManager.showProgressBarDialog(modalId, this._getDialogTitle(), msg,
				buttonText, this._onConfirmCopyAction.bind(this), 0, this._onClose.bind(this), this._infinite);

			JSDialog.enableButtonInModal(modalId, modalId + '-response', false);
		} else {
			// downloading for paste, next: dialog will disappear
			this._map.uiManager.showProgressBarDialog(modalId, this._getDialogTitle(), msg,
				'', this._onClose.bind(this), 0, this._onClose.bind(this), this._infinite);
		}
	},

	_closeDownloadProgressDialog: function () {
		var modalId = this._getDownloadProgressDialogId();
		if (!this._userAlreadyWarned())
			this._map.uiManager.closeModal(this._map.uiManager.generateModalId(modalId));
	},

	// Step 3. Download complete

	_showDownloadComplete: function (inSnackbar) {
		var modalId = this._getDownloadProgressDialogId();
		var snackbarMsg = _('Download completed and ready to be copied to clipboard.');
		var dialogMsg = snackbarMsg + ' ' + _('From now on clipboard notifications will discreetly appear at the bottom.');
		var buttonText = _('Copy') + app.util.replaceCtrlAltInMac(' (Ctrl + C)');

		if (inSnackbar) {
			this._map.uiManager.setSnackbarProgress(100);

			this._map.uiManager.showSnackbar(
				snackbarMsg, buttonText, this._onConfirmCopyAction.bind(this), this.options.snackbarTimeout);

			this.setupKeyboardShortcutForSnackbar();
		} else {
			JSDialog.setMessageInModal(modalId, dialogMsg, '');
			JSDialog.enableButtonInModal(modalId, modalId + '-response', true);
			this.setupKeyboardShortcutForDialog(modalId);
		}
	},

	_setupKeyboardShortcutForElement: function (eventTargetId, buttonId) {
		var keyDownCallback = (e) => {
			var modifierKeys = !e.altKey && !e.shiftKey;
			if (window.L.Browser.mac) {
				modifierKeys = modifierKeys && e.metaKey && !e.ctrlKey;
			} else {
				modifierKeys = modifierKeys && e.ctrlKey && !e.metaKey;
			}
			// CTRL + C / Command + C
			if (modifierKeys && e.key === 'c') {
				document.getElementById(buttonId).click();
				e.preventDefault();
			}
		};
		if (document.getElementById(eventTargetId))
			document.getElementById(eventTargetId).onkeydown = keyDownCallback.bind(this); // FIXME
	},

	setupKeyboardShortcutForDialog: function (modalId) {
		app.layoutingService.appendLayoutingTask(() => {
			var dialogId = this._map.uiManager.generateModalId(modalId);
			var buttonId = modalId + '-response';
			this._setupKeyboardShortcutForElement(dialogId, buttonId);
			app.layoutingService.appendLayoutingTask(() => {
				const button = document.getElementById(buttonId);
				if (button) button.focus();
			}); // FIXME
		});
	},

	setupKeyboardShortcutForSnackbar: function () {
		app.layoutingService.appendLayoutingTask(() => {
			this._setupKeyboardShortcutForElement('snackbar', 'button-button');
		});
	},

	// isLargeCopy specifies if we are copying and have to explain user the process
	// if it is false we do paste so only show download progress
	show: function (isLargeCopy) {
		window.app.console.log('DownloadProgress.show ' + (isLargeCopy ? 'large' : 'regular'));
		// better to init the following state variables here,
		// since the widget could be re-used without having been destroyed
		this._started = false;
		this._complete = false;
		this._closed = false;
		this._isLargeCopy = isLargeCopy;

		if (isLargeCopy)
			this._showLargeCopyPasteWarning(this._userAlreadyWarned());
		else
			this._showDownloadProgress(this._userAlreadyWarned());
	},

	isClosed: function () {
		return this._closed;
	},

	isStarted: function () {
		return this._started;
	},

	isComplete: function () {
		return this._complete;
	},

	currentStatus: function () {
		if (this._closed)
			return 'closed';
		if (!this._started && !this._complete)
			return 'downloadButton';
		if (this._started)
			return 'progress';
		if (this._complete)
			return 'confirmPasteButton';
	},

	setURI: function (uri) {
		// set up data uri to be downloaded
		this._uri = uri;
	},

	setValue: function (value) {
		if (this._userAlreadyWarned())
			this._map.uiManager.setSnackbarProgress(Math.round(value), this._infinite);
		else {
			var modalId = this._getDownloadProgressDialogId();
			this._map.uiManager.setDialogProgress(modalId, Math.round(value), this._infinite);
		}
	},

	_setProgressCursor: function() {
		this._map.getContainer().style.cursor = 'progress';
	},

	_setNormalCursor: function() {
		this._map.getContainer().style.cursor = 'default';
	},

	startProgressMode: function() {
		this._setProgressCursor();
		this._started = true;
		this.setValue(0);
	},

	_onStartDownload: function () {
		if (!this._uri)
			return;

		this._showDownloadProgress(this._userAlreadyWarned());

		this.startProgressMode();
		this._download();

		return true;
	},

	_onUpdateProgress: function (e) {
		if (e.statusType === 'setvalue') {
			this.setValue(e.value);
		}
		else if (e.statusType === 'finish') {
			this._onComplete();
		}
	},

	_onComplete: function () {
		if (this._complete)
			return;
		this.setValue(100);
		this._setNormalCursor();
		this._complete = true;
		this._started = false;

		if (this._isLargeCopy)
			this._showDownloadComplete(this._userAlreadyWarned());
		else
			this._closeDownloadProgressDialog();
	},

	_onConfirmCopyAction: function () {
		this._map._clip.filterExecCopyPaste('.uno:Copy');
		this._onClose();
		this._setUserAlreadyWarned();

		var msg = _('Content copied to clipboard');
		this._map.uiManager.showSnackbar(msg);
	},

	_onClose: function () {
		if (this._userAlreadyWarned())
			this._map.uiManager.closeSnackbar();

		if (this._started)
			this._closeDownloadProgressDialog();

		this._started = false;
		this._complete = false;
		this._closed = true;

		this._setNormalCursor();
		this._cancelDownload();
		if (this._map)
			this._map.focus();
	},

	_download: async function () {
		let response;
		try {
			response = await this._map._clip._doAsyncDownload(
				'GET', this._uri, null, true,
				function(progress) { return progress/2; },
			);
		} catch (error) {
			this._onClose();
			app.showAsyncDownloadError(error, _('Download failed'));
			return;
		}

		window.app.console.log('clipboard async download done');
		// annoying async parse of the blob ...
		var reader = new FileReader();
		reader.onload = () => {
			var text = reader.result;
			window.app.console.log('async clipboard parse done: ' + text.substring(0, 256));
			let result = this._map._clip.parseClipboard(text);
			this._map._clip.setTextSelectionHTML(result['html'], result['plain']);
		};
		// TODO: failure to parse ? ...
		reader.readAsText(response);
	},

	_cancelDownload: function () {
		this._setNormalCursor();
		if (!this._started || this._complete)
			return;
		// TODO: insert code for cancelling an async download
	}
});

window.L.control.downloadProgress = function (options) {
	return new window.L.Control.DownloadProgress(options);
};
