/* global describe expect it cy before afterEach require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// a11ycaretchanged and a11ytextselectionchanged put the caret and the
// selection of the document into the readable content, which is where a screen
// reader reads them from.
describe(['tagdesktop'], 'Writer caret and selection', { testIsolation: false }, function () {
	let win;

	const TYPED = 'abcdefghij';
	const STEPS = 3;

	function readable() {
		return win.document.getElementById('readable-content');
	}

	function selection() {
		return win.getSelection();
	}

	// The selection lives in the readable content whether or not the reader is
	// handed it, so assert the paragraph reaches the tree as well.
	function announced(text) {
		return a11yHelper.getAXNodes().then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.name &&
					node.name.indexOf(text) !== -1;
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/copy_paste.odt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		cy.then(function () {
			expect(win.prefs.getBoolean('accessibilityState'),
				'accessibility is on').to.equal(true);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument(TYPED);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('the caret position follows the document', function () {
		helper.typeIntoDocument('{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(TYPED).then(function (nodes) {
				expect(nodes, 'the paragraph reaches the reader').to.not.be.empty;
			});
		});

		cy.cGet('#readable-content').should(function () {
			const sel = selection();
			expect(readable().contains(sel.anchorNode),
				'the caret sits in the readable content').to.equal(true);
			expect(sel.isCollapsed, 'nothing is selected').to.equal(true);
			expect(sel.anchorOffset, 'the caret is at the start').to.equal(0);
		});

		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#readable-content').should(function () {
			expect(selection().anchorOffset, 'the caret moved by that many')
				.to.equal(STEPS);
		});
	});

	it('a selection reaches the readable content', function () {
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{shift}{end}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(TYPED).then(function (nodes) {
				expect(nodes, 'the paragraph reaches the reader').to.not.be.empty;
			});
		});

		cy.cGet('#readable-content').should(function () {
			const sel = selection();
			const paragraph = readable().textContent;

			expect(sel.isCollapsed, 'there is a selection').to.equal(false);
			expect(sel.anchorOffset, 'it starts where the caret was')
				.to.equal(STEPS);
			expect(sel.focusOffset, 'it reaches the end of the paragraph')
				.to.equal(paragraph.length);
			expect(sel.toString(), 'it holds the rest of the paragraph')
				.to.equal(paragraph.slice(STEPS));
		});
	});

	// the reports of the caret arrive late, as over a slow connection
	function delayCaretReports(delay) {
		cy.then(function () {
			const docLayer = win.app.map._docLayer;
			const onMessage = docLayer._onMessage;
			docLayer._onMessage = function (textMsg, img) {
				if (textMsg.startsWith('a11ycaretchanged:')
					|| textMsg.startsWith('a11ytextselectionchanged:'))
					setTimeout(onMessage.bind(this, textMsg, img), delay);
				else
					onMessage.call(this, textMsg, img);
			};
		});
	}

	afterEach(function () {
		if (win)
			delete win.app.map._docLayer._onMessage;
	});

	it('over a slow connection the caret stays where Right left it', function () {
		helper.typeIntoDocument('{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		const positions = [];
		const record = function () {
			positions.push(win.app.map._textInput._getSelectionEnd());
		};
		delayCaretReports(2500);
		cy.then(function () {
			win.document.addEventListener('selectionchange', record);
		});

		for (let i = 0; i < 5; i++)
			cy.realPress('ArrowRight');
		// past the delay and the 1 s hold after the last key
		cy.wait(5000);
		cy.then(function () {
			win.document.removeEventListener('selectionchange', record);
			const back = positions.findIndex((pos, i) => i > 0 && pos < positions[i - 1]);
			expect(back, 'the first move back in ' + JSON.stringify(positions)).to.equal(-1);
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret').to.equal(5);
		});
	});

	it('over a slow connection Left after Home goes from where Home went', function () {
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		delayCaretReports(2500);
		// Home moves only the document's caret, Left the one here too, which is then off by Home;
		// the paragraph is the first, so Left from its start moves nothing and sends no report
		cy.realPress('Home');
		cy.realPress('ArrowLeft');
		cy.then(function () {
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret before the reports')
				.to.equal(STEPS - 1);
		});
		cy.wait(5000);
		cy.then(function () {
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret').to.equal(0);
		});
	});

	it('over a slow connection Shift+Left keeps the selection Shift+Home made', function () {
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		delayCaretReports(2500);
		// as for Home and Left: no report follows Shift+Left at the start of the first paragraph
		cy.realPress(['Shift', 'Home']);
		cy.realPress(['Shift', 'ArrowLeft']);
		cy.wait(5000);
		cy.then(function () {
			const sel = selection();
			expect(sel.toString(), 'the selection').to.equal(TYPED.slice(0, STEPS));
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret end').to.equal(0);
		});
	});

	it('over a slow connection Shift+Right grows the selection without a collapse', function () {
		helper.typeIntoDocument('{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		// the report of Home ends the selection of the test before
		cy.cGet('#readable-content').should(function () {
			expect(selection().isCollapsed, 'nothing is selected').to.equal(true);
		});

		const lengths = [];
		const record = function () {
			lengths.push(selection().toString().length);
		};
		delayCaretReports(2500);
		cy.then(function () {
			win.document.addEventListener('selectionchange', record);
		});

		for (let i = 0; i < 5; i++)
			cy.realPress(['Shift', 'ArrowRight']);
		cy.wait(5000);
		cy.then(function () {
			win.document.removeEventListener('selectionchange', record);
			const back = lengths.findIndex((length, i) => i > 0 && length < lengths[i - 1]);
			expect(back, 'the first shrink in ' + JSON.stringify(lengths)).to.equal(-1);
			expect(selection().toString(), 'the selection').to.equal(TYPED.slice(0, 5));
		});
	});

	it('over a slow connection Right after Shift+Right goes from the caret end', function () {
		helper.typeIntoDocument('{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		delayCaretReports(2500);
		for (let i = 0; i < STEPS; i++)
			cy.realPress(['Shift', 'ArrowRight']);
		cy.wait(5000);
		cy.realPress('ArrowRight');
		cy.then(function () {
			expect(selection().isCollapsed, 'nothing is selected').to.equal(true);
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret').to.equal(STEPS + 1);
		});
		cy.wait(5000);
	});

	it('a caret report kept in the hold after Left gives way to the newer ones', function () {
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(2));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// the first report arrives in the hold after the key, the next one after two more keys
		delayCaretReports(300);
		cy.realPress('ArrowLeft');
		cy.wait(500);
		cy.realPress('ArrowLeft');
		cy.realPress('ArrowLeft');
		cy.wait(2000);
		cy.then(function () {
			expect(win.app.map._textInput._getSelectionEnd(), 'the caret').to.equal(0);
		});
	});

	// last, as it adds to the paragraph
	it('over a slow connection arrows step over a character of two code units', function () {
		helper.typeIntoDocument('{end}\u{1F600}xy');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		const textInput = function () {
			return win.app.map._textInput;
		};
		const positions = [];
		const record = function () {
			positions.push(textInput()._getSelectionEnd());
		};
		let end;
		cy.then(function () {
			end = textInput().getPlainTextContent().length;
			win.document.addEventListener('selectionchange', record);
		});
		delayCaretReports(2500);
		for (let i = 0; i < 3; i++)
			cy.realPress('ArrowLeft');
		cy.wait(5000);
		cy.then(function () {
			win.document.removeEventListener('selectionchange', record);
			const back = positions.findIndex((pos, i) => i > 0 && pos > positions[i - 1]);
			expect(back, 'the first move back in ' + JSON.stringify(positions)).to.equal(-1);
			expect(textInput()._getSelectionEnd(), 'the caret').to.equal(end - 4);
			positions.length = 0;
			win.document.addEventListener('selectionchange', record);
		});
		for (let i = 0; i < 3; i++)
			cy.realPress('ArrowRight');
		cy.wait(5000);
		cy.then(function () {
			win.document.removeEventListener('selectionchange', record);
			const back = positions.findIndex((pos, i) => i > 0 && pos < positions[i - 1]);
			expect(back, 'the first move back to the right in ' + JSON.stringify(positions))
				.to.equal(-1);
			expect(textInput()._getSelectionEnd(), 'the caret back').to.equal(end);
		});
	});
});
