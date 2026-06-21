/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

// In view mode the slide order must stay fixed. Dragging a slide in the
// slide sorter must not start a reorder, so the document cannot be altered
// and silently saved. See the regression where view mode still let the
// desktop slide-sorter drag reorder slides.
describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide sorter reorder in view mode', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function dispatchDragStart(win, frame) {
		var event = new win.DragEvent('dragstart', {
			bubbles: true,
			cancelable: true,
			dataTransfer: new win.DataTransfer(),
		});
		frame.dispatchEvent(event);
		return event;
	}

	it('allows starting a slide drag while editing', function() {
		// Sanity check: the guard must not break reordering for editors.
		cy.cGet('#preview-frame-part-1').then(function($frame) {
			var event = dispatchDragStart(this.win, $frame[0]);
			expect(event.defaultPrevented).to.equal(false);
		}.bind(this));
	});

	it('refuses to start a slide drag in view mode', function() {
		cy.then(function() {
			this.win.app.map.setPermission('readonly');
		}.bind(this));

		cy.cGet('#preview-frame-part-1').then(function($frame) {
			var event = dispatchDragStart(this.win, $frame[0]);
			// A cancelled dragstart means the browser never begins the drag,
			// so no reorder message can be sent.
			expect(event.defaultPrevented).to.equal(true);
		}.bind(this));
	});
});
