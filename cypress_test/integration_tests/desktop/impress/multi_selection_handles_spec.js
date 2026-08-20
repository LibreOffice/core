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

	// Extent of one preview outline, in the same core pixels as the shape geometry.
	function boundingBox(points) {
		var xs = points.map(function(point) { return point[0]; });
		var ys = points.map(function(point) { return point[1]; });
		var left = Math.min.apply(null, xs);
		var top = Math.min.apply(null, ys);
		var right = Math.max.apply(null, xs);
		var bottom = Math.max.apply(null, ys);

		return {
			width: right - left,
			height: bottom - top,
			centerX: (left + right) / 2,
			centerY: (top + bottom) / 2
		};
	}

	// The extent a set of boxes covers together, as left, top, right and bottom.
	function unionOf(boxes) {
		return boxes.reduce(function(all, b) {
			var left = b.centerX - b.width / 2;
			var top = b.centerY - b.height / 2;
			return all === null ? [left, top, left + b.width, top + b.height] : [
				Math.min(all[0], left), Math.min(all[1], top),
				Math.max(all[2], left + b.width), Math.max(all[3], top + b.height)];
		}, null);
	}

	// The same extent, read from the geometry the shapes settled at.
	function unionOfGeometries(geometries) {
		return unionOf(geometries.map(function(g) {
			return { centerX: g.center.pX, centerY: g.center.pY,
				width: g.width, height: g.height };
		}));
	}

	// The two shapes are far apart, so the closer center tells which one an outline is for.
	function nearestGeometry(geometries, box) {
		return geometries.reduce(function(best, candidate) {
			var distance = Math.abs(candidate.center.pX - box.centerX)
				+ Math.abs(candidate.center.pY - box.centerY);
			return (best === null || distance < best.distance)
				? { distance: distance, width: candidate.width, height: candidate.height,
					center: candidate.center }
				: best;
		}, null);
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

	// The preview drawn while a handle is dragged has to show the geometry the drop lands on.
	// Core resizes every selected shape about its own corner, which one scale of the whole
	// selection cannot show, so the preview comes from the outlines core reports.
	it('The drag preview shows what the drop produces.', function() {
		selectBothShapes();

		// Drag the bottom-right corner of the first shape well away from the shape.
		var handleName = null;
		var widthsBefore = null;
		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			widthsBefore = section.sectionProperties.shapeRectangleProperties.map(
				function(own) { return own.width; });
			var handle = getScalingHandles(section).find(function(subSection) {
				return subSection.sectionProperties.ownInfo.kind === '8'
					&& subSection.sectionProperties.ownInfo.objectNumber === 0;
			});
			expect(handle, 'bottom-right handle of the first shape').to.not.be.undefined;
			handleName = handle.name;
		});

		cy.then(function() {
			cy.cGet('#test-div-' + handleName).then(($handle) => {
				var rect = $handle[0].getBoundingClientRect();
				var startX = rect.left + rect.width / 2;
				var startY = rect.top + rect.height / 2;
				var endX = startX + 80;
				var endY = startY + 50;

				cy.cGet('body').realMouseDown({ x: startX, y: startY });
				cy.cGet('body').realMouseMove(startX + 40, startY + 25);
				cy.cGet('body').realMouseMove(endX, endY);

				// Core reports an outline for each of the two selected shapes. Wait for the
				// answer to the last position asked about, since a request in flight holds back
				// the newest position until it returns.
				cy.waitUntil(() => {
					var properties = getHandlesSection(this.win).sectionProperties;
					return Boolean(properties.shapeDragPreviewPolygons
						&& properties.shapeDragPreviewPolygons.length === 2
						&& properties.shapeDragPreviewRequestTime === null
						&& properties.queuedShapeDragPreview === null);
				}, {
					errorMsg: 'The drag preview outlines for the last position did not arrive.'
				});

				cy.then(() => {
					var polygons = getHandlesSection(this.win)
						.sectionProperties.shapeDragPreviewPolygons;
					this.previewBoxes = polygons.map(boundingBox);
				});

				cy.cGet('body').realMouseUp({ x: endX, y: endY });

				cy.then(() => {
					return helper.processToIdle(this.win);
				});

				// Both shapes now sit where their preview outline was. Asking core for a
				// preview runs a drag of its own there, and each of those reports the
				// selection again, so a width that merely differs from the one before the drag
				// can still be an answer to a preview that is on its way. The geometry is read
				// until two readings in a row agree on it.
				var settledWidth = null;
				helper.retryUntil(
					() => {
						cy.getFrameWindow().then((win) => {
							return helper.processToIdle(win);
						});
					},
					() => {
						var geometries = getHandlesSection(this.win)
							.sectionProperties.shapeRectangleProperties;
						if (geometries.length !== 2) return false;

						var width = geometries[0].width;
						var settled = settledWidth === width
							&& Math.abs(width - widthsBefore[0]) > 5;
						settledWidth = width;
						return settled;
					},
					{ errorMsg: 'The shapes did not settle at the size the drop gives them.' });

				cy.then(() => {
					var geometries = getHandlesSection(this.win)
						.sectionProperties.shapeRectangleProperties;

					/*
						An object answers with one outline per path it is drawn from, so a
						frame or a custom shape gives several and an outline cannot be paired
						with a shape by its place in the list. What the outlines cover
						together is compared instead, which holds whatever the shapes are.
					*/
					expect(this.previewBoxes.length, 'outlines for two shapes')
						.to.be.at.least(geometries.length);

					var previewArea = unionOf(this.previewBoxes);
					var droppedArea = unionOfGeometries(geometries);
					['left', 'top', 'right', 'bottom'].forEach(function(side, i) {
						expect(previewArea[i], side + ' of what the outlines cover')
							.to.be.closeTo(droppedArea[i], 4);
					});

					// Every outline lies on a shape, so none of them is left somewhere else.
					this.previewBoxes.forEach(function(box, outline) {
						var dropped = nearestGeometry(geometries, box);
						expect(Math.abs(box.centerX - dropped.center.pX),
							'outline ' + outline + ' away from its shape in x')
							.to.be.at.most(dropped.width / 2 + 4);
						expect(Math.abs(box.centerY - dropped.center.pY),
							'outline ' + outline + ' away from its shape in y')
							.to.be.at.most(dropped.height / 2 + 4);
					});
				});
			});
		});
	});

});
