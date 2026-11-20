/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// A line and a rectangle with rounded corners whose size is held fast, on a vector slide.
describe(['tagdesktop'], 'The handles of an object whose size is held fast', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/held_size.fodp');
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

	function objectWhere(win, test) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (object.id && object.kind === undefined && test(object))
				found = object;
		});
		return found;
	}

	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	function shapingHandles(win) {
		return selectionSection(win).handles().filter(function(handle) {
			return handle.group === 'shaping';
		});
	}

	it('A line still shows its ends, a shape with the eight around it no handle that shapes it.', function() {
		var win = this.win;
		var line;
		var rectangle;
		cy.wrap(null).should(function() {
			line = objectWhere(win, function(object) { return object.objectKind === 2; });
			rectangle = objectWhere(win, function(object) { return object.objectKind !== 2; });
			expect(line, 'the line').to.not.equal(undefined);
			expect(rectangle, 'the rectangle').to.not.equal(undefined);
			expect(line.cannotBeResized, 'the line held').to.equal(true);
			expect(rectangle.cannotBeResized, 'the rectangle held').to.equal(true);
		});

		// the line has no eight around it, so its two ends show it is selected, pale
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([line.id]);
		});
		cy.wrap(null).should(function() {
			var ends = shapingHandles(win);
			expect(ends, 'the two ends of the line').to.have.length(2);
			ends.forEach(function(end) {
				expect(end.deactivated, 'pale').to.equal(true);
			});
		});

		// the rectangle shows the eight around it, the handle of its corners is left out
		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([rectangle.id]);
		});
		cy.wrap(null).should(function() {
			expect(selectionSection(win).handles().length, 'the eight').to.be.greaterThan(0);
			expect(shapingHandles(win), 'no handle for the corners').to.have.length(0);
		});
	});
});
