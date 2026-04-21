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

/* global globalThis UIManager */
/* global errorMessages accessToken accessTokenTTL noAuthHeader accessHeader createOnlineModule */
/* global app $ host idleTimeoutSecs outOfFocusTimeoutSecs _ LocaleService LayoutingService DropFileOverlay */
/* global ServerConnectionService createEmscriptenModule */
/*eslint indent: [error, "tab", { "outerIIFEBody": 0 }]*/

(function (global) {


var wopiParams = {};
var wopiSrc = global.coolParams.get('WOPISrc');

if (wopiSrc !== '' && accessToken !== '') {
	wopiParams = { 'access_token': accessToken, 'access_token_ttl': accessTokenTTL };
	if (noAuthHeader == "1" || noAuthHeader == "true") {
		wopiParams.no_auth_header = noAuthHeader;
	}
}
else if (wopiSrc !== '' && accessHeader !== '') {
	wopiParams = { 'access_header': accessHeader };
}

// Debug-only: an explicit user id, sent as a WOPI param (a sibling of the
// access token, NOT part of WOPISrc, so the doc key and collaboration are
// unaffected). The test WOPI host reads it as the UserId. This lets us debug
// multi-user scenarios that the single hard-coded test user cannot.
if (window.enableDebug) {
	var debugUserId = global.coolParams.get('userid');
	if (wopiSrc !== '' && debugUserId)
		wopiParams.userid = debugUserId;
}

var filePath = global.coolParams.get('file_path');

app.localeService = new LocaleService();
app.setPermission(global.coolParams.get('permission') || 'edit');
app.serverConnectionService = new ServerConnectionService();
app.layoutingService = new LayoutingService();
app.pendingOnDemandRenders = 0;
app.pendingOnDemandRenderRequests = new Set();

var timestamp = global.coolParams.get('timestamp');
var target = global.coolParams.get('target') || '';
// Should the document go inactive or not
var alwaysActive = global.coolParams.get('alwaysactive');
// Cool Debug mode
var debugMode = global.coolParams.get('debug');

var docURL, docParams;
var isWopi = false;
if (wopiSrc != '') {
	docURL = decodeURIComponent(wopiSrc);
	docParams = wopiParams;
	isWopi = true;
} else {
	docURL = filePath;
	docParams = {};
}

var notWopiButIframe = global.coolParams.get('NotWOPIButIframe') != '';
var map = window.L.map('map', {
	server: host,
	doc: docURL,
	docParams: docParams,
	timestamp: timestamp,
	docTarget: target,
	documentContainer: 'document-container',
	debug: debugMode,
	// the wopi and wopiSrc properties are in sync: false/true : empty/non-empty
	wopi: isWopi,
	wopiSrc: wopiSrc,
	notWopiButIframe: notWopiButIframe,
	alwaysActive: alwaysActive,
	idleTimeoutSecs: idleTimeoutSecs,  // Dim when user is idle.
	outOfFocusTimeoutSecs: outOfFocusTimeoutSecs, // Dim after switching tabs.
});

////// Controls /////

window.L.Map.THIS = map;
app.map = map;

map.uiManager = new UIManager();
map.addControl(map.uiManager);
if (!window.L.Browser.cypressTest)
	map.tooltip = window.L.control.tooltip();

app.idleHandler.map = map;

// The native shell toggles these while a file is dragged over the window, so
// they must exist in both the starter screen and an open document.
app.showDropOverlay = function () { DropFileOverlay.show(); };
app.hideDropOverlay = function () { DropFileOverlay.hide(); };

if (window.coolParams.get('starterMode')) {
	if (window.ThisIsTheQtApp && !window.qtBridgeReady) {
		window.addEventListener('qtbridgeready', () => {
			map.uiManager.initializeBackstageView();
		}, { once: true });
	} else {
		map.uiManager.initializeBackstageView();
	}
	return;
}

var initEmscriptenModule = function(docKind, docDescriptor) {
	globalThis.Module = createEmscriptenModule(docKind, docDescriptor);
	globalThis.Module.onRuntimeInitialized = function() {
		map.loadDocument(global.socket);
	};
	createOnlineModule(globalThis.Module);
};

var initUI = function() {
	if (window.coolParams.get('welcome'))
		map.uiManager.initializeNonInteractiveUI();
	else
		map.uiManager.initializeBasicUI();

	if (wopiSrc === '' && filePath === '' && !window.ThisIsAMobileApp) {
		map.uiManager.showInfoModal('wrong-wopi-src-modal', '', errorMessages.wrongwopisrc, '', _('OK'), null, false);
	}
	if (host === '' && !window.ThisIsAMobileApp) {
		map.uiManager.showInfoModal('empty-host-url-modal', '', errorMessages.emptyhosturl, '', _('OK'), null, false);
	}
};

if (window.ThisIsTheEmscriptenApp) {
	// Ensure the access token is available on the global object
	// for switchToServerMode (MobileAppInitializer does not set
	// window.accessToken, unlike BrowserAppInitializer).
	if (typeof accessToken !== 'undefined') {
		global.accessToken = accessToken;
		global.accessTokenTTL = accessTokenTTL;
	}

	if (isWopi) {
		// Use collab WebSocket endpoint to get a download URL
		var docParamsString = $.param(docParams);
		var docParamsPart = docParamsString ? (docURL.includes('?') ? '&' : '?') + docParamsString : '';
		var fullDocUrl = docURL + docParamsPart;

		// Set up save callback before initializing the module.
		// The C++ saveToServer() calls this via MAIN_THREAD_EM_ASM
		// with the file bytes, and we handle the token exchange and upload.
		globalThis.collabSaveToServer = function(fileBytes) {
			window.app.console.log('WASM: collabSaveToServer called with ' + fileBytes.length + ' bytes');
			map.fire('showbusy', {label: _('Saving...')});
			global.collabUploadFile(fileBytes).then(function() {
				window.app.console.log('WASM: save completed successfully');
				map.fire('hidebusy');
				if (window._switchToServerAfterSave) {
					window._switchToServerAfterSave = false;
					window.app.console.log('WASM: switching to server mode after save');
					window.collabSendMessage({type: 'saved_and_switching'});
					window.switchToServerMode();
				}
			}).catch(function(err) {
				window.app.console.error('WASM: save failed: ' + err.message);
				map.fire('hidebusy');
				if (window._switchToServerAfterSave) {
					window._switchToServerAfterSave = false;
					window.switchToServerMode();
				}
			});
		};

		global.collabFetchFile(fullDocUrl, accessToken).then(function(result) {
			window.app.console.log('WASM: Using collab fetch URL: ' + result.url);
			if (result.filename) {
				map['wopi'].BaseFileName = result.filename;
				map['wopi'].BreadcrumbDocName = result.filename;
				globalThis.collabFilename = result.filename;
			}
			initUI();
			initEmscriptenModule('collab', result.url);

			// If a collaborative editing session is already active,
			// ask the new user whether they want to join.  Defer
			// to the next tick after updatepermission, because
			// _enterReadOnlyMode fires closealldialogs right after
			// updatepermission.
			if (global.collabEditingActive) {
				var showCollabDialog = function () {
					app.events.off('updatepermission', showCollabDialog);
					setTimeout(function () {
						map._onCollabEditingActive();
					}, 0);
				};
				app.events.on('updatepermission', showCollabDialog);
			}

			// Listen for collab notifications from other users
			global.addCollabNotificationListener(function(msg) {
				if (msg.type === 'editing_started' && msg.user) {
					map._onOtherUserEditingStarted(
						msg.user.name || msg.user.id,
						msg.user.avatar);
				} else if (msg.type === 'switch_to_collab') {
					// Another user wants collaborative editing.
					// Save local changes and switch to server mode.
					map._onSwitchToCollabRequest();
				} else if (msg.type === 'saved_and_switching') {
					// The editor has saved and is switching.
					map._onEditorSavedAndSwitching();
				} else if (msg.type === 'user_left') {
					// If we were waiting and all users left,
					// stop waiting.
					map._onCollabUserLeft();
				}
			});
		}).catch(function(err) {
			window.app.console.error('WASM: Collab fetch failed: ' + err.message + ', falling back to direct fetch');
			initUI();
			// Fallback to old /wasm/ endpoint
			var encodedWOPI = encodeURIComponent(fullDocUrl);
			initEmscriptenModule('server', encodedWOPI);
		});
	} else {
		initUI();
		// Local file, use directly
		initEmscriptenModule('local', docURL);
	}
} else {
	initUI();
	map.loadDocument(global.socket);
}

window.addEventListener('beforeunload', function () {
	if (map && app.socket) {
		if (app.socket.setUnloading)
			app.socket.setUnloading();
		app.socket.close();
	}
});

window.bundlejsLoaded = true;


////// Unsupported Browser Warning /////

var uaLowerCase = navigator.userAgent.toLowerCase();
if (uaLowerCase.indexOf('msie') != -1 || uaLowerCase.indexOf('trident') != -1) {
	map.uiManager.showInfoModal(
		'browser-not-supported-modal', '',
		_('Warning! The browser you are using is not supported.'),
		'', _('OK'), null, false);
}

if (window.ThisIsAMobileApp && !window.ThisIsTheEmscriptenApp && !window.ThisIsTheiOSApp && !window.ThisIsTheAndroidApp)
	window.postMobileMessage('SYNCSETTINGS');

}(window));
