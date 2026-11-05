/* global describe it cy require beforeEach expect */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Special Characters block follows the scroll', { testIsolation: false }, function () {
	let win;

	desktopHelper.shareDocumentAcrossTests('writer/navigator.odt');

	beforeEach(function () {
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
	});

	function subset() {
		return cy.cGet('#SpecialCharactersDialog #subsetlb-input option:selected').invoke('text');
	}

	// A wheel event first, so the view takes the scroll for one the user made.
	function scrollCharsetTo(top) {
		cy.cGet('#SpecialCharactersDialog #showcharset')
			.trigger('wheel', { deltaY: top })
			.scrollTo(0, top);
	}

	it('Scrolling the charset by hand shows its block in the dropdown', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:InsertSymbol');
		});
		cy.cGet('#SpecialCharactersDialog #showcharset .ui-iconview-entry').should('exist');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		let firstBlock;
		subset().then(function (text) {
			firstBlock = text;
			expect(firstBlock, 'the dialog opens on a block').to.not.equal('');
		});

		scrollCharsetTo(3000);
		cy.then(function () {
			return helper.processToIdle(win);
		});
		let scrolledBlock;
		subset().should(function (text) {
			expect(text, 'the dropdown left the first block').to.not.equal(firstBlock);
			scrolledBlock = text;
		});

		// Picking a block moves the dropdown away from what the scroll reported. Scrolling
		// back to the same row must report that row again.
		cy.then(function () {
			cy.cGet('#SpecialCharactersDialog #subsetlb-input').select(firstBlock);
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});
		subset().should(function (text) {
			expect(text, 'the picked block').to.equal(firstBlock);
		});

		scrollCharsetTo(3000);
		cy.then(function () {
			return helper.processToIdle(win);
		});
		subset().should(function (text) {
			expect(text, 'the same row as before shows its block again').to.equal(scrolledBlock);
		});

		scrollCharsetTo(0);
		cy.then(function () {
			return helper.processToIdle(win);
		});
		subset().should(function (text) {
			expect(text, 'back at the top is the first block again').to.equal(firstBlock);
		});
	});
});
