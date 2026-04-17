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
 * L.WOPI contains WOPI related logic
 */

/* global _ app _UNO JSDialog errorMessages URLPopUpSection */
window.L.Map.WOPI = window.L.Handler.extend({
	// If the CheckFileInfo call fails on server side, we won't have any PostMessageOrigin.
	// So use '*' because we still needs to send 'close' message to the parent frame which
	// wouldn't be possible otherwise.
	PostMessageOrigin: window.postmessageOriginExt || '*',
	BaseFileName: '',
	BreadcrumbDocName: '',
	DocumentLoadedTime: false,
	HidePrintOption: false,
	HideSaveOption: false,
	HideExportOption: false,
	HideRepairOption: false,
	HideChangeTrackingControls: false,
	DisablePrint: false,
	DisableExport: false,
	DisableCopy: false,
	DisableInactiveMessages: false,
	DownloadAsPostMessage: false,
	UserCanNotWriteRelative: true,
	EnableInsertRemoteImage: false,
	EnableInsertRemoteFile: false, /* Separate, because requires explicit integration support */
	DisableInsertLocalImage: false,
	EnableInsertRemoteLink: false,
	EnableRemoteAIContent: false,
	DisableAISettings: false,
	AIConfigured: false,
	AIModelName: '',
	EnableShare: false,
	HideUserList: null,
	CallPythonScriptSource: null,
	SupportsRename: false,
	UserCanRename: false,
	UserCanWrite: false,
	DisablePresentation: false,
	PresentationLeader: '',

	_appLoadedConditions: {
		docloaded: false,
		updatepermission: false,
		viewinfo: false, /* Whether view information has already arrived */
		initializedui: false,
	},

	_appLoaded: false,
	_insertImageMenuSetupDone: false,

	postLoadEnable: function() {
		this._map.on('docloaded', this._postLoaded, this);
		app.events.on('updatepermission', this._postLoadedBound);
		this._map.on('viewinfo', this._postLoaded, this);
		this._map.on('initializedui', this._postLoaded, this);
	},

	postLoadDisable: function() {
		this._map.off('docloaded', this._postLoaded, this);
		app.events.off('updatepermission', this._postLoadedBound);
		this._map.off('viewinfo', this._postLoaded, this);
		this._map.off('initializedui', this._postLoaded, this);
	},

	initialize: function(map) {
		this._map = map;
		this._postLoadedBound = this._postLoaded.bind(this);
		// init messages handlers should be available as soon as possible
		this.postLoadEnable();
	},

	addHooks: function() {
		this._map.on('postMessage', this._postMessage, this);

		this._map.on('wopiprops', this._setWopiProps, this);
		window.L.DomEvent.on(window, 'message', this._postMessageListener, this);

		this._map.on('updateviewslist', function() { this._postViewsMessage('Views_List'); }, this);

		if (!window.ThisIsAMobileApp) {
			// override the window.open to issue a postMessage, so that
			// it is possible to handle the hyperlink in the integration
			var that = this;
			window.open = function (open) {
				return function (url, name, features) {
					const eSignature = that._map.eSignature;
					const eSignInProgress = eSignature && eSignature.signInProgress;
					if (eSignInProgress) {
						return open.call(window, url, name, features);
					}

					that._map.fire('postMessage', {
						msgId: 'UI_Hyperlink',
						args: {
							Url: url,
							Name: name,
							Features: features
						}
					});
					if (!that._map._disableDefaultAction['UI_Hyperlink'])
						return open.call(window, url, name, features);
					else
						return null;
				};
			}(window.open);
		}
	},

	removeHooks: function() {
		this._map.off('postMessage', this._postMessage, this);

		this.postLoadDisable();

		this._map.off('wopiprops', this._setWopiProps, this);
		window.L.DomEvent.off(window, 'message', this._postMessageListener, this);

		this._map.off('updateviewslist');
	},

	// Return whether there is the capability to rename, not the permission.
	// Since we fall back on Save As for rename isn't supported.
	_supportsRename: function() {
		return !!this.SupportsRename || !this.UserCanNotWriteRelative;
	},

	_setWopiProps: function(wopiInfo) {
		var overridenFileInfo = window.checkFileInfoOverride;
		// Store postmessageorigin property, if it exists
		if (wopiInfo['PostMessageOrigin']) {
			this.PostMessageOrigin = wopiInfo['PostMessageOrigin'];
		}

		this.BaseFileName = wopiInfo['BaseFileName'];
		this.BreadcrumbDocName = wopiInfo['BreadcrumbDocName'];
		if (this.BreadcrumbDocName === undefined)
			this.BreadcrumbDocName = this.BaseFileName;
		this.HidePrintOption = !!wopiInfo['HidePrintOption'];
		this.HideSaveOption = !!wopiInfo['HideSaveOption'];
		this.HideExportOption = !!wopiInfo['HideExportOption'];
		this.HideRepairOption = !!wopiInfo['HideRepairOption'];
		this.HideChangeTrackingControls = !!wopiInfo['HideChangeTrackingControls'];
		this.DisablePrint = !!wopiInfo['DisablePrint'];
		this.DisableExport = !!wopiInfo['DisableExport'];
		this.DisableCopy = !!wopiInfo['DisableCopy'];
		this.DisableInactiveMessages = !!wopiInfo['DisableInactiveMessages'];
		this.DownloadAsPostMessage = Object.prototype.hasOwnProperty.call(overridenFileInfo, 'DownloadAsPostMessage') ?
			overridenFileInfo.DownloadAsPostMessage : !!wopiInfo['DownloadAsPostMessage'];
		this.UserCanNotWriteRelative = !!wopiInfo['UserCanNotWriteRelative'];
		this.EnableInsertRemoteImage = !!wopiInfo['EnableInsertRemoteImage'];
		this.EnableInsertRemoteFile = !!wopiInfo['EnableInsertRemoteFile'];
		this.DisableInsertLocalImage = !!wopiInfo['DisableInsertLocalImage'];
		this.EnableRemoteLinkPicker = !!wopiInfo['EnableRemoteLinkPicker'];
		this.EnableRemoteAIContent = !!wopiInfo['EnableRemoteAIContent'];
		this.DisableAISettings = !!wopiInfo['DisableAISettings'];
		this.AIConfigured = !!wopiInfo['AIConfigured'];
		this.AIModelName = wopiInfo['AIModelName'] || '';
		app.serverConnectionService.onWopiProps({
			AIConfigured: this.AIConfigured,
			AIModelName: this.AIModelName,
		});
		this.SupportsRename = !!wopiInfo['SupportsRename'];
		this.UserCanRename = !!wopiInfo['UserCanRename'];
		this.EnableShare = !!wopiInfo['EnableShare'];
		this.UserCanWrite = !!wopiInfo['UserCanWrite'];
		this.DisablePresentation = wopiInfo['DisablePresentation'];
		this.PresentationLeader = wopiInfo['PresentationLeader'];
		this.CommentAvatarUrl = wopiInfo['CommentAvatarUrl'];

		if (this.UserCanWrite && !app.isReadOnly()) // There are 2 places that set the file permissions, WOPI and URI. Don't change permission if URI doesn't allow.
			app.setPermission('edit');

		this.IsOwner = !!wopiInfo['IsOwner'];

		if (wopiInfo['HideUserList'])
			this.HideUserList = wopiInfo['HideUserList'].split(',');

		this.sendFrameReady();

		if ('TemplateSaveAs' in wopiInfo) {
			this._map.showBusy(_('Creating new file from template...'), false);
			this._map.saveAs(wopiInfo['TemplateSaveAs']);
		}

		this.setupImageInsertionMenu();
	},

	sendFrameReady: function() {
		this._map.fire('postMessage', {
			msgId: 'App_LoadingStatus',
			args: {
				Status: 'Frame_Ready',
				Features: {
					VersionStates: true
				}
			}
		});
	},

	sendDocumentLoaded: function() {
		this._map.fire('postMessage', {
			msgId: 'App_LoadingStatus',
			args: {
				Status: 'Document_Loaded',
				DocumentLoadedTime: this.DocumentLoadedTime
			}
		});
	},

	setupImageInsertionMenu: function() {
		if (this._insertImageMenuSetupDone) {
			return;
		}

		if (this.DisableInsertLocalImage) {
			JSDialog.MenuDefinitions.set('InsertImageMenu', []);
			JSDialog.MenuDefinitions.set('InsertMultimediaMenu', []);
			JSDialog.MenuDefinitions.set('CompareDocumentsMenu', []);
		}

		var menuEntriesImage = JSDialog.MenuDefinitions.get('InsertImageMenu');
		var menuEntriesMultimedia = JSDialog.MenuDefinitions.get('InsertMultimediaMenu');
		var menuEntriesCompare = JSDialog.MenuDefinitions.get('CompareDocumentsMenu');

		if (this.EnableInsertRemoteImage) {
			menuEntriesImage.push({action: 'remotegraphic', text: _UNO('.uno:InsertGraphic', '', true)});
		}

		if (this.EnableInsertRemoteFile) {
			/* Separate, because needs explicit integration support */
			menuEntriesMultimedia.push({action: 'remotemultimedia', text: _UNO('.uno:InsertAVMedia', '', true)});

			menuEntriesCompare.unshift({action: 'remotecomparedocuments', text: _('Compare Document...')});
		}

		this._insertImageMenuSetupDone = true;
	},

	resetAppLoaded: function() {
		this._appLoaded = false;
		for (var key in this._appLoadedConditions) {
			this._appLoadedConditions[key] = false;
		}
	},

	_postLoaded: function(e) {
		app.console.debug('PostMessage: _postLoaded - ' + e.type);

		if (this._appLoaded) {
			return;
		}

		if (e.type === 'docloaded') {
			// doc unloaded
			if (!e.status)
			{
				this._appLoadedConditions[e.type] = false;
				return;
			}

			this.DocumentLoadedTime = Date.now();
		}
		this._appLoadedConditions[e.type] = true;
		for (var key in this._appLoadedConditions) {
			if (!this._appLoadedConditions[key])
				return;
		}

		this._appLoaded = true;
		this.sendDocumentLoaded();
	},

	// Naturally we set a CSP to catch badness, but check here as well.
	// Checking whether a message came from our iframe's parents is
	// un-necessarily difficult.
	_allowMessageOrigin: function(e) {
		// e.origin === 'null' when sandboxed
		if (e.origin === 'null')
			return false;

		// cache - to avoid regexps.
		if (this._cachedGoodOrigin && this._cachedGoodOrigin === e.origin)
			return true;

		try {
			if (e.origin === window.parent.origin)
				return true;
		} catch (secErr) { // security error de-referencing window.parent.origin.
		}

		// sent from the server
		var i;
		if (!this._allowedOrigins && window.frameAncestors)
		{
			var ancestors = window.frameAncestors.trim().split(' ');
			this._allowedOrigins = ancestors;
			// convert to JS regexps from localhost:* to https*://localhost:.*
			for (i = 0; i < ancestors.length; i++) {
				this._allowedOrigins[i] = '^(http|https)://' + ancestors[i].replace(/:\*/, ':?.*') + '$';
			}
		}

		if (this._allowedOrigins)
		{
			for (i = 0; i < this._allowedOrigins.length; i++) {
				if (e.origin.match(this._allowedOrigins[i]))
				{
					this._cachedGoodOrigin = e.origin;
					return true;
				}
			}
		}

		// chrome only
		if (window.location.ancestorOrigins &&
		    window.location.ancestorOrigins.contains(e.origin))
		{
			this._cachedGoodOrigin = e.origin;
			return true;
		}

		const eSignature = this._map.eSignature;
		if (eSignature && eSignature.url === e.origin) {
			// The sender is our esign popup: accept it.
			return true;
		}

		return false;
	},

	_postMessageListener: function(e) {
		if (!this._allowMessageOrigin(e)) {
			window.app.console.error('PostMessage not allowed due to incorrect origin.');
			return;
		}

		var msg;

		if (('data' in e) && Object.hasOwnProperty.call(e.data, 'MessageId')) {
			// when e.data already contains the right props, but isn't JSON (a blob is passed for ex)
			msg = e.data;
		} else if (typeof e.data === 'object') {
			// E.g. the esign popup sends us an object, no need to JSON-parse it.
			msg = e.data;
		} else {
			try {
				msg = JSON.parse(e.data);
			} catch (e) {
				window.app.console.error(e);
				return;
			}
		}

		// allow closing documents before they are completely loaded
		if (msg.MessageId === 'Close_Session') {
			app.socket.sendMessage('closedocument');
			return;
		}

		// Exception: UI modification can be done before WOPIPostmessageReady was fulfilled
		if (msg.MessageId === 'Show_Button' || msg.MessageId === 'Hide_Button' || msg.MessageId === 'Remove_Button') {
			if (!msg.Values) {
				window.app.console.error('Property "Values" not set');
				return;
			}

			if (!msg.Values.id) {
				window.app.console.error('Property "Values.id" not set');
				return;
			}
			var show = msg.MessageId === 'Show_Button';
			this._map.uiManager.showButton(msg.Values.id, show);
			return;
		}
		else if (msg.MessageId === 'Show_Command' || msg.MessageId === 'Hide_Command') {
			if (!msg.Values) {
				window.app.console.error('Property "Values" not set');
				return;
			}

			if (!msg.Values.id) {
				window.app.console.error('Property "Values.id" not set');
				return;
			}
			var show = msg.MessageId === 'Show_Command';
			this._map.uiManager.showCommand(msg.Values.id, show);
			return;
		}
		else if (msg.MessageId === 'Remove_Statusbar_Element') {
			if (!msg.Values) {
				window.app.console.error('Property "Values" not set');
				return;
			}
			if (!msg.Values.id) {
				window.app.console.error('Property "Values.id" not set');
				return;
			}
			// TODO: remove
			window.app.map.statusBar.showItem(msg.Values.id, false);
			return;
		}
		else if (msg.MessageId === 'Show_Menubar') {
			this._map.uiManager.showMenubar();
			return;
		}
		else if (msg.MessageId === 'Hide_Menubar') {
			this._map.uiManager.hideMenubar();
			return;
		}
		else if (msg.MessageId === 'Show_Ruler') {
			this._map.uiManager.showRuler();
			return;
		}
		else if (msg.MessageId === 'Hide_Ruler') {
			this._map.uiManager.hideRuler();
			return;
		}
		else if (msg.MessageId === 'Show_StatusBar') {
			this._map.uiManager.showStatusBar();
			return;
		}
		else if (msg.MessageId === 'Hide_StatusBar') {
			this._map.uiManager.hideStatusBar(false);
			return;
		}
		else if (msg.MessageId === 'Collapse_Notebookbar') {
			this._map.uiManager.collapseNotebookbar();
			return;
		}
		else if (msg.MessageId === 'Extend_Notebookbar') {
			this._map.uiManager.extendNotebookbar();
			return;
		}
		else if (msg.MessageId === 'Show_NotebookTab' || msg.MessageId === 'Hide_NotebookTab') {
			if (!msg.Values) {
				window.app.console.error('Property "Values" not set');
				return;
			}
			if (!msg.Values.id) {
				window.app.console.error('Property "Values.id" not set');
				return;
			}

			let show = msg.MessageId === 'Show_NotebookTab';
			this._map.uiManager.showNotebookTab(msg.Values.id, show);
			return;
		}
		else if (msg.MessageId === 'Show_Sidebar') {
			/* id is optional */
			if (msg.Values) {
				switch (msg.Values.id) {
				case 'Navigator':
				case 'ModifyPage':
				case 'CustomAnimation':
				case 'MasterSlidesPanel':
					this._map.sendUnoCommand(`.uno:${msg.Values.id}`);
					return;
				}
			}
			this._map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			return;
		}
		else if (msg.MessageId === 'Hide_Sidebar') {
			this._map.sendUnoCommand('.uno:SidebarHide');
			return;
		}
		else if (msg.MessageId === 'Show_Menu_Item' || msg.MessageId === 'Hide_Menu_Item') {
			if (!msg.Values) {
				window.app.console.error('Property "Values" not set');
				return;
			}
			if (!msg.Values.id) {
				window.app.console.error('Property "Values.id" not set');
				return;
			}
			if (!this._map.menubar || !this._map.menubar.hasItem(msg.Values.id)) {
				window.app.console.error('Menu item with id "' + msg.Values.id + '" not found.');
				if (this._map.uiManager.getCurrentMode() === 'notebookbar') {
					window.app.console.error('No menu items in notebookbar');
				}
				return;
			}

			if (msg.MessageId === 'Show_Menu_Item') {
				if (!this._map.menubar.showItem(msg.Values.id)) {
					window.app.console.error('Menu entry with id "' + msg.Values.id + '" not found.');
				}
			} else if (!this._map.menubar.hideItem(msg.Values.id)) {
				window.app.console.error('Menu entry with id "' + msg.Values.id + '" not found.');
			}
			return;
		}
		else if (msg.MessageId === 'Insert_Button' &&
			msg.Values && msg.Values.id) {
			this._map.uiManager.insertButton(msg.Values);
			return;
		}
		else if (msg.MessageId === 'Insert_ContextualButton' &&
			msg.Values && msg.Values.id) {
			this._map.uiManager.map.contextToolbar.insertAdditionalContextButton(msg.Values);
			return;
		}
		else if (msg.MessageId === 'Send_UNO_Command' && msg.Values && msg.Values.Command) {
			this._map.sendUnoCommand(msg.Values.Command, msg.Values.Args || '');
			return;
		}
		else if (msg.MessageId === 'Hint_OnscreenKeyboard') {
			window.keyboard.hintOnscreenKeyboard(true);
			return;
		}
		else if (msg.MessageId === 'Hint_NoOnscreenKeyboard') {
			window.keyboard.hintOnscreenKeyboard(false);
			return;
		}
		else if (msg.MessageId === 'Disable_Default_UIAction') {
			// Disable the default handler and action for a UI command.
			// When set to true, the given UI command will issue a postmessage
			// only. For example, UI_Save will be issued for invoking the save
			// command (from the menu, toolbar, or keyboard shortcut) and no
			// action will take place if 'UI_Save' is disabled via
			// the Disable_Default_UIAction command.
			if (msg.Values && msg.Values.action && msg.Values.disable !== undefined) {
				this._map._disableDefaultAction[msg.Values.action] = msg.Values.disable;
			}
			return;
		}
		else if (msg.MessageId === 'Error_Messages') {
			if (msg.Values && msg.Values.list) {
				msg.Values.list.forEach(function (item) {
					if (Object.prototype.hasOwnProperty.call(errorMessages.storage, item.type)) {
						errorMessages.storage[item.type] = item.msg;
					} else if (Object.prototype.hasOwnProperty.call(errorMessages.uploadfile, item.type)) {
						errorMessages.uploadfile[item.type] = item.msg;
					} else if (Object.prototype.hasOwnProperty.call(errorMessages, item.type)) {
						errorMessages[item.type] = item.msg;
					}
				});
			}
		}

		// All following actions must be done after initialization is completed.
		if (!window.WOPIPostmessageReady) {
			window.app.console.error('PostMessage ignored: not ready.');
			return;
		}

		if (msg.MessageId === 'Host_PostmessageReady') {
			// We already have a listener for this in cool.html, so ignore it here
			return;
		}

		if (msg.MessageId === 'Grab_Focus') {
			app.idleHandler._activate();
			app.map.focus();
			return;
		}

		// allow closing documents before they are completely loaded
		if (msg.MessageId === 'Close_Session') {
			app.socket.sendMessage('closedocument');
			return;
		}

		// when user goes idle we have 'this._appLoaded == false'
		if (msg.MessageId === 'Get_User_State') {
			var isIdle = app.idleHandler.isDimActive();
			this._postMessage({msgId: 'Get_User_State_Resp', args: {
				State: (isIdle ? 'idle' : 'active'),
				Elapsed: app.idleHandler.getElapsedFromActivity()
			}});
		}

		// For all other messages, warn if trying to interact before we are completely loaded
		if (!this._appLoaded) {
			window.app.console.error('Collabora Online not loaded yet. Listen for App_LoadingStatus (Document_Loaded) event before using PostMessage API. Ignoring post message \'' + msg.MessageId + '\'.');
			return;
		}

		if (msg.MessageId === 'Set_Settings') {
			if (msg.Values) {
				var alwaysActive = msg.Values.AlwaysActive;
				this._map.options.alwaysActive = !!alwaysActive;
			}
		}
		else if (msg.MessageId === 'Get_Views') {
			this._postViewsMessage('Get_Views_Resp');
		}
		else if (msg.MessageId === 'Reset_Access_Token') {
			app.socket.sendMessage('resetaccesstoken ' + msg.Values.token);
		}
		else if (msg.MessageId === 'Action_Save') {
			var dontTerminateEdit = msg.Values && msg.Values['DontTerminateEdit'];
			var dontSaveIfUnmodified = msg.Values && msg.Values['DontSaveIfUnmodified'];
			var extendedData = msg.Values && msg.Values['ExtendedData'];
			extendedData = encodeURIComponent(extendedData);
			this._notifySave = msg.Values && msg.Values['Notify'];

			this._map.save(dontTerminateEdit, dontSaveIfUnmodified, extendedData);
		}
		else if (msg.MessageId === 'Action_Close') {
			this._map.remove();
		}
		else if (msg.MessageId === 'Action_Fullscreen') {
			app.util.toggleFullScreen();
		}
		else if (msg.MessageId === 'Action_FullscreenPresentation' && this._map.getDocType() === 'presentation') {
			if (msg.Values) {
				var slideNumber;
				if (typeof msg.Values.StartSlideNumber != 'undefined') {
					slideNumber = msg.Values.StartSlideNumber;
				} else if (msg.Values.CurrentSlide) {
					slideNumber = this._map.getCurrentPartNumber();
				}
				this._map.fire('fullscreen',
					       {
						       startSlideNumber: slideNumber
					       });
			} else {
				this._map.fire('fullscreen');
			}
		}
		else if (msg.MessageId === 'Action_Print') {
			this._map.print();
		}
		else if (msg.MessageId === 'Action_Export') {
			if (msg.Values) {
				this._notifySave = msg.Values['Notify'];
				var format = msg.Values.Format;
				var fileName = this._map['wopi'].BaseFileName;
				fileName = fileName.substr(0, fileName.lastIndexOf('.'));
				fileName = fileName === '' ? 'document' : fileName;
				this._map.downloadAs(fileName + '.' + format, format);
			}
		}
		else if (msg.MessageId == 'Action_InsertGraphic') {
			if (msg.Values) {
				this._map.insertURL(msg.Values.url, "graphicurl");
			}
		}
		else if (msg.MessageId == 'Action_InsertMultimedia') {
			if (msg.Values) {
				this._map.insertURL(msg.Values.url, "multimediaurl");
			}
		}
		else if (msg.MessageId == 'Action_CompareDocuments') {
			if (msg.Values) {
				if (msg.Values.filename) {
					// Remember old file name for CompareChangesLabelSection.
					app.writer.compareDocumentOldFileName = msg.Values.filename;
				}

				this._map.insertURL(msg.Values.url, "comparedocumentsurl");
			}
		}
		else if (msg.MessageId == 'Action_InsertLink') {
			if (msg.Values) {
				var link = this._map.makeURLFromStr(msg.Values.url);
				var selection = this._map.getTextForLink();

				var text = selection ? selection.trim()
					: (msg.Values.text ? String(msg.Values.text).trim() : '')
					|| link;

				var command = {
					'Hyperlink.Text': {
						type: 'string',
						value: text
					},
					'Hyperlink.URL': {
						type: 'string',
						value: link
					}
				};
				this._map.sendUnoCommand('.uno:SetHyperlink', command);
				this._map.focus();
			}
		}
		else if (msg.MessageId == 'Action_GetLinkPreview_Resp') {
			var popup = URLPopUpSection.getCurrent();
			if (popup)
				popup.updatePreview(msg.Values);
		}
		else if (msg.MessageId === 'Action_InsertFile') {
			if (msg.Values && (msg.Values.File instanceof Blob)) {
				this._map.fire('insertfile', {file: msg.Values.File});
			}
		}
		else if (msg.MessageId == 'Action_Paste') {
			if (msg.Values && msg.Values.Mimetype && msg.Values.Data) {
				var blob = new Blob(['paste mimetype=' + msg.Values.Mimetype + '\n', msg.Values.Data]);
				app.socket.sendMessage(blob);
			}
		}
		else if (msg.MessageId == 'Action_Copy') {
			// Request the current text selection in some format.
			if (msg.Values && msg.Values.Mimetype && this._map._clip) {
				this._map._clip.setActionCopy(true);
				app.socket.sendMessage('gettextselection mimetype=' + msg.Values.Mimetype);
			}
		}
		else if (msg.MessageId === 'Action_ShowBusy') {
			if (msg.Values && msg.Values.Label) {
				this._map.fire('showbusy', {label: msg.Values.Label});
			}
		}
		else if (msg.MessageId === 'Action_HideBusy') {
			this._map.fire('hidebusy');
		}
		else if (msg.MessageId === 'Get_Export_Formats') {
			var exportFormatsResp = [];
			for (var index in app.file.exportFormats) {
				exportFormatsResp.push({
					Label: app.file.exportFormats[index].label,
					Format: app.file.exportFormats[index].format
				});
			}

			this._postMessage({msgId: 'Get_Export_Formats_Resp', args: exportFormatsResp});
		}
		else if (msg.MessageId === 'Get_Comments') {
			let commentsResp = [];
			const commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			if (commentSection) {
				if (this._map._docLayer._docType === 'spreadsheet') {
					// calcMasterList has raw data for all sheets.
					const masterList = commentSection.sectionProperties.calcMasterList;
					for (let i = 0; i < masterList.length; i++) {
						const data = masterList[i];
						const entry = {
							Id: data.id,
							Author: data.author,
							DateTime: data.dateTime,
							Text: data.text,
						};
						if (data.threaded) {
							entry.Resolved = data.resolved;
							entry.Parent = data.parent;
						}
						commentsResp.push(entry);
					}
				} else {
					const commentList = commentSection.sectionProperties.commentList;
					for (let i = 0; i < commentList.length; i++) {
						const data = commentList[i].sectionProperties.data;
						if (data.trackchange)
							continue;
						commentsResp.push({
							Id: data.id,
							Author: data.author,
							DateTime: data.dateTime,
							Text: commentList[i].sectionProperties.contentText.textContent,
							Resolved: data.resolved,
							Parent: data.parent,
						});
					}
				}
			}
			this._postMessage({msgId: 'Get_Comments_Resp', args: { Comments: commentsResp }});
		}
		else if (msg.MessageId === 'Action_SaveAs') {
			if (msg.Values) {
				if (msg.Values.Filename !== null && msg.Values.Filename !== undefined) {
					this._notifySave = msg.Values['Notify'];
					var nameParts = msg.Values.Filename.split('.');
					var format = undefined;
					if (nameParts.length > 1)
						format = nameParts.pop();
					else {
						this._map.uiManager.showInfoModal('error', _('Error'), _('File name should contain an extension.'), '', _('OK'));
						return;
					}

					var isExport = format === 'pdf' || format === 'epub' || this._map._saveImageToWopi;
					this._map._saveImageToWopi = false;
					if (isExport) {
						this._map.exportAs(msg.Values.Filename);
					} else {
						this._map.showBusy(_('Creating copy...'), false);
						this._map.saveAs(msg.Values.Filename, format);
					}
				}
			}
		}
		else if (msg.MessageId === 'Action_FollowUser') {
			if (msg.Values) {
				this._map._setFollowing(msg.Values.Follow, msg.Values.ViewId);
			}
			else {
				this._map._setFollowing(true, null);
			}
		}
		else if (msg.MessageId === 'Host_VersionRestore') {
			if (msg.Values.Status === 'Pre_Restore') {
				app.socket.sendMessage('versionrestore prerestore');
			}
		}
		else if (msg.MessageId === 'CallPythonScript' &&
			 Object.prototype.hasOwnProperty.call(msg, 'ScriptFile') &&
			 Object.prototype.hasOwnProperty.call(msg, 'Function')) {
			this._map.CallPythonScriptSource = e.source;
			this._map.sendUnoCommand('vnd.sun.star.script:' + msg.ScriptFile + '$' + msg.Function + '?language=Python&location=share', msg.Values);
		}
		else if (msg.MessageId === 'Action_RemoveView') {
			if (msg.Values && msg.Values.ViewId !== null && msg.Values.ViewId !== undefined) {
				app.socket.sendMessage('removesession ' + msg.Values.ViewId);
			}
		}
		else if (msg.MessageId === 'Action_ChangeUIMode') {
			this._map.uiManager.onChangeUIMode({mode: msg.Values.Mode, force: true});
		}
		else if (msg.MessageId === 'Action_Mention') {
			var list = msg.Values.list;
			this._map.mention.openMentionPopup(list);
		}
		else if (msg.MessageId === 'Action_ResolveComment') {
			var docType = this._map._docLayer._docType;
			if (msg.Values && (docType === 'text' || docType === 'spreadsheet')) {
				const commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
				if (commentSection) {
					const comment = commentSection.getComment(msg.Values.Id);
					if (comment && comment.sectionProperties.data.resolved !== 'true'
						&& (docType === 'text' || comment.sectionProperties.data.threaded)) {
						commentSection.resolve(comment);
					}
				}
			}
		}
		else if (msg.MessageId === 'Action_GoToComment') {
			if (msg.Values) {
				var commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
				if (!commentSection) {
					this._sendGoToCommentResp(msg.Values.Id, false, 'Comment section not available');
					return;
				}
				var docType = this._map._docLayer._docType;
				if (docType === 'spreadsheet')
					this._goToCalcComment(commentSection, msg.Values.Id);
				else if (docType === 'text')
					this._goToComment(commentSection, msg.Values.Id);
				else
					this._sendGoToCommentResp(msg.Values.Id, false, 'Unsupported document type'); // TODO: support Draw/Impress
			}
		}
		else if (msg.sender === 'EIDEASY_SINGLE_METHOD_SIGNATURE') {
			// This is produced by the esign popup.
			const eSignature = this._map.eSignature;
			if (eSignature) {
				eSignature.handleSigned(msg);
			}
		}
	},

	_goToComment: function(commentSection, commentId) {
		// Writer and Calc use different types for Id: string vs. integer?
		var comment = commentSection.getComment(commentId);
		if (!comment)
			comment = commentSection.getComment(parseInt(commentId));
		if (!comment) {
			this._sendGoToCommentResp(commentId, false, 'Comment not found');
			return;
		}

		this._map.showComments(true);
		if (comment.sectionProperties.data.resolved === 'true')
			this._map.showResolvedComments(true);

		// Move the cursor to the comment's anchor
		var clickX, clickY;
		var cellRange = comment.sectionProperties.data.cellRange;
		if (this._map._docLayer._docType === 'spreadsheet' && cellRange) {
			var cellRect = this._map._docLayer._cellRangeToTwipRect(cellRange).toRectangle();
			clickX = Math.round(cellRect[0] + cellRect[2] / 2);
			clickY = Math.round(cellRect[1] + cellRect[3] / 2);
		} else {
			var anchorPos = comment.sectionProperties.data.anchorPos;
			clickX = anchorPos[0];
			clickY = anchorPos[1];
		}
		if (clickX && clickY) {
			this._map._docLayer._postMouseEvent('buttondown', clickX, clickY, 1, 1, 0);
			this._map._docLayer._postMouseEvent('buttonup', clickX, clickY, 1, 1, 0);
		}

		commentSection.navigateAndFocusComment(comment);

		if (this._map._docLayer._docType === 'spreadsheet') {
			// The sheet switch and mouse click (which sets cursor to anchor) trigger
			// async events (_onSetPartMsg, onNewDocumentTopLeft, onCellAddressChanged)
			// that would normally hide the comment. Set a guard to prevent that, show
			// the comment, then clear the guard after a timeout to let events settle.
			// 2 s timeout is an arbitrary value, hoped to cover typical cases, and at
			// the same time, not block expected responsiveness, when user expects it
			// to hide.
			var props = commentSection.sectionProperties;
			if (props.doNotHideCommentTimer)
				clearTimeout(props.doNotHideCommentTimer);
			props.doNotHideCommentTimer = setTimeout(function() {
				props.doNotHideCommentTimer = null;
			}, 2000);

			// Finally, an additional operation specific to Calc (maybe also Draw?):
			// it actually shows the comment on mouse hover
			comment.onMouseEnter();
		}

		this._sendGoToCommentResp(commentId, true);
	},

	_goToCalcComment: function(commentSection, commentId) {
		var map = this._map;
		var props = commentSection.sectionProperties;

		// Try to find the comment in the current calcMasterList.
		var entry = props.calcMasterList.find(el => el.id == commentId);
		if (!entry) {
			this._sendGoToCommentResp(commentId, false, 'Comment not found');
			return;
		}

		// If timeout from previous command is still active, stop it; and hide any shown comment,
		// to avoid a case when doNotHideCommentTimer would prevent another comment from hiding.
		if (props.doNotHideCommentTimer)
			clearTimeout(props.doNotHideCommentTimer);
		props.doNotHideCommentTimer = null;
		commentSection.hideAllComments();

		var targetTab = parseInt(entry.tab);
		if (map._docLayer._selectedPart == targetTab) {
			// Already on the right sheet - navigate immediately.
			this._goToComment(commentSection, commentId);
			return;
		}

		// The comment is on a different sheet. Switch to it and wait for
		// sheetgeometrychanged so the geometry is ready for positioning.

		var safetyTimer = null;
		var self = this;

		function cleanup() {
			clearTimeout(safetyTimer);
			map.off('sheetgeometrychanged', onGeometry);
		}

		function onGeometry() {
			cleanup();
			self._goToComment(commentSection, commentId);
		}

		safetyTimer = setTimeout(function() {
			cleanup();
			self._sendGoToCommentResp(commentId, false, 'Timed out waiting for server');
		}, 10000);

		map.once('sheetgeometrychanged', onGeometry);
		map.setPart(targetTab);
	},

	_sendGoToCommentResp: function(commentId, success, errorMsg) {
		var args = { Id: String(commentId), success: success };
		if (errorMsg)
			args.errorMsg = errorMsg;
		this._map.fire('postMessage', {
			msgId: 'Action_GoToComment_Resp',
			args: args
		});
	},

	_postMessage: function(e) {
		if (!this.enabled) { return; }
		var msgId = e.msgId;
		var values = e.args || {};
		if (!!this.PostMessageOrigin && window.parent !== window.self) {
			// Filter out unwanted save request response
			if (msgId === 'Action_Save_Resp') {
				if (!this._notifySave)
					return;

				this._notifySave = false;
			}

			var msg = {
				'MessageId': msgId,
				'SendTime': Date.now(),
				'Values': values
			};
			window.parent.postMessage(JSON.stringify(msg), this.PostMessageOrigin);
		}
	},

	_postViewsMessage: function(messageId) {
		var getMembersRespVal = [];
		for (var viewInfoIdx in this._map._viewInfo) {
			getMembersRespVal.push({
				ViewId: viewInfoIdx,
				UserName: this._map._viewInfo[viewInfoIdx].username,
				UserId: this._map._viewInfo[viewInfoIdx].userid,
				UserExtraInfo: this._map._viewInfo[viewInfoIdx].userextrainfo,
				Color: this._map._viewInfo[viewInfoIdx].color,
				ReadOnly: this._map._viewInfo[viewInfoIdx].readonly,
				IsCurrentView: this._map._docLayer._viewId === parseInt(viewInfoIdx, 10)
			});
		}

		this._postMessage({msgId: messageId, args: getMembersRespVal});
	}
});

// This handler would only get 'enabled' by map if map.options.wopi = true
window.L.Map.addInitHook('addHandler', 'wopi', window.L.Map.WOPI);
