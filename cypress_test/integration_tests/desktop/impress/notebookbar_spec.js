/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Notebookbar tests', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('impress/statusbar.odp');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Symbol button reflects disabled state outside text edit mode', function() {
		// In Impress, core disables .uno:InsertSymbol (SID_CHARMAP) when there
		// is no active text edit (sd/source/ui/func/fubullet.cxx). Before the
		// fix the notebookbar button dispatched via a 'charmapcontrol' alias
		// whose state never matched the .uno: command, so it stayed enabled.
		// It should now reflect the disabled state.
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoCharmapControl').should('have.attr', 'disabled');
	});

	it('Ruler visible after reload', function() {
		// Visible check and enable if needed
		cy.cGet('#View-tab-label').click();

		cy.cGet('#showruler-input').then(($input) => {
			if (!$input.is(':checked')) {
				cy.wrap($input).check();
			}
		});

		// Verify ruler is visible
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// Reload
		helper.reloadDocument(newFilePath);

		// Verify ruler is still visible
		cy.cGet('.cool-ruler').should('be.visible');
	});

	it('New slide layout dropdown does not allow multi-selection via keyboard', function() {
		// Wide viewport so the (default) Home tab does not overflow the slide
		// layout group, keeping the "New Slide" split button on screen.
		cy.viewport(1920, 1080);

		// Open the "New Slide" split-button layout dropdown via its arrow.
		cy.cGet('.unoInsertPage.splitbutton:visible').first().find('.arrowbackground').click();
		cy.cGet('[id$="-dropdown"]:visible').should('exist');

		// The layout presets form a single-choice 4x4 grid.
		var cells = '[id$="-dropdown"]:visible .ui-grid-cell[index]';
		cy.cGet(cells).should('have.length', 16);

		// Sequence: arrow down, Tab, then arrow down again. With list
		// navigation this accumulated a second '.selected' cell; grid
		// navigation keeps the grid single-choice.
		cy.cGet(cells).first().focus();
		cy.realPress('ArrowDown');
		cy.realPress('Tab');
		cy.realPress('ArrowDown');

		// A single-choice grid must never show more than one selected cell.
		cy.cGet('[id$="-dropdown"]:visible').then(function($dropdown) {
			var selected = $dropdown.find('.ui-grid-cell.selected');
			expect(selected.length, 'selected layout cells').to.be.lessThan(2);
		});
	});
});
