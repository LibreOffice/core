/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const redlineHelper = require('../../common/redline_helper');

describe(['tagredline', 'tagnextcloud', 'tagproxy'], 'Track Changes as comments', function() {

	beforeEach(function () {
		cy.viewport(1400, 600);
		helper.setupAndLoadDocument('writer/track_changes.odt');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up [id^="sidebar"] button:visible').click();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Text redlines are visible as comments', function () {
		redlineHelper.enableRecord();
		helper.typeIntoDocument('Hello World');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-accept-button').should('exist');
		cy.cGet('.cool-redline-reject-button').should('exist');
	});


	it('Tracked deletion of comment shows visual indicators', function () {
		desktopHelper.insertComment('test comment');
		helper.typeIntoDocument('{home}');
		cy.cGet('div.cool-annotation').should('have.length', 1);

		redlineHelper.enableRecord();

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();

		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'test comment');
		cy.cGet('#comment-container-1 .cool-annotation-content-removed').should('contain', 'Removed');
		cy.cGet('#comment-container-1 .cool-annotation-menubar').should('have.css', 'display', 'none');
	});

	it('Accept text redline via annotation button', function () {
		helper.setDummyClipboardForCopy();
		redlineHelper.enableRecord();
		helper.typeIntoDocument('Hello');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-accept-button').click();
		cy.cGet('.cool-annotation-redline-content-wrapper').should('not.exist');

		helper.selectAllText();
		cy.wait(500);
		helper.copy();
		helper.expectTextForClipboard('Hello');
	});

	it('Reject text redline via annotation button', function () {
		redlineHelper.enableRecord();
		helper.typeIntoDocument('Hello');

		cy.cGet('.cool-annotation-redline-content-wrapper').should('exist');
		cy.cGet('.cool-redline-reject-button').click();
		cy.cGet('.cool-annotation-redline-content-wrapper').should('not.exist');

		helper.typeIntoDocument('{ctrl}a');
		cy.wait(500);
		helper.textSelectionShouldNotExist();
	});

	it('Accept All', function () {
		helper.typeIntoDocument('Hello World');
		cy.wait(500);
		desktopHelper.insertComment('some text');
		helper.typeIntoDocument('{home}');

		redlineHelper.enableRecord();

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();
		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');

		helper.clearAllText();
		helper.selectAllText();
		cy.wait(500);
		redlineHelper.confirmChange('Accept All');
		helper.typeIntoDocument('{ctrl}a');
		cy.wait(500);
		helper.textSelectionShouldNotExist();
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('Reject All', function () {
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('Hello World');
		cy.wait(500);
		desktopHelper.insertComment('some text');
		helper.typeIntoDocument('{home}');

		redlineHelper.enableRecord();

		cy.cGet('#comment-container-1 .cool-annotation-menubar .cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Remove').click();
		cy.cGet('#comment-container-1').should('have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');

		helper.clearAllText();
		helper.selectAllText();
		cy.wait(500);
		redlineHelper.confirmChange('Reject All');
		cy.cGet('#document-container').click();
		helper.selectAllText();
		cy.wait(500);
		helper.copy();
		helper.expectTextForClipboard('Hello World');
		cy.cGet('#comment-container-1').should('not.have.class', 'tracked-deleted-comment-show');
		cy.cGet('#comment-container-1').should('contain', 'some text');
	});
});
