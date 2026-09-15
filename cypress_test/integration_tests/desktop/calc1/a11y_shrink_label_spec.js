/* global describe it cy before expect require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Shrunken reference dialog', { testIsolation: false }, function () {
	let win;
	let whole = '';
	let caption = '';

	before(function () {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
		cy.then(function () { return helper.processToIdle(win); });
	});

	function title() {
		const bar = win.document.querySelector('#StandardFilterDialog').closest('.jsdialog-window')
			.querySelector('.ui-dialog-titlebar-text, .ui-dialog-title');
		return bar ? bar.innerText.trim() : '';
	}

	it('says which field it shrank to, and says it again when it grows back', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:DataFilterStandardFilter');
			return helper.processToIdle(win);
		});

		cy.cGet('#StandardFilterDialog').should('be.visible');
		cy.cGet('#more-button').click();
		cy.cGet('#copyresult-input').click();
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			whole = title();
			caption = win.document.querySelector('#copyresult').innerText.trim();
			expect(whole, 'the dialog is named while it is whole').to.not.be.empty;
			expect(caption, 'the field carries a caption while the dialog is whole').to.not.be.empty;
		});

		cy.cGet('#rbcopyarea-button').click();
		cy.then(function () { return helper.processToIdle(win); });
		cy.cGet('#edcopyarea').should('be.visible');

		cy.then(function () {
			const named = whole + ': ' + caption.replace(/:$/, '');
			expect(title(), 'the shrunken dialog names the field it holds').to.equal(named);
		});

		cy.cGet('#rbcopyarea-button').click();
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			expect(title(), 'the dialog takes its own name back').to.equal(whole);
		});
	});
});
