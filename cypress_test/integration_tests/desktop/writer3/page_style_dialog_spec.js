/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper')

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Page Style dialog tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/styles.odt');
		cy.viewport(1920, 720);
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	/*
	 * tests if the width of the 'Page Style dialog' is larger than a "reasonable"
	 * width and if it's larger that means something is obviously wrong, probably
	 * some css property.
	 * `reasonableWidth` = width at the time of writing this test + 15px ;)
	 */
	it('Page Style dialog width', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]')
			.should('be.visible')
			.invoke('width')
			.should('be.greaterThan', 588).and('be.lessThan', 605);
	});
});
