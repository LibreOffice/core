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

	it('on Windows a paragraph change never leaves the editable empty', function () {
		let emptied = false;
		let observer;
		let wasWin;
		cy.then(function () {
			wasWin = win.L.Browser.win;
			win.L.Browser.win = true;
			observer = new win.MutationObserver(function () {
				if (editableText() === '')
					emptied = true;
			});
			observer.observe(win.document.getElementById('clipboard-area'),
				{ childList: true, subtree: true, characterData: true });
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			observer.disconnect();
			win.L.Browser.win = wasWin;
			expect(emptied, 'NVDA reads the line at the caret, so the editable is never caught empty')
				.to.equal(false);
		});
		expectWholeParagraphInEditable('after Down on Windows');
	});

	it('a scroll leaves alone what an input method is still composing', function () {
		const uncommitted = ' still composing';
		let requests = 0;
		let composed;

		cy.then(function () {
			const socket = win.app.socket;
			const send = socket.sendMessage;
			socket.sendMessage = function (msg) {
				if (msg === 'geta11yfocusedparagraph')
					requests++;
				return send.apply(this, arguments);
			};

			const editable = win.document.getElementById('clipboard-area');
			editable.dispatchEvent(new win.CompositionEvent('compositionstart'));
			win.document.getElementById('readable-content').textContent += uncommitted;
			composed = editableText();

			win.app.map._textInput.onVisibleAreaChanged();
		});

		cy.wrap(null).should(function () {
			expect(requests, 'the scroll asked core for the paragraph').to.equal(1);
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			expect(requests, 'the scroll asked core for the paragraph again').to.equal(1);
			expect(editableText(), 'the composition is still in the editable').to.equal(composed);
		});
	});
});
