/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Change bar click', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/track_changes.odt');
		cy.getFrameWindow().then(function (win) {
			this.win = win;
		}.bind(this));
	});

	it('Clicking a change bar shows the change in the Manage Changes dialog', function () {
		// Given a paragraph typed while changes are recorded, so it has a change bar in the left
		// margin, and the cursor at the start of that paragraph:
		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:TrackChanges');
		});
		helper.waitForMapState('.uno:TrackChanges', 'true');
		helper.typeIntoDocument('Hello World');
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);
		cy.getFrameWindow().its('app.file.textCursor.rectangle.x1').should('be.greaterThan', 0);

		// When clicking the change bar, which is a quarter of a centimetre left of the text:
		var CHANGE_BAR_DISTANCE_TWIPS = 567 / 4;
		cy.getFrameWindow().then(function (win) {
			var cursor = win.app.file.textCursor.rectangle;
			var layout = win.app.activeDocument.activeLayout;
			var changeBar = new win.cool.SimplePoint(cursor.x1 - CHANGE_BAR_DISTANCE_TWIPS,
				cursor.y1 + cursor.height / 2);
			var x = layout.documentToViewX(changeBar) / win.app.dpiScale;
			var y = layout.documentToViewY(changeBar) / win.app.dpiScale;
			cy.cGet('#document-canvas').click(x, y);
		});
		helper.processToIdle(this.win);

		// Then the change is selected in the document and in the list of the Manage Changes dialog:
		cy.cGet('.jsdialog-window #writerchanges').should('be.visible');
		cy.cGet('.jsdialog-window #writerchanges [aria-selected="true"]')
			.should('have.length', 1);
		helper.textSelectionShouldExist();
	});
});
