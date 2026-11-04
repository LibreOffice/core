/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The points a connector ties itself to, on a slide drawn from vector primitives: showing them,
// adding one, choosing the way a connector leaves it and taking it away. The slide holds a
// rectangle, which has no points of its own, and an ellipse, whose points come from its geometry.
describe(['tagdesktop'], 'Glue points of a shape on a vector slide', function() {

	beforeEach(function() {
		// The Shape tab then shows the glue points button in the tab itself, not in an overflow
		// group.
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/glue_points.fodp');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			// The stock load checks wait for tiles that a vector view lacks.
			cy.wrap(null).should(function() {
				expect(win.app.impress.partList).to.have.length.greaterThan(0);
			});
			return helper.processToIdle(win);
		}.bind(this));
		desktopHelper.switchUIToNotebookbar();
	});

	// The section that draws the handles of the one selected object, or undefined while no
	// single object is selected.
	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	// The id of the selected object, or undefined while none is selected.
	function selectedObject(win) {
		var section = selectionSection(win);
		return section ? section.selectedObjects()[0] : undefined;
	}

	// What the render geometry holds about the selected object.
	function selectedGeometry(win) {
		var geometry = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name);
		return geometry.constructor.objectOf(selectedObject(win));
	}

	// The glue points of the selected object, as the engine last reported them.
	function gluePoints(win) {
		var object = selectedGeometry(win);
		return (object && object.gluePoints) || [];
	}

	// The CSS pixel position on the document canvas of a place on the selected object, 0 and 0
	// being its upper left corner and 1 and 1 its lower right one.
	function onCanvas(win, x, y) {
		var transform = win.cool.Matrix2D.fromArray(selectedGeometry(win).transform);
		var twips = transform.apply(x, y);
		var visible = win.app.activeDocument.activeLayout.viewedRectangle;
		var scale = win.app.twipsToPixels / win.app.dpiScale;
		return {
			x: twips.x * scale - visible.pX1 / win.app.dpiScale,
			y: twips.y * scale - visible.pY1 / win.app.dpiScale,
		};
	}

	// A press and a release at a place on the selected object. The click before it has run out
	// first, so the two are not taken as a double click.
	function clickOnTheObject(win, x, y) {
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

	// Tab walks the objects of the slide in their order, so the test does not depend on where a
	// click lands. The rectangle comes first and the ellipse second.
	function selectByTab(win, count) {
		helper.typeIntoDocument('{esc}');
		var previous;
		for (var step = 0; step < count; ++step) {
			cy.then(function() {
				previous = selectedObject(win);
			});
			cy.realPress('Tab');
			cy.then(function() {
				return helper.processToIdle(win);
			});
			cy.wrap(null).should(function() {
				var now = selectedObject(win);
				expect(now, 'selected object').to.not.be.undefined;
				expect(now, 'selected object').to.not.equal(previous);
			});
		}
	}

	/*
		Opens the menu of the glue points button on the Shape tab. Its entries are Show and Add New.
		The button runs a command of the client, not a uno command, so it keeps the id it was
		declared with and carries no modelId.
	*/
	function openGluePointsMenu() {
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-gluepoints-button:visible').click();
		desktopHelper.getOpenMenuEntries().should('have.length', 2);
	}

	function showGluePoints(win) {
		openGluePointsMenu();
		desktopHelper.getOpenMenuEntries().eq(0).click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('showgluepoints', 'true');
	}

	function startAddingAPoint(win) {
		openGluePointsMenu();
		desktopHelper.getOpenMenuEntries().eq(1).should('not.have.attr', 'disabled');
		desktopHelper.getOpenMenuEntries().eq(1).click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('insertgluepoint', 'true');
	}

	// Opens the menu of the glue point at that place in the list of the selected object. Its
	// entries are Left, Right, Up and Down at 1 to 4, and Delete at 6.
	function openMenuOfPoint(win, at) {
		helper.waitForMapState('showgluepoints', 'true');
		cy.then(function() {
			var point = gluePoints(win)[at];
			clickOnTheObject(win, point.x, point.y);
		});
		cy.cGet('#tie-point-menu-entry-1').should('be.visible');
	}

	// Chooses an entry of the menu of a point and waits until the engine has answered.
	function chooseInPointMenu(win, entry) {
		cy.cGet('#tie-point-menu-entry-' + entry).click();
		cy.cGet('#tie-point-menu-entry-' + entry).should('not.exist');
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	function expectWayOut(win, wayOut) {
		cy.wrap(null).should(function() {
			var points = gluePoints(win);
			expect(points, 'glue points').to.have.length(1);
			expect(points[0].wayOut, 'way out').to.equal(wayOut);
		});
	}

	it('A point is added with one press, its way out is chosen and it is deleted.', function() {
		var win = this.win;
		selectByTab(win, 1);
		cy.wrap(null).should(function() {
			expect(gluePoints(win), 'points of the rectangle').to.have.length(0);
		});

		// Add New waits until the points are shown.
		openGluePointsMenu();
		desktopHelper.getOpenMenuEntries().eq(1).should('have.attr', 'disabled');
		desktopHelper.getOpenMenuEntries().eq(0).click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('showgluepoints', 'true');

		// Over the object the pointer is a cross while a point is being added. Escape stops
		// adding, the pointer is the object's again at once, and the points stay shown.
		// The canvas follows the mouse only once the mouse has entered it, so the move is a real
		// one, which the browser starts with an enter.
		startAddingAPoint(win);
		cy.then(function() {
			var place = onCanvas(win, 0.5, 0.5);
			cy.cGet('#document-canvas').realMouseMove(place.x, place.y);
		});
		cy.cGet('#document-canvas').should(function($canvas) {
			expect($canvas[0].style.cursor).to.equal('crosshair');
		});
		helper.typeIntoDocument('{esc}');
		helper.waitForMapState('insertgluepoint', 'false');
		cy.cGet('#document-canvas').should(function($canvas) {
			expect($canvas[0].style.cursor).to.not.equal('crosshair');
		});
		helper.waitForMapState('showgluepoints', 'true');

		// Once adding has stopped, a press on the object adds nothing.
		clickOnTheObject(win, 0.5, 0.5);
		cy.wrap(null).should(function() {
			expect(gluePoints(win), 'points after Escape').to.have.length(0);
		});

		// A press adds one point where it lands, and adding ends with it.
		startAddingAPoint(win);
		clickOnTheObject(win, 0.25, 0.25);
		cy.wrap(null).should(function() {
			var points = gluePoints(win);
			expect(points, 'points after the press').to.have.length(1);
			expect(points[0].x, 'across the object').to.be.closeTo(0.25, 0.01);
			expect(points[0].y, 'down the object').to.be.closeTo(0.25, 0.01);
			expect(points[0].fromTheShape, 'added by the user').to.not.equal(true);
			expect(points[0].wayOut, 'way out').to.be.undefined;
		});
		helper.waitForMapState('insertgluepoint', 'false');

		// The next press adds nothing more.
		clickOnTheObject(win, 0.75, 0.75);
		cy.wrap(null).should(function() {
			expect(gluePoints(win), 'points after the second press').to.have.length(1);
		});

		// Left, then Right beside it makes the pair that faces each other.
		openMenuOfPoint(win, 0);
		chooseInPointMenu(win, 1);
		expectWayOut(win, 'left');

		openMenuOfPoint(win, 0);
		cy.cGet('#tie-point-menu-entry-1').should('have.class', 'checked');
		cy.cGet('#tie-point-menu-entry-2').should('not.have.class', 'checked');
		chooseInPointMenu(win, 2);
		expectWayOut(win, 'horizontal');

		// Up on a point that leaves left and right turns it to leave upwards only.
		openMenuOfPoint(win, 0);
		cy.cGet('#tie-point-menu-entry-1').should('have.class', 'checked');
		cy.cGet('#tie-point-menu-entry-2').should('have.class', 'checked');
		chooseInPointMenu(win, 3);
		expectWayOut(win, 'top');

		// Up again takes the last way away, and the connector chooses its own way.
		openMenuOfPoint(win, 0);
		cy.cGet('#tie-point-menu-entry-3').should('have.class', 'checked');
		chooseInPointMenu(win, 3);
		expectWayOut(win, undefined);

		openMenuOfPoint(win, 0);
		chooseInPointMenu(win, 6);
		cy.wrap(null).should(function() {
			expect(gluePoints(win), 'points after Delete').to.have.length(0);
		});
	});

	it('A point the shape makes from its geometry offers no change.', function() {
		var win = this.win;
		selectByTab(win, 2);
		showGluePoints(win);

		cy.wrap(null).should(function() {
			var points = gluePoints(win);
			expect(points, 'points of the ellipse').to.have.length(8);
			points.forEach(function(point, at) {
				expect(point.fromTheShape, 'point ' + at + ' from the shape').to.equal(true);
			});
		});

		openMenuOfPoint(win, 0);
		[1, 2, 3, 4, 6].forEach(function(entry) {
			cy.cGet('#tie-point-menu-entry-' + entry).should('have.attr', 'disabled');
		});
	});
});
