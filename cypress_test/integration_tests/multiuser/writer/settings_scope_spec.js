/* global describe it cy beforeEach require Cypress */

const helper = require('../../common/helper');

// A document runs with the settings of whoever opened it: they are installed
// once, for the first session, and one kit holds one configuration. Anyone
// joining afterwards can still open the dialog and save, and their dialog
// shows what they saved, because it reads that back from the host rather than
// from the kit. So each client is told which of the two it is, and the dialog
// uses it to say when a change will only take effect in the next document.
describe(['tagmultiuser'], 'A joining user is told the document uses other settings', function() {
	// iframe1 opens the document, iframe2 joins it.
	const FIRST = 1;
	const SECOND = 2;

	function presetsApplied() {
		return cy.getFrameWindow().its('app').its('userPresetsApplied');
	}

	function settingsIframeBody() {
		return cy.cframe()
			.find('.iframe-settings-modal')
			.its('0.contentDocument').should('exist')
			.its('body').should('not.be.empty')
			.then(cy.wrap);
	}

	// The note sits in the document settings, so wait for those to be built
	// rather than for the dialog frame, which is there before its contents.
	function openSettings() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.settings.showSettingsDialog('xcu-section');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody().find('#xcu-editor', { timeout: 30000 }).should('exist');
	}

	beforeEach(function() {
		// Writing the file gives each user a store of their own, which is what
		// makes them two users rather than the shared debug one.
		cy.task('writeUserSetting', { userId: FIRST, settings: { darkTheme: 'false' } });
		cy.task('writeUserSetting', { userId: SECOND, settings: { darkTheme: 'false' } });

		// The settings store is the host's to name, and the multiuser page
		// passes on what it is given, so say where it is: without this the
		// dialog opens on empty sections with nothing to fetch.
		const settingsBase = encodeURIComponent(
			Cypress.config('baseUrl') + '/wopi/settings');

		helper.setupAndLoadDocument('writer/annotation.odt', true, false, undefined,
			'userid1=' + FIRST + '&userid2=' + SECOND
				+ '&wopi_setting_base_url=' + settingsBase);
	});

	it('The one who opened the document is running with their own settings', function() {
		cy.cSetActiveFrame('#iframe1');
		presetsApplied().should('eq', true);
	});

	it('The one who joined is told the document is not using theirs', function() {
		cy.cSetActiveFrame('#iframe2');
		presetsApplied().should('eq', false);
	});

	// With two people here a change is felt by neither, so both are told it
	// waits; only the one who joined is also told the settings in force are
	// not theirs. The note carries its own hidden flag, which is what the
	// dialog sets: visibility is no use to assert on, because a section the
	// dialog is not currently showing counts as invisible whatever it says.
	it('Both are told the change waits, and the joining one why', function() {
		cy.cSetActiveFrame('#iframe1');
		openSettings();
		settingsIframeBody()
			.find('.settings-scope-note')
			.should('have.length.at.least', 1)
			.first()
			.should('have.prop', 'hidden', false)
			.and('contain.text', 'open in more than one window')
			.and('not.contain.text', 'the document settings it was opened with');

		cy.cSetActiveFrame('#iframe2');
		openSettings();
		settingsIframeBody()
			.find('.settings-scope-note')
			.should('have.length.at.least', 1)
			.first()
			.should('have.prop', 'hidden', false)
			.and('contain.text', 'the document settings it was opened with');
	});

	// The answer depends on who else is on the document, and people come and
	// go while the dialog sits open. The one who opened it is left alone
	// here, so what was true when the dialog opened stops being true and the
	// note has to go.
	it('The note goes when the other one leaves with the dialog open', function() {
		let second;

		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) { second = win; });

		cy.cSetActiveFrame('#iframe1');
		openSettings();
		settingsIframeBody()
			.find('.settings-scope-note')
			.first()
			.should('have.prop', 'hidden', false);

		cy.then(function() { second.app.socket.close(); });

		// No processToIdle: the other view is gone, so nobody answers it.
		// The assertion retries until documentsettingslive arrives. Every
		// note the dialog holds is asserted, because it puts one in the
		// document settings and one in the sentence checker section.
		settingsIframeBody()
			.find('.settings-scope-note', { timeout: 30000 })
			.should('have.length.at.least', 1)
			.each(function(note) {
				cy.wrap(note).should('have.prop', 'hidden', true);
			});
	});
});
