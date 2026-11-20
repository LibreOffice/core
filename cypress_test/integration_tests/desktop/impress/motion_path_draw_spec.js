/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// Drawing a motion path for an object on a vector slide, as the animation sidebar asks for one.
// The slide has a rectangle and a polygon, neither of them animated.
describe(['tagdesktop'], 'Drawing a motion path from the animation sidebar', function() {

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

	// The first object of the slide that has no motion path.
	function objectWithoutPath(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (!found && object.id && !(object.motionPaths && object.motionPaths.length))
				found = object.id;
		});
		return found;
	}

	function pathCount(win, objectId) {
		return (geometry(win).objectOf(objectId).motionPaths || []).length;
	}

	// The middle of that object on the page, in twips.
	function middleOf(win, objectId) {
		var object = geometry(win).objectOf(objectId);
		return { x: object.x + object.width / 2, y: object.y + object.height / 2 };
	}

	// The mode one is in, or null. The classes of the page are names of its script, not
	// properties of its window, so the page itself is asked.
	function modeOn(win) {
		return win.eval('Interaction.modeOn()');
	}

	function selected(win) {
		return win.eval('GraphicSelection').selectedObjectIDs;
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

	// Clicks at that place in twips on the page, without moving.
	function clickAt(win, x, y) {
		cy.then(function() {
			var place = onCanvas(win, x, y);
			cy.cGet('#document-canvas')
				.trigger('mousedown', place.x, place.y, { button: 0, buttons: 1 })
				.trigger('mouseup', place.x, place.y, { button: 0, buttons: 0 });
		});
	}

	// Draws a polygon of three points, the first at the middle of the object and the others
	// that far away from it in twips; a double click on the last point ends it.
	function drawPolygon(win, from, across, down) {
		clickAt(win, from.x, from.y);
		clickAt(win, from.x + across, from.y);
		cy.then(function() {
			var place = onCanvas(win, from.x + across, from.y + down);
			cy.cGet('#document-canvas').dblclick(place.x, place.y);
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A polygon drawn after Add makes an effect, the object stays selected, and Add draws another one.', function() {
		var win = this.win;
		var objectId;
		var before;
		var middle;
		cy.wrap(null).should(function() {
			objectId = objectWithoutPath(win);
			expect(objectId, 'an object without a path').to.not.equal(undefined);
		});
		cy.then(function() {
			before = pathCount(win, objectId);
			middle = middleOf(win, objectId);
			win.eval('GraphicSelection').selectObjects([objectId]);
			win.app.map.sendUnoCommand('.uno:SidebarDeck.SdCustomAnimationDeck');
		});
		cy.cGet('#add_effect-button').should('be.visible');

		// Add puts in an effect of the category shown, which the user path then replaces
		cy.cGet('#add_effect-button').click();
		cy.cGet('#categorylb-input').select('Motion Paths');
		cy.cGet('#effect_list').contains(/^\s*Polygon\s*$/).click();

		// the sidebar's choice of a user path lets go of the object, as the office does, and the
		// path is drawn on the slide
		cy.wrap(null).should(function() {
			var mode = modeOn(win);
			expect(mode && mode.constructor.name, 'the mode').to.equal('ClassicMotionPathCreateContext');
			expect(selected(win), 'the selection while drawing').to.have.length(0);
		});
		cy.then(function() {
			drawPolygon(win, middle, 1500, 1500);
		});

		cy.wrap(null).should(function() {
			expect(!!modeOn(win), 'the mode after the path').to.equal(false);
			expect(pathCount(win, objectId), 'the paths').to.equal(before + 1);
			var drawn = geometry(win).objectOf(objectId).motionPaths[before].path[0];
			expect(drawn.points, 'the points drawn').to.have.length(3);
			expect(!!drawn.closed, 'a closed path').to.equal(false);
			expect(selected(win), 'the selection after the path').to.deep.equal([objectId]);
		});
		// the object's one effect is the polygon, so the sidebar offers it, and Add draws one more
		cy.cGet('#categorylb-input').should('have.value', '3');

		cy.cGet('#add_effect-button').click();
		cy.wrap(null).should(function() {
			var mode = modeOn(win);
			expect(mode && mode.constructor.name, 'the mode').to.equal('ClassicMotionPathCreateContext');
		});
		cy.then(function() {
			drawPolygon(win, middle, -1500, 1500);
		});
		cy.wrap(null).should(function() {
			expect(pathCount(win, objectId), 'the paths').to.equal(before + 2);
		});

		// one undo takes away the last path alone
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:Undo');
		});
		cy.wrap(null).should(function() {
			expect(pathCount(win, objectId), 'the paths after the undo').to.equal(before + 1);
		});
	});

	it('A click without a drag on a line drawn by hand makes nothing.', function() {
		var win = this.win;
		var objectId;
		var before;
		var middle;
		cy.wrap(null).should(function() {
			objectId = objectWithoutPath(win);
			expect(objectId, 'an object without a path').to.not.equal(undefined);
		});
		cy.then(function() {
			before = pathCount(win, objectId);
			middle = middleOf(win, objectId);
			win.eval('GraphicSelection').selectObjects([objectId]);
			win.app.map.sendUnoCommand('.uno:SidebarDeck.SdCustomAnimationDeck');
		});
		cy.cGet('#add_effect-button').should('be.visible');
		cy.cGet('#add_effect-button').click();
		cy.cGet('#categorylb-input').select('Motion Paths');
		cy.cGet('#effect_list').contains(/^\s*Freeform Line\s*$/).click();
		cy.wrap(null).should(function() {
			var mode = modeOn(win);
			expect(mode && mode.constructor.name, 'the mode').to.equal('ClassicMotionPathCreateContext');
		});

		cy.then(function() {
			clickAt(win, middle.x + 1500, middle.y + 1500);
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(!!modeOn(win), 'the mode after the click').to.equal(false);
			// the effect Add put in was taken away by the choice of the user path
			expect(pathCount(win, objectId), 'the paths').to.equal(before);
		});
	});
});
