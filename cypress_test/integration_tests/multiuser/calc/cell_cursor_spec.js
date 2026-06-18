/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Check cell cursor and view behavior', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor_jump.ods',true);
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
	});

	it('Jump on modification above in the sheet', function() {
		// second view follow the first one
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then((win2) => {
			cy.cGet('#userListHeader').click();
			cy.cGet('.user-list-item').eq(1).click();
			cy.cGet('.jsdialog-overlay').should('not.exist');

			// first view goes somewhere in the middle of a sheet: A400
			cy.cSetActiveFrame('#iframe1');

			cy.cGet(helper.addressInputSelector).type('{selectAll}A400{enter}');
			desktopHelper.assertScrollbarPosition('vertical', 400, 670);
			cy.cGet('#sc_input_window .ui-custom-textarea-text-layer').click();
			cy.cGet('#sc_input_window .ui-custom-textarea-text-layer').type('some text{enter}');

			cy.getFrameWindow().then((win1) => {
				helper.processToIdle(win1);
			});

			// turn off following in the second view
			cy.cSetActiveFrame('#iframe2');
			cy.cGet('#followingChip').click();

			// verify that second view is scrolled to the: A400
			desktopHelper.assertScrollbarPosition('vertical', 400, 670);

			// second view should still have cursor at the end: A588
			calcHelper.assertAddressAfterIdle(win2, 'A588');

			// now insert row in the first view
			cy.cSetActiveFrame('#iframe1');
			cy.getFrameWindow().then((win1) => {
				desktopHelper.getNbIcon('InsertRowsBefore').first().click();

				// wait for row insertion to complete in first view
				helper.processToIdle(win1);

				// verify that second view is still at the: A400
				cy.cSetActiveFrame('#iframe2');
				desktopHelper.assertScrollbarPosition('vertical', 400, 670);

				// second view should still have cursor at the previous cell: A588+1
				calcHelper.assertAddressAfterIdle(win2, 'A589');
			});
		});
	});

	it('Jump to the other sheet', function() {
		// second view follow the first one
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');

		// first view goes somewhere in the middle of a sheet
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win1) => {
			cy.cGet(helper.addressInputSelector).type('{selectAll}A400{enter}');
			desktopHelper.assertScrollbarPosition('vertical', 400, 500);
			calcHelper.clickOnFirstCell(true, false, 'A400');
			helper.typeIntoDocument('abc{enter}');

			// second view should jump there
			cy.cSetActiveFrame('#iframe2');
			desktopHelper.assertScrollbarPosition('vertical', 400, 500);

			// first view inserts sheet before current one
			cy.cSetActiveFrame('#iframe1');
			calcHelper.selectOptionFromContextMenu('Insert sheet before this');
			cy.cGet('#map').focus();

			// we should see A1
			calcHelper.assertAddressAfterIdle(win1, 'A1');
			desktopHelper.assertScrollbarPosition('vertical', 0, 30);

			// verify that second view followed the first one
			cy.cSetActiveFrame('#iframe2');
			desktopHelper.assertScrollbarPosition('vertical', 0, 30);

			// first goes to second sheet and we should see A388
			cy.cSetActiveFrame('#iframe1');
			cy.cGet('#spreadsheet-tab1').click();
			desktopHelper.assertScrollbarPosition('vertical', 400, 500);
		});
	});

	it('Other view text cursor stays visible on a later edit', function() {
		// The first view observes, the second view edits. The test document
		// restores a scrolled-down position on load, so first bring the
		// observer to A1 - otherwise the second view's A1 cursor would fall
		// outside the observer's viewport and be legitimately hidden.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win1) => {
			helper.processToIdle(win1);
		});
		// The address input is briefly disabled during initial load, so wait
		// for it to become editable before navigating.
		cy.cGet(helper.addressInputSelector).should('not.be.disabled');
		cy.cGet(helper.addressInputSelector).type('{selectAll}A1{enter}');
		cy.getFrameWindow().then((win1) => {
			helper.processToIdle(win1);
		});

		// First edit by the second view: enter cell edit mode, type and commit.
		cy.cSetActiveFrame('#iframe2');
		calcHelper.dblClickOnFirstCell();
		helper.typeIntoDocument('first{enter}');
		cy.getFrameWindow().then((win2) => {
			helper.processToIdle(win2);
		});

		// Second edit by the second view: re-enter cell edit mode so a text
		// cursor is shown again.
		calcHelper.dblClickOnFirstCell();
		cy.getFrameWindow().then((win2) => {
			helper.processToIdle(win2);
		});

		// The first view must see the second view's text cursor while it edits.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win1) => {
			helper.processToIdle(win1);

			// cy.wrap(...).should(callback) retries the callback until it
			// passes or times out, and on timeout reports the failing
			// assertion (with its label) instead of hanging silently.
			cy.wrap(win1).should((win) => {
				// Re-run the observer's own cursor-update path on every retry.
				// In the buggy code this is _onUpdateCursor -> updateVisibilities(true),
				// which forced every other view cursor object to opacity 0 and
				// never restored it. Triggering it here makes the regression
				// deterministic: a transient status: message can reset opacity
				// to 1 and otherwise mask the bug. The fixed code no longer
				// touches opacity, so this is a no-op for visibility.
				win.app.map._docLayer._onUpdateCursor();

				const sections = win.app.sectionContainer.sections.filter(
					(s) => s.name.startsWith('OtherViewCursor ')
				);
				expect(sections.length, 'other view cursor section count').to.be.greaterThan(0);

				const section = sections[0];
				const obj = section.getHTMLObject();
				expect(section.showSection, 'cursor showSection').to.be.true;
				expect(obj.style.display, 'cursor div display').to.not.equal('none');
				expect(obj.style.opacity, 'cursor div opacity').to.not.equal('0');
			});
		});
	});
});
