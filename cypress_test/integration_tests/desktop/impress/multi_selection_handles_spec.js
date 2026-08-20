/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Handles of several selected shapes.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/shape_alignment_guides.fodp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function selectBothShapes() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			return helper.processToIdle(win);
		});

		// With several shapes selected the resize handles are named after their place in
		// core's handle list.
		cy.cGet('[id^="test-div-shape-handle-index"]').should('have.length', 16);
	}

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	function getScalingHandles(section) {
		return section.sectionProperties.subSections.filter(function(subSection) {
			return ['1', '2', '3', '4', '5', '6', '7', '8']
				.includes(subSection.sectionProperties.ownInfo.kind);
		});
	}

	// Both shapes are marked, so both of them show a full set of resize handles. Reading
	// only the first entry core sends under each handle kind left the second shape bare.
	it('Every selected shape shows its own resize handles.', function() {
		selectBothShapes();

		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			expect(section.sectionProperties.selectedObjectCount, 'selected shapes').to.equal(2);

			var scalingHandles = getScalingHandles(section);
			expect(scalingHandles, 'resize handles').to.have.length(16);

			[0, 1].forEach(function(shapeNumber) {
				var own = scalingHandles.filter(function(subSection) {
					return subSection.sectionProperties.ownInfo.objectNumber === shapeNumber;
				});
				expect(own, 'resize handles of shape ' + shapeNumber).to.have.length(8);
			});
		});
	});

	// The two shapes in the document have clearly different sizes, so handles positioned
	// from a single shared geometry would land away from the smaller shape.
	it('Each selected shape carries its own geometry.', function() {
		selectBothShapes();

		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			var geometries = section.sectionProperties.shapeRectangleProperties;
			expect(geometries, 'geometry per shape').to.have.length(2);

			var widths = [geometries[0].width, geometries[1].width];
			expect(Math.abs(widths[0] - widths[1]), 'difference in width').to.be.greaterThan(5);

			// Every handle sits within its own shape's half diagonal of that shape's center.
			getScalingHandles(section).forEach(function(subSection) {
				var own = geometries[subSection.sectionProperties.ownInfo.objectNumber];
				var halfDiagonal = Math.sqrt(Math.pow(own.width * 0.5, 2) + Math.pow(own.height * 0.5, 2));
				var handleCenterX = subSection.position[0] + subSection.size[0] * 0.5;
				var handleCenterY = subSection.position[1] + subSection.size[1] * 0.5;
				var distance = Math.sqrt(Math.pow(handleCenterX - own.center.pX, 2)
					+ Math.pow(handleCenterY - own.center.pY, 2));

				expect(distance, 'distance from the shape center').to.be.closeTo(
					subSection.sectionProperties.distanceToCenter, 2);
				expect(distance, 'distance within the shape').to.be.at.most(halfDiagonal + 2);
			});
		});
	});

	// Dropping the selection has to take the handles of every shape away with it.
	it('Dropping the selection leaves no handles behind.', function() {
		selectBothShapes();

		cy.cGet('[id^="test-div-shape-handle-index"]').should('have.length', 16);

		impressHelper.removeShapeSelection();

		cy.cGet('[id^="test-div-shape-handle-"]').should('not.exist');
	});

});
