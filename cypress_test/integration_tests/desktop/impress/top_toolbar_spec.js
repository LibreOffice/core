/* global describe it cy before beforeEach require Cypress */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Top toolbar tests.', { testIsolation: false }, function() {

	// No viewport is set here: this spec drives the compact toolbar, whose items move
	// into the overflow when the window size changes.
	desktopHelper.shareDocumentAcrossTests('impress/top_toolbar.odp');

	// Both of these hold for the whole file. Hiding the sidebar in particular is a
	// toggle that starts from "it is showing", so it happens once.
	before(function() {
		desktopHelper.switchUIToCompact();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebar();
		} else {
			desktopHelper.hideSidebarImpress();
		}
	});

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});

		// The single click in the middle means "select the shape", which needs a slide
		// with nothing selected: from a shape that is already selected the same click
		// starts editing its text and no rotation handle appears.
		impressHelper.removeShapeSelection();

		impressHelper.selectTextShapeInTheCenter();
	});

	it('Apply bold on text shape.', function() {
		desktopHelper.getCompactIcon('Bold').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-weight', '700');
	});

	it('Apply italic on text shape.', function() {
		desktopHelper.getCompactIcon('Italic').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-style', 'italic');
	});

	it('Apply underline on text shape.', function() {
		desktopHelper.getCompactIcon('Underline').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'text-decoration', 'underline');
	});

	it('Apply strikethrough on text shape.', function() {
		desktopHelper.getCompactIcon('Strikeout').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'text-decoration', 'line-through');
	});

	it('Apply font color on text shape.', function() {
		desktopHelper.getCompactIconArrow('FontColor').click();
		desktopHelper.getCompactIconArrow('Color').click();
		desktopHelper.selectColorFromPalette('FFFF00');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'fill', 'rgb(255,255,0)');
	});

	it('Apply highlight color on text shape.', function() {
		desktopHelper.getCompactIconArrow('FontColor').click();
		desktopHelper.getCompactIconArrow('CharBackColor').click();
		desktopHelper.selectColorFromPalette('FFBF00');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		//highlight color is not in the SVG
		// that's why we didn't test there
	});

	it('Apply a selected font name on the text shape', function() {
		cy.cGet('#fontnamecombobox .ui-combobox-button').click();
		desktopHelper.selectFromListbox('Liberation Mono');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-family', 'Liberation Mono');
	});

	it('Apply a selected font size on the text shape', function() {
		cy.cGet('#fontsizecombobox .ui-combobox-button').click();
		desktopHelper.selectFromListbox('22');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-size', '776px');
	});

	it.skip('Apply left/right alignment on text selected text.', function() {
		impressHelper.selectTextOfShape();
		cy.cGet('text tspan.TextPosition').should('have.attr', 'x', '1400');

		// Set right alignment first
		cy.cGet('#rightpara').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition').should('have.attr', 'x', '24530');

		// Set left alignment
		impressHelper.selectTextOfShape();
		cy.cGet('#leftpara').click();

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition').should('have.attr', 'x', '1400');
	});

	it.skip('Apply superscript on selected text.', function() {
		impressHelper.selectTextOfShape();

		cy.cGet('text tspan.TextPosition').should('have.attr', 'y', '8643');
		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-size', '1129px');

		helper.typeIntoDocument('{ctrl}{shift}p');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition').invoke('attr','y').then((y)=>+y).should('be.gt',8200);
		cy.cGet('text tspan.TextPosition').invoke('attr','y').then((y)=>+y).should('be.lt',8300);
		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-size', '655px');
	});

	it.skip('Apply subscript on selected text.', function() {
		impressHelper.selectTextOfShape();

		cy.cGet('text tspan.TextPosition').should('have.attr', 'y', '8643');
		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-size', '1129px');

		helper.typeIntoDocument('{ctrl}{shift}b');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('text tspan.TextPosition').invoke('attr','y').then((y)=>+y).should('be.gt',8700);
		cy.cGet('text tspan.TextPosition').invoke('attr','y').then((y)=>+y).should('be.lt',8750);
		cy.cGet('text tspan.TextPosition tspan').should('have.attr', 'font-size', '655px');
	});

	it('Click shape hyperlink.', function() {
		// Insert shape - this creates and selects the shape
		// immediately, no additional click needed to select it.
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIconArrow('BasicShapes').click();
		cy.cGet('.col.w2ui-icon.basicshapes_round-quadrat').click();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.processToIdle(this.win);

		helper.typeIntoDocument('{ctrl}k');
		cy.cGet('#target').should('exist').should('be.visible');
		cy.cGet('#indication').should('exist').should('not.be.visible');
		cy.cGet('#name').should('exist').should('not.be.visible');

		cy.cGet('#target-input').type('www.something.com');
		cy.cGet('#ok').click();
		cy.cGet('#target').should('not.exist');

		helper.processToIdle(this.win);

		impressHelper.removeShapeSelection();

		// Ctrl-click to open hyperlink pop-up
		impressHelper.clickCenterOfSlide( {ctrlKey: true} );
		helper.processToIdle(this.win);

		cy.cGet('[id^="info-modal-label2"]').should('have.text', 'http://www.something.com/');
		cy.cGet('#openlink-response').should('exist');
	});
});
