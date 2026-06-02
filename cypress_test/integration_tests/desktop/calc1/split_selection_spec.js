/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

// NOTE ON LONGEVITY:
// This test relies on the current architecture where Calc reuses the generic
// ViewLayout (browser/src/app/ViewLayout.ts). Its documentToViewX/Y are plain
// linear transforms with no split-pane awareness, so the frozen-pane offset is
// applied manually, per point, inside TextSelectionSection.onDraw. That manual
// offset is exactly what we capture and assert here.
//
// When Calc gets its own ViewLayout subclass that folds the split-pane math
// into documentToViewX/Y, the manual offset in onDraw will move there or
// disappear. At that point this test must be updated: the spy target and the
// "frozen stays / moving moves" derivation below need revisiting, even though
// the user-visible invariant (frozen part stays anchored, moving part scrolls)
// stays the same.

describe(['tagdesktop'], 'Calc selection drawing with frozen panes', function() {

	beforeEach(function() {
		// A plain sheet: cell_cursor.ods has a huge merged cell at A1 that makes
		// the view jump on selection, pushing the whole selection out of the
		// visible moving pane.
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// The active view's selection overlay is the only *-text-selections section
	// that currently holds polygons.
	function getSelectionSection(win) {
		return win.app.sectionContainer.sections.find(
			(s) =>
				s.name.endsWith('-text-selections') &&
				s.polygons &&
				s.polygons.length > 0 &&
				s.polygons[0].length > 0
		);
	}

	// Drive the section's onDraw synchronously and record the exact view-pixel
	// coordinates it feeds to the canvas (moveTo/lineTo). Calling onDraw directly
	// avoids the redraw scheduler and shouldDrawSection gating; the captured
	// argument values are the vX/vY view pixels, independent of the container's
	// translate, so a standalone call yields the same coordinates as a real draw.
	function captureDraw(section) {
		const ctx = section.context;
		const realMoveTo = ctx.moveTo;
		const realLineTo = ctx.lineTo;
		const points = [];
		ctx.moveTo = function (x, y) {
			points.push({ x: x, y: y });
			return realMoveTo.call(ctx, x, y);
		};
		ctx.lineTo = function (x, y) {
			points.push({ x: x, y: y });
			return realLineTo.call(ctx, x, y);
		};
		try {
			section.onDraw();
		} finally {
			ctx.moveTo = realMoveTo;
			ctx.lineTo = realLineTo;
		}
		return points;
	}

	it('Selection crossing frozen rows and columns is drawn correctly after scroll', function() {
		const win = this.win;
		const TOL = 2; // view-pixel rounding tolerance.

		// 1. Freeze at C3 so both rows (1,2) and columns (A,B) are frozen, i.e.
		//    splitCoordinate.x !== 0 and splitCoordinate.y !== 0.
		helper.typeIntoInputField(helper.addressInputSelector, 'C3');
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:FreezePanes');
		});
		helper.waitForMapState('.uno:FreezePanes', 'true');

		cy.then(function () {
			expect(win.app.calc.splitCoordinate, 'splitCoordinate').to.not.be.null;
			expect(win.app.calc.splitCoordinate.x, 'frozen columns').to.be.greaterThan(0);
			expect(win.app.calc.splitCoordinate.y, 'frozen rows').to.be.greaterThan(0);
		});

		// 2. Normalize the view to the top-left, then select a range that crosses
		//    both split lines and extends well into the moving pane (so plenty of
		//    points stay in the moving area after a small scroll).
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		calcHelper.selectCellsInRange('A1:N40');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// Entering a range in the Name Box scrolls the view to the selection's
		// far corner; bring it back to the top-left so the selection extends far
		// into the moving pane on both axes.
		cy.then(function () {
			win.app.activeDocument.activeLayout.scrollTo(0, 0);
			return helper.processToIdle(win);
		});

		// Verification A (split logic, render-independent):
		// The crossing rectangle must be cut at the split lines into separate
		// polygons, and polygon edges must sit exactly on splitCoordinate.
		cy.then(function () {
			const section = getSelectionSection(win);
			expect(section, 'selection section with polygons').to.not.be.undefined;

			// Crossing both lines splits the single rectangle into 4 groups.
			expect(section.polygons.length, 'polygon count').to.be.greaterThan(1);

			let edgeOnSplitX = false;
			let edgeOnSplitY = false;
			const splitX = win.app.calc.splitCoordinate.x;
			const splitY = win.app.calc.splitCoordinate.y;
			for (const polygon of section.polygons) {
				for (const p of polygon) {
					if (Math.abs(p.x - splitX) < 2) edgeOnSplitX = true;
					if (Math.abs(p.y - splitY) < 2) edgeOnSplitY = true;
				}
			}
			expect(edgeOnSplitX, 'a polygon edge sits on the column split line').to.be.true;
			expect(edgeOnSplitY, 'a polygon edge sits on the row split line').to.be.true;
		});

		// Verification B (drawn geometry invariant):
		// Capture the view-pixel coordinates before and after a small scroll.
		// Frozen-pane corners must stay put; moving-pane corners must shift by
		// exactly the moving pane's scroll delta. Before the fix a single
		// deflection was applied to the whole polygon, so all points moved
		// together - making it impossible to have both a fixed and a moving
		// point at once.
		let before;
		let viewBefore;
		cy.then(function () {
			const section = getSelectionSection(win);
			expect(section, 'selection section for draw capture').to.not.be.undefined;
			before = captureDraw(section);
			expect(before.length, 'captured draw points before scroll').to.be.greaterThan(0);
			const vr = win.app.activeDocument.activeLayout.viewedRectangle;
			viewBefore = { pX1: vr.pX1, pY1: vr.pY1 };
		});

		// Scroll the moving pane slightly down and to the right.
		cy.then(function () {
			win.L.Map.THIS.panBy({ x: 200, y: 200 });
			return helper.processToIdle(win);
		});

		cy.then(function () {
			const section = getSelectionSection(win);
			const after = captureDraw(section);
			const vr = win.app.activeDocument.activeLayout.viewedRectangle;

			// documentToViewX/Y subtract viewedRectangle, so a moving point's
			// view coordinate changes by the negative of the scroll delta.
			const movingDeltaX = -(vr.pX1 - viewBefore.pX1);
			const movingDeltaY = -(vr.pY1 - viewBefore.pY1);

			expect(Math.abs(movingDeltaX), 'horizontal scroll happened').to.be.greaterThan(TOL);
			expect(Math.abs(movingDeltaY), 'vertical scroll happened').to.be.greaterThan(TOL);
			expect(after.length, 'same polygon structure across draws').to.equal(before.length);

			let fixedX = false, movedX = false, fixedY = false, movedY = false;
			for (let i = 0; i < before.length; i++) {
				const dx = after[i].x - before[i].x;
				const dy = after[i].y - before[i].y;

				if (Math.abs(dx) <= TOL) fixedX = true;
				else if (Math.abs(dx - movingDeltaX) <= TOL) movedX = true;

				if (Math.abs(dy) <= TOL) fixedY = true;
				else if (Math.abs(dy - movingDeltaY) <= TOL) movedY = true;
			}

			expect(fixedX, 'a frozen-column point stayed anchored').to.be.true;
			expect(movedX, 'a moving-pane point tracked the horizontal scroll').to.be.true;
			expect(fixedY, 'a frozen-row point stayed anchored').to.be.true;
			expect(movedY, 'a moving-pane point tracked the vertical scroll').to.be.true;
		});
	});
});
