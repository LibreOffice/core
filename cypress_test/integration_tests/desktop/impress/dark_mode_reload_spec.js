/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The browser saves a changed setting locally at once and sends it to the server
// in a batch a few seconds later. On the next load the server's copy wins over the
// local one, so a reload made while the batch is still waiting has to send it
// before the page goes away.
//
// The test uses an explicit &userid so the test WOPI server keeps this user's
// settings across the reload (see userPresetDir() in test/TestWopiFileServer.hpp).
describe(['tagdesktop'], 'Dark mode chosen just before a reload is kept', function() {
	var USER = 'dark-mode-reload';
	var userQuery = 'userid=' + USER;
	var filePath;

	function assertThemeIs(theme) {
		cy.cframe().find('html').should('have.attr', 'data-theme', theme);
	}

	beforeEach(function() {
		cy.task('writeUserSetting', { userId: USER, settings: { darkTheme: 'false' } });
		filePath = helper.setupAndLoadDocument('impress/slideshow.odp', false, false, undefined,
			userQuery);
		desktopHelper.switchUIToNotebookbar();
	});

	it('The page opens in dark mode after a reload right after turning it on', function() {
		assertThemeIs('light');

		// Tests shorten the batch delay to a fraction of a second. Hold the batch
		// for a minute instead, so the reload below lands while the change is still
		// waiting to be sent, as it does for a user who reloads at once.
		cy.getFrameWindow().then(function(win) {
			win.prefs._settingUpdateDebounceMs = function() { return 60000; };
		});

		desktopHelper.selectNotebookbarTab('View');
		desktopHelper.getNbItem('toggledarktheme', 'View').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('toggledarktheme', 'View').click();
		assertThemeIs('dark');

		// The change is waiting in the batch, not yet on the server.
		cy.getFrameWindow().should(function(win) {
			expect(win.prefs._settingUpdateJSON.darkTheme).to.equal('true');
		});

		helper.reloadDocument(filePath, userQuery);

		// The server's copy is what the page reads after a reload, so check it as
		// well as the theme on the page.
		cy.getFrameWindow().should(function(win) {
			expect(win.prefs._userBrowserSetting.darkTheme).to.equal('true');
		});
		assertThemeIs('dark');
	});
});
