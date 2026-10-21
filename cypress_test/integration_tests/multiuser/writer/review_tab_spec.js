/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Recording tracked changes and the selected tab', function() {

	// Each frame keeps its own user interface mode, so switch the active one.
	function switchFrameToNotebookbar() {
		cy.getFrameWindow().then(function(win) {
			if (win.userInterfaceMode !== 'notebookbar') {
				cy.cGet('#menu-view').click();
				cy.cGet('#menu-toggleuimode').click();
			}
		});
		cy.cGet('#Home-tab-label').should('be.visible');
	}

	// Each frame takes half of the page, so the Tracking group of the Review tab is folded into
	// one button. Its menu holds the real Record button. The button id carries a uniqueness
	// suffix, so match on its prefix.
	function clickRecordOnReviewTab() {
		var menu = '[id^="overflow-button-review-tracking-dropdown"]';
		cy.cGet('#Review-container [id^="overflow-button-review-tracking"].menubutton')
			.should('be.visible')
			.find('.arrowbackground').click();
		cy.cGet(menu + ' .unoTrackChanges')
			.should('be.visible')
			.should('not.have.attr', 'disabled');
		cy.cGet(menu + ' .unoTrackChanges > button.unobutton').click();
	}

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/track_changes.odt', true);
		Cypress.env('USER_INTERFACE', 'notebookbar');

		cy.cSetActiveFrame('#iframe2');
		switchFrameToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win2 = win;
		}.bind(this));

		cy.cSetActiveFrame('#iframe1');
		switchFrameToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win1 = win;
		}.bind(this));
	});

	it('Another user keeps the Home tab when one user starts recording', function() {
		// The second user is on the Home tab and knows that recording is off.
		cy.cSetActiveFrame('#iframe2');
		helper.waitForMapState('.uno:TrackChanges', 'false');
		cy.cGet('#Home-tab-label').should('have.class', 'selected');

		// The first user starts recording from the Review tab.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#Review-tab-label').click();
		cy.cGet('#Review-tab-label').should('have.class', 'selected');
		clickRecordOnReviewTab();
		helper.processToIdle(this.win1);
		helper.waitForMapState('.uno:TrackChanges', 'true');

		// The second user sees recording on and stays on the Home tab.
		cy.cSetActiveFrame('#iframe2');
		helper.processToIdle(this.win2);
		helper.waitForMapState('.uno:TrackChanges', 'true');
		cy.cGet('#Home-tab-label').should('have.class', 'selected');
		cy.cGet('#Review-tab-label').should('not.have.class', 'selected');
	});
});
