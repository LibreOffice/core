/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc non-adjacent cell selection with a modifier click', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('calc/focus.ods');

		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		helper.typeIntoDocument('10{enter}');
		helper.typeIntoInputField(helper.addressInputSelector, 'C1');
		helper.typeIntoDocument('20{enter}');

		calcHelper.clickOnFirstCell();

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// A modifier click on a second cell must add it to the selection rather than
	// replacing it, so a copy afterwards picks up both cells. Before the fix,
	// core only recognized Ctrl for this; a browser sends macOS's Command key
	// as the separate CTRLMAC/KEY_MOD3 bit, which core dropped before it ever
	// reached Calc's selection handling, so a Command-click only selected the
	// second cell and the copy would have missed the first one.
	function assertModifierClickExtendsSelection(win, modifiers) {
		// Without a pause, this click lands inside the OS double-click window of
		// the cell click in the beforeEach above, so the browser reports it with
		// clickCount 2 (entering edit mode) instead of a fresh single click.
		cy.wait(600);
		cy.cGet('#test-div-OwnCellCursor').then(function (items) {
			var r = items[0].getBoundingClientRect();
			var clickX = r.left + r.width * 2.5;
			var clickY = r.top + r.height * 0.5;
			// realClick dispatches actual OS-level input (via CDP) rather than a
			// synthetic DOM event, which matters for modifiers: a plain
			// .click(x, y, {ctrlKey: true}) does not reliably reach the browser's
			// own notion of which keys are held, so a Ctrl/Cmd-click here would
			// not read back as one.
			cy.cGet('body').realClick(Object.assign({ x: clickX, y: clickY }, modifiers));
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});
		// A click with ctrlKey/metaKey set can leave the window without focus in
		// headless Chrome; the clipboard write path only retries once focus comes
		// back, so force it back before copying rather than depending on that.
		cy.then(function () {
			win.focus();
		});
		helper.copy();

		calcHelper.assertDataClipboardTable(['10', '', '20']);
	}

	it('Ctrl-click on a second cell adds it to the selection', function () {
		assertModifierClickExtendsSelection(this.win, { ctrlKey: true });
	});

	it('Command-click on a second cell adds it to the selection', function () {
		assertModifierClickExtendsSelection(this.win, { metaKey: true });
	});
});
