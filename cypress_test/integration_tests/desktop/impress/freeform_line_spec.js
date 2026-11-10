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

	// Chooses the tool from the Insert tab, or puts it away, and waits for its state.
	function clickTheTool(win, chosen) {
		desktopHelper.selectNotebookbarTab('Insert');
		cy.cGet('#insert-freeform-line-button').click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('freeformline', chosen ? 'true' : 'false');
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

	it('A line drawn with the tool becomes a selected polyline through its corners.', function() {
		var win = this.win;
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});

		clickTheTool(win, true);

		// A V shape below the two objects: down to the right, then up to the right.
		var places = [
			{ x: 1701, y: 6237 },
			{ x: 3402, y: 7938 },
			{ x: 5103, y: 6237 },
		];
		drawThrough(win, places);

		// The engine adds the line as a polyline, the kind 9, and selects it. The tool is put away
		// and the line drawn while waiting for the engine goes.
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(3);
			var objectId = selectedObject(win);
			expect(objectId, 'selected object').to.not.be.undefined;
			var object = geometry(win).objectOf(objectId);
			expect(object.objectKind, 'kind of the selected object').to.equal(9);

			// The straight runs between the corners are kept as single edges, give or take a
			// place where a step of the mouse was rounded.
			var points = object.path[0].points;
			expect(points.length, 'points of the line').to.be.within(3, 6);
			expect(points[0].x, 'first point').to.be.closeTo(places[0].x, 60);
			expect(points[0].y, 'first point').to.be.closeTo(places[0].y, 60);
			expect(points[points.length - 1].x, 'last point').to.be.closeTo(places[2].x, 60);
			expect(points[points.length - 1].y, 'last point').to.be.closeTo(places[2].y, 60);

			expect(win.app.sectionContainer.doesSectionExist('freeform line'),
				'the line drawn while waiting').to.equal(false);
		});
		helper.waitForMapState('freeformline', 'false');
	});

	it('The tool stays chosen after a press that draws no line, and Escape puts it away.', function() {
		var win = this.win;
		clickTheTool(win, true);

		// A press that does not move draws no line, so nothing is added.
		cy.then(function() {
			var place = onCanvas(win, 1701, 6237);
			cy.cGet('#document-canvas').click(place.x, place.y);
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
		helper.waitForMapState('freeformline', 'true');
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});

		helper.typeIntoDocument('{esc}');
		helper.waitForMapState('freeformline', 'false');
		cy.wrap(null).should(function() {
			expect(win.app.sectionContainer.doesSectionExist('freeform line'),
				'the section of the tool').to.equal(false);
		});

		// The button chooses the tool again, and a second click puts it away.
		clickTheTool(win, true);
		clickTheTool(win, false);
	});

	it('Escape during a press ends it, so its release selects nothing.', function() {
		var win = this.win;
		clickTheTool(win, true);

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
			var section = win.app.sectionContainer.getSectionWithName('freeform line');
			expect(section && section.stroke, 'the line the press started').to.be.ok;
		});
		helper.typeIntoDocument('{esc}');
		helper.waitForMapState('freeformline', 'false');
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
