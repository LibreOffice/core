/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Annotation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/annotation.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	it('Insert',function() {
		// Make sure we know the cell address.
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2');

		desktopHelper.insertComment();

		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});

		// Move the mouse over commented cell without using trigger. "realMouseMove" function seems safer.
		cy.cGet('#test-div-OwnCellCursor').should('exist');
		cy.cGet('#test-div-OwnCellCursor').then((items) => {
			const cursor = items[0];
			const clientRectangle = cursor.getBoundingClientRect();
			const x = Math.round(clientRectangle.left + clientRectangle.width * 0.7);
			const y = Math.round(clientRectangle.top + clientRectangle.height * 0.5);
			const width = clientRectangle.width;
			const height = clientRectangle.height;

			cy.cGet('body').realMouseMove(x, y);

			// Comment should be visible now.
			cy.cGet('#annotation-content-area-1').should('be.visible');
			cy.cGet('#annotation-content-area-1').should('contain','some text');

			// Move the mouse to A1.
			cy.cGet('body').realMouseMove(x - width, y - height, { position: "topLeft" });
			// Comment shouldn't be visible now.
			cy.cGet('#annotation-content-area-1').should('not.be.visible');

			// Click on A1 while we are here.
			cy.cGet('body').realClick({ x: x - width, y: y - height });
			cy.cGet(helper.addressInputSelector).should('have.value', 'A1');

			// Now click again to cell B2. There was an issue with commented cells. We should be able to click on the commented cell.
			cy.cGet('body').realClick({ x: x, y: y });
			cy.cGet(helper.addressInputSelector).should('have.value', 'B2');
		});
	});

	it('Click on comment emits Clicked_Comment postMessage', function() {
		desktopHelper.insertComment();

		// This will record usage of window.postMessage (called from
		// _postMessage in browser/src/map/handler/Map.WOPI.js
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// <div id="comment-container-1" ...> is the topmost element of the comment.
		// Override its hidden attributes; then 'mouseover' should make the comment visible.
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#comment-container-1').then(element => {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});

		// <div class="cool-annotation-content-wrapper" ...> is its immediate child,
		// and its internals are the same as in other modules
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

	it('Modify',function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('contain','some text');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#annotation-content-area-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('contain','some text0, some other text');
		cy.cGet('#comment-container-1').should('exist');
	});

	it('Reply should not be possible', function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('contain','some text');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Reply').should('not.exist');
	});

	it('Remove',function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('contain','some text');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Remove').click();
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('Delete then Create Sheet should not retain comment',function() {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#spreadsheet-toolbar #insertsheet').click();
		calcHelper.assertNumberofSheets(2);

		desktopHelper.insertComment();
		cy.cGet('.cool-annotation').should('exist');

		calcHelper.selectOptionFromContextMenu('Delete Sheet...');
		cy.cGet('#delete-sheet-modal-response').click();
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#spreadsheet-toolbar #insertsheet').click();
		calcHelper.assertNumberofSheets(2);
		cy.cGet('#comment-container-1').should('not.exist');
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

	it('View Jump', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A100');
		desktopHelper.insertComment();
		/* comments are hidden in calc by default, so no visibility assert */
		cy.cGet('#comment-container-1').should('exist')
		cy.cGet('#Home-tab-label').click();

		calcHelper.enterCellAddressAndConfirm(this.win, 'A150');
		calcHelper.enterCellAddressAndConfirm(this.win, 'A135');

		/*
			NOTE: this scrollbar position might change in future. one can
			get the new scrollbar position by printing `x` to the console
			in `assertScrollbarPosition` function.
		*/
		desktopHelper.assertScrollbarPosition('vertical', 249, 252);
		desktopHelper.insertComment('second comment', false);
		desktopHelper.assertScrollbarPosition('vertical', 249, 252);
	});

	it('Action_GoToComment postMessage navigates to a comment across sheets', function() {
		// Put text and a comment in cell B2 on Sheet 1.
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2');
		helper.typeIntoDocument('COMMENT_CELL{enter}');
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2');
		desktopHelper.insertComment();
		cy.cGet('#comment-container-1').should('exist');

		// Move to a faraway cell, to check that GoToComment will scroll back
		calcHelper.enterCellAddressAndConfirm(this.win, 'Z1000');
		helper.typeIntoDocument('FAR_AWAY_CELL{enter}');

		// Create a new sheet and put text there.
		cy.cGet('#spreadsheet-toolbar #insertsheet').click();
		calcHelper.assertNumberofSheets(2);
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		helper.typeIntoDocument('SHEET2_CELL{enter}');

		// We are now on Sheet 2; the comment is on Sheet 1.

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

		// The comment should be visible and the anchor cell (B2) selected.
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'B2');
	});

	it('Get_Comments postMessage returns note and threaded comments from all sheets', function() {
		// Insert a legacy Note comment on Sheet 1.
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2');
		desktopHelper.insertComment('note comment');
		cy.cGet('#comment-container-1').should('exist');

		// Avoid notebookbar collapse before the next insertion.
		cy.cGet('#Home-tab-label').click();

		// Insert a Threaded comment on Sheet 1.
		calcHelper.enterCellAddressAndConfirm(this.win, 'D4');
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#insert-insert-threaded-comment').click();
		cy.cGet('#comment-container-new').should('exist');
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
			.type('threaded comment', {force: true});
		cy.cGet('.cool-annotation').last().find('[value="Save"]').click({force: true});
		cy.cGet('#comment-container-2').should('exist');
		cy.getFrameWindow().then(win => { helper.processToIdle(win); });

		// Create a new sheet and switch to it, so we're not on the sheet with comments.
		cy.cGet('#spreadsheet-toolbar #insertsheet').click();
		calcHelper.assertNumberofSheets(2);

		// Stub postMessage to capture the response.
		cy.getFrameWindow().then(win => {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// Send Get_Comments postMessage from Sheet 2.
		cy.getFrameWindow().then(win => {
			const message = { 'MessageId': 'Get_Comments' };
			win.postMessage(JSON.stringify(message), '*');
		});

		// Verify the response contains both comments from Sheet 1.
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
			// Legacy Note: has Text, Author, DateTime; no Resolved.
			expect(comments[0].Text).to.equal('note comment');
			expect(comments[0]).to.have.property('Author');
			expect(comments[0]).to.have.property('DateTime');
			expect(comments[0]).to.not.have.property('Resolved');
			// Threaded comment: has Text, Author, DateTime, Resolved.
			expect(comments[1].Text).to.equal('threaded comment');
			expect(comments[1]).to.have.property('Author');
			expect(comments[1]).to.have.property('DateTime');
			expect(comments[1].Resolved).to.equal('false');
		});
	});

});

describe(['tagdesktop'], 'Annotation Autosave Tests', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('calc/annotation.ods');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Insert autosave',function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});

	it('Insert autosave save',function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('.annotation-button-autosaved').should('be.not.visible');
		cy.cGet('.annotation-button-delete').should('be.not.visible');
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});

	it('Insert autosave cancel',function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('.cool-annotation').should('not.exist');
		cy.cGet('.annotation-button-autosaved').should('not.exist');
		cy.cGet('.annotation-button-delete').should('not.exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('not.exist');
	});

	it('Modify autosave',function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
	});

	it('Modify autosave save',function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#annotation-content-area-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
		cy.cGet('#comment-container-1').should('exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0, some other text');
	});

	it('Modify autosave cancel',function() {
		desktopHelper.insertComment();

		cy.cGet('#comment-container-1').should('exist');

		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-annotation-menu-1').click();
		cy.cGet('#comment-menu-1-dropdown').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.annotation-button-autosaved').should('be.visible');
		cy.cGet('.annotation-button-delete').should('be.visible');
		cy.cGet('.cool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#annotation-content-area-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('#comment-container-1').should('exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.cool-annotation').should('exist');
		cy.cGet('#comment-container-1').then(function (element) {
			element[0].style.visibility = '';
			element[0].style.display = '';
		});
		cy.cGet('#comment-container-1').trigger('mouseover', {force: true});
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
	});
});
