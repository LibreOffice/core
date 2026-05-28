/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Check mouse cursor type in various interactions', function() {

	/*
		NOTE: we need more of these tests for the elements where
		we change the cursor style:
			- the shape resize handles
			- the autofill marker
			- column/row headers
	*/
	it('Cell cursor over the sheet', function() {
		helper.setupAndLoadDocument('calc/mouse_cursor.ods');

		cy.cGet('#document-canvas').trigger('mouseover').then(() => {
			cy.cGet('#document-canvas').should('have.class', 'spreadsheet-cursor');
		});
	});

	it('Text cursor over the cell being edited', function() {
		// A document with no drawing object over the sheet, so core reports a
		// text pointer over the edited cell rather than a hand pointer for an
		// overlapping shape.
		helper.setupAndLoadDocument('calc/mouse_cursor_editing.fods');

		// Enter edit mode; the blinking text cursor appears inside the cell.
		helper.typeIntoDocument('abc');
		helper.getBlinkingCursorPosition('cursorPos');

		cy.cGet('#document-canvas').then(function($canvas) {
			var cr = $canvas[0].getBoundingClientRect();
			// A point well inside the sheet body but away from the edited cell
			// (top-left). Kept clear of the canvas edges so the scrollbar section
			// does not intercept the move instead of the mouse-control section.
			var awayX = Math.round(cr.left + cr.width * 0.5);
			var awayY = Math.round(cr.top + cr.height * 0.6);

			cy.get('@cursorPos').then(function(point) {
				// Over the edited cell -> text cursor, no spreadsheet-cursor class.
				cy.cGet('body').realMouseMove(point.x, point.y);
				cy.cGet('#document-canvas')
					.should('not.have.class', 'spreadsheet-cursor')
					.and('have.css', 'cursor', 'text');

				// Away from the edited cell -> back to the cell cursor.
				cy.cGet('body').realMouseMove(awayX, awayY);
				cy.cGet('#document-canvas')
					.should('have.class', 'spreadsheet-cursor')
					.and('have.css', 'cursor', 'cell');
			});
		});
	});

	it('Mouse over the clipboard-container does not trigger onMouseLeave on the canvas', function() {
		helper.setupAndLoadDocument('calc/mouse_cursor.ods');

		// Start with the canvas in the "spreadsheet-cursor" state.
		cy.cGet('#document-canvas').trigger('mouseover');
		cy.cGet('#document-canvas').should('have.class', 'spreadsheet-cursor');

		cy.cGet('#document-canvas').then(function($canvas) {
			var cr = $canvas[0].getBoundingClientRect();
			var clipboardX = Math.round(cr.left + 50);
			var clipboardY = Math.round(cr.top + 50);
			var awayX = Math.round(cr.left + 75);
			var awayY = Math.round(cr.top + 75);

			cy.getFrameWindow().then(function(win) {
				var mouseControl = win.app.sectionContainer.getSectionWithName('mouse-control');
				cy.spy(mouseControl, 'onMouseLeave').as('onMouseLeaveSpy');

				// Place the clipboard-container over a known point on the canvas.
				var container = win.document.getElementById('doc-clipboard-container');
				container.style.transform = 'translate(' + clipboardX + 'px, ' + clipboardY + 'px)';
			});

			// Ensure clipboard-container doesn't intercept the mouse events.
			// Otherwise it would catch the mousemove and change the canvas cursor.
			cy.cGet('body').realMouseMove(awayX, awayY);
			cy.cGet('body').realMouseMove(clipboardX, clipboardY);
		});

		cy.get('@onMouseLeaveSpy').should('not.have.been.called');
	});
});
