/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Drawing a polygon point by point on a vector slide. The slide has a rectangle and a polygon, and
// the lower part of it is empty.
describe(['tagdesktop'], 'Polygon drawn point by point on a vector slide', function() {

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

	// Clicks at that place in twips on the page, without moving.
	function clickAt(win, x, y) {
		cy.then(function() {
			var place = onCanvas(win, x, y);
			cy.cGet('#document-canvas')
				.trigger('mousedown', place.x, place.y, { button: 0, buttons: 1 })
				.trigger('mouseup', place.x, place.y, { button: 0, buttons: 0 });
		});
	}

	it('A double click ends the polygon, which stays selected, with no text edit begun.', function() {
		var win = this.win;
		cy.wrap(null).should(function() {
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(2);
		});

		// The filled polygon, from the grid of the Curves button.
		desktopHelper.selectNotebookbarTab('Insert');
		cy.cGet('[id^="insert-curve"] > .arrowbackground').click();
		cy.cGet('.col.w2ui-icon.lines_polygon').click();
		cy.wrap(null).should(function() {
			expect(!!modeOn(win), 'the mode that draws the polygon').to.equal(true);
		});

		clickAt(win, 1701, 6237);
		clickAt(win, 5103, 6237);
		clickAt(win, 3402, 7938);

		// The double click sets its first click as the last point, which lies on the polygon,
		// so what the browser reports after it would reach the polygon made.
		cy.then(function() {
			var place = onCanvas(win, 3402, 6800);
			cy.cGet('#document-canvas').dblclick(place.x, place.y);
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});

		cy.wrap(null).should(function() {
			expect(!!modeOn(win), 'the mode that draws the polygon').to.equal(false);
			expect(geometry(win).heldByThePage(), 'objects of the slide').to.have.length(3);
			var objectId = selectedObject(win);
			expect(objectId, 'selected object').to.not.be.undefined;
			var object = geometry(win).objectOf(objectId);
			expect(object.path[0].points.length, 'points of the polygon').to.equal(4);
			expect(win.app.file.textCursor.visible, 'the text cursor').to.equal(false);
		});
	});
});
