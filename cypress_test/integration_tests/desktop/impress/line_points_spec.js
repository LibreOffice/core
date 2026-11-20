/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The points of a line and of a polyline on a vector slide. A line is a path of two points, which
// takes a point as a path does and so becomes a polyline; a polyline left with two points becomes
// a line. The slide has a line and an open polyline of three points.
describe(['tagdesktop'], 'Points of a line on a vector slide', function() {

	beforeEach(function() {
		// The window is wide enough for the Shape tab to show all its buttons.
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/line_points.fodp');
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

	// The kind the drawing layer gives the selected object: 2 for a line, 9 for a polyline.
	function kindOfTheSelection(win) {
		var geometry = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name);
		var object = geometry.constructor.objectOf(selectedObject(win));
		return object && object.objectKind;
	}

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

	// Clicks Edit Points and waits for its state. The button runs a command of the client.
	function switchThePoints(win, shown) {
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-edit-points .unobutton:visible').click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('editpathpoints', shown ? 'true' : 'false');
	}

	// A click that switches a mode waits as long as a click for the engine does, so the second
	// click of a double click can take it back.
	function waitForTheClick(win) {
		cy.then(function() {
			return helper.waitForTimers(win, 'selectionClick');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A line shows its points like a path, and a point added to it makes it a polyline.', function() {
		var win = this.win;
		selectByTab(win, 1);
		cy.wrap(null).should(function() {
			expect(kindOfTheSelection(win), 'a line').to.equal(2);
			expect(pathPoints(win), 'the points of the line').to.have.length(2);
		});

		// a click on the line switches its points on, as for any path
		cy.then(function() {
			var points = pathPoints(win);
			clickAt(win, (3 * points[0].x + points[1].x) / 4, (3 * points[0].y + points[1].y) / 4);
		});
		waitForTheClick(win);
		helper.waitForMapState('editpathpoints', 'true');

		// Add Point waits for one press, which adds a point on the line
		desktopHelper.selectNotebookbarTab('Shape');
		cy.cGet('#Shape-container #shape-edit-points .arrowbackground:visible').click();
		desktopHelper.getOpenMenuEntries().eq(0).click();
		helper.waitForMapState('insertpathpoint', 'true');
		cy.then(function() {
			var points = pathPoints(win);
			clickAt(win, (points[0].x + 3 * points[1].x) / 4, (points[0].y + 3 * points[1].y) / 4);
		});
		cy.wrap(null).should(function() {
			expect(pathPoints(win), 'the points with the one added').to.have.length(3);
			expect(kindOfTheSelection(win), 'a polyline now').to.equal(9);
		});
	});

	it('A polyline left with two points becomes a line, which shows its two ends in object mode.', function() {
		var win = this.win;
		selectByTab(win, 2);
		cy.wrap(null).should(function() {
			expect(kindOfTheSelection(win), 'a polyline').to.equal(9);
		});
		switchThePoints(win, true);

		// the middle point is taken away by the key, with the keyboard on it
		cy.then(function() {
			win.document.getElementById('clipboard-area').focus();
		});
		for (var step = 0; step < 12; ++step) {
			cy.then(function() {
				var active = selectionSection(win).travel.activeHandle();
				if (!active || active.name !== '9.0.1')
					cy.realPress(['Control', 'Tab']);
			});
		}
		cy.realPress('Delete');

		// a line is a path of two points, so its points stay shown while they are edited
		cy.wrap(null).should(function() {
			expect(kindOfTheSelection(win), 'a line now').to.equal(2);
			expect(pathPoints(win), 'the points of the line').to.have.length(2);
			expect(selectionSection(win).pathPointsAreShown(), 'the points shown').to.equal(true);
		});

		// out of point editing the line shows its two ends, as a line does
		switchThePoints(win, false);
		cy.wrap(null).should(function() {
			var ends = selectionSection(win).handles().filter(function(handle) {
				return handle.group === 'shaping';
			});
			expect(ends, 'the two ends of the line').to.have.length(2);
		});
	});
});
