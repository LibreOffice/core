/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Ruler spacer section tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#View-tab-label').click();
	});

	it('RulerSpacer section is created when ruler is shown and removed when hidden', function() {
		// Ensure ruler is hidden initially.
		cy.cGet('#showruler-input').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');

		// Verify ruler spacer test-div does not exist.
		cy.cGet('[id="test-div-ruler spacer"]').should('not.exist');

		// Show the ruler.
		cy.cGet('#showruler-input').check();
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// Verify ruler spacer test-div is created with non-zero height.
		cy.cGet('[id="test-div-ruler spacer"]').should('exist');
		cy.cGet('[id="test-div-ruler spacer"]').then(function($el) {
			var height = parseInt($el.css('height'), 10);
			expect(height).to.be.greaterThan(0);
		});

		// Hide the ruler.
		cy.cGet('#showruler-input').uncheck();
		cy.cGet('#showruler-input').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');

		// Verify ruler spacer test-div is removed.
		cy.cGet('[id="test-div-ruler spacer"]').should('not.exist');
	});

	it('Rulers are hidden from assistive technology', function() {
		// Show the ruler.
		cy.cGet('#showruler-input').check();
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// The horizontal ruler wrapper must have aria-hidden so screen
		// readers skip it.  The accessible alternative is the Paragraph
		// dialog (Format > Paragraph).
		cy.cGet('.cool-ruler:not(.vruler)')
			.should('have.attr', 'aria-hidden', 'true');

		// Same for the vertical ruler.
		cy.cGet('#vertical-ruler')
			.should('have.attr', 'aria-hidden', 'true');
	});
});

describe(['tagdesktop'], 'Ruler position tests.', function() {

	// Three pages, one short paragraph on each, no page header and no paragraph
	// indent, so the caret on a first line sits exactly on the page's text margin.
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/ruler_pages.fodt');
		cy.viewport(1920, 1080);
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#View-tab-label').click();
		cy.cGet('#showruler-input').check();
		cy.cGet('.cool-ruler').should('be.visible');
	});

	// The caret and the ruler band are placed through different code, so allow a
	// few pixels for rounding, for the 2px margin border the band sits on, and for
	// the tile margin the horizontal face still starts from, which is about half a
	// pixel off the margin engine leaves around the pages.
	var alignmentTolerance = 4;

	// The page the caret sits on, counted from the page rectangles engine reports.
	// This is plain containment in document coordinates, so it holds in both views.
	function caretPageShouldBe(pageNumber) {
		cy.getFrameWindow().then(function(win) {
			var caret = win.app.file.textCursor.rectangle;
			var pages = win.app.file.writer.pageRectangleList;
			var index = pages.findIndex(function(page) {
				return caret.y1 >= page[1] && caret.y1 <= page[1] + page[3];
			});

			expect(index + 1, 'page holding the caret').to.equal(pageNumber);
		});
	}

	// A zoom pivots on the middle of the view rather than on the caret, and it
	// does not scroll the caret back into sight; the end of the document is not
	// the start of a line either. A step of the caret puts the view back on it,
	// the way the view follows typing, and Home puts it on the line start.
	function putTheCaretAtTheStartOfItsLine() {
		helper.typeIntoDocument('{rightarrow}');
		helper.typeIntoDocument('{home}');
	}

	// Two pages share the screen when the grid puts them side by side: the second
	// page's slot starts to the right of the first one's, at the same height, and
	// inside the drawing area. All values are canvas (core) pixels.
	function twoPagesShouldShareTheScreen() {
		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			var anchor = win.app.sectionContainer.getDocumentAnchorSection();
			var pages = win.app.file.writer.pageRectangleList;
			var first = layout.documentPointToScreenWithIndex(
				new win.cool.SimplePoint(pages[0][0], pages[0][1]), 0);
			var second = layout.documentPointToScreenWithIndex(
				new win.cool.SimplePoint(pages[1][0], pages[1][1]), 1);

			expect(second.y, 'the two pages sit at the same height')
				.to.be.closeTo(first.y, 2);
			expect(second.x, 'the second page starts right of the first one')
				.to.be.greaterThan(first.x);
			expect(second.x, 'the second page starts inside the drawing area')
				.to.be.lessThan(anchor.myTopLeft[0] + anchor.size[0]);
		});
	}

	// A ruler's tab stop band spans the text area of the page the caret is on: the
	// horizontal band starts at the left text margin, the vertical one at the top
	// text margin. With the caret on the first line of a paragraph that carries no
	// indent, both edges meet the caret, whatever the page, the scale or the view.
	function rulersShouldMeetTheCaret(step) {
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.cGet('.cursor-overlay .blinking-cursor').should(function($caret) {
			expect($caret, step + ': caret is on screen').to.be.visible;
		});

		cy.getFrameWindow().then(function(win) {
			var caret = win.document
				.querySelector('.cursor-overlay .blinking-cursor')
				.getBoundingClientRect();
			var horizontalBand = win.document
				.querySelector('.cool-ruler:not(.vruler) .cool-ruler-horizontal-tabstopcontainer')
				.getBoundingClientRect();
			var verticalBand = win.document
				.querySelector('#vertical-ruler .cool-ruler-tabstopcontainer')
				.getBoundingClientRect();

			expect(horizontalBand.left, step + ': horizontal ruler text area starts at the caret')
				.to.be.closeTo(caret.left, alignmentTolerance);
			expect(verticalBand.top, step + ': vertical ruler text area starts at the caret')
				.to.be.closeTo(caret.top, alignmentTolerance);
		});
	}

	it('Rulers follow the caret across pages, scales and the multi-page view', function() {
		// One paragraph per page, so a single step down from the start of the
		// document lands on the first line of the second page.
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}');
		caretPageShouldBe(2);
		rulersShouldMeetTheCaret('page 2');

		// A larger scale keeps the caret on the same line, and the ruler has to be
		// redrawn at the new scale to stay on it.
		desktopHelper.zoomIn();
		putTheCaretAtTheStartOfItsLine();
		caretPageShouldBe(2);
		rulersShouldMeetTheCaret('page 2, zoomed in');

		// Back to the first page at that same scale.
		helper.typeIntoDocument('{ctrl}{home}');
		caretPageShouldBe(1);
		rulersShouldMeetTheCaret('page 1, zoomed in');

		// A narrower browser tab recentres the page and a wider one moves it back,
		// so the rulers have to follow the page to its new place both ways.
		cy.viewport(1280, 900);
		rulersShouldMeetTheCaret('page 1, narrower window');

		cy.viewport(1920, 1080);
		rulersShouldMeetTheCaret('page 1, wider window');

		// The multi-page view gives every page its own slot on screen, so the
		// rulers are placed from the slot of the page the caret is on.
		cy.cGet('#multi-page-view-button').click();
		cy.getFrameWindow().then(function(win) {
			expect(win.app.activeDocument.activeLayout.type).to.equal('ViewLayoutMultiPage');
		});
		caretPageShouldBe(1);
		rulersShouldMeetTheCaret('page 1, multi-page view');

		// Two pages have to share the screen for the per-page placement to be
		// exercised. The multi-page view puts at most two pages in a row and fits
		// them to the width, so a smaller scale keeps both of them on screen.
		desktopHelper.zoomOut();
		twoPagesShouldShareTheScreen();
		rulersShouldMeetTheCaret('page 1, two pages on screen');

		// The last page sits in the row below, so the rulers have to move to a
		// different slot both horizontally and vertically.
		helper.typeIntoDocument('{ctrl}{end}');
		putTheCaretAtTheStartOfItsLine();
		caretPageShouldBe(3);
		rulersShouldMeetTheCaret('page 3, multi-page view');

		// The grid is packed to the width of the window, so a narrower window hands
		// the page a different slot. A resize does not chase the caret, so the step
		// below brings the view back to it, and the rulers have to be on the page
		// in its new slot. That a resize moves the rulers at all is what the two
		// checks in the normal view above establish.
		cy.viewport(1280, 900);
		putTheCaretAtTheStartOfItsLine();
		caretPageShouldBe(3);
		rulersShouldMeetTheCaret('page 3, multi-page view, narrower window');
	});
});
