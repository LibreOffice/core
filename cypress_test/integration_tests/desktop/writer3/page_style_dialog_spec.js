/* global describe it cy beforeEach require expect */

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
	 * (about 150px). The dialog opens at the width of its widest page, which is the
	 * Page tab. How narrow that page can be laid out is up to the browser, and the
	 * Chromium the CI runs keeps it some 250px wider than a current Chrome does, so
	 * the upper bound has to hold for both while still catching a gross width
	 * blow-up.
	 */
	it('Page Style dialog width', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]')
			.should('be.visible')
			.invoke('width')
			.should('be.greaterThan', 588).and('be.lessThan', 1200);
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

	/*
	 * The tab page area is sized once to the largest page and cuts off whatever
	 * does not fit, so a page measured narrower than it renders loses the right
	 * hand side of its content. The Page tab is the widest page of this dialog, so
	 * it is the one that goes over the edge, and it stays over the edge when the
	 * tab is left and opened again.
	 */
	it('Page Style dialog shows the whole Page tab after a tab switch', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]').should('be.visible');
		cy.cGet('button#page.ui-tab').click();
		helper.processToIdle(this.win);
		cy.cGet('button#organizer.ui-tab').click();
		helper.processToIdle(this.win);
		cy.cGet('button#page.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#PageFormatPage').should('be.visible');
		cy.cGet('.ui-tabs-content.jsdialog').then(function($tabArea) {
			const areaRight = $tabArea[0].getBoundingClientRect().right;
			cy.cGet('#PageFormatPage').find('*').then(function($widgets) {
				const widest = Array.prototype.reduce.call($widgets, function(max, widget) {
					return Math.max(max, widget.getBoundingClientRect().right);
				}, 0);

				// The frames of a page carry a 2px margin around a box that is already
				// as wide as the page, so they reach that far past the edge on a page
				// that fits. A page that is cut off misses tens of pixels.
				expect(widest - areaRight,
					'pixels of the Page tab reaching past the tab page area')
					.to.be.at.most(4);
			});
		});
	});
});
