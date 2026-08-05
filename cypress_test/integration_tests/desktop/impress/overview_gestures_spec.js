/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Overview page gestures', { testIsolation: false }, function() {

	var desktopHelper = require('../../common/desktop_helper');
	desktopHelper.shareDocumentAcrossTests('impress/overview_gestures.odp', {
		notebookbar: true,
	});

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// Twips in the canvas page's document space to CSS pixel offsets from the canvas
	// element's top-left corner, the same conversion the preview edit spec uses.
	function toClient(win, x, y) {
		const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
		return {
			x: (x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale,
			y: (y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale,
		};
	}

	function previewRect(win, part) {
		const entry = win.app.impress.previewRects.find(function(preview) {
			return preview.part === part;
		});
		expect(entry).to.exist;
		return entry.rectangle;
	}

	// A twips point on empty canvas: just right of the whole preview grid, level with the
	// first preview. The test asserts it is scrolled into view so a silent miss cannot pass.
	function emptyCanvasPoint(win) {
		let maxX2 = 0;
		for (const preview of win.app.impress.previewRects)
			maxX2 = Math.max(maxX2, preview.rectangle.x2);
		const first = previewRect(win, 1);
		const point = { x: maxX2 + 2000, y: first.y1 + first.height / 2 };
		const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
		expect(point.x).to.be.lessThan(viewed.x2);
		return point;
	}

	// Set once a drag lands (see dragCanvas below): a real drag inside an entered preview
	// leaves the overview page's own zoom and scroll position changed, and core keeps
	// re-asserting the changed view on every idle round trip afterwards, so there is no
	// client-side call that un-does it. The next ensureOverviewPage call clears it by
	// switching to a slide and back, which re-derives the view from scratch, then resets the
	// flag - so the switch happens only on the one test after a drag, not on every test.
	let viewNeedsRefresh = false;

	function ensureOverviewPage(win) {
		cy.then(function() {
			if (!win.app.impress.hasOverviewPage)
				win.app.map.sendUnoCommand('.uno:InsertCanvasSlide');
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.hasOverviewPage).to.equal(true);
			expect(win.app.impress.previewRects).to.not.equal(null);
		});
		if (viewNeedsRefresh) {
			viewNeedsRefresh = false;
			cy.then(function() {
				win.app.map.setPart(1);
			});
			cy.then(() => helper.processToIdle(win));
			cy.then(function() {
				win.app.map.setPart(0);
			});
			cy.then(() => helper.processToIdle(win));
		}
	}

	// Ctrl-double-clicks the given preview until the mode is entered for it. A plain retry
	// (not just a passive wait) is needed for two reasons: the click sequence arms a 250ms
	// timer that decides whether it is a plain click or a double click, so a check made too
	// soon after the click can still see the pre-click state; and the view can still be
	// settling after a scroll or zoom from a previous test, so the client coordinates
	// computed for the click need recomputing fresh against the current view on every
	// attempt, not just once up front.
	function enterByCtrlDoubleClick(win, part) {
		helper.retryUntil(
			function() {
				cy.then(function() {
					const rect = previewRect(win, part);
					const client = toClient(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
					cy.cGet('#document-canvas').dblclick(client.x, client.y, {
						force: true,
						ctrlKey: true,
					});
				});
				cy.then(() => helper.waitForTimers(win, 'clicktimer'));
				cy.then(() => helper.processToIdle(win));
			},
			function() {
				return cy.wrap(win, { log: false }).then(function(w) {
					return w.app.impress.previewEditMode.active === true &&
						w.app.impress.previewEditMode.part === part;
				});
			},
			{ errorMsg: 'ctrl double click never entered preview edit mode for part ' + part }
		);
	}

	function clickCanvasAtTwips(win, x, y) {
		cy.then(function() {
			const client = toClient(win, x, y);
			cy.cGet('#document-canvas').click(client.x, client.y, { force: true });
		});
		// See the comment in enterByCtrlDoubleClick: let the click's own multi-click
		// timer resolve before the next click reads or changes state.
		cy.then(() => helper.waitForTimers(win, 'clicktimer'));
		cy.then(() => helper.processToIdle(win));
	}

	// Clicks the centered content shape inside the entered preview, retrying until the
	// selection lands. A click that selects it settles in two steps: core reports the
	// selection empty for a moment right after the click, then re-reports the same shape a
	// beat later, by a variable amount, so a fixed number of idle rounds can either read it
	// too early or, waiting past when it lands, too late. Retrying the click itself, not
	// just the wait, also re-reads the entered preview's rectangle fresh on every attempt,
	// the same reason enterByCtrlDoubleClick above recomputes its own click point on every try.
	function selectPreviewShape(win) {
		helper.retryUntil(
			function() {
				cy.then(function() {
					const rect = win.app.impress.previewEditMode.rectangle;
					clickCanvasAtTwips(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
				});
			},
			function() {
				return cy.wrap(win, { log: false }).then(function(w) {
					return w.app.impress.previewShapeSelection !== null;
				});
			},
			{ errorMsg: 'previewShapeSelection never settled on the clicked shape' }
		);
	}

	// Escape leaves preview edit mode, unless a leftover graphic selection on the canvas
	// page counts as inner state for Escape to clear first. A real mouse click or double
	// click on a preview marks its placeholder object on the canvas page as a side effect,
	// on both the click that entered the mode and one that switched it to another preview,
	// so the first Escape after either can end up clearing that selection instead of
	// leaving. Retry the key so the helper leaves the mode regardless of how many presses
	// that takes, rather than assuming a fixed count.
	function leavePreviewEditMode(win) {
		helper.retryUntil(
			function() {
				helper.typeIntoDocument('{esc}');
				cy.then(() => helper.processToIdle(win));
			},
			function() {
				return cy.wrap(win, { log: false }).then(function(w) {
					return w.app.impress.previewEditMode.active === false;
				});
			},
			{ errorMsg: 'Escape never left preview edit mode' }
		);
	}

	// A plain trigger('mousedown'/'mousemove'/'mouseup') on the canvas does not carry the
	// browser's own held-button state from one synthetic event to the next, so the
	// container's drag tracking never sees a move past its tolerance and the shape never
	// budges. realMouseDown/realMouseMove/realMouseUp drive the real input path instead,
	// the same pattern the existing connector and ruler marker drag tests use. Their
	// coordinates are relative to the target element, so the drag targets 'body' (whose
	// rect starts at the viewport origin) with absolute viewport coordinates, converted
	// from the canvas-relative ones toClient returns by adding the canvas element's own
	// bounding rect offset.
	function dragCanvas(win, fromTwips, toTwips) {
		cy.cGet('#document-canvas').then(function($canvas) {
			const canvasRect = $canvas[0].getBoundingClientRect();
			const from = toClient(win, fromTwips.x, fromTwips.y);
			const to = toClient(win, toTwips.x, toTwips.y);
			const fromAbs = { x: canvasRect.left + from.x, y: canvasRect.top + from.y };
			const toAbs = { x: canvasRect.left + to.x, y: canvasRect.top + to.y };
			const middleAbs = {
				x: (fromAbs.x + toAbs.x) / 2,
				y: (fromAbs.y + toAbs.y) / 2,
			};
			cy.cGet('body').realMouseDown({ x: fromAbs.x, y: fromAbs.y });
			cy.cGet('body').realMouseMove(middleAbs.x, middleAbs.y);
			cy.cGet('body').realMouseMove(toAbs.x, toAbs.y);
			cy.cGet('body').realMouseUp({ x: toAbs.x, y: toAbs.y });
		});
		// The release lands back on the canvas, so it fires a trailing click there like any
		// other mouseup, arming the same 250ms multi-click timer described in
		// enterByCtrlDoubleClick above. Let it finish so it does not fold into the next click.
		cy.then(() => helper.waitForTimers(win, 'clicktimer'));
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			viewNeedsRefresh = true;
		});
	}

	it('Hovering a preview shows the move cursor and the outline', function() {
		const win = this.win;
		ensureOverviewPage(win);
		let client;
		cy.then(function() {
			const rect = previewRect(win, 1);
			client = toClient(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
			// The section container drops mousemove until the pointer entered the canvas.
			cy.cGet('#document-canvas').trigger('mouseenter', client.x, client.y, { force: true });
		});
		// The cursor core reports for the point under the pointer can arrive after
		// processToIdle returns, so retry the move until the cursor catches up.
		cy.then(function() {
			helper.retryUntil(
				function() {
					cy.cGet('#document-canvas')
						.trigger('mousemove', client.x, client.y, { force: true });
					cy.then(() => helper.processToIdle(win));
				},
				function() {
					return cy.wrap(win, { log: false }).then(function(w) {
						return w.document.getElementById('document-canvas').style.cursor
							=== 'move';
					});
				},
				{ errorMsg: 'hovering the preview never set the move cursor' }
			);
		});
		cy.then(function() {
			const section = win.app.sectionContainer.getSectionWithName('overviewPreviews');
			expect(section).to.exist;
			expect(section.sectionProperties.hoveredPart).to.equal(1);
			expect(win.document.getElementById('document-canvas').style.cursor)
				.to.equal('move');
		});
	});

	it('A single click selects the preview without core resize handles', function() {
		const win = this.win;
		ensureOverviewPage(win);
		cy.then(function() {
			const rect = previewRect(win, 1);
			clickCanvasAtTwips(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
		});
		cy.then(function() {
			expect(win.app.impress.selectedPreviewPart).to.equal(1);
			expect(win.app.definitions.graphicSelection.hasActiveSelection()).to.equal(true);
			expect(win.app.definitions.graphicSelection.handlesSection).to.equal(null);
		});
		// Clear the selection for the next test.
		helper.typeIntoDocument('{esc}');
		cy.then(() => helper.processToIdle(win));
	});

	it('A ctrl double click enters, a click outside leaves', function() {
		const win = this.win;
		ensureOverviewPage(win);
		enterByCtrlDoubleClick(win, 1);
		cy.then(function() {
			const point = emptyCanvasPoint(win);
			clickCanvasAtTwips(win, point.x, point.y);
		});
		cy.then(function() {
			expect(win.app.impress.previewEditMode.active).to.equal(false);
		});
	});

	it('A plain double click switches to the slide the preview shows', function() {
		const win = this.win;
		ensureOverviewPage(win);
		// The same retry reasoning as enterByCtrlDoubleClick: the click point needs
		// recomputing against the current view on every attempt, and the multi-click
		// timer has to resolve before the outcome is readable.
		helper.retryUntil(
			function() {
				cy.then(function() {
					const rect = previewRect(win, 1);
					const client = toClient(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
					cy.cGet('#document-canvas').dblclick(client.x, client.y, { force: true });
				});
				cy.then(() => helper.waitForTimers(win, 'clicktimer'));
				cy.then(() => helper.processToIdle(win));
			},
			function() {
				return cy.wrap(win, { log: false }).then(function(w) {
					return w.app.map._docLayer._selectedPart === 1;
				});
			},
			{ errorMsg: 'plain double click never switched to the slide' }
		);
		cy.then(function() {
			expect(win.app.impress.previewEditMode.active).to.equal(false);
		});
		// Back to the overview page for the tests that follow.
		cy.then(function() {
			win.app.map.setPart(0);
		});
		cy.then(() => helper.processToIdle(win));
	});

	it('A click on another preview switches the entered preview', function() {
		const win = this.win;
		ensureOverviewPage(win);
		enterByCtrlDoubleClick(win, 1);
		cy.then(function() {
			const rect = previewRect(win, 2);
			clickCanvasAtTwips(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
		});
		cy.then(function() {
			expect(win.app.impress.previewEditMode.active).to.equal(true);
			expect(win.app.impress.previewEditMode.part).to.equal(2);
			expect(win.app.impress.previewShapeSelection).to.equal(null);
		});
		leavePreviewEditMode(win);
	});

	it('The hint bar shows while editing and goes away on leave', function() {
		const win = this.win;
		ensureOverviewPage(win);
		enterByCtrlDoubleClick(win, 1);
		cy.cGet('#snackbar').should('exist');
		cy.cGet('#snackbar').should('contain.text', 'Editing slide 1');
		leavePreviewEditMode(win);
		cy.cGet('body').find('#snackbar').should('not.exist');
	});

	// This test runs before the marquee and corner-grip drag tests below, on top of what
	// those two already leave behind for each other (see dragCanvas above): it drags a shape
	// on the plain canvas page before entering the mode, which leaves the same kind of view
	// state behind as those two tests' own drags.
	it('The drag frame follows a canvas shape dragged in the mode', function() {
		const win = this.win;
		ensureOverviewPage(win);

		// A plain shape, not a text placeholder: selecting a placeholder shows a text cursor
		// immediately, Impress's "click to add text" prompt for an empty placeholder, and
		// MouseControl.showShapeDragPreview does not feed the drag frame while any text
		// cursor is visible, so a placeholder can never exercise the frame this test checks.
		// Insert a plain rectangle on the canvas page, move it clear of every preview, and
		// deselect it, so the click below that marks it while entering the mode is a fresh
		// selection, not a second click on an already-selected shape.
		let shapeCenter;
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:BasicShapes.rectangle');
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			const rectangle = win.app.definitions.graphicSelection.rectangle;
			expect(rectangle).to.not.equal(null);
			const insertedCenter = {
				x: rectangle.x1 + rectangle.width / 2,
				y: rectangle.y1 + rectangle.height / 2,
			};
			shapeCenter = emptyCanvasPoint(win);
			dragCanvas(win, insertedCenter, shapeCenter);
		});
		helper.typeIntoDocument('{esc}');
		cy.then(() => helper.processToIdle(win));

		enterByCtrlDoubleClick(win, 1);

		// Select the plain shape with a click. It sits outside the entered preview, on the
		// wider canvas page, so the in-mode click routing keeps the mode active and marks it
		// as an ordinary selection instead of a preview-page one (see fusel.cxx's
		// FuSelection::MouseButtonDown), then drag it and read the frame offset before
		// releasing.
		cy.then(function() {
			clickCanvasAtTwips(win, shapeCenter.x, shapeCenter.y);
		});
		cy.then(function() {
			expect(win.app.definitions.graphicSelection.hasActiveSelection()).to.equal(true);
			const selection = win.app.definitions.graphicSelection.rectangle;
			shapeCenter = {
				x: selection.x1 + selection.width / 2,
				y: selection.y1 + selection.height / 2,
			};
		});

		// MouseControl.showShapeDragPreview, the code that feeds the dashed drag frame this
		// test checks, only does so once the handles section and its preview SVG have both
		// arrived after the click above, and while no text cursor is visible - true for a
		// plain shape, unlike the placeholder the comment above this test explains. Wait for
		// all of those before the drag below relies on them.
		cy.wrap(win, { log: false }).should(function(w) {
			const selection = w.app.definitions.graphicSelection;
			expect(selection.hasActiveSelection()).to.equal(true);
			const handles = selection.handlesSection;
			expect(handles).to.not.equal(null);
			expect(handles.sectionProperties.svg).to.not.equal(null);
			expect(handles.sectionProperties.hasVideo).to.equal(false);
			expect(selection.extraInfo && selection.extraInfo.isDraggable).to.equal(true);
			expect(w.app.file.textCursor.visible).to.equal(false);
		});

		// Scroll to the shape explicitly, so its canvas-space position maps to an on-screen
		// client point: a real mouse drag (below) needs that, unlike the force-clicks used
		// elsewhere in this spec, which land even off-screen because they bypass the
		// browser's own hit-testing.
		cy.then(function() {
			const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
			win.app.activeDocument.activeLayout.scrollTo(
				shapeCenter.x * win.app.twipsToPixels - viewed.pWidth / 2,
				shapeCenter.y * win.app.twipsToPixels - viewed.pHeight / 2
			);
		});
		cy.then(() => helper.processToIdle(win));

		// See the comment on dragCanvas above: a real mouse drag is needed, not a plain
		// trigger, so this reads the frame mid-drag with the button still held via
		// realMouseDown/realMouseMove, before the matching realMouseUp below.
		cy.cGet('#document-canvas').then(function($canvas) {
			const canvasRect = $canvas[0].getBoundingClientRect();
			const from = toClient(win, shapeCenter.x, shapeCenter.y);
			const fromAbs = { x: canvasRect.left + from.x, y: canvasRect.top + from.y };
			const toAbs = { x: fromAbs.x + 40, y: fromAbs.y + 40 };
			const middleAbs = { x: fromAbs.x + 20, y: fromAbs.y + 20 };
			cy.cGet('body').realMouseDown({ x: fromAbs.x, y: fromAbs.y });
			cy.cGet('body').realMouseMove(middleAbs.x, middleAbs.y);
			cy.cGet('body').realMouseMove(toAbs.x, toAbs.y);
			// cy.wrap(...).should(callback) retries the callback, so a move that has not
			// finished updating the frame yet at the first check gets picked up on a
			// following retry instead of failing outright.
			cy.wrap(win, { log: false }).should(function(w) {
				const handles = w.app.definitions.graphicSelection.handlesSection;
				expect(handles).to.not.equal(null);
				const distance = handles.sectionProperties.lastDragDistance;
				expect(Math.abs(distance[0]) + Math.abs(distance[1])).to.be.greaterThan(0);
			});
			cy.cGet('body').realMouseUp({ x: toAbs.x, y: toAbs.y });
		});
		// See the comment on dragCanvas above: the release fires a trailing click on the
		// canvas, which arms the multi-click timer, so let it finish before moving on.
		cy.then(() => helper.waitForTimers(win, 'clicktimer'));
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			viewNeedsRefresh = true;
		});
		leavePreviewEditMode(win);
	});

	it('Dragging the marquee moves the shape and snaps to the slide edge', function() {
		const win = this.win;
		ensureOverviewPage(win);
		enterByCtrlDoubleClick(win, 1);

		// Select the centered content shape.
		selectPreviewShape(win);

		// The sample slide's centered content shape already sits within a few pixels of
		// the preview's left edge, so aiming for a landing spot near that edge would drag
		// it a distance too small for the container to tell apart from a click (it takes
		// 5 core pixels to register as a drag at all). Send it well past the edge on the
		// far side instead - still inside the snap tolerance - so the drag distance is
		// unambiguous and the snap pulls it back to sit flush with the edge.
		cy.then(function() {
			const selection = win.app.impress.previewShapeSelection;
			const preview = win.app.impress.previewEditMode.rectangle;
			const snapTolerance = Math.round(8 * win.app.pixelsToTwips);
			const overshoot = Math.round(2 * win.app.pixelsToTwips);
			const targetOffset = -(snapTolerance - overshoot);
			const from = {
				x: selection.x1 + selection.width / 2,
				y: selection.y1 + selection.height / 2,
			};
			const to = {
				x: from.x + (targetOffset - (selection.x1 - preview.x1)),
				y: from.y,
			};
			dragCanvas(win, from, to);
		});
		cy.then(function() {
			const selection = win.app.impress.previewShapeSelection;
			const preview = win.app.impress.previewEditMode.rectangle;
			expect(selection).to.not.equal(null);
			expect(Math.abs(selection.x1 - preview.x1)).to.be.at.most(20);
		});
		leavePreviewEditMode(win);
	});

	it('Dragging a corner grip resizes the shape', function() {
		const win = this.win;
		ensureOverviewPage(win);
		enterByCtrlDoubleClick(win, 1);
		selectPreviewShape(win);
		let widthBefore;
		cy.then(function() {
			const selection = win.app.impress.previewShapeSelection;
			expect(selection).to.not.equal(null);
			widthBefore = selection.width;
			const grow = Math.round(30 * win.app.pixelsToTwips);
			dragCanvas(
				win,
				{ x: selection.x2, y: selection.y2 },
				{ x: selection.x2 + grow, y: selection.y2 + grow }
			);
		});
		cy.then(function() {
			const selection = win.app.impress.previewShapeSelection;
			expect(selection).to.not.equal(null);
			expect(selection.width).to.be.greaterThan(widthBefore);
		});
		leavePreviewEditMode(win);
	});
});
