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

		// a click into an empty corner of the picture meets no 3D object, and so nothing
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([]);
		});
		cy.then(function() {
			var corner = onCanvas(win, scene.x + scene.width - 200, scene.y + 200);
			cy.cGet('#document-canvas').click(corner.x, corner.y);
		});
		cy.wrap(null).should(function() {
			expect(win.eval('GraphicSelection').selectedObjectIDs).to.deep.equal([]);
		});
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

	it('A 3D scene is entered at a 3D object, gone through by Tab and left by Escape.', function() {
		var win = this.win;
		var scene;
		var sphere;
		var cube;
		cy.wrap(null).should(function() {
			scene = theScene(win);
			expect(scene, 'the scene').to.not.equal(undefined);
			sphere = objectsWhere(win, function(object) { return object.objectKind === 205; })[0];
			cube = objectsWhere(win, function(object) { return object.objectKind === 204; })[0];
		});

		function selected() {
			return win.eval('GraphicSelection').selectedObjectIDs;
		}
		// what is selected stays so once the engine answered; the object is asked for when the
		// step runs, once it was found
		function selects(objectOf) {
			cy.wrap(null).should(function() {
				expect(selected()).to.deep.equal([objectOf().id]);
			});
			cy.then(function() {
				return helper.processToIdle(win);
			});
			cy.wrap(null).should(function() {
				expect(selected(), 'after the engine answered').to.deep.equal([objectOf().id]);
			});
		}

		// a double click on the sphere enters the scene there
		cy.then(function() {
			var middle = onCanvas(win, scene.x + scene.width / 2, scene.y + scene.height / 2);
			cy.cGet('#document-canvas').dblclick(middle.x, middle.y);
		});
		selects(function() { return sphere; });
		cy.then(function() {
			expect(win.eval('Interaction').placeOf(selected()).kind).to.equal('scene');
			expect(sphere.cannotBeMoved, 'a 3D object changes in 3D').to.equal(true);
		});

		// beside the scene stand Enter, pale as there is nothing to enter, and Leave, but no
		// Ungroup, as a scene is never taken apart
		cy.wrap(null).should(function() {
			var buttons = {};
			win.eval('GraphicSelection').groupButtons.forEach(function(button) {
				buttons[button.role] = button.getHTMLObject().classList.contains('group-button-off');
			});
			expect(buttons, 'the buttons and whether each is pale').to.deep.equal({ enter: true, leave: false });
		});

		// Tab goes on to the cube in the scene inside, and round to the sphere, as in a group
		cy.realPress('Tab');
		selects(function() { return cube; });
		cy.realPress('Tab');
		selects(function() { return sphere; });

		// Escape leaves to the scene, Enter goes in again at the first 3D object
		cy.realPress('Escape');
		selects(function() { return scene; });
		cy.realPress('Enter');
		selects(function() { return sphere; });
		cy.realPress('Escape');
		selects(function() { return scene; });
	});

	it('A lasso takes a 3D scene and its 3D objects by their faces as one sees them.', function() {
		var win = this.win;
		var scene;
		var sphere;
		var cube;
		cy.wrap(null).should(function() {
			scene = theScene(win);
			expect(scene, 'the scene').to.not.equal(undefined);
			sphere = objectsWhere(win, function(object) { return object.objectKind === 205; })[0];
			cube = objectsWhere(win, function(object) { return object.objectKind === 204; })[0];
		});

		// a lasso just round the faces of one object or of the whole scene, in twips
		function roundTheFaces(objectId) {
			var faces = win.eval('cool').Scene3D.facesOnThePage(
				geometry(win).currentPart(), objectId);
			var points = [];
			faces.forEach(function(face) { points = points.concat(face.points); });
			var range = win.eval('cool').Range2D.fromPoints(points);
			var Point = win.eval('cool').Point;
			return [
				new Point(range.minX - 50, range.minY - 50),
				new Point(range.maxX + 50, range.minY - 50),
				new Point(range.maxX + 50, range.maxY + 50),
				new Point(range.minX - 50, range.maxY + 50),
			];
		}
		function lassoSelects(objectId) {
			return win.eval('Interaction').context().lassoSelects(roundTheFaces(objectId), false);
		}

		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([]);
		});
		cy.wrap(null).should(function() {
			var lasso = roundTheFaces(scene.id);
			expect(lasso[0].x, 'smaller than the picture').to.be.above(scene.x - 50);
			expect(lassoSelects(scene.id), 'round the whole scene').to.deep.equal([scene.id]);
			expect(lassoSelects(sphere.id), 'round the sphere alone').to.deep.equal([]);
		});

		// inside the scene a lasso round the cube takes the cube
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([sphere.id]);
		});
		cy.wrap(null).should(function() {
			expect(lassoSelects(cube.id), 'round the cube').to.deep.equal([cube.id]);
		});
	});
});
