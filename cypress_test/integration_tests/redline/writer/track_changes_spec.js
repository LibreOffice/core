/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const redlineHelper = require('../../common/redline_helper');

describe(['tagredline', 'tagnextcloud', 'tagproxy'], 'Track Changes as comments', function() {

	beforeEach(function () {
		cy.viewport(1400, 600);
		helper.setupAndLoadDocument('writer/track_changes.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up [id^="sidebar"] button:visible').click();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Text redlines are visible as comments', function () {
		redlineHelper.enableRecord(this.win);
		helper.typeIntoDocument('Hello World');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-accept-button').should('exist');
		cy.cGet('.cool-redline-reject-button').should('exist');
	});


	it('Tracked deletion of comment shows visual indicators', function () {
		desktopHelper.insertComment('test comment');
		helper.typeIntoDocument('{home}');
		cy.cGet('div.cool-annotation').should('have.length', 1);

		redlineHelper.enableRecord(this.win);

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();

		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'test comment');
		cy.cGet('#comment-container-1 .cool-annotation-content-removed').should('contain', 'Removed');
		cy.cGet('#comment-container-1 .cool-annotation-menubar').should('have.css', 'display', 'none');
	});

	it('Accept text redline via annotation button', function () {
		helper.setDummyClipboardForCopy();
		redlineHelper.enableRecord(this.win);
		helper.typeIntoDocument('Hello');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-accept-button').click();
		cy.cGet('.cool-annotation-redline-content-wrapper').should('not.exist');

		helper.selectAllText();
		helper.processToIdle(this.win);
		helper.copy();
		helper.expectTextForClipboard('Hello');
	});

	it('Reject text redline via annotation button', function () {
		redlineHelper.enableRecord(this.win);
		helper.typeIntoDocument('Hello');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-reject-button').click();
		cy.cGet('.cool-annotation-redline-content-wrapper').should('not.exist');

		helper.typeIntoDocument('{ctrl}a');
		helper.processToIdle(this.win);
		helper.textSelectionShouldNotExist();
	});

	it('Accept All', function () {
		helper.typeIntoDocument('Hello World');
		helper.processToIdle(this.win);
		desktopHelper.insertComment('some text');
		helper.typeIntoDocument('{home}');

		redlineHelper.enableRecord(this.win);

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();
		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');

		helper.clearAllText();
		helper.selectAllText();
		helper.processToIdle(this.win);
		redlineHelper.confirmChange(this.win, 'Accept All');
		helper.typeIntoDocument('{ctrl}a');
		helper.processToIdle(this.win);
		helper.textSelectionShouldNotExist();
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('Reject All', function () {
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('Hello World');
		helper.processToIdle(this.win);
		desktopHelper.insertComment('some text');
		helper.typeIntoDocument('{home}');

		redlineHelper.enableRecord(this.win);

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();
		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');

		helper.clearAllText();
		helper.selectAllText();
		helper.processToIdle(this.win);
		redlineHelper.confirmChange(this.win, 'Reject All');
		cy.cGet('#document-container').click();
		helper.selectAllText();
		helper.processToIdle(this.win);
		helper.copy();
		helper.expectTextForClipboard('Hello World');
		cy.cGet('#comment-container-1').should('not.have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');
	});
});
