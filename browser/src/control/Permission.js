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
 * Document permission handler
 */
/* global app $ _ */
window.L.Map.include({
	readonlyStartingFormats: {
		'txt': { canEdit: true, odfFormat: 'odt' },
		'csv': { canEdit: true, odfFormat: 'ods' },
		'xlsb': { canEdit: false, odfFormat: 'ods' }
	},

	setPermission: function (perm) {
		var button = $('#mobile-edit-button');
		button.off('click');
		button.attr('tabindex', 0);
		button.attr('role', 'button');
		button.attr('title', _('Edit document'));
		button.attr('aria-label', _('Edit document'));
		// app.file.fileBasedView is new view that has continuous scrolling
		// used for PDF and we don't permit editing for PDFs
		// this._shouldStartReadOnly() is a check for files that should start in readonly mode and even on desktop browser
		// we warn the user about loosing the rich formatting and offer an option to
		// save as ODF instead of the current format
		//
		// For mobile we need to display the edit button for all the cases except for PDF (which is read-only)
		// we offer save-as to another place where the user can edit the document
		if (!app.file.readOnly && (window.mode.isSmallScreenDevice() || window.mode.isTablet())) {
			button.css('display', 'flex');
		} else {
			button.hide();
		}
		var that = this;
		if (perm === 'edit') {
			// Only apply the opt-in gate when the doc is first opened;
			// later setPermission calls (reload, save-as, server perm
			// changes) honor what was asked.
			var firstOpen = this._permission === undefined;
			if (firstOpen && (this._shouldStartReadOnly() || window.mode.isSmallScreenDevice() || window.mode.isTablet())) {
				button.on('click', function () {
					that._switchToEditMode();
				});

				// temporarily, before the user touches the floating action button
				this._enterReadOnlyMode('readonly');
			}
			else if (this.options.canTryLock) {
				// This is a success response to an attempt to lock using mobile-edit-button
				this._switchToEditMode();
			}
			else {
				this._enterEditMode(perm);
			}
		}
		else if (perm === 'view' || perm === 'readonly') {
			if (this.isLockedReadOnlyUser()) {
				button.on('click', function () {
					that.openUnlockPopup();
				});
			}
			else if (window.ThisIsTheAndroidApp) {
				button.on('click', function () {
					that._requestFileCopy();
				});
			} else if ((!window.ThisIsAMobileApp && !this['wopi'].UserCanWrite) || (!this.options.canTryLock && (window.mode.isSmallScreenDevice() || window.mode.isTablet()))) {
				$('#mobile-edit-button').hide();
			}

			this._enterReadOnlyMode(perm);
		}
	},

	onLockFailed: function(reason) {
		if (this.options.canTryLock === undefined) {
			// This is the initial notification. This status is not permanent.
			// Allow to try to lock the file for edit again.
			this.options.canTryLock = true;

			var alertMsg = _('The document could not be locked, and is opened in read-only mode.');
			if (reason) {
				alertMsg += '\n' + _('Server returned this reason:') + '\n"' + reason + '"';
			}
			this.uiManager.showConfirmModal('lock_failed_message', '', alertMsg, _('OK'), function() {
				app.socket.sendMessage('attemptlock');
			}, true);
		}
		else if (this.options.canTryLock) {
			// This is a failed response to an attempt to lock using mobile-edit-button
			alertMsg = _('The document could not be locked.');
			if (reason) {
				alertMsg += '\n' + _('Server returned this reason:') + '\n"' + reason + '"';
			}
			this.uiManager.showConfirmModal('lock_failed_message', '', alertMsg, _('OK'), null, true);
		}
		// do nothing if this.options.canTryLock is defined and is false
	},

	_getFileExtension: function (filename) {
		return filename.substring(filename.lastIndexOf('.') + 1);
	},

	_shouldStartReadOnly: function () {
		if (this.isLockedReadOnlyUser())
			return true;
		if (window.coolParams.get('startreadonly') === 'true') {
			return true;
		}
		var fileName = this['wopi'].BaseFileName;
		// use this feature for only integration.
		if (!fileName) return false;
		var extension = this._getFileExtension(fileName).toLowerCase();
		
		// Check if this is a view mode format from server configuration
		if (app.isViewModeExtension(extension)) return true;
		
		if (!Object.prototype.hasOwnProperty.call(this.readonlyStartingFormats, extension))
			return false;
		return true;
	},

	_proceedEditMode: function() {
		var fileName = this['wopi'].BaseFileName;
		if (fileName) {
			var extension = this._getFileExtension(fileName);
			var extensionInfo = this.readonlyStartingFormats[extension];
			if (extensionInfo && !extensionInfo.canEdit)
				return;
		}
		this.options.canTryLock = false; // don't respond to lockfailed anymore
		$('#mobile-edit-button').hide();
		this._enterEditMode('edit');
		if (window.mode.isSmallScreenDevice() || window.mode.isTablet() || window.mode.isCODesktop()) {
			this.fire('editorgotfocus');
			this.fire('closemobilewizard');
			// In the iOS/android app, just clicking the mobile-edit-button is
			// not reason enough to pop up the on-screen keyboard.
			if (!(window.ThisIsTheiOSApp || window.ThisIsTheAndroidApp))
				this.focus();
		}
	},

	_offerSaveAs: function() {
		var fileName = this['wopi'].BaseFileName;
		if (!fileName) return false;
		var extension = this._getFileExtension(fileName);
		var extensionInfo = this.readonlyStartingFormats[extension];
		var saveAsFormat = extensionInfo.odfFormat;

		var defaultValue = fileName.substring(0, fileName.lastIndexOf('.')) + '.' + saveAsFormat;
		this.uiManager.showInputModal('save-as-modal', '', _('Enter a file name'), defaultValue, _('OK'), function() {
			var value = document.getElementById('save-as-modal').querySelectorAll('#input-modal-input')[0].value;
			if (!value)
				return;
			else if (value.substring(value.lastIndexOf('.') + 1) !== saveAsFormat) {
				value += '.' + saveAsFormat;
			}
			this.saveAs(value, saveAsFormat);
		}.bind(this));
	},

	// from read-only to edit mode
	_switchToEditMode: function () {
		// This will be handled by the native mobile app instead
		if (this._shouldStartReadOnly() && !window.ThisIsAMobileApp) {
			var fileName = this['wopi'].BaseFileName;
			var extension = this._getFileExtension(fileName);
			
			// For defined formats (from server config), just proceed to edit mode without dialog
			if (app.isViewModeExtension(extension)) {
				this._proceedEditMode();
				return;
			}
			
			var extensionInfo = this.readonlyStartingFormats[extension];

			var yesButtonText = !this['wopi'].UserCanNotWriteRelative ? _('Save as ODF format'): null;
			var noButtonText = extensionInfo.canEdit ? _('Continue editing') : _('Continue read only');

			if (!yesButtonText) {
				yesButtonText = noButtonText;
				noButtonText = null;
			}

			var yesFunction = !noButtonText ? function() { this._proceedEditMode(); }.bind(this) : function() { this._offerSaveAs(); }.bind(this);
			var noFunction = function() { this._proceedEditMode(); }.bind(this);

			this.uiManager.showYesNoButton(
				'switch-to-edit-mode-modal', // id.
				'', // Title.
				_('This document may contain formatting or content that cannot be saved in the current file format.'), // Message.
				yesButtonText,
				noButtonText,
				yesFunction,
				noFunction,
				false // Cancellable.
			);
		} else {
			this._proceedEditMode();
		}
	},

	_requestFileCopy: function() {
		if (app.isReadOnly()) {
			window.postMobileMessage('REQUESTFILECOPY');
		} else {
			this._switchToEditMode();
		}
	},

	// Tell core whether this view is read-only, and flip the client-side
	// comment/redline gates the UI checks via isCommentEditingAllowed()/
	// isRedlineManagementAllowed(). Only meaningful for users that actually
	// have WOPI write permission: users without write permission already
	// get read-only set up server-side at session start, and their
	// app.file.editComment / allowManageRedlines flags already reflect the
	// real doc state (e.g. comment-only PDFs) and must not be touched.
	_applyViewReadOnly: function (readOnly) {
		if (!this['wopi'] || !this['wopi'].UserCanWrite)
			return;
		if (app.socket)
			app.socket.sendMessage('setviewreadonly value=' + readOnly);
		app.file.editComment = !readOnly;
		app.file.allowManageRedlines = !readOnly;
	},

	_enterEditMode: function (perm) {
		this._permission = perm;

		if ((window.mode.isSmallScreenDevice() || window.mode.isTablet()) && this._textInput && this.getDocType() === 'text') {
			this._textInput.setSwitchedToEditMode();
		}

		if (app.map['stateChangeHandler'].getItemValue('EditDoc') === 'false')
			app.map.sendUnoCommand('.uno:EditDoc?Editable:bool=true');

		// Re-enable direct-canvas interactions (shape drag, arrow-key
		// shape move) that the matching _enterReadOnlyMode branch
		// disabled.
		this._applyViewReadOnly(false);

		app.events.fire('updatepermission', {perm : perm});

		if (this._docLayer._docType === 'text') {
			this.setZoom(10);
		}

		if (window.ThisIsTheiOSApp && window.mode.isTablet() && this._docLayer._docType === 'spreadsheet')
			this.showCalcInputBar();

		if (window.ThisIsTheAndroidApp)
			window.postMobileMessage('EDITMODE on');
	},

	_enterReadOnlyMode: function (perm) {
		this._permission = perm;

		// disable all user interaction, will need to add keyboard too
		if (this._docLayer) {
			this._docLayer._onUpdateCursor();
			this._docLayer._clearSelections();
		}

		// Block direct-canvas interactions (shape drag, arrow-key shape
		// move) server-side and hide per-comment edit/redline controls
		// in the UI.
		this._applyViewReadOnly(true);

		app.events.fire('updatepermission', {perm : perm});
		this.fire('closemobilewizard');
		this.fire('closealldialogs');

		if (window.ThisIsTheAndroidApp)
			window.postMobileMessage('EDITMODE off');
	},

	// Is user currently in read only mode (i.e: initial mobile read only view mode, user may have write access)
	isReadOnlyMode: function() {
		return this._permission === 'readonly';
	},

	// Is user currently in editing mode
	isEditMode: function() {
		return this._permission === 'edit';
	}
});
