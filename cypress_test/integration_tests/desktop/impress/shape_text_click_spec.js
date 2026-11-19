/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// A click on a selected shape with text on a vector slide. The slide holds one rectangle with a
// line of text.
describe(['tagdesktop'], 'A click on a shape with text on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/shape_text_click.fodp');
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

	it('On a shape with text a click beside the text switches its glue points, the text edits as before.', function() {
		var win = this.win;
		selectByTab(win, 1);

		/*
			A place on the shape where the hit test finds its text, or finds the shape beside its
			text: the first one of a grid over its box, away from the edges and their handles.
			Where the text lies is the shape's own layout, so it is asked for rather than assumed.
		*/
		var placeWhere = function(onTheText) {
			var geometry = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.RenderGeometry.name).constructor;
			var box = geometry.boxOf(selectedObject(win));
			for (var down = 0.15; down < 0.9; down += 0.05) {
				for (var across = 0.15; across < 0.9; across += 0.05) {
					var x = box.minX + across * (box.maxX - box.minX);
					var y = box.minY + down * (box.maxY - box.minY);
					if ((geometry.pointerAt(x, y) === 'text') === onTheText)
						return { x: x, y: y };
				}
			}
			return null;
		};
		var clickWhere = function(onTheText) {
			cy.then(function() {
				var place = placeWhere(onTheText);
				expect(place, onTheText ? 'a place on the text' : 'a place beside it').to.not.equal(null);
				clickAt(win, place.x, place.y);
			});
			waitForTheClick(win);
		};

		clickWhere(false);
		helper.waitForMapState('showgluepoints', 'true');
		clickWhere(false);
		helper.waitForMapState('showgluepoints', 'false');

		// on the text the click edits the text
		clickWhere(true);
		cy.wrap(null).should(function() {
			expect(win.app.file.textCursor.visible, 'the text edited').to.equal(true);
		});
		helper.waitForMapState('showgluepoints', 'false');

		// Enter, F2 and a letter typed start the text edit of the selected shape as well
		[function() { helper.typeIntoDocument('{enter}'); }, function() { cy.realPress('F2'); }].forEach(function(press, which) {
			var key = which === 0 ? 'Enter' : 'F2';
			helper.typeIntoDocument('{esc}');
			cy.wrap(null).should(function() {
				expect(win.app.file.textCursor.visible, 'the text left').to.equal(false);
				expect(selectedObject(win), 'the shape still selected').to.not.equal(undefined);
			});
			press();
			cy.wrap(null).should(function() {
				expect(win.app.file.textCursor.visible, 'the text edited by ' + key).to.equal(true);
			});
		});
		helper.typeIntoDocument('{esc}');
		cy.wrap(null).should(function() {
			expect(win.app.file.textCursor.visible, 'the text left').to.equal(false);
		});
		helper.typeIntoDocument('x');
		cy.wrap(null).should(function() {
			expect(win.app.file.textCursor.visible, 'the text edited by typing').to.equal(true);
		});
	});
});
