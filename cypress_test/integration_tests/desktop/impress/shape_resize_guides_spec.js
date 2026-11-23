/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Guides while a handle is dragged on a vector slide. The slide has two rectangles with an
// outline, side by side with their tops on one line: one 3 cm high on the left and one 2.5 cm high
// on the right.
describe(['tagdesktop'], 'Guides while resizing on a vector slide', function() {

	var CM = 1440 / 2.54;

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/shape_resize_guides.fodp');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			// The usual load checks wait for tiles, which a vector view does not have.
			cy.wrap(null).should(function() {
				expect(win.app.impress.partList).to.have.length.greaterThan(0);
			});
			return helper.processToIdle(win);
		}.bind(this));
	});

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The box of the object on the left or the right half of the page, in twips, outline and all.
	function boxOn(win, right) {
		var objectId = geometry(win).heldByThePage().find(function(one) {
			return (geometry(win).boxOf(one).minX > 10 * CM) === right;
		});
		return geometry(win).boxOf(objectId);
	}

	// Converts a place on the page in twips to CSS pixels on the document canvas.
	function onCanvas(win, x, y) {
		var visible = win.app.activeDocument.activeLayout.viewedRectangle;
		var scale = win.app.twipsToPixels / win.app.dpiScale;
		return {
			x: x * scale - visible.pX1 / win.app.dpiScale,
			y: y * scale - visible.pY1 / win.app.dpiScale,
		};
	}

	// Selects the rectangle on the right by its middle.
	function selectTheRightOne(win) {
		cy.then(function() {
			var box = boxOn(win, true);
			var middle = onCanvas(win, (box.minX + box.maxX) / 2, (box.minY + box.maxY) / 2);
			cy.cGet('#document-canvas').click(middle.x, middle.y);
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(win.eval('GraphicSelection.selectedObjectIDs'), 'selected').to.have.length(1);
		});
	}

	/*
		Drags from one place in twips to another place in CSS pixels on the canvas, in steps, with
		those keys held. Both places are named by functions, because they are known only once the
		commands before this one have run. The test is handed the snap of the selection while the button
		is still down, and the button is let go after it.
	*/
	function drag(win, from, to, keys, whileHeld) {
		cy.then(function() {
			var place = from();
			var start = onCanvas(win, place.x, place.y);
			cy.cGet('#document-canvas').trigger('mousemove', start.x, start.y,
				Object.assign({ button: 0, buttons: 0 }, keys));
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', start.x, start.y,
				Object.assign({ button: 0, buttons: 1 }, keys));
			var end = to();
			for (var step = 1; step <= 10; ++step)
				drawing = drawing.trigger('mousemove',
					start.x + step / 10 * (end.x - start.x),
					start.y + step / 10 * (end.y - start.y),
					Object.assign({ button: 0, buttons: 1 }, keys));
			cy.then(function() {
				whileHeld(win.eval('GraphicSelection.selectionSection').snap);
			});
			cy.cGet('#document-canvas').trigger('mouseup', end.x, end.y,
				Object.assign({ button: 0, buttons: 0 }, keys));
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	// The bottom edge, outline and all, of the right one is dragged to 3 pixels short of that of the
	// left one, and lands on it.
	function dragTheBottomOnto(win, keys) {
		var middle = function() {
			var box = boxOn(win, true);
			return { x: (box.minX + box.maxX) / 2, y: 5.5 * CM };
		};
		var guide;
		drag(win, middle, function() {
			var to = onCanvas(win, middle().x, boxOn(win, false).maxY);
			return { x: to.x, y: to.y - 3 };
		}, keys, function(snap) {
			guide = snap.closestY;
			expect(guide, 'the guide while dragging').to.not.equal(null);
		});

		// The outer edge is where the guide was, not where the mouse was let go, and not where the
		// handle on the inside of the outline would put it.
		cy.wrap(null).should(function() {
			expect(boxOn(win, true).maxY, 'the bottom edge').to.be.closeTo(
				guide / win.app.twipsToPixels, 5);
		});
	}

	it('A resize that keeps the ratio lands on the guide it showed.', function() {
		var win = this.win;
		selectTheRightOne(win);
		dragTheBottomOnto(win, { shiftKey: true });
	});

	it('A resize onto a guide makes two objects the same height.', function() {
		var win = this.win;
		selectTheRightOne(win);
		dragTheBottomOnto(win, {});

		cy.wrap(null).should(function() {
			var left = boxOn(win, false);
			var right = boxOn(win, true);
			expect(right.maxY - right.minY, 'the height').to.be.closeTo(left.maxY - left.minY, 5);
		});

		// So that a move lined up with the other one shows both its top and its bottom.
		var from = function() {
			var box = boxOn(win, true);
			return { x: (box.minX + box.maxX) / 2, y: (box.minY + box.maxY) / 2 };
		};
		drag(win, from, function() {
			var start = from();
			var to = onCanvas(win, start.x, start.y);
			return { x: to.x - 20, y: to.y + 2 };
		}, {}, function(snap) {
			expect(snap.edgesY, 'lines along the top and the bottom').to.have.length(2);
		});
	});
});
