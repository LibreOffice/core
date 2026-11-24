/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// A 3D scene on a vector slide: a sphere, and a scene inside it holding a cube.
describe(['tagdesktop'], 'A 3D scene on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/scene_3d.fodp');
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

	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	function objectsWhere(win, test) {
		var found = [];
		geometry(win).currentPart().objects.forEach(function(object) {
			if (object.id && test(object))
				found.push(object);
		});
		return found;
	}

	// The scene on the slide, the one that sits in no other.
	function theScene(win) {
		return objectsWhere(win, function(object) {
			return object.objectKind === 202 && object.parent === 0;
		})[0];
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

	it('The scene draws what it holds and is selected and moved as one object.', function() {
		var win = this.win;
		var scene;
		cy.wrap(null).should(function() {
			scene = theScene(win);
			expect(scene, 'the scene').to.not.equal(undefined);
		});

		// what is inside comes with its 3D mapping and 3D primitive, the scene draws it all
		cy.then(function() {
			expect(scene.primitives.length, 'the picture of the scene').to.be.greaterThan(0);
			var inside = objectsWhere(win, function(object) {
				return object.parent === scene.id;
			});
			expect(inside.map(function(object) { return object.objectKind; }).sort())
				.to.deep.equal([202, 205]);
			var sphere = inside.find(function(object) { return object.objectKind === 205; });
			expect(sphere.transform3D).to.have.length(16);
			expect(sphere.primitives3D[0].type).to.equal('sphere');
			expect(sphere.primitives.length, 'nothing drawn in 2D').to.equal(0);
			expect(geometry(win).holdsObjects(scene.id), 'a group').to.equal(false);
		});

		// a click into the picture selects the scene
		cy.then(function() {
			var middle = onCanvas(win, scene.x + scene.width / 2, scene.y + scene.height / 2);
			cy.cGet('#document-canvas').click(middle.x, middle.y);
		});
		cy.wrap(null).should(function() {
			expect(win.eval('GraphicSelection').selectedObjectIDs).to.deep.equal([scene.id]);
		});

		// a move by key moves the scene, and the engine moves what it holds along
		var sphereBefore;
		cy.then(function() {
			sphereBefore = objectsWhere(win, function(object) {
				return object.objectKind === 205;
			})[0].x;
			helper.typeIntoDocument('{rightarrow}');
		});
		cy.wrap(null).should(function() {
			expect(theScene(win).x, 'the scene').to.be.above(scene.x);
			var sphere = objectsWhere(win, function(object) {
				return object.objectKind === 205;
			})[0];
			expect(sphere.x, 'the sphere').to.be.above(sphereBefore);
			expect(theScene(win).primitives.length).to.be.greaterThan(0);
		});
	});
});
