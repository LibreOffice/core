/* global describe it cy beforeEach require expect */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Convert to curve', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/to_curve.fodg');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920, 1080);
	});

	function selectObject(position) {
		helper.typeIntoDocument('{esc}');
		for (let i = 0; i < position; i++)
			cy.realPress('Tab');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
		cy.getFrameWindow().its('app.definitions.graphicSelection.rectangle')
			.should('not.be.null');
	}

	function selectionKind() {
		return cy.getFrameWindow()
			.its('app.definitions.graphicSelection.extraInfo.typeString');
	}

	function selectionRectangle() {
		return cy.getFrameWindow()
			.its('app.definitions.graphicSelection.rectangle');
	}

	function drawnHandle() {
		return cy.cGet('#test-div-shape-handle-rotation');
	}

	function convertToCurve() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:ChangeBezier');
			helper.processToIdle(win);
		});
	}

	function openShapeTab() {
		cy.cGet('#Shape-tab-label').should('be.visible').then(function($tab) {
			if (!$tab.hasClass('selected'))
				cy.wrap($tab).click();
		});
		cy.cGet('#Shape-container').should('be.visible');
	}

	function clickConvertToPolygonButton() {
		const button = '#Shape-container .unoConvertToEditableShape';
		cy.cGet(button).should('not.have.attr', 'disabled');
		// The button holds the polygon command, its arrow opens the other conversions.
		cy.cGet(button + ' .unobutton').click();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	}

	const convertMenu = '[id^="shape-convert-editable"].modalpopup';

	function openConvertMenu() {
		const button = '#Shape-container .unoConvertToEditableShape';
		cy.cGet(button).should('not.have.attr', 'disabled');
		cy.cGet(button + ' > .arrowbackground').click();
		cy.cGet(convertMenu).should('be.visible');
	}

	it('Keeps the shape selected after converting it to a curve', function() {
		selectObject(1);
		selectionKind().should('equal', 'rectangle');

		convertToCurve();

		selectionRectangle().should('not.be.null');
		selectionKind().should('equal', 'pathFill');
	});

	it('A converted line can still be selected afterwards', function() {
		selectObject(2);
		selectionKind().should('equal', 'line');

		convertToCurve();
		selectionKind().should('equal', 'pathLine');

		helper.typeIntoDocument('{esc}');
		selectionRectangle().should('be.null');

		selectObject(2);

		cy.getFrameWindow().then(function(win) {
			const selection = win.app.definitions.graphicSelection;
			expect(selection.extraInfo.typeString, 'kind').to.equal('pathLine');
			expect(selection.rectangle.cWidth, 'selection width').to.be.greaterThan(0);
			expect(selection.rectangle.cHeight, 'selection height').to.be.greaterThan(0);
		});
	});

	it('Keeps the drawn handles after the Shape tab button converts the shape', function() {
		selectObject(1);
		drawnHandle().should('exist');

		openShapeTab();
		clickConvertToPolygonButton();

		// The edges of a rectangle are straight, so it becomes a polygon with no curve.
		selectionKind().should('equal', 'polygon');
		drawnHandle().should('exist');

		helper.typeIntoDocument('{esc}');
		drawnHandle().should('not.exist');

		selectObject(1);
		drawnHandle().should('exist');
	});

	it('The arrow of the polygon button offers the other conversions', function() {
		selectObject(1);
		selectionKind().should('equal', 'rectangle');

		openShapeTab();
		openConvertMenu();

		cy.cGet(convertMenu + ' .ui-combobox-entry').should(function($entries) {
			const texts = $entries.toArray().map(function(entry) {
				return entry.textContent.trim();
			});
			expect(texts).to.deep.equal(['To Curve', 'To Contour', 'To Polygon']);
		});

		// The rectangle is no curve yet, so it may be changed to one.
		cy.cGet(convertMenu).contains('.ui-combobox-entry', 'To Curve')
			.should('not.have.class', 'disabled')
			.click();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.cGet(convertMenu).should('not.exist');
		// The curve keeps its straight edges as curves, unlike the polygon of the button itself.
		selectionKind().should('equal', 'pathFill');
	});
});
