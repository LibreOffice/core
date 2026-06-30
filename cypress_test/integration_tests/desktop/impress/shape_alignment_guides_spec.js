/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Shape alignment guides.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/shape_alignment_guides.fodp');
	});

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	// Select the shape in the center of the slide and return, for the test
	// callback, the handles section together with the geometry of both the
	// selected shape and the other shape (the alignment target).
	function withSelectedShape(callback) {
		// The smaller rectangle sits in the center of the slide.
		impressHelper.clickCenterOfSlide({});
		cy.cGet('#test-div-shape-handle-rotation').should('exist');

		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			var extraInfo = win.app.definitions.graphicSelection.extraInfo;

			// Object-boundary guides are only computed when snap-to-grid is off
			// (otherwise the grid wins). The test environment may load with it on,
			// so force it off before driving the snap.
			win.app.map.stateChangeHandler.setItemValue('.uno:GridUse', 'false');

			// Core sends the rectangles of the objects (in pixels) so the browser
			// can snap the dragged shape to the others' boundaries.
			var rectangles = extraInfo.ObjectRectangles;
			expect(rectangles, 'ObjectRectangles from core').to.be.an('array');

			// Identify the alignment target as the rectangle whose width differs
			// most from the selected shape. The two shapes are intentionally very
			// different sizes, so this reliably picks the other shape without
			// depending on the ordinal number.
			var selfWidth = section.size[0];
			var other = null;
			var bestDiff = -1;
			rectangles.forEach(function(r) {
				var diff = Math.abs(r[2] - selfWidth);
				if (diff > bestDiff) { bestDiff = diff; other = r; }
			});
			expect(bestDiff, 'a differently sized other shape exists').to.be.greaterThan(5);

			callback(section, section.position, section.size, other);
		});
	}

	// Regression test for the alignment guides drawn while a shape is dragged
	// close to another shape's boundaries (edge-to-edge snapping).
	it('Snaps an edge to another shape edge.', function() {
		withSelectedShape(function(section, position, size, other) {
			// Drag so the selected shape's left and top edges meet the other
			// shape's left and top edges.
			var dragX = other[0] - position[0];
			var dragY = other[1] - position[1];
			section.checkHelperLinesAndSnapPoints(size, position, [dragX, dragY]);

			var p = section.sectionProperties;

			// A guide is drawn on each axis at the other shape's edge.
			expect(p.closestX, 'closestX').to.be.closeTo(other[0], 1);
			expect(p.closestY, 'closestY').to.be.closeTo(other[1], 1);

			// The left/top edge snapped, so there is no offset from the corner.
			expect(p.snapOffsetX, 'snapOffsetX').to.equal(0);
			expect(p.snapOffsetY, 'snapOffsetY').to.equal(0);

			// An edge match is not a center match: no center markers.
			expect(p.centerToCenterX, 'centerToCenterX').to.be.false;
			expect(p.centerToCenterY, 'centerToCenterY').to.be.false;
			expect(p.centerSnapX, 'centerSnapX').to.be.null;
			expect(p.centerSnapY, 'centerSnapY').to.be.null;
			expect(p.draggedCenter, 'draggedCenter').to.be.null;
		});
	});

	// Regression test for center-to-center snapping and the red center dots
	// drawn on both the target shape and the active (dragged) shape.
	it('Snaps the center to another shape center.', function() {
		withSelectedShape(function(section, position, size, other) {
			var activeCenterX = position[0] + size[0] / 2;
			var activeCenterY = position[1] + size[1] / 2;
			var otherCenterX = other[0] + other[2] / 2;
			var otherCenterY = other[1] + other[3] / 2;

			// Drag so the selected shape's center meets the other shape's center.
			var dragX = otherCenterX - activeCenterX;
			var dragY = otherCenterY - activeCenterY;
			section.checkHelperLinesAndSnapPoints(size, position, [dragX, dragY]);

			var p = section.sectionProperties;

			// A guide is drawn through the other shape's center on each axis.
			expect(p.closestX, 'closestX').to.be.closeTo(otherCenterX, 1);
			expect(p.closestY, 'closestY').to.be.closeTo(otherCenterY, 1);

			// The center snapped, so the offset from the corner is half the size.
			expect(p.snapOffsetX, 'snapOffsetX').to.be.closeTo(size[0] / 2, 1);
			expect(p.snapOffsetY, 'snapOffsetY').to.be.closeTo(size[1] / 2, 1);

			// Both axes are a true center-to-center match.
			expect(p.centerToCenterX, 'centerToCenterX').to.be.true;
			expect(p.centerToCenterY, 'centerToCenterY').to.be.true;

			// The red dot on the target shape sits at its center.
			expect(p.centerSnapX, 'centerSnapX').to.be.an('array');
			expect(p.centerSnapX[0], 'centerSnapX[0]').to.be.closeTo(otherCenterX, 1);
			expect(p.centerSnapX[1], 'centerSnapX[1]').to.be.closeTo(otherCenterY, 1);

			// The red dot on the active shape sits on the guides (the snapped center).
			expect(p.draggedCenter, 'draggedCenter').to.be.an('array');
			expect(p.draggedCenter[0], 'draggedCenter[0]').to.be.closeTo(otherCenterX, 1);
			expect(p.draggedCenter[1], 'draggedCenter[1]').to.be.closeTo(otherCenterY, 1);
		});
	});
});
