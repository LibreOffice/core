/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Selecting by a drag on the empty page of a vector slide. The slide has a rectangle on the left
// and a polygon on the right, and empty page around both.
describe(['tagdesktop'], 'Selecting by a drag on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/path_points.fodp');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			// The usual load checks wait for tiles, which a vector view does not have.
			cy.wrap(null).should(function() {
				expect(win.app.impress.partList).to.have.length.greaterThan(0);
			});
			return helper.processToIdle(win);
		}.bind(this));
		desktopHelper.switchUIToNotebookbar();
	});

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The ids of the objects selected, in the order of the page.
	function selected(win) {
		return win.eval('GraphicSelection.selectedObjectIDs').slice().sort(function(a, b) {
			return a - b;
		});
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

	// Drags from one place to another, in twips on the page, in steps, with those keys held.
	function dragFrom(win, from, to, keys) {
		var held = { shiftKey: !!(keys && keys.shift) };
		cy.then(function() {
			var first = onCanvas(win, from.x, from.y);
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', first.x, first.y,
				Object.assign({ button: 0, buttons: 1 }, held));
			for (var step = 1; step <= 10; ++step) {
				var place = onCanvas(win,
					from.x + step / 10 * (to.x - from.x), from.y + step / 10 * (to.y - from.y));
				drawing = drawing.trigger('mousemove', place.x, place.y,
					Object.assign({ button: 0, buttons: 1 }, held));
			}
			var last = onCanvas(win, to.x, to.y);
			drawing.trigger('mouseup', last.x, last.y,
				Object.assign({ button: 0, buttons: 0 }, held));
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A frame selects the objects wholly inside it, and Shift changes their state.', function() {
		var win = this.win;
		var rectangle, polygon;
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
			rectangle = objectAt(win, 1134, 2268);
			polygon = objectAt(win, 9071, 2268);
			expect(rectangle, 'the rectangle').to.not.be.undefined;
			expect(polygon, 'the polygon').to.not.be.undefined;
		});

		// around both
		dragFrom(win, { x: 500, y: 1500 }, { x: 15000, y: 7000 });
		cy.wrap(null).should(function() {
			expect(selected(win), 'selected after a frame around both')
				.to.deep.equal([rectangle, polygon].sort(function(a, b) { return a - b; }));
		});

		// around the rectangle only, and across the polygon, which it does not hold wholly
		dragFrom(win, { x: 500, y: 1500 }, { x: 10000, y: 7000 });
		cy.wrap(null).should(function() {
			expect(selected(win), 'selected after a frame around the rectangle')
				.to.deep.equal([rectangle]);
		});

		// with Shift a frame around the polygon adds it, and one around the rectangle takes it away
		dragFrom(win, { x: 8500, y: 1500 }, { x: 15000, y: 7000 }, { shift: true });
		cy.wrap(null).should(function() {
			expect(selected(win), 'selected after Shift and a frame around the polygon')
				.to.deep.equal([rectangle, polygon].sort(function(a, b) { return a - b; }));
		});
		dragFrom(win, { x: 500, y: 1500 }, { x: 7000, y: 7000 }, { shift: true });
		cy.wrap(null).should(function() {
			expect(selected(win), 'selected after Shift and a frame around the rectangle')
				.to.deep.equal([polygon]);
		});

		// nothing of the frame is left drawn
		cy.wrap(null).should(function() {
			expect(win.eval('RenderGeometrySection.dragSelect'), 'the frame drawn').to.equal(null);
		});
	});
});
