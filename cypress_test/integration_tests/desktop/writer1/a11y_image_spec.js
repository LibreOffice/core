/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer images for the reader', { testIsolation: false }, function () {
	let win;

	const PARAGRAPH = 'Before \ufffc after.';
	const NAME = 'A red square';
	const DESCRIPTION = 'A small square filled with red';

	function expectImage(selector) {
		cy.cGet(selector + ' [role="img"]').should(function ($images) {
			expect($images, 'the images').to.have.length(1);
			expect($images.attr('aria-label'), 'the name').to.equal(NAME);
			expect($images.attr('aria-description'), 'the description').to.equal(DESCRIPTION);
			expect($images.text(), 'the character standing for it').to.equal('\ufffc');
		});
		cy.then(function () {
			return a11yHelper.getAXNodesWithin(selector).then(function (nodes) {
				const images = nodes.filter(function (node) {
					return !node.ignored && (node.role === 'image' || node.role === 'img');
				});
				expect(images.map(function (node) { return node.name; }), 'the images read')
					.to.deep.equal([NAME]);
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_image.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('an image of the paragraph is read with its alternative text', function () {
		expectImage('#clipboard-area');
		cy.then(function () {
			expect(win.app.map._textInput.getPlainTextContent(), 'the editable text')
				.to.equal(PARAGRAPH);
		});
	});

	it('the caret after an image is in the text after it', function () {
		cy.then(function () {
			win.app.map._textInput._setSelectionRange(8, 8);
		});
		cy.cGet('#clipboard-area').should(function () {
			const selection = win.getSelection();
			expect(selection.anchorNode.textContent, 'the text with the caret').to.equal(' after.');
			expect(selection.anchorOffset, 'the caret offset in it').to.equal(0);
		});
	});

	it('an image of a paragraph around the caret is read too', function () {
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		expectImage('#a11y-context-before');
	});

	it('the caret at the end of a paragraph ending in an image is after it', function () {
		cy.cGet('#clipboard-area [role="img"]')
			.should('have.attr', 'aria-label', 'A second square');
		cy.then(function () {
			const input = win.app.map._textInput;
			const length = input.getPlainTextContent().length;
			input._setSelectionRange(length, length);
			const anchor = win.getSelection().anchorNode;
			expect(anchor.nodeType === win.Node.TEXT_NODE
				&& anchor.parentNode.closest('[role="img"]') !== null, 'the caret in the image')
				.to.equal(false);
			expect(input._getSelectionStart(), 'the caret offset').to.equal(length);
		});
	});

	it('an image selected from the keyboard is announced by its name', function () {
		// nothing lies below the last line, so Shift+F4 wraps around to the first image
		helper.typeIntoDocument('{ctrl}{end}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.realPress(['Shift', 'F4']);
		cy.cGet('#clipboard-area').should('have.attr', 'aria-description', NAME + ' selected. ');
		cy.realPress('Escape');
		// typing is blocked while a selection is held, though not under Cypress
		cy.cGet('#clipboard-area').should(function () {
			expect(win.app.map._textInput._hasAnySelection, 'a selection held').to.equal(false);
		});
	});
});
