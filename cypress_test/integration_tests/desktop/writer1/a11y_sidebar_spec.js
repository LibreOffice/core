/* global describe expect it cy before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Writer Sidebar Tests', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');

		// to make insertImage use the correct buttons
		desktopHelper.switchUIToNotebookbar();

		// Hide sidebar before enabling UICoverage tracking
		cy.cGet('#sidebar-dock-wrapper').should('be.visible').should('not.be.empty');
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
			a11yHelper.enableUICoverage(win);
		});

		// Show sidebar again after UICoverage tracking is enabled
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		cy.cGet('.jsdialog-window').should('not.exist');
	});

	after(function () {
		a11yHelper.reportUICoverage(win);

		cy.get('@uicoverageResult').then(result => {
			expect(result.used, `used .ui files`).to.not.be.empty;
			expect(result.CompleteWriterSidebarCoverage, `complete writer sidebar coverage`).to.be.true;
		});
	});

	afterEach(function () {
		a11yHelper.resetState();
	});

	it('PropertyDeck: Table Context', function () {
		helper.processToIdle(win);
		runA11yValidation(win);
	});

	it('PropertyDeck: Graphic Context:', function () {
		helper.clearAllText({ isTable: true });
		desktopHelper.insertImage();

		helper.processToIdle(win);
		runA11yValidation(win);
	});

	it('Chart (LinePropertyPanel)', function () {
		helper.clearAllText({ isTable: true });
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectChart');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Enter chart edit mode
		cy.realPress('Enter');
		helper.processToIdle(win);

		// Default chart deck panels
		runA11yValidation(win);

		// Enter the chart's inner object hierarchy
		cy.realPress('Enter');
		helper.processToIdle(win);

		// Select the first sub-object
		cy.realPress('Tab');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Go a level down the hierarchy, Data Series: Column 1
		cy.realPress('Enter');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Go a level down the hierarchy, Data point 1 in data series 1
		cy.realPress('Enter');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Back up to Data Series: Column 1
		cy.realPress('Escape');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Data Series: Column 2
		cy.realPress('Tab');
		helper.processToIdle(win);
		runA11yValidation(win);

		// X Axis
		cy.realPress('Tab');
		cy.realPress('Tab');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Esc out of chart navigation, chart edit mode, and chart selection
		escLevel(win, 3);
		helper.processToIdle(win);

		// At which point the sidebar disappears
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		// esc to get back to main document
		escLevel(win, 1);
		helper.processToIdle(win);

		// sidebar starts again, and grabs focus to itself
		desktopHelper.assertSidebarStealsFocus(win);

		// esc to send focus back to main document
		escLevel(win, 1);
		helper.processToIdle(win);

		// focus stays here after that
		cy.cGet('div.clipboard').should('have.focus');
	});

	it('Math (PosSizePropertyPanel)', function () {
		helper.clearAllText({ isTable: true });
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectStarMath');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation(win);

		// Exit the math edit code, which conveniently for testing gives
		// the 'empty' placeholder sidebar
		helper.typeIntoDocument('{esc}');

		helper.processToIdle(win);
		runA11yValidation(win);

		// Deselect the math object
		helper.typeIntoDocument('{esc}');
	});

	it('PropertyDeck: Line Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:Line');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation(win);
	});

	it('PropertyDeck: Fontwork Context:', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FontworkGalleryFloater');
		});

		cy.cGet('.ui-dialog[role="dialog"]');
		cy.cGet('#ok-button').click();
		helper.processToIdle(win);
		// and the sidebar we're interested in should appear
		runA11yValidation(win);
	});

	it('A11yCheckDeck', function() {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.A11yCheckDeck');
			helper.processToIdle(win);
		});

		cy.cGet('#updateLinkButton').click();

		helper.processToIdle(win);
		runA11yValidation(win);

		// Verify expander heading hierarchy: depth 0 -> h2, depth 1 -> h3, depth 2 -> h4
		cy.cGet('#A11yCheckDeck [data-expander-depth="0"] > .ui-expander > h2.ui-expander-heading').should('exist');
		cy.cGet('#A11yCheckDeck [data-expander-depth="1"] > .ui-expander > h3.ui-expander-heading').should('exist');
		cy.cGet('#A11yCheckDeck [data-expander-depth="2"] > .ui-expander > h4.ui-expander-heading').should('exist');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('WriterPageDeck', function() {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.WriterPageDeck');
			helper.processToIdle(win);
		});

		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('StyleListDeck', function() {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.StyleListDeck');
			helper.processToIdle(win);
		});

		// Maybe we should click on every style-type toolbar button.
		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('InspectorDeck', function() {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.InspectorDeck');
			helper.processToIdle(win);
		});

		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	function runA11yValidation(win) {
		a11yHelper.runA11yValidation(win, 'validatesidebara11y');
	}

	function escLevel(win, count) {
		for (var i = 0; i < count; i++) {
			helper.typeIntoDocument('{esc}');
			helper.processToIdle(win);
		}
	}

});
