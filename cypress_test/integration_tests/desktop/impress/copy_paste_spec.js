/* global describe it cy require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress clipboard tests.', function() {

	it('Paste Special offers Markdown.', function() {
		// Given an Impress document with a text shape in the center:
		helper.setupAndLoadDocument('impress/top_toolbar.odp');
		impressHelper.removeShapeSelection();
		impressHelper.selectTextShapeInTheCenter();

		// When copying the text of that shape, ending text edit and pasting:
		impressHelper.selectTextOfShape();
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:Copy');
			helper.processToIdle(win);
		});
		helper.typeIntoDocument('{esc}');
		helper.typeIntoDocument('{esc}');
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:PasteSpecial');
		});

		// Then the paste special dialog should offer a "Markdown" item:
		cy.cGet('#PasteSpecialDialog').should('be.visible');
		// Without the accompanying fix in place, this test would have failed, the list had
		// no markdown item.
		cy.cGet('#PasteSpecialDialog .ui-treeview-cell-text:contains("Markdown")')
			.should('be.visible');
	});
});
