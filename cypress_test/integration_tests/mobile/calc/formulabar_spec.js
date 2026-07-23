/* global describe it cy beforeEach require expect Cypress*/

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud'], 'Formula bar tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');

		mobileHelper.enableEditingMobile();
	});

	it('Select a cell by address', function() {
		// Select first cell by clicking on it
		calcHelper.clickOnFirstCell();

		// Select a different cell using address input.
		helper.typeIntoInputField(helper.addressInputSelector, 'B2');

		helper.typeIntoInputField(helper.addressInputSelector, 'A1');

		cy.cGet('#test-div-cell_selection_handle_start').should('exist');
	});

	it('Select a cell range by address', function() {
		// Select first cell by clicking on it.
		calcHelper.clickOnFirstCell();

		// Select a cell range using address input.
		helper.typeIntoInputField(helper.addressInputSelector, 'B2:B3');

		cy.cGet('#test-div-cell_selection_handle_start').should('exist');
	});

	it.skip('Check input field content', function() {
		// First cell has some long content
		calcHelper.clickOnFirstCell();

		calcHelper.typeIntoFormulabar('{ctrl}a');
		helper.expectTextForClipboard('long line long line long line');

		// A2 cell is empty
		helper.typeIntoInputField(helper.addressInputSelector, 'A2');

		cy.cGet('[id="test-div-auto fill marker"]').should('exist');

		calcHelper.typeIntoFormulabar('{end}');

		cy.cGet('#formulabar .lokdialog-cursor')
			.should(function(cursor) {
				expect(cursor.offset().left).to.be.equal(93);
			});
	});

	it.skip('Edit cell via formula bar', function() {
		// First cell has some long content
		calcHelper.clickOnFirstCell();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long line');

		// Change first cell content via formula bar
		calcHelper.clickOnFirstCell();

		calcHelper.typeIntoFormulabar('{end}xxxxxxx{enter}');

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long linexxxxxxx');
	});

	it.skip('Accept formula bar change', function() {
		// First cell has some long content
		calcHelper.clickOnFirstCell();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long line');

		// Change first cell content via formula bar
		calcHelper.clickOnFirstCell();

		calcHelper.clickFormulaBar();
		helper.assertCursorAndFocus();

		helper.moveCursor('end', undefined, true, '#formulabar .lokdialog-cursor');

		calcHelper.typeIntoFormulabar('{backspace}{backspace}{backspace}');

		cy.cGet('#toolbar-up #acceptformula')
			.click();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long l');
	});

	it.skip('Reject formula bar change', function() {
		// First cell has some long content
		calcHelper.clickOnFirstCell();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long line');

		// Change first cell content via formula bar
		calcHelper.clickOnFirstCell();

		calcHelper.typeIntoFormulabar('{end}{backspace}{backspace}{backspace}');

		cy.cGet('#toolbar-up #cancelformula')
			.click();

		cy.wait(2000);

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td:nth-of-type(1)')
			.should('have.text', 'long line long line long line');
	});

	it.skip('Switch oneline-multiline mode of input bar', function() {
		// Get the initial height of the input field.
		var inputOriginalHeight = 0;
		cy.cGet('#formulabar')
			.should(function(inputbar) {
				inputOriginalHeight = inputbar.height();
				expect(inputOriginalHeight).to.not.equal(0);
			});

		// Switch to multiline mode.
		var arrowPos = [255, 10];
		cy.cGet('#formulabar')
			.click(arrowPos[0], arrowPos[1]);

		cy.cGet('#formulabar')
			.should(function(inputbar) {
				expect(inputbar.height()).to.be.greaterThan(inputOriginalHeight);
			});

		cy.cGet('#formulabar')
			.should(function(inputbar) {
				expect(inputbar.height()).to.be.equal(111);
			});

		// Switch back to one-line mode.
		cy.cGet('#formulabar')
			.click(arrowPos[0], arrowPos[1]);

		cy.cGet('#formulabar')
			.should(function(inputbar) {
				expect(inputbar.height()).to.be.equal(inputOriginalHeight);
			});
	});

	it.skip('Check formula help', function() {
		cy.cGet('.unoFunctionDialog').click();
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard', 'Logical').click();

		cy.cGet('body').contains('.func-entry', 'AND').find('.func-info-icon').click();

		cy.cGet('#mobile-wizard-title').should('be.visible').should('have.text', 'AND');
		cy.cGet('.ui-content.level-1.mobile-wizard[title=\'AND\'] .func-info-sig').should('be.visible')
			.should('contain.text', 'AND( Logical value 1, Logical value 2, ...');

		cy.cGet('.ui-content.level-1.mobile-wizard[title=\'AND\'] .func-info-desc')
			.should('be.visible')
			.should('have.text', 'Returns TRUE if all arguments are TRUE.');
	});

	it.skip('Add formula to cell', function() {
		calcHelper.clickOnFirstCell();

		cy.cGet('.unoFunctionDialog').click();
		cy.cGet('#mobile-wizard-content').should('be.visible');

		// Select average
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard', 'Statistical').click();

		cy.cGet('body').contains('.ui-content.level-0.mobile-wizard[title=\'Statistical\'] .func-entry', 'AVERAGE')
			.find('.ui-header-left')
			.click();

		cy.cGet('#mobile-wizard-content').should('not.be.visible');
		cy.cGet('#formulabar .lokdialog-cursor').should('be.visible');

		// Add a range
		calcHelper.typeIntoFormulabar('B2:B4');
		cy.cGet('#acceptformula').click();

		// Close mobile wizard with formulas.
		cy.waitUntil(function() {
			cy.cGet('#mobile-wizard-back').click();

			return cy.cGet('#mobile-wizard-content')
				.then(function(wizardContent) {
					return !Cypress.dom.isVisible(wizardContent[0]);
				});
		});

		cy.cGet('#mobile-wizard-content').should('not.be.visible');
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td:nth-of-type(1)').should('have.text', '5');
	});
});

describe(['tagmobile'], 'Formula bar context menu tests.', function() {
	var textLayerSelector = '#sc_input_window .ui-custom-textarea-text-layer';
	var selectionSelector = '#sc_input_window .ui-custom-textarea-cursor-layer span.selection';
	var cellText = 'long line long line long line';
	var win;

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');

		mobileHelper.enableEditingMobile();

		cy.getFrameWindow().then(function(frameWindow) {
			win = frameWindow;
		});

		calcHelper.clickOnFirstCell();
	});

	// Dispatch the touch sequence of a long press on the formula bar. Chrome
	// fires contextmenu itself part way through the press, pass false to leave
	// it out and let the widget's own long press timer trigger instead.
	function longPressOnFormulabar(withContextMenu = true) {
		cy.cGet(textLayerSelector).then(function(textLayer) {
			var element = textLayer[0];
			var rectangle = element.getBoundingClientRect();
			var posX = rectangle.left + 20;
			var posY = rectangle.top + rectangle.height / 2;

			var touch = new win.Touch({
				identifier: 1,
				target: element,
				clientX: posX,
				clientY: posY,
				pageX: posX,
				pageY: posY,
			});

			element.dispatchEvent(new win.TouchEvent('touchstart', {
				bubbles: true,
				cancelable: true,
				touches: [touch],
				targetTouches: [touch],
				changedTouches: [touch],
			}));

			if (withContextMenu) {
				element.dispatchEvent(new win.MouseEvent('contextmenu', {
					bubbles: true,
					cancelable: true,
					clientX: posX,
					clientY: posY,
				}));
			} else {
				// A touchend would cancel the timer, so outwait it first.
				cy.wait(win.app.LOUtil.longPressTime + 50);
			}

			cy.then(function() {
				element.dispatchEvent(new win.TouchEvent('touchend', {
					bubbles: true,
					cancelable: true,
					touches: [],
					targetTouches: [],
					changedTouches: [touch],
				}));
			});
		});
	}

	// A right click or the context menu key arrives without any touch events.
	function contextMenuOnFormulabar() {
		cy.cGet(textLayerSelector).then(function(textLayer) {
			var element = textLayer[0];
			var rectangle = element.getBoundingClientRect();

			element.dispatchEvent(new win.MouseEvent('contextmenu', {
				bubbles: true,
				cancelable: true,
				clientX: rectangle.left + 20,
				clientY: rectangle.top + rectangle.height / 2,
			}));
		});
	}

	function assertContextMenu() {
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('#mobile-wizard-content .ui-header', 'Cut').should('be.visible');
		cy.cGet('body').contains('#mobile-wizard-content .ui-header', 'Copy').should('be.visible');
		cy.cGet('body').contains('#mobile-wizard-content .ui-header', 'Paste').should('be.visible');
	}

	// The selection is drawn from what core reports back, so this also asserts
	// that core knows about the selection - not just the browser.
	function assertWholeFormulaSelected() {
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.cGet(selectionSelector)
			.should('have.length', 1)
			.should('have.text', cellText)
			.should('be.visible');
		cy.cGet('#sc_input_window .formulabar-selection-handle-start').should('be.visible');
		cy.cGet('#sc_input_window .formulabar-selection-handle-end').should('be.visible');
	}

	it('Long press selects the formula and opens the context menu', function() {
		longPressOnFormulabar();

		assertContextMenu();
		assertWholeFormulaSelected();
	});

	it('Copy and Paste from the formula bar', function() {
		helper.setDummyClipboardForCopy();

		longPressOnFormulabar();

		assertContextMenu();
		assertWholeFormulaSelected();

		cy.then(function() {
			cy.stub(win.app.idleHandler, '_deactivate');
		});

		cy.cGet('body').contains('#mobile-wizard-content .menu-entry-with-icon', 'Copy').click();

		// Pasting into the empty cell below shows what the copy really captured.
		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		cy.then(function() {
			win.app.socket.sendMessage('uno .uno:Paste');
		});

		calcHelper.assertSheetContents(
			[cellText, '', cellText, '1', '', '4', '', '10'], true);
	});

	it('Cut empties the formula bar', function() {
		helper.setDummyClipboardForCopy('text/plain');

		longPressOnFormulabar();

		assertContextMenu();
		assertWholeFormulaSelected();

		cy.cGet('body').contains('#mobile-wizard-content .menu-entry-with-icon', 'Cut').click();

		cy.cGet(textLayerSelector).should('have.text', '');
	});

	it('Long press works without a native contextmenu event', function() {
		longPressOnFormulabar(false);

		assertContextMenu();
		assertWholeFormulaSelected();
	});

	it('Long press works when the formula bar already has the cursor', function() {
		cy.cGet(textLayerSelector).click();
		cy.then(function() {
			return helper.processToIdle(win);
		});

		longPressOnFormulabar();

		assertContextMenu();
		assertWholeFormulaSelected();
	});

	it('Repeated context menu requests keep opening the menu', function() {
		contextMenuOnFormulabar();
		assertContextMenu();

		cy.then(function() {
			win.app.map.fire('closemobilewizard');
		});
		cy.cGet('#mobile-wizard-content').should('not.be.visible');

		contextMenuOnFormulabar();
		assertContextMenu();
	});
});
