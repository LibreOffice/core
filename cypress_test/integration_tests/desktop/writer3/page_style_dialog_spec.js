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

		cy.cGet('#customcolor-button').click();
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

		cy.cGet('#moreeffects-button').click();
		helper.processToIdle(this.win);

		cy.cGet('[id^="FillEffectsDialog"]').should('be.visible');

		cy.cGet('[id^="FillEffectsDialog"] [role="tab"][aria-controls="hatch"]').click();
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
	 * The pattern selected in Pattern Style is the fill that the page style keeps, and Custom
	 * pattern opens the pattern editor in a dialog of its own.
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
		cy.cGet('#patternstylelist img[alt="20 Percent"]').click();
		helper.processToIdle(this.win);
		cy.cGet('button#patternstyle').should('contain.text', '20 Percent');

		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);

		openArea();
		cy.cGet('button#patternstyle').should('contain.text', '20 Percent');

		cy.cGet('#custompattern-button').click();
		helper.processToIdle(this.win);
		cy.cGet('#CTL_PIXEL').should('be.visible').closest('.jsdialog-window').find('#cancel-button').click();
		helper.processToIdle(this.win);
		cy.cGet('#CTL_PIXEL').should('not.exist');
	});

	/*
	 * The Color tab has the palette with More colors and More effects below it, Pattern Style to
	 * the right, and one preview under both. The caption names the fill the preview shows.
	 */
	it('Area tab previews the fill under the palette', function() {
		var caption = () => cy.cGet('#previewcaption');
		var rect = (selector) => cy.cGet(selector).then(($element) => $element[0].getBoundingClientRect());

		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		rect('#iconview_colors').then((palette) => {
			rect('button#patternstyle').then((patternStyle) => {
				expect(patternStyle.left, 'Pattern Style is right of the palette').to.be.greaterThan(palette.right);
			});
			rect('#preview').then((preview) => {
				expect(preview.top, 'the preview is under the palette').to.be.greaterThan(palette.bottom);
				expect(preview.width, 'the preview is wider than the palette').to.be.greaterThan(palette.width);
			});
		});

		// The page style opens with a color fill.
		caption().invoke('text').should('match', /^Color: /);

		cy.cGet('#iconview_colors .ui-iconview-entry').eq(5).click();
		helper.processToIdle(this.win);
		caption().invoke('text').should('match', /^Color: .+ \(#[0-9A-F]{6}\)$/);

		cy.cGet('button#patternstyle').click();
		helper.processToIdle(this.win);
		cy.cGet('#patternstylelist img[alt="20 Percent"]').click();
		helper.processToIdle(this.win);
		caption().should('have.text', 'Pattern: 20 Percent');

		cy.cGet('#moreeffects-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="FillEffectsDialog"] [role="tab"][aria-controls="hatch"]').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="FillEffectsDialog"] #ok-button').click();
		helper.processToIdle(this.win);
		caption().invoke('text').should('match', /^Hatch: /);

		cy.cGet('#paletteselector-input').select('None');
		helper.processToIdle(this.win);
		caption().should('have.text', 'None');
	});

	/*
	 * Each kind of fill set on the Area tab is the fill the page style keeps: the preview caption
	 * names it when the dialog opens again.
	 */
	it('Area tab sets each kind of fill', function() {
		var caption = () => cy.cGet('#previewcaption');
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
			cy.cGet('[id^="TemplateDialog"]').should('not.exist');
		};
		var setEffect = (tab) => {
			cy.cGet('#moreeffects-button').click();
			helper.processToIdle(this.win);
			cy.cGet('[id^="FillEffectsDialog"] [role="tab"][aria-controls="' + tab + '"]').click();
			helper.processToIdle(this.win);
			cy.cGet('[id^="FillEffectsDialog"] #ok-button').click();
			helper.processToIdle(this.win);
			cy.cGet('[id^="FillEffectsDialog"]').should('not.exist');
		};

		cy.cGet('#Format-tab-label').click();

		openArea();
		cy.cGet('#paletteselector-input').select('Standard');
		helper.processToIdle(this.win);
		cy.cGet('#iconview_colors .ui-iconview-entry').eq(56).click();
		helper.processToIdle(this.win);
		closeWithOk();
		openArea();
		caption().should('have.text', 'Color: Light Blue 2 (#729FCF)');

		setEffect('gradient');
		closeWithOk();
		openArea();
		caption().should('have.text', 'Gradient: Pastel Bouquet');

		setEffect('hatch');
		closeWithOk();
		openArea();
		caption().should('have.text', 'Hatch: Black 0 Degrees');

		setEffect('bitmap');
		closeWithOk();
		openArea();
		caption().should('have.text', 'Image: Painted White');

		cy.cGet('button#patternstyle').click();
		helper.processToIdle(this.win);
		cy.cGet('#patternstylelist img[alt="5 Percent"]').click();
		helper.processToIdle(this.win);
		closeWithOk();
		openArea();
		caption().should('have.text', 'Pattern: 5 Percent');

		cy.cGet('#paletteselector-input').select('None');
		helper.processToIdle(this.win);
		closeWithOk();
		openArea();
		caption().should('have.text', 'None');
	});

	/*
	 * A color can be selected from any palette, and the last color the page style was filled
	 * with leads Recent Colors the next time.
	 */
	it('Area tab selects colors from each palette and lists the last one in Recent Colors', function() {
		var caption = () => cy.cGet('#previewcaption');

		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#paletteselector-input').select('Chart Palettes');
		helper.processToIdle(this.win);
		cy.cGet('#iconview_colors .ui-iconview-entry').eq(1).click();
		helper.processToIdle(this.win);
		caption().invoke('text').should('match', /^Color: Chart 2 \(#[0-9A-F]{6}\)$/);

		cy.cGet('#paletteselector-input').select('HTML');
		helper.processToIdle(this.win);
		cy.cGet('#iconview_colors .ui-iconview-entry').eq(0).click();
		helper.processToIdle(this.win);
		caption().should('have.text', 'Color: White (#FFFFFF)');

		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="TemplateDialog"]').should('not.exist');

		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);
		// Another color first, so it is the recent color that brings White back.
		caption().should('have.text', 'Color: White (#FFFFFF)');
		cy.cGet('#iconview_colors .ui-iconview-entry').eq(1).click();
		helper.processToIdle(this.win);
		caption().should('not.have.text', 'Color: White (#FFFFFF)');
		cy.cGet('#iconview_recent_colors .ui-iconview-entry').first().click();
		helper.processToIdle(this.win);
		caption().should('have.text', 'Color: White (#FFFFFF)');
	});

	/*
	 * A color typed in Pick a Color is the fill, and stays the fill when the dialog opens again.
	 */
	it('Area tab fills with a color typed in Pick a Color', function() {
		var caption = () => cy.cGet('#previewcaption');

		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#customcolor-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="ColorPicker"] #hexEntry-input').type('{selectall}{backspace}23a9d3');
		helper.processToIdle(this.win);
		cy.cGet('[id^="ColorPicker"] #ok-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="ColorPicker"]').should('not.exist');
		caption().should('have.text', 'Color: #23A9D3');

		cy.cGet('[id^="TemplateDialog"] #ok-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="TemplateDialog"]').should('not.exist');

		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);
		caption().should('have.text', 'Color: #23A9D3');
	});

	/*
	 * Add to Palette shows a swatch of the color it would add, so selecting another color
	 * changes it.
	 */
	it('Add to Palette shows the selected color', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		// The first swatch is read once, after should has waited for it, to compare with the next.
		cy.cGet('#add-button img').should('have.attr', 'src').then((before) => {
			cy.cGet('#iconview_colors .ui-iconview-entry').eq(64).click();
			helper.processToIdle(this.win);
			cy.cGet('#add-button img').should('have.attr', 'src').and('not.equal', before);
		});
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Page Style dialog of a page without a fill', function() {
	beforeEach(function() {
		cy.viewport(1920, 720);
		helper.setupAndLoadDocument('writer/focus.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	/*
	 * A page without a fill opens the Area tab on None in the palette list and in Pattern Style,
	 * with None in the preview. Selecting a color moves the palette list to the palette shown.
	 */
	it('Area tab opens on None and leaves it when a color is selected', function() {
		cy.cGet('#Format-tab-label').click();
		desktopHelper.getNbIcon('PageDialog', 'Format').click();
		helper.processToIdle(this.win);
		cy.cGet('button#area.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#paletteselector-input option:selected').should('have.text', 'None');
		cy.cGet('button#patternstyle').should('contain.text', 'None');
		cy.cGet('#previewcaption').should('have.text', 'None');

		cy.cGet('#iconview_colors .ui-iconview-entry').eq(5).click();
		helper.processToIdle(this.win);
		cy.cGet('#paletteselector-input option:selected').should('have.text', 'Standard');
		cy.cGet('#previewcaption').invoke('text').should('match', /^Color: /);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Page Style dialog of a spreadsheet', function() {
	beforeEach(function() {
		cy.viewport(1920, 720);
		helper.setupAndLoadDocument('calc/autofilter.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	/*
	 * The Background tab offers no pattern, so More effects sits in the row of Custom color and
	 * opens Fill Effects from there.
	 */
	it('Background tab opens Fill Effects from beside Custom color', function() {
		this.win.app.map.sendUnoCommand('.uno:PageFormatDialog');
		helper.processToIdle(this.win);
		cy.cGet('button#background.ui-tab').click();
		helper.processToIdle(this.win);

		cy.cGet('#colorbuttonbox #moreeffects-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="FillEffectsDialog"]').should('be.visible');
		cy.cGet('[id^="FillEffectsDialog"] #cancel-button').click();
		helper.processToIdle(this.win);
		cy.cGet('[id^="FillEffectsDialog"]').should('not.exist');
	});
});
