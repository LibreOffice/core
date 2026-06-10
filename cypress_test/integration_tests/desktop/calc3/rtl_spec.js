/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach expect */
var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'RTL sheet tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/rtl-load.ods');
		// The doc loads in notebookbar mode but desktopHelper.insertComment
		// branches on Cypress.env('USER_INTERFACE'), which is only set by
		// switchUIToNotebookbar. Call it here so the env var matches the
		// actual UI - the function is a no-op (no menu click) when the UI
		// is already notebookbar.
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function (win) {
			this.win = win;
			helper.processToIdle(win);
		}.bind(this));
	});

	// Single combined test - intentional. Each step is a discrete RTL
	// invariant we want to keep, but spinning up a new document per
	// assertion costs more than the CI tolerates today. See the spec
	// header comments for rationale; if you add a step, keep it inside
	// this `it` and gate it with processToIdle so flakes stay rare.
	it('RTL Tests', function () {
		const win = this.win;

		// The test document has frozen columns/rows, which makes the
		// non-frozen pane render below/right of the frozen area and
		// pushes row 1 above the viewport for non-frozen-area cells.
		// Toggle FreezePanes off so the whole sheet is one pane and
		// row 1 sits at the top of the canvas - this keeps the layout
		// reads (CellCursor, validity dropdown arrow) free of the
		// frozen-pane offset.
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:FreezePanes');
		});
		helper.processToIdle(win);
		// Ctrl+Home for good measure: navigates to A1 and resets any
		// remaining scroll position.
		helper.typeIntoDocument('{ctrl}{home}');
		helper.processToIdle(win);

		// --- Cell navigation: A1 -> B1.
		// In RTL Calc, B1 sits visually LEFT of A1,
		// so the CellCursor / AutoFillMarker test-div `left` value should
		// DECREASE when moving from A1 to B1.
		calcHelper.enterCellAddressAndConfirm(win, 'A1');

		let cellLeftA1, fillLeftA1;
		cy.cGet('#test-div-OwnCellCursor').then(function ($el) {
			cellLeftA1 = parseFloat($el[0].style.left);
		});
		cy.cGet('[id="test-div-auto fill marker"]').then(function ($el) {
			fillLeftA1 = parseFloat($el[0].style.left);
		});

		calcHelper.enterCellAddressAndConfirm(win, 'B1');

		cy.cGet('#test-div-OwnCellCursor').then(function ($el) {
			const cellLeftB1 = parseFloat($el[0].style.left);
			expect(cellLeftB1, 'CellCursor.left A1 vs B1 in RTL')
				.to.be.lessThan(cellLeftA1);
		});
		cy.cGet('[id="test-div-auto fill marker"]').then(function ($el) {
			const fillLeftB1 = parseFloat($el[0].style.left);
			expect(fillLeftB1, 'AutoFillMarker.left A1 vs B1 in RTL')
				.to.be.lessThan(fillLeftA1);
		});

		// --- Shape: insert, nudge, verify handle moves, delete.
		// .uno:BasicShapes.octagon auto-inserts and selects the shape.
		// Then nudging with an arrow key should change the corner handle's
		// canvas position - we only assert it moved, not the direction
		// (left-arrow vs RTL doc-X handling is core's concern; we just
		// need the section to re-place itself).
		// Press Escape first to make sure we're not in any pending
		// cell-edit / address-input state left over from the previous
		// navigation step. Without this the shape insert sometimes
		// reaches core but the graphicselection round-trip fails to
		// materialize handle subsections in the DOM.
		helper.typeIntoDocument('{esc}');
		helper.processToIdle(win);
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});
		// Wait for core to finish insertion + selection round-trip before
		// querying the per-handle test-divs.
		helper.processToIdle(win);
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		// At least one rectangle scaling handle subsection should be
		// present (core sends 8 of them with ids 0..7 for a freshly
		// selected octagon). We capture whichever appears first so the
		// test stays resilient to id-ordering details.
		cy.cGet('[id^="test-div-shape-handle-"]').should('have.length.greaterThan', 0);

		let handleId;
		let handleLeft0;
		cy.cGet('[id^="test-div-shape-handle-"]').first().then(function ($el) {
			handleId = $el[0].id;
			handleLeft0 = parseFloat($el[0].style.left);
		});

		// A few presses to make the displacement comfortably larger than
		// any sub-pixel rounding in setPosition.
		helper.typeIntoDocument('{leftarrow}{leftarrow}{leftarrow}{leftarrow}{leftarrow}');
		helper.processToIdle(win);

		cy.then(function () {
			cy.cGet('#' + handleId).then(function ($el) {
				const handleLeft1 = parseFloat($el[0].style.left);
				expect(handleLeft1, handleId + '.left after nudge')
					.to.not.equal(handleLeft0);
			});
		});

		// Delete: shape handles section should disappear.
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');

		// --- Validity dropdown on C1.
		calcHelper.enterCellAddressAndConfirm(win, 'C1');
		// Press Escape after address-bar navigation: enterCellAddressAndConfirm
		// types into the AddressInput combobox and presses {enter}, but the
		// combobox can retain focus. With focus stuck on the address bar
		// .uno:Validation reaches core but no dialog message is emitted -
		// confirmed in coolwsd logs as the command going out with no
		// jsdialog response. Escape returns focus to the canvas/cell so the
		// dialog dispatch produces a jsdialog message.
		helper.typeIntoDocument('{esc}');
		helper.processToIdle(win);

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:Validation');
		});
		helper.processToIdle(win);
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);

		// Set Allow -> List, then provide a few entries. The widget ids
		// come from validationcriteriapage.ui (`allow` combo, `minlist`
		// textview). jsdialog wraps the native <select> in a div with the
		// .ui-listbox-container id; the actual <select> sits at
		// `<id>-input` (Widget.Listbox.ts).
		cy.cGet('#allow-input').select('List');
		cy.cGet('#minlist').should('be.visible');
		cy.cGet('#minlist').type('A\nB\nC');
		// OK button closes the dialog.
		cy.cGet('.ui-dialog .button-primary').click();
		cy.cGet('.ui-dialog[role="dialog"]').should('not.exist');
		helper.processToIdle(win);

		// The dropdown arrow div should now exist on the cell - confirms
		// the validity round-trip completed and the validity section was
		// placed. Popup-open + popup-position checks are skipped: the
		// test-div is a pure positioning marker (z-index:-1) and clicking
		// it does not route to the CalcValidityDropDownSection click
		// handler, so popup behaviour needs visual verification.
		cy.cGet('[id="test-div-calc validity dropdown"]').should('exist');

		// In RTL Calc, the most common bug class is sections landing in
		// the wrong place. Check the arrow lands near C1's cell cursor:
		// - Horizontally: it is anchored at the cell's doc-right edge
		//   (cellCursorRectangle.x2 in CanvasTileLayer.js:3218), and in
		//   RTL the doc-right edge mirrors to the visual-left of the
		//   cell on screen. So the arrow's center should sit to the
		//   left of cellRect.left in viewport coords.
		// - Vertically: it should overlap the cell's vertical range,
		//   with one cell-height of slack on each side because the
		//   arrow can hang slightly above/below the cell border
		//   depending on dpiScale and dropDownArrowSize.
		cy.cGet('#test-div-OwnCellCursor').then(function ($cell) {
			const cellRect = $cell[0].getBoundingClientRect();
			cy.cGet('[id="test-div-calc validity dropdown"]').then(function ($arrow) {
				const arrowRect = $arrow[0].getBoundingClientRect();
				const arrowCenterX = arrowRect.left + arrowRect.width / 2;
				expect(arrowCenterX,
					'validity arrow horizontal vs cell.left (RTL: visual-left of cell)')
					.to.be.lessThan(cellRect.left);
				const vTol = cellRect.height;
				expect(arrowRect.top,
					'validity arrow top vs cell vertical band')
					.to.be.greaterThan(cellRect.top - vTol);
				expect(arrowRect.bottom,
					'validity arrow bottom vs cell vertical band')
					.to.be.lessThan(cellRect.bottom + vTol);
			});
		});

		// --- Comment on C4.
		calcHelper.enterCellAddressAndConfirm(win, 'C4');
		helper.typeIntoDocument('{esc}');
		helper.processToIdle(win);
		desktopHelper.insertComment('rtl hover');
		helper.processToIdle(win);

		// The Calc comment container is initially hidden via inline
		// visibility/display; un-hide so we can read its layout
		// position. The container is positioned with translate3d via
		// positionCalcComment, so its bounding rect is valid even
		// without a hover making the content area visible.
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').then(function ($el) {
			$el[0].style.visibility = '';
			$el[0].style.display = '';
		});

		// The fix we made earlier shifts the container by -cellSize so
		// it hangs off the visual-left edge in RTL. Assert the container
		// ended up on the cell's visual-left side (left of cell.right).
		cy.cGet('#test-div-OwnCellCursor').then(function ($cell) {
			const cellRect = $cell[0].getBoundingClientRect();
			cy.cGet('#comment-container-1').then(function ($cont) {
				const r = $cont[0].getBoundingClientRect();
				expect(r.left, 'comment container left vs cell right')
					.to.be.lessThan(cellRect.right);
			});
		});

		// Clean up: delete cell content.
		helper.typeIntoDocument('{del}');
	});
});
