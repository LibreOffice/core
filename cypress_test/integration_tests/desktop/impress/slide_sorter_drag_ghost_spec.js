/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

// Dragging slides in the slide sorter shows a ghost under the pointer:
// the grabbed slide's picture on top, the other dragged slides stacked
// behind it, and a badge counting them, so a multi-slide drag reads as
// more than one slide.
describe(['tagdesktop'], 'Slide sorter drag ghost.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	// Starts a reorder drag on the given slide frame and reports the
	// ghost's picture count and badge text. The ghost only exists while
	// the dragstart handlers run, so it is captured from a listener
	// behind the sorter's own.
	function captureDragGhost(win, frame) {
		var ghost = {};
		win.document.addEventListener('dragstart', function() {
			var el = win.document.querySelector('.slide-drag-ghost');
			ghost.pictures = el ? el.querySelectorAll('img').length : 0;
			var badge = el && el.querySelector('.slide-drag-ghost-count');
			ghost.badge = badge ? badge.textContent : null;
		}, {once: true});
		var event = new win.DragEvent('dragstart', {
			bubbles: true,
			cancelable: true,
			dataTransfer: new win.DataTransfer(),
		});
		frame.dispatchEvent(event);
		return ghost;
	}

	it('shows one picture and no badge for a single-slide drag', function() {
		cy.cGet('#preview-frame-part-1').then(function($frame) {
			var ghost = captureDragGhost(this.win, $frame[0]);
			expect(ghost.pictures, 'ghost pictures').to.equal(1);
			expect(ghost.badge, 'ghost badge').to.equal(null);
		}.bind(this));
	});

	it('stacks the pictures and counts the slides of a multi-slide drag', function() {
		// Select two slides, then grab one of them. The clicked slide
		// becomes the current part and the ctrl-clicked one joins the
		// selection.
		cy.cGet('#preview-img-part-1').click();
		cy.cGet('#preview-img-part-2').click({ctrlKey: true});
		cy.cGet('#preview-img-part-2')
			.should('have.class', 'preview-img-selectedpart');

		cy.cGet('#preview-frame-part-2').then(function($frame) {
			var ghost = captureDragGhost(this.win, $frame[0]);
			expect(ghost.pictures, 'ghost pictures').to.equal(2);
			expect(ghost.badge, 'ghost badge').to.equal('2');
		}.bind(this));
	});
});
