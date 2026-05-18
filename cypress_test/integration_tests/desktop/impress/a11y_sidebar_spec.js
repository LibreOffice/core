/* global describe expect it cy before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Impress Sidebar Tests', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('impress/help_dialog.odp');

		// to make insertImage use the correct buttons
		desktopHelper.switchUIToNotebookbar();

		// Hide sidebar before enabling UICoverage tracking. The impress
		// PropertyDeck button is .unoModifyPage (the deck is mapped to the
		// .uno:ModifyPage command in sfx2's SidebarController), not
		// SidebarDeck.PropertyDeck as in writer/calc.
		desktopHelper.hideSidebarImpress();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
			a11yHelper.enableUICoverage(win);
		});

		// Show sidebar again after UICoverage tracking is enabled
		cy.cGet('.unoModifyPage button').click();
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		cy.cGet('.jsdialog-window').should('not.exist');
	});

	after(function () {
		a11yHelper.reportUICoverage(win);

		cy.get('@uicoverageResult').then(result => {
			expect(result.used, `used .ui files`).to.not.be.empty;
			expect(result.CompleteImpressSidebarCoverage, `complete impress sidebar coverage`).to.be.true;
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
		desktopHelper.insertImage();

		helper.processToIdle(win);
		runA11yValidation(win);

		// Deselect image and restore zoom changed by insertImage
		helper.typeIntoDocument('{esc}');
		desktopHelper.selectZoomLevel('100', false);
	});

	it('PropertyDeck: Shape Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation(win);

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

		helper.typeIntoDocument('{esc}');
	});

	it('PropertyDeck: Fontwork Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FontworkGalleryFloater');
		});

		cy.cGet('.ui-dialog[role="dialog"]');
		cy.cGet('#ok-button').click();
		helper.processToIdle(win);
		runA11yValidation(win);

		helper.typeIntoDocument('{esc}');
	});

	it('Math (PosSizePropertyPanel)', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectStarMath');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation(win);

		// Exit math edit mode, which gives the PosSizePropertyPanel sidebar
		helper.typeIntoDocument('{esc}');

		helper.processToIdle(win);
		runA11yValidation(win);

		// Deselect math object
		helper.typeIntoDocument('{esc}');
	});

	it('SdSlideTransitionDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.SdSlideTransitionDeck');
			helper.processToIdle(win);
		});

		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('SdCustomAnimationDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.SdCustomAnimationDeck');
			helper.processToIdle(win);
		});

		runA11yValidation(win);

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
			helper.processToIdle(win);
		});
	});

	it('SdMasterPagesDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.SdMasterPagesDeck');
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
});
