/* global describe expect it cy before require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer editable for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_editable_position.fodt', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		// into the second paragraph, after its line break
		helper.typeIntoDocument('{downarrow}' + '{rightarrow}'.repeat(10));
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('the caret of the editable is at the caret of the document', function () {
		cy.cGet('.cursor-overlay .blinking-cursor').should('be.visible');
		cy.cGet('#clipboard-area').should(function ($area) {
			const doc = $area[0].ownerDocument;
			const caret = doc.querySelector('.cursor-overlay .blinking-cursor')
				.getBoundingClientRect();
			const selection = doc.getSelection();
			expect(selection.rangeCount, 'the editable has a caret').to.equal(1);
			const editableCaret = selection.getRangeAt(0).getBoundingClientRect();
			expect(editableCaret.top - $area[0].getBoundingClientRect().top,
				'the editable caret below its first line').to.be.above(5);
			// Cypress moves the editable 10px off the caret
			expect(editableCaret.top, 'the editable caret top')
				.to.be.within(caret.top - 1, caret.top + 11);
			expect(editableCaret.left, 'the editable caret left')
				.to.be.within(caret.left - 1, caret.left + 11);
		});
	});

	it('a paragraph further down is where the caret goes there', function () {
		let paragraphTop;
		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#a11y-context-after > span').eq(1).should(function ($span) {
			expect($span.text(), 'the second paragraph after the caret')
				.to.match(/^Third paragraph/);
			paragraphTop = $span[0].getBoundingClientRect().top;
		});

		helper.typeIntoDocument('{ctrl}{end}');
		cy.cGet('.cursor-overlay .blinking-cursor').should(function ($caret) {
			expect($caret[0].getBoundingClientRect().top, 'the caret top in that paragraph')
				.to.be.within(paragraphTop - 2, paragraphTop + 2);
		});
	});
});
