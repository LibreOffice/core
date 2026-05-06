/* global describe expect it cy Cypress before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Calc Sidebar Tests', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('calc/help_dialog.ods');

		// to make insertImage use the correct buttons
		desktopHelper.switchUIToNotebookbar();

		// Hide sidebar before enabling UICoverage tracking
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
			expect(result.CompleteCalcSidebarCoverage, `complete calc sidebar coverage`).to.be.true;
		});
	});

	afterEach(function () {
		a11yHelper.resetState();
	});

	it('PropertyDeck: Default Context', function () {
		helper.processToIdle(win);
		runA11yValidation(win);
	});

	it('PropertyDeck: Graphic Context', function () {
		cy.viewport(1920, 1080);

		desktopHelper.insertImage();

		helper.processToIdle(win);
		runA11yValidation(win);

		// Deselect image and restore viewport/zoom
		helper.typeIntoDocument('{esc}');
		cy.viewport(Cypress.config('viewportWidth'), Cypress.config('viewportHeight'));
		desktopHelper.selectZoomLevel('100', false);
	});

	it('Chart panels', function () {

		// Switch to Sheet2 which has data and select a range
		cy.cGet('#spreadsheet-tab2').click();
		helper.processToIdle(win);
		helper.typeIntoInputField(helper.addressInputSelector, 'A1:C4');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectChart');
		});

		// Calc shows a chart wizard dialog, dismiss it to insert the chart
		cy.cGet('#CHART2_HID_SCH_WIZARD_ROADMAP').should('exist');
		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();
		cy.cGet('#CHART2_HID_SCH_WIZARD_ROADMAP').should('not.exist');

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Chart is already in edit mode after wizard dismissal
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
	});

	it('Math (PosSizePropertyPanel)', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectStarMath');
		});

		// Math edit mode shows the Elements panel in the sidebar
		helper.processToIdle(win);
		runA11yValidation(win);

		// Exit the math edit mode, which gives the PosSizePropertyPanel sidebar
		helper.typeIntoDocument('{esc}');

		helper.processToIdle(win);
		runA11yValidation(win);

		// Deselect the math object
		helper.typeIntoDocument('{esc}');
	});

	it('PropertyDeck: Shape Text Context', function () {
		// Deselect object from previous test
		helper.typeIntoDocument('{esc}');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Enter text editing mode inside the shape
		helper.typeIntoDocument('{enter}');
		helper.processToIdle(win);
		runA11yValidation(win);

		// Exit text editing and deselect shape
		helper.typeIntoDocument('{esc}');
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

	it('PropertyDeck: Fontwork Context', function () {
		// Deselect line from previous test
		helper.typeIntoDocument('{esc}');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FontworkGalleryFloater');
		});

		cy.cGet('.ui-dialog[role="dialog"]');
		cy.cGet('#ok-button').click();
		helper.processToIdle(win);
		runA11yValidation(win);
	});

	it('ScFunctionsDeck', function () {
		// Deselect fontwork from previous test
		helper.typeIntoDocument('{esc}');

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.ScFunctionsDeck');
			helper.processToIdle(win);
		});

		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('StyleListDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.StyleListDeck');
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
