/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Animated GIFs in a text document', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/image_operation.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			return helper.processToIdle(win);
		}.bind(this));
	});

	it('An animated GIF plays only while it is selected', function() {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoInsertGraphic').filter(':visible').click();
		cy.cGet('#insertgraphic[type=file]').attachFile('/desktop/impress/animated_to_insert.gif');

		// The inserted image is selected, so it plays as an overlay section.
		cy.cGet('#document-container svg g').should('exist');
		cy.cGet('[id^="test-div-AnimatedGif-"]').should('exist');

		// Once deselected the tiles show its first frame and the overlay goes.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#document-container svg g').should('not.exist');
		cy.cGet('[id^="test-div-AnimatedGif-"]').should('not.exist');

		// Selected again, it plays again.
		cy.then(function() {
			this.win.app.map.sendUnoCommand('.uno:JumpToNextFrame');
		}.bind(this));
		cy.cGet('#document-container svg g').should('exist');
		cy.cGet('[id^="test-div-AnimatedGif-"]').should('exist');
	});

	it('A still image gets no overlay', function() {
		desktopHelper.insertImage();
		cy.cGet('#document-container svg g').should('exist');
		helper.processToIdle(this.win);
		cy.cGet('[id^="test-div-AnimatedGif-"]').should('not.exist');
	});
});
