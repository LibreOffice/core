/* global describe expect it cy beforeEach require */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Tab control keyboard navigation', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// The WAI-ARIA tabs pattern enters the panel on the cross-axis key, which
	// follows the rail orientation, so read it rather than naming a key.
	const enterPanelKey = ($tablist) =>
		$tablist.attr('aria-orientation') === 'vertical' ? 'ArrowRight' : 'ArrowDown';

	const openFormatCells = function (win) {
		calcHelper.dblClickOnFirstCell();
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FormatCellDialog');
		});
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		helper.processToIdle(win);
		cy.cGet('.ui-dialog [role="tablist"]').should('exist');
	};

	it('The cross-axis key moves focus to the first focusable of the tab page', function() {
		openFormatCells(this.win);

		cy.cGet('.ui-dialog [role="tab"]').first().focus();
		cy.cGet('.ui-dialog [role="tablist"]').then(($tablist) => {
			cy.realPress(enterPanelKey($tablist));
		});

		// Derive the expected target from the helper the handler itself uses.
		cy.cGet('.ui-dialog [role="tab"]').first().then(($tab) => {
			const win = this.win;
			const panel = win.document.getElementById($tab.attr('aria-controls'));
			const expected = win.JSDialog.FindFocusableWithin(panel, 'next');
			expect(expected, 'the tab page has a focusable widget').to.not.be.oneOf([null, undefined]);
			expect(win.document.activeElement).to.equal(expected);
		});

		helper.typeIntoDocument('{esc}');
	});

	it('A tab page with no focusable widget leaves the focus on the tab', function() {
		openFormatCells(this.win);

		cy.cGet('.ui-dialog [role="tab"]').first().focus();

		// Take every widget of the page out of the focusable set, so the
		// handler finds nothing to move the focus to.
		cy.cGet('.ui-dialog [role="tab"]').first().then(($tab) => {
			const win = this.win;
			const panel = win.document.getElementById($tab.attr('aria-controls'));
			panel.querySelectorAll('*').forEach((el) => el.classList.add('hidden'));
			expect(win.JSDialog.FindFocusableWithin(panel, 'next'),
				'no focusable left in the page').to.be.oneOf([null, undefined]);
		});

		cy.cGet('.ui-dialog [role="tablist"]').then(($tablist) => {
			cy.realPress(enterPanelKey($tablist));
		});

		// Nothing to focus must not throw: the focus stays where it was.
		cy.cGet('.ui-dialog [role="tab"]').first().should('have.focus');

		helper.typeIntoDocument('{esc}');
	});
});
