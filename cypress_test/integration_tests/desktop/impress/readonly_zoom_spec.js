/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// A read-only presentation shows all slides stacked one under the other. The first zoom fits
// one slide in the window, the same as in an editable view, and the reader scrolls down
// through the rest.
describe(['tagdesktop'], 'Impress: zoom of a read-only presentation', function() {

	beforeEach(function() {
		cy.viewport(1280, 800);
		var filePath = helper.setupDocument('impress/slide_navigation.odp');
		helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly&vector=false');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('opens with one whole slide filling the window', function() {
		cy.cGet('#document-canvas').should('be.visible');
		helper.processToIdle(this.win);

		cy.getFrameWindow().should(function(win) {
			var docLayer = win.app.map._docLayer;
			var frame = win.app.activeDocument.activeLayout.frameSize;
			var slideWidth = win.app.activeDocument.fileSize.pX;
			var slideHeight = docLayer._partHeightTwips * win.app.twipsToPixels;
			var zoomPercent = win.app.activeDocument.getZoomPercent();

			// The measured view, repeated in each message so a failure explains itself.
			var summary = 'one slide shows at ' + Math.round(slideWidth) + ' x '
				+ Math.round(slideHeight) + ' px in a ' + frame.pX + ' x ' + frame.pY
				+ ' px window, at ' + zoomPercent + '% zoom, slide '
				+ (docLayer._selectedPart + 1) + ' of ' + docLayer._parts;

			expect(win.app.file.fileBasedView,
				'precondition: all slides are shown in one stack').to.equal(true);
			expect(win.app.activeDocument.fileSize.pY,
				'precondition: the stack of slides is taller than the window')
				.to.be.greaterThan(frame.pY);

			var widthShare = slideWidth / frame.pX;
			var heightShare = slideHeight / frame.pY;
			var filledPercent = Math.round(Math.max(widthShare, heightShare) * 100);
			expect(filledPercent, 'percent of the window that one slide fills; ' + summary)
				.to.be.greaterThan(60);
			expect(zoomPercent, 'zoom in percent, 20 is the lowest zoom; ' + summary)
				.to.be.greaterThan(20);
			expect(slideWidth, 'one slide is wider than the window; ' + summary)
				.to.be.at.most(frame.pX);
			expect(slideHeight, 'one slide is taller than the window; ' + summary)
				.to.be.at.most(frame.pY);
		});
	});

	it('zooming out while tiles are pre-fetched asks for each tile once', function() {
		var win = this.win;
		var tileErrors = [];
		var prefetchedInAnimation = null;

		cy.cGet('#document-canvas').should('be.visible');
		helper.processToIdle(win);

		cy.then(function() {
			cy.stub(win.app.socket, 'sendMessage').as('sendMessage').callThrough();
			win.app.socket.socket.addEventListener('message', function(event) {
				if (typeof event.data === 'string' && event.data.startsWith('error: cmd=tile'))
					tileErrors.push(event.data);
			});

			// Each frame of the zoom animation draws the slides at an in-between scale. The
			// pre-fetch timer can fire between two frames, so run one pre-fetch pass right
			// after the first frame drawn at a scale other than the start one.
			var tilesSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.Tiles.name);
			tilesSection.drawZoomFrame = function() {
				Object.getPrototypeOf(tilesSection).drawZoomFrame.apply(this, arguments);
				if (prefetchedInAnimation === null
					&& win.app.tile.size.pX !== win.RenderManager.tileSize) {
					var state = {
						inAnimation: win.app.sectionContainer.isInZoomAnimation(),
						tilePixels: win.app.tile.size.pX,
					};
					// The state is kept only when the pass returns. An exception inside an
					// animation frame reaches no test, so a pass that throws leaves false.
					prefetchedInAnimation = false;
					win.RenderManager.ensureInstance().preFetchTiles(false);
					prefetchedInAnimation = state;
				}
			};

			// The test build zooms in one step. The user sees the animated zoom, so turn
			// the animation on for this zoom.
			win.L.Browser.cypressTest = false;
		});

		desktopHelper.zoomOut();

		cy.then(function() {
			win.L.Browser.cypressTest = true;
			delete win.app.sectionContainer.getSectionWithName(
				win.app.CSections.Tiles.name).drawZoomFrame;
		});
		helper.processToIdle(win);

		cy.get('@sendMessage').should(function(sendMessage) {
			expect(prefetchedInAnimation,
				'precondition: tiles were pre-fetched during the zoom animation').to.be.an('object');
			expect(prefetchedInAnimation.inAnimation,
				'precondition: the zoom animation was running').to.equal(true);

			var tileRequests = sendMessage.getCalls()
				.map(function(call) { return call.args[0]; })
				.filter(function(message) {
					return typeof message === 'string' && message.startsWith('tilecombine');
				});
			tileRequests.forEach(function(message) {
				var positionsX = message.match(/tileposx=(\S+)/)[1].split(',');
				var positionsY = message.match(/tileposy=(\S+)/)[1].split(',');
				var positions = positionsX.map(function(x, i) { return x + ',' + positionsY[i]; });
				expect(new Set(positions).size,
					'distinct tiles in a request sent with ' + prefetchedInAnimation.tilePixels
					+ ' px tiles on screen: ' + message).to.equal(positions.length);
			});
			expect(tileErrors, 'tile errors from the server').to.deep.equal([]);
		});
		cy.cGet('#modal-dialog-cool_alert').should('not.exist');
	});
});
