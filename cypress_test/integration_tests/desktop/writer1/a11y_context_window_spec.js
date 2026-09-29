/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');
const desktopHelper = require('../../common/desktop_helper');

// Core sends the paragraphs around the caret beside the focused one. The client
// holds them outside #clipboard-area, whose textContent is what every caret and
// selection offset is measured against.
describe(['tagdesktop'], 'Writer off-screen context', { testIsolation: false }, function () {
	let win;

	const BEFORE_FIRST = 'Context alpha line';
	const BEFORE_SECOND = 'Context beta line';
	const FOCUSED = 'Context gamma line';

	function announced(text) {
		return a11yHelper.getAXNodes().then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.name &&
					node.name.indexOf(text) !== -1;
			});
		});
	}

	function editableText() {
		return win.app.map._textInput.getPlainTextContent();
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
		helper.typeIntoDocument(BEFORE_FIRST + '{enter}' + BEFORE_SECOND + '{enter}' + FOCUSED);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('the paragraphs above the caret reach the reader', function () {
		cy.then(function () {
			return announced(FOCUSED).then(function (nodes) {
				expect(nodes, 'the paragraph being edited').to.not.be.empty;
			});
		});

		[BEFORE_FIRST, BEFORE_SECOND].forEach(function (text) {
			cy.then(function () {
				return announced(text).then(function (nodes) {
					expect(nodes, 'the paragraph "' + text + '" above the caret').to.not.be.empty;
				});
			});
		});
	});

	it('a reader moving by line is given each paragraph whole', function () {
		cy.cGet('#a11y-context-before').should(function ($region) {
			const doc = $region[0].ownerDocument;
			const lines = [BEFORE_FIRST, BEFORE_SECOND].map(function (text) {
				const paragraph = Array.from($region[0].children).find(function (child) {
					return child.textContent === text;
				});
				expect(paragraph, 'the paragraph "' + text + '" in the region').to.exist;
				const range = doc.createRange();
				range.selectNodeContents(paragraph);
				return Array.from(range.getClientRects());
			});

			lines.forEach(function (rects, index) {
				expect(rects.length, 'line boxes of "' + [BEFORE_FIRST, BEFORE_SECOND][index] + '"')
					.to.equal(1);
			});
			expect(lines[1][0].top, 'the second paragraph starts a line of its own')
				.to.be.greaterThan(lines[0][0].top);
		});
	});

	it('a reader is handed each paragraph of the context as a paragraph of its own', function () {
		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#a11y-context-before').then(function (nodes) {
				const paragraphs = nodes.filter(function (node) {
					return !node.ignored && node.role === 'paragraph';
				});
				const count = win.document.getElementById('a11y-context-before').children.length;
				expect(count, 'paragraphs in the region').to.equal([BEFORE_FIRST, BEFORE_SECOND].length);
				expect(paragraphs.length, 'paragraph nodes the reader is handed')
					.to.equal(count);
			});
		});
	});

	it('the context stays out of the offsets the caret is measured in', function () {
		cy.then(function () {
			expect(editableText(), 'the editable holds the focused paragraph')
				.to.contain(FOCUSED);
		});

		[BEFORE_FIRST, BEFORE_SECOND].forEach(function (text) {
			cy.then(function () {
				expect(editableText(), 'the editable is free of "' + text + '"')
					.to.not.contain(text);
			});
		});

		cy.cGet('#readable-content').should(function ($span) {
			const text = $span.text();
			expect(text, 'readable-content carries the focused paragraph').to.contain(FOCUSED);
			[BEFORE_FIRST, BEFORE_SECOND].forEach(function (other) {
				expect(text, 'readable-content is free of "' + other + '"').to.not.contain(other);
			});
		});
	});

	// The numbers of the "Paragraph number N" lines a region carries, which is
	// what says where in the document the reader is being given.
	function paragraphNumbers(text) {
		const found = text.match(/Paragraph number (\d+)/g) || [];
		return found.map(function (one) {
			return parseInt(one.replace('Paragraph number ', ''), 10);
		});
	}

	function announcedNumbers() {
		return a11yHelper.getAXNodes().then(function (nodes) {
			const names = nodes.filter(function (node) {
				return !node.ignored && node.name;
			}).map(function (node) { return node.name; });
			return paragraphNumbers(names.join('\n'));
		});
	}

	it('what the reader gets follows the scroll', function () {
		let body = '';
		for (let i = 1; i <= 60; i++)
			body += 'Paragraph number ' + i + '{enter}';

		helper.typeIntoDocument('{ctrl}{end}{enter}');
		helper.typeIntoDocument(body);
		cy.then(function () {
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		let atTop;
		cy.cGet('#a11y-context-after').should(function ($el) {
			expect(paragraphNumbers($el.text()), 'the region was filled before scrolling')
				.to.not.be.empty;
		});
		cy.then(function () {
			return announcedNumbers().then(function (found) {
				expect(found, 'the reader has context before scrolling').to.not.be.empty;
				atTop = found;
			});
		});

		// the wheel and scrollbar path, so the view bookkeeping is a real one,
		// and the caret is left where it was
		cy.then(function () {
			desktopHelper.scrollViewDown(win);
			desktopHelper.scrollViewDown(win);
			desktopHelper.scrollViewDown(win);
			return helper.processToIdle(win);
		});

		cy.cGet('#a11y-context-after', { timeout: 20000 }).should(function ($el) {
			const found = paragraphNumbers($el.text());
			expect(found, 'the region was refilled after scrolling').to.not.be.empty;
			expect(Math.min.apply(null, found), 'the region followed the view')
				.to.be.greaterThan(Math.min.apply(null, atTop));
		});
		cy.then(function () {
			return announcedNumbers().then(function (afterScroll) {
				expect(afterScroll, 'the reader still has context after scrolling').to.not.be.empty;
				expect(Math.min.apply(null, afterScroll),
					'the window the reader is given moved down with the view, from '
					+ Math.min.apply(null, atTop))
					.to.be.greaterThan(Math.min.apply(null, atTop));
			});
		});
	});

	it('the context sits above and below the editable, never on its line', function () {
		helper.typeIntoDocument('{ctrl}{end}{uparrow}{uparrow}{uparrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		function lineBoxes(element) {
			const range = element.ownerDocument.createRange();
			range.selectNodeContents(element);
			return Array.from(range.getClientRects()).filter(function (rect) {
				return rect.height > 0;
			});
		}

		cy.cGet('#clipboard-area').should(function ($editable) {
			const doc = $editable[0].ownerDocument;
			const text = lineBoxes($editable[0]);
			expect(text, 'the editable holds text').to.not.be.empty;
			const top = Math.min.apply(null, text.map(function (rect) { return rect.top; }));
			const bottom = Math.max.apply(null, text.map(function (rect) { return rect.bottom; }));

			const before = Array.from(doc.getElementById('a11y-context-before').children);
			const after = Array.from(doc.getElementById('a11y-context-after').children);
			expect(before, 'paragraphs before the caret').to.not.be.empty;
			expect(after, 'paragraphs after the caret').to.not.be.empty;

			before.forEach(function (paragraph) {
				lineBoxes(paragraph).forEach(function (rect) {
					expect(rect.bottom, '"' + paragraph.textContent + '" ends above the editable text')
						.to.be.at.most(top);
				});
			});
			after.forEach(function (paragraph) {
				lineBoxes(paragraph).forEach(function (rect) {
					expect(rect.top, '"' + paragraph.textContent + '" starts below the editable text')
						.to.be.at.least(bottom);
				});
			});
		});
	});

	it('the caret still lands where the offsets say', function () {
		const TYPED = ' tail';
		let before;
		let caret;

		cy.cGet('#readable-content').then(function ($span) {
			before = $span.text();
			caret = win.app.map._textInput._getLastCursorPosition();
		});

		helper.typeIntoDocument(TYPED);
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#readable-content').should(function ($span) {
			const expected = before.slice(0, caret) + TYPED + before.slice(caret);
			expect($span.text(), 'the text landed at the caret offset').to.equal(expected);
		});
	});

	function refillContext() {
		helper.typeIntoDocument('{uparrow}{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function contextFocusable(region) {
		return a11yHelper.getAXNodesWithin(region).then(function (nodes) {
			const paragraphs = nodes.filter(function (node) {
				return !node.ignored && node.role === 'paragraph';
			});
			expect(paragraphs, 'paragraphs of ' + region).to.not.be.empty;
			return paragraphs.map(function (node) {
				return node.properties.focusable === true;
			});
		});
	}

	// Readers on Windows focus the object under their cursor on leaving browse mode; Orca
	// focuses every focusable object it reads.
	it('the context is focusable only on Windows', function () {
		let platform;
		cy.then(function () {
			platform = win.L.Browser.win;
		});

		[false, true].forEach(function (onWindows) {
			cy.then(function () {
				win.L.Browser.win = onWindows;
			});
			refillContext();
			['#a11y-context-before', '#a11y-context-after'].forEach(function (region) {
				cy.then(function () {
					return contextFocusable(region).then(function (focusable) {
						focusable.forEach(function (one, index) {
							expect(one, region + ' paragraph ' + index + ' focusable, on Windows: '
								+ onWindows).to.equal(onWindows);
						});
					});
				});
			});
		});

		cy.then(function () {
			win.L.Browser.win = platform;
		});
	});

	['#a11y-context-before', '#a11y-context-after'].forEach(function (region) {
		it('focusing a paragraph of ' + region + ' puts the caret on it', function () {
			let platform;
			let target;

			cy.then(function () {
				platform = win.L.Browser.win;
				win.L.Browser.win = true;
			});
			refillContext();

			cy.cGet(region + ' > span').first().then(function ($paragraph) {
				target = $paragraph.text();
				expect(editableText(), 'the caret starts elsewhere').to.not.equal(target);
				$paragraph[0].focus();
			});
			cy.then(function () {
				return helper.processToIdle(win);
			});

			cy.then(function () {
				win.L.Browser.win = platform;
				expect(editableText(), 'the editable holds the focused paragraph').to.equal(target);
				return a11yHelper.getFocusedAXNode().then(function (node) {
					expect(node, 'something holds the focus').to.not.equal(null);
					expect(node.properties.editable, 'the focus is back in the editable')
						.to.not.equal(undefined);
				});
			});
		});
	});

	// A reader whose position is on a paragraph that goes away looks for another and leaves focus mode.
	it('moving the caret keeps the paragraphs of the context', function () {
		let before;

		cy.cGet('#a11y-context-after > span').then(function ($paragraphs) {
			before = $paragraphs.toArray();
			expect(before, 'paragraphs after the caret').to.not.be.empty;
		});
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#a11y-context-after > span').should(function ($paragraphs) {
			const after = $paragraphs.toArray();
			const kept = Math.min(before.length, after.length);
			expect(kept, 'paragraphs in both').to.be.greaterThan(0);
			for (let index = 0; index < kept; index++)
				expect(after[index] === before[index], 'paragraph ' + index + ' is the same node').to.equal(true);
		});
	});

	// Orca enters focus mode with the focus on the page and the selection where it was reading.
	it('a key reaching the page from a paragraph of the context moves the caret past it', function () {
		let next;

		refillContext();
		cy.cGet('#a11y-context-after > span').then(function ($paragraphs) {
			expect($paragraphs.length, 'paragraphs after the caret').to.be.at.least(2);
			next = $paragraphs[1].textContent;
			win.document.activeElement.blur();
			win.getSelection().collapse($paragraphs[0].firstChild, 3);
			expect(win.document.activeElement, 'the focus is on the page').to.equal(win.document.body);
			win.document.body.dispatchEvent(new win.KeyboardEvent('keydown',
				{ key: 'ArrowDown', code: 'ArrowDown', keyCode: 40, bubbles: true, cancelable: true }));
		});
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			expect(editableText(), 'the caret is on the paragraph after the one read').to.equal(next);
			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'something holds the focus').to.not.equal(null);
				expect(node.properties.editable, 'the focus is back in the editable')
					.to.not.equal(undefined);
			});
		});
	});
});
