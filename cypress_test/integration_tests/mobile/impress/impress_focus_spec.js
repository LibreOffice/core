/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagmobile', 'tagproxy'], 'Impress focus tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/focus.odp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Select text box, no editing', function() {

		mobileHelper.enableEditingMobile();

		impressHelper.assertNotInTextEditMode();

		// Clipboard has the focus but the keyboard stays down
		helper.assertFocus('className', 'clipboard');
		helper.assertNoKeyboardInput();

		// One tap on a text shape, on the whitespace area,
		// does not start editing.
		cy.cGet('#document-container')
			.then(function(items) {
				expect(items).have.length(1);

				// Click in the left-bottom corner where there is no text.
				let posX = items[0].getBoundingClientRect().left + items[0].getBoundingClientRect().width / 4;
				let posY = items[0].getBoundingClientRect().top + items[0].getBoundingClientRect().height / 2;
				cy.log('Got left-bottom quantile at (' + posX + ', ' + posY + ')');

				cy.cGet('#document-container').click(posX, posY);
			});

		// Still no keyboard
		helper.assertFocus('className', 'clipboard');
		helper.assertNoKeyboardInput();

		// Shape selection.
		cy.cGet('#document-container svg g').should('exist');

		// But no editing.
		impressHelper.assertNotInTextEditMode();
	});

	it('Double-click to edit', function() {
		helper.setDummyClipboardForCopy();

		mobileHelper.enableEditingMobile();

		impressHelper.assertNotInTextEditMode();

		// Enter edit mode by double-clicking.
		mobileHelper.doubleTapOnDocument(this.win);

		impressHelper.typeTextAndVerify('Hello Impress', undefined, true);

		// End editing.
		helper.typeIntoDocument('{esc}');

		impressHelper.assertNotInTextEditMode();

		// Enter edit mode by double-clicking again.
		mobileHelper.doubleTapOnDocument(this.win);

		// Clear the text.
		helper.clearAllText();

		impressHelper.typeTextAndVerify('Bazinga Impress', undefined, true);
	});

	it('Zoom while editing shape text does not raise the keyboard', function() {
		mobileHelper.enableEditingMobile();

		mobileHelper.doubleTapOnDocument(this.win);
		helper.assertHaveKeyboardInput();

		cy.then(() => {
			this.win.app.map.setAcceptInput(false);
			this.win.app.map.fire('zoomstart');
			this.win.app.map.fire('zoomend');
		});
		helper.processToIdle(this.win);

		helper.assertNoKeyboardInput();
	});

	it('Tap into edited text raises the keyboard', function() {
		mobileHelper.enableEditingMobile();

		mobileHelper.doubleTapOnDocument(this.win);
		helper.assertHaveKeyboardInput();

		cy.then(() => {
			this.win.app.map.setAcceptInput(false);
		});
		helper.processToIdle(this.win);

		helper.assertNoKeyboardInput();

		// Centre of the text box the double click entered.
		cy.cGet('#document-container').click();
		helper.assertHaveKeyboardInput();
	});

	it('Graphic selection reply keeps the keyboard down', function() {
		mobileHelper.enableEditingMobile();

		cy.then(() => {
			cy.spy(this.win.app.socket, '_onMessage');
		});

		mobileHelper.doubleTapOnDocument(this.win);
		helper.assertHaveKeyboardInput();

		cy.then(() => {
			this.win.app.map.setAcceptInput(false);
		});
		helper.processToIdle(this.win);

		cy.then(() => {
			var last = null;
			this.win.app.socket._onMessage.getCalls().forEach(function(call) {
				var textMsg = call.args[0] && call.args[0].textMsg;
				if (typeof textMsg === 'string' && textMsg.startsWith('graphicselection:'))
					last = textMsg;
			});
			expect(last).to.be.a('string');
			expect(last).not.to.contain('EMPTY');

			// The shape stays selected while its text is edited.
			this.win.app.map._docLayer._onMessage(last);
			helper.processToIdle(this.win);
		});

		helper.assertNoKeyboardInput();

		cy.then(() => {
			this.win.app.socket._onMessage.restore();
		});
	});

	it.skip('Single-click to edit', function() {

		mobileHelper.enableEditingMobile();

		impressHelper.assertNotInTextEditMode();

		cy.cGet('#document-container')
			.then(function(items) {
				expect(items).have.length(1);

				// Click in the top left corner where there is no text.
				let posX = items[0].getBoundingClientRect().width / 2;
				let posY = items[0].getBoundingClientRect().height * (9 / 16);
				cy.log('Got center coordinates at (' + posX + ', ' + posY + ')');

				// Start editing; click on the text.
				cy.cGet('#document-container').click(posX, posY);

				impressHelper.typeTextAndVerify('Hello Impress');

				// End editing.
				helper.typeIntoDocument('{esc}');

				impressHelper.assertNotInTextEditMode();

				// Single-click to re-edit.
				cy.cGet('#document-container')
					.then(function(items) {
						expect(items).have.length(1);

						cy.cGet('#document-container').click(posX, posY);

						impressHelper.assertInTextEditMode();

						// Clear the text.
						helper.clearAllText();

						impressHelper.typeTextAndVerify('Bazinga Impress');

						// End editing.
						helper.typeIntoDocument('{esc}');

						impressHelper.assertNotInTextEditMode();
					});
			});
	});
});
