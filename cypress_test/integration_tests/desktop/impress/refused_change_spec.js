/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// A change the engine refuses on a vector slide. The slide holds one rectangle whose position is
// protected.
describe(['tagdesktop'], 'A change the engine refuses on a vector slide', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/refused_change.fodp');
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

	// The section that keeps the geometry of the objects the client holds.
	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The one object of the slide.
	function theRectangle(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (!found && object.id)
				found = object;
		});
		return found;
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

	// Drags from that place on the page, in twips, by that many twips across and down, in steps.
	function dragFrom(win, from, across, down) {
		cy.then(function() {
			var start = onCanvas(win, from.x, from.y);
			var drawing = cy.cGet('#document-canvas').trigger('mousedown', start.x, start.y,
				{ button: 0, buttons: 1 });
			for (var step = 1; step <= 10; ++step) {
				var place = onCanvas(win, from.x + step / 10 * across, from.y + step / 10 * down);
				drawing = drawing.trigger('mousemove', place.x, place.y, { button: 0, buttons: 1 });
			}
			var end = onCanvas(win, from.x + across, from.y + down);
			drawing.trigger('mouseup', end.x, end.y, { button: 0, buttons: 0 });
		});
	}

	it('A move the engine refuses goes back to where the object stands.', function() {
		var win = this.win;
		var before;
		cy.wrap(null).should(function() {
			before = theRectangle(win);
			expect(before, 'the rectangle').to.not.equal(undefined);
			expect(before.cannotBeMoved, 'its position is protected').to.equal(true);
		});

		// the client is told the object may move, as where it does not know better yet, so the
		// drag goes through here and the engine is the one that refuses it
		var startedAt;
		var sent = [];
		cy.then(function() {
			var send = win.app.socket.sendMessage.bind(win.app.socket);
			win.app.socket.sendMessage = function(message) {
				if (typeof message === 'string') sent.push(message);
				return send.apply(null, arguments);
			};
			startedAt = { x: before.x, y: before.y };
			before.cannotBeMoved = false;
			dragFrom(win, { x: before.x + before.width / 2, y: before.y + before.height / 2 },
				1500, 1000);
		});

		cy.wrap(null).should(function() {
			expect(sent.some(function(message) {
				return message.indexOf('setobjecttransform') === 0;
			}), 'the move went to the engine').to.equal(true);
			var now = theRectangle(win);
			expect(geometry(win).awaitsAnAnswer(now.id), 'a change waiting').to.equal(false);
			expect([Math.round(now.x), Math.round(now.y)], 'where it stands')
				.to.deep.equal([Math.round(startedAt.x), Math.round(startedAt.y)]);
			// the object came anew from the engine
			expect(now.cannotBeMoved, 'its position is protected').to.equal(true);
		});
	});
});
