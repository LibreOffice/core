/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Scaling a turned shape on a vector slide. The slide holds one rectangle drawn as a custom shape,
// turned by 30 degrees.
describe(['tagdesktop'], 'Scaling a turned shape on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/turned_shape_scale.fodp');
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

	// The one object of the slide.
	function theShape(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (!found && object.id)
				found = object;
		});
		return found;
	}

	// The length of the two sides of the object, in twips, as its mapping has them.
	function sidesOf(object) {
		var t = object.transform;
		return { across: Math.hypot(t[0], t[1]), down: Math.hypot(t[2], t[3]) };
	}

	// The handle of that name of the selection, in twips.
	function handleAt(win, name) {
		var section = win.app.sectionContainer.sections.find(function(one) {
			return one.name.indexOf('selected-object-') === 0;
		});
		var handle = section && section.handles().find(function(one) {
			return one.name === name;
		});
		return handle && handle.point;
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

	// Drags from that place on the page, in twips, by that many twips across and down, in steps.
	function dragFrom(win, from, across, down) {
		cy.then(function() {
			var start = onCanvas(win, from.x, from.y);
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', start.x, start.y,
				{ button: 0, buttons: 1 });
			for (var step = 1; step <= 10; ++step) {
				var place = onCanvas(win, from.x + step / 10 * across, from.y + step / 10 * down);
				drawing = drawing.trigger('mousemove', place.x, place.y, { button: 0, buttons: 1 });
			}
			var end = onCanvas(win, from.x + across, from.y + down);
			drawing.trigger('mouseup', end.x, end.y, { button: 0, buttons: 0 });
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A pull on one side of a turned shape scales it in ratio, as the office does.', function() {
		var win = this.win;
		var shape;
		var before;
		cy.wrap(null).should(function() {
			shape = theShape(win);
			expect(shape, 'the shape').to.not.equal(undefined);
			expect(shape.cannotBeResizedFreely, 'a turned shape allows no free resize')
				.to.equal(true);
		});
		cy.then(function() {
			before = sidesOf(shape);
			win.eval('GraphicSelection').selectObjects([shape.id]);
		});
		cy.wrap(null).should(function() {
			expect(handleAt(win, '5.0.0'), 'the handle of the right side').to.not.equal(undefined);
		});

		// the right side is pulled outwards along the turned across axis of the shape
		cy.then(function() {
			var t = shape.transform;
			var length = Math.hypot(t[0], t[1]);
			dragFrom(win, handleAt(win, '5.0.0'), 2000 * t[0] / length, 2000 * t[1] / length);
		});

		cy.wrap(null).should(function() {
			var now = theShape(win);
			expect(geometry(win).awaitsAnAnswer(now.id), 'a change waiting').to.equal(false);
			var after = sidesOf(now);
			var across = after.across / before.across;
			var down = after.down / before.down;
			expect(across, 'grown across').to.be.above(1.1);
			expect(Math.abs(across - down), 'the same factor both ways').to.be.below(0.02);
		});
	});
});
