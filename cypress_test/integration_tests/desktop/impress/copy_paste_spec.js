/* global describe it cy require Blob */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress clipboard tests.', function() {

	it('Notebookbar Paste routes to the notes pane', function() {
		// Given an open notes panel with its editengine widget focused:
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('notespanel');
		});
		cy.cGet('#notespanel-dock-wrapper').should('be.visible');
		cy.cGet('#notespanel-container .ui-editengine').should('exist');
		cy.cGet('#notespanel-container .ui-editengine').click();

		// When simulating a notebookbar Paste click with a fake async clipboard
		// carrying external text:
		cy.getFrameWindow().then(function(win) {
			const editable = win.document.querySelector(
				'#notespanel-container .ui-editengine'
			);
			editable.focus();

			const clip = win.app.map._clip;
			const clipboardItem = {
				types: ['text/plain'],
				getType: function(type) {
					return {
						then: function(resolve, reject) {
							if (type === 'text/plain') {
								resolve(new Blob(['external text']));
							} else {
								reject({ message: 'no ' + type });
							}
						},
					};
				},
			};
			clip._dummyClipboard = {
				read: function() {
					return {
						then: function(resolve) {
							resolve([clipboardItem]);
						},
					};
				},
			};

			clip.filterExecCopyPaste('.uno:Paste');
		});

		// Then the pasted text lands in the notes pane, not on the slide:
		// Without the accompanying fix in place, this test would have failed with:
		// assert expected <div.ui-editengine-paragraph> to contain text external text, but the text was Click to add Notes
		// i.e. the pasted content didn't end up in the notes panel.
		cy.cGet('#notespanel-container .ui-editengine-paragraph').should(
			'contain.text',
			'external text'
		);
	});

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
