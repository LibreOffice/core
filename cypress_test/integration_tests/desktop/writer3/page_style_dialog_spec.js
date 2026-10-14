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

	/*
	 * cool#16155: the Style and Color rows of the Border tab hold menu buttons, the Thickness
	 * row a list box. All three sit in the same grid column, so their left edges line up.
	 */
	it('Border tab lines up the controls of the Line section', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();

		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]').should('be.visible');
		cy.cGet('button#borders.ui-tab').click();

		helper.processToIdle(this.win);

		cy.cGet('#linewidthlb').should('be.visible').invoke('offset').then((thickness) => {
			cy.cGet('#linestylelb').should('be.visible').invoke('offset').its('left').should('eq', thickness.left);
			cy.cGet('#linecolorlb').should('be.visible').invoke('offset').its('left').should('eq', thickness.left);
		});
	});

	/*
	 * The Page Style dialog takes no input while the color picker opened from its Area tab is
	 * open, so it cannot be closed with the color picker left behind.
	 */
	it('Page Style dialog is blocked while the color picker is open', function() {
		const pageStyleWindow = () => cy.cGet('[id^="TemplateDialog"]').closest('.jsdialog-window');

		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#edit-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="ColorPicker"]').should('be.visible');
		pageStyleWindow().should('have.attr', 'inert');

		cy.cGet('[id^="ColorPicker"] #cancel-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="ColorPicker"]').should('not.exist');
		pageStyleWindow().should('not.have.attr', 'inert');

		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="TemplateDialog"]').should('not.exist');
	});

	/*
	 * The Area tab offers None, Color and Pattern itself. Gradient, Hatch and Image are tabs
	 * of the Fill Effects dialog that More effects opens, and the effect chosen there is the
	 * one it opens on next time.
	 */
	it('Area tab opens the other fills in Fill Effects', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);

		cy.cGet('[id^="TemplateDialog"]').should('be.visible');
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('[role="tab"][aria-controls="lbcolor"]').should('exist');
		cy.cGet('[role="tab"][aria-controls="lbhatch"]').should('not.exist');

		cy.cGet('#moreeffects-button').should('be.visible').click();
		helper.processToIdle(this.win);

		cy.cGet('[id^="FillEffectsDialog"]').should('be.visible').within(() => {
			cy.get('[role="tab"]').then(($tabs) => {
				const labels = $tabs.toArray().map((tab) => tab.textContent.trim());
				expect(labels).to.deep.equal(['Gradient', 'Hatch', 'Image']);
			});
			cy.get('[role="tab"][aria-controls="gradient"]').should('have.attr', 'aria-selected', 'true');
			cy.get('[role="tab"][aria-controls="hatch"]').click();
		});
		helper.processToIdle(this.win);
		cy.cGet('[id^="FillEffectsDialog"] #ok-button').click();
		helper.processToIdle(this.win);

		cy.cGet('[id^="FillEffectsDialog"]').should('not.exist');
		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);

		// The Format tab is still selected, so the button is there to click again.
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);
		cy.cGet('#moreeffects-button').click();
		helper.processToIdle(this.win);

		cy.cGet('[id^="FillEffectsDialog"] [role="tab"][aria-controls="hatch"]')
			.should('have.attr', 'aria-selected', 'true');
	});

	/*
	 * None is the first entry of the palette list on the Color tab of the Area page. Selecting
	 * a color leaves None, and a page style without a fill opens with None selected.
	 */
	it('Area tab offers None in the palette list', function() {
		var openArea = () => {
			desktopHelper.getNbIcon('PageDialog', 'Format').click();
			helper.processToIdle(this.win);
			cy.cGet('[id^="TemplateDialog"]').should('be.visible');
			cy.cGet('button#area.ui-tab').click();
			helper.processToIdle(this.win);
		};
		var closeWithOk = () => {
			cy.cGet('[id^="TemplateDialog"] #ok-button').click();
			helper.processToIdle(this.win);
		};

		cy.cGet('#Format-tab-label').click();
		openArea();

		cy.cGet('[role="tab"][aria-controls="lbnone"]').should('not.exist');
		cy.cGet('#paletteselector-input option').first().should('have.text', 'None');

		cy.cGet('#iconview_colors .ui-iconview-entry').eq(5).click();
		helper.processToIdle(this.win);
		cy.cGet('#paletteselector-input option:selected').should('not.have.text', 'None');
		closeWithOk();

		openArea();
		cy.cGet('#paletteselector-input option:selected').should('not.have.text', 'None');
		cy.cGet('#paletteselector-input').select('None');
		helper.processToIdle(this.win);
		closeWithOk();

		openArea();
		cy.cGet('#paletteselector-input option:selected').should('have.text', 'None');
	});

	/*
	 * Pattern Style opens a grid of None and the shipped patterns, named by their tooltips, with
	 * Custom below it. The pattern selected there is the fill that the page style keeps, and
	 * Custom opens the pattern editor in a dialog of its own.
	 */
	it('Area tab offers the patterns in Pattern Style', function() {
		var openArea = () => {
			desktopHelper.getNbIcon('PageDialog', 'Format').click();
			helper.processToIdle(this.win);
			cy.cGet('[id^="TemplateDialog"]').should('be.visible');
			cy.cGet('button#area.ui-tab').click();
			helper.processToIdle(this.win);
		};
		var openStyleGrid = () => {
			cy.cGet('button#patternstyle').click();
			helper.processToIdle(this.win);
			cy.cGet('#patternstylelist').should('be.visible');
		};

		cy.cGet('#Format-tab-label').click();
		openArea();
		cy.cGet('button#patternstyle').should('contain.text', 'None');

		openStyleGrid();
		// The entries show only their pattern, and the name is in the tooltip.
		cy.cGet('#patternstylelist .ui-iconview-entry-title').should('not.exist');
		cy.cGet('#patternstylelist img[alt="None"]').should('exist');
		cy.cGet('#patternstylelist .ui-iconview-entry').then(($entries) => {
			const tops = $entries.toArray().slice(0, 5).map((entry) => entry.offsetTop);
			expect(tops.slice(0, 4).every((top) => top === tops[0]), 'four in the first row').to.be.true;
			expect(tops[4], 'the fifth starts a new row').to.be.greaterThan(tops[0]);
		});
		cy.cGet('#patternstylelist img[alt="20 Percent"]').click();
		helper.processToIdle(this.win);
		cy.cGet('button#patternstyle').should('contain.text', '20 Percent');

		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);

		openArea();
		cy.cGet('button#patternstyle').should('contain.text', '20 Percent');

		openStyleGrid();
		cy.cGet('#patternstylecustom-button').click();
		helper.processToIdle(this.win);

		// The Pattern dialog holds the pattern editor alone, with OK below it and no tab strip.
		cy.cGet('#BTN_ADD').should('be.visible').closest('.jsdialog-window').within(($dialog) => {
			cy.get('.ui-dialog-title').should('have.text', 'Pattern');
			cy.get('[role="tab"]').should('not.exist');
			cy.get('#CTL_PIXEL').then(($editor) => {
				const okButton = $dialog[0].querySelector('#ok-button, #ok');
				expect(okButton.getBoundingClientRect().top, 'OK sits below the editor')
					.to.be.greaterThan($editor[0].getBoundingClientRect().bottom);
			});
		});
	});
});
