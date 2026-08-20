/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Handles of a shape in point edit mode.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/multi_selection_point_edit.fodp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	function getPointHandles(section) {
		return section.sectionProperties.subSections.filter(function(subSection) {
			return subSection.sectionProperties.ownInfo.kind === '9';
		});
	}

	// Where each point handle sits, kept under the handle's name so that the reading survives
	// the sub sections being gathered in another order.
	function getPointHandlePositions(section) {
		var positions = {};

		getPointHandles(section).forEach(function(subSection) {
			positions[subSection.name] = [subSection.position[0], subSection.position[1]];
		});

		return positions;
	}

	// Tab moves the selection onto the first shape on the slide, so the test does not depend
	// on where a click lands in the view.
	function selectFirstShape() {
		helper.typeIntoDocument('{esc}');
		cy.realPress('Tab');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		// The triangle is one shape on its own, so its handles carry the plain names.
		cy.cGet('#test-div-shape-handle-rotation').should('exist');
	}

	// Point edit mode keeps the same shape selected, so the handles drawn for it are replaced
	// in place rather than built afresh. The three corners of the triangle take the names the
	// resize handles held.
	function turnPointEditModeOn() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:ToggleObjectBezierMode');
			return helper.processToIdle(win);
		});

		cy.cGet('#test-div-shape-handle-rotation').should('not.exist');
		cy.cGet('[id^="test-div-shape-handle-"]').should('have.length', 3);
	}

	// A resize handle left in place for a point would draw the wrong marker and read geometry
	// the shape no longer reports.
	it('Point edit mode replaces the resize handles with point handles.', function() {
		selectFirstShape();
		turnPointEditModeOn();

		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			var pointHandles = getPointHandles(section);
			expect(pointHandles, 'point handles').to.have.length(3);

			pointHandles.forEach(function(subSection) {
				expect(subSection.constructor === win.app.definitions.shapeHandlePolySubSection,
					'handle ' + subSection.name + ' drawn as ' + subSection.constructor.name)
					.to.be.true;
			});
		});
	});

	// A shape in point edit mode reports no resize geometry, so a point handle left in the
	// hands of a resize handle had nothing to read and the drag went nowhere.
	it('Dragging a point handle in point edit mode moves that point.', function() {
		selectFirstShape();
		turnPointEditModeOn();

		var handleName = null;
		var positionsBefore = null;
		cy.getFrameWindow().then(function(win) {
			var section = getHandlesSection(win);
			positionsBefore = getPointHandlePositions(section);
			handleName = getPointHandles(section)[0].name;
		});

		cy.then(function() {
			cy.cGet('#test-div-' + handleName).then(($handle) => {
				var rect = $handle[0].getBoundingClientRect();
				var startX = rect.left + rect.width / 2;
				var startY = rect.top + rect.height / 2;
				var dragX = 60;
				var dragY = 40;
				var endX = startX + dragX;
				var endY = startY + dragY;

				cy.cGet('body').realMouseDown({ x: startX, y: startY });
				cy.cGet('body').realMouseMove(startX + dragX / 2, startY + dragY / 2);
				cy.cGet('body').realMouseMove(endX, endY);
				cy.cGet('body').realMouseUp({ x: endX, y: endY });

				cy.then(() => {
					return helper.processToIdle(this.win);
				});

				cy.waitUntil(() => {
					var moved = getPointHandlePositions(getHandlesSection(this.win))[handleName];
					return Boolean(moved)
						&& Math.abs(moved[0] - positionsBefore[handleName][0]) > 2;
				}, {
					errorMsg: 'The dragged point did not move.'
				});

				cy.then(() => {
					var positionsAfter = getPointHandlePositions(getHandlesSection(this.win));
					var dpiScale = this.win.app.dpiScale;

					// The dragged point followed the pointer.
					expect(positionsAfter[handleName][0] - positionsBefore[handleName][0],
						'sideways move of the dragged point')
						.to.be.closeTo(dragX * dpiScale, 5);
					expect(positionsAfter[handleName][1] - positionsBefore[handleName][1],
						'downward move of the dragged point')
						.to.be.closeTo(dragY * dpiScale, 5);

					// The other corners stayed put, so the drag reached one point only.
					Object.keys(positionsBefore).forEach(function(name) {
						if (name === handleName) return;

						expect(positionsAfter[name], 'point ' + name).to.not.be.undefined;
						expect(positionsAfter[name][0], 'sideways place of point ' + name)
							.to.be.closeTo(positionsBefore[name][0], 2);
						expect(positionsAfter[name][1], 'downward place of point ' + name)
							.to.be.closeTo(positionsBefore[name][1], 2);
					});
				});
			});
		});
	});
});
