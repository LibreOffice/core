/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// In the browser, dark mode should follow the OS light/dark setting until the
// user (or integrator) chooses a theme explicitly - so a document opens
// matching the desktop and flips live when the desktop theme changes. Before
// the fix the theme defaulted to light and there was no OS query at all
// (getBoolean('darkTheme') fell through to false), so the user had to toggle
// View -> Dark mode by hand and nothing reacted to the desktop.
//
// The OS is read through prefs.prefersDarkOS() (a prefers-color-scheme media
// query); seedDarkModeDefault() caches that value when nothing is stored, and
// UIManager.followSystemDarkMode() re-applies it when the media query changes.
// We stub prefersDarkOS() and drive those two entry points directly (the same
// ones the load message and the media-query listener use). The real query can
// be emulated on Chromium via the DevTools Protocol (Emulation.setEmulatedMedia
// through Cypress.automation('remote:debugger:protocol'), which does reach this
// iframe); switching to that would also cover the media-query wiring itself,
// but only on Chromium-family browsers - worth revisiting.
describe(['tagdesktop'], 'Dark mode follows the OS setting', function() {

	function assertThemeIs(theme) {
		cy.cframe().find('html').should('have.attr', 'data-theme', theme);
	}

	// Remove every stored source so the theme is genuinely "unset" (the fresh
	// user / debug.html case), independent of any setting seeded by earlier runs.
	function clearStoredTheme(win) {
		delete win.prefs._userBrowserSetting.darkTheme;
		if (win.prefs.canPersist)
			win.localStorage.removeItem('darkTheme');
		delete win.prefs._localStorageCache.darkTheme;
		delete (win.uiDefaults || {}).darkTheme;
	}

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
	});

	it('Follows the OS mode when no theme was chosen', function() {
		cy.getFrameWindow().then(function(win) {
			clearStoredTheme(win);
			expect(win.prefs.hasExplicitDarkModePref()).to.be.false;

			// 1. OS is dark -> the default resolves to dark, and applying it
			//    (as the OS-change listener does) switches the UI to dark.
			cy.stub(win.prefs, 'prefersDarkOS').returns(true);
			delete win.prefs._localStorageCache.darkTheme;
			expect(win.prefs.seedDarkModeDefault()).to.be.true;
			win.app.map.uiManager.followSystemDarkMode();
		});
		assertThemeIs('dark');

		cy.getFrameWindow().then(function(win) {
			// 2. OS flips back to light -> the UI follows without a manual toggle.
			win.prefs.prefersDarkOS.returns(false);
			win.app.map.uiManager.followSystemDarkMode();
		});
		assertThemeIs('light');
	});

	it('Stops following the OS once the user chooses a theme', function() {
		cy.getFrameWindow().then(function(win) {
			clearStoredTheme(win);
			// The user explicitly picks light and it is persisted.
			win.app.map.uiManager.applyDarkMode(false, /*persist*/ true);
			expect(win.prefs.hasExplicitDarkModePref()).to.be.true;
		});
		assertThemeIs('light');

		cy.getFrameWindow().then(function(win) {
			// The OS switches to dark, but the explicit light choice must win:
			// followSystemDarkMode is a no-op while a theme is stored (before the
			// fix there was no such guard because there was no OS following at all).
			cy.stub(win.prefs, 'prefersDarkOS').returns(true);
			win.app.map.uiManager.followSystemDarkMode();
		});
		assertThemeIs('light');
	});
});
