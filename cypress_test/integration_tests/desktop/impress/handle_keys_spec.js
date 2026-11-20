/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

/*
	Delete, Backspace, Return and Space while the keyboard is on a handle of a vector slide: on a
	point they act on the point, on other handles they do nothing, and they never reach the object
	or its text.
*/
describe(['tagdesktop'], 'Keys on the handle the keyboard is on', function() {

	function load(document) {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/' + document);
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		var win;
		cy.getFrameWindow().then(function(frame) {
			win = frame;
		});
		// The usual load checks wait for tiles, which a vector view does not have.
		cy.wrap(null).should(function() {
			expect(win.app.impress.partList).to.have.length.greaterThan(0);
		});
		return cy.then(function() {
			return helper.processToIdle(win);
		}).then(function() {
			return win;
		});
	}

	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	// Selects the object Tab reaches after that many presses.
	function selectByTab(win, count) {
		helper.typeIntoDocument('{esc}');
		for (var step = 0; step < count; ++step) {
			cy.realPress('Tab');
			cy.then(function() {
				return helper.processToIdle(win);
			});
		}
		cy.wrap(null).should(function() {
			expect(selectionSection(win), 'an object selected').to.not.equal(undefined);
		});
	}

	// Walks the handles with Ctrl+Tab until the keyboard is on the one of that name.
	function travelTo(win, name) {
		for (var step = 0; step < 20; ++step) {
			cy.then(function() {
				var active = selectionSection(win).travel.activeHandle();
				if (!active || active.name !== name)
					cy.realPress(['Control', 'Tab']);
			});
		}
		cy.wrap(null).should(function() {
			var active = selectionSection(win).travel.activeHandle();
			expect(active && active.name, 'the handle the keyboard is on').to.equal(name);
		});
	}

	function expectNoTextEdit(win) {
		cy.wait(500);
		cy.then(function() {
			expect(win.app.file.textCursor.visible, 'no text edited').to.equal(false);
		});
	}

	it('On a point of the path Return and Space open its menu.', function() {
		load('path_points.fodp').then(function(win) {
			selectByTab(win, 2);
			desktopHelper.selectNotebookbarTab('Shape');
			cy.cGet('#Shape-container #shape-edit-points .unobutton:visible').click();
			helper.waitForMapState('editpathpoints', 'true');
			cy.then(function() {
				win.document.getElementById('clipboard-area').focus();
			});
			travelTo(win, '9.0.1');

			['Enter', ' '].forEach(function(key) {
				cy.realPress(key);
				cy.cGet('#path-point-menu-entry-1').should('be.visible');
				cy.realPress('Escape');
				cy.cGet('#path-point-menu-entry-1').should('not.exist');
			});
			expectNoTextEdit(win);
		});
	});

	it('On a point of a motion path Delete takes the point away and the object stays.', function() {
		load('motion_path_edit.fodp').then(function(win) {
			selectByTab(win, 1);
			var objectId;
			var objects;
			cy.then(function() {
				objectId = selectionSection(win).selectedObjects()[0];
				objects = geometry(win).heldByThePage().length;
			});
			travelTo(win, 'motion 0 9.0.1');

			cy.realPress('Enter');
			cy.cGet('#motion-path-point-menu-entry-1').should('be.visible');
			cy.realPress('Escape');
			cy.cGet('#motion-path-point-menu-entry-1').should('not.exist');
			expectNoTextEdit(win);

			cy.realPress('Delete');
			cy.wrap(null).should(function() {
				var object = geometry(win).objectOf(objectId);
				expect(geometry(win).heldByThePage(), 'the objects').to.have.length(objects);
				expect(object.motionPaths[0].path[0].points, 'the points left').to.have.length(2);
			});
		});
	});

	it('On a crop mark Delete, Backspace, Return and Space leave the picture as it is.', function() {
		load('picture_crop.fodp').then(function(win) {
			selectByTab(win, 1);
			cy.then(function() {
				win.eval('GraphicSelection.toggleCropping()');
			});
			travelTo(win, 'crop1');

			['Delete', 'Backspace', 'Enter', ' '].forEach(function(key) {
				cy.realPress(key);
			});
			cy.wait(500);
			cy.then(function() {
				expect(geometry(win).heldByThePage(), 'the picture').to.have.length(1);
				expect(win.app.file.textCursor.visible, 'no text edited').to.equal(false);
			});
		});
	});

	it('On a glue point Return and Space open its menu, Backspace takes it away.', function() {
		load('glue_points.fodp').then(function(win) {
			selectByTab(win, 1);
			var objectId;
			cy.then(function() {
				objectId = selectionSection(win).selectedObjects()[0];
			});

			// the glue points are shown by the button, and Add New adds one by a press
			desktopHelper.selectNotebookbarTab('Shape');
			cy.cGet('#Shape-container #shape-gluepoints-button:visible').click();
			desktopHelper.getOpenMenuEntries().eq(0).click();
			helper.waitForMapState('showgluepoints', 'true');
			cy.cGet('#Shape-container #shape-gluepoints-button:visible').click();
			desktopHelper.getOpenMenuEntries().eq(1).click();
			cy.then(function() {
				var box = geometry(win).boxOf(objectId);
				var visible = win.app.activeDocument.activeLayout.viewedRectangle;
				var scale = win.app.twipsToPixels / win.app.dpiScale;
				cy.cGet('#document-canvas').click(
					(box.minX + box.maxX) / 2 * scale - visible.pX1 / win.app.dpiScale,
					(box.minY + box.maxY) / 2 * scale - visible.pY1 / win.app.dpiScale);
			});
			cy.wrap(null).should(function() {
				expect(geometry(win).objectOf(objectId).gluePoints, 'the point added').to.have.length(1);
			});
			cy.then(function() {
				win.document.getElementById('clipboard-area').focus();
			});
			// a glue point is named by its glue id, which the engine gives it
			var name;
			cy.then(function() {
				name = 'GluePoint.' + geometry(win).objectOf(objectId).gluePoints[0].id;
			});
			cy.then(function() {
				travelTo(win, name);
			});

			['Enter', ' '].forEach(function(key) {
				cy.realPress(key);
				cy.cGet('#tie-point-menu-entry-1').should('be.visible');
				cy.realPress('Escape');
				cy.cGet('#tie-point-menu-entry-1').should('not.exist');
			});
			expectNoTextEdit(win);

			cy.realPress('Backspace');
			cy.wrap(null).should(function() {
				expect(geometry(win).objectOf(objectId).gluePoints || [], 'the points left')
					.to.have.length(0);
			});
		});
	});
});
