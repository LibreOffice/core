/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Context toolbar with table cell selection.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/context_toolbar_table.fodt');
		desktopHelper.switchUIToNotebookbar();
	});

	// Retries until the view's text selection flag has the expected value.
	function waitForTextSelection(expected) {
		cy.getFrameWindow().should(function(win) {
			expect(win.app.activeDocument.activeView.hasTextSelection).to.equal(expected);
		});
	}

	// Waits until the editor has processed everything sent so far.
	function settle() {
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
	}

	// Double-click at the cursor in the first paragraph: selects a word, shows the toolbar.
	function selectWordWithMouse() {
		helper.getBlinkingCursorPosition('wordPos');
		helper.clickAt('wordPos', true);
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
		waitForTextSelection(true);
	}

	// Locate cells A1 and B2 with the text cursor, then click A1 and shift+click B2.
	// The toolbar only reacts to a mouse button-up, so the keyboard cannot be used here.
	function selectCellsWithMouse() {
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}');
		helper.getBlinkingCursorPosition('cellA1');
		helper.typeIntoDocument('{rightarrow}');
		helper.typeIntoDocument('{downarrow}');
		helper.getBlinkingCursorPosition('cellB2');

		cy.get('@cellA1').then(function(from) {
			cy.cGet('body').click(from.x, from.y);
		});
		// Any earlier word selection is gone, so the next 'true' is the cell selection.
		waitForTextSelection(false);

		cy.get('@cellB2').then(function(to) {
			cy.cGet('body').click(to.x, to.y, { shiftKey: true });
		});
		waitForTextSelection(true);
		settle();
	}

	function assertNoToolbar() {
		cy.cGet('#context-toolbar').should('have.class', 'hidden');

		cy.getFrameWindow().should(function(win) {
			expect(win.eval('TextSelections.getStartRectangle()')).to.equal(null);
		});
	}

	it('No context toolbar for empty table cell selection.', function() {
		selectCellsWithMouse();
		assertNoToolbar();
	});

	it('Context toolbar stays hidden for cell selection after a text selection.', function() {
		selectWordWithMouse();
		selectCellsWithMouse();
		assertNoToolbar();
	});
});
