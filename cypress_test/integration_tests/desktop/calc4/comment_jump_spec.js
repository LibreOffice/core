/* -*- js-indent-level: 8 -*- */
/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Comment on a cell in the frozen area', function() {

	beforeEach(function() {
		// jumpA1comment.ods has frozen split panes (one frozen row and one
		// frozen column) and a comment on A1, which sits inside the frozen
		// top-left pane. It is saved scrolled down/right so the main pane
		// shows a far-away area while A1 stays pinned in the frozen pane.
		helper.setupAndLoadDocument('calc/jumpA1comment.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	// Reads the full currently viewed rectangle as [x1, y1, width, height]
	// twips, so the before/after comparison covers the exact same condition.
	function getViewRectangle(win) {
		return win.app.activeDocument.activeLayout.viewedRectangle.toArray();
	}

	function getComment(win, text) {
		var section = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.CommentList.name);
		return section.sectionProperties.commentList.find(function (c) {
			return c.sectionProperties.data.text.indexOf(text) !== -1;
		});
	}

	function getA1Comment(win) {
		return getComment(win, 'this is note for A1');
	}

	// Touching the A1 comment (which is visible in the frozen pane) must not
	// scroll the rest of the document. Before the fix the view jumped back to
	// 0,0 even though A1 never left the frozen area.
	function assertViewDoesNotMove(trigger) {
		var before;
		cy.getFrameWindow().then((win) => {
			before = getViewRectangle(win);
			expect(before[0], 'document starts scrolled right').to.be.greaterThan(0);
			expect(before[1], 'document starts scrolled down').to.be.greaterThan(0);
		});

		cy.getFrameWindow().then((win) => {
			trigger(win);
			helper.processToIdle(win);
		});

		// The viewed rectangle must be exactly the same as before.
		cy.getFrameWindow().should((win) => {
			expect(getViewRectangle(win), 'viewed rectangle').to.deep.equal(before);
		});
	}

	it('hovering the comment cell A1 does not move the view', function() {
		assertViewDoesNotMove((win) => { getA1Comment(win).onMouseEnter(); });
	});

	it('selecting the comment cell A1 does not move the view', function() {
		assertViewDoesNotMove((win) => {
			var section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			section.select(getA1Comment(win), true);
		});
	});

	// A547 is in the frozen left column too, but on a scrolling row that is
	// visible in the saved view (next to the "starting position" cell). Column
	// A is frozen, so it is horizontally pinned: hovering its comment must keep
	// the horizontal scroll exactly where it was. Before the fix the view was
	// yanked back to column A (x1 -> 0). A small vertical scroll to reveal the
	// rest of the comment popup is expected and allowed.
	it('hovering the comment in the left frozen column keeps the horizontal position', function() {
		var before;
		cy.getFrameWindow().then((win) => {
			before = getViewRectangle(win);
			expect(before[0], 'document starts scrolled right').to.be.greaterThan(0);
		});

		cy.getFrameWindow().then((win) => {
			getComment(win, 'this is note for A547').onMouseEnter();
			helper.processToIdle(win);
		});

		cy.getFrameWindow().should((win) => {
			var after = getViewRectangle(win);
			expect(after[0], 'horizontal position is preserved').to.equal(before[0]);
		});
	});
});
