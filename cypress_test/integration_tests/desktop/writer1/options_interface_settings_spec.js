/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

function settingsIframeBody() {
	return cy.cframe()
		.find('.iframe-settings-modal')
		.its('0.contentDocument').should('exist')
		.its('body').should('not.be.empty')
		.then(cy.wrap);
}

describe(['tagdesktop'], 'Options view toggles persist across reload', function () {
	var newFilePath;
	var win;

	beforeEach(function () {
		newFilePath = helper.setupAndLoadDocument('writer/annotation.odt');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.cGet('div.leaflet-layer').should('exist');
	});

	it('remembers Show Formatting Marks after reload', function () {
		// Core reports the marks as off once the load settles. Wait for that
		// report, so the toggle below is the first change after it and the
		// client keeps it as the state to persist.
		cy.getFrameWindow().then(function (w) {
			cy.wrap(w.app.map['stateChangeHandler'])
				.invoke('getItemValue', '.uno:ControlCodes')
				.should('eq', 'false');
		});

		// Turn the marks on through the same command the menu uses.
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:ControlCodes');
		});
		// Core confirms marks are on, which is what gets persisted.
		cy.getFrameWindow().then(function (w) {
			cy.wrap(w.app.map['stateChangeHandler'])
				.invoke('getItemValue', '.uno:ControlCodes')
				.should('eq', 'true');
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowFormattingMarks', false)
			.should('eq', true);

		helper.reloadDocument(newFilePath);
		cy.cGet('div.leaflet-layer').should('exist');

		// The marks are actually shown again, not just the stored flag. This
		// is the live state reported by core, which the early-toggle bug left
		// off even though the preference said on.
		cy.getFrameWindow().then(function (w) {
			cy.wrap(w.app.map['stateChangeHandler'])
				.invoke('getItemValue', '.uno:ControlCodes')
				.should('eq', 'true');
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowFormattingMarks', false)
			.should('eq', true);
	});

	it('remembers hidden comments after reload', function () {
		cy.then(function () {
			win.app.map.showComments(false);
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);

		helper.reloadDocument(newFilePath);
		cy.cGet('div.leaflet-layer').should('exist');

		// Comments stay hidden, and the in-document state reflects it.
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);
		cy.getFrameWindow().then(function (w) {
			var state = w.app.map['stateChangeHandler'].getItemValue('showannotations');
			expect(state).to.eq('false');
		});
	});

	it('hides the comments as soon as the Options dialog is saved', function () {
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.insertComment();
		cy.cGet('#comment-container-1').should('exist');

		// The comments start out shown, so switching the option off below is a
		// real change.
		cy.getFrameWindow().then(function (w) {
			var state = w.app.map['stateChangeHandler'].getItemValue('showannotations');
			expect(state).to.eq('true');
		});

		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');

		// The Interface Settings open on the Calc tab; wait for that, otherwise
		// the tab it opens with replaces the Writer options picked below.
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');
		settingsIframeBody().find('#bs-tab-text').click();

		// The comment toggle sits on the Writer tab, on because the comments are
		// shown.
		settingsIframeBody()
			.find('#text-ShowAnnotations-input')
			.should('be.checked')
			.click({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// The comment goes away with the save, no reload needed, and the choice is
		// recorded the same way the Show Comments button records it.
		cy.cGet('#comment-container-1').should('be.not.visible');
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);
	});

	it('shows the ruler as soon as the Options dialog is saved', function () {
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.cool-ruler').should('not.be.visible');

		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');
		settingsIframeBody().find('#bs-tab-text').click();

		settingsIframeBody()
			.find('#text-ShowRuler-input')
			.should('not.be.checked')
			.click({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// The ruler appears with the save, no reload needed, and the View tab
		// toggle shows it on.
		cy.cGet('.cool-ruler').should('be.visible');
		cy.cGet('#showruler-input').should('be.checked');
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowRuler', false)
			.should('eq', true);
	});

	it('switches to the dark theme as soon as the Options dialog is saved', function () {
		cy.cframe().find('html').should('have.attr', 'data-theme', 'light');

		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');

		// The theme is shared by every document type, so its toggle sits above
		// the document type tabs.
		settingsIframeBody()
			.find('#common-darkTheme-input')
			.should('not.be.checked')
			.click({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// The theme changes with the save, no reload needed, and the choice is
		// recorded the same way the dark mode button records it.
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'darkTheme', false)
			.should('eq', true);
	});

	it('tells the server every setting the Options dialog saved', function () {
		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');

		// Smooth scrolling is shared by every document type and has no toggle in
		// the document itself, so the dialog is the one place it changes. It
		// starts out on, so switching it off below is a real change.
		settingsIframeBody()
			.find('#common-smoothScroll-input')
			.should('be.checked')
			.click({ force: true });

		// A zoom of the user's own: neither the server's choice nor the smart
		// zoom, and 200% is the 14th entry of the dropdown.
		settingsIframeBody()
			.find('#common-followServerZoom-input')
			.uncheck({ force: true });
		settingsIframeBody()
			.find('#common-smartZoom-input')
			.uncheck({ force: true });
		settingsIframeBody()
			.find('#common-defaultZoom-select')
			.should('be.enabled')
			.select('13');

		cy.then(function () {
			cy.spy(win.socket, 'send').as('socketSend');
		});
		cy.cGet('#iframe-settings-save').click();

		// The server keeps its own copy of the settings file and writes it out
		// again on the next preference change made in the document, so the save
		// has to reach that copy too, shared settings included. Every value
		// arrives as text, the zoom index included, which is how the dialog
		// wrote the file, so the copy the server writes out later has the same
		// shape as the file the dialog saved.
		cy.get('@socketSend').should(function (send) {
			var pattern = /^browsersetting action=update (upload=false )?json=(.*)$/;
			var updates = send.args
				.map(function (args) { return args[0]; })
				.filter(function (message) {
					return typeof message === 'string' && pattern.test(message);
				});
			expect(updates).to.have.length.greaterThan(0);
			var json = JSON.parse(updates[updates.length - 1].match(pattern)[2]);
			expect(json.smoothScroll).to.eq('false');
			expect(json.smartZoom).to.eq('false');
			expect(json.defaultZoom).to.eq('13');
		});
	});

	it('reads the zoom the server set once the dialog follows the server', function () {
		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');

		// The shared cypress user's settings uploads are not persisted (see
		// skipPersist in test/TestWopiFileServer.hpp), so the dialog opens from
		// the defaults, not from the zoom the earlier test chose. Checking the
		// box, rather than clicking it, ends up checked whatever its default is.
		settingsIframeBody()
			.find('#common-followServerZoom-input')
			.check({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// Following the server is written out as an empty zoom. An empty value is
		// no choice, so a read of the zoom goes back to the one the server set,
		// in the document already open as well.
		cy.getFrameWindow()
			.its('prefs._userBrowserSetting.defaultZoom')
			.should('eq', '');
		cy.then(function () {
			expect(win.prefs.get('defaultZoom')).to.eq(win.uiDefaults.defaultZoom);
		});
	});
});

describe(['taga11yenabled'], 'Options dialog and screen reading', function () {
	var win;

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/annotation.odt');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.cGet('div.leaflet-layer').should('exist');
	});

	it('turns screen reading on as soon as the Options dialog is saved', function () {
		// Start from screen reading off, the state the Help menu toggle leaves.
		cy.then(function () {
			win.app.map.setAccessibilityState(false);
			expect(win.app.map._textInput.hasAccessibilitySupport()).to.eq(false);
		});

		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');

		// The dialog stores the In-document Screen Reader choice as the screen
		// reading state as well.
		settingsIframeBody()
			.find('#common-lockAccessibilityOn-input')
			.should('not.be.checked')
			.click({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// Screen reading comes on with the save, no reload needed.
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'accessibilityState', false)
			.should('eq', true);
		cy.then(function () {
			expect(win.app.map._textInput.hasAccessibilitySupport()).to.eq(true);
		});
	});
});
