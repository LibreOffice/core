/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Equal spacing guides while an object is carried on a vector slide. The slide has two rows of
// three rectangles, each 3 cm wide. In the upper row the first two stand 2 cm apart and the third
// stands 3 cm from the second. In the lower row the middle one stands off centre between the
// other two.
describe(['tagdesktop'], 'Equal spacing while carrying an object on a vector slide', function() {

	var CM = 1440 / 2.54;

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/shape_equal_spacing.fodp');
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

	// The object of the page whose box begins at that place, in twips, near enough.
	function objectAt(win, x, y) {
		return geometry(win).heldByThePage().find(function(objectId) {
			var box = geometry(win).boxOf(objectId);
			return box && Math.abs(box.minX - x) < 30 && Math.abs(box.minY - y) < 30;
		});
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

	/*
		Selects the object the function names, presses on its middle and carries it the given
		distance in twips, in steps. The object is named by a function, because it is found only
		once the commands before this one have run. The test is handed the snap of the selection while the button is still down, and
		the button is let go after it.
	*/
	function carry(win, which, across, whileHeld) {
		var from, to, objectId;
		cy.then(function() {
			objectId = which();
			var box = geometry(win).boxOf(objectId);
			from = { x: (box.minX + box.maxX) / 2, y: (box.minY + box.maxY) / 2 };
			to = { x: from.x + across, y: from.y };
			var at = onCanvas(win, from.x, from.y);
			cy.cGet('#document-canvas').click(at.x, at.y);
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(win.eval('GraphicSelection.selectedObjectIDs'), 'selected').to.deep.equal([objectId]);
		});

		cy.then(function() {
			var first = onCanvas(win, from.x, from.y);
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', first.x, first.y,
				{ button: 0, buttons: 1 });
			for (var step = 1; step <= 10; ++step) {
				var place = onCanvas(win, from.x + step / 10 * (to.x - from.x), from.y);
				drawing = drawing.trigger('mousemove', place.x, place.y, { button: 0, buttons: 1 });
			}
		});
		cy.then(function() {
			whileHeld(win.eval('GraphicSelection.selectionSection').snap);
		});
		cy.then(function() {
			var last = onCanvas(win, to.x, to.y);
			cy.cGet('#document-canvas').trigger('mouseup', last.x, last.y, { button: 0, buttons: 0 });
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('Carried near the gap of its row, an object lands that far from its neighbour.', function() {
		var win = this.win;
		var first, second, third;
		cy.wrap(null).should(function() {
			first = objectAt(win, 2 * CM, 2 * CM);
			second = objectAt(win, 7 * CM, 2 * CM);
			third = objectAt(win, 13 * CM, 2 * CM);
			expect([first, second, third], 'the upper row').to.not.include(undefined);
		});

		// 2 cm from the second one is 12 cm, so this stops a little short of that.
		carry(win, function() { return third; }, -0.95 * CM, function(snap) {
			expect(snap.spacingX, 'equal spacing across').to.equal(true);
			// The gap between the first two and the one beside the carried object.
			expect(snap.equalGaps, 'equal gaps').to.have.length(2);
			// The row has one height, so the carried object lies on its top and its bottom line.
			expect(snap.edgesY, 'lines along the row').to.have.length(2);
		});

		cy.wrap(null).should(function() {
			var a = geometry(win).boxOf(first);
			var b = geometry(win).boxOf(second);
			var c = geometry(win).boxOf(third);
			expect(c.minX - b.maxX, 'the new gap').to.be.closeTo(b.minX - a.maxX, 5);
			expect(c.minY, 'the top').to.be.closeTo(2 * CM, 5);
		});
	});

	it('Carried near the middle between its neighbours, an object lands there.', function() {
		var win = this.win;
		var first, middle, last;
		cy.wrap(null).should(function() {
			first = objectAt(win, 1.5 * CM, 10 * CM);
			middle = objectAt(win, 8.5 * CM, 10 * CM);
			last = objectAt(win, 14 * CM, 10 * CM);
			expect([first, middle, last], 'the lower row').to.not.include(undefined);
		});

		// The middle is at 7.75 cm, so this stops a little short of that.
		carry(win, function() { return middle; }, -0.65 * CM, function(snap) {
			expect(snap.spacingX, 'equal spacing across').to.equal(true);
			expect(snap.equalGaps, 'equal gaps').to.have.length(2);
		});

		cy.wrap(null).should(function() {
			var a = geometry(win).boxOf(first);
			var b = geometry(win).boxOf(middle);
			var c = geometry(win).boxOf(last);
			expect(b.minX - a.maxX, 'the gap before').to.be.closeTo(c.minX - b.maxX, 5);
		});
	});
});
