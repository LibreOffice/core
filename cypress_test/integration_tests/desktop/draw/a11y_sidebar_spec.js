/* global describe expect it cy before after afterEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Draw Sidebar Tests', { testIsolation: false }, function () {
	let win;

	function getWin() {
		return win;
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/insert_position.fodg');

		// to make insertImage use the correct buttons
		desktopHelper.switchUIToNotebookbar();

		// Hide the deck first, so opening it again records its panels.
		desktopHelper.ensureSidebarHidden();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
			a11yHelper.enableUICoverage(win);
		});

		a11yHelper.openSidebarPropertyDeck(getWin);

		cy.cGet('.jsdialog-window').should('not.exist');
	});

	after(function () {
		a11yHelper.reportUICoverage(win);

		// Draw has no sidebar .ui of its own, and eight common panels have no
		// trigger here, so only assert that tracking ran.
		cy.get('@uicoverageResult').then(result => {
			expect(result.used, 'used .ui files').to.not.be.empty;
		});
	});

	afterEach(function () {
		a11yHelper.resetState();
	});

	function runA11yValidation() {
		a11yHelper.runA11yValidation(win, 'validatesidebara11y');
	}

	it('PropertyDeck: Default Context', function () {
		helper.processToIdle(win);
		runA11yValidation();
	});

	it('PropertyDeck: Graphic Context', function () {
		desktopHelper.insertImage();

		helper.processToIdle(win);
		runA11yValidation();

		helper.typeIntoDocument('{esc}');
		desktopHelper.selectZoomLevel('100', false);
	});

	it('PropertyDeck: Shape Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation();

		// Text editing inside the shape brings the text panels up
		helper.typeIntoDocument('{enter}');
		helper.processToIdle(win);
		runA11yValidation();

		helper.typeIntoDocument('{esc}');
		helper.typeIntoDocument('{esc}');
	});

	it('PropertyDeck: Fontwork Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:FontworkGalleryFloater');
		});

		cy.cGet('.ui-dialog[role="dialog"]');
		cy.cGet('#ok-button').click();
		helper.processToIdle(win);
		runA11yValidation();

		helper.typeIntoDocument('{esc}');
	});

	it('Chart Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:InsertObjectChart');
		});

		helper.processToIdle(win);
		runA11yValidation();

		helper.typeIntoDocument('{esc}');
		helper.typeIntoDocument('{esc}');
	});

	it('ShapesDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.ShapesDeck');
		});

		// The deck switch reaches the browser asynchronously. Validate only once
		// the deck content is up, otherwise the check runs against the old deck.
		cy.cGet('#DefaultShapesPanel').should('be.visible');
		cy.then(() => {
			return helper.processToIdle(win);
		});

		runA11yValidation();

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
		});
		cy.cGet('#PropertyDeck').should('be.visible');
		cy.then(() => {
			return helper.processToIdle(win);
		});
	});

	it('StyleListDeck', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.StyleListDeck');
		});

		// The deck switch reaches the browser asynchronously. Validate only once
		// the deck content is up, otherwise the check runs against the old deck.
		cy.cGet('#StyleListDeck').should('be.visible');
		cy.then(() => {
			return helper.processToIdle(win);
		});

		runA11yValidation();

		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
		});
		cy.cGet('#PropertyDeck').should('be.visible');
		cy.then(() => {
			return helper.processToIdle(win);
		});
	});

	it('PropertyDeck: Line Context', function () {
		cy.then(() => {
			win.app.map.sendUnoCommand('.uno:Line');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		helper.processToIdle(win);
		runA11yValidation();

		helper.typeIntoDocument('{esc}');
	});
});
