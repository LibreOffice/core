/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The scrollable width covers the comment column wherever the page
// sits, so a comment card beside the page can be scrolled into view.
describe(['tagdesktop'], 'Writer comments can be scrolled into view.', function() {

	beforeEach(function() {
		// The document holds one comment.
		helper.setupAndLoadDocument('writer/annotation-large.odt');
		cy.viewport(1400, 900);
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		cy.cGet('.cool-annotation').should('exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	});

	function commentSection(win) {
		return win.app.sectionContainer.getSectionWithName(win.app.CSections.CommentList.name);
	}

	// The page, the comment column and the scrollable area, in canvas pixels.
	function readLayout(win) {
		var layout = win.app.activeDocument.activeLayout;
		var anchor = win.app.sectionContainer.getDocumentAnchorSection();
		var comments = commentSection(win);
		return {
			frame: anchor.size[0],
			page: win.app.activeDocument.fileSize.pX,
			margin: comments.calculateAvailableSpace(),
			commentWidth: comments.sectionProperties.commentWidth,
			scrollableWidth: layout.viewSize.pX,
			scrollsHorizontally: layout.canScrollHorizontal(anchor),
			viewX: layout.scrollProperties.viewX,
			viewY: layout.scrollProperties.viewY,
		};
	}

	// Zooms in a step at a time until the margin beside the page fits the
	// condition, and fails if ten steps are not enough.
	function zoomUntil(condition, steps = 0) {
		cy.then(() => {
			if (condition(readLayout(this.win)))
				return;
			expect(steps, 'zoom steps until the margin fits').to.be.below(10);
			desktopHelper.zoomIn();
			cy.then(() => {
				return helper.processToIdle(this.win);
			});
			zoomUntil.call(this, condition, steps + 1);
		});
	}

	// Scrolls through the scroll section, the path a wheel and a scroll bar drag take.
	function scrollFullyRight() {
		cy.then(() => {
			this.win.app.sectionContainer
				.getSectionWithName(this.win.app.CSections.Scroll.name)
				.scrollHorizontalWithOffset(100000);
			return helper.processToIdle(this.win);
		});
	}

	// The right edge of the card is inside the document area.
	function cardEndIsInView() {
		cy.cGet('#document-container').then((container) => {
			var right = container[0].getBoundingClientRect().right;
			cy.cGet('.cool-annotation').last().should((card) => {
				expect(card[0].getAnimations().length, 'card animations').to.equal(0);
				expect(card[0].getBoundingClientRect().right, 'card right edge')
					.to.be.at.most(right + 1);
			});
		});
	}

	// The margin is narrower than half a comment, so the page stays centred and
	// the comment column overflows the right edge.
	function comesOutCentred(layout) {
		return layout.margin > 0 && layout.margin * 2 < layout.commentWidth;
	}

	it('The end of a comment beside a centred page can be scrolled into view.', function() {
		zoomUntil.call(this, comesOutCentred);
		scrollFullyRight.call(this);
		cardEndIsInView.call(this);
	});

	it('A comment that fits beside the shifted page needs no sideways scrolling.', function() {
		zoomUntil.call(this, (layout) => {
			return layout.margin * 2 >= layout.commentWidth && layout.margin < layout.commentWidth;
		});

		// One of two widths a pixel apart leaves an odd gap beside the page,
		// where rounding the margin up used to add a pixel of scroll.
		[1400, 1401].forEach((width) => {
			cy.viewport(width, 900);
			cy.then(() => {
				return helper.processToIdle(this.win);
			});
			cy.then(() => {
				var layout = readLayout(this.win);
				expect(layout.scrollsHorizontally, 'sideways scrolling at ' + width).to.equal(false);
				expect(layout.scrollableWidth, 'scrollable width at ' + width).to.be.at.most(layout.frame);
			});
		});
	});

	it('Pressing the horizontal scroll bar beside a centred page scrolls sideways.', function() {
		zoomUntil.call(this, comesOutCentred);

		cy.then(() => {
			var win = this.win;
			var before = readLayout(win);
			expect(before.scrollsHorizontally, 'sideways scrolling').to.equal(true);

			// A press near the right end of the bar's track, in canvas pixels.
			var scroll = win.app.sectionContainer.getSectionWithName('scroll');
			var thickness = win.app.activeDocument.activeLayout.scrollProperties.usableThickness;
			var point = win.cool.SimplePoint.fromCorePixels(
				[scroll.size[0] - 2 * thickness, scroll.size[1] - thickness / 2]);
			var event = { stopPropagation: function() {} };
			scroll.onMouseDown(point, event);
			scroll.onMouseUp(point, event);

			var after = readLayout(win);
			expect(after.viewX, 'horizontal scroll').to.be.greaterThan(before.viewX);
			expect(after.viewY, 'vertical scroll').to.equal(before.viewY);
		});
	});

	it('The comment column stays reachable while the server reports new document sizes.', function() {
		zoomUntil.call(this, comesOutCentred);

		cy.then(() => {
			var win = this.win;
			var before = readLayout(win);
			var size = win.app.activeDocument.fileSize;

			// A taller document, as reported while the pages are still being laid out.
			win.app.map._docLayer._setNewSize(new win.cool.SimplePoint(size.x, size.y + 1000));

			var after = readLayout(win);
			expect(after.scrollableWidth, 'scrollable width').to.equal(before.scrollableWidth);
			expect(after.scrollsHorizontally, 'sideways scrolling').to.equal(true);
		});

		scrollFullyRight.call(this);
		cardEndIsInView.call(this);
	});

	it('A wider page from the server ends the scroll at the end of the comment.', function() {
		zoomUntil.call(this, comesOutCentred);

		cy.then(() => {
			var win = this.win;
			var size = win.app.activeDocument.fileSize;
			win.app.map._docLayer._setNewSize(new win.cool.SimplePoint(Math.round(size.x * 1.5), size.y));
		});

		// The page is now wider than the view, so it starts at the left edge and
		// the scroll ends where the comment column ends.
		cy.wrap(null).should(() => {
			var layout = readLayout(this.win);
			expect(layout.page, 'page width').to.be.greaterThan(layout.frame);
			expect(layout.scrollableWidth, 'scrollable width')
				.to.be.closeTo(layout.page + layout.commentWidth, 2);
		});
	});

	it('The comment column follows a change of the screen scale.', function() {
		cy.then(() => {
			var win = this.win;
			var scale = win.devicePixelRatio * 1.25;
			Object.defineProperty(win, 'devicePixelRatio', { configurable: true, get: function() { return scale; } });
			win.app.sectionContainer.onResize();

			var comments = commentSection(win);
			expect(win.app.dpiScale, 'screen scale').to.equal(scale);
			expect(comments.sectionProperties.commentWidth, 'comment width')
				.to.equal(comments.constructor.getCommentWidth());
			delete win.devicePixelRatio;
			win.app.sectionContainer.onResize();
		});
	});
});

// A read-only document animates the wheel scroll.
describe(['tagdesktop'], 'Writer wheel scrolling in a read-only document.', function() {

	beforeEach(function() {
		// A read-only view has no notebookbar, which the usual load checks wait for.
		var filePath = helper.setupDocument('writer/scrolling.odt');
		helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly');
		cy.getFrameWindow().should((win) => {
			expect(win.app.map._docLoaded, 'document loaded').to.equal(true);
			this.win = win;
		});
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	});

	it('The wheel still scrolls after a wheel turn during a zoom animation.', function() {
		var layout;
		var topBeforeScroll;

		// One mouse wheel notch on the middle of the canvas, so the document area gets it.
		function turnWheel(win) {
			var canvas = win.document.querySelector('#document-canvas');
			var rect = canvas.getBoundingClientRect();
			canvas.dispatchEvent(new win.WheelEvent('wheel', {
				deltaY: 3,
				deltaMode: win.WheelEvent.DOM_DELTA_LINE,
				clientX: rect.left + rect.width / 2,
				clientY: rect.top + rect.height / 2,
				bubbles: true,
				cancelable: true,
			}));
		}

		cy.then(() => {
			var win = this.win;
			var scroll = win.app.sectionContainer.getSectionWithName(win.app.CSections.Scroll.name);
			expect(scroll.sectionProperties.animateWheelScroll, 'animated wheel scroll').to.be.true;

			// The zoom section holds the canvas animation while the wheel turns.
			var zoom = win.app.sectionContainer.getSectionWithName(win.app.CSections.ZoomControl.name);
			expect(zoom.startAnimating({ duration: 250 }), 'zoom animation started').to.be.true;
			turnWheel(win);
			return helper.waitForCanvasAnimation(win);
		});

		cy.then(() => {
			var win = this.win;
			layout = win.app.activeDocument.activeLayout;
			topBeforeScroll = layout.viewedRectangle.pY1;
			turnWheel(win);
		});

		cy.wrap(null).should(function() {
			expect(layout.viewedRectangle.pY1, 'top of the view').to.be.greaterThan(topBeforeScroll);
		});
	});
});
