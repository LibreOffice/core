/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

// The speaker notes pane below the slide renders the notes outliner as an
// editengine custom widget, so what is typed there ends up on the slide's notes
// page rather than on the slide itself.
describe(['tagdesktop'], 'Impress speaker notes pane', function () {
	let newFileName;

	beforeEach(function () {
		newFileName = helper.setupAndLoadDocument('impress/empty-placeholder.fodp');
	});

	function openNotesPane() {
		cy.getFrameWindow().then(function (win) {
			win.app.dispatcher.dispatch('notespanel');
		});

		cy.cGet('#notespanel-dock-wrapper').should('be.visible');
		cy.cGet('#notespanel-container .ui-editengine').should('exist');
	}

	it('opens on request and shows an editable notes area', function () {
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');

		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').should(
			'have.attr',
			'contenteditable',
			'true'
		);
	});

	it('typed text reaches the notes of the current slide', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('Remember the demo');

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		helper.reloadDocument(newFileName);
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);
	});

	// The three notes views are mutually exclusive, so switching the pane on
	// from the status bar has to leave the handout page behind and bring back
	// a pane that holds the notes of the slide and still takes typing.
	it('the status bar button swaps the handout page for a working pane', function () {
		cy.viewport(1920, 1080);

		openNotesPane();
		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('Remember the demo');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		cy.getFrameWindow().then(function (win) {
			win.app.dispatcher.dispatch('notespanelhandout');
		});

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', true);
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');

		cy.cGet('#toolbar-down #notespanel').click();

		cy.getFrameWindow().its('app.impress.notesMode').should('eq', false);
		cy.cGet('#notespanel-dock-wrapper').should('be.visible');

		// The notes of the slide are there, and they stay: the engine sends a
		// late update for every page it passes through on the way out of the
		// handout page, and an empty notes placeholder reads "Click to add
		// Notes".
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'Remember the demo'
		);

		// Typing still reaches the notes editor.
		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type(' again');
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'again'
		);
	});

	it('Enter starts a new paragraph', function () {
		openNotesPane();

		cy.cGet('#notespanel-container .ui-editengine').click();
		cy.cGet('#notespanel-container .ui-editengine').type('first{enter}second');

		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'have.length',
			2
		);
	});
});
