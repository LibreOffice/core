/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Editing the points of a polygon on a vector slide. For a polygon, the Shape tab shows Edit Points
// instead of the convert button. The points start hidden, and the menu of a point changes the
// path. The slide has a rectangle, which has no path, and a polygon of four points.
describe(['tagdesktop'], 'Points of a polygon on a vector slide', function() {

	beforeEach(function() {
		// The window is wide enough for the Shape tab to show all its buttons.
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
		desktopHelper.switchUIToNotebookbar();
	});

	// The section that draws the handles of the selected object, or undefined when not exactly
	// one object is selected.
	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	// The id of the selected object, or undefined when none is selected.
	function selectedObject(win) {
		var section = selectionSection(win);
		return section ? section.selectedObjects()[0] : undefined;
	}

	// The points of the first polygon of the selected object, in twips, as last reported by the
	// engine.
	function pathPoints(win) {
		var geometry = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name);
		var object = geometry.constructor.objectOf(selectedObject(win));
		return (object && object.path && object.path[0].points) || [];
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

	// Clicks at a place on the page, in twips. It first waits for the previous click to expire,
	// so the two are not taken as a double click.
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

	// Selects an object with Tab, so the test does not depend on where a click lands. The
	// rectangle comes first and the polygon second.
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

	// Waits until the eight handles that frame the selection are drawn, or until they are not.
	function framingHandlesShown(win, shown) {
		cy.wrap(null).should(function() {
			var section = selectionSection(win);
			var framing = (section ? section.handles() : []).filter(function(handle) {
				return handle.group === 'framing';
			});
			expect(framing.length > 0, 'the handles that frame the selection are drawn')
				.to.equal(shown);
		});
	}

	// Clicks Edit Points and waits for its state. The button runs a command of the client.
	function switchThePoints(win, shown) {
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-edit-points .unobutton:visible').click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('editpathpoints', shown ? 'true' : 'false');
	}

	/*
		Opens the menu of the point with that index. The entries are: Delete at 1, To Curve or To
		Line at 3, Corner Point, Smooth Transition and Symmetric Transition at 5 to 7, and Split
		Path at 9.
	*/
	function openMenuOfPoint(win, at) {
		helper.waitForMapState('editpathpoints', 'true');
		cy.then(function() {
			var point = pathPoints(win)[at];
			clickAt(win, point.x, point.y);
		});
		cy.cGet('#path-point-menu-entry-1').should('be.visible');
	}

	// Chooses an entry of the point menu and waits for the engine to answer.
	function chooseInPointMenu(win, entry) {
		cy.cGet('#path-point-menu-entry-' + entry).click();
		cy.cGet('#path-point-menu-entry-' + entry).should('not.exist');
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('The Shape tab offers to edit the points of a polygon, which start hidden.', function() {
		var win = this.win;

		// A rectangle gets the convert button. The Shape tab exists only while a shape is
		// selected.
		selectByTab(win, 1);
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container .unoConvertToEditableShape').should('be.visible');
		cy.cGet('#Shape-container #shape-edit-points').should('not.be.visible');

		// A polygon gets Edit Points instead. Its points start hidden and the frame is shown.
		selectByTab(win, 2);
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-edit-points').should('be.visible');
		cy.cGet('#Shape-container .unoConvertToEditableShape').should('not.be.visible');
		// The state is sent only when it changes, so it is not set before the points are first
		// shown.
		cy.wrap(null).should(function() {
			expect(win.app.map['stateChangeHandler'].getItemValue('editpathpoints'))
				.to.not.equal('true');
		});
		framingHandlesShown(win, true);

		// While the points are shown, the frame handles are hidden.
		switchThePoints(win, true);
		framingHandlesShown(win, false);

		switchThePoints(win, false);
		framingHandlesShown(win, true);

		// Selecting the rectangle again brings back the convert button.
		selectByTab(win, 1);
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container .unoConvertToEditableShape').should('be.visible');
		cy.cGet('#Shape-container #shape-edit-points').should('not.be.visible');
	});

	it('The menu of a point changes the path, and a point is added with one press.', function() {
		var win = this.win;
		selectByTab(win, 2);
		cy.wrap(null).should(function() {
			expect(pathPoints(win), 'points of the polygon').to.have.length(4);
		});
		switchThePoints(win, true);

		// The polygon has straight edges, so the menu offers To Curve. The point has no weights,
		// so Smooth and Symmetric are disabled.
		openMenuOfPoint(win, 0);
		cy.cGet('#path-point-menu-entry-3').should('contain.text', 'To Curve');
		cy.cGet('#path-point-menu-entry-5').should('have.attr', 'disabled');
		cy.cGet('#path-point-menu-entry-6').should('have.attr', 'disabled');

		// The edge after the first point becomes a curve, so the point gets a weight.
		chooseInPointMenu(win, 3);
		cy.wrap(null).should(function() {
			var points = pathPoints(win);
			expect(points, 'points after To Curve').to.have.length(4);
			expect(points[0].aheadX, 'weight leaving the first point').to.not.be.undefined;
		});

		// Delete removes the point, and the polygon stays selected.
		openMenuOfPoint(win, 1);
		chooseInPointMenu(win, 1);
		cy.wrap(null).should(function() {
			expect(selectedObject(win), 'the polygon').to.not.be.undefined;
			expect(pathPoints(win), 'points after Delete').to.have.length(3);
		});

		// Add Point waits for one press and adds a point on the nearest edge. The press is a little
		// inside the left edge, so it lands on the object.
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-edit-points .arrowbackground:visible').click();
		desktopHelper.getOpenMenuEntries().eq(0).click();
		helper.waitForMapState('insertpathpoint', 'true');
		cy.then(function() {
			var points = pathPoints(win);
			var last = points[points.length - 1];
			clickAt(win, (last.x + points[0].x) / 2 + 300, (last.y + points[0].y) / 2);
		});
		cy.wrap(null).should(function() {
			expect(pathPoints(win), 'points after Add Point').to.have.length(4);
		});
		helper.waitForMapState('insertpathpoint', 'false');
	});
});
