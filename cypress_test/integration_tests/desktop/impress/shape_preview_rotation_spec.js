/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Picture of a shape that is turned.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/shape_preview_rotation.fodp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	/*
		This gives the box the picture of the selection covers, in the core pixels the handles are
		placed in. The outer element carries the place, and the inner one carries the size as soon
		as the picture names a viewBox.
	*/
	function pictureBox(win, outer) {
		var inner = outer.querySelector('svg');
		var width = parseInt(inner && inner.style.width) || parseInt(outer.style.width);
		var height = parseInt(inner && inner.style.height) || parseInt(outer.style.height);
		var left = parseInt(outer.style.left);
		var top = parseInt(outer.style.top);

		return {
			width: width * win.app.dpiScale,
			height: height * win.app.dpiScale,
			centerX: (left + width / 2) * win.app.dpiScale,
			centerY: (top + height / 2) * win.app.dpiScale
		};
	}

	/*
		The document holds a circle turned by 45 degrees. A circle keeps its round shape when it
		turns, so it covers the same ground as before, while the frame the handles sit on grows
		by about two fifths in each direction. The picture the browser shows while the shape is
		dragged has to stay the size of the circle.
	*/
	it('The picture shown while a turned shape is dragged is the size of the shape.', function() {
		var dragX = 60;
		var dragY = 40;

		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			return helper.processToIdle(win);
		});

		cy.cGet('#canvas-container > svg > svg').should('exist');

		cy.cGet('#document-canvas').then(function($canvas) {
			var section = getHandlesSection(this.win);
			var shape = section.sectionProperties.shapeRectangleProperties[0];
			expect(shape, 'geometry of the selected shape').to.not.be.undefined;

			// The circle is turned, so the frame around it is a good deal wider than it is.
			expect(section.size[0], 'width of the frame around the selection')
				.to.be.greaterThan(shape.width * 1.25);

			this.shape = shape;
			this.frameCenter = [
				section.myTopLeft[0] + section.size[0] / 2,
				section.myTopLeft[1] + section.size[1] / 2
			];

			var canvas = $canvas[0].getBoundingClientRect();
			this.start = {
				x: canvas.left + this.frameCenter[0] / this.win.app.dpiScale,
				y: canvas.top + this.frameCenter[1] / this.win.app.dpiScale
			};
		}.bind(this));

		// Dragging the shape is what brings the picture up.
		cy.then(function() {
			cy.cGet('body').realMouseDown({ x: this.start.x, y: this.start.y });
			cy.cGet('body').realMouseMove(this.start.x + dragX / 2, this.start.y + dragY / 2);
			cy.cGet('body').realMouseMove(this.start.x + dragX, this.start.y + dragY);
		}.bind(this));

		cy.cGet('#canvas-container > svg').should(function($outer) {
			expect($outer[0].style.display, 'the picture is shown').to.not.equal('none');
		});

		cy.cGet('#canvas-container > svg').then(function($outer) {
			var box = pictureBox(this.win, $outer[0]);
			var tolerance = Math.max(8, this.shape.width * 0.05);

			expect(box.width, 'width of the picture').to.be.closeTo(this.shape.width, tolerance);
			expect(box.height, 'height of the picture').to.be.closeTo(this.shape.height, tolerance);

			// The picture follows the pointer, so it sits a drag away from the shape.
			expect(box.centerX, 'where the picture sits across the page').to.be.closeTo(
				this.frameCenter[0] + dragX * this.win.app.dpiScale, tolerance);
			expect(box.centerY, 'where the picture sits down the page').to.be.closeTo(
				this.frameCenter[1] + dragY * this.win.app.dpiScale, tolerance);
		}.bind(this));

		// Drop the shape back where it started, so the document ends as it began.
		cy.then(function() {
			cy.cGet('body').realMouseMove(this.start.x, this.start.y);
			cy.cGet('body').realMouseUp({ x: this.start.x, y: this.start.y });
		}.bind(this));
	});
});
