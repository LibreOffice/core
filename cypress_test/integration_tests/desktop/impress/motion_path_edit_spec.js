/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Editing the motion paths of an object on a vector slide by their handles. The slide holds a
// rectangle with a line of text that moves along a curve whose middle point is symmetric, a square that moves along a
// closed triangle, and a rectangle whose path was drawn beside it.
describe(['tagdesktop'], 'Editing a motion path on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/motion_path_edit.fodp');
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

	// Whether the first motion path of that object starts at the middle of the object.
	function startsAtTheObject(object) {
		var start = object.motionPaths[0].path[0].points[0];
		return Math.abs(start.x) < 10 && Math.abs(start.y) < 10;
	}

	// The object of the slide whose first motion path starts at the object and is closed, or is
	// not.
	function objectWithPath(win, closed) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			var path = object.motionPaths && object.motionPaths[0].path[0];
			if (!found && path && !!path.closed === closed && startsAtTheObject(object))
				found = object.id;
		});
		return found;
	}

	// The object of the slide whose first motion path starts away from the object.
	function objectWithPathBeside(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (!found && object.motionPaths && !startsAtTheObject(object))
				found = object.id;
		});
		return found;
	}

	// The points of the first motion path of that object, as the client holds them.
	function pointsOf(win, objectId) {
		return geometry(win).objectOf(objectId).motionPaths[0].path[0].points;
	}

	// The handles of the motion paths of the selection, by name.
	function motionHandles(win) {
		var section = win.app.sectionContainer.sections.find(function(one) {
			return one.name.indexOf('selected-object-') === 0;
		});
		var handles = {};
		(section ? section.handles() : []).forEach(function(handle) {
			if (handle.name.indexOf('motion ') === 0)
				handles[handle.name] = handle.point;
		});
		return handles;
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

	// Selects the object that answers for and waits for the handles of its motion paths. The id
	// is asked for when the step runs, as it is found by a step before.
	function select(win, objectId) {
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([objectId()]);
		});
		cy.wrap(null).should(function() {
			expect(Object.keys(motionHandles(win)), 'the handles').to.have.length.greaterThan(0);
		});
	}

	// Drags the handle of that name by that many twips across and down, in steps.
	function dragHandle(win, name, across, down) {
		cy.then(function() {
			var from = motionHandles(win)[name];
			expect(from, 'the handle ' + name).to.not.equal(undefined);
			dragFrom(win, from, across, down);
		});
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

	// Clicks a place on the page, in twips. It first waits for the click before to expire, so
	// the two are not taken as a double click.
	function clickAt(win, x, y) {
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			var place = onCanvas(win, x, y);
			cy.cGet('#document-canvas').click(place.x, place.y);
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	// Chooses an entry of an open menu and waits for the engine to answer.
	function choose(menuId, entry) {
		cy.cGet('#' + menuId + '-entry-' + entry).click();
		cy.cGet('#' + menuId + '-entry-' + entry).should('not.exist');
	}

	function undo(win) {
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:Undo');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A point and a weight of a curve are moved, and the engine keeps what was moved.', function() {
		var win = this.win;
		var curved;
		cy.wrap(null).should(function() {
			curved = objectWithPath(win, false);
			expect(curved, 'the rectangle on a curve').to.not.equal(undefined);
		});
		select(win, function() { return curved; });

		// the point the path starts at has a handle as every other point, and so has its weight
		cy.then(function() {
			var handles = motionHandles(win);
			expect(handles, 'the handles').to.have.property('motion 0 9.0.0');
			expect(handles, 'the handles').to.have.property('motion 0 10.0.0.out');
			expect(handles, 'the handles').to.have.property('motion 0 9.0.2');
		});

		// the end point goes where it was dragged
		var before;
		cy.then(function() {
			before = pointsOf(win, curved)[2];
		});
		dragHandle(win, 'motion 0 9.0.2', 600, 400);
		cy.wrap(null).should(function() {
			var end = pointsOf(win, curved)[2];
			expect(Math.abs(end.x - before.x - 600), 'the end across').to.be.below(60);
			expect(Math.abs(end.y - before.y - 400), 'the end down').to.be.below(60);
		});

		// an undo brings back what the engine had, so the engine had taken the change
		undo(win);
		cy.wrap(null).should(function() {
			var end = pointsOf(win, curved)[2];
			expect([end.x, end.y], 'the end after the undo').to.deep.equal([before.x, before.y]);
		});

		// the weight coming into the symmetric middle point swings the other one with it
		dragHandle(win, 'motion 0 10.0.1.in', 0, -400);
		cy.wrap(null).should(function() {
			var middle = pointsOf(win, curved)[1];
			expect(middle.continuity, 'how the curve runs through the point').to.equal(2);
			expect(Math.abs(middle.aheadX - (2 * middle.x - middle.behindX)), 'mirrored across')
				.to.be.below(3);
			expect(Math.abs(middle.aheadY - (2 * middle.y - middle.behindY)), 'mirrored down')
				.to.be.below(3);
		});
	});

	it('A point of a closed path is moved and the path stays closed.', function() {
		var win = this.win;
		var closed;
		cy.wrap(null).should(function() {
			closed = objectWithPath(win, true);
			expect(closed, 'the square on a triangle').to.not.equal(undefined);
		});
		select(win, function() { return closed; });

		var before;
		cy.then(function() {
			before = pointsOf(win, closed)[1];
		});
		dragHandle(win, 'motion 0 9.0.1', 500, 0);
		cy.wrap(null).should(function() {
			var path = geometry(win).objectOf(closed).motionPaths[0].path[0];
			expect(path.closed, 'the path closed').to.equal(true);
			expect(path.points, 'the points').to.have.length(3);
			expect(Math.abs(path.points[1].x - before.x - 500), 'the point across').to.be.below(60);
		});
	});

	it('The start of a path drawn beside its object is moved, and the rest of the path stays.', function() {
		var win = this.win;
		var beside;
		cy.wrap(null).should(function() {
			beside = objectWithPathBeside(win);
			expect(beside, 'the rectangle with a path beside it').to.not.equal(undefined);
		});
		select(win, function() { return beside; });

		var before;
		cy.then(function() {
			before = pointsOf(win, beside).map(function(point) { return [point.x, point.y]; });
		});
		dragHandle(win, 'motion 0 9.0.0', -500, 300);
		cy.wrap(null).should(function() {
			var points = pointsOf(win, beside);
			expect(Math.abs(points[0].x - before[0][0] + 500), 'the start across').to.be.below(60);
			expect(Math.abs(points[0].y - before[0][1] - 300), 'the start down').to.be.below(60);
			expect([points[1].x, points[1].y], 'the end').to.deep.equal(before[1]);
		});

		// an undo brings back what the engine had, so the engine had taken the change
		undo(win);
		cy.wrap(null).should(function() {
			var points = pointsOf(win, beside).map(function(point) { return [point.x, point.y]; });
			expect(points, 'the points after the undo').to.deep.equal(before);
		});
	});

	it('A drag on the line of a path moves the whole path, and the object stays.', function() {
		var win = this.win;
		var beside;
		cy.wrap(null).should(function() {
			beside = objectWithPathBeside(win);
			expect(beside, 'the rectangle with a path beside it').to.not.equal(undefined);
		});
		select(win, function() { return beside; });

		// the press is in the middle of the straight first edge, away from every handle
		var before;
		var box;
		cy.then(function() {
			before = pointsOf(win, beside).map(function(point) { return [point.x, point.y]; });
			box = geometry(win).boxOf(beside);
			var handles = motionHandles(win);
			var first = handles['motion 0 9.0.0'];
			var second = handles['motion 0 9.0.1'];
			dragFrom(win, { x: (first.x + second.x) / 2, y: (first.y + second.y) / 2 }, -400, -300);
		});
		cy.wrap(null).should(function() {
			var points = pointsOf(win, beside);
			points.forEach(function(point, at) {
				expect(Math.abs(point.x - before[at][0] + 400), 'point ' + at + ' across')
					.to.be.below(60);
				expect(Math.abs(point.y - before[at][1] + 300), 'point ' + at + ' down')
					.to.be.below(60);
			});
			expect(geometry(win).boxOf(beside), 'the box of the object').to.deep.equal(box);
		});

		undo(win);
		cy.wrap(null).should(function() {
			var points = pointsOf(win, beside).map(function(point) { return [point.x, point.y]; });
			expect(points, 'the points after the undo').to.deep.equal(before);
		});
	});

	it('A click on a point opens its menu, whose Corner Point lays the weights on the edges.', function() {
		var win = this.win;
		var curved;
		cy.wrap(null).should(function() {
			curved = objectWithPath(win, false);
			expect(curved, 'the rectangle on a curve').to.not.equal(undefined);
		});
		select(win, function() { return curved; });

		// the entries are Delete at 1, To Curve or To Line at 3, Corner Point, Smooth Transition
		// and Symmetric Transition at 5 to 7, and Split Path at 9
		cy.then(function() {
			var middle = motionHandles(win)['motion 0 9.0.1'];
			clickAt(win, middle.x, middle.y);
		});
		cy.cGet('#motion-path-point-menu-entry-7').should('have.attr', 'disabled');
		choose('motion-path-point-menu', 5);
		cy.then(function() {
			return helper.processToIdle(win);
		});
		// the neighbours of the point lie on one line with it, so the weights come to lie on it too
		cy.wrap(null).should(function() {
			var middle = pointsOf(win, curved)[1];
			expect([middle.behindY, middle.aheadY], 'the weights on the line').to.deep.equal([0, 0]);
		});

		undo(win);
		cy.wrap(null).should(function() {
			var middle = pointsOf(win, curved)[1];
			expect(middle.behindY, 'the weight coming in after the undo').to.be.below(-100);
			expect(middle.continuity, 'symmetric as before').to.equal(2);
		});
	});

	it('A click on the line of a path opens its menu, which adds a point there.', function() {
		var win = this.win;
		var beside;
		cy.wrap(null).should(function() {
			beside = objectWithPathBeside(win);
			expect(beside, 'the rectangle with a path beside it').to.not.equal(undefined);
		});
		select(win, function() { return beside; });

		cy.then(function() {
			var handles = motionHandles(win);
			var first = handles['motion 0 9.0.0'];
			var second = handles['motion 0 9.0.1'];
			clickAt(win, (first.x + second.x) / 2, (first.y + second.y) / 2);
		});
		choose('motion-path-menu', 1);
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(pointsOf(win, beside), 'the points with the one added').to.have.length(3);
		});

		undo(win);
		cy.wrap(null).should(function() {
			expect(pointsOf(win, beside), 'the points after the undo').to.have.length(2);
			expect(win.app.map.stateChangeHandler.getItemValue('.uno:Undo'), 'undo')
				.to.equal('disabled');
		});

		// Simplify Curves leaves a straight path as it is, so it leaves no step to undo either
		cy.then(function() {
			var handles = motionHandles(win);
			var first = handles['motion 0 9.0.0'];
			var second = handles['motion 0 9.0.1'];
			clickAt(win, (first.x + second.x) / 2, (first.y + second.y) / 2);
		});
		choose('motion-path-menu', 4);
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(pointsOf(win, beside), 'the points after Simplify Curves').to.have.length(2);
			expect(win.app.map.stateChangeHandler.getItemValue('.uno:Undo'), 'undo')
				.to.equal('disabled');
		});
	});

	it('A click where the path runs over the text of its object edits the text.', function() {
		var win = this.win;
		var curved;
		cy.wrap(null).should(function() {
			curved = objectWithPath(win, false);
			expect(curved, 'the rectangle on a curve').to.not.equal(undefined);
		});
		select(win, function() { return curved; });

		/*
			A place on the line of the path where the hit test finds the text of the object, or
			finds no text, away from every handle of the path. Where the text lies is the shape's
			own layout, so it is asked for along the path rather than assumed.
		*/
		var placeOnThePath = function(onTheText) {
			var object = geometry(win).objectOf(curved);
			var points = win.cool.MotionPaths.placedOf(object)[0].polygons[0].points;
			var handles = Object.values(motionHandles(win));
			for (var edge = 0; edge + 1 < points.length; ++edge) {
				var from = points[edge];
				var to = points[edge + 1];
				var out = { x: from.aheadX === undefined ? from.x : from.aheadX,
					y: from.aheadY === undefined ? from.y : from.aheadY };
				var into = { x: to.behindX === undefined ? to.x : to.behindX,
					y: to.behindY === undefined ? to.y : to.behindY };
				for (var t = 0.02; t < 1; t += 0.02) {
					var u = 1 - t;
					var x = u * u * u * from.x + 3 * u * u * t * out.x + 3 * u * t * t * into.x + t * t * t * to.x;
					var y = u * u * u * from.y + 3 * u * u * t * out.y + 3 * u * t * t * into.y + t * t * t * to.y;
					var nearAHandle = handles.some(function(handle) {
						return Math.abs(handle.x - x) < 300 && Math.abs(handle.y - y) < 300;
					});
					if (!nearAHandle && (geometry(win).pointerAt(x, y) === 'text') === onTheText)
						return { x: x, y: y };
				}
			}
			return null;
		};

		var onTheText;
		cy.then(function() {
			onTheText = placeOnThePath(true);
			expect(onTheText, 'a place where the path runs over the text').to.not.equal(null);
			clickAt(win, onTheText.x, onTheText.y);
		});
		cy.wrap(null).should(function() {
			expect(win.app.file.textCursor.visible, 'the text edited').to.equal(true);
		});
		cy.cGet('#motion-path-menu-entry-1').should('not.exist');

		// beside the text the line opens the menu of the path, as before
		helper.typeIntoDocument('{esc}');
		cy.wrap(null).should(function() {
			expect(win.app.file.textCursor.visible, 'the text left').to.equal(false);
		});
		select(win, function() { return curved; });
		cy.then(function() {
			var beside = placeOnThePath(false);
			expect(beside, 'a place on the path beside the text').to.not.equal(null);
			clickAt(win, beside.x, beside.y);
		});
		cy.cGet('#motion-path-menu-entry-1').should('be.visible');
	});
});
