/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Annotation Tests', function() {

	beforeEach(function() {
		cy.viewport(1400, 600);
		helper.setupAndLoadDocument('writer/annotation.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.sidebarToggle();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
	});

	it('Modify', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0, some other text');
	});

	it('Reply', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#annotation-reply-1').click();
		cy.cGet('#annotation-content-area-2').should('contain','some reply text');
	});

	it('Remove', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove').click();
		cy.cGet('.cool-annotation-content-wrapper').should('not.exist');
	});

	it('Click on comment emits Clicked_Comment postMessage', function() {
		desktopHelper.insertComment();

		// This will record usage of window.postMessage (called from
		// _postMessage in browser/src/map/handler/Map.WOPI.js
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// <div class="cool-annotation-content-wrapper" ...> is the topmost element of the comment
		cy.cGet('.cool-annotation-content-wrapper').should('be.visible');
		cy.cGet('.cool-annotation-content-wrapper').click();

		cy.get('@postMessage').should(stub => {
			const found = stub.getCalls().some(call => {
				const msg = JSON.parse(call.args[0]);
				return msg.MessageId === 'Clicked_Comment'
					&& msg.Values && msg.Values.Id !== undefined;
			});
			expect(found, "Clicked_Comment was not posted").to.be.true;
		});
	});

	it('Action_ResolveComment postMessage resolves a comment', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('be.visible');
		cy.cGet('.cool-annotation-content-resolved').should('have.text', '');

		// Send Action_ResolveComment postMessage with the comment's Id
		cy.getFrameWindow().then(win => {
			const message = {
				'MessageId': 'Action_ResolveComment',
				'Values': {'Id': '1'}
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		// The comment should now show as resolved
		cy.cGet('.cool-annotation-content-resolved').should('have.text', 'Resolved');
	});

	it('Toggle Resolved/Unresolved', function() {
		desktopHelper.insertComment("unresolved comment", true);
		cy.cGet('#comment-container-1').should('exist');
		/*
			after the last `insertComment` call the insert tab is selected.
			if we don't change the tab and call `insertComment` again, then
			it will collapse the notebookbar (clicking on the same tab twice).
			to avoid the 'collapsed notebookbar' state, we click on the home
			tab to 'reset' the state for the next `insertComment` call.
		*/
		cy.cGet('#Home-tab-label').click();

		desktopHelper.insertComment("resolved comment", true);
		cy.cGet('body').type('focus out of comments');
		cy.cGet('#comment-container-2').should('exist');
		cy.cGet('#comment-annotation-menu-2').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Resolve').click();
		cy.cGet('.cool-annotation-content-resolved').should('exist');

		/* scenario 1:
		 *   - hide all comments -> all hidden
		 *   - show all comments -> all visible
		 */
		desktopHelper.toggleComments();
		cy.cGet('#comment-container-1').should('be.not.visible');
		cy.cGet('#comment-container-2').should('be.not.visible');
		desktopHelper.toggleComments();
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet('#comment-container-2').should('be.visible');

		/* scenario 2:
		 *   - hide resolved comments -> resolved comment hidden
		 *   - hide all comments 	  -> both hidden
		 *   - show all comments 	  -> resolved comment hidden
		 */
		desktopHelper.toggleComments(/*resolved = */ true);
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet('#comment-container-2').should('be.not.visible');
		desktopHelper.toggleComments();
		cy.cGet('#comment-container-1').should('be.not.visible');
		cy.cGet('#comment-container-2').should('be.not.visible');
		desktopHelper.toggleComments();
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet('#comment-container-2').should('be.not.visible');
	});

	it('Visibility at Different Zoom Levels', function() {
		/*
			1. insert comment at 50% zoom level
			2. then keep increasing the zoom level and assert comment visibility.
			3. visible at 100% and 120%, and hidden (collapsed) at 150%
		*/
		desktopHelper.selectZoomLevel('100', false);
		desktopHelper.insertComment('test comment', true);
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('120', false);
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('150', false);
		cy.cGet('#comment-container-1').should('be.not.visible');
	});

	it('Visibility at Different Window Widths (increasing)', function() {
		/*
			1. start with collapsed comment and increase window width
			2. cy.viewport(1400, 600); at 150% comment is collapsed
			3. increase width by 20 and assert visibility
		*/
		desktopHelper.insertComment('test comment', true);
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('150', false);
		cy.cGet('#comment-container-1').should('be.not.visible');

		for (let width = 1420; width < 1500; width += 20) {
			cy.viewport(width, 600);
			cy.cGet('#comment-container-1').should('be.visible');
		}

		for (let width = 1500; width < 1620; width += 20) {
			cy.viewport(width, 600);
			cy.cGet('#comment-container-1').should('be.visible');
		}
	});

	it('Visibility at Different Window Widths (decreasing)', function() {
		/*
			1. start with wide window (== zoomed out document) and reduce window width
			2. cy.viewport(1400, 600); at 100% comments are visible
			3. decrease width by 10 and assert visibility
		*/
		desktopHelper.insertComment('test comment', true);
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('120', false);
		cy.cGet('#comment-container-1').should('be.visible');

		for (let width = 1420; width > 1260; width -= 20) {
			cy.viewport(width, 600);
			cy.cGet('#comment-container-1').should('be.visible');
		}
	});

	it('Visibility on Small Resizes (1px width increase/decrease)', function() {
		desktopHelper.insertComment('test comment', true);
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('120', false);
		cy.cGet('#comment-container-1').should('be.visible');

		for (let width = 1420; width > 1300; width -= 1) {
			cy.viewport(width, 600);
			cy.cGet('#comment-container-1').should('be.visible');
		}

		for (let width = 1300; width < 1420; width += 1) {
			cy.viewport(width, 600);
			cy.cGet('#comment-container-1').should('be.visible');
		}
	});

	it('Collapse/Expand On Last 1px Resize', function() {
		desktopHelper.insertComment('test comment', true);
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').should('be.visible');

		desktopHelper.selectZoomLevel('120', false);
		cy.cGet('#comment-container-1').should('be.visible');

		/*
			at this point, the space on the left of the document and the 
			space on the right of the document (without moving the document
			to the left) is same, equal to half of the comment width;
		*/
		cy.viewport(1285, 600);
		cy.cGet('#comment-container-1').should('be.visible');

		/*
			we reduce the width by just one pixel at this point, and
			`haveEnoughLeftMarginForMove` becomes false in
			`ViewLayoutWriter.documentCanMoveLeft(...)` and thus document
			can't move left anymore, so we collapse the comments.
		*/
		cy.viewport(1284, 600);
		cy.cGet('#comment-container-1').should('be.visible');

		cy.viewport(1285, 600);
		cy.cGet('#comment-container-1').should('be.visible');
	});

	it('Tab Navigation', function() {
		desktopHelper.insertComment(undefined, false);

		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');
		cy.realPress('Tab');
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');
		cy.cGet('#annotation-cancel-new:focus-visible');

		cy.realPress('Tab');
		cy.cGet('#annotation-save-new:focus-visible');
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');

		cy.realPress('Tab');
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
	});

	it('Global opreations without doc focused', function () {
		cy.getFrameWindow().then(function (win) {
			cy.spy(win.app.socket, 'sendMessage').as('sendMessage');
		});
		cy.getFrameWindow().then(function (win) {
			cy.stub(win, 'open').as('windowOpen');
		});

		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').click();

		cy.cGet('body').type('{ctrl}p');


		const downloadAsMessage = 'downloadas ' +
			'name=print.pdf ' +
			'id=print ' +
			'format=pdf ' +
			'options={\"ExportFormFields\":{\"type\":\"boolean\",\"value\":\"false\"},' +
			'\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}';
		cy.get('@sendMessage').should('have.been.calledWith', downloadAsMessage);
		cy.get('@windowOpen').should('be.called');
	});

	it('Action_GoToComment postMessage navigates to a comment', function() {
		// Type identifiable text on the first line where the comment will be.
		helper.typeIntoDocument('COMMENT_ANCHOR_LINE');

		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('be.visible');
		cy.cGet('#annotation-content-area-1').should('contain', 'some text0');

		// Type lots of paragraph breaks so the document scrolls well past the comment.
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{enter}'.repeat(80) + 'BOTTOM_OF_DOCUMENT');

		// The comment should now be scrolled out of view.
		cy.cGet('#comment-container-1').should('not.be.visible');

		// Stub postMessage to capture the response.
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// Send Action_GoToComment postMessage with the comment's Id.
		cy.getFrameWindow().then(win => {
			var message = {
				'MessageId': 'Action_GoToComment',
				'Values': { 'Id': '1' }
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		// Verify the response postMessage was sent with success and no error.
		cy.get('@postMessage').should(stub => {
			var calls = stub.getCalls().filter(call => {
				try {
					var msg = typeof call.args[0] === 'string' ? JSON.parse(call.args[0]) : call.args[0];
					return msg.MessageId === 'Action_GoToComment_Resp';
				} catch (e) { return false; }
			});
			expect(calls.length, 'Action_GoToComment_Resp was not posted').to.be.greaterThan(0);
			var resp = typeof calls[0].args[0] === 'string' ? JSON.parse(calls[0].args[0]) : calls[0].args[0];
			expect(resp.Values.success, 'Action_GoToComment_Resp reported error: ' + resp.Values.errorMsg).to.be.true;
			expect(resp.Values.Id).to.equal('1');
		});

		// After GoToComment, the comment should be scrolled back into view.
		cy.cGet('#comment-container-1').should('be.visible');
		// The cursor should be at the end of the first paragraph (the comment anchor).
		// #clipboard-area has a copy of current cursor's node text (including anchor character):
		cy.cGet('#clipboard-area').should('have.prop', 'textContent', 'COMMENT_ANCHOR_LINE\uFFFC');
		cy.getFrameWindow().then(win => {
			var textInput = win.app.map._textInput;
			expect(textInput._lastSelectionStart).to.equal(20);
			expect(textInput._lastSelectionEnd).to.equal(20);
		});
	});

	it('Action_GoToComment postMessage returns error for invalid comment', function() {
		// Stub postMessage to capture the response.
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// Send Action_GoToComment with a non-existent comment Id.
		cy.getFrameWindow().then(win => {
			var message = {
				'MessageId': 'Action_GoToComment',
				'Values': { 'Id': '999' }
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		// Verify error response was sent.
		cy.get('@postMessage').should(stub => {
			var found = stub.getCalls().some(call => {
				try {
					var msg = typeof call.args[0] === 'string' ? JSON.parse(call.args[0]) : call.args[0];
					return msg.MessageId === 'Action_GoToComment_Resp'
						&& msg.Values && msg.Values.success === false
						&& msg.Values.Id === '999';
				} catch (e) { return false; }
			});
			expect(found, 'Action_GoToComment_Resp with failure was not posted').to.be.true;
		});
	});

	it('Annotation minimum width', function () {
		cy.viewport(1920, 1080); // Let's have plenty of space
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain', 'some text');

		// Add several nested replies to create a deep thread
		const replyCount = 5;
		for (let i = 1; i <= replyCount; i++) {
			cy.cGet('#comment-annotation-menu-' + i).click();
			cy.cGet('body').contains('.ui-combobox-entry', 'Reply').click();
			cy.cGet('#annotation-reply-textarea-' + i).type('reply ' + i);
			cy.cGet('#annotation-reply-' + i).click();
			cy.cGet('#annotation-content-area-' + (i + 1)).should('contain', 'reply ' + i);
		}

		// Check that the last reply content is wide enough
		cy.cGet('#comment-container-' + (replyCount + 1) + ' .cool-annotation-content')
			.should(el => expect(el.width()).gte(200));
	});

	it('Get_Comments postMessage returns all comments', function() {
		desktopHelper.insertComment('first comment');
		cy.cGet('#annotation-content-area-1').should('contain', 'first comment');

		// Avoid notebookbar collapse before the second insertComment.
		cy.cGet('#Home-tab-label').click();
		helper.typeIntoDocument('{end}{enter}');

		desktopHelper.insertComment('second comment');
		cy.cGet('#annotation-content-area-2').should('contain', 'second comment');

		// Stub postMessage to capture the response.
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// Send Get_Comments postMessage.
		cy.getFrameWindow().then(win => {
			const message = { 'MessageId': 'Get_Comments' };
			win.postMessage(JSON.stringify(message), '*');
		});

		// Verify the response contains both comments.
		cy.get('@postMessage').should(stub => {
			const calls = stub.getCalls().filter(call => {
				try {
					const msg = typeof call.args[0] === 'string' ? JSON.parse(call.args[0]) : call.args[0];
					return msg.MessageId === 'Get_Comments_Resp';
				} catch (e) { return false; }
			});
			expect(calls.length, 'Get_Comments_Resp was not posted').to.be.greaterThan(0);
			const resp = typeof calls[0].args[0] === 'string' ? JSON.parse(calls[0].args[0]) : calls[0].args[0];
			const comments = resp.Values.Comments;
			expect(comments.length).to.equal(2);
			expect(comments[0].Id).to.equal('1');
			expect(comments[0].Text).to.equal('first comment');
			expect(comments[0]).to.have.property('Author');
			expect(comments[0]).to.have.property('DateTime');
			expect(comments[0].Resolved).to.equal('false');
			expect(comments[0].Parent).to.equal('0');
			expect(comments[1].Id).to.equal('2');
			expect(comments[1].Text).to.equal('second comment');
		});
	});

});

describe(['tagdesktop'], 'Collapsed Annotation Tests', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('writer/annotation.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.sidebarToggle();
	});

	it('Insert', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
	});

	it('Modify', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0, some other text');
	});

	it('Reply', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text');
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#annotation-reply-1').click();
		cy.cGet('#annotation-content-area-2').should('contain','some reply text');
	});

	it('Remove', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('.cool-annotation-content > div').should('contain','some text');
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('.cool-annotation-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove').click();
		cy.cGet('.cool-annotation-content-wrapper').should('not.exist');
	});

	it('Autosave Collapse', function() {
		desktopHelper.selectZoomLevel('100', false);
		helper.typeIntoDocument('placeholder text');
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		helper.typeIntoDocument('{home}');
		cy.cGet('.cool-annotation-info-collapsed').should('have.text','!');
		cy.cGet('.cool-annotation-info-collapsed').should('be.not.visible');
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		helper.typeIntoDocument('{home}');
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('.cool-annotation-info-collapsed').should('not.have.text','!');
		cy.cGet('#map').focus();
		helper.typeIntoDocument('{home}');
		cy.cGet('.cool-annotation-info-collapsed').should('be.not.visible');

		helper.reloadDocument(newFilePath);
		desktopHelper.sidebarToggle();
		cy.cGet('.cool-annotation-img').click();
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.cool-annotation-info-collapsed').should('be.not.visible');
	})

});

describe(['tagdesktop'], 'Annotation Autosave Tests', function() {
	var newFilePath;

	beforeEach(function() {
		cy.viewport(1400, 600);
		newFilePath = helper.setupAndLoadDocument('writer/annotation.odt');
		desktopHelper.switchUIToNotebookbar();
		// TODO: skip sidebar detection on reload
		// desktopHelper.sidebarToggle();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert autosave', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});

	it('Insert autosave save', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});

	it('Insert autosave cancel', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('#comment-container-1').should('not.exist');
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('not.exist');
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('Modify autosave', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
	});

	it('Modify autosave save', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
	});

	it('Modify autosave cancel', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.not.visible');
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});

	it('Reply autosave', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('#annotation-modify-textarea-2').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-2').should('have.text','some reply text');
	});

	it('Reply autosave save', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('#annotation-modify-textarea-2').should('be.visible');
		cy.cGet('#annotation-modify-textarea-2').should('have.text','some reply text');
		cy.cGet('#annotation-save-2').click();
		cy.cGet('#annotation-modify-textarea-2').should('be.not.visible');
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#annotation-content-area-2').should('have.text','some reply text');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-2').should('have.text','some reply text');
	});

	it('Reply autosave cancel', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('#annotation-modify-textarea-2').should('be.visible');
		cy.cGet('#annotation-modify-textarea-2').should('have.text','some reply text');
		cy.cGet('#annotation-cancel-2').click();
		cy.cGet('#annotation-modify-textarea-2').should('not.exist');
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#annotation-content-area-2').should('not.exist');
		cy.cGet('#comment-container-1 .annotation-button-autosaved').should('not.exist');
		cy.cGet('#comment-container-1 .annotation-button-delete').should('not.exist');
		cy.cGet('#comment-container-2 .annotation-button-autosaved').should('not.exist');
		cy.cGet('#comment-container-2 .annotation-button-delete').should('not.exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#annotation-content-area-2').should('not.exist');
	});
});

describe(['tagdesktop'], 'Annotation with @mention', function() {
	beforeEach(function() {
		cy.viewport(1400, 600);
		helper.setupAndLoadDocument('writer/annotation.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.sidebarToggle();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert comment with mention', function() {
		desktopHelper.insertComment('some text0', false);

		cy.cGet('.cool-annotation').find('#annotation-modify-textarea-new').type(' @Ale');
		cy.cGet('#mentionPopup').should('be.visible');
		cy.cGet('#mentionPopupList .ui-treeview-entry:nth-child(1)').click();

		cy.cGet('#annotation-modify-textarea-new a').should('exist');
		cy.cGet('#annotation-modify-textarea-new a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-modify-textarea-new a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-modify-textarea-new').should('have.text','some text0 @Alexandra\u00A0');

		cy.cGet('#annotation-save-new').click();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1 a').should('exist');
		cy.cGet('#annotation-content-area-1 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-content-area-1 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0 @Alexandra ');
	});

	it('Modify comment by adding mention', function () {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain', 'some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();

		cy.cGet('#annotation-modify-textarea-1').type('{end}');
		cy.cGet('#annotation-modify-textarea-1').type(' @Ale');
		cy.cGet('#mentionPopup').should('be.visible');
		cy.cGet('#mentionPopupList .ui-treeview-entry:nth-child(1)').click();

		cy.cGet('#annotation-modify-textarea-1 a').should('exist');
		cy.cGet('#annotation-modify-textarea-1 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-modify-textarea-1 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-modify-textarea-1').should('have.text', 'some text0 @Alexandra\u00A0');

		cy.cGet('#annotation-save-1').click();
		cy.cGet('.cool-annotation-content-wrapper').should('exist');

		cy.cGet('#annotation-content-area-1 a').should('exist');
		cy.cGet('#annotation-content-area-1 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-content-area-1 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text0 @Alexandra ');
	})

	it('Reply to parent comment by adding mention', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();

		cy.cGet('#annotation-reply-textarea-1').type('some reply text @Ale');

		cy.cGet('#mentionPopup').should('be.visible');
		cy.cGet('#mentionPopupList .ui-treeview-entry:nth-child(1)').click();

		cy.cGet('#annotation-reply-textarea-1 a').should('exist');
		cy.cGet('#annotation-reply-textarea-1 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-reply-textarea-1 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-reply-textarea-1').should('have.text', 'some reply text @Alexandra\u00A0');

		cy.cGet('#annotation-reply-1').click();
		cy.cGet('#annotation-content-area-2').should('contain','some reply text @Alexandra ');
	});

	it('Reply to reply comment by adding mention', function() {
		desktopHelper.insertComment();

		cy.cGet('.cool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();

		cy.cGet('#annotation-reply-textarea-1').type('some reply text @Ale');

		cy.cGet('#mentionPopup').should('be.visible');
		cy.cGet('#mentionPopupList .ui-treeview-entry:nth-child(1)').type('{enter}');

		cy.cGet('#annotation-reply-textarea-1 a').should('exist');
		cy.cGet('#annotation-reply-textarea-1 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-reply-textarea-1 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-reply-textarea-1').should('have.text', 'some reply text @Alexandra\u00A0');

		cy.cGet('#annotation-reply-1').click();
		cy.cGet('#annotation-content-area-2').should('contain','some reply text @Alexandra ');

		cy.cGet('#comment-annotation-menu-2').should('exist').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').click();
		cy.cGet('#annotation-reply-textarea-2').type('some reply to reply text @Ale');

		cy.cGet('#mentionPopup').should('be.visible');
		cy.cGet('#mentionPopupList .ui-treeview-entry:nth-child(1)').type('{enter}');

		cy.cGet('#annotation-reply-textarea-2 a').should('exist');
		cy.cGet('#annotation-reply-textarea-2 a').should('have.text', '@Alexandra');
		cy.cGet('#annotation-reply-textarea-2 a').should('have.attr', 'href', 'https://github.com/CollaboraOnline/online');
		cy.cGet('#annotation-reply-textarea-2').should('have.text', 'some reply to reply text @Alexandra\u00A0');

		cy.cGet('#annotation-reply-2').click();
		cy.cGet('#annotation-content-area-3').should('contain','some reply to reply text @Alexandra ');
	});

	it('Escape should close the mentionPopup, comment should be in focus', function() {
		desktopHelper.insertComment('some text0', false);

		cy.cGet('.cool-annotation').find('#annotation-modify-textarea-new').type(' @Ale');
		cy.cGet('#mentionPopup').should('be.visible');
		helper.typeIntoDocument('{esc}');

		cy.cGet('#mentionPopup').should('not.exist');
		cy.cGet('#annotation-modify-textarea-new').should('have.focus');
	});

	it('Typing email address should not show mention popup', function() {
		desktopHelper.insertComment('collaboraonline@al', false);

		cy.cGet('#mentionPopup').should('not.exist');
		cy.cGet('#annotation-modify-textarea-new').should('have.focus');
	});
});
