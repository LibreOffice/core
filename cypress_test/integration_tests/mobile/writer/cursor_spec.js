/* global describe it require cy */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Text cursor tests.', function() {

	function before(filePath) {
		helper.setupAndLoadDocument(filePath, false, false, undefined, 'vector=false');

		mobileHelper.enableEditingMobile();
	}

	it('Extensive cursor movements.', function() {
		before('writer/cursor.odt');

		for (var i = 0; i < 5; i++) {
			helper.moveCursor('right');
		}
		for (var j = 0; j < 5; j++) {
			helper.moveCursor('left');
		}
	});

	it('View jumps by cursor movement.', function() {
		before('writer/cursor.odt');

		for (var i = 0; i < 5; i++) {
			helper.moveCursor('end');
			helper.moveCursor('home');
		}
	});

	it('Cursor is visible after text selection.', function() {
		before('writer/cursor.odt');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('.blinking-cursor').should('be.visible');
		// Blinking cursor and so the view should be at the end of the text selection.
		cy.cGet('.text-selection-handle-end').should('be.visible');
		cy.cGet('.text-selection-handle-start').should('not.be.visible');
	});

	it('Move cursor through table.', function() {
		before('writer/cursor_in_table.odt');

		for (var i = 0; i < 5; i++) {
			helper.moveCursor('down');
		}

		for (var j = 0; j < 5; j++) {
			helper.moveCursor('up');
		}
	});
});
