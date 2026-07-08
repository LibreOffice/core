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

// Local save callback shared by COWASM and CODA remote-doc paths.
// Both flavours have already saved the document bytes locally; this
// pushes them up to the integrator via /co/collab/put, then runs any
// queued "save-and-switch-to-server-mode" hand-off.
var _codaUploadOnSave = function(fileBytes, tag) {
	window.app.console.log(tag + ': uploading ' + fileBytes.length + ' bytes');
	map.fire('showbusy', {label: _('Saving...')});
	return global.collabUploadFile(fileBytes).then(function() {
		window.app.console.log(tag + ': upload completed');
		map.fire('hidebusy');
		if (window._switchToServerAfterSave) {
			window._switchToServerAfterSave = false;
			window.app.console.log(tag + ': switching to server mode after save');
			window.collabSendMessage({type: 'saved_and_switching'});
			window.switchToServerMode();
		}
	}).catch(function(err) {
		window.app.console.error(tag + ': upload failed: ' + err.message);
		map.fire('hidebusy');
		if (window._switchToServerAfterSave) {
			window._switchToServerAfterSave = false;
			window.switchToServerMode();
		}
	});
};

// Common per-doc collab-notification wiring (peer joins, peer-driven
// switch requests, etc.).  Used by both COWASM and CODA after the
// /co/collab fetch resolves.
var _codaWireCollabNotifications = function() {
	// If a collaborative editing session is already active, ask the
	// new user whether they want to join.  Defer to the next tick
	// after updatepermission, because _enterReadOnlyMode fires
	// closealldialogs right after updatepermission.
	if (global.collabEditingActive) {
		var showCollabDialog = function () {
			app.events.off('updatepermission', showCollabDialog);
			setTimeout(function () {
				map._onCollabEditingActive();
			}, 0);
		};
		app.events.on('updatepermission', showCollabDialog);
	}

	global.addCollabNotificationListener(function(msg) {
		if (msg.type === 'editing_started' && msg.user) {
			map._onOtherUserEditingStarted(
				msg.user.name || msg.user.id,
				msg.user.avatar);
		} else if (msg.type === 'switch_to_collab') {
			map._onSwitchToCollabRequest();
		} else if (msg.type === 'saved_and_switching') {
			map._onEditorSavedAndSwitching();
		} else if (msg.type === 'user_left') {
			map._onCollabUserLeft();
		}
	});
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
		// with the file bytes.
		globalThis.collabSaveToServer = function(fileBytes) {
			_codaUploadOnSave(fileBytes, 'WASM');
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
			_codaWireCollabNotifications();
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
} else if (window.ThisIsTheQtApp) {
	// CODA-Q: the picker side sets _document._remoteInfo (wopiSrc,
	// accessToken, coolServer, coolPath) on the C++ side before
	// cool.html loads, for remote docs only.  Pull that via
	// Bridge::getRemoteInfo once the QWebChannel bridge is ready; if
	// non-empty, run the COWASM-style /co/collab fetch from JS, then
	// hand the bytes back to native via Bridge::writeRemoteDocFile
	// so LOKit can load them through the standard fakesocket flow.
	// Local-only docs take the default path: initUI + map.loadDocument.
	var _codaQtRemoteBootstrap = function() {
		// On every successful .uno:Save, tell the bridge the save
		// round-trip is over so it can clear _saveInFlight (and run
		// any deferred-close callback).  For a remote doc still in
		// local-edit mode, additionally drive the integrator upload
		// via global.collabWs (closed-over below).  Registered up
		// front rather than inside the remote-doc branch so local-
		// only docs also get _saveInFlight cleared.
		map.on('commandresult', function(ev) {
			if (ev.commandName !== '.uno:Save')
				return;
			var done = function() {
				window.postMobileMessage('SAVECOMPLETED');
				if (window._closeAfterSave) {
					window._closeAfterSave = false;
					window.postMobileMessage('CLOSE_WINDOW');
				}
			};
			if (
				!ev.success ||
				!global.collabWs ||
				global.collabWs.readyState !== WebSocket.OPEN
			) {
				done();
				return;
			}
			new Promise(function(r) {
				window.bridge.readLocalDocBytes(r);
			}).then(function(b64) {
				if (!b64) {
					done();
					return;
				}
				var bin = atob(b64);
				var arr = new Uint8Array(bin.length);
				for (var i = 0; i < bin.length; i++)
					arr[i] = bin.charCodeAt(i);
				_codaUploadOnSave(arr, 'CODA-Q').then(done, done);
			}, done);
		});

		new Promise(function(resolve) {
			window.bridge.getRemoteInfo(resolve);
		}).then(function(infoJson) {
			if (!infoJson) {
				// Local-only document: standard path.
				initUI();
				map.loadDocument(global.socket);
				return;
			}
			var info = JSON.parse(infoJson);
			// Cache for Permission.js's switchToServerMode override,
			// which rebuilds the cool-server /cool/ws URL from these
			// values.
			window._codaRemoteInfo = info;
			window.app.console.log('CODA-Q: remote doc, wopiSrc=' + info.wopiSrc);
			initUI();
			global.collabFetchFile(info.wopiSrc, info.accessToken, info.coolServer).then(function(result) {
				window.app.console.log('CODA-Q: fetch URL: ' + result.url);
				if (result.filename) {
					map['wopi'].BaseFileName = result.filename;
					map['wopi'].BreadcrumbDocName = result.filename;
				}
				return fetch(result.url).then(function(r) {
					if (!r.ok)
						throw new Error('GET ' + result.url + ' returned ' + r.status);
					return r.arrayBuffer();
				}).then(function(buf) {
					// QWebChannel does not marshal ArrayBuffer cleanly,
					// so base64 the body across the JS<->Qt boundary.
					var bytes = new Uint8Array(buf);
					var binary = '';
					for (var i = 0; i < bytes.length; i++)
						binary += String.fromCharCode(bytes[i]);
					var b64 = btoa(binary);
					return new Promise(function(r) {
						window.bridge.writeRemoteDocFile(result.filename || 'document', b64, r);
					});
				}).then(function(localPath) {
					if (!localPath)
						throw new Error('writeRemoteDocFile failed');
					window.app.console.log('CODA-Q: wrote local temp file ' + localPath);
					// Point the FakeWebSocket's load message at the
					// just-written file before opening the socket.
					global.docURL = 'file://' + localPath;
					map.options.doc = global.docURL;
					if (result.filename) {
						var fileName = result.filename;
						document.title = fileName + ' - ' + window.brandProductName;
					}
					// Mirror the auto-fire in global.js (now gated on
					// docURL being non-empty): now that we have a path,
					// kick HULLO + the FakeWebSocket's onopen so
					// coolwsd gets a non-empty 'load url=' message.
					// Must happen *before* map.loadDocument hands the
					// socket to app.socket.connect, which replaces
					// onopen without firing it.
					window.postMobileMessage('HULLO');
					global.socket.onopen();
					map.loadDocument(global.socket);
					_codaWireCollabNotifications();
					// (Save-completion handling - upload-when-collabWs-
					// open or SAVECOMPLETED-when-not - is registered at
					// the top of this bootstrap so it covers both local-
					// only and remote-doc paths uniformly.)
				});
			}).catch(function(err) {
				var msg = err && err.message ? err.message : String(err);
				window.app.console.error(
					'CODA-Q: remote bootstrap failed: ' + msg);
				// Surface the failure as a native error dialog on the
				// doc window rather than just a console message: the
				// page has no visible affordance to report it, so the
				// window would otherwise sit on its pre-load screen.
				window.postMobileMessage(
					'REMOTEBOOTSTRAPFAILED ' + msg);
			});
		});
	};
	if (window.qtBridgeReady)
		_codaQtRemoteBootstrap();
	else
		window.addEventListener('qtbridgeready', _codaQtRemoteBootstrap);
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
