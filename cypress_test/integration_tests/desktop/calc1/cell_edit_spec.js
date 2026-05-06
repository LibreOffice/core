/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test rendering of a cell on edit', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_edit.fods');
		desktopHelper.selectZoomLevel(200); // make differences more significant

		cy.viewport(800, helper.maxScreenshotableViewportHeight);
		cy.window().then(win => { win.dispatchEvent(new Event('resize')); });
		cy.getFrameWindow().then((win) => {
			this.win = win;
			// wait for zoom rendering and viewport resize to complete
			helper.processToIdle(win);
		});
	});

	function selectInitialCell(win) {
		calcHelper.enterCellAddressAndConfirm(win, 'CA980');
	}

	function checkTextContent(expected) {
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-cursor-layer')
			.should('have.text', expected);
	}

	function checkVisualContent(win, expected) {
		helper.processToIdle(win);
		cy.cGet('#document-container').compareSnapshot(expected, 0.02);
	}

	it('Redraw after undo', function() {
		// setup initial state
		desktopHelper.assertScrollbarPosition('horizontal', 325, 355);
		desktopHelper.assertScrollbarPosition('vertical', 235, 300);

		selectInitialCell(this.win);
		checkTextContent('');
		checkVisualContent(this.win, 'empty');

		// type something
		const testString = 'TEST STRING';

		desktopHelper.getNbIconArrow('Grow').click();
		desktopHelper.getNbIcon('Bold').click();
		cy.cGet('.jsdialog-overlay').click();
		desktopHelper.getNbIconArrow('Grow').click();
		desktopHelper.getNbIcon('Underline').click();
		cy.cGet('.jsdialog-overlay').click();

		helper.typeIntoDocument(testString + '{enter}');
		checkTextContent('');

		// verify cell content
		selectInitialCell(this.win);
		checkTextContent(testString);
		checkVisualContent(this.win, 'teststring');

		// undo
		desktopHelper.getNbIcon('Undo').click();

		// verify cell content
		selectInitialCell(this.win);
		checkTextContent('');
		checkVisualContent(this.win, 'empty_selected');
	});
});
