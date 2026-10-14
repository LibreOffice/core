/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer list given to the reader', { testIsolation: false }, function () {
	let win;

	const FRUIT = ['Apples', 'Citrus', 'Lemons', 'Oranges', 'Pears'];
	const NESTED = ['Lemons', 'Oranges'];
	const STEPS = ['First step', 'Second step'];

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	function editing(text) {
		cy.cGet('#readable-content').should('contain.text', text);
	}

	// queryAXTree is in document order: an item's text is the static text up to the next item.
	function listItems() {
		return a11yHelper.getAXNodesWithin('#a11y-context-list').then(function (nodes) {
			const items = [];
			nodes.forEach(function (node) {
				if (node.ignored)
					return;
				if (node.role === 'listitem')
					items.push({ level: node.properties.level, text: '' });
				else if (node.role === 'StaticText' && items.length)
					items[items.length - 1].text += node.name;
			});
			return items;
		});
	}

	function holding(items, text) {
		return items.find(function (item) {
			return item.text.indexOf(text) !== -1;
		});
	}

	function regionText(id) {
		return cy.cGet('#' + id).invoke('text');
	}

	before(function () {
		helper.setupAndLoadDocument('writer/list_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		move('{ctrl}{home}');
		editing('Paragraph above.');
	});

	it('outside a list the reader is given none', function () {
		cy.cGet('#a11y-context-list').should('be.empty');
	});

	it('the list being edited reaches the reader', function () {
		move('{downarrow}');
		editing('Apples');
		cy.cGet('#a11y-context-list').should('contain.text', 'Pears');

		cy.then(function () {
			return listItems().then(function (items) {
				expect(items, 'an item for each entry of the list').to.have.length(FRUIT.length);
				FRUIT.forEach(function (text) {
					expect(holding(items, text), 'an item holding ' + text).to.not.equal(undefined);
				});
				expect(holding(items, 'First step'), 'an item of the next list').to.equal(undefined);
			});
		});

		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#a11y-context-list').then(function (nodes) {
				const markers = nodes.filter(function (node) {
					return !node.ignored && node.role === 'ListMarker';
				});
				expect(markers, 'a marker beside the bullet each entry starts with').to.have.length(0);
			});
		});
	});

	it('a nested entry is one level deeper', function () {
		cy.then(function () {
			return listItems().then(function (items) {
				const top = holding(items, 'Citrus').level;
				NESTED.forEach(function (text) {
					expect(holding(items, text).level, 'the level of ' + text).to.equal(top + 1);
				});
				expect(holding(items, 'Pears').level, 'the level of Pears').to.equal(top);
			});
		});
	});

	function onWindows(windows) {
		cy.then(function () {
			win.L.Browser.win = windows;
		});
		move('{downarrow}{uparrow}');
		editing('Apples');
	}

	function entry(text) {
		return Array.from(win.document.querySelectorAll('#a11y-context-list li')).find(function (item) {
			return item.firstChild.textContent.indexOf(text) !== -1;
		});
	}

	function backInTheEditable(text) {
		cy.then(function () {
			expect(win.app.map._textInput.getPlainTextContent(), 'the editable holds the entry').to.contain(text);
			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'something holds the focus').to.not.equal(null);
				expect(node.properties.editable, 'the focus is back in the editable').to.not.equal(undefined);
			});
		});
	}

	// Readers on Windows focus the object under their cursor on leaving browse mode.
	it('an entry can take the focus only on Windows', function () {
		let platform;
		cy.then(function () {
			platform = win.L.Browser.win;
		});

		[false, true].forEach(function (windows) {
			onWindows(windows);
			cy.then(function () {
				return a11yHelper.getAXNodesWithin('#a11y-context-list').then(function (nodes) {
					const entries = nodes.filter(function (node) {
						return !node.ignored && node.role === 'listitem';
					});
					expect(entries, 'the entries').to.have.length(FRUIT.length);
					entries.forEach(function (node, index) {
						expect(node.properties.focusable === true, 'entry ' + index + ' focusable, on Windows: ' + windows)
							.to.equal(windows);
					});
				});
			});
		});

		cy.then(function () {
			win.L.Browser.win = platform;
		});
	});

	it('focusing an entry puts the caret on it', function () {
		let platform;
		cy.then(function () {
			platform = win.L.Browser.win;
		});
		onWindows(true);

		cy.then(function () {
			entry('Lemons').focus();
		});
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		editing('Lemons');
		backInTheEditable('Lemons');

		cy.then(function () {
			win.L.Browser.win = platform;
		});
		move('{ctrl}{home}{downarrow}');
		editing('Apples');
	});

	// Orca enters focus mode with the focus on the page and the selection where it was reading.
	[
		{ from: 'Lemons', key: 'ArrowDown', keyCode: 40, to: 'Oranges' },
		{ from: 'Lemons', key: 'ArrowUp', keyCode: 38, to: 'Citrus' },
		{ from: 'Pears', key: 'ArrowDown', keyCode: 40, to: 'Paragraph between.' },
		{ from: 'Apples', key: 'ArrowUp', keyCode: 38, to: 'Paragraph above.' },
	].forEach(function (step) {
		it('a key reaching the page from ' + step.from + ' moves the caret to ' + step.to, function () {
			cy.then(function () {
				const element = entry(step.from);
				win.document.activeElement.blur();
				win.getSelection().collapse(element.firstChild, 3);
				expect(win.document.activeElement, 'the focus is on the page').to.equal(win.document.body);
				win.document.body.dispatchEvent(new win.KeyboardEvent('keydown',
					{ key: step.key, code: step.key, keyCode: step.keyCode, bubbles: true, cancelable: true }));
			});
			cy.then(function () {
				return a11yHelper.processToIdleWithA11yContext(win);
			});
			editing(step.to);
			backInTheEditable(step.to);

			move('{ctrl}{home}{downarrow}');
			editing('Apples');
		});
	});

	it('the entries are given once, in the list and not around it', function () {
		move('{downarrow}{downarrow}{downarrow}');
		editing('Oranges');

		regionText('a11y-context-before').then(function (text) {
			expect(text, 'the paragraphs before').to.contain('Paragraph above.');
			FRUIT.forEach(function (entry) {
				expect(text, 'the paragraphs before').to.not.contain(entry);
			});
		});
		regionText('a11y-context-after').then(function (text) {
			expect(text, 'the paragraphs after').to.contain('Paragraph between.');
			expect(text, 'the paragraphs after').to.contain(STEPS[0]);
			FRUIT.forEach(function (entry) {
				expect(text, 'the paragraphs after').to.not.contain(entry);
			});
		});

		cy.then(function () {
			const text = win.app.map._textInput.getPlainTextContent();
			expect(text, 'the editable holds the entry being edited').to.contain('Oranges');
			expect(text, 'the editable is free of the other entries').to.not.contain('Pears');
		});
	});

	it('another list is given on its own', function () {
		move('{downarrow}{downarrow}{downarrow}');
		editing(STEPS[0]);
		cy.cGet('#a11y-context-list').should('contain.text', STEPS[1]);
		cy.cGet('#a11y-context-list').should('not.contain.text', 'Apples');

		cy.then(function () {
			return listItems().then(function (items) {
				expect(items, 'the entries of the numbered list').to.have.length(STEPS.length);
			});
		});
	});

	it('leaving the list takes it away from the reader', function () {
		move('{downarrow}{downarrow}');
		editing('Paragraph below.');
		cy.cGet('#a11y-context-list').should('be.empty');
		cy.cGet('#a11y-context-before').should('contain.text', STEPS[1]);
	});
});
