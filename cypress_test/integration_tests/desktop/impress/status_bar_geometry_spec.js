/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// The place and the size of the selection in the status bar of a vector slide. The slide holds
// a rectangle 9 cm wide and 6 cm high, 2 cm from the left and 4 cm from the top, and a polygon.
describe(['tagdesktop'], 'The selection geometry in the status bar of a vector slide', function() {

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
	});

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The rectangle of the slide, the object whose box starts furthest left.
	function theRectangle(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (object.id && (!found || object.x < found.x))
				found = object;
		});
		return found;
	}

	// A length in twips as the status bar writes it, in the unit of the ruler.
	function inTheUnit(win, twips) {
		return win.eval('ClassicCreateContext').inTheUnit(twips);
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

	it('The status bar shows where the selection or the mouse stands and how large the selection is.', function() {
		var win = this.win;
		var rectangle;
		cy.wrap(null).should(function() {
			rectangle = theRectangle(win);
			expect(rectangle, 'the rectangle').to.not.equal(undefined);
		});

		// with nothing selected the place is the one of the mouse, and the size none
		cy.cGet('#objectposition-container').should('be.visible');
		cy.cGet('#objectsize-container').should('be.visible');
		cy.then(function() {
			var place = onCanvas(win, 2880, 7200);
			cy.cGet('#document-canvas').trigger('mouseenter', place.x, place.y)
				.trigger('mousemove', place.x, place.y);
		});
		cy.cGet('#ObjectPosition').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 2880) + ' / ' + inTheUnit(win, 7200));
		});
		cy.cGet('#ObjectSize').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 0) + ' x ' + inTheUnit(win, 0));
		});

		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([rectangle.id]);
		});
		// 2 cm and 4 cm from the corner, 9 cm by 6 cm, which are 1134, 2268, 5103 and 3402 twips
		cy.cGet('#ObjectPosition').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 1134) + ' / ' + inTheUnit(win, 2268));
		});
		cy.cGet('#ObjectSize').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 5103) + ' x ' + inTheUnit(win, 3402));
		});

		// a move of about 1 cm to the right and down shows the new place and the same size
		cy.then(function() {
			var from = onCanvas(win, 1134 + 5103 / 2, 2268 + 3402 / 2);
			var to = onCanvas(win, 1134 + 567 + 5103 / 2, 2268 + 567 + 3402 / 2);
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', from.x, from.y,
				{ button: 0, buttons: 1 });
			for (var step = 1; step <= 10; ++step)
				drawing = drawing.trigger('mousemove', from.x + step / 10 * (to.x - from.x),
					from.y + step / 10 * (to.y - from.y), { button: 0, buttons: 1 });
			drawing.trigger('mouseup', to.x, to.y, { button: 0, buttons: 0 });
		});
		// the drag may snap, so the place is the one the rectangle's mapping has after it
		cy.cGet('#ObjectPosition').should(function(item) {
			var mapping = geometry(win).objectOf(rectangle.id).transform;
			expect(mapping[4], 'moved').to.be.above(1134 + 300);
			expect(item.text()).to.equal(inTheUnit(win, mapping[4]) + ' / ' + inTheUnit(win, mapping[5]));
		});
		cy.cGet('#ObjectSize').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 5103) + ' x ' + inTheUnit(win, 3402));
		});

		// nothing selected again: the fields stay, with no size
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([]);
		});
		cy.cGet('#objectposition-container').should('be.visible');
		cy.cGet('#ObjectSize').should(function(item) {
			expect(item.text()).to.equal(inTheUnit(win, 0) + ' x ' + inTheUnit(win, 0));
		});

		// the text of the selection and the master slide have their fields too
		cy.cGet('#selectiontext-container').should('be.visible');
		cy.cGet('#layoutstatus-container').should('be.visible');
	});
});
