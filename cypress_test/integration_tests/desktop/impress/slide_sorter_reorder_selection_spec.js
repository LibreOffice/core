/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

// Reordering slides in the maximized slide grid moves exactly the slides
// the grid shows as selected, also when an earlier drag moved several.
describe(['tagdesktop'], 'Slide grid reorder selection.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	function partHashes(win) {
		return win.app.impress.partList.map(function(entry) { return entry.part; });
	}

	// The slide frame at the given position in the sorter. The frame ids
	// stay with a slide when it moves, so frames are looked up by position.
	function frameAt(win, index) {
		return win.app.map._docLayer._preview._previewTiles[index].parentNode;
	}

	function fireDragEvent(win, target, type, x, y) {
		target.dispatchEvent(new win.DragEvent(type, {
			bubbles: true,
			cancelable: true,
			clientX: x,
			clientY: y,
			dataTransfer: new win.DataTransfer(),
		}));
	}

	// Drags the slide at the given position and drops it in front of the
	// first slide (toEnd false) or after the last one (toEnd true). The
	// dragged frames leave the grid once the drag has started, so the drop
	// point is measured in a later step.
	function dragSlide(win, index, toEnd) {
		cy.then(function() {
			fireDragEvent(win, frameAt(win, index), 'dragstart', 0, 0);
		});
		helper.processToIdle(win);
		cy.then(function() {
			var preview = win.app.map._docLayer._preview;
			var dragged = preview._dragState.draggedParts;
			var target = toEnd ? preview._previewTiles.length - 1 : 0;
			var step = toEnd ? -1 : 1;
			while (dragged.indexOf(target) !== -1)
				target += step;
			var rect = frameAt(win, target).getBoundingClientRect();
			var x = toEnd ? rect.right - 2 : rect.left + 2;
			var y = rect.top + rect.height / 2;
			var sorter = win.document.getElementById('slide-sorter');
			fireDragEvent(win, sorter, 'dragover', x, y);
			fireDragEvent(win, sorter, 'drop', x, y);
		});
		helper.processToIdle(win);
	}

	it('moves only the slide that is selected after a multi-slide move', function() {
		cy.cGet('.navigation-expand-button').click();
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._preview._gridMode).to.equal(true);
			expect(win.app.impress.partList).to.have.length(14);
		});

		// Select the third and the fourth slide.
		cy.cGet('#preview-img-part-2').click();
		helper.processToIdle(this.win);
		helper.waitForTimers(this.win, 'clicktimer');
		cy.cGet('#preview-img-part-3').click({ctrlKey: true});
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.impress.getSelectedSlidesCount()).to.equal(2);
		});

		var originalOrder;
		cy.getFrameWindow().then(function(win) {
			originalOrder = partHashes(win);
		});

		// Both selected slides move to the front.
		dragSlide(this.win, 3, false);
		cy.getFrameWindow().should(function(win) {
			var order = partHashes(win);
			expect(order.slice(0, 2)).to.deep.equal(originalOrder.slice(2, 4));
			expect(order.slice(2)).to.deep.equal(
				originalOrder.slice(0, 2).concat(originalOrder.slice(4)));
		});

		// Record the slides the grid marks as selected. A drag of one of them
		// takes them all along.
		var orderBefore;
		var selectedBefore;
		cy.getFrameWindow().then(function(win) {
			orderBefore = partHashes(win);
			selectedBefore = [];
			for (var i = 0; i < orderBefore.length; i++) {
				if (win.app.impress.isSlideSelected(i) ||
				    i === win.app.map._docLayer._selectedPart)
					selectedBefore.push(orderBefore[i]);
			}
			expect(selectedBefore).to.have.length.greaterThan(0);
		});

		// Move the first marked slide to the end.
		cy.getFrameWindow().then(function(win) {
			dragSlide(win, orderBefore.indexOf(selectedBefore[0]), true);
		});

		// Exactly the marked slides moved, in their order, to the end.
		cy.getFrameWindow().should(function(win) {
			var order = partHashes(win);
			var rest = orderBefore.filter(function(part) {
				return selectedBefore.indexOf(part) === -1;
			});
			expect(order).to.deep.equal(rest.concat(selectedBefore));
		});
	});
});
