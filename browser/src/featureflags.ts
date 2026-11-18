/* -*- js-indent-level: 2 -*- */
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
 * Throughout the code there used to be a lot of hardcoded app detection.
 *
 * It was fairly common for it to be mixed (i.e. checking for both the desktop
 * app or any app) or flat-out wrong (i.e. checking for the desktop app when the
 * feature also worked on mobile) apps.
 *
 * Additionally, when trying to make platforms behave more like each other it
 * easy to fail to edit the cases properly.
 *
 * At the moment, these feature flags do not support toggling. Eventually, it might
 * be worth making preferences in the debug options that let you toggle these
 * individually (which is expected to break your app most of the time, but you're a
 * developer)
 */

/**
 * A bitfield enum for platform detection
 *
 * You probably want to detect features instead (see window.features and window.detect)
 *
 * You must not use the numbers from this enum directly, as they may change if new platforms are added
 */
enum Platform {
	APP_ANDROID = 1 << 0,
	APP_CHROMEOS = 1 << 1,
	APP_IOS = 1 << 2,
	APP_LINUX = 1 << 3,
	APP_MAC = 1 << 4,
	APP_WINDOWS = 1 << 5,
	EMSCRIPTEN_APPLE = 1 << 6,
	EMSCRIPTEN_LINUX = 1 << 7,
	EMSCRIPTEN_WINDOWS = 1 << 8,
	ONLINE_APPLE = 1 << 9,
	ONLINE_LINUX = 1 << 10, // Includes platforms who's browser can't be detected - notably includes Android
	ONLINE_WINDOWS = 1 << 11,
	ALL = ~0,
}

const features = {
	/**
	 * Environments which are a downloadable app on your system.
	 *
	 * These apps don't use a browser, which might let them have better native
	 * integration with your system
	 */
	app:
		Platform.APP_ANDROID |
		Platform.APP_CHROMEOS |
		Platform.APP_IOS |
		Platform.APP_LINUX |
		Platform.APP_MAC |
		Platform.APP_WINDOWS,
	/**
	 * Environments which are the emscripten WASM app.
	 *
	 * These apps are opened and run in your browser, but can run offline - e.g.
	 * in the case where you have patchy network on a train they will not need to
	 * reconnect
	 */
	emscripten:
		Platform.EMSCRIPTEN_APPLE |
		Platform.EMSCRIPTEN_LINUX |
		Platform.EMSCRIPTEN_WINDOWS,
	/**
	 * Environments which run online
	 *
	 * This is the traditional Collabora Online experience, with a WOPI host and
	 * a COOLWSD server
	 */
	online:
		Platform.ONLINE_APPLE | Platform.ONLINE_LINUX | Platform.ONLINE_WINDOWS,

	/**
	 * Environments on Apple devices
	 *
	 * Apple users tend to expect slightly different keybinds/etc. than other systems
	 */
	apple:
		Platform.APP_IOS |
		Platform.APP_MAC |
		Platform.EMSCRIPTEN_APPLE |
		Platform.ONLINE_APPLE,

	/**
	 * Environments which run in a browser
	 *
	 * These apps can't use native integrations
	 */
	get browser() {
		return this.emscripten | this.online;
	},
	/**
	 * Environments which can run offline
	 *
	 * These environments won't care if your network connection dies
	 */
	get offline() {
		return this.app | this.emscripten;
	},

	/**
	 * Environments which we can open new windows in while still leaving the others open
	 *
	 * A window does not have to be able to be visible at the same time as other
	 * windows, per-se. It's just as good to open a new tab provided that the
	 * original tab is not unloaded
	 * // TODO: test/support opening more windows on ChromeOS
	 */
	get multiWindow() {
		return (
			this.browser |
			Platform.APP_LINUX |
			Platform.APP_MAC |
			Platform.APP_WINDOWS
		);
	},

	/**
	 * Environments which have the file backstage
	 *
	 * The file backstage is an alternative view for the "file" tab which supports
	 * opening documents, viewing properties, etc. in a fullscreen page
	 */
	backstage: Platform.APP_LINUX | Platform.APP_MAC | Platform.APP_WINDOWS,

	/**
	 * Environments which show file shortcuts as acting locally on the machine (e.g. "Print" instead of "Print (Download as PDF)")
	 */
	fileLocalShortcuts:
		Platform.APP_LINUX | Platform.APP_MAC | Platform.APP_WINDOWS,

	/**
	 * Environments that support tabs for opening multiple files
	 */
	tabs: Platform.APP_LINUX | Platform.APP_MAC | Platform.APP_WINDOWS,

	/**
	 * Environments that support moving through tabs using keybinds (like in a browser)
	 */
	get tabsKeybinds() {
		return this.tabs & Platform.APP_LINUX;
	},

	/**
	 * Environments that show the welcome slideshow
	 */
	welcomeSlideshow:
		Platform.APP_LINUX | Platform.APP_MAC | Platform.APP_WINDOWS,
} as const;

interface Window {
	platform: Platform;
	features: Record<keyof typeof features, Platform>;
	detect: (features: Platform, detectionPlatform?: Platform | null) => boolean;
}

window.platform =
	window.mode.isChromebook() ? Platform.APP_CHROMEOS
	: window.ThisIsTheAndroidApp ?
		Platform.APP_ANDROID // Cannot come before window.mode.isChromebook() as all Chromebook app users are necessarily also Android app users
	: window.ThisIsTheiOSApp ? Platform.APP_IOS
	: window.ThisIsTheWindowsApp ? Platform.APP_WINDOWS
	: window.ThisIsTheMacOSApp ? Platform.APP_MAC
	: window.ThisIsTheQtApp ? Platform.APP_LINUX
	: window.ThisIsTheEmscriptenApp && window.L.Browser.mac ?
		Platform.EMSCRIPTEN_APPLE
	: window.ThisIsTheEmscriptenApp && window.L.Browser.win ?
		Platform.EMSCRIPTEN_WINDOWS
	: window.ThisIsTheEmscriptenApp ? Platform.EMSCRIPTEN_LINUX
	: window.L.Browser.mac ? Platform.ONLINE_APPLE
	: window.L.Browser.win ? Platform.ONLINE_WINDOWS
	: Platform.ONLINE_LINUX;

window.features = features;

/**
 * Detect whether a feature is supported on your platform
 *
 * For example:
 * - window.detect(window.features.apple) => true on iOS or MacOS, false otherwise
 * - window.detect(window.features.colorSchemeFollowSystem & ~window.features.nativeColorScheme) => true iff the color scheme is handled in JavaScript and follows whatever the system says
 * - window.detect(window.features.presentFollow | window.features.presentInWindow) => true if either of the presentation features are supported... useful for, e.g., showing menu sections
 *
 * You may also provide a custom set of platforms to detect against
 * For example:
 * - window.detect(window.features.apple, Platform.APP_IOS) => true, because iOS is an Apple platform
 * - window.detect(window.features.backstage, window.features.browser) => false today, true if backstage ever becomes supported on any browser platform
 */
window.detect = (
	features: Platform,
	detectionPlatform: Platform | null = null,
): boolean => {
	if (detectionPlatform === null) {
		detectionPlatform = window.platform;
	}

	return !!(detectionPlatform & features);
};
