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

describe(['tagdesktop'], 'Comment on a cell outside the frozen area', function() {

	beforeEach(function() {
		// comment_frozen_panes.fods holds a comment on E10, the one every test
		// here jumps to, and a second one far away on K60. Each test freezes the
		// rows and columns it needs, so E10 scrolls with both axes.
		helper.setupAndLoadDocument('calc/comment_frozen_panes.fods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	function getCommentCellRectangle(win) {
		const section = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.CommentList.name);
		const comment = section.sectionProperties.commentList.find(
			(candidate) => candidate.sectionProperties.data.id === '1');
		return win.app.map._docLayer._cellRangeToTwipRect(
			comment.sectionProperties.data.cellRange).toRectangle();
	}

	// Freezes at the given cell, so the rows above it and the columns left of it
	// keep their place on screen.
	function freezeAt(win, address) {
		helper.typeIntoInputField(helper.addressInputSelector, address);
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:FreezePanes');
		});
		helper.waitForMapState('.uno:FreezePanes', 'true');
		// The split position arrives with its own message, so let it settle.
		cy.getFrameWindow().should((frameWindow) => {
			expect(frameWindow.app.calc.splitCoordinate.x, 'frozen columns').to.be.greaterThan(0);
			expect(frameWindow.app.calc.splitCoordinate.y, 'frozen rows').to.be.greaterThan(0);
		});
		cy.then(function() { return helper.processToIdle(win); });
	}

	// The comment's cell counts as on screen only when it sits inside one of the
	// panes. The frozen row and column each form their own pane, so a cell can be
	// off screen while sitting between a pane's edge and the split line.
	function commentCellPlacement(win) {
		const cellRectangle = getCommentCellRectangle(win);
		const panes = win.app.getViewRectangles();
		return {
			onScreen: panes.some((pane) => pane.containsRectangle(cellRectangle)),
			description: 'cell ' + JSON.stringify(cellRectangle) + ' is inside one of '
				+ JSON.stringify(panes.map((pane) => pane.toArray())),
		};
	}

	function assertCommentCellIsOnScreen(win) {
		const placement = commentCellPlacement(win);
		expect(placement.onScreen, placement.description).to.be.true;
	}

	function assertCommentCellIsOffScreen(win) {
		const placement = commentCellPlacement(win);
		expect(placement.onScreen, placement.description).to.be.false;
	}

	function scrollFarAway(win) {
		win.app.activeDocument.activeLayout.scroll(4000, 8000);
		return helper.processToIdle(win);
	}

	function goToComment(win) {
		win.postMessage(JSON.stringify({
			MessageId: 'Action_GoToComment',
			Values: { Id: '1' }
		}), '*');
		return helper.processToIdle(win);
	}

	// Both jumps stay in one test on purpose. The second one repeats the command
	// with the cell cursor already on the comment's cell, and only the first jump
	// puts the cursor there. A jump that finds the cursor already in place is the
	// case where nothing but the jump itself can bring the cell back on screen.
	it('jumping to the comment brings its cell on screen', function() {
		const win = this.win;

		freezeAt(win, 'B2');

		// Scrolling has to take the cell away first, or the jump would have
		// nothing to bring back and the check below would hold either way.
		cy.then(function() { return scrollFarAway(win); });
		cy.getFrameWindow().should((frameWindow) => {
			assertCommentCellIsOffScreen(frameWindow);
		});
		cy.then(function() { return goToComment(win); });

		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'E10');
		cy.getFrameWindow().should((frameWindow) => {
			assertCommentCellIsOnScreen(frameWindow);
		});

		cy.then(function() { return scrollFarAway(win); });
		cy.getFrameWindow().should((frameWindow) => {
			assertCommentCellIsOffScreen(frameWindow);
		});
		cy.then(function() { return goToComment(win); });

		cy.cGet('#comment-container-1').should('be.visible');
		cy.getFrameWindow().should((frameWindow) => {
			assertCommentCellIsOnScreen(frameWindow);
		});
	});

	// Freezing at E10 puts the comment's own cell on both split lines. A cell that
	// starts exactly where the frozen rows and columns end belongs to the pane that
	// scrolls, so it goes off screen with it and the jump has to fetch it back.
	it('jumping to a comment on the first cell after the split brings it on screen',
		function() {
			const win = this.win;

			freezeAt(win, 'E10');

			cy.then(function() { return scrollFarAway(win); });
			cy.getFrameWindow().should((frameWindow) => {
				assertCommentCellIsOffScreen(frameWindow);
			});
			cy.then(function() { return goToComment(win); });

			cy.cGet('#comment-container-1').should('be.visible');
			cy.getFrameWindow().should((frameWindow) => {
				assertCommentCellIsOnScreen(frameWindow);
			});
		});

	// The document opens with E10 on screen while the second comment, on K60, is
	// far outside the view. Showing the comments walks the whole list, and no
	// comment in it may drag the view along.
	it('jumping to a comment already on screen leaves the view where it is', function() {
		const win = this.win;
		let before;

		cy.getFrameWindow().then((frameWindow) => {
			assertCommentCellIsOnScreen(frameWindow);
			before = frameWindow.app.activeDocument.activeLayout.viewedRectangle.toArray();
		});

		cy.then(function() { return goToComment(win); });

		cy.cGet('#comment-container-1').should('be.visible');
		cy.getFrameWindow().should((frameWindow) => {
			expect(frameWindow.app.activeDocument.activeLayout.viewedRectangle.toArray(),
				'viewed rectangle').to.deep.equal(before);
		});
	});
});
