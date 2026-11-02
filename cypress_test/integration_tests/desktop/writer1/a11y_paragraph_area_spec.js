/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer header and footnote told to the reader', { testIsolation: false }, function () {
	let win;
	let told;
	let observer;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_paragraph_area.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	});

	// what the reader is told while the command runs: an announcement lasts a second only
	function jump(command, expectedText) {
		cy.then(function () {
			told = [];
			const editable = win.document.getElementById('clipboard-area');
			observer = new win.MutationObserver(function () {
				told.push(editable.getAttribute('aria-description') || '');
			});
			observer.observe(editable, { attributes: true, attributeFilter: ['aria-description'] });
			win.app.map.sendUnoCommand(command);
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		cy.cGet('#readable-content').should('have.text', expectedText);
		cy.then(function () {
			observer.disconnect();
		});
	}

	function description() {
		return win.document.getElementById('clipboard-area').getAttribute('aria-description') || '';
	}

	const body = 'Text with a footnote1 and an endnotei.';

	it('entering the header is told, and stays in the description', function () {
		jump('.uno:JumpToHeader', 'Header text.');
		cy.then(function () {
			expect(description(), 'the description in the header').to.equal('Header');
		});
	});

	it('leaving the header is told once', function () {
		jump('.uno:JumpToHeader', body);
		cy.then(function () {
			const exits = told.filter(function (text) { return text.indexOf('Out of header') !== -1; });
			expect(exits.length, 'leaving the header told once: ' + told.join(' / ')).to.equal(1);
			return new Cypress.Promise(function (resolve) { win.setTimeout(resolve, 1500); });
		});
		cy.then(function () {
			expect(description(), 'the description back in the body text').to.equal('');
		});
	});

	it('entering and leaving a footnote is told', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:JumpToNextFootnote');
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		jump('.uno:JumpToFootnoteOrAnchor', 'The footnote text.');
		cy.then(function () {
			expect(description(), 'the description in the footnote').to.equal('Footnote');
		});
		jump('.uno:JumpToFootnoteOrAnchor', body);
		cy.then(function () {
			expect(told.join(' / '), 'what the reader is told on leaving the footnote').to.contain('Out of footnote');
		});
	});
});
