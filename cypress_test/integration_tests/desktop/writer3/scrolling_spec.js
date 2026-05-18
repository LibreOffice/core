/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Scroll through document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/scrolling.odt');
	});

	it('Check if we jump the view on new page insertion', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
		desktopHelper.assertVisiblePage(1, 1, 4);

		helper.typeIntoDocument('{ctrl+enter}');
		helper.typeIntoDocument('{ctrl+enter}');

		cy.wait(500);
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

		cy.cGet('.notebookbar #View-tab-label').click();
		cy.cGet('.notebookbar #View-container .unoControlCodes').click();

		cy.cGet('.notebookbar #View-container .unoControlCodes').should('have.class', 'selected');
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
	});

	it('Drag vertical scrollbar while mouse moves into sidebar area', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);

			var canvas = win.document.getElementById('document-canvas');
			var rect = canvas.getBoundingClientRect();
			var scrollProps = win.app.activeDocument.activeLayout.scrollProperties;
			var dpi = win.app.dpiScale;

			var barX = Math.floor(rect.right - 10);
			var barY = Math.floor(
				rect.top + (scrollProps.startY + scrollProps.verticalScrollSize / 2) / dpi);
			var sidebarX = Math.floor(rect.right + 80);

			// Hover the canvas so mouseIsInside is true on mousedown.
			cy.cGet('body').realMouseMove(barX, barY);
			cy.wrap(null).should(function() {
				expect(win.app.sectionContainer.isMouseInside()).to.be.true;
			});

			cy.cGet('body').realMouseDown({
				pointer: 'mouse', button: 'left',
				x: barX, y: barY,
				scrollBehavior: false
			});

			cy.cGet('body').realMouseMove(barX, barY + 30);
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

});
