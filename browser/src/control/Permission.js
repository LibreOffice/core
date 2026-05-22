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
			} else if (!window.ThisIsAMobileApp && !this['wopi'].UserCanWrite) {
				$('#mobile-edit-button').hide();
			} else if (window.mode.isSmallScreenDevice() || window.mode.isTablet()) {
				// Writeable user stepped back from edit to readonly: keep the FAB
				// visible so they can re-enter edit mode.
				button.on('click', function () {
					that._switchToEditMode();
				});
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
		}

		// Defer focus until the layouting queue drains; on desktop the
		// notebookbar dropdown queues a task on close that would otherwise
		// steal focus back. In the iOS/android app, just clicking the
		// mobile-edit-button is not reason enough to pop up the on-screen
		// keyboard, so skip the focus there.
		if (!(window.ThisIsTheiOSApp || window.ThisIsTheAndroidApp))
			app.layoutingService.onDrain(this.focus.bind(this));
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

	// Show dialog when in WASM mode asking user to choose between
	// editing locally (WASM) or joining collaborative editing (server).
	// Show a two-column "card" dialog with a title, a subtitle,
	// and two side-by-side options each with a heading, a short
	// description, and a button.  If @p titleAvatar is a non-empty
	// URL, it is rendered as a circular image to the left of the
	// title (used for "X started editing").  Styling is applied by
	// CSS via the #collab-choice-dialog id.
	// @p title may be either a plain string or an object
	//   { name: '...', rest: '...' }
	// In the latter case, and when @p titleAvatar is set,
	// the avatar + name are wrapped in a pill and the rest
	// of the title text follows outside the pill.
	_showTwoCardDialog: function (id, title, subtitle, cards, titleAvatar) {
		var dialogId = this.uiManager.generateModalId(id);
		var that = this;

		var titleChildren = [];
		if (typeof title === 'object' && titleAvatar) {
			titleChildren.push({
				id: 'collab-choice-pill',
				type: 'container',
				vertical: false,
				children: [
					{
						id: 'collab-choice-avatar',
						type: 'image',
						image: titleAvatar,
						text: ''
					},
					{
						id: 'collab-choice-name',
						type: 'fixedtext',
						text: title.name
					}
				]
			});
			titleChildren.push({
				id: 'collab-choice-title',
				type: 'fixedtext',
				text: title.rest
			});
		}
		else {
			if (titleAvatar) {
				titleChildren.push({
					id: 'collab-choice-avatar',
					type: 'image',
					image: titleAvatar,
					text: ''
				});
			}
			titleChildren.push({
				id: 'collab-choice-title',
				type: 'fixedtext',
				text: (typeof title === 'object' ?
					(title.name + ' ' + title.rest) : title)
			});
		}

		// Icon paths in c.icon are theme-agnostic stems
		// ('images/coda-collab-<name>'); pick the dark or light
		// variant once, here at build time, based on the current
		// dark-theme pref.  The dialog is modal and short-lived, so
		// a theme change mid-decision (which would leave the icon
		// stale) is not worth optimising for.
		var darkTheme = window.prefs.getBoolean('darkTheme');
		var iconSuffix = darkTheme ? '-dark.svg' : '-light.svg';

		var cardWidgets = cards.map(function (c) {
			return {
				id: c.id + '-card',
				type: 'container',
				vertical: true,
				children: [
					{
						id: c.id + '-card-icon',
						type: 'image',
						image: c.icon + iconSuffix,
						text: ''
					},
					{
						id: c.id + '-card-heading',
						type: 'fixedtext',
						text: c.heading
					},
					{
						id: c.id + '-card-description',
						type: 'fixedtext',
						text: c.description
					}
				]
			};
		});

		var json = {
			id: dialogId,
			dialogid: id,
			type: 'modalpopup',
			title: '',
			hasClose: false,
			hasOverlay: true,
			cancellable: false,
			jsontype: 'dialog',
			'init_focus_id': 'response',
			children: [{
				id: 'collab-choice-dialog',
				type: 'container',
				vertical: true,
				children: [
					{
						id: 'collab-choice-title-row',
						type: 'container',
						vertical: false,
						children: titleChildren
					},
					{
						id: 'collab-choice-subtitle',
						type: 'fixedtext',
						text: subtitle
					},
					{
						id: 'collab-choice-cards',
						type: 'container',
						vertical: false,
						children: cardWidgets
					}
				]
			}]
		};

		this.uiManager.showModal(json);

		// The jsdialog builder renders the dialog asynchronously, so the card containers (and
		// the avatar <img>, if any) only exist some short time after showModal returns; poll
		// briefly for them and wire up the click and image-error handlers once present.
		var tries = 0;
		var wire = function () {
			var anyMissing = false;
			cards.forEach(function (c) {
				var card = document.getElementById(c.id + '-card');
				if (!card) {
					anyMissing = true;
					return;
				}
				if (card.dataset.codaCollabCardWired) return;
				card.dataset.codaCollabCardWired = '1';
				card.setAttribute('role', 'button');
				card.setAttribute('tabindex', '0');
				card.addEventListener('click', function () {
					that.uiManager.closeModal(dialogId);
					c.onClick();
				});
				card.addEventListener('keydown', function (e) {
					if (e.key === 'Enter' || e.key === ' ') {
						e.preventDefault();
						that.uiManager.closeModal(dialogId);
						c.onClick();
					}
				});
			});
			if (titleAvatar) {
				var img = document.getElementById('collab-choice-avatar');
				if (!img) {
					anyMissing = true;
				} else if (!img.dataset.codaCollabAvatarWired) {
					img.dataset.codaCollabAvatarWired = '1';
					img.onerror = function () { img.style.display = 'none'; };
					if (img.complete && img.naturalWidth === 0)
						img.style.display = 'none';
				}
			}
			if (anyMissing && ++tries < 50)
				setTimeout(wire, 50);
		};
		setTimeout(wire, 0);
	},

	// The user wants to start editing while other users are
	// viewing.  Offer the choice between editing locally
	// (changes sync on save) or starting a collaborative
	// session (all users edit together in real-time).
	_showWasmEditChoice: function () {
		var that = this;
		this._showTwoCardDialog(
			'wasm-edit-choice-modal',
			_('How would you like to edit?'),
			_('Other users are viewing this document. Choose how you\'d like to continue:'),
			[
				{
					id: 'edit-locally',
					icon: 'images/coda-collab-local-editing',
					heading: _('Edit locally'),
					description: _('Changes sync when you save'),
					onClick: function () { that._proceedEditMode(); }
				},
				{
					id: 'start-collaborative',
					icon: 'images/coda-collab-collaborative-editing',
					heading: _('Collaborative editing'),
					description: _('Edit together in real-time'),
					onClick: function () { that._saveAndSwitchToServerMode(); }
				}
			]
		);
	},

	// Show a "keep viewing vs. join collab editing" choice using
	// the two-card layout.  Used both when another user has just
	// started editing and when a collaborative session is already
	// active - only the title/subtitle (and optional avatar image
	// shown next to the title) differ.
	_showCollabJoinDialog: function (title, subtitle, avatar) {
		var that = this;
		this._showTwoCardDialog(
			'collab-join-modal',
			title,
			subtitle,
			[
				{
					id: 'collab-keep-viewing',
					icon: 'images/coda-collab-local-viewing',
					heading: _('Keep viewing local copy'),
					description: _('Continue with your version'),
					onClick: function () {
						// Stay in current WASM read-only mode.
					}
				},
				{
					id: 'collab-join',
					icon: 'images/coda-collab-collaborative-editing',
					heading: _('Join collaborative editing'),
					description: _('Edit together in real-time'),
					onClick: function () {
						if (window.collabEditingActive) {
							// Someone is editing locally - ask them to
							// save and switch, then wait.
							that._waitForCollabSave();
							window.collabSendMessage({type: 'switch_to_collab'});
						} else {
							// No active editor - just switch directly.
							window.switchToServerMode();
						}
					}
				}
			],
			avatar
		);
	},

	// Start waiting for the local editor to save and switch.
	// Shows a busy indicator and sets a timeout so we switch
	// even if the editor does not respond (e.g., they have a
	// dialog open or have already left).
	_waitForCollabSave: function () {
		this._waitingForCollabSwitch = true;
		this.fire('showbusy', {
			label: _('Waiting for changes to be saved...')
		});
		var that = this;
		this._collabSaveTimeout = setTimeout(function () {
			that._finishCollabSwitch();
		}, 10000);
	},

	// Called when the wait is over (editor saved, timed out, or
	// all other users left).  Cleans up and switches.
	_finishCollabSwitch: function () {
		if (!this._waitingForCollabSwitch)
			return;
		this._waitingForCollabSwitch = false;
		if (this._collabSaveTimeout) {
			clearTimeout(this._collabSaveTimeout);
			this._collabSaveTimeout = null;
		}
		this.fire('hidebusy');
		window.switchToServerMode();
	},

	// The local editor has saved and is switching to server mode.
	_onEditorSavedAndSwitching: function () {
		this._finishCollabSwitch();
	},

	// A collab user left while we were waiting for their save.
	// If no other collab users remain, switch now.
	_onCollabUserLeft: function () {
		if (this._waitingForCollabSwitch
			&& window.collabUsers && window.collabUsers.length === 0) {
			this._finishCollabSwitch();
		}
	},

	// Save any local-edit changes and then switch to server mode.
	// If the document has been modified, .uno:Save triggers the
	// per-platform upload path (collabSaveToServer for COWASM, the
	// commandresult-driven collabUploadFile in main.js for CODA),
	// and the _switchToServerAfterSave flag causes switchToServerMode
	// to be called after the upload completes.  If there are no
	// modifications, .uno:Save is a no-op and we switch immediately.
	_saveAndSwitchToServerMode: function () {
		if (this._permission === 'edit' && this._everModified) {
			window._switchToServerAfterSave = true;
			this.save(true /* dontTerminateEdit */,
				false /* dontSaveIfUnmodified */);
		} else {
			window.collabSendMessage({type: 'saved_and_switching'});
			window.switchToServerMode();
		}
	},

	// Save any local-edit changes and then close the window.  Mirrors
	// _saveAndSwitchToServerMode but the post-save action is window
	// close, signalled to the native host via a CLOSE_WINDOW message
	// in main.js's commandresult handler once the save (and, for
	// remote docs, the subsequent integrator upload) completes.
	_saveAndClose: function () {
		if (this._permission === 'edit' && this._everModified) {
			window._closeAfterSave = true;
			this.save(true /* dontTerminateEdit */,
				false /* dontSaveIfUnmodified */);
		} else {
			window.postMobileMessage('CLOSE_WINDOW');
		}
	},

	// Another user wants to start collaborative editing.  Save
	// local changes and switch to server mode.  If a dialog is
	// already open (e.g., the edit choice dialog), do nothing -
	// the user will make their own choice via the dialog buttons.
	_onSwitchToCollabRequest: function () {
		if (this.uiManager.isAnyDialogOpen())
			return;
		this._saveAndSwitchToServerMode();
	},

	// A newly-joining user opens a document where a
	// collaborative editing session is already active.
	_onCollabEditingActive: function () {
		this._showCollabJoinDialog(
			_('How would you like to edit?'),
			_('Other users are editing this document. Choose how you\'d like to continue:'));
	},

	// Another user has just started editing while we were
	// viewing.  Offer to keep viewing locally or join the
	// collaborative session.
	_onOtherUserEditingStarted: function (userName, avatar) {
		// We are editing ourselves: don't offer to switch (and risk
		// dropping our in-progress local changes); the conflict gets
		// resolved at next save.  This is reachable via the /cool/ws
		// bridge in CollabBroker, since plain-COOL's editing_started
		// can land here while we are mid-edit.
		if (this.isEditMode())
			return;
		this._showCollabJoinDialog(
			avatar
				? { name: userName, rest: _('started editing') }
				: _('%0 started editing').replace('%0', userName),
			_('Someone else is now editing this document. Choose how you\'d like to continue:'),
			avatar);
	},

	// from read-only to edit mode
	_switchToEditMode: function () {
		// Notify the collab broker that editing is starting so that
		// users who join later are informed.
		if (window.ThisIsTheEmscriptenApp || window.mode.isCODesktop()) {
			window.collabSendMessage({type: 'editing_started'});
		}

		// With other collab users present, offer the choice between
		// local and collaborative editing.
		if ((window.ThisIsTheEmscriptenApp || window.mode.isCODesktop())
			&& window.collabUsers && window.collabUsers.length > 0) {
			this._showWasmEditChoice();
			return;
		}

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

	// Tell core whether this view is read-only, and update the client-side
	// comment and redline edit flags to match. This applies only to a session
	// that is editing-capable but currently viewing, so toggling between view
	// and edit must flip the flags. A session whose document is genuinely
	// read-only is skipped: core already set it read-only at session start,
	// and its comment and redline flags already reflect the real document
	// state (for example comments allowed on an otherwise read-only PDF), so
	// they must not be overwritten here. app.isReadOnly() carries the real
	// document permission for every host, online or desktop, and the view
	// and edit toggle does not change it, so it is the right gate to use.
	_applyViewReadOnly: function (readOnly) {
		if (app.isReadOnly())
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

		if (this._docLayer._docType === 'text' && (window.mode.isSmallScreenDevice() || window.mode.isTablet())) {
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
