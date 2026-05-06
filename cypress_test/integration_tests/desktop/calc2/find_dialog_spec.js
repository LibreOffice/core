/* global describe it cy beforeEach expect require*/

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');
var findHelper = require('../../common/find_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching via find dialog.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/search_bar.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			calcHelper.assertAddressAfterIdle(this.win, 'A2');
		});
	});

	it('Search existing word.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');

		findHelper.findNext(this.win);
		// First cell should be selected
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B1');

		findHelper.typeIntoSearchField('c');
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C1');

		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'c');

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A301');
	});

	it('Search existing word when not following own view', function() {
		helper.processToIdle(this.win);
		desktopHelper.assertScrollbarPosition('vertical', 10, 30);

		cy.wrap(this.win.app).should((app) => {
			expect(app.isFollowingOff()).to.be.false;
		});

		desktopHelper.scrollViewDown(this.win);
		helper.processToIdle(this.win);
		desktopHelper.assertScrollbarPosition('vertical', 175, 205);

		cy.wrap(this.win.app).should((app) => {
			expect(app.isFollowingOff()).to.be.true;
		});

		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');
		helper.processToIdle(this.win);

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
		desktopHelper.assertScrollbarPosition('vertical', 10, 30);

		desktopHelper.scrollViewDown(this.win);
		helper.processToIdle(this.win);

		findHelper.typeIntoSearchField('c');
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C1');
		desktopHelper.assertScrollbarPosition('vertical', 10, 30);
	});

	it('Search not existing word.', function() {
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('q');

		// Should be no new selection
		calcHelper.assertAddressAfterIdle(this.win, 'A2');
	});

	it('Search next / prev instance.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('d');
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A472');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'd');

		// Search next instance
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'D1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'd');

		// Search prev instance
		findHelper.findPrev(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A472');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'd');
	});

	it('Search wrap at document end', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance, which is in the beginning of the document.
		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Jump to result.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/search_jump.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			calcHelper.assertAddressAfterIdle(this.win, 'C2707');
		});
	});

	it('Search existing word.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('result');

		findHelper.findNext(this.win);
		// First cell should be selected
		calcHelper.assertAddressAfterIdle(this.win, 'C8');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'result top');
		desktopHelper.assertScrollbarPosition('vertical', 0, 30);

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C300');
		desktopHelper.assertScrollbarPosition('vertical', 40, 60);

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C2303');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'result bottom');

		findHelper.findNext(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C8');
	});
});
