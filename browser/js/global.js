/* -*- js-indent-level: 8; fill-column: 100 -*- */

/* global Module ArrayBuffer Uint8Array _ */

/*
	For extending window.app object, please see "docstate.ts" file.
	Below definition is only for the properties that this (global.js) file needs at initialization.
*/
window.app = {
	socket: null,
	console: {}
};

// For typings (including the global object), please see browser/src/global.d.ts

// This function may look unused, but it's needed in Android to send data through the fake websocket. Please
// don't remove it without first grepping for 'Base64ToArrayBuffer' in the C++ code
// eslint-disable-next-line
var Base64ToArrayBuffer = function(base64Str) {
	var binStr = atob(base64Str);
	var ab = new ArrayBuffer(binStr.length);
	var bv = new Uint8Array(ab);
	for (var i = 0, l = binStr.length; i < l; i++) {
		bv[[i]] = binStr.charCodeAt(i);
	}
	return ab;
};

// Written and named as a sort of analog to plain atob ... except this one supports non-ascii
// Nothing is perfect so this also mangles binary - don't decode tiles with it
// This function may look unused, but it's needed in mobile to send data through the fake websocket. Please
// don't remove it without first grepping for 'Base64ToArrayBuffer' in the C++ code
// eslint-disable-next-line
var b64d = function(base64Str) {
	var binStr = atob(base64Str);
	var u8Array = Uint8Array.from(binStr, c => c.codePointAt(0));
	return new TextDecoder().decode(u8Array);
}

// Put these into a class to separate them better.
class BrowserProperties {
	static initiateBrowserProperties(global) {
		global.L = {};

		let ua = navigator.userAgent.toLowerCase();
		let uv = navigator.vendor.toLowerCase();
		let doc = document.documentElement;

		let cypressTest = ua.indexOf('cypress') !== -1;

		// Firefox has undefined navigator.clipboard.read and navigator.clipboard.write,
		// unsecure contexts (such as http + non-localhost) has the entire navigator.clipboard
		// undefined.
		let clipboardApiAvailable = navigator.clipboard !== undefined && navigator.clipboard.write !== undefined && navigator.clipboard.read !== undefined;

		let webkit    = ua.indexOf('webkit') !== -1;
		let chrome    = ua.indexOf('chrome') !== -1;
		let gecko     = (ua.indexOf('gecko') !== -1 || (cypressTest && 'MozUserFocus' in doc.style)) && !webkit && !global.opera;
		let safari    = !chrome && (ua.indexOf('safari') !== -1 || uv.indexOf('apple') == 0);

		let win = navigator.platform.indexOf('Win') === 0;

		let mobile = typeof orientation !== 'undefined' || ua.indexOf('mobile') !== -1;
		let msPointer = !global.PointerEvent && global.MSPointerEvent;
		let pointer = (global.PointerEvent && navigator.pointerEnabled && navigator.maxTouchPoints) || msPointer;

		let webkit3d = ('WebKitCSSMatrix' in global) && ('m11' in new global.WebKitCSSMatrix());
		let gecko3d = 'MozPerspective' in doc.style;

		var mac = navigator.appVersion.indexOf('Mac') != -1 || navigator.userAgent.indexOf('Mac') != -1;
		var chromebook = global.ThisIsTheAndroidApp && global.COOLMessageHandler.isChromeOS();

		var navigatorLang = navigator.languages && navigator.languages.length ? navigator.languages[0] :
		(navigator.language || navigator.userLanguage || navigator.browserLanguage || navigator.systemLanguage);

		function getFirefoxVersion() {
			var version = '';

			var userAgent = navigator.userAgent.toLowerCase();
			if (userAgent.indexOf('firefox') !== -1) {
				var matches = userAgent.match(/firefox\/([0-9]+\.*[0-9]*)/);
				if (matches) {
					version = matches[1];
				}
			}
			return version;
		}

		window.L.Browser = {
			// @property edge: Boolean
			// `true` for the Edge web browser.
			edge: 'msLaunchUri' in navigator && !('documentMode' in document),

			// @property webkit: Boolean
			// `true` for webkit-based browsers like Chrome and Safari (including mobile versions).
			webkit: webkit,

			// @property gecko: Boolean
			// `true` for gecko-based browsers like Firefox.
			gecko: gecko,

			// @property geckoVersion: String
			// Firefox version: abc.d.
			geckoVersion: getFirefoxVersion(),

			// @property android: Boolean
			// `true` for any browser running on an Android platform.
			android: ua.indexOf('android') !== -1,

			// @property chrome: Boolean
			// `true` for the Chrome browser.
			chrome: chrome,

			// @property safari: Boolean
			// `true` for the Safari browser.
			safari: safari,

			// @property win: Boolean
			// `true` when the browser is running in a Windows platform
			win: win,

			// @property mac: Boolean
			// `true` when the browser is running in a Mac platform
			mac: mac,

			// @property any3d: Boolean
			// `true` for all browsers supporting CSS transforms.
			any3d: !global.L_DISABLE_3D && (webkit3d || gecko3d),

			// @property mobile: Boolean
			// `true` for all browsers running in a mobile device.
			mobile: mobile,

			// @property mobileWebkit: Boolean
			// `true` for all webkit-based browsers in a mobile device.
			mobileWebkit: mobile && webkit,

			// @property cypressTest: Boolean
			// `true` when the browser run by cypress
			cypressTest: cypressTest,


			// @property clipboardApiAvailable: Boolean
			// `true` when permission-based clipboard api is available.
			clipboardApiAvailable: clipboardApiAvailable,

			// @property msPointer: Boolean
			// `true` for browsers implementing the Microsoft touch events model (notably IE10).
			msPointer: !!msPointer,

			// @property pointer: Boolean
			// `true` for all browsers supporting [pointer events](https://msdn.microsoft.com/en-us/library/dn433244%28v=vs.85%29.aspx).
			pointer: !!pointer,

			// @property retina: Boolean
			// `true` for browsers on a high-resolution "retina" screen.
			retina: (global.devicePixelRatio || (global.screen.deviceXDPI / global.screen.logicalXDPI)) > 1,

			// @property lang: String
			// browser language locale
			lang: navigatorLang
		};

		global.mode = {
			isChromebook: function() {
				return chromebook;
			},
			// This typically means a mobile phone. Has to match small screen size requirement.
			isSmallScreenDevice: function() {
				if (global.ThisIsTheWindowsApp || global.ThisIsTheQtApp || global.ThisIsTheMacOSApp)
					return false;

				if (global.mode.isChromebook())
					return false;

				if (global.L.Browser.mobile && global.L.Browser.cypressTest) {
					return true;
				}

				return global.L.Browser.mobile && (screen.width < 768 || screen.height < 768);
			},
			// Mobile device with big screen size.
			isTablet: function() {
				if (global.ThisIsTheWindowsApp || global.ThisIsTheQtApp || global.ThisIsTheMacOSApp)
					return false;

				if (global.mode.isChromebook())
					return false;

				return global.L.Browser.mobile && !global.mode.isSmallScreenDevice();
			},
			isCODesktop: function() {
				return global.ThisIsTheMacOSApp || global.ThisIsTheQtApp || global.ThisIsTheWindowsApp;
			},
			isNewDocument: function() {
				return window.coolParams.get('isnewdocument');
			},
			isDesktop: function() {
				if (global.ThisIsTheWindowsApp || global.ThisIsTheQtApp	|| global.ThisIsTheMacOSApp)
					return true;

				if (global.mode.isChromebook())
					return true;

				return !global.L.Browser.mobile;
			},
			getDeviceFormFactor: function() {
				if (global.mode.isSmallScreenDevice())
					return 'mobile';
				else if (global.mode.isTablet())
					return 'tablet';
				else if (global.mode.isDesktop())
					return 'desktop';
				else
					return null;
			}
		};
	}
}

class InitializerBase {
	constructor() {
		BrowserProperties.initiateBrowserProperties(window);

		this.uriPrefix = document.getElementById('init-uri-prefix').value;
		this.brandingUriPrefix = this.uriPrefix;

		window.welcomeUrl = document.getElementById("init-welcome-url") ? document.getElementById("init-welcome-url").value: "";
		window.feedbackUrl = document.getElementById("init-feedback-url") ? document.getElementById("init-feedback-url").value: "";
		window.buyProductUrl = document.getElementById("init-buy-product-url") ? document.getElementById("init-buy-product-url").value: "";
		let initCSSVars = document.getElementById("init-css-vars") ? document.getElementById("init-css-vars").value: "";

		if (initCSSVars) {
			initCSSVars = atob(initCSSVars);
			const sheet = new CSSStyleSheet();
			if (typeof sheet.replace === 'function')
			{
				sheet.replace(initCSSVars);
				document.adoptedStyleSheets.push(sheet);
			} // else jsdom
		}

		const element = window.L.initial = document.getElementById("initial-variables");
		window.L.initial._stubMessage = function () {};

		window.host = "";
		window.serviceRoot = "";
		window.hexifyUrl = false;
		window.versionPath = "";
		window.accessToken = element.dataset.accessToken;
		window.accessTokenTTL = element.dataset.accessTokenTtl;
		window.noAuthHeader = element.dataset.noAuthHeader;
		window.accessHeader = element.dataset.accessHeader;
		window.postMessageOriginExt = "";
		window.coolwsdVersion = "";
		window.enableWelcomeMessage = false;
		window.autoShowWelcome = false;
		window.autoShowFeedback = true;
		window.allowUpdateNotification = false;
		window.useIntegrationTheme = false;
		window.enableMacrosExecution = false;
		window.enableAccessibility = false;
		window.protocolDebug = false;
		window.enableDebug = false;
		window.frameAncestors = "";
		window.socketProxy = false;
		window.uiDefaults = {};
		window.useStatusbarSaveIndicator = false;
		window.checkFileInfoOverride = {};
		window.deeplEnabled = false;
		window.zoteroEnabled = false;
		window.savedUIState = true;
		window.extraExportFormats = [];
		window.wasmEnabled = false;
		window.indirectionUrl = "";
		window.geolocationSetup = false;
		window.canvasSlideshowEnabled = false;
		window.wopiSettingBaseUrl = element.dataset.wopiSettingBaseUrl;
		window.enableExperimentalFeatures = element.dataset.enableExperimentalFeatures === 'true';

		window.tileSize = 256;

		window.ThisIsAMobileApp = false;
		window.ThisIsTheiOSApp = false;
		window.ThisIsTheAndroidApp = false;
		window.ThisIsTheEmscriptenApp = false;
		window.ThisIsTheQtApp = false;

		window.bundlejsLoaded = false;
		window.fullyLoadedAndReady = false;
		window.addEventListener('load', function() {
			window.fullyLoadedAndReady = true;

			const contentKeeper = document.getElementById('content-keeper');
			while (contentKeeper.children.length > 0)
				document.body.insertBefore(contentKeeper.children[contentKeeper.children.length - 1], document.body.firstChild);

			document.getElementById('content-keeper').remove();
		}, false);

		let productName = document.getElementById("init-product-branding-name").value;
		if (typeof productName === 'string' && productName.length) {
			window.brandProductName = productName;
		}
		let productURL = document.getElementById("init-product-branding-url").value;
		if (typeof productURL === 'string' && productURL.length) {
			window.brandProductURL = productURL;
		}
		let logoURL = document.getElementById("init-logo-url").value;
		if (typeof logoURL === 'string' && logoURL.length) {
			window.logoURL= logoURL;
		}

		this.initiateCoolParams();
	}

	initiateCoolParams() {
		const gls = window.location.search;

		const coolParams = { p: new URLSearchParams(gls.slice(gls.lastIndexOf('?') + 1)) };

		/* We need to return an empty string instead of `null` */
		coolParams.get = function(name) {
			const value = this.p.get(name);
			return value === null ? '' : value;
		}.bind(coolParams);

		coolParams.set = function(name, value) {
			this.p.set(name, value);
		}.bind(coolParams);

		window.coolParams = coolParams;

		// Returns true if the simulateError param matches the check
		window.simulateError = function(name) {
			return coolParams.get('simulateError') === name;
		};

		// Starter Screen: Show BackstageView without document when app launches
		window.starterScreen = coolParams.get('starterMode') === 'true';
	}

	loadCSSFiles() {
		// Dynamically load the appropriate *-mobile.css, *-tablet.css or *-desktop.css
		const link = document.createElement('link');
		link.setAttribute("rel", "stylesheet");
		link.setAttribute("type", "text/css");

		const brandingLink = document.createElement('link');
		brandingLink.setAttribute("rel", "stylesheet");
		brandingLink.setAttribute("type", "text/css");

		const theme_name = document.getElementById('init-branding-name').value;
		let theme_prefix = '';

		if(window.useIntegrationTheme && theme_name !== '')
			theme_prefix = theme_name + '/';

		if (window.mode.isSmallScreenDevice()) {
			link.setAttribute("href", this.uriPrefix + 'device-mobile.css');
			brandingLink.setAttribute("href", this.brandingUriPrefix + theme_prefix + 'branding-mobile.css');
		} else if (window.mode.isTablet()) {
			link.setAttribute("href", this.uriPrefix + 'device-tablet.css');
			brandingLink.setAttribute("href", this.brandingUriPrefix + theme_prefix + 'branding-tablet.css');
		} else {
			link.setAttribute("href", this.uriPrefix + 'device-desktop.css');
			brandingLink.setAttribute("href", this.brandingUriPrefix + theme_prefix + 'branding-desktop.css');
		}

		const otherStylesheets = document.querySelectorAll('link[rel="stylesheet"]');
		const lastOtherStylesheet = otherStylesheets[otherStylesheets.length - 1];

		lastOtherStylesheet
			.insertAdjacentElement('afterend', link)
			.insertAdjacentElement('afterend', brandingLink);
	}

	initializeViewMode() {
		const darkTheme = window.coolParams.get('darkTheme');
		if (darkTheme) { window.uiDefaults = { 'darkTheme': 'true' }; }
	}

	afterInitialization() {
		this.initializeViewMode();
		this.loadCSSFiles();
	}
}

class BrowserInitializer extends InitializerBase {
	constructor() {
		super();

		window.WOPIpostMessageReady = false;

		// Start listening for Host_PostmessageReady message and save the result for future
		this._boundPostMessageHandler = this.postMessageHandler.bind(this);
		window.addEventListener('message', this._boundPostMessageHandler, false);

		const element = document.getElementById("initial-variables");

		window.host = element.dataset.host;
		window.serviceRoot = element.dataset.serviceRoot;
		window.hexifyUrl = element.dataset.hexifyUrl.toLowerCase().trim() === "true";
		window.versionPath = element.dataset.versionPath;

		window.postMessageOriginExt = element.dataset.postMessageOriginExt;
		window.coolLogging = element.dataset.coolLogging;
		window.coolwsdVersion = element.dataset.coolwsdVersion;
		window.enableWelcomeMessage = element.dataset.enableWelcomeMessage.toLowerCase().trim() === "true";
		window.autoShowWelcome = element.dataset.autoShowWelcome.toLowerCase().trim() === "true";
		window.autoShowFeedback = element.dataset.autoShowFeedback.toLowerCase().trim() === "true";
		window.allowUpdateNotification = element.dataset.allowUpdateNotification.toLowerCase().trim() === "true";
		window.userInterfaceMode = element.dataset.userInterfaceMode;
		window.useIntegrationTheme = element.dataset.useIntegrationTheme.toLowerCase().trim() === "true";
		window.useStatusbarSaveIndicator = element.dataset.statusbarSaveIndicator.toLowerCase().trim() === "true";
		window.enableMacrosExecution = element.dataset.enableMacrosExecution.toLowerCase().trim() === "true";
		window.enableAccessibility = element.dataset.enableAccessibility.toLowerCase().trim() === "true";
		window.outOfFocusTimeoutSecs = parseInt(element.dataset.outOfFocusTimeoutSecs);
		window.idleTimeoutSecs = parseInt(element.dataset.idleTimeoutSecs);
		window.minSavedMessageTimeoutSecs = parseInt(element.dataset.minSavedMessageTimeoutSecs);
		window.protocolDebug = element.dataset.protocolDebug.toLowerCase().trim() === "true";
		window.enableDebug = element.dataset.enableDebug.toLowerCase().trim() === "true";
		window.frameAncestors = decodeURIComponent(element.dataset.frameAncestors);
		window.socketProxy = element.dataset.socketProxy.toLowerCase().trim() === "true";
		window.uiDefaults = JSON.parse(atob(element.dataset.uiDefaults));
		window.checkFileInfoOverride = element.dataset.checkFileInfoOverride;
		window.deeplEnabled = element.dataset.deeplEnabled.toLowerCase().trim() === "true";
		window.zoteroEnabled = element.dataset.zoteroEnabled.toLowerCase().trim() === "true";
		window.documentSigningEnabled = element.dataset.documentSigningEnabled.toLowerCase().trim() === "true";
		window.savedUIState = element.dataset.savedUiState.toLowerCase().trim() === "true";
		window.extraExportFormats = Array.from(element.dataset.extraExportFormats.split(" "));
		window.wasmEnabled = element.dataset.wasmEnabled.toLowerCase().trim() === "true";
		window.indirectionUrl = element.dataset.indirectionUrl;
		window.geolocationSetup = element.dataset.geolocationSetup.toLowerCase().trim() === "true";
		window.canvasSlideshowEnabled = element.dataset.canvasSlideshowEnabled.toLowerCase().trim() === "true";
		window.wopiSettingBaseUrl = element.dataset.wopiSettingBaseUrl;
		window.wopiHostId = element.dataset.wopiHostId;
		window.vendor = element.dataset.vendor;
		window.copyrightYear = element.dataset.copyrightYear;
	}

	postMessageHandler(e) {
		if (!(e && e.data))
			return;

		try {
			var msg = JSON.parse(e.data);
		} catch (err) {
			return;
		}

		if (msg.MessageId === 'Host_PostmessageReady') {
			window.WOPIPostmessageReady = true;
			window.removeEventListener('message', this._boundPostMessageHandler, false);
			console.log('Received Host_PostmessageReady.');
		}
	}
}

class MobileAppInitializer extends InitializerBase {
	constructor() {
		super();

		window.ThisIsAMobileApp = true;
		window.HelpFile = document.getElementById("init-help-file").value;

		// Related to issue #5841: the mobile apps set the base text direction via the "dir" parameter
		document.dir = window.coolParams.get('dir');

		// stash this so we can use it for presenter console despite
		// MobileAppInitializer redirection of general 'open' use
		window.origOpen = window.open;
		// eslint-disable-next-line
		window.open = function (url, windowName, windowFeatures) {
		  window.postMobileMessage('HYPERLINK ' + url); /* don't call the 'normal' window.open on mobile at all */
		};

		const element = document.getElementById("initial-variables");

		window.MobileAppName = element.dataset.mobileAppName;
		window.brandProductName = element.dataset.mobileAppName;

		window.coolLogging = "true";
		window.outOfFocusTimeoutSecs = 1000000;
		window.idleTimeoutSecs = 1000000;

		window.canvasSlideshowEnabled = true;
	}
}

class IOSAppInitializer extends MobileAppInitializer {
	constructor() {
		super();

		window.ThisIsTheiOSApp = true;
		window.postMobileMessage = function(msg) { window.webkit.messageHandlers.lok.postMessage(msg); };
		window.postMobileCall    = window.postMobileMessage;
		window.postMobileError   = function(msg) { window.webkit.messageHandlers.error.postMessage(msg); };
		window.postMobileDebug   = function(msg) { window.webkit.messageHandlers.debug.postMessage(msg); };

		window.userInterfaceMode = window.coolParams.get('userinterfacemode');

		this.brandingUriPrefix = "Branding/" + this.brandingUriPrefix;
	}
}

class MacOSAppInitializer extends MobileAppInitializer {
	constructor() {
		super();

		window.ThisIsTheMacOSApp = true;
		window.postMobileMessage = function(msg) { return window.webkit.messageHandlers.lok.postMessage(msg); };
		window.postMobileCall    = window.postMobileMessage;
		window.postMobileError   = function(msg) { return window.webkit.messageHandlers.error.postMessage(msg); };
		window.postMobileDebug   = function(msg) { return window.webkit.messageHandlers.debug.postMessage(msg); };

		window.userInterfaceMode = window.coolParams.get('userinterfacemode');
	}
}

class WindowsAppInitializer extends MobileAppInitializer {
	constructor() {
		super();

		window.ThisIsTheWindowsApp = true;
		window.postMobileMessage = function(msg) { window.chrome.webview.postMessage('MSG ' + msg); };

		// Here, and elsewhere, things would be nicer if we just used JSON for all our
		// messages, instead of plain text with a home-grown syntax of mostly "command
		// parameter1=value..."  but also some cases of "command JSON".

		// The name "postMobileCall" is a bit misleading as this isn't just "posting" a
		// message like the other postMobileFoo() functions, but to be used when a return
		// value is expected. In other platforms, the postMobileMessage() can be used for
		// that, too, but not on Windows. The WebView2 does not have the required
		// functionality built-in.
		window.postMobileCall = (() => {
			let nextId = 1;
			const pending = new Map();

			window.replyFromNativeToCall = (id, reply) => {
				const resolveFunc = pending.get(id);
				pending.delete(id);
				resolveFunc(reply);
			};

			return function call(msg) {
				return new Promise((resolveFunc) => {
					const id = nextId++;
					pending.set(id, resolveFunc);
					window.chrome.webview.postMessage("CALL " + id + " " + msg);
				});
			};
		})();

		// FIXME: No registration of separate handlers in Windows WebView2, so just log
		// errors and debug messages? Maybe instead send a JSON object with separate name
		// and body? But then we would have to parse that JSON object from the string in C#
		// anyway.
		window.postMobileError   = function(msg) { window.chrome.webview.postMessage('ERR ' + msg); };
		window.postMobileDebug   = function(msg) { window.chrome.webview.postMessage('DBG ' + msg); };

		window.userInterfaceMode = window.coolParams.get('userinterfacemode');
	}
}

class AndroidAppInitializer extends MobileAppInitializer {
	constructor() {
		super();

		window.ThisIsTheAndroidApp = true;
		window.postMobileMessage = function(msg) { window.COOLMessageHandler.postMobileMessage(msg); };
		window.postMobileCall    = window.postMobileMessage;
		window.postMobileError   = function(msg) { window.COOLMessageHandler.postMobileError(msg); };
		window.postMobileDebug   = function(msg) { window.COOLMessageHandler.postMobileDebug(msg); };

		window.userInterfaceMode = window.coolParams.get('userinterfacemode');
	}
}

class EMSCRIPTENAppInitializer extends MobileAppInitializer {
	constructor() {
		super();

		window.ThisIsTheEmscriptenApp = true;
		window.postMobileMessage = function(msg) { Module._handle_cool_message(Module.stringToNewUTF8(msg)); };
		window.postMobileCall    = window.postMobileMessage;
		window.postMobileError   = function(msg) { console.log('COOL Error: ' + msg); };
		window.postMobileDebug   = function(msg) { console.log('COOL Debug: ' + msg); };

		window.userInterfaceMode = 'notebookbar';
	}
}

class QtAppInitializer extends MobileAppInitializer {
	constructor() {
		super();
		window.ThisIsTheQtApp = true;

		const messageQueue = [];

		// Define safe stub functions that queue messages, as QWebChannel doesn't initialize immediately.
		window.postMobileMessage = (msg) => messageQueue.push({ type: 'cool', msg });
		window.postMobileCall    = window.postMobileMessage;
		window.postMobileError   = (msg) => messageQueue.push({ type: 'error', msg });
		window.postMobileDebug   = (msg) => messageQueue.push({ type: 'debug', msg });
		window.userInterfaceMode = window.coolParams.get('userinterfacemode');

		// Initialize QWebChannel and replace stubs when bridge is ready
		// eslint-disable-next-line no-undef
		new QWebChannel(qt.webChannelTransport, (channel) => {
			const bridge = channel.objects.bridge;
			window.bridge = bridge;

			// Replace stubs with real implementations
			window.postMobileMessage = (msg) => window.bridge.cool(msg);
			window.postMobileCall    = window.postMobileMessage;
			window.postMobileError   = (msg) => window.bridge.error(msg);
			window.postMobileDebug   = (msg) => window.bridge.debug(msg);

			// Flush queued messages
			for (const { type, msg } of messageQueue) {
				if (typeof bridge[type] === 'function') {
					bridge[type](msg);
				}
			}
			messageQueue.length = 0;

			window.qtBridgeReady = true;
			window.dispatchEvent(new Event('qtbridgeready'));
			if (bridge.debug) {
				bridge.debug("Qt bridge initialized");
			}
		});
	}
}

function getInitializerClass() {
	window.appType = document.getElementById("init-app-type").value;

	if (window.appType === "browser") {
		return new BrowserInitializer();
	}
	else if (window.appType === "mobile") {
		let osType = document.getElementById("init-mobile-app-os-type");

		if (osType) {
			osType = osType.value;

			if (osType === "IOS")
				return new IOSAppInitializer();
			else if (osType === "MACOS")
				return new MacOSAppInitializer();
			else if (osType === "WINDOWS")
				return new WindowsAppInitializer();
			else if (osType === "ANDROID")
				return new AndroidAppInitializer();
			else if (osType === "EMSCRIPTEN")
				return new EMSCRIPTENAppInitializer();
			else if (osType === "QT")
				return new QtAppInitializer();
		}
	}
}

function showWelcomeSVG() {
	const loaderDiv = document.createElement('div');
	loaderDiv.id = 'welcome-loader';
	loaderDiv.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100vw;
        height: 100vh;
        background-color: #faf8fc;
        z-index: 9999999999;
        opacity: 1;
        transition: opacity 0.8s ease-out;
    `;

	const img = document.createElement('img');
	img.src = 'welcome/welcome.svg';
	img.alt = 'Welcome';
	img.style.cssText = `
        width: 100%;
        height: 100%;
        object-fit: fill;
    `;

	loaderDiv.appendChild(img);
	document.body.insertBefore(loaderDiv, document.body.firstChild);
}

(function (global) {
	const initializer = getInitializerClass();
	initializer.afterInitialization();

	global.logServer = function (log) {
		if (global.ThisIsAMobileApp) {
			global.postMobileError(log);
		} else if (global.socket && (global.socket instanceof WebSocket || global.socket instanceof global.IndirectSocket) && global.socket.readyState === 1) {
			global.socket.send(log);
		} else if (global.socket && global.L && global.app.definitions.Socket &&
			   (global.socket instanceof global.app.definitions.Socket) && global.socket.connected()) {
			global.socket.sendMessage(log);
		} else {
			fetch(global.location.pathname.match(/.*\//) + 'browser-logging', {
				method: 'POST',
				headers: { 'Content-Type' : 'application/json' },
				body: global.coolLogging + ' ' + log
			});
		}
	};

	// enable later toggling
	global.setLogging = function(doLogging)
	{
		var loggingMethods = ['error', 'warn', 'info', 'debug', 'trace', 'log', 'assert', 'time', 'timeEnd', 'group', 'groupEnd'];
		if (!doLogging) {
			var noop = function() {};

			for (var i = 0; i < loggingMethods.length; i++) {
				global.app.console[loggingMethods[i]] = noop;
			}
		} else {
			for (var i = 0; i < loggingMethods.length; i++) {
				if (!Object.prototype.hasOwnProperty.call(global.console, loggingMethods[i])) {
					continue;
				}
				(function(method) {
					global.app.console[method] = function logWithCool() {
						var args = Array.prototype.slice.call(arguments);
						if (method === 'error') {
							var log = 'jserror ';
							for (var arg = 0; arg < arguments.length; arg++) {
								if (typeof arguments[arg] === 'string')
									log += arguments[arg] + '\n';
							}
							global.logServer(log);
						}

						// Can use optional chaining if we increase the ecma version
						if (global.L && global.L.Map && global.L.Map.THIS &&
								global.L.Map.THIS._debug && global.L.Map.THIS._debug.logTrace === true) {
							console.groupCollapsed("Trace");
							console.trace();
							console.groupEnd();
						}

						return global.console[method].apply(console, args);
					};
				}(loggingMethods[i]));
			}

			global.onerror = function (msg, src, row, col, err) {
				var data = {
					userAgent: navigator.userAgent.toLowerCase(),
					vendor: navigator.vendor.toLowerCase(),
					message: msg,
					source: src,
					line: row,
					column: col
				};
				var desc = err ? err.message || '(no message)': '(no err)', stack = err ? err.stack || '(no stack)': '(no err)';
				var log = 'jserror ' + JSON.stringify(data, null, 2) + '\n' + desc + '\n' + stack + '\n';
				global.logServer(log);

				if (L.Browser.cypressTest && window.parent !== window && err !== null) {
					console.log("Sending global error to Cypress...:", err);
					window.parent.postMessage(err);
				}

				return false;
			};
		}
	};

	global.setLogging(global.coolLogging != '');

	function parseBool(val) {
		if (typeof val !== 'string') return false;
		switch (val.toLowerCase().trim()) {
		case '1':
		case 'true':
		case 'yes':
		case 'on':
			return true;
		case '0':
		case 'false':
		case 'no':
		case 'off':
			return false;
		default:
			return false;
		}
	}

	global.L.Params = {
		/// Shows close button if non-zero value provided
		closeButtonEnabled: parseBool(global.coolParams.get('closebutton')),

		/// Shows revision history file menu option
		revHistoryEnabled: parseBool(global.coolParams.get('revisionhistory')),
	};

	global.prefs = {
		_localStorageCache: {}, // TODO: change this to new Map() when JS version allows
		_userBrowserSetting: new Map(),
		_settingUpdateJSON: {},
		_pendingSettingUpdate: undefined,
		useBrowserSetting: false,
		canPersist: (function() {
			var str = 'localstorage_test';
			try {
				global.localStorage.setItem(str, str);
				global.localStorage.removeItem(str);
				return true;
			} catch (e) {
				return false;
			}
		})(),

		_initializeBrowserSetting: function (msg) {
			let settingJSON = JSON.parse(msg.substring('browsersetting:'.length + 1));;

			if (typeof settingJSON === 'undefined')
				return;

			const processObject = (object, parentKey = '') => {
				Object.keys(object).forEach((key) => {
					const fullKey = parentKey ? `${parentKey}.${key}` : key;
					const value = object[key];

					if (typeof value === 'object' && !Array.isArray(value) && value !== null) {
						processObject(value, fullKey);
					} else if (Array.isArray(value)) {
						global.prefs._userBrowserSetting[fullKey] = JSON.stringify(value);
					} else {
						global.prefs._userBrowserSetting[fullKey] =
							typeof value === 'boolean' ? (value ? "true" : "false") : value;
					}
				});
			};

			processObject(settingJSON);

			global.prefs._localStorageCache = {};
			global.prefs.useBrowserSetting = true;

			// make sure set accessibilityState for cypress
			global.getAccessibilityState();
			if (this._initialized) {
				window.dispatchEvent(new Event("browsersettingchanged"));
			}
			this._initialized = true;
		},

		_renameLocalStoragePref: function(oldName, newName) {
			if (!global.prefs.canPersist) {
				return;
			}

			const oldValue = global.localStorage.getItem(oldName);
			const newValue = global.localStorage.getItem(newName);

			if (oldValue === null || newValue !== null) {
				return;
			}

			// we do not remove the old value, both for downgrades and in case we split an old global preference to a per-app one
			global.localStorage.setItem(newName, oldValue);
		},

		/// Similar to using window.uiDefaults directly, but this can handle dotted keys like "presentation.ShowSidebar" and does not allow partially referencing a value (like just "presentation")
		_getUIDefault: function(key, defaultValue = undefined) {
			const parts = key.split('.');
			let result = global.uiDefaults;

			for (const part of parts) {
				if (!Object.prototype.hasOwnProperty.call(result, part)) {
					return defaultValue;
				}

				if (typeof result === 'string') {
					return defaultValue;
				}

				result = result[part];
			}

			if (typeof result !== 'string') {
				return defaultValue;
			}

			return result;
		},

		get: function(key, defaultValue = undefined) {
			if (global.prefs._localStorageCache[key] !== undefined) {
				return global.prefs._localStorageCache[key];
			}

			const uiDefault = global.prefs._getUIDefault(key);
			if (
				!global.savedUIState &&
				uiDefault !== undefined
			) {
				global.prefs._localStorageCache[key] = uiDefault;
				return uiDefault;
			}

			if (global.prefs.useBrowserSetting) {
				let val = defaultValue;
				if (Object.prototype.hasOwnProperty.call(global.prefs._userBrowserSetting, key))
					val = global.prefs._userBrowserSetting[key];

				if(val !== undefined && val !== '') {
					global.prefs._localStorageCache[key] = val;
					return val;
				}
			}

			if (global.prefs.canPersist) {
				const localStorageItem = global.localStorage.getItem(key);

				if (localStorageItem) {
					global.prefs._localStorageCache[key] = localStorageItem;
					return localStorageItem;
				}
			}

			if (uiDefault !== undefined) {
				global.prefs._localStorageCache[key] = uiDefault;
				return uiDefault;
			}

			global.prefs._localStorageCache[key] = defaultValue;
			return defaultValue;
		},

		sendPendingBrowserSettingsUpdate: function() {
			const isEmpty = (obj) => Object.keys(obj).length === 0;
			if (!isEmpty(global.prefs._settingUpdateJSON)) {
				global.socket.send('browsersetting action=update json=' + JSON.stringify(global.prefs._settingUpdateJSON));
				global.prefs._settingUpdateJSON = {};
			}
			clearTimeout(global.prefs._pendingSettingUpdate);
			global.prefs._pendingSettingUpdate = undefined;
		},

		// set multiple preference together and when browsersetting is enabled send
		// update only once
		setMultiple: function (prefsObject) {
			const browserSettingEnabled = global.prefs.useBrowserSetting;
			for (const [key, value] of Object.entries(prefsObject)) {
				if (browserSettingEnabled) {
					const oldValue = global.prefs._userBrowserSetting[key];
					global.prefs._userBrowserSetting[key] = value;
					if (oldValue !== value)
						global.prefs._settingUpdateJSON[key] = value;
				}
				if (global.prefs.canPersist) {
					global.localStorage.setItem(key, value);
				}
				global.prefs._localStorageCache[key] = value;
			}

			const isEmpty = (obj) => Object.keys(obj).length === 0;
			if (browserSettingEnabled && !isEmpty(global.prefs._settingUpdateJSON) && global.socket && (global.socket instanceof WebSocket || global.socket instanceof global.IndirectSocket) && global.socket.readyState === 1) {
				clearTimeout(global.prefs._pendingSettingUpdate);
				global.prefs._pendingSettingUpdate = setTimeout(L.bind(this.sendPendingBrowserSettingsUpdate, this), 5000);
			}
		},

		set: function(key, value) {
			value = String(value); // NOT "new String(...)". We cannot use .toString here because value could be null/undefined
			if (global.prefs.useBrowserSetting) {
				const oldValue = global.prefs._userBrowserSetting[key];
				global.prefs._userBrowserSetting[key] = value;
				if (global.socket && (global.socket instanceof WebSocket || global.socket instanceof global.IndirectSocket) && global.socket.readyState === 1 && oldValue !== value) {
					global.prefs._settingUpdateJSON[key] = value;
					clearTimeout(global.prefs._pendingSettingUpdate);
					global.prefs._pendingSettingUpdate = setTimeout(L.bind(this.sendPendingBrowserSettingsUpdate, this), 5000);
				}
			}
			if (global.prefs.canPersist) {
				global.localStorage.setItem(key, value);
			}
			global.prefs._localStorageCache[key] = value;
		},

		remove: function(key) {
			if (global.prefs.useBrowserSetting) {
				global.prefs._userBrowserSetting.delete(key);
			}
			if (global.prefs.canPersist) {
				global.localStorage.removeItem(key);
			}
			delete global.prefs._localStorageCache[key];
		},

		getBoolean: function(key, defaultValue = false) {
			const value = global.prefs.get(key, '').toLowerCase();

			if (value === 'false') {
				return false;
			}

			if (value === 'true') {
				return true;
			}

			return defaultValue;
		},

		getNumber: function(key, defaultValue = NaN) {
			const value = global.prefs.get(key, '').toLowerCase();

			const parsedValue = parseFloat(value);

			if (isNaN(parsedValue)) {
				return defaultValue;
			}

			return parsedValue;
		},
	};

	global.getAccessibilityState = function () {
		var isCalcTest =
			global.docURL.includes('data/desktop/calc/') ||
			global.docURL.includes('data/mobile/calc/') ||
			global.docURL.includes('data/idle/calc/') ||
			global.docURL.includes('data/multiuser/calc/');

		// FIXME: a11y doesn't work in calc under cypress
		if (L.Browser.cypressTest && isCalcTest)
			global.enableAccessibility = false;

		if (L.Browser.cypressTest)
			global.prefs.set('accessibilityState', global.enableAccessibility);

		return global.prefs.getBoolean('accessibilityState');
	};

	// Renamed in 24.04.4.1
	const prefDocTypes = ['text', 'spreadsheet', 'presentation', 'drawing'];
	for (const docType of prefDocTypes) {
		global.prefs._renameLocalStoragePref(`UIDefaults_${docType}_darkTheme`, 'darkTheme');
	}

	const oldDocTypePrefs = [
		"A11yCheckDeck",
		"NavigatorDeck",
		"PropertyDeck",
		"SdCustomAnimationDeck",
		"SdMasterPagesDeck",
		"SdSlideTransitionDeck",
		"ShowResolved",
		"ShowRuler",
		"ShowSidebar",
		"ShowStatusbar",
		"ShowToolbar",
	];
	for (const pref of oldDocTypePrefs) {
		for (const docType of prefDocTypes) {
			global.prefs._renameLocalStoragePref(`UIDefaults_${docType}_${pref}`, `${docType}.${pref}`);
		}
	}
	// End 24.04.4.1 renames

	global.keyboard = {
		onscreenKeyboardHint: global.uiDefaults['onscreenKeyboardHint'],
		// If there's an onscreen keyboard, we don't want to trigger it with innocuous actions like panning around a spreadsheet
		// on the other hand, if there is a hardware keyboard we want to do things like focusing contenteditables so that typing is
		// recognized without tapping again. This is an impossible problem, because browsers do not give us enough information
		// Instead, let's just guess
		guessOnscreenKeyboard: function() {
			if (global.ThisIsTheWindowsApp || global.ThisIsTheQtApp || global.ThisIsTheMacOSApp)
				return false;
			if (global.keyboard.onscreenKeyboardHint != undefined) return global.keyboard.onscreenKeyboardHint;
			return (global.ThisIsAMobileApp && !global.ThisIsTheEmscriptenApp) || global.mode.isSmallScreenDevice() || global.mode.isTablet();
			// It's better to guess that more devices will have an onscreen keyboard than reality,
			// because calc becomes borderline unusable if you miss a device that pops up an onscreen keyboard which covers
			// a sizeable portion of the screen
		},
		// alternatively, maybe someone else (e.g. an integrator) knows more about the situation than we do. In this case, let's
		// let them override our default
		hintOnscreenKeyboard: function(hint) {
			if (global.app
					&& global.L.Map
					&& global.L.Map.THIS._docLayer.isCalc()
					&& hint !== undefined) {
				var command = {
					Enable: {
						type: 'boolean',
						value: hint
					}
				};
				global.L.Map.THIS.sendUnoCommand('.uno:MoveKeepInsertMode', command);
			}
			global.keyboard.onscreenKeyboardHint = hint;
		},
	};

	global.memo = {
		_lastId: 0,

		/// This does pretty much the same as L.stamp. We can't use L.stamp because it's not yet in-scope by the first time we want to call global.memo.decorator
		/// If you are able to use L.stamp instead, you probably should
		_getId: function(obj) {
			if (obj === null || obj === undefined) {
				return '' + obj;
			}
			if (!('_coolMemoId' in obj)) {
				obj['_coolMemoId'] = ++global.memo._lastId;
			}
			return obj._coolMemoId;
		},

		_decoratorMemo: {},

		/// A decorator factory, which takes a decorator and prevents it from creating new instances when wrapping the same function
		/// This is particularly useful for functions that take events, say, as .on and .off won't work properly if you don't provide the same function instance
		decorator: function(decorator, context) {
			var decoratorId = global.memo._getId(decorator);
			var contextId = global.memo._getId(context);

			return function(f) {
				var functionId = global.memo._getId(f);

				if (global.memo._decoratorMemo[decoratorId + ' ' + contextId + ' ' + functionId] === undefined) {
					global.memo._decoratorMemo[decoratorId + ' ' + contextId + ' ' + functionId] = decorator.apply(this, arguments);

					if (context !== null && context !== undefined) {
						global.memo._decoratorMemo[decoratorId + ' ' + contextId + ' ' + functionId] = global.memo._decoratorMemo[decoratorId + ' ' + contextId + ' ' + functionId].bind(context);
					}
				}

				return global.memo._decoratorMemo[decoratorId + ' ' + contextId + ' ' + functionId];
			};
		},

		_bindMemo: {},

		/// A decorator, which takes a function and binds it to an object
		/// Similar to L.bind, but when given the same function and context we will return the previously bound function
		bind: function(f, context) {
			var functionId = global.memo._getId(f);
			var contextId = global.memo._getId(context);
			if (global.memo._bindMemo[functionId + ' ' + contextId] === undefined) {
				global.memo._bindMemo[functionId + ' ' + contextId] = f.bind(context);
			}
			return global.memo._bindMemo[functionId + ' ' + contextId];
		}
	};

	global.touch = {
		/// a touchscreen event handler, supports both DOM and hammer.js events
		isTouchEvent: function(e) {
			if (e.originalEvent) {
				e = e.originalEvent;
			}

			if (L.Browser.cypressTest && global.L.Browser.mobile) {
				return true; // As cypress tests on mobile tend to use "click" events instead of touches... we cheat to get them recognized as touch events
			}

			if (e.pointerType) {
				return e.pointerType === 'touch' || e.pointerType === 'kinect';
			}

			if (e.isMouseEvent !== undefined) {
				return !e.isMouseEvent;
			}

			if (e.guessEmulatedFromTouch) {
				return true;
			}

			return !(e instanceof MouseEvent);
		},

		/// a decorator that only runs the function if the event is a touch event
		touchOnly: global.memo.decorator(function(f) {
			return function(e) {
				if (!global.touch.isTouchEvent(e)) return;
				return f.apply(this, arguments);
			};
		}),

		/// a decorator that only runs the function if the event is not a touch event
		mouseOnly: global.memo.decorator(function(f) {
			return function(e) {
				if (global.touch.isTouchEvent(e)) return;
				return f.apply(this, arguments);
			};
		}),

		/// detect if the primary pointing device is of limited accuracy (generally a touchscreen)
		/// you shouldn't use this for determining the behavior of an event (use isTouchEvent instead), but this may
		///   be useful for determining what UI to show (e.g. the draggable teardrops under the cursor)
		hasPrimaryTouchscreen: function() {
			return global.matchMedia('(pointer: coarse)').matches;
		},
		/// detect any pointing device is of limited accuracy (generally a touchscreen)
		/// you shouldn't use this for determining the behavior of an event (use isTouchEvent instead), but this may
		///   be useful for determining what UI to show (e.g. the draggable teardrops under the cursor)
		hasAnyTouchscreen: function() {
			return global.matchMedia('(any-pointer: coarse)').matches;
		},

		/// a tristate (boolean | null) determining whether the last event was a touch event
		/// may be useful to supplement hasAnyTouchscreen or hasPrimaryTouchscreen for, for example, determining UI or
		///   hitboxes after a tap in a place where you can't sensibly figure out whether the direct trigger was a
		///   touchscreen. Examples might be click events that are roundtripped through core
		/// is null when no touch or click events have yet occurred, true when the last touch or click event was from a
		///   touchscreen, and false when the last touch or click event was from a mouse
		/// is updated with active listeners during the capture phase of the <html> element, so should be done before
		///   most other event processing takes place
		lastEventWasTouch: null,

		/// a timestamp to indicate when lastEventWasTouch was last set
		/// internally used to determine if hover (mouseover/out/enter/leave) events are likely from a mouse or from a
		/// touch event
		lastEventTime: null,

		/// detect if the last event was a touch event, or if no events have yet occurred whether we have a touchscreen
		///   available to us. Should be able to replace uses of hasAnyTouchscreen for uses where we are OK with the
		///   result being less stable
		currentlyUsingTouchscreen: function() {
			if (global.touch.lastEventWasTouch !== null) {
				return global.touch.lastEventWasTouch;
			}

			return global.touch.hasAnyTouchscreen();
		},
	};

	const registerTapOrClick = (e) => {
		registerGuessEmulatedFromTouch(e);
		global.touch.lastEventWasTouch = global.touch.isTouchEvent(e);
		global.touch.lastEventTime = Date.now();
	};
	const registerGuessEmulatedFromTouch = (e) => {
		// on some touchscreens (e.g. iPads) these MouseEvents are emulated for the movement caused by touch events,
		// leading to tooltips erroneously triggering ... these are all movement events so don't necessarily need a click,
		// but for touch events they will happen around other touch events so we can still tell what they are
		e.guessEmulatedFromTouch = global.touch.lastEventWasTouch && Date.now() - global.touch.lastEventTime < 50;
	};
	document.addEventListener('touchstart', registerTapOrClick, { capture: true });
	document.addEventListener('touchend', registerTapOrClick, { capture: true });
	document.addEventListener('mousedown', registerTapOrClick, { capture: true });
	document.addEventListener('mouseup', registerTapOrClick, { capture: true });
	document.addEventListener('pointerdown', registerTapOrClick, { capture: true });
	document.addEventListener('pointerup', registerTapOrClick, { capture: true });
	document.addEventListener('mouseenter', registerGuessEmulatedFromTouch, { capture: true });
	document.addEventListener('mouseleave', registerGuessEmulatedFromTouch, { capture: true });
	document.addEventListener('mouseover', registerGuessEmulatedFromTouch, { capture: true });
	document.addEventListener('mouseout', registerGuessEmulatedFromTouch, { capture: true });

	global.deviceFormFactor = global.mode.getDeviceFormFactor();

	if (global.ThisIsTheiOSApp) {
		global.addEventListener('keydown', function(e) {
			if (e.metaKey) {
				e.preventDefault();
			}
			if (global.MagicKeyDownHandler)
				global.MagicKeyDownHandler(e);
		});
		global.addEventListener('keyup', function(e) {
			if (e.metaKey) {
				e.preventDefault();
			}
			if (global.MagicKeyUpHandler)
				global.MagicKeyUpHandler(e);
		});
	}

	document.addEventListener('contextmenu', function(e) {
		if (e.preventDefault) {
			e.preventDefault();
		} else {
			e.returnValue = false;
		}
	}, false);

	global.fakeWebSocketCounter = 0;
	global.FakeWebSocket = function () {
		this.binaryType = 'arraybuffer';
		this.bufferedAmount = 0;
		this.extensions = '';
		this.protocol = '';
		this.readyState = 1;
		this.id = global.fakeWebSocketCounter++;
		this.onclose = function() {
		};
		this.onerror = function() {
		};
		this.onmessage = function() {
		};
		this.onopen = function() {
		};
		this.close = function() {
		};
	};
	global.FakeWebSocket.prototype.send = function(data) {
		global.postMobileMessage(data);
	};

	global.proxySocketCounter = 0;
	global.ProxySocket = function (uri) {
		var that = this;
		this.uri = uri;
		this.binaryType = 'arraybuffer';
		this.bufferedAmount = 0;
		this.extensions = '';
		this.unloading = false;
		this.protocol = '';
		this.connected = true;
		this.readyState = 0; // connecting
		this.sessionId = 'open';
		this.id = global.proxySocketCounter++;
		this.msgInflight = 0;
		this.openInflight = 0;
		this.inSerial = 1; // monotonic serial of the last processed received message
		this.outSerial = 1; // monotonic serial of the next message to send.
		this.minPollMs = 25; // Anything less than ~25 ms can overwhelm the HTTP server.
		this.maxPollMs = 500; // We can probably go as much as 1-2 seconds without ill-effect.
		this.curPollMs = this.minPollMs; // The current poll period.
		this.minIdlePollsToThrottle = 3; // This many 'no data' responses and we throttle.
		this.throttleFactor = 1.15; // How rapidly to throttle. 15% takes 4s to go from 25 to 500ms.
		this.lastDataTimestamp = performance.now(); // The last time we got any data.
		this.serialQueue = new Map();
		this.onclose = function() {
		};
		this.onerror = function() {
		};
		this.onmessage = function() {
		};

		this.decoder = new TextDecoder();
		this.doSlice = function(bytes,start,end) { return bytes.slice(start,end); };

		this.decode = function(bytes,start,end) {
			return this.decoder.decode(this.doSlice(bytes, start,end));
		};
		this.processBufferedMessages = function (expectedSerial) {
			while (this.serialQueue.has(expectedSerial)) {
				let bufferedMessage = this.serialQueue.get(expectedSerial);
				this.inSerial = bufferedMessage.serial;

				try {
					this.onmessage({ data: bufferedMessage.data });
				} catch (e) {
					global.app.console.error(e);
					global.app.console.warn(`Failed processing a ProxySocket message (due to ${e}), ignoring`);
					// It's better to ignore any failures rather than to lose the rest of the messages in this packet
				}

				this.serialQueue.delete(expectedSerial);
				expectedSerial++;
			}
		},
		this.parseIncomingArray = function(arr) {
			// global.app.console.debug('proxy: parse incoming array of length ' + arr.length);
			for (var i = 0; i < arr.length; ++i)
			{
				var left = arr.length - i;
				if (left < 4)
				{
					//global.app.console.debug('no data left');
					break;
				}
				var type = String.fromCharCode(arr[i+0]);
				if (type != 'T' && type != 'B')
				{
					global.app.console.debug('wrong data type: ' + type);
					break;
				}
				i++;

				// Serial
				if (arr[i] !== 48 && arr[i+1] !== 120) // '0x'
				{
					global.app.console.debug('missing hex preamble');
					break;
				}
				i += 2;
				var numStr = '';
				var start = i;
				while (arr[i] != 10) // '\n'
					i++;
				numStr = this.decode(arr, start, i);
				var serial = parseInt(numStr, 16);

				i++; // skip \n

				// Size:
				if (arr[i] !== 48 && arr[i+1] !== 120) // '0x'
				{
					global.app.console.debug('missing hex preamble');
					break;
				}
				i += 2;
				start = i;
				while (arr[i] != 10) // '\n'
					i++;
				numStr = this.decode(arr, start, i);
				var size = parseInt(numStr, 16);

				i++; // skip \n

				var data;
				if (type == 'T')
					data = this.decode(arr, i, i + size);
				else
					data = this.doSlice(arr, i, i + size);

				this.serialQueue.set(serial, {
					'data': data,
					'serial': serial
				});
				this.processBufferedMessages(serial);

				i += size; // skip trailing '\n' in loop-increment
			}
		};
		this.sendQueue = '';
		this._signalErrorClose = function() {
			clearInterval(this.pollInterval);
			clearTimeout(this.delaySession);
			this.pollInterval = undefined;
			this.delaySession = undefined;

			if (that.readyState < 3)
			{
				this.onerror();
				this.onclose();
			}
			this.sessionId = 'open';
			this.inSerial = 0;
			this.outSerial = 0;
			this.msgInflight = 0;
			this.openInflight = 0;
			this.readyState = 3; // CLOSED
		};
		// For those who think that long-running sockets are a
		// better way to wait: you're so right. However, each
		// consumes a scarce server worker thread while it waits,
		// so ... back in the real world:
		this._setPollInterval = function(intervalMs) {
			clearInterval(this.pollInterval);
			if (this.readyState === 1)
				this.pollInterval = setInterval(this.doSend, intervalMs);
		},
		this.doSend = function () {
			if (that.sessionId === 'open')
			{
				if (that.readyState === 3)
					global.app.console.debug('Error: sending on closed socket');
				return;
			}

			if (that.msgInflight >= 4) // something went badly wrong.
			{
				// We shouldn't get here because we throttle sending when we
				// have something in flight, but if the server hangs, we
				// will do up to 3 retries before we end up here and yield.
				if (that.curPollMs < that.maxPollMs)
				{
					that.curPollMs = Math.min(that.maxPollMs, that.curPollMs * that.throttleFactor) | 0;
					global.app.console.debug('High latency connection - too much in-flight, throttling to ' + that.curPollMs + ' ms.');
					that._setPollInterval(that.curPollMs);
				}
				else if (performance.now() - that.lastDataTimestamp > 30 * 1000)
				{
					global.app.console.debug('Close connection after no response for 30secs');
					that._signalErrorClose();
				}
				else
					global.app.console.debug('High latency connection - too much in-flight, pausing.');
				return;
			}

			// Maximize the timeout, instead of stopping altogethr,
			// so we don't hang when the following request takes
			// too long, hangs, throws, etc. we can recover.
			that._setPollInterval(that.maxPollMs);

			//global.app.console.debug('send msg - ' + that.msgInflight + ' on session ' +
			//	      that.sessionId + '  queue: "' + that.sendQueue + '"');
			var req = new XMLHttpRequest();
			const url = that.getEndPoint('write');
			req.open('POST', url);
			req.responseType = 'arraybuffer';
			req.addEventListener('load', function() {
				if (this.status == 200)
				{
					var data = new Uint8Array(this.response);
					if (data.length)
					{
						// We have some data back from WSD.
						// Another user might be editing and we want
						// to see their changes in real time.
						that.curPollMs = that.minPollMs; // Drain fast.
						that._setPollInterval(that.curPollMs);
						that.lastDataTimestamp = performance.now();

						that.parseIncomingArray(data);
						return;
					}
				}
				else
				{
					global.app.console.debug('proxy: error on incoming response ' + this.status);
					that._signalErrorClose();
				}

				if (that.curPollMs < that.maxPollMs) // If we aren't throttled, see if we should.
				{
					// Has it been long enough since we got any data?
					var timeSinceLastDataMs = (performance.now() - that.lastDataTimestamp) | 0;
					if (timeSinceLastDataMs >= that.minIdlePollsToThrottle * that.curPollMs)
					{
						// Throttle.
						that.curPollMs = Math.min(that.maxPollMs, that.curPollMs * that.throttleFactor) | 0;
						//global.app.console.debug('No data for ' + timeSinceLastDataMs + ' ms -- throttling to ' + that.curPollMs + ' ms.');
					}
				}

				that._setPollInterval(that.curPollMs);
			});
			req.addEventListener('loadend', function() {
				that.msgInflight--;
			});
			const toSend = that.sendQueue;
			that.sendQueue = '';
			that.msgInflight++;
			// terminate all messages with an end-marker
			req.send(toSend.concat('.'));
		};
		this.getSessionId = function() {
			if (this.openInflight > 0)
			{
				global.app.console.debug('Waiting for session open');
				return;
			}

			if (this.delaySession)
				return;

			// avoid attempting to re-connect too quickly
			if (global.lastCreatedProxySocket)
			{
				var msSince = performance.now() - global.lastCreatedProxySocket;
				if (msSince < 250) {
					var delay = 250 - msSince;
					global.app.console.debug('Wait to re-try session creation for ' + delay + 'ms');
					this.curPollMs = delay; // ms
					this.delaySession = setTimeout(function() {
						that.delaySession = undefined;
						that.getSessionId();
					}, delay);
					return;
				}
			}
			global.lastCreatedProxySocket = performance.now();

			var req = new XMLHttpRequest();
			const endPoint = that.getEndPoint('open');

			req.open('POST', endPoint);
			req.responseType = 'text';
			req.addEventListener('load', function() {
				global.app.console.debug('got session: ' + this.responseText);
				if (this.status !== 200 || !this.responseText ||
				    this.responseText.indexOf('\n') >= 0) // multi-line error
				{
					global.app.console.debug('Error: failed to fetch session id! error: ' + this.status);
					that._signalErrorClose();
				}
				else // we connected - lets get going ...
				{
					that.sessionId = this.responseText;
					that.readyState = 1;
					that.onopen();
					that._setPollInterval(that.curPollMs);
				}
			});
			req.addEventListener('loadend', function() {
				global.app.console.debug('Open completed state: ' + that.readyState);
				that.openInflight--;
			});
			req.send('');
			this.openInflight++;
		};
		this.send = function(msg) {
			var hadData = this.sendQueue.length > 0;
			this.sendQueue = this.sendQueue.concat(
				'B0x' + this.outSerial.toString(16) + '\n' +
				'0x' + (new TextEncoder().encode(msg)).length.toString(16) + '\n' + msg + '\n');
			this.outSerial++;

			// Send ASAP, if we have throttled.
			if (that.curPollMs > that.minPollMs || !hadData)
			{
				// Unless we are backed up.
				if (that.msgInflight <= 3)
				{
					//global.app.console.debug('Have data to send, lowering poll interval.');
					that.curPollMs = that.minPollMs;
					that._setPollInterval(that.curPollMs);
				}
			}
		};
		this.sendCloseMsg = function(beacon) {
			const url = that.getEndPoint('close');

			if (!beacon)
			{
				var req = new XMLHttpRequest();
				req.open('POST', url);
				req.send('');
			}
			else
				navigator.sendBeacon(url, '');
		};
		this.close = function() {
			var oldState = this.readyState;
			global.app.console.debug('proxy: close socket');
			this.readyState = 3;
			this.onclose();
			clearInterval(this.pollInterval);
			clearTimeout(this.delaySession);
			this.pollInterval = undefined;
			if (oldState === 1) // was open
				this.sendCloseMsg(this.unloading);
			this.sessionId = 'open';
		};
		this.setUnloading = function() {
			this.unloading = true;
		};
		this.getEndPoint = function(command) {
			var base = this.uri;
			return base + '/' + this.sessionId + '/' + command + '/' + this.outSerial;
		};
		global.app.console.debug('proxy: new socket ' + this.id + ' ' + this.uri);

		// queue fetch of session id.
		this.getSessionId();
	};

	class MobileSocket extends global.ProxySocket {
		constructor(url) {
			super("cool:/cool/mobilesocket" + url);

			delete this.send;
			delete this._setPollInterval;
			delete this.close;
			// HACK: We need this to complete the override because ProxySocket messed up the protoype chain... evenually I want to convert it to a Real Class which will fix it
		}

		send(data) {
			global.postMobileMessage(data);
		}

		close() {} // We don't support re-opening the mobile socket, so let's make sure we don't close it...
		_setPollInterval() {} // This is a no-op on mobile since as we will be calling from the native part to notify when we get a message
	}

	global.iterateCSSImages = function(visitor) {
		var visitUrls = function(rules, visitor, base) {
			if (!rules)
				return;

			for (var r = 0; r < rules.length; ++r) {
				// check subset of rules like @media or @import
				if (rules[r] && rules[r].type != 1) {
					visitUrls(rules[r].cssRules || rules[r].rules, visitor, base);
					continue;
				}
				if (!rules[r] || !rules[r].style)
					continue;
				var img = rules[r].style.backgroundImage;
				if (img === '' || img === undefined)
					continue;

				if (img.startsWith('url("images/'))
				{
					visitor(rules[r].style, img,
						img.replace('url("images/', base + '/images/'));
				}
				if (img.startsWith('url("remote/'))
				{
					visitor(rules[r].style, img,
						img.replace('url("remote/', base + '/remote/'));
				}
			}
		};
		var sheets = document.styleSheets;
		for (var i = 0; i < sheets.length; ++i) {
			var relBases;
			try {
				relBases = sheets[i].href.split('/');
			} catch (err) {
				global.app.console.log('Missing href from CSS number ' + i);
				continue;
			}
			relBases.pop(); // bin last - css name.
			var base = 'url("' + relBases.join('/');

			var rules;
			try {
				rules = sheets[i].cssRules || sheets[i].rules;
			} catch (err) {
				global.app.console.log('Missing CSS from ' + sheets[i].href);
				continue;
			}
			visitUrls(rules, visitor, base);
		}
	};

	if (global.socketProxy)
	{
		// re-write relative URLs in CSS - somewhat grim.
		global.addEventListener('load', function() {
			global.iterateCSSImages(
				function(style, img, fullUrl)
				{
					style.backgroundImage = fullUrl;
				});
		} , false);
	}

	// indirect socket to wrap the asyncness around fetching the routetoken from indirection url endpoint
	global.IndirectSocket = function(uri) {
		var that = this;
		this.uri = uri;
		this.binaryType = '';
		this.unloading = false;
		this.readyState = 0; // connecting
		this.innerSocket = undefined;

		this.onclose = function() {};
		this.onerror = function () {};
		this.onmessage = function () {};
		this.onopen = function () {};

		this.close = function() {
			this.innerSocket.close();
		};

		this.send = function(msg) {
			this.innerSocket.send(msg);
		};

		this.setUnloading = function() {
			this.unloading = true;
		};

		this.sendPostMsg = function(errorCode) {
			var errorMsg;
			if (errorCode === 0) {
				errorMsg = _('The system is currently adjusting resources. Please wait a moment while we retry your request...');
			} else if (errorCode === 1) {
				errorMsg = _('The document is being migrated to a new server. Retrying shortly...');
			} else {
				errorMsg = _('Failed to get RouteToken from controller');
			}
			var msg = {
				'MessageId': 'Action_Load_Resp',
				'SendTime': Date.now(),
				'Values': {
					success: false,
					errorMsg: errorMsg,
					errorType: 'clusterscaling'
				}
			};
			global.parent.postMessage(JSON.stringify(msg), '*');
		};

		this.sendRouteTokenRequest = function (requestUri) {
			var http = new XMLHttpRequest();
			// let url = global.indirectionUrl + '?Uri=' + encodeURIComponent(that.uri);
			http.open('GET', requestUri, true);
			http.responseType = 'json';
			http.addEventListener('load', function () {
				if (this.status === 200) {
					var uriWithRouteToken = http.response.uri;
					global.expectedServerId = http.response.serverId;
					var params = (new URL(uriWithRouteToken)).searchParams;
					global.routeToken = params.get('RouteToken');
					global.app.console.log('updated routeToken: ' + global.routeToken);
					that.innerSocket = new WebSocket(uriWithRouteToken);
					that.innerSocket.binaryType = that.binaryType;
					that.innerSocket.onerror = function () {
						that.readyState = that.innerSocket.readyState;
						that.onerror();
					};
					that.innerSocket.onclose = function () {
						that.readyState = 3;
						that.onclose();
						that.innerSocket.onerror = function () { };
						that.innerSocket.onclose = function () { };
						that.innerSocket.onmessage = function () { };
					};
					that.innerSocket.onopen = function () {
						that.readyState = 1;
						that.onopen();
					};
					that.innerSocket.onmessage = function (e) {
						that.readyState = that.innerSocket.readyState;
						that.onmessage(e);
					};
				} else if (this.status === 202) {
					if (!(window.app && window.app.socket && window.app.socket._reconnecting)) {
						that.sendPostMsg(http.response.errorCode);
					}
					var timeoutFn = function (requestUri) {
						console.warn('Requesting again for routeToken');
						this.open('GET', requestUri, true);
						this.send();
					}.bind(this);
					setTimeout(timeoutFn, 3000, requestUri);
				} else {
					global.app.console.error('Indirection url: error on incoming response ' + this.status);
					that.sendPostMsg(-1);
				}
			});
			http.send();
		};

		let requestUri = global.indirectionUrl + '?Uri=' + encodeURIComponent(that.uri);
		if (global.geolocationSetup) {
			let timeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
			requestUri += "&TimeZone=" + timeZone;
		}
		this.sendRouteTokenRequest(requestUri);
	};

	global.createWebSocket = function(uri) {
		if ('processCoolUrl' in global) {
			uri = global.processCoolUrl({ url: uri, type: 'ws' });
		}

		if (global.ThisIsAMobileApp) {
			return new MobileSocket(uri);
		} else if (global.socketProxy) {
			return new global.ProxySocket(uri);
		} else if (global.indirectionUrl != '' && !global.migrating) {
			global.indirectSocket = true;
			return new global.IndirectSocket(uri);
		} else {
			return new WebSocket(uri);
		}
	};

	// Save the original method to call it when not in "mobile app"  mode
	const originalStringToLocaleString = String.prototype.toLocaleString;

	String.prototype.toLocaleString = function () {
		// `this` is the string being localized
		const string = this.valueOf();

		if (global.ThisIsAMobileApp) {
			if (global.LOCALIZATIONS && Object.prototype.hasOwnProperty.call(global.LOCALIZATIONS, string)) {
				let result = global.LOCALIZATIONS[string];
				if (global.LANG === 'de-CH') {
					result = result.replace(/ß/g, 'ss');
				}
				return result;
			} else {
				return string;
			}
		} else {
			// fallback to original
			return originalStringToLocaleString.call(this);
		}
	};

	// In the mobile app we have a separate localization dictionary for help strings
	String.prototype.toLocaleHelpString = function () {
		// `this` is the string being localized
		const string = this.valueOf();

		if (global.ThisIsAMobileApp) {
			if (global.LOCALIZATIONS_HELP && Object.prototype.hasOwnProperty.call(global.LOCALIZATIONS_HELP, string)) {
				let result = global.LOCALIZATIONS_HELP[string];
				if (global.LANG === 'de-CH') {
					result = result.replace(/ß/g, 'ss');
				}
				return result;
			} else {
				// try also the normal localization dictionary (global.LOCALIZATIONS) if not present in the help one
				return string.toLocaleString();
			}
		} else {
			// fallback to original
			return originalStringToLocaleString.call(this);
		}
	};

	global._ = function (string) {
		return string.toLocaleString();
	};

	// Some global variables are defined in cool.html, among them:
	// global.host: the host URL, with ws(s):// protocol
	// global.serviceRoot: an optional root path on the server, typically blank.

	// Setup global.webserver: the host URL, with http(s):// protocol (used to fetch files).
	if (global.webserver === undefined) {
		var protocol = global.location.protocol === 'file:' ? 'https:' : global.location.protocol;
		global.webserver = global.host.replace(/^(ws|wss):/i, protocol);
		global.webserver = global.webserver.replace(/\/*$/, ''); // Remove trailing slash.
	}

	var docParams, wopiParams;
	var filePath = global.coolParams.get('file_path');
	global.wopiSrc = global.coolParams.get('WOPISrc');
	if (global.wopiSrc != '') {
		global.docURL = decodeURIComponent(global.wopiSrc);
		if (global.accessToken !== '') {
			wopiParams = { 'access_token': global.accessToken, 'access_token_ttl': global.accessTokenTTL, 'no_auth_header': global.noAuthHeader };
		}
		else if (global.accessHeader !== '') {
			wopiParams = { 'access_header': global.accessHeader };
		}

		if (wopiParams) {
			docParams = Object.keys(wopiParams).map(function(key) {
				return encodeURIComponent(key) + '=' + encodeURIComponent(wopiParams[key]);
			}).join('&');
		}
	} else {
		global.docURL = filePath;
	}

	// Form a valid WS URL to the host with the given path.
	global.makeWsUrl = function (path) {
		if (!global.ThisIsAMobileApp) {
			global.app.console.assert(global.host.startsWith('ws'), 'host is not ws: ' + global.host);
		}
		return global.host + global.serviceRoot + path;
	};

	// Form a URI from the docUrl and wopiSrc and encodes.
	// The docUrlParams, suffix, and wopiSrc are optionally hexified.
	global.routeToken = '';
	global.makeDocAndWopiSrcUrl = function (root, docUrlParams, suffix, wopiSrcParam) {
		var wopiSrc = '';
		if (global.wopiSrc != '') {
			wopiSrc = '?WOPISrc=' + encodeURIComponent(global.wopiSrc);
			if (global.routeToken != '')
				wopiSrc += '&RouteToken=' + global.routeToken;
			wopiSrc += '&compat=';
			if (wopiSrcParam && wopiSrcParam.length > 0)
				wopiSrc += '&' + wopiSrcParam;
		}
		else if (wopiSrcParam && wopiSrcParam.length > 0) {
			wopiSrc = '?' + wopiSrcParam;
		}

		suffix = suffix || '/ws';
		var encodedDocUrl = encodeURIComponent(docUrlParams) + suffix + wopiSrc;
		if (global.hexifyUrl)
			encodedDocUrl = global.hexEncode(encodedDocUrl);
		return root + encodedDocUrl + '/ws';
	};

	// Form a valid new Cool WebSocket URL from its components.
	// ws://localhost:9980/cool/ws?WOPISrc=<encoded-document-URI>[&<docParams>]
	global.makeWopiCoolWsUrl = function (root, docParams) {
		var wopiSrc = '';
		if (global.wopiSrc != '') {
			wopiSrc = '?WOPISrc=' + encodeURIComponent(global.wopiSrc);

			if (global.routeToken != '')
				wopiSrc += '&RouteToken=' + global.routeToken;
		}

		return root + '/ws' + wopiSrc + '&' + docParams;
	};

	// Form a valid WS URL to the host with the given path and
	// encode the document URL and params.
	global.makeWsUrlWopiSrc = function (path, docUrlParams, suffix, wopiSrcParam) {
		var websocketURI = global.makeWsUrl(path);
		return global.makeDocAndWopiSrcUrl(websocketURI, docUrlParams, suffix, wopiSrcParam);
	};

	// Form a valid HTTP URL to the host with the given path.
	global.makeHttpUrl = function (path) {
		global.app.console.assert(global.webserver.startsWith('http'), 'webserver is not http: ' + global.webserver);
		return global.webserver + global.serviceRoot + path;
	};

	// Form a valid HTTP URL to the host with the given path and
	// encode the document URL and params.
	global.makeHttpUrlWopiSrc = function (path, docUrlParams, suffix, wopiSrcParam) {
		var httpURI = global.makeHttpUrl(path);
		return global.makeDocAndWopiSrcUrl(httpURI, docUrlParams, suffix, wopiSrcParam);
	};

	global.makeClientVisibleArea = function() {
		// An approximation till we don't yet have CanvasTileLayer, which would properly use
		// map.getPixelBounds() and pixelsToTwips().
		const width = window.innerWidth * 15;
		const height = window.innerHeight * 15;
		return '0;0;' + width + ';' + height;
	};

	// Encode a string to hex.
	global.hexEncode = function (string) {
		var bytes = new TextEncoder().encode(string);
		var hex = '0x';
		for (var i = 0; i < bytes.length; ++i) {
			hex += bytes[i].toString(16);
		}
		return hex;
	};

	// Decode hexified string back to plain text.
	global.hexDecode = function (hex) {
		if (hex.startsWith('0x'))
			hex = hex.substr(2);
		var bytes = new Uint8Array(hex.length / 2);
		for (var i = 0; i < bytes.length; i++) {
			bytes[i] = parseInt(hex.substr(i * 2, 2), 16);
		}
		return new TextDecoder().decode(bytes);
	};


	// Create a WebSocket to the server
	// FIXME The Android and iOS apps use the new approach, other apps should follow
	if (global.ThisIsAMobileApp && !global.ThisIsTheAndroidApp && !global.ThisIsTheiOSApp) {
		// FIXME Remove FakeWebSocket when the mobile apps are updated to use the new approach
		global.socket = new global.FakeWebSocket();
		global.TheFakeWebSocket = global.socket;
	} else {
		if (global.enableExperimentalFeatures) {
			var websocketURI = global.makeWopiCoolWsUrl(global.makeWsUrl('/cool'), docParams);
		} else {
			// The URL may already contain a query (e.g., 'http://server.tld/foo/wopi/files/bar?desktop=baz') - then just append more params
			var docParamsPart = docParams ? (global.docURL.includes('?') ? '&' : '?') + docParams : '';
			var websocketURI = global.makeWsUrlWopiSrc('/cool/', global.docURL + docParamsPart);
		}
		try {
			global.socket = global.createWebSocket(websocketURI);
		} catch (err) {
			global.app.console.log(err);
		}
	}

	var isRandomUser = global.coolParams.get('randomUser');
	if (isRandomUser) {
		// List of languages supported in core
		var randomUserLangs = [
			'ar', 'bg', 'ca', 'cs', 'da', 'de', 'el', 'en-US',
			'en-GB', 'eo', 'es', 'eu', 'fi', 'fr', 'gl', 'he',
			'hr', 'hu', 'id', 'is', 'it', 'ja', 'ko', 'lo',
			'nb', 'nl', 'oc', 'pl', 'pt', 'pt-BR', 'sq', 'ru',
			'sk', 'sl', 'sv', 'tr', 'uk', 'vi', 'zh-CN', 'zh-TW'];
		var randomUserLang = randomUserLangs[Math.floor(Math.random() * randomUserLangs.length)];
		window.app.console.log('Randomize Settings: Set language to: ',randomUserLang);
		global.coolParams.set('lang', randomUserLang);
		global.coolParams.set('debug',true);
	}

	var lang = global.coolParams.get('lang');
	if (lang) {
		// Workaround for broken integrations vs. LOKit language fallback
		if (lang === 'en-us')
			lang = 'en-US';
		if (lang === 'en-gb')
			lang = 'en-GB';
		if (lang === 'pt-br')
			lang = 'pt-BR';
		if (lang === 'zh-cn')
			lang = 'zh-CN';
		if (lang === 'zh-tw')
			lang = 'zh-TW';
		global.langParam = encodeURIComponent(lang);
		}
	else
		global.langParam = 'en-US';
	global.langParamLocale = new Intl.Locale(global.langParam);
	global.queueMsg = [];
	if (global.ThisIsTheEmscriptenApp)
		// Temporary hack
		global.LANG = 'en-US';
	else if (global.ThisIsAMobileApp)
		global.LANG = lang;
	if (global.socket && global.socket.readyState !== 3) {
		global.socket.onopen = function () {
			// Note there are two socket "onopen" handlers, this one and the other in browser/src/core/Socket.js.
			// See the notes there for explanation.
			if (global.socket.readyState === 1) {
				var ProtocolVersionNumber = '0.1';
				var timestamp = encodeURIComponent(global.coolParams.get('timestamp'));
				var msg = 'load url=' + encodeURIComponent(global.docURL);

				var now0 = Date.now();
				var now1 = performance.now();
				var now2 = Date.now();
				global.socket.send('coolclient ' + ProtocolVersionNumber + ' ' + ((now0 + now2) / 2) + ' ' + now1);

				msg += ' accessibilityState=' + global.getAccessibilityState();

				if (global.ThisIsAMobileApp) {
					msg += ' lang=' + global.LANG;
				} else {

					if (timestamp) {
						msg += ' timestamp=' + timestamp;
					}
					if (lang) {
						msg += ' lang=' + lang;
					}
					// renderingOptions?
				}

				if (global.deviceFormFactor) {
					msg += ' deviceFormFactor=' + global.deviceFormFactor;
				}
				var spellOnline = window.prefs.get('spellOnline');
				if (spellOnline) {
					msg += ' spellOnline=' + spellOnline;
				}

				const darkTheme = window.prefs.getBoolean('darkTheme');
				msg += ' darkTheme=' + darkTheme;

				const darkBackground = window.prefs.getBoolean('darkBackgroundForTheme.' + (darkTheme ? 'dark' : 'light'), darkTheme);
				msg += ' darkBackground=' + darkBackground;

				msg += ' timezone=' + Intl.DateTimeFormat().resolvedOptions().timeZone;
				msg += ' clientvisiblearea=' + window.makeClientVisibleArea();

				if (global.coolParams.get('welcome') === 'true') {
					msg += ' batch=true';
					showWelcomeSVG();
				}

				global.socket.send(msg);
			}
		};

		global.socket.onerror = function (event) {
			global.app.console.log(event);
		};

		global.socket.onclose = function (event) {
			global.app.console.log(event);
		};

		global.socket.onmessage = function (event) {
			if (event.data.startsWith('browsersetting:')) {
				try {
					global.prefs._initializeBrowserSetting(event.data);
				} catch (e) {
					global.app.console.error('Failed to initialize browser settings: ', e.message)
				}
			}
			if (typeof global.socket._onMessage === 'function') {
				global.socket._emptyQueue();
				global.socket._onMessage(event);
			} else {
				global.queueMsg.push(event.data);
			}
		};

		global.socket.binaryType = 'arraybuffer';

		if (global.ThisIsAMobileApp && !global.ThisIsTheEmscriptenApp && !window.starterScreen) {
			// This corresponds to the initial GET request when creating a WebSocket
			// connection and tells the app's code that it is OK to start invoking
			// TheFakeWebSocket's onmessage handler. The app code that handles this
			// special message knows the document to be edited anyway, and can send it
			// on as necessary to the Online code.
			global.postMobileMessage('HULLO');
			// A FakeWebSocket is immediately open.
			this.socket.onopen();
		}
	}

	function handleViewportChange(event) {
		var visualViewport = event.target;

		window.scroll(0, 0);
		document.body.style.height = visualViewport.height + 'px';
	}

	if (window.visualViewport !== undefined) {
		window.visualViewport.addEventListener('scroll', handleViewportChange);
		window.visualViewport.addEventListener('resize', handleViewportChange);
	}
}(window));
