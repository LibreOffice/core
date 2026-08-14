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

	function clickConvertToCurveButton() {
		const button = '#Shape-container .unoChangeBezier';
		cy.cGet(button).should('not.have.attr', 'disabled');
		cy.cGet(button).click();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
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
		clickConvertToCurveButton();

		selectionKind().should('equal', 'pathFill');
		drawnHandle().should('exist');

		helper.typeIntoDocument('{esc}');
		drawnHandle().should('not.exist');

		selectObject(1);
		drawnHandle().should('exist');
	});
});
