/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// The engine paints the document in the theme the load message told it, and the
// browser has to work that value out before the user's stored settings have
// arrived - from the OS light/dark setting when nothing is stored yet. Those
// settings win once they are here, so the answer the browser settles on can be a
// different one from the answer the engine holds. In the browser nothing used to
// close that gap: initDarkModeFromSettings pushed the theme to the engine on the
// desktop app only. The document then kept being painted for a theme nobody was
// looking at - white automatic text on a light background, or a dark page under
// a light window - until someone toggled the theme by hand.
//
// The browser now remembers what the load message carried
// (rememberThemeSentWithLoad, called from Socket.ts) and hands the engine its
// answer whenever the two differ (Control.UIManager).
//
// The test puts the engine on the other theme through that same record and lets
// the browser resolve the settings again, which is what a stored setting
// arriving after the load message does. Note that the answer the browser shows
// stays what it was - only the engine moves - so the theme of the page cannot
// stand in for what the engine holds here. Neither can
// prefs.getBoolean('darkTheme'): with nothing stored anywhere it answers with
// its own default rather than with the theme in force, which is the very
// confusion this fix is about.
describe(['tagdesktop'], 'The engine is told the theme the browser settled on', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
	});

	it('A theme the engine does not hold is handed to it, once', function() {
		cy.getFrameWindow().then(function(win) {
			var uiManager = win.app.map.uiManager;
			// The answer the browser settles on, resolved the way the code does.
			var settledTheme = win.prefs.seedDarkModeDefault();
			var sendSpy = cy.spy(win.app.socket, 'sendMessage');

			uiManager.rememberThemeSentWithLoad(!settledTheme, !settledTheme);
			uiManager.initDarkModeFromSettings();

			// Resolving a second time must not repeat it: the browser keeps what
			// it handed the engine.
			uiManager.initDarkModeFromSettings();

			// Handing the theme over is synchronous, so the calls are all in by now
			// and there is nothing to wait for.
			var themeCommands = sendSpy.getCalls()
				.map(function(call) { return String(call.args[0]); })
				.filter(function(message) { return message.indexOf('.uno:ChangeTheme') !== -1; });

			expect(themeCommands).to.have.length(1);
			expect(themeCommands[0]).to.contain(
				'"value":"' + (settledTheme ? 'Dark' : 'Light') + '"');
		});
	});
});
