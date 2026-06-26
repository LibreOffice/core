/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// When the integrator forces a UI theme (ui_defaults UITheme=dark with
// SavedUIState=false, which debug.html sets from &UITheme), that default has to
// win over a contradicting theme the user saved earlier - on the client and in
// the engine. The client always shows the integrator theme (SavedUIState=false
// makes it win in prefs.get), but the engine reads only the saved browser
// setting, so without reconciliation the two diverge (the classic unreadable
// "transparent" text: dark canvas under engine-light text, or vice versa).
//
// reconcileIntegratorThemeOverride() (Control.UIManager) closes that gap by
// writing the integrator value back into the saved setting, so the engine
// agrees and the document opens correctly on the next load too.
//
// The test uses an explicit &userid (other than the default "test" user) so the
// test WOPI server persists this user's settings (see userPresetDir() in
// test/TestWopiFileServer.hpp); the seeded file gives them a saved light theme
// that contradicts the integrator.
describe(['tagdesktop'], 'Integrator theme overrides a contradicting saved setting', function() {
	var USER = 4;
	var integratorOverride = 'UITheme=dark&userid=' + USER;
	var filePath;

	function assertThemeIs(theme) {
		cy.cframe().find('html').should('have.attr', 'data-theme', theme);
	}

	beforeEach(function() {
		// Saved user theme is light, while the integrator will force dark.
		cy.task('writeUserSetting', { userId: USER, settings: { darkTheme: 'false' } });
		filePath = helper.setupAndLoadDocument('calc/cell_cursor.ods', false, false, undefined,
			integratorOverride);
	});

	it('A user choosing light is forced back to the integrator dark on reload', function() {
		// The integrator dark wins over the saved light on first open.
		assertThemeIs('dark');

		// The user switches to light; let the debounced server update flush so the
		// saved setting becomes light.
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.applyDarkMode(false, /*persist*/ true);
		});
		assertThemeIs('light');
		cy.wait(1200);

		helper.reloadDocument(filePath, integratorOverride);

		// The page opens dark again.
		assertThemeIs('dark');

		// data-theme alone cannot guard the fix: the client shows dark even
		// without reconciliation (SavedUIState=false makes the integrator default
		// win in prefs.get). What reconciliation uniquely does is write the
		// integrator value back into the saved setting so the engine agrees.
		// Assert that saved value was flipped from the user's light back to dark;
		// reverting the fix leaves it at light and fails here.
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				expect(String(win.prefs._userBrowserSetting.darkTheme)).to.eq('true');
			});
		});
	});
});
