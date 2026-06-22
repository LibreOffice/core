/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Notebookbar contextual tabs (Calc).', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Shape tab stays available while editing shape text', function() {
		// Insert a shape; it is selected, so the Shape tab is offered.
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		cy.cGet('#Shape-tab-label').should('be.visible');

		// Enter text edit inside the shape. The Shape tab must stay offered so
		// the shape can still be formatted while its text is edited.
		helper.getShapeSVGCenter().then(function(pos) {
			cy.cGet('#document-canvas').dblclick(pos.x, pos.y, { force: true });
		});
		cy.cGet('.leaflet-cursor-container, .text-selection-handle-start, .leaflet-cursor.blinking-cursor')
			.should('exist');
		cy.cGet('#Shape-tab-label').should('be.visible');
	});
});
