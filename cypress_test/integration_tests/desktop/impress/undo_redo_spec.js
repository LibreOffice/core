/* global describe it beforeEach require cy */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');
var repairHelper = require('../../common/repair_document_helper');

describe(['tagdesktop'], 'Editing Operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/undo_redo.odp', false, false, undefined, 'vector=false');
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.selectZoomLevel('30', false);

		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});

		impressHelper.selectTextShapeInTheCenter();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		impressHelper.dblclickOnSelectedShape();
	});

	// Type text one character at a time with processToIdle
	// between each keystroke.  This avoids text corruption
	// caused by accessibility echo-back racing with rapid
	// typing under load.
	function typeTextSafely(win, text) {
		for (var i = 0; i < text.length; i++) {
			helper.typeIntoDocument(text[i]);
			helper.processToIdle(win);
		}
	}

	// Exit text editing to commit the change, refresh the
	// shape SVG and verify it contains the expected text.
	function expectShapeText(text) {
		impressHelper.triggerNewSVGForShapeInTheCenter();
		cy.cGet('#document-container svg g.Page g')
			.should('contain.text', text);
	}

	// Refresh the shape SVG and verify it does not contain
	// the given text (the shape initially has an empty paragraph).
	function expectShapeTextAbsent(text) {
		impressHelper.triggerNewSVGForShapeInTheCenter();
		cy.cGet('#document-container svg g.Page g').invoke('text')
			.should('not.include', text);
	}

	function undo(win) {
		helper.processToIdle(win);
		typeTextSafely(win, 'Hello World');
		expectShapeText('Hello World');
		helper.typeIntoDocument('{ctrl+z}');
		helper.processToIdle(win);
		expectShapeTextAbsent('Hello World');
	}

	it('Undo', function() {
		undo(this.win);
	});

	it('Redo', function() {
		undo(this.win);
		helper.typeIntoDocument('{ctrl+y}');
		helper.processToIdle(this.win);
		expectShapeText('Hello World');
	});

	it('Repair Document', function() {
		helper.processToIdle(this.win);
		typeTextSafely(this.win, 'Hello World');
		impressHelper.triggerNewSVGForShapeInTheCenter();
		impressHelper.dblclickOnSelectedShape();
		helper.processToIdle(this.win);
		typeTextSafely(this.win, 'Overwrite Text');
		impressHelper.triggerNewSVGForShapeInTheCenter();
		repairHelper.rollbackPastChange('Undo', false, true);
		expectShapeText('Hello World');
	});
});
