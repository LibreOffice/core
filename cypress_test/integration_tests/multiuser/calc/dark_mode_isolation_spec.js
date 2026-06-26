/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Two users open the same document. The theme each one chooses is kept in their
// own per-user settings on the server (see userPresetDir() in
// test/TestWopiFileServer.hpp) and must not leak into the other user on reload.
//
// The default cypress user does not persist browser settings, so each user opts
// in with an explicit &userid that the test WOPI server routes to its own store.
// We seed that store first: writing the file both sets the starting theme and
// makes the client receive a browsersetting: message, which is what turns on
// persistence of the later change.
describe(['tagmultiuser'], 'Dark mode is isolated per user', function() {
	var DARK_USER = 1;   // iframe1
	var LIGHT_USER = 2;  // iframe2

	function assertThemeIs(theme) {
		// Re-query each retry so this converges on the freshly reloaded frame.
		cy.cframe().find('html').should('have.attr', 'data-theme', theme);
	}

	beforeEach(function() {
		// Both users start in light mode.
		cy.task('writeUserSetting', { userId: DARK_USER, settings: { darkTheme: 'false' } });
		cy.task('writeUserSetting', { userId: LIGHT_USER, settings: { darkTheme: 'false' } });

		helper.setupAndLoadDocument('calc/invalidations.ods', true, false, undefined,
			'userid1=' + DARK_USER + '&userid2=' + LIGHT_USER);
		desktopHelper.switchUIToNotebookbar();
	});

	it('A theme change by one user does not follow the other on reload', function() {
		// Both start light.
		cy.cSetActiveFrame('#iframe1');
		assertThemeIs('light');
		cy.cSetActiveFrame('#iframe2');
		assertThemeIs('light');

		// User 1 switches to dark; let the (debounced) server update flush.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.applyDarkMode(true, /*persist*/ true);
		});
		assertThemeIs('dark');
		cy.wait(1200);

		// User 2 reloads: their own (light) setting comes back, not user 1's dark.
		cy.cSetActiveFrame('#iframe2');
		cy.get('#form2').submit();
		cy.wait(1000); // let the frame unload
		helper.documentChecks(true);
		assertThemeIs('light');

		// User 1 reloads: dark is restored from their own setting. (Reloading
		// iframe1 also reloads iframe2 through the harness; it stays light.)
		cy.cSetActiveFrame('#iframe1');
		cy.get('#form1').submit();
		cy.wait(1000);
		helper.documentChecks(true);
		assertThemeIs('dark');

		cy.cSetActiveFrame('#iframe2');
		helper.documentChecks(true);
		assertThemeIs('light');
	});
});
