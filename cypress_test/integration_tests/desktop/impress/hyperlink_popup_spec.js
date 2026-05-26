/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress hyperlink popup tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/top_toolbar.odp');
		desktopHelper.switchUIToCompact();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebar();
		} else {
			desktopHelper.hideSidebarImpress();
		}

		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});

		impressHelper.selectTextShapeInTheCenter();
		impressHelper.dblclickOnSelectedShape();

		helper.typeIntoDocument('{ctrl}a');
		helper.typeIntoDocument('{del}');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument('before ');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument('{ctrl}k');
		cy.cGet('#target-input').should('be.visible');
		cy.cGet('#indication-input').clear();
		cy.cGet('#indication-input').type('linktext');
		cy.cGet('#target-input').type('http://www.example.com/');
		cy.cGet('#ok').click();
		cy.cGet('#target-input').should('not.exist');
		cy.then(() => helper.processToIdle(this.win));

		// InsertURLField selects the newly inserted field. Press right
		// arrow to collapse the selection past the field so that typing
		// does not replace the URL field.
		helper.typeIntoDocument('{rightArrow}');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument(' after');
		cy.then(() => helper.processToIdle(this.win));
	});

	it('Popup appears when cursor is at the start of hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Move right to reach the start of the hyperlink field.
		// "before " is 7 characters, so 7 right-arrow presses
		// lands at the field boundary.
		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up').should('have.text', 'http://www.example.com/');
	});

	it('Popup appears when cursor is at the end of hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Move right past the hyperlink field. "before " is 7 chars,
		// the field is 1 dummy char, so 8 right-arrow presses lands
		// just after the field.
		for (var i = 0; i < 8; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up').should('have.text', 'http://www.example.com/');
	});

	it('Popup disappears after navigating away from hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Navigate into the hyperlink field.
		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');

		// Navigate past the field and the trailing text to a position
		// that is clearly outside the hyperlink.
		helper.typeIntoDocument('{end}');
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('not.exist');
	});
});
