/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Chart wizard', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/chart_wizard.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.selectZoomLevel('70', false);
	});

	function insertTableAndSelect() {
		// Create a 2x2 table with some data and select all cells.
		cy.cGet('.notebookbar #Insert-tab-label').click();
		desktopHelper.getNbIconArrow('InsertTable', 'Insert').click();
		cy.cGet('.inserttable-grid > :nth-child(2) > :nth-child(2)').click();
		helper.typeIntoDocument('12{rightArrow}1{rightArrow}20{rightArrow}2');
		helper.typeIntoDocument('{shift}{upArrow}{shift}{leftArrow}');
	}

	function insertChartFromSelection() {
		// Insert a chart using the selection.
		cy.cGet('.notebookbar #Insert-tab-label').click();
		desktopHelper.getNbIconArrow('InsertGraphic', 'Insert').click();
		cy.cGet('#Insert-container .unoInsertObjectChart button').click();
		// Chart insert wizard must popup.
		cy.cGet('.jsdialog-container.lokdialog_container').should('exist');
		// The chart itself must be present in active mode and has selection.
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	}

	it('Open the chart wizard and press cancel', function() {

		insertTableAndSelect();
		insertChartFromSelection();

		// Click cancel on the wizard.
		cy.cGet('.jsdialog-container').contains('button.jsdialog.ui-pushbutton', 'Cancel').click();

		// Both the wizard and the chart must disappear.
		cy.cGet('.jsdialog-container.lokdialog_container').should('not.exist');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it('Open the chart wizard and press finish', function() {

		insertTableAndSelect();
		insertChartFromSelection();

		// Click cancel on the wizard.
		cy.cGet('.jsdialog-container').contains('button.jsdialog.ui-pushbutton', 'Finish').click();

		// The wizard should not exist.
		cy.cGet('.jsdialog-container.lokdialog_container').should('not.exist');
		// The chart with selection still must exist.
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	});

	it('Chart shows drag preview SVG after closing wizard', function() {

		insertTableAndSelect();
		insertChartFromSelection();

		// Close the wizard; the chart stays selected as a graphic object.
		cy.cGet('.jsdialog-container').contains('button.jsdialog.ui-pushbutton', 'Finish').click();
		cy.cGet('.jsdialog-container.lokdialog_container').should('not.exist');
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Selecting the chart triggers a rendershapeselection request whose
		// response is appended to #canvas-container as the semi-transparent
		// drag-preview overlay. Before the fix the isFrame guard skipped the
		// request for Writer charts and writer_svg_Export refused embedded
		// objects, so this overlay was never created.
		cy.cGet('#canvas-container > svg').should('exist');
		cy.cGet('#canvas-container > svg > svg').should('exist');

		// The internal private:stream render must not surface a user-facing
		// "Document cannot be exported" alert.
		cy.cGet('body').should('not.contain', 'Document cannot be exported');
	});
});
