/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Picture of a shape that is being turned.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/shape_preview_rotation.fodg');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));

		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:SelectAll');
			return helper.processToIdle(win);
		});

		cy.cGet('#canvas-container > svg > svg').should('exist');

		cy.cGet('#document-canvas').then(function($canvas) {
			this.canvas = $canvas[0].getBoundingClientRect();
		}.bind(this));
	});

	function getHandlesSection(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.ShapeHandlesSection.name);
	}

	function getShape(win) {
		return getHandlesSection(win).sectionProperties.shapeRectangleProperties[0];
	}

	// This gives the middle of the shape on the screen, from the shape the handles carry.
	function shapeCenter(win, canvas) {
		var shape = getShape(win);
		return {
			x: canvas.left + shape.center.pX / win.app.dpiScale,
			y: canvas.top + shape.center.pY / win.app.dpiScale,
			angle: (shape.angleRadian * 180) / Math.PI,
			width: shape.width,
			height: shape.height
		};
	}

	/*
		This gives the middle of the picture the browser draws while a handle is dragged. The
		picture turns and scales about its own middle, so this is the middle the shape heads for.
	*/
	function previewCenter(win) {
		var inner = win.document.querySelector('#canvas-container > svg > svg');
		var box = inner.getBoundingClientRect();
		return { x: box.left + box.width / 2, y: box.top + box.height / 2 };
	}

	// This gives the point the picture turns about, in the pixels the page is drawn in.
	function picturePivot(win, outer) {
		var origin = win.getComputedStyle(outer).transformOrigin.split(' ');

		return [
			(parseFloat(outer.style.left) + parseFloat(origin[0])) * win.app.dpiScale,
			(parseFloat(outer.style.top) + parseFloat(origin[1])) * win.app.dpiScale
		];
	}

	// Takes the rotation handle round the middle of the shape by the given angle, and drops it.
	function turnBy(self, degrees) {
		cy.then(function() {
			self.center = shapeCenter(self.win, self.canvas);
		});

		cy.cGet('#test-div-shape-handle-rotation').then(function($handle) {
			var rect = $handle[0].getBoundingClientRect();
			var start = { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
			var angle = (degrees * Math.PI) / 180;
			var across = start.x - self.center.x;
			var down = start.y - self.center.y;

			self.handleStart = start;
			self.handleEnd = {
				x: self.center.x + across * Math.cos(angle) - down * Math.sin(angle),
				y: self.center.y + across * Math.sin(angle) + down * Math.cos(angle)
			};
		});

		cy.then(function() {
			cy.cGet('body').realMouseMove(self.handleStart.x, self.handleStart.y);
			cy.cGet('body').realMouseDown({ x: self.handleStart.x, y: self.handleStart.y });
			cy.cGet('body').realMouseMove(
				(self.handleStart.x + self.handleEnd.x) / 2,
				(self.handleStart.y + self.handleEnd.y) / 2);
			cy.cGet('body').realMouseMove(self.handleEnd.x, self.handleEnd.y);
		});

		cy.cGet('#canvas-container > svg').should(function($outer) {
			expect($outer[0].style.transform, 'the picture is turned').to.contain('rotate');
		});

		cy.then(function() {
			self.preview = previewCenter(self.win);
			self.pivot = picturePivot(self.win, self.win.document.querySelector('#canvas-container > svg'));
			cy.cGet('body').realMouseUp({ x: self.handleEnd.x, y: self.handleEnd.y });
		});

		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.cGet('#test-div-shape-handle-rotation').should('exist');
	}

	/*
		A shape stays where it is while it turns, because it turns about the middle of its own
		selection, whatever margin the page has. The picture turns about that same point, so the
		shape lands where the picture showed it.
	*/
	it('The shape lands where the turned picture showed it.', function() {
		turnBy(this, 180);

		cy.then(function() {
			var now = shapeCenter(this.win, this.canvas);

			expect(this.pivot[0] / this.win.app.dpiScale + this.canvas.left,
				'the picture turns about the middle of the shape, across')
				.to.be.closeTo(this.center.x, 2);
			expect(this.pivot[1] / this.win.app.dpiScale + this.canvas.top,
				'the picture turns about the middle of the shape, down')
				.to.be.closeTo(this.center.y, 2);

			expect(Math.abs(now.angle - this.center.angle),
				'how far the shape turned, in degrees').to.be.closeTo(180, 10);
			expect(now.x, 'where the shape sits across the page').to.be.closeTo(this.center.x, 2);
			expect(now.y, 'where the shape sits down the page').to.be.closeTo(this.center.y, 2);
		}.bind(this));
	});

	/*
		Every turn works from the shape as it stands. A shape that is already turned keeps its own
		middle, and the turn adds to the angle it stands at.
	*/
	it('A shape that is turned again stays where it is.', function() {
		var turns = [40, 40, 40];

		turns.forEach(function(degrees) {
			turnBy(this, degrees);

			cy.then(function() {
				var now = shapeCenter(this.win, this.canvas);
				var turned = Math.abs(now.angle - this.center.angle) % 360;
				if (turned > 180) turned = 360 - turned;

				expect(this.preview.x, 'the middle of the picture, across')
					.to.be.closeTo(this.center.x, 3);
				expect(this.preview.y, 'the middle of the picture, down')
					.to.be.closeTo(this.center.y, 3);

				expect(turned, 'how far this turn took the shape, in degrees')
					.to.be.closeTo(degrees, 10);
				expect(now.x, 'where the shape sits across the page')
					.to.be.closeTo(this.center.x, 2);
				expect(now.y, 'where the shape sits down the page')
					.to.be.closeTo(this.center.y, 2);
			}.bind(this));
		}.bind(this));
	});
});
