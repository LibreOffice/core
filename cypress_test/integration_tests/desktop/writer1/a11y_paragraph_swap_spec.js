/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer paragraph swap for the reader', { testIsolation: false }, function () {
	let win;

	function editableText() {
		return win.app.map._textInput.getPlainTextContent();
	}

	// Orca reads an object inside the editable as "￼", and loses its focus when that object is replaced.
	function readerView() {
		return a11yHelper.getAXNodesWithin('#clipboard-area').then(function (nodes) {
			const live = nodes.filter(function (node) { return !node.ignored; });
			return {
				objects: live.filter(function (node) {
					return node.role !== 'StaticText' && node.role !== 'InlineTextBox';
				}),
				text: live.filter(function (node) { return node.role === 'StaticText'; })
					.map(function (node) { return node.name; }).join(''),
			};
		});
	}

	function expectWholeParagraphInEditable(when) {
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.then(function () {
			return readerView().then(function (view) {
				expect(view.objects.map(function (node) { return node.role; }),
					'only the editable itself ' + when).to.have.length(1);
				expect(editableText(), 'the editable holds a paragraph ' + when).to.not.equal('');
				expect(view.text, 'the reader is given the whole paragraph ' + when)
					.to.equal(editableText());
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_paragraph_swap.fodt');

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

	it('the paragraph is the editable\'s own text, with no object inside it', function () {
		expectWholeParagraphInEditable('at the start');

		let previous;
		cy.then(function () {
			previous = editableText();
		});
		helper.typeIntoDocument('{downarrow}');
		expectWholeParagraphInEditable('after Down');
		cy.then(function () {
			expect(editableText(), 'Down moved to another paragraph').to.not.equal(previous);
		});

		helper.typeIntoDocument('{uparrow}');
		expectWholeParagraphInEditable('after Up');
		cy.then(function () {
			expect(editableText(), 'Up came back to the first paragraph').to.equal(previous);
		});
	});

	it('a burst of Down leaves the reader focused on the editable and its whole paragraph', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		const burst = 5;
		let expected;
		cy.then(function () {
			const after = Array.from(win.document.querySelectorAll('#a11y-context-after > *'))
				.map(function (paragraph) { return paragraph.textContent; });
			expect(after.length, 'the view holds enough paragraphs below the caret')
				.to.be.at.least(burst);
			expected = after[burst - 1];
		});

		helper.typeIntoDocument('{downarrow}'.repeat(burst));
		expectWholeParagraphInEditable('after a burst of Down');

		cy.then(function () {
			expect(editableText(), 'the burst ended ' + burst + ' paragraphs down').to.equal(expected);
			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'something holds the reader\'s focus').to.not.equal(null);
				expect(node.properties.editable, 'the reader is focused on the editable')
					.to.equal('richtext');
			});
		});
	});
});
