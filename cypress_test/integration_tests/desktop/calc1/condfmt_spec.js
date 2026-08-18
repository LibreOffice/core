/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

// Open a new Date condition from the Conditional menu of the Home tab. A cell that already
// carries a conditional format brings up a question about editing that format first, so start
// from a cell that has none.
function openDateCondition() {
	calcHelper.selectCellsInRange('E10');
	cy.cGet('#toolbar-up #Home .unoConditionalFormatMenu:visible').click();
	desktopHelper.getDropdown('home-conditional-format-menu').click();
	cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', 'Date...').click();
}

describe(['tagdesktop'], 'Conditional Format Dialog Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/condfmt.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	it('JSDialog conditional formatting', function() {
		cy.cGet('#toolbar-up #Home .unoConditionalFormatMenu:visible').click();

		desktopHelper.getDropdown('home-conditional-format-menu').click();

		// Should not have the first entry selected by default.
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', 'Highlight cells with').parent().should('not.have.class', 'selected');
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', 'Manage...').parent().should('not.have.class', 'selected');

		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', 'Manage...').click();

		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Edit').click();
		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Down').should('be.visible');
		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Add').click();
		const span = cy.cGet('div.ui-grid.ui-grid-cell')
			.contains('span.jsdialog.static-label.ui-text', 'Condition 5');
		span.should('have.length', 1);
		span.should('have.prop', 'tagName', 'SPAN');
		span.should('be.visible');
		const topDiv = span.parent().parent();
		topDiv.should('have.length', 1);
		topDiv.should('have.prop', 'tagName', 'DIV');
		topDiv.should('be.visible');
		// Second row(div), inside that third column(div), inside that first part(div).
		const input = topDiv.children().eq(1).children().eq(2).children().eq(0).find('input.ui-edit.jsdialog');
		input.should('be.visible');
		input.should('have.length', 1);
		input.type('1331');
		// Without the fix only the first char '1' will be in the input box.
		input.should('have.value', '1331');
	});

	function checkSubmenuGridHasEvenColumnSpacing(entryText, iconPrefix) {
		cy.cGet('#toolbar-up #Home .unoConditionalFormatMenu:visible').click();
		desktopHelper.getDropdown('home-conditional-format-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', entryText).click();

		cy.cGet('#conditionalformatmenu-grid').should('be.visible');
		cy.cGet('#conditionalformatmenu-grid button.' + iconPrefix + '00' +
			', #conditionalformatmenu-grid button.' + iconPrefix + '01' +
			', #conditionalformatmenu-grid button.' + iconPrefix + '02')
			.should('have.length', 3)
			.then(function(buttons) {
				const left0 = buttons[0].getBoundingClientRect().left;
				const left1 = buttons[1].getBoundingClientRect().left;
				const left2 = buttons[2].getBoundingClientRect().left;
				// Without the fix, the "More..." button at the bottom of the grid
				// does not span the full row, so it widens the first column
				// instead, and the gap before the second column no longer
				// matches the gap before the third.
				expect(left1 - left0).to.be.closeTo(left2 - left1, 1);
			});
	}

	it('Data bar submenu grid has even column spacing', function() {
		checkSubmenuGridHasEvenColumnSpacing('Data Bar', 'databarset');
	});

	it('Color scale submenu grid has even column spacing', function() {
		checkSubmenuGridHasEvenColumnSpacing('Color Scale', 'scaleset');
	});

	it('an ODF spreadsheet offers every date period', function() {
		openDateCondition();

		cy.cGet('#datetype-input option').should('have.length', 13);
		cy.cGet('#datetype-input option').first().should('have.text', 'Today');
		cy.cGet('#datetype-input option').last().should('have.text', 'Next year');
	});
});

describe(['tagdesktop'], 'Conditional Format Date Conditions In An Excel Format', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/testfile.xlsx');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	it('the whole year periods are not offered', function() {
		openDateCondition();

		cy.cGet('#datetype-input option').should('have.length', 10);
		cy.cGet('#datetype-input option').first().should('have.text', 'Today');
		cy.cGet('#datetype-input option').last().should('have.text', 'Next month');
	});
});
