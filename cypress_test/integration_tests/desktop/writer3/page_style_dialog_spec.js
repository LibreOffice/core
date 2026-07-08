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
	 * Checks that the Page Style dialog is not unreasonably wide, which would
	 * point to a broken css property. The Page Style dialog has more than four
	 * tabs, so the tabs render as a vertical rail beside the page content and
	 * the dialog is wider than the page content alone by the fixed rail width
	 * (about 150px). The upper bound allows for that rail while still catching a
	 * gross width blow-up.
	 */
	it('Page Style dialog width', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]')
			.should('be.visible')
			.invoke('width')
			.should('be.greaterThan', 588).and('be.lessThan', 800);
	});

	/*
	 * cool#7778: pageformatpage.ui puts "Orientation:" and "Top:" in the same GTK size
	 * group even though they sit in different rows of the Page tab, so their columns line
	 * up. The two labels differ a lot in length, so if the group is not honoured they end
	 * up with different widths.
	 */
	it('Page Style dialog aligns size-grouped labels', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]').should('be.visible');
		cy.cGet('button#page.ui-tab').click();

		helper.processToIdle(this.win);

		cy.cGet('#labelOrientation').should('be.visible').invoke('width').then((orientationWidth) => {
			cy.cGet('#labelTopMargin').should('be.visible').invoke('width').should('eq', orientationWidth);
		});
	});
});
