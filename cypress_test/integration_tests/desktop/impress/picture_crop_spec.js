/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Cropping a picture on a vector slide. The slide holds one picture, which is not cropped.
describe(['tagdesktop'], 'Cropping a picture on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/picture_crop.fodp');
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

	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	function picture(win) {
		var section = selectionSection(win);
		return section ? geometry(win).objectOf(section.selectedObjects()[0]) : undefined;
	}

	// The crop of the picture on its four sides, as the engine reports it.
	function cropOf(win) {
		var object = picture(win);
		return [object.cropLeft || 0, object.cropTop || 0, object.cropRight || 0,
			object.cropBottom || 0];
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

	it('The 0 key takes the crop away while the crop marks are shown.', function() {
		var win = this.win;
		helper.typeIntoDocument('{esc}');
		cy.realPress('Tab');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(picture(win), 'the picture selected').to.not.equal(undefined);
		});

		// the crop switch of the toolbar, as the button is
		cy.then(function() {
			win.eval('GraphicSelection.toggleCropping()');
		});
		cy.wrap(null).should(function() {
			expect(win.eval('GraphicSelection.cropping'), 'cropping').to.equal(true);
		});

		// the mark at the upper left corner is dragged into the picture
		cy.then(function() {
			var mark = selectionSection(win).handles().find(function(handle) {
				return handle.name.indexOf('crop') === 0 && handle.name !== 'cropMove';
			});
			dragFrom(win, mark.point, 2000, 1500);
		});
		cy.wrap(null).should(function() {
			var crop = cropOf(win);
			expect(crop[0], 'cropped from the left').to.be.greaterThan(0);
			expect(crop[1], 'cropped from the top').to.be.greaterThan(0);
		});

		helper.typeIntoDocument('0');
		cy.wrap(null).should(function() {
			expect(cropOf(win), 'the crop after 0').to.deep.equal([0, 0, 0, 0]);
		});

		// the crop is one step to undo, as any other
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:Undo');
		});
		cy.wrap(null).should(function() {
			expect(cropOf(win)[0], 'cropped again after the undo').to.be.greaterThan(0);
		});
	});
});
