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
/* global app $ host idleTimeoutSecs outOfFocusTimeoutSecs _ LocaleService LayoutingService */
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

var filePath = global.coolParams.get('file_path');

app.localeService = new LocaleService();
app.setPermission(global.coolParams.get('permission') || 'edit');
app.serverConnectionService = new ServerConnectionService();
app.layoutingService = new LayoutingService();

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

map.uiManager = new UIManager();
map.addControl(map.uiManager);
if (!window.L.Browser.cypressTest)
	map.tooltip = window.L.control.tooltip();

window.L.Map.THIS = map;
app.map = map;
app.idleHandler.map = map;

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
else if (window.coolParams.get('welcome'))
	map.uiManager.initializeNonInteractiveUI();
else
	map.uiManager.initializeBasicUI();

if (wopiSrc === '' && filePath === '' && !window.ThisIsAMobileApp) {
	map.uiManager.showInfoModal('wrong-wopi-src-modal', '', errorMessages.wrongwopisrc, '', _('OK'), null, false);
}
if (host === '' && !window.ThisIsAMobileApp) {
	map.uiManager.showInfoModal('empty-host-url-modal', '', errorMessages.emptyhosturl, '', _('OK'), null, false);
}

if (window.ThisIsTheEmscriptenApp) {
	var docParamsString = $.param(docParams);
	// The URL may already contain a query (e.g., 'http://server.tld/foo/wopi/files/bar?desktop=baz') - then just append more params
	var docParamsPart = docParamsString ? (docURL.includes('?') ? '&' : '?') + docParamsString : '';
	var encodedWOPI = encodeURIComponent(docURL + docParamsPart);

	globalThis.Module = createEmscriptenModule(
		isWopi ? 'server' : 'local', isWopi ? encodedWOPI : docURL);
	globalThis.Module.onRuntimeInitialized = function() {
		map.loadDocument(global.socket);
	};
	createOnlineModule(globalThis.Module);
} else {
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
