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
