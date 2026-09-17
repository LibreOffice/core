/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Writer multi-page view leaves room for the comments.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/scrolling.odt');
		cy.viewport(1920, 1080);
		// The document opens with the notebookbar, but the comment helper picks
		// its menu path from this switch rather than from the document.
		desktopHelper.switchUIToNotebookbar();
	});

	function switchToMultiPageView() {
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutMultiPage');
	}

	// Insert a comment and leave it unselected. A selected comment is pulled out
	// of the column towards the document, which tells us nothing about where the
	// column itself sits.
	function insertCommentAndDeselect() {
		desktopHelper.insertComment();

		cy.getFrameWindow().then(function(win) {
			win.app.sectionContainer
				.getSectionWithName(win.app.CSections.CommentList.name).unselect();
			return helper.processToIdle(win);
		});
	}

	// The page slots and the space around them, in canvas (core) pixels.
	function readLayout(win) {
		var layout = win.app.activeDocument.activeLayout;
		var anchor = win.app.sectionContainer.getDocumentAnchorSection();
		var commentSection = win.app.sectionContainer
			.getSectionWithName(win.app.CSections.CommentList.name);
		var rectangles = layout.viewRectangles;

		var pagesLeft = rectangles[0].pX1;
		var pagesRight = 0;
		var pagesInFirstRow = 0;

		for (var i = 0; i < rectangles.length; i++) {
			pagesLeft = Math.min(pagesLeft, rectangles[i].pX1);
			pagesRight = Math.max(pagesRight, rectangles[i].pX2);
			if (rectangles[i].pY1 === rectangles[0].pY1)
				pagesInFirstRow++;
		}

		// The comment column is placed this far in from the right edge of the view.
		var columnSpace = commentSection.calculateAvailableSpace();

		return {
			zoom: win.app.map.getZoom(),
			floorZoom: win.app.activeDocument.getZoomIndex(60),
			pagesInFirstRow: pagesInFirstRow,
			pagesLeft: pagesLeft,
			pagesRight: pagesRight,
			viewWidth: anchor.size[0],
			scrollableWidth: layout.viewSize.pX,
			scrollsHorizontally: layout.canScrollHorizontal(anchor),
			columnLeft: anchor.size[0] - columnSpace,
			columnSpace: columnSpace,
			commentWidth: commentSection.sectionProperties.commentWidth,
			commentGap: layout.gapBeforeCommentColumn,
			sideMargin: layout.minimumSideMargin,
		};
	}

	it('A comment column fits beside the two pages.', function() {
		switchToMultiPageView();

		var before;
		cy.getFrameWindow().then(function(win) {
			before = readLayout(win);

			expect(before.pagesInFirstRow, 'pages side by side').to.equal(2);
			expect(before.scrollsHorizontally, 'sideways scrolling').to.equal(false);
			expect(before.scrollableWidth - before.pagesRight, 'space right of the pages')
				.to.be.closeTo(before.pagesLeft, 2);
		});

		insertCommentAndDeselect();

		cy.getFrameWindow().should(function(win) {
			var after = readLayout(win);

			expect(after.pagesInFirstRow, 'pages still side by side').to.equal(2);
			expect(after.zoom, 'smaller scale makes room for the column')
				.to.be.lessThan(before.zoom);
			expect(after.columnLeft, 'column starts after the pages')
				.to.be.closeTo(after.pagesRight + after.commentGap, 2);
			expect(after.columnSpace, 'column shown at its full width')
				.to.be.at.least(after.commentWidth);
			expect(after.scrollsHorizontally, 'sideways scrolling').to.equal(false);
			expect(after.scrollableWidth - after.columnLeft - after.commentWidth,
				'space right of the column').to.be.closeTo(after.pagesLeft, 2);
		});

		// What the user sees: the card is drawn beside the pages, not over them.
		cy.cGet('.cool-annotation').last().then(function(card) {
			cy.getFrameWindow().then(function(win) {
				var layout = win.app.activeDocument.activeLayout;
				var canvasBounds = win.app.sectionContainer.getCanvasBoundingClientRect();
				var pagesRight = readLayout(win).pagesRight
					- layout.scrollProperties.viewX
					+ win.app.sectionContainer.getDocumentAnchor()[0];

				expect(card[0].getBoundingClientRect().left, 'card starts right of the pages')
					.to.be.at.least(canvasBounds.left + pagesRight / win.app.dpiScale);
			});
		});
	});

	it('The scale holds at its floor when only the comment column takes it lower.', function() {
		// The open sidebar takes a fixed 329 pixels, which leaves a drawing area
		// where the pages alone are still readable at 60% but the comment column
		// beside them is not.
		cy.viewport(1400, 900);
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		switchToMultiPageView();

		var before;
		cy.getFrameWindow().then(function(win) {
			before = readLayout(win);

			expect(before.zoom, 'scale with no comments').to.equal(before.floorZoom);
			expect(before.scrollsHorizontally, 'sideways scrolling').to.equal(false);
		});

		insertCommentAndDeselect();

		cy.getFrameWindow().should(function(win) {
			var after = readLayout(win);

			expect(after.zoom, 'scale stays at the floor').to.equal(after.floorZoom);
			expect(after.columnLeft, 'column starts after the pages')
				.to.be.closeTo(after.pagesRight + after.commentGap, 2);
			expect(after.scrollsHorizontally, 'the column is reached by scrolling')
				.to.equal(true);
			expect(after.scrollableWidth, 'scrollable area ends just after the column')
				.to.be.closeTo(after.columnLeft + after.commentWidth + after.sideMargin, 2);
		});
	});
});
