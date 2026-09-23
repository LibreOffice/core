/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Scroll through document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/scrolling.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Check if we jump the view on new page insertion', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
		desktopHelper.assertVisiblePage(1, 1, 4);

		helper.typeIntoDocument('{ctrl+enter}');
		helper.typeIntoDocument('{ctrl+enter}');

		helper.processToIdle(this.win);
		desktopHelper.assertVisiblePage(2, 3, 6);

		desktopHelper.assertScrollbarPosition('vertical', 120, 250);
	});

	it('Scrolling to bottom/top', function() {
		desktopHelper.selectZoomLevel('40');

		helper.typeIntoDocument('{ctrl}{home}');
		//scroll to bottom
		desktopHelper.assertVisiblePage(1, 1, 4);
		desktopHelper.pressKey(2, 'pagedown');
		desktopHelper.assertVisiblePage(2, 2, 4);
		desktopHelper.pressKey(1, 'pagedown');
		desktopHelper.assertVisiblePage(3, 3, 4);
		desktopHelper.pressKey(1, 'pagedown');
		desktopHelper.assertVisiblePage(4, 4, 4);
		//scroll to top
		desktopHelper.pressKey(1, 'pageup');
		desktopHelper.assertVisiblePage(3, 3, 4);
		desktopHelper.pressKey(1, 'pageup');
		desktopHelper.assertVisiblePage(2, 2, 4);
		desktopHelper.pressKey(2, 'pageup');
		desktopHelper.assertVisiblePage(1, 1, 4);
	});

	// FIXME: from time to time fails with x always 0 (check test div)
	it.skip('Scrolling to left/right', function() {
		desktopHelper.selectZoomLevel('200');
		cy.wait(1000);

		// reset initial position and show horizontal scrollbar
		helper.typeIntoDocument('{home}');
		cy.cGet('#document-container').click('bottom');
		desktopHelper.assertScrollbarPosition('horizontal', 0, 270);

		helper.typeIntoDocument('{home}{end}');
		cy.wait(500);
		desktopHelper.assertScrollbarPosition('horizontal', 430, 653);

		helper.typeIntoDocument('{home}');
		cy.wait(500);
		desktopHelper.assertScrollbarPosition('horizontal', 0, 270);

		helper.typeIntoDocument('{end}{home}{end}');
		cy.wait(500);
		desktopHelper.assertScrollbarPosition('horizontal', 430, 653);
	});

	it('Check if we jump the view on change of formatting mark', function() {
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.selectZoomLevel('40');

		helper.typeIntoDocument('{ctrl}{home}');
		desktopHelper.pressKey(2, 'pagedown');
		desktopHelper.pressKey(1, 'pagedown');
		desktopHelper.assertScrollbarPosition('vertical', 220, 240);

		// cursor on the bottom, scroll to top
		desktopHelper.scrollWriterDocumentToTop();
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);

		// Core reports the initial marks state a few seconds after the load. The click below
		// is the first change after that report, so the client keeps it.
		helper.waitForMapState('.uno:ControlCodes', 'false');

		cy.cGet('.notebookbar #View-tab-label').click();
		cy.cGet('.notebookbar #View-container .unoControlCodes').click();

		cy.cGet('.notebookbar #View-container .unoControlCodes').should('have.class', 'selected');
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
	});

	it('Drag vertical scrollbar while mouse moves into sidebar area', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);

		// The notebookbar styles iconview renders its entries on demand after load, and each
		// render can shift the layout and resize the canvas. Wait until those renders are done
		// and the layout is idle, so the canvas rectangle measured below is the final one.
		helper.processToIdle(this.win);
		helper.waitForOnDemandRenders(this.win);

		cy.then(() => {
			var win = this.win;
			var canvas = win.document.getElementById('document-canvas');
			var rect = canvas.getBoundingClientRect();
			var scrollProps = win.app.activeDocument.activeLayout.scrollProperties;
			var dpi = win.app.dpiScale;

			var barX = Math.floor(rect.right - 10);
			var barY = Math.floor(
				rect.top + (scrollProps.startY + scrollProps.verticalScrollSize / 2) / dpi);
			var sidebarX = Math.floor(rect.right + 80);
			var startScroll = parseInt(
				win.document.querySelector('#test-div-vertical-scrollbar').textContent);

			cy.cGet('body').realMouseDown({
				pointer: 'mouse', button: 'left',
				x: barX, y: barY,
				scrollBehavior: false
			});

			// Re-fire the in-canvas move (the sidebar moves below auto-scroll)
			// until the drag engages and the scrollbar leaves its start.
			helper.retryUntil(
				function() { cy.cGet('body').realMouseMove(barX, barY + 30); },
				function() {
					var el = win.document.querySelector('#test-div-vertical-scrollbar');
					return parseInt(el.textContent) !== startScroll;
				});
			helper.processToIdle(win);

			// Drag continues while the cursor is over the sidebar.
			cy.cGet('body').realMouseMove(sidebarX, barY + 100);
			helper.processToIdle(win);
			cy.cGet('body').realMouseMove(sidebarX, barY + 200);
			helper.processToIdle(win);
			cy.cGet('body').realMouseMove(sidebarX, barY + 300);
			helper.processToIdle(win);

			cy.cGet('body').realMouseUp({ pointer: 'mouse', button: 'left' });
			helper.processToIdle(win);
		});

		desktopHelper.assertScrollbarPosition('vertical', 200, 900);
	});

	it('The view stops following the caret once the user scrolls it out of sight', function() {
		desktopHelper.selectZoomLevel('40');

		// Put the caret on the last page. The view follows its own caret while the
		// caret is on screen.
		helper.typeIntoDocument('{ctrl}{end}');
		helper.processToIdle(this.win);
		cy.then(() => {
			expect(this.win.app.isFollowingUser()).to.equal(true);
		});

		desktopHelper.scrollWriterDocumentToTop();

		cy.then(() => {
			expect(this.win.app.isFollowingOff()).to.equal(true);
		});
	});

	it('The view stays put when the caret is re-reported after a reflow', function() {
		desktopHelper.selectZoomLevel('40');

		helper.typeIntoDocument('{ctrl}{end}');
		helper.processToIdle(this.win);

		desktopHelper.scrollWriterDocumentToTop();

		// Where the user left the view, read from the layout, which carries the
		// position through every scroll.
		let readingPosition;

		// A reflow makes core report the caret again, naming our own view.
		cy.then(() => {
			const layout = this.win.app.activeDocument.activeLayout;
			readingPosition = layout.viewedRectangle.y1;

			const caret = this.win.app.file.textCursor.rectangle;
			const CARET_SHIFT_TWIPS = 15;
			const payload = {
				viewId: this.win.app.map._docLayer._viewId,
				rectangle: (caret.x1 + CARET_SHIFT_TWIPS) + ', ' + caret.y1 + ', ' +
					caret.width + ', ' + caret.height,
			};
			this.win.app.map._docLayer._onMessage(
				'invalidatecursor: ' + JSON.stringify(payload));
		});

		// The user is reading the top of the document, so the view stays there.
		cy.then(() => {
			const layout = this.win.app.activeDocument.activeLayout;
			expect(layout.viewedRectangle.y1).to.equal(readingPosition);
		});
	});

});
