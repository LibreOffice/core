/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Calc clipboard tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/clipboard.ods');

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showStatusBarIfHidden();
		}
		cy.viewport(1920,1080);
		desktopHelper.shouldHaveZoomLevel('100');

		// make it more complex and prevent form initial being A1
		helper.typeIntoInputField(helper.addressInputSelector, 'D5');

		cy.cGet('#map').focus();
		calcHelper.clickOnFirstCell();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function setDummyClipboard(type, content, image = false, fail = false, imageHtml = undefined) {
		cy.window().then(win => {
			var app = win['0'].app;
			var metaURL = encodeURIComponent(app.map._clip.getMetaURL());
			if (type === 'text/html') {
				content = content.replace('%META_URL%', metaURL);
			}
			var blob = new Blob([content]);
			var clipboard = app.map._clip;
			var clipboardItem = {
				getType: function(type) {
					return {
						then: function(resolve/*, reject*/) {
							if (image && type === 'text/html') {
								if (imageHtml === undefined) {
									imageHtml = '<img></img>';
								}
								resolve(new Blob([imageHtml]));
							} else {
								resolve(blob);
							}
						},
					};
				},
				types: image ? [type, 'text/html'] : [type],
			};
			var clipboardItems = [clipboardItem];
			clipboard._dummyClipboard = {
				read: function() {
					return {
						then: function(resolve, reject) {
							if (fail) {
								reject({
									message: 'rejected',
								});
							} else {
								resolve(clipboardItems);
							}
						},
					};
				},
			};
		});
	}

	it('HTML paste, internal case', function() {
		// Given a document with a SUM() in C1, and copying that to the clipboard:
		// A1 is 1, B1 is 2, so C1 is 3.
		helper.typeIntoInputField(helper.addressInputSelector, 'C1');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'C1');
		cy.window().then(win => {
			var app = win['0'].app;
			app.socket.sendMessage('uno .uno:Copy');
		});
		var html = '<div id="meta-origin" data-coolorigin="%META_URL%">ignored</div>';
		setDummyClipboard('text/html', html);

		helper.processToIdle(this.win);

		// When pasting C1 to D1:
		helper.typeIntoInputField(helper.addressInputSelector, 'D1');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'D1');
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make sure the formula gets rewritten as expected:
		// Internal paste: B1 is 2, C1 is 3, so D1 is 5.
		// Without the accompanying fix in place, this test would have failed with:
		// expected **#copy-paste-container table td:nth-of-type(1)** to have text **'5'**, but the text was **''**
		// i.e. a popup dialog was shown, instead of working, like with Ctrl-V.
		helper.setDummyClipboardForCopy();
		helper.copy();
		cy.cGet('#copy-paste-container table td:nth-of-type(1)').should('have.text', '5');
	});

	it('HTML paste, external case', function() {
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		var html = '<div>clipboard</div>';
		setDummyClipboard('text/html', html);

		// When pasting the clipboard to A1:
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make sure we actually consider the content of the HTML:
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', 'clipboard');
	});

	it('Plain text paste', function() {
		var text = 'plain text';
		setDummyClipboard('text/plain', text);

		// When pasting the clipboard to A1:
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make the paste happened:
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', 'plain text');
	});

	it('Image paste', function() {
		var base64 = 'iVBORw0KGgoAAAANSUhEUgAAAQAAAAEAAQMAAABmvDolAAAAA1BMVEW10NBjBBbqAAAAH0lEQVRo';
		base64 += 'ge3BAQ0AAADCoPdPbQ43oAAAAAAAAAAAvg0hAAABmmDh1QAAAABJRU5ErkJggg==';
		var blob = Cypress.Blob.base64StringToBlob(base64, 'image/png');
		setDummyClipboard('image/png', blob, /*image=*/true);

		// When pasting the clipboard:
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make sure the paste happened:
		cy.cGet('#document-container svg g').should('exist');
	});

	it('Image paste with meta', function() {
		let base64 = 'iVBORw0KGgoAAAANSUhEUgAAAQAAAAEAAQMAAABmvDolAAAAA1BMVEW10NBjBBbqAAAAH0lEQVRo';
		base64 += 'ge3BAQ0AAADCoPdPbQ43oAAAAAAAAAAAvg0hAAABmmDh1QAAAABJRU5ErkJggg==';
		let blob = Cypress.Blob.base64StringToBlob(base64, 'image/png');
		let imageHtml = '<meta http-equiv="content-type" content="text/html; charset=utf-8"><img></img>';
		setDummyClipboard('image/png', blob, /*image=*/true, /*fail=*/false, imageHtml);

		// When pasting the clipboard:
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make sure the paste happened:
		// Without the accompanying fix in place, this test would have failed, no image was
		// pasted.
		cy.cGet('#document-container svg g').should('exist');
	});

	it('HTML paste, internal case failing', function() {
		// Given a document with an A1 cell copied to the clipboard:
		cy.window().then(win => {
			var app = win['0'].app;
			app.socket.sendMessage('uno .uno:Copy');
		});
		var html = '<div id="meta-origin" data-coolorigin="%META_URL%">ignored</div>';
		setDummyClipboard('text/html', html, /*image=*/false, /*fail=*/true);

		// When pasting the clipboard to B1, which fails:
		helper.typeIntoInputField(helper.addressInputSelector, 'B1');
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste

		// Then make sure a warning popup is shown:
		cy.cGet('#copy_paste_warning-box').should('exist');

		// Close the dialog by clicking the OK button. Then try again to make sure the dialog appears again.
		cy.cGet('#modal-dialog-copy_paste_warning-box #modal-dialog-copy_paste_warning-box-yesbutton').click();
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste
		cy.cGet('#copy_paste_warning-box').should('exist');

		// Now when clicking the "Don't show again" button, the dialog should not appear again.
		cy.cGet('#modal-dialog-copy_paste_warning-box #modal-dialog-copy_paste_warning-box-nobutton').click();
		cy.cGet('#Home .ui-overflow-group-content > .unoPaste .arrowbackground').click();
		helper.getMenuEntry(0).click(); // Paste
		cy.cGet('#copy_paste_warning-box').should('not.exist');
	});

	it('Copy Hyperlink from pop-up', function () {
		desktopHelper.selectZoomLevel('100', false);
		helper.setDummyClipboardForCopy();

		calcHelper.dblClickOnFirstCell();
		cy.cGet('.ui-custom-textarea-text-layer').should('not.have.text', '');

		let url = 'http://www.example.com/';
		helper.typeIntoDocument('{rightArrow}{backspace}' + url + '{enter}');
		helper.processToIdle(this.win);

		// We need to close the hyperlink popup because currently mouse cursor is on the cell and it opens hyperlink popup window.
		helper.typeIntoDocument('{rightArrow}');
		helper.typeIntoDocument('{leftArrow}');
		helper.processToIdle(this.win);

		calcHelper.clickOnFirstCell();
		cy.cGet('.hyperlink-pop-up-container').should('be.visible');

		helper.processToIdle(this.win);
		cy.cGet('#hyperlink-pop-up-copy').click();

		cy.cGet('#copy-paste-container').should('contain.text', url); // TODO: There is an extra \n here.
	});

	it('Paste-Special', function () {
		helper.setDummyClipboardForCopy();

		calcHelper.clickOnFirstCell();
		helper.typeIntoDocument('Something to copy paste.');
		helper.typeIntoDocument('{enter}');
		helper.typeIntoDocument('{upArrow}');

		cy.cGet('#canvas-container').then((items) => {
			const rect = items[0].getBoundingClientRect();
			cy.cGet('body').rightclick(rect.left + 55, rect.top + 40);
		});

		const copyEntry = helper.getContextMenuItem('Copy');
		copyEntry.should('be.visible');
		copyEntry.click();

		helper.typeIntoDocument('{rightArrow}');
		helper.typeIntoDocument('{rightArrow}');

		cy.cGet('#canvas-container').rightclick(250, 35);
		const pasteSpecialEntry = helper.getContextMenuItem('Paste Special');
		pasteSpecialEntry.should('be.visible');
		pasteSpecialEntry.click();

		cy.cGet('#modal-dialog-paste_special_dialog-box').should('be.visible');

		cy.cGet('#modal-dialog-paste_special_dialog-box-yesbutton').should('be.visible');
		cy.cGet('#modal-dialog-paste_special_dialog-box-yesbutton').click();

		cy.cGet('#PasteSpecial').should('be.visible');
		cy.cGet('#ok').click();

		cy.cGet('#formulabar').should('contain.text', 'Something to copy paste.');

		cy.cGet('#PasteSpecial').should('not.exist');
	});
});
