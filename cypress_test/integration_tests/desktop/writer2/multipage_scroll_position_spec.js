/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Writer view switch keeps the reading position.', function() {

	beforeEach(function() {
		// The window has its final size at load, so the fit-width zoom is applied then, before
		// the test scrolls.
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/six_pages.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			return helper.processToIdle(win);
		});
	});

	// How far below the top edge of the drawing area the top of the given page
	// sits, in canvas pixels. Zero means the page starts right at the top of the
	// view. Pages are numbered from one.
	function pageTopOffset(win, pageNumber) {
		var layout = win.app.activeDocument.activeLayout;
		var anchor = win.app.sectionContainer.getDocumentAnchorSection();
		var page = win.app.file.writer.pageRectangleList[pageNumber - 1];

		return layout.documentToViewY(new win.cool.SimplePoint(page[0], page[1]))
			- anchor.myTopLeft[1];
	}

	function switchView(win) {
		win.app.dispatcher.dispatch('multipageview');
		return helper.processToIdle(win);
	}

	// Regression test: a view switch built a layout whose scroll position started
	// at zero, so the reader was thrown back to the first page. Repeating the
	// switch also walked the view further down the document each time, because
	// the position was measured after the new layout had already changed the
	// zoom.
	it('Switching to multi-page view and back stays on the page being read.', function() {
		// Read page three: bring it to the top of the view and put the caret on it.
		cy.then(() => {
			this.win.app.activeDocument.activeLayout.scroll(0, pageTopOffset(this.win, 3));
			return helper.processToIdle(this.win);
		});

		cy.cGet('#document-container').click();
		cy.then(() => helper.processToIdle(this.win));

		cy.wrap(null).should(() => {
			expect(pageTopOffset(this.win, 3), 'page three before the switch')
				.to.be.closeTo(0, 4);
		});

		// Two round trips, so that a view creeping down the document on every
		// switch is caught as well as one that jumps back to the start.
		for (var round = 0; round < 2; round++) {
			cy.then(() => switchView(this.win));
			cy.wrap(null).should(() => {
				expect(this.win.app.activeDocument.activeLayout.type)
					.to.equal('ViewLayoutMultiPage');
				expect(pageTopOffset(this.win, 3), 'page three in multi-page view')
					.to.be.closeTo(0, 4);
			});

			cy.then(() => switchView(this.win));
			cy.wrap(null).should(() => {
				expect(this.win.app.activeDocument.activeLayout.type)
					.to.equal('ViewLayoutWriter');
				expect(pageTopOffset(this.win, 3), 'page three in normal view')
					.to.be.closeTo(0, 4);
			});
		}
	});
});
