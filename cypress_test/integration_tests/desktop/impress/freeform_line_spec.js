/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Drawing a freeform line on a vector slide. The slide has a rectangle and a polygon, and the
// lower part of it is empty.
describe(['tagdesktop'], 'Freeform line on a vector slide', function() {

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
		desktopHelper.switchUIToNotebookbar();
	});

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The id of the selected object, or undefined when not exactly one is selected.
	function selectedObject(win) {
		var section = win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
		return section ? section.selectedObjects()[0] : undefined;
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

	// The mode one is in, or null. The classes of the page are names of its script, not
	// properties of its window, so the page itself is asked.
	function modeOn(win) {
		return win.eval('Interaction.modeOn()');
	}

	// Waits until the mode that draws the line is on, or off.
	function waitForTheMode(win, on) {
		cy.wrap(null).should(function() {
			expect(!!modeOn(win), 'the mode that draws the line').to.equal(on);
		});
	}

	// Chooses the freeform line from the grid of the Curves button in the Insert tab, which puts
	// the view into the mode that draws it.
	function chooseTheLine(win) {
		desktopHelper.selectNotebookbarTab('Insert');
		cy.cGet('[id^="insert-curve"] > .arrowbackground').click();
		cy.cGet('.col.w2ui-icon.lines_freeline_unfilled').click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		waitForTheMode(win, true);
	}

	// Clicks at that place in twips on the page, without moving.
	function clickAt(win, x, y) {
		cy.then(function() {
			var place = onCanvas(win, x, y);
			cy.cGet('#document-canvas')
				.trigger('mousedown', place.x, place.y, { button: 0, buttons: 1 })
				.trigger('mouseup', place.x, place.y, { button: 0, buttons: 0 });
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	// Presses the button at the first place, moves through the others in steps, and lets go at
	// the last one. The places are in twips on the page.
	function drawThrough(win, places) {
		cy.then(function() {
			var first = onCanvas(win, places[0].x, places[0].y);
			var drawing = cy.cGet('#document-canvas')
				.trigger('mousedown', first.x, first.y, { button: 0, buttons: 1 });
			for (var at = 1; at < places.length; ++at) {
				for (var step = 1; step <= 10; ++step) {
					var share = step / 10;
					var place = onCanvas(win,
						places[at - 1].x + share * (places[at].x - places[at - 1].x),
						places[at - 1].y + share * (places[at].y - places[at - 1].y));
					drawing = drawing.trigger('mousemove', place.x, place.y,
						{ button: 0, buttons: 1 });
				}
			}
			var last = onCanvas(win, places[places.length - 1].x, places[places.length - 1].y);
			drawing.trigger('mouseup', last.x, last.y, { button: 0, buttons: 0 });
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('A line drawn by hand becomes a selected curve with a corner where it turns.', function() {
		var win = this.win;
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});

		chooseTheLine(win);

		// A V shape below the two objects: down to the right, then up to the right.
		var places = [
			{ x: 1701, y: 6237 },
			{ x: 3402, y: 7938 },
			{ x: 5103, y: 6237 },
		];
		drawThrough(win, places);

		// The stroke is one part; a click that draws nothing makes the object of it.
		waitForTheMode(win, true);
		clickAt(win, 8000, 7000);

		// The engine adds the line as a freehand line, the kind 12, and it is selected. The mode is
		// over.
		waitForTheMode(win, false);
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(3);
			var objectId = selectedObject(win);
			expect(objectId, 'selected object').to.not.be.undefined;
			var object = geometry(win).objectOf(objectId);
			expect(object.objectKind, 'kind of the selected object').to.equal(12);

			// Each straight run is fitted with one curve, and the turn between them stays a
			// corner, give or take a place where a step of the mouse was rounded.
			var points = object.path[0].points;
			expect(points.length, 'points of the line').to.be.within(3, 5);
			expect(points[0].x, 'first point').to.be.closeTo(places[0].x, 60);
			expect(points[0].y, 'first point').to.be.closeTo(places[0].y, 60);
			expect(points[points.length - 1].x, 'last point').to.be.closeTo(places[2].x, 60);
			expect(points[points.length - 1].y, 'last point').to.be.closeTo(places[2].y, 60);
		});
	});

	it('A click before any line ends the mode with nothing made, as Escape does.', function() {
		var win = this.win;
		chooseTheLine(win);

		clickAt(win, 1701, 6237);
		waitForTheMode(win, false);
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});

		chooseTheLine(win);
		helper.typeIntoDocument('{esc}');
		waitForTheMode(win, false);
	});

	it('Escape during a press drops the line, so its release selects nothing.', function() {
		var win = this.win;
		chooseTheLine(win);

		// The keys go to the document without a click that would end the press first.
		cy.cGet('div.clipboard').focus();

		// The press is on the rectangle, which a click there would select.
		var place;
		cy.then(function() {
			place = onCanvas(win, 3685, 3969);
			cy.cGet('#document-canvas')
				.trigger('mousedown', place.x, place.y, { button: 0, buttons: 1 });
		});
		cy.wrap(null).should(function() {
			var mode = modeOn(win);
			expect(mode && mode.drawing.stroking(), 'the line the press started').to.be.ok;
		});
		helper.typeIntoDocument('{esc}');
		waitForTheMode(win, false);
		cy.then(function() {
			cy.cGet('#document-canvas')
				.trigger('mouseup', place.x, place.y, { button: 0, buttons: 0 })
				.trigger('click', place.x, place.y, { button: 0, buttons: 0 });
		});
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(selectedObject(win), 'selected object').to.be.undefined;
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});
	});
});
