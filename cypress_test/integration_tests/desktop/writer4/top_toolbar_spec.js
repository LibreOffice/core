/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop'], 'Top toolbar tests.', function() {
	var newFilePath;

	beforeEach(function() {
		cy.viewport(1920,1080);
		newFilePath = helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToNotebookbar();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showSidebar();
		}
		cy.getFrameWindow().then((win) => {
			this.win = win;
		})

		writerHelper.selectAllTextOfDoc();
	});

	function refreshCopyPasteContainer() {
		helper.typeIntoDocument('{rightArrow}');
		writerHelper.selectAllTextOfDoc();
	}

	it('Apply highlight color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIconArrow('CharBackColor', 'Home').click();
		desktopHelper.selectColorFromPalette('FFB66C');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font span')
			.should('have.attr', 'style', 'background: #ffb66c');
	});

	it('Apply transparent highlight color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('CharBackColor', 'Home').first().click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font span')
			.should('have.attr', 'style', 'background: transparent');
		desktopHelper.getNbIcon('CharBackColor').find('.selected-color')
				.should('have.attr', 'style', 'background-color: transparent; border-color: var(--color-border);');
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIconArrow('FontColor', 'Home').click();
		desktopHelper.selectColorFromPalette('3FAF46');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#3faf46');
	});

	it('Apply style.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#stylesview').scrollTo('bottom');
		cy.cGet('#stylesview .notebookbar.ui-iconview-entry img[title=Title]').first().scrollIntoView().should('be.visible').click();
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font font').should('have.attr', 'style', 'font-size: 28pt');
	});

	it('Apply font name.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.actionOnSelector('fontName', (selector) => { cy.cGet(selector).click(); });
		cy.cGet('[id$="-dropdown"].modalpopup span').contains('Alef').scrollIntoView();
		desktopHelper.selectFromJSDialogListbox('Alef', true);
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'face', 'Alef');
	});

	it('Apply bold font.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Bold').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Apply italic font.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Italic').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p i').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Underline').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p u').should('exist');
	});

	it('Apply strikethrough.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Strikeout').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p strike').should('exist');
	});

	it('Apply font size', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.actionOnSelector('fontSize', (selector) => { cy.cGet(selector).click(); });
		desktopHelper.selectFromJSDialogListbox('72', false);
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'style', 'font-size: 72pt');
	});

	it('Clear direct formatting', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Bold').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
		desktopHelper.getNbIcon('ResetAttributes', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('not.exist');
	});

	it('Apply left/right alignment.', function() {
		desktopHelper.getNbIcon('Bold').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'right');
		desktopHelper.getNbIcon('RightPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'left');
	});

	it('Apply center alignment.', function() {
		desktopHelper.getNbIcon('CenterPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'center');
	});

	it('Apply justified.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('JustifyPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'justify');
	});

	function selectLineSpacing(entry) {
		desktopHelper.getNbIcon('LineSpacing').click();
		desktopHelper.getDropdown('home-line-spacing').contains('.ui-combobox-entry', entry).click();
	}

	it('Apply Line spacing: 1 and 1.5', function() {
		helper.setDummyClipboardForCopy();

		selectLineSpacing('Line Spacing: 1.5');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 150%');

		selectLineSpacing('Line Spacing: 1');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 100%');
	});

	it('Apply Line spacing: 2', function() {
		helper.setDummyClipboardForCopy();
		selectLineSpacing('Line Spacing: 2');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 200%');
	});

	it('Increase/Decrease Paragraph spacing', function() {
		helper.setDummyClipboardForCopy();
		selectLineSpacing('Increase Paragraph Spacing');

		writerHelper.selectAllTextOfDoc();
		helper.copy();

		cy.cGet('#copy-paste-container p').should('have.attr', 'style')
			.should('contain', 'margin-top: 0.04in');

		writerHelper.selectAllTextOfDoc();

		selectLineSpacing('Decrease Paragraph Spacing');

		writerHelper.selectAllTextOfDoc();
		helper.copy();

		cy.cGet('#copy-paste-container p')
			.should('have.attr', 'style')
			.should('not.contain', 'margin-top: 0.04in');
	});

	it('Toggle numbered list.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('DefaultNumbering', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container ol').should('exist');
	});

	it('Toggle bulleted list.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('DefaultBullet', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container ul').should('exist');
	});

	it('Increase/Decrease Indent.', function() {
		helper.setDummyClipboardForCopy();
		//Increase indent
		desktopHelper.getNbIcon('IncrementIndent', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p')
			.should('have.attr', 'style')
			.should('contain', 'margin-left: 0.49in');

		//Decrease indent
		desktopHelper.getNbIcon('DecrementIndent', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p')
			.should('have.attr', 'style')
			.should('not.contain', 'margin-left: 0.49in');
	});

	it('Insert/delete table.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('InsertTable', 'Home').click();
		cy.cGet('.inserttable-grid > .row > .col').eq(3).click();
		helper.typeIntoDocument('{ctrl}a');
		helper.copy();
		cy.cGet('#copy-paste-container table').should('exist');
		helper.typeIntoDocument('{ctrl}a');
		helper.typeIntoDocument('{shift}{del}');
		cy.cGet('.table-column-resize-marker').should('not.exist');
	});

	it('Insert image.', function() {
		desktopHelper.getNbIcon('InsertGraphic', 'Home').click();
		cy.cGet('#insertgraphic[type=file]').attachFile('/desktop/writer/image_to_insert.png');
		cy.cGet('#document-container svg g.Graphic').should('exist');
	});

	it('Insert text hyperlink.', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .hyperlinkdialog button').click();

		// All 3 fields should be visible
		cy.cGet('#target').should('exist').should('be.visible');
		cy.cGet('#indication').should('exist').should('be.visible');
		cy.cGet('#name').should('exist').should('be.visible');

		// Wait for the dialog to fully initialize
		helper.processToIdle(this.win);
		cy.cGet('#indication-input').should('have.value', 'text text1');

		cy.cGet('#indication-input').type('link');
		// Wait for indication field response to be processed before typing in target
		helper.processToIdle(this.win);
		cy.cGet('#target-input').type('www.something.com');
		cy.cGet('#ok').click();

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		helper.processToIdle(this.win);
		helper.expectTextForClipboard('text text1link');
		cy.cGet('#copy-paste-container p a').should('have.attr', 'href', 'http://www.something.com/');
	});

	it.skip('Insert mail hyperlink.', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .hyperlinkdialog button').click();
		cy.cGet('#mail').click();

		// Both mail fields should be visible
		cy.cGet('#receiver').should('exist').should('be.visible');
		cy.cGet('#subject').should('exist').should('be.visible');

		// Wait for the dialog to fully initialize
		helper.processToIdle(this.win);

		cy.cGet('#receiver-input').type('john.doe@test.abc');
		// Wait for receiver field response to be processed before typing in target
		helper.processToIdle(this.win);
		cy.cGet('#subject-input').type('planning-meeting');
		cy.cGet('#ok').click();

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		helper.processToIdle(this.win);
		helper.expectTextForClipboard('text text1');
		cy.cGet('#copy-paste-container p a').should('have.attr', 'href', 'mailto:john.doe@test.abc?subject=planning-meeting');
	});

	it('Insert image hyperlink.', function () {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoBasicShapes button').click();
		cy.cGet('.col.w2ui-icon.basicshapes_octagon').click();
		cy.cGet('#document-container svg g').should('exist');
		helper.processToIdle(this.win);

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .hyperlinkdialog button').click();

		// Only URL field should be visible
		cy.cGet('#target').should('exist').should('be.visible');
		cy.cGet('#indication').should('exist').should('not.be.visible');
		cy.cGet('#name').should('exist').should('not.be.visible');

		cy.cGet('#target-input').type('www.something.com');
		cy.cGet('#ok').click();

		//Can't ctrl click shape, so re-enter dialog to check value persists
		cy.cGet('#Insert-container .hyperlinkdialog button').click();
		cy.cGet('#target-input').should('have.value', 'http://www.something.com/');
	});

	it('Insert/delete shape.', function() {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoBasicShapes button').click();
		cy.cGet('.col.w2ui-icon.basicshapes_octagon').click();
		cy.cGet('#document-container svg g').should('exist');

		//delete
		helper.typeIntoDocument('{del}');

		cy.cGet('#test-div-shapeHandlesSection')
			.should('not.exist');
	});

	it('Insert/delete chart.', function() {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoInsertObjectChart button').click();

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// exit active object mode
		helper.typeIntoDocument('{esc}');
		helper.typeIntoDocument('{esc}');

		//delete
		helper.typeIntoDocument('{del}');

		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it('Save.', function() {
		desktopHelper.getNbIcon('Bold').click();
		cy.cGet('.notebookbar-shortcuts-bar .unoSave').click();
		helper.reloadDocument(newFilePath);
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		// document was reloaded, fetch the frame window again
		cy.getFrameWindow().then((win) => {
			helper.processToIdle(win);
		})
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Print', function() {
		// A new window should be opened with the PDF.
		cy.stub(this.win, 'open').as('windowOpen');

		cy.cGet('#File-tab-label').click();
		cy.cGet('#File-container .unoPrint button').click();

		cy.get('@windowOpen').should('be.called');
	});

	it('Apply Undo/Redo.', function() {
		helper.setDummyClipboardForCopy();
		//Do
		desktopHelper.getNbIcon('Italic').click();
		helper.copy();
		helper.processToIdle(this.win); // wait for new clipboard
		cy.cGet('#copy-paste-container p i').should('exist');

		//Undo
		cy.cGet('#Home-container .unoUndo').should('not.have.attr','disabled');
		cy.cGet('#Home-container .unoUndo button').click();
		helper.copy();
		helper.processToIdle(this.win); // wait for new clipboard
		cy.cGet('#copy-paste-container p i').should('not.exist');

		// Dismiss tooltip
		cy.cGet('#Home-tab-label').click();
		cy.cGet('#Home-tab-label').click();
		cy.cGet('[role="tooltip"]:not(.visuallyhidden)').should('not.exist');

		//Redo
		cy.cGet('#Home-container .unoRedo').should('not.have.attr','disabled');
		cy.cGet('#Home-container .unoRedo button').click();
		helper.copy();
		helper.processToIdle(this.win); // wait for new clipboard
		cy.cGet('#copy-paste-container p i').should('exist');
	});

	it('Enable/Disable Screen Reading', function() {
		// when accessibility is disabled at server level
		// this unit passes but doesn't perform any check
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.checkAccessibilityEnabledToBe(true);
		desktopHelper.setAccessibilityState(false);
		desktopHelper.checkAccessibilityEnabledToBe(false);
		desktopHelper.setAccessibilityState(true);
		desktopHelper.checkAccessibilityEnabledToBe(true);
		desktopHelper.switchUIToCompact();
		desktopHelper.checkAccessibilityEnabledToBe(true);
		desktopHelper.setAccessibilityState(false);
		desktopHelper.checkAccessibilityEnabledToBe(false);
		desktopHelper.setAccessibilityState(true);
		desktopHelper.checkAccessibilityEnabledToBe(true);
		desktopHelper.setAccessibilityState(false);
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.checkAccessibilityEnabledToBe(false);
		desktopHelper.switchUIToCompact();
		desktopHelper.setAccessibilityState(true);
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.checkAccessibilityEnabledToBe(true);
		desktopHelper.setAccessibilityState(false);
		desktopHelper.switchUIToCompact();
		desktopHelper.checkAccessibilityEnabledToBe(false);
	});

	it('Show/Hide sidebar.', function() {
		cy.cGet('#View-tab-label').click();
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		// Hide.
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
		// Show.
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
	});

	it('Insert Special Character.', function() {
		cy.cGet('#Home-container .unoCharmapControl').click();
		cy.cGet('.jsdialog-container.ui-dialog.ui-widget-content.lokdialog_container').should('be.visible');
		cy.cGet('.ui-dialog-title').should('have.text', 'Special Characters');

		// FIXME: dialog is not async, shows popup
		cy.cGet('#favchar1').click();
		cy.cGet('#SpecialCharactersDialog .ui-pushbutton.jsdialog.button-primary').click();

		//helper.expectTextForClipboard('€');
	});

	it('Clone Formatting.', function() {
		helper.setDummyClipboardForCopy();
		// Select one character at the beginning of the text.
		helper.typeIntoDocument('{home}');
		helper.textSelectionShouldNotExist();
		helper.typeIntoDocument('{shift}{rightArrow}');
		helper.textSelectionShouldExist();

		// Apply bold and try to clone it to the whole word.
		desktopHelper.getNbIcon('Bold').click();
		desktopHelper.getNbIcon('FormatPaintbrush').click();

		// Wait for the paintbrush to become active (single-click has a 250ms delay
		// due to double-click detection).
		cy.cGet('#document-canvas').should('have.class', 'bucket-cursor');

		// Click at the blinking cursor position.
		cy.cGet('.leaflet-cursor.blinking-cursor')
			.then(function(cursor) {
				var boundRect = cursor[0].getBoundingClientRect();
				var XPos = boundRect.left;
				var YPos = (boundRect.top + boundRect.bottom) / 2;
				cy.cGet('body').click(XPos, YPos);
			});

		writerHelper.selectAllTextOfDoc();

		// Full word should have bold font.
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('contain', 'text');
	});

	it.skip('Insert Page Break', function() {
		desktopHelper.assertVisiblePage(1, 1, 1);
		helper.selectAllText();
		helper.expectTextForClipboard('text text1');
		helper.typeIntoDocument('{end}');
		helper.typeIntoDocument('{ctrl}{leftarrow}');
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container-row .unoInsertPagebreak').click();
		desktopHelper.assertVisiblePage(1, 2, 2);
		helper.selectAllText();

		//var data = [];
		//var expectedData = ['text ', 'text1'];

		//cy.cGet('#copy-paste-container').find('p').each($el => {
		//	cy.wrap($el)
		//		.invoke('text')
		//		.then(text => {
		//			data.push(text);
		//		});
		//	cy.log(data);
		//}).then(() => {
		//	expect(data.length).eq(expectedData.length);
		//	var isEqual = true;
		//	for (var i = 0; i < data.length; i++) {
		//		isEqual = isEqual && ((data[i] == expectedData[i]) ||
		//			(data[i] == '\n' + expectedData[i]) ||
		//			(data[i] == '\n' + expectedData[i] + '\n'));
		//	}
		//	expect(isEqual).to.be.true;
		//});
	});

	it('Apply superscript.', function() {
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		desktopHelper.getNbIcon('SuperScript').click();
		cy.cGet('#document-container').click('center');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sup').should('exist');
	});

	it('Apply subscript.', function() {
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		desktopHelper.getNbIcon('SubScript').click();
		cy.cGet('#document-container').click('center');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sub').should('exist');
	});

	it('Delete Text', function() {
		helper.setDummyClipboardForCopy();
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('text text1');
		helper.typeIntoDocument('{del}');
		helper.typeIntoDocument('{ctrl}a');
		helper.textSelectionShouldNotExist();
	});

	it('Insert/delete Fontwork', function() {
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoFontworkGalleryFloater').click();
		cy.cGet('#ok').click();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		//delete
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it.skip('Scroll', function() {
		// Start all the way on the left side of the toolbar
		cy.cGet('#Home-container #home-do').should('be.visible');
		// TODO: Cypress thinks buttons are visible even though they are not
		//cy.cGet('#Home-container #home-search-dialog').should('not.be.visible');
		cy.cGet('#toolbar-up .ui-scroll-left').should('not.be.visible');
		cy.cGet('#toolbar-up .ui-scroll-right').should('be.visible');

		// Scroll right until the scroll right button is disabled
		cy.waitUntil(function() {
			cy.cGet('#toolbar-up .ui-scroll-right').click();
			cy.wait(300); // Wait for scroll animation
			return cy.cGet('#toolbar-up .ui-scroll-right')
				.then(function(scrollRightButton) {
					return !Cypress.dom.isVisible(scrollRightButton);
				});
		});

		// Now we are all the way on the right side of the toolbar
		// TODO: Cypress thinks buttons are visible even though they are not
		//cy.cGet('#Home-container #home-do').should('not.be.visible');
		cy.cGet('#Home-container #home-search-dialog').should('be.visible');
		cy.cGet('#toolbar-up .ui-scroll-left').should('be.visible');
		cy.cGet('#toolbar-up .ui-scroll-right').should('not.be.visible');

		// Scroll left until the scroll left button is disabled
		cy.waitUntil(function() {
			cy.cGet('#toolbar-up .ui-scroll-left').click();
			cy.wait(300); // Wait for scroll animation
			return cy.cGet('#toolbar-up .ui-scroll-left')
				.then(function(scrollLeftButton) {
					return !Cypress.dom.isVisible(scrollLeftButton);
				});
		});

		// Now back on the left side of the toolbar
		cy.cGet('#Home-container #home-do').should('be.visible');
		// TODO: Cypress thinks buttons are visible even though they are not
		//cy.cGet('#Home-container #home-search-dialog').should('not.be.visible');
		cy.cGet('#toolbar-up .ui-scroll-left').should('not.be.visible');
		cy.cGet('#toolbar-up .ui-scroll-right').should('be.visible');
	});

	it('Switch Tabs', function() {
		// Start in Home tab
		cy.cGet('.notebookbar#Home').should('be.visible');
		cy.cGet('#Home-tab-label').should('have.class','selected');
		cy.cGet('.notebookbar#Insert').should('not.be.visible');
		cy.cGet('#Insert-tab-label').should('not.have.class','selected');

		// Switch to Insert tab
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('.notebookbar#Home').should('not.be.visible');
		cy.cGet('#Home-tab-label').should('not.have.class','selected');
		cy.cGet('.notebookbar#Insert').should('be.visible');
		cy.cGet('#Insert-tab-label').should('have.class','selected');
	});

	it('Formatting shortcuts blocked in view mode.', function() {
		// Verify baseline: no bold in edit mode.
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('exist');
		cy.cGet('#copy-paste-container p b').should('not.exist');

		// Switch from edit mode to view mode.
		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('readonly');
		});
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Viewing');

		// Press Ctrl+B - should be blocked in view mode.
		helper.typeIntoDocument('{ctrl}b');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Switch back to edit mode to verify bold was not applied.
		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('edit');
		});

		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('exist');
		cy.cGet('#copy-paste-container p b').should('not.exist');
	});
});
