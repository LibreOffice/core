/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Track Changes', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/track_changes.odt');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function confirmChange(action) {
		cy.cGet('#toolbar-hamburger').click();
		cy.cGet('.menu-entry-icon.changesmenu').parent().click();
		if (action === 'Accept All') {
			cy.cGet('.menu-entry-icon.acceptalltrackedchanges').click();
		} else if (action === 'Reject All') {
			cy.cGet('.menu-entry-icon.rejectalltrackedchanges').click();
		}
	}
	//enable record for track changes
	function enableRecord(win) {
		cy.cGet('#toolbar-hamburger').click();
		cy.cGet('.menu-entry-icon.changesmenu').parent().click();
		cy.cGet('.menu-entry-icon.trackchanges').parent().click();

		//if we don't wait , the test will fail in CLI
		helper.processToIdle(win);

		cy.cGet('#toolbar-hamburger').click();
		cy.cGet('.menu-entry-icon.changesmenu').parent().click();
		cy.cGet('.menu-entry-icon.trackchanges').parent().should('have.class', 'menu-entry-checked');

		//to close
		cy.cGet('#toolbar-hamburger').click();
	}

	it('Accept All', function() {
		helper.typeIntoDocument('Hello World');
		helper.processToIdle(this.win);
		enableRecord(this.win);
		helper.selectAllText();
		helper.typeIntoDocument('{del}');
		//if we don't wait, the test will fail in CI
		helper.processToIdle(this.win);
		helper.selectAllText();
		confirmChange('Accept All');
		helper.typeIntoDocument('{ctrl}a');
		helper.textSelectionShouldNotExist();
	});

	it('Reject All',function() {
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('Hello World');
		helper.processToIdle(this.win);
		enableRecord(this.win);
		helper.selectAllText();
		helper.typeIntoDocument('{del}');
		helper.processToIdle(this.win);
		confirmChange('Reject All');
		cy.cGet('#document-container').click();
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('Hello World');
	});
});
