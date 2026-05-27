/* -*- js-indent-level: 8 -*- */
/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Threaded Comment', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/focus.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	// A WOPI host can request a new threaded comment by posting Send_UNO_Command
	// with .uno:InsertThreadedComment. That should enter the in-browser handling,
	// instead of sending the command to engine.
	it('Send_UNO_Command .uno:InsertThreadedComment opens a new comment editor', function() {
		cy.getFrameWindow().then(function(win) {
			const message = {
				MessageId: 'Send_UNO_Command',
				Values: { Command: '.uno:InsertThreadedComment' }
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		// Same expectation as with the toolbar button: the new-comment editor
		// placeholder appears so the user can type before the slot is dispatched.
		cy.cGet('#comment-container-new').should('exist');
		cy.cGet('.cool-annotation').last().find('#annotation-modify-textarea-new')
			.should('exist');
	});

	// Variant where the host ships full Args plus an InteractiveAnchor sentinel.
	// Outside PDF there is no anchor picker, so the browser strips the sentinel
	// and dispatches the command verbatim; engine inserts the threaded comment
	// with the host-provided text and no in-place editor is opened.
	it('Send_UNO_Command .uno:InsertThreadedComment with InteractiveAnchor inserts directly', function() {
		const commentText = 'interactive-anchor-calc ' + Date.now();
		cy.getFrameWindow().then(function(win) {
			win.postMessage(JSON.stringify({
				MessageId: 'Send_UNO_Command',
				Values: {
					Command: '.uno:InsertThreadedComment',
					Args: {
						Author: { type: 'string', value: 'PostMessageBot' },
						Text: { type: 'string', value: commentText },
						InteractiveAnchor: true,
					},
				},
			}), '*');
		});

		// Engine acks a real (non-'new') comment carrying the host text -
		// no #comment-container-new is ever created because newAnnotation
		// is skipped.
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#annotation-modify-textarea-new').should('not.exist');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const comments = section.sectionProperties.commentList;
			expect(comments.length).to.equal(1);
			expect(comments[0].sectionProperties.data.text).to.contain(commentText);
			expect(comments[0].sectionProperties.data.threaded).to.equal('true');
		});
	});

	it('Insert, resolve, unresolve, and remove threaded comment', function() {
		// Click the "Insert Comment" button on the Insert tab.
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#insert-insert-threaded-comment').click();

		// Wait for the annotation to be created.
		cy.cGet('#comment-container-new').should('exist');
		cy.cGet('.cool-annotation').last().find('#annotation-modify-textarea-new')
			.should('exist');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Make the annotation and its container visible.
		cy.cGet('#comment-container-new').then(function (el) {
			el[0].style.visibility = '';
			el[0].style.display = '';
		});

		// Read the author name shown in the new comment dialog.
		var authorName;
		cy.cGet('#comment-container-new .cool-annotation-content-author')
			.invoke('text').then(function (text) { authorName = text; });

		// Type text into the textarea.
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.type('test threaded comment', {force: true});

		// Click Save.
		cy.cGet('.cool-annotation').last().find('[value="Save"]').click({force: true});

		// Wait for the comment to arrive from core with a real ID.
		cy.cGet('#comment-container-1').should('exist');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Verify the comment data has threaded and resolved fields.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			var comments = commentSection.sectionProperties.commentList;
			expect(comments.length).to.equal(1);
			expect(comments[0].sectionProperties.data.threaded).to.equal('true');
			expect(comments[0].sectionProperties.data.resolved).to.equal('false');
		});

		// Verify the author name in the updated comment matches the original.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			var data = commentSection.sectionProperties.commentList[0].sectionProperties.data;
			expect(data.author).to.equal(authorName);
		});

		// Show the comment and open its three-dot menu.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			commentSection.sectionProperties.commentList[0].onMouseEnter();
		});
		cy.cGet('#comment-annotation-menu-1').click();

		// The context menu should contain "Resolve" for threaded comments.
		cy.cGet('#comment-menu-1-dropdown')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Resolve')
			.should('exist');

		// Click "Resolve".
		cy.cGet('#comment-menu-1-dropdown')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Resolve')
			.click();
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Verify the comment is now resolved.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			var data = commentSection.sectionProperties.commentList[0].sectionProperties.data;
			expect(data.resolved).to.equal('true');
		});

		// Show the comment again and open its three-dot menu.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			commentSection.sectionProperties.commentList[0].onMouseEnter();
		});
		cy.cGet('#comment-annotation-menu-1').click();

		// The menu should now show "Unresolve".
		cy.cGet('#comment-menu-1-dropdown')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Unresolve')
			.should('exist');

		// Click "Unresolve".
		cy.cGet('#comment-menu-1-dropdown')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Unresolve')
			.click();
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Verify the comment is unresolved again.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			var data = commentSection.sectionProperties.commentList[0].sectionProperties.data;
			expect(data.resolved).to.equal('false');
		});

		// Show the comment again and open its three-dot menu to remove it.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			commentSection.sectionProperties.commentList[0].onMouseEnter();
		});
		cy.cGet('#comment-annotation-menu-1').click();

		// Click "Remove".
		cy.cGet('#comment-menu-1-dropdown')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove')
			.click();
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Verify the comment is gone.
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('saves a new comment when Ctrl+Enter is pressed', function() {
		// Click the "Insert Comment" button on the Insert tab.
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#insert-insert-threaded-comment').click();

		// Wait for the annotation to be created.
		cy.cGet('#comment-container-new').should('exist');
		cy.cGet('.cool-annotation').last().find('#annotation-modify-textarea-new')
			.should('exist');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Make the annotation and its container visible.
		cy.cGet('#comment-container-new').then(function (el) {
			el[0].style.visibility = '';
			el[0].style.display = '';
		});

		// Type text and submit with Ctrl+Enter (instead of clicking Save).
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.type('comment saved with ctrl+enter{ctrl}{enter}', {force: true});

		// Wait for the comment to arrive from core with a real ID.
		cy.cGet('#comment-container-1').should('exist');
		cy.getFrameWindow().then((win) => { helper.processToIdle(win); });

		// Verify the comment was saved with the typed text.
		cy.getFrameWindow().then((win) => {
			var commentSection = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name
			);
			var comments = commentSection.sectionProperties.commentList;
			expect(comments.length).to.equal(1);
			expect(comments[0].sectionProperties.data.text)
				.to.contain('comment saved with ctrl+enter');
		});
	});
});
