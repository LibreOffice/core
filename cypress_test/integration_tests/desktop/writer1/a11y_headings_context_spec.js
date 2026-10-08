/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer headings given to the reader', { testIsolation: false }, function () {
	let win;

	const OUTLINE = [
		{ level: 1, text: 'Introduction' },
		{ level: 2, text: 'Scope' },
		{ level: 1, text: 'Method' },
		{ level: 2, text: 'Samples' },
		{ level: 3, text: 'Collection' },
		{ level: 1, text: 'Results' },
	];

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	function editing(text) {
		cy.cGet('#readable-content').should('have.text', text);
	}

	const REGIONS = ['#a11y-headings-above', '#a11y-context-before', '#a11y-context-after', '#a11y-headings-below'];
	const OUTLINE_REGIONS = '#a11y-headings-above, #a11y-headings-below';

	function treeOrder() {
		return a11yHelper.getAXNodes().then(function (tree) {
			const byId = {};
			tree.forEach(function (node) { byId[node.nodeId] = node; });
			const order = [];
			const walk = function (node) {
				order.push(node);
				node.childIds.forEach(function (id) {
					if (byId[id]) walk(byId[id]);
				});
			};
			walk(tree[0]);
			return order;
		});
	}

	function liveIds(selector) {
		return a11yHelper.getAXNodesWithin(selector).then(function (nodes) {
			return nodes.filter(function (node) { return !node.ignored && node.backendDOMNodeId; })
				.map(function (node) { return node.backendDOMNodeId; });
		});
	}

	function regionIds(regions, done) {
		if (!regions.length)
			return done([]);
		return liveIds(regions[0]).then(function (first) {
			return regionIds(regions.slice(1), function (rest) {
				return done([first].concat(rest));
			});
		});
	}

	function everyHeadingOnce(outline, where) {
		cy.then(function () {
			const names = outline.map(function (heading) { return heading.text; });
			const order = [];
			REGIONS.forEach(function (region) {
				const items = win.document.querySelectorAll(region + (region === '#a11y-headings-above'
					|| region === '#a11y-headings-below' ? ' a' : ' > span'));
				items.forEach(function (item) {
					if (names.indexOf(item.textContent) !== -1)
						order.push(item.textContent);
				});
			});
			expect(order, 'the headings a reader moves through ' + where + ', in document order')
				.to.deep.equal(names);

			return a11yHelper.getAXNodesWithin('#a11y-headings-above').then(function (above) {
				return a11yHelper.getAXNodesWithin('#a11y-headings-below').then(function (below) {
					const found = above.concat(below).filter(function (node) {
						return !node.ignored && node.role === 'heading';
					}).map(function (node) {
						return { level: node.properties.level, text: node.name.trim() };
					});
					const expected = outline.filter(function (heading) {
						return found.some(function (node) { return node.text === heading.text; });
					});
					expect(found, 'the headings outside the paragraphs around the caret, with their level ' + where)
						.to.deep.equal(expected);
				});
			});
		});
	}

	function described() {
		return a11yHelper.getAXNodes().then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.description;
			}).map(function (node) { return node.description; }).join(' | ');
		});
	}

	function toldLevel(level) {
		cy.then(function () {
			return described().then(function (text) {
				expect(text, 'what the reader is told about the paragraph')
					.to.contain('Heading level ' + level);
			});
		});
	}

	function toldNoLevel() {
		cy.then(function () {
			return described().then(function (text) {
				expect(text, 'what the reader is told about the paragraph')
					.to.not.contain('Heading level');
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/headings_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		move('{ctrl}{home}{downarrow}');
		editing('Opening paragraph.');
	});

	function outlineFlanksContext() {
		cy.then(function () {
			return regionIds(REGIONS, function (found) {
				const above = found[0];
				const given = found[1].concat(found[2]);
				const below = found[3];
				return treeOrder().then(function (tree) {
					const order = tree.map(function (node) { return node.backendDOMNodeId; });
					const at = function (ids) {
						return ids.map(function (id) { return order.indexOf(id); });
					};
					expect(above.length + below.length, 'headings outside the paragraphs given around the caret')
						.to.be.greaterThan(0);
					if (above.length)
						expect(Math.max.apply(null, at(above)),
							'the last heading above, before the first paragraph given before the caret')
							.to.be.lessThan(Math.min.apply(null, at(given)));
					if (below.length)
						expect(Math.min.apply(null, at(below)),
							'the first heading below, after the last paragraph given after the caret')
							.to.be.greaterThan(Math.max.apply(null, at(given)));
				});
			});
		});
	}

	it('every heading of the document reaches the reader once, in order', function () {
		cy.cGet('#a11y-headings-below').should('contain.text', OUTLINE[OUTLINE.length - 1].text);
		everyHeadingOnce(OUTLINE, 'from the top');
		outlineFlanksContext();

		move('{ctrl}{end}');
		editing('Results paragraph.');
		cy.cGet('#a11y-headings-above').should('contain.text', OUTLINE[0].text);
		everyHeadingOnce(OUTLINE, 'from the end');
		outlineFlanksContext();

		move('{ctrl}{home}{downarrow}');
		editing('Opening paragraph.');
	});

	it('the headings stay out of the offsets the caret is measured in', function () {
		cy.then(function () {
			const text = win.app.map._textInput.getPlainTextContent();
			expect(text, 'the editable holds the paragraph being edited').to.contain('Opening paragraph.');
			expect(text, 'the editable is free of the headings').to.not.contain('Introduction');
		});
	});

	it('the heading the caret is in reaches the reader with its level', function () {
		move('{ctrl}{home}');
		editing(OUTLINE[0].text);
		toldLevel(OUTLINE[0].level);
		cy.then(function () {
			win.app.map._textInput._textArea.blur();
			win.app.map._textInput.focus();
		});
		toldLevel(OUTLINE[0].level);
		move('{downarrow}{downarrow}{downarrow}');
		editing(OUTLINE[1].text);
		toldLevel(OUTLINE[1].level);
	});

	it('the headings above the caret reach the reader in order', function () {
		move('{ctrl}{home}{downarrow}{downarrow}{downarrow}{downarrow}');
		editing('Scope paragraph.');

		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#a11y-context-before').then(function (nodes) {
				const given = nodes.filter(function (node) {
					return !node.ignored && node.role === 'StaticText';
				}).map(function (node) { return node.name.trim(); });
				expect(given, 'the paragraphs before the caret').to.deep.equal(
					[OUTLINE[0].text, 'Opening paragraph.', 'Quoted words.', OUTLINE[1].text]);
			});
		});
	});

	it('the headings around the caret are headings of their level', function () {
		move('{ctrl}{home}{downarrow}');
		editing('Opening paragraph.');

		const check = function (region, near) {
			return a11yHelper.getAXNodesWithin(region).then(function (nodes) {
				const found = nodes.filter(function (node) {
					return !node.ignored && node.role === 'heading';
				}).map(function (node) {
					return { level: node.properties.level, text: node.name.trim() };
				});
				const spans = Array.from(win.document.querySelectorAll(region + ' > span'));
				const expected = spans.map(function (span) {
					return OUTLINE.find(function (heading) { return heading.text === span.textContent; });
				}).filter(Boolean);
				const given = spans.map(function (span) {
					return span.textContent + ' (' + span.getAttribute('role') + ' ' + span.getAttribute('aria-level') + ')';
				}).join(', ');
				expect(found, 'the headings in ' + region + ': ' + given).to.deep.equal(expected);
				expect(found, 'the heading next to the caret').to.deep.include(near);
			});
		};

		cy.then(function () {
			return check('#a11y-context-before', OUTLINE[0]);
		});
		cy.then(function () {
			return check('#a11y-context-after', OUTLINE[1]);
		});
	});

	function applyStyle(style) {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:StyleApply', {
				Style: { type: 'string', value: style },
				FamilyName: { type: 'string', value: 'ParagraphStyles' },
			});
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	it('a heading added to the document reaches the reader', function () {
		move('{ctrl}{end}');
		editing('Results paragraph.');
		move('{enter}Discussion');
		applyStyle('Heading 1');
		toldLevel(1);

		move('{uparrow}');
		editing('Results paragraph.');
		toldNoLevel();
		cy.cGet('#a11y-context-after').should('contain.text', 'Discussion');
		everyHeadingOnce(OUTLINE.concat([{ level: 1, text: 'Discussion' }]), 'with the new heading last');
	});

	it('a heading moved to another level is told the new one', function () {
		move('{downarrow}');
		editing('Discussion');
		applyStyle('Heading 2');
		toldLevel(2);
	});

	function outlineLink(text) {
		return cy.cGet(OUTLINE_REGIONS).find('a').contains(text).then(function (link) {
			return link[0];
		});
	}

	function editableHasFocus() {
		cy.then(function () {
			expect(win.document.activeElement, 'the focus is back on the editable')
				.to.equal(win.document.getElementById('clipboard-area'));
		});
	}

	it('every heading is a link the reader can activate', function () {
		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#a11y-headings-above').then(function (above) {
				return a11yHelper.getAXNodesWithin('#a11y-headings-below').then(function (below) {
					const live = above.concat(below).filter(function (node) { return !node.ignored; });
					const count = function (role) {
						return live.filter(function (node) { return node.role === role; }).length;
					};
					expect(count('heading'), 'headings outside the paragraphs around the caret').to.be.greaterThan(0);
					expect(count('link'), 'a link for each heading').to.equal(count('heading'));
				});
			});
		});
	});

	it('activating a heading moves the caret to it', function () {
		move('{ctrl}{end}');
		outlineLink(OUTLINE[0].text).click({ force: true });
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		editing(OUTLINE[0].text);
		toldLevel(OUTLINE[0].level);
		editableHasFocus();
	});

	it('a heading activated from the keyboard moves the caret to it', function () {
		const last = OUTLINE[OUTLINE.length - 1];
		move('{ctrl}{home}');
		outlineLink(last.text).then(function (link) {
			link.focus();
		});
		cy.realPress('Enter');
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		editing(last.text);
		toldLevel(last.level);
		editableHasFocus();
	});

	it('a key pressed on a heading of the outline moves the caret to it', function () {
		let givenAtFocus = null;
		move('{ctrl}{home}');
		cy.then(function () {
			win.document.getElementById('clipboard-area').addEventListener('focus', function () {
				givenAtFocus = win.document.getElementById('a11y-context-before').textContent;
			}, { once: true });
		});
		cy.cGet('#a11y-headings-below a').first().then(function (link) {
			const at = OUTLINE.findIndex(function (heading) { return heading.text === link.text(); });
			expect(at, 'a heading below the caret that has one before it').to.be.greaterThan(0);
			link[0].focus();
			cy.realPress('ArrowDown');
			cy.then(function () {
				return a11yHelper.processToIdleWithA11yContext(win);
			});
			editing(OUTLINE[at].text);
			toldLevel(OUTLINE[at].level);
			editableHasFocus();
			cy.then(function () {
				expect(givenAtFocus, 'the paragraphs before the heading are given before the focus comes back')
					.to.contain(OUTLINE[at - 1].text);
			});
		});
	});
});
