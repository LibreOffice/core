/* global describe expect it cy before require module */

const helper = require('./helper');
const a11yHelper = require('./a11y_helper');

// The name of a select's value is the value itself, and a filter offers =
// and <= as values, so only a control that should carry a label is held to
// this. A collapsed select reaches the tree as a menu, which is why the
// element is asked rather than the role.
function labelled(node) {
	return !node.ignored && node.name && node.properties.focusable &&
		node.domName !== 'OPTION';
}

function wordless(name) {
	const text = String(name).trim();

	if (!text) return false;

	return !/[a-z0-9]/i.test(text) &&
		text === text.toUpperCase() && text === text.toLowerCase();
}

function symbolOnly(nodes) {
	return nodes.filter(function (node) {
		return labelled(node) && wordless(node.name);
	}).map(function (node) {
		return node.role + ' named "' + node.name.trim() + '"';
	});
}

// A glyph an expander draws is decoration, and a reader that is handed it says
// the symbol instead of the thing. The rule is kept to what is objective: a
// name a reader would have to spell out because it holds no word at all.
//
// moreTests, when given, adds the tests of one app, and is handed the window
// and symbolOnly, which lists the nodes named only with symbols.
function sweepDecorativeGlyphs(app, documentPath, moreTests) {
	describe(['tagdesktop'], app + ' decorative glyphs', { testIsolation: false }, function () {
		let win;

		before(function () {
			cy.viewport(1920, 1080);
			helper.setupAndLoadDocument(documentPath);

			cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
			cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		});

		it('no name a reader is handed is made only of symbols', function () {
			if (!a11yHelper.axTreeAvailable()) {
				this._runnable.title += ' (skipped: needs a chromium browser)';
				this.skip();
			}

			cy.then(function () { return a11yHelper.getAXNodes(); })
				.then(function (nodes) { return a11yHelper.withDomNames(nodes); })
				.then(function (nodes) {
					const reachable = nodes.filter(labelled);

					expect(reachable.length, 'reachable named nodes in the tree')
						.to.be.greaterThan(0);

					expect(symbolOnly(nodes).join(', '), 'controls named only with symbols')
						.to.be.empty;
				});
		});

		it('no image beside a label answers the pointer with a native title', function () {
			cy.then(function () {
				const offenders = Array.from(win.document.querySelectorAll('img[title]'))
					.filter(function (img) {
						return img.checkVisibility({ visibilityProperty: true });
					})
					.map(function (img) {
						return (img.id || img.className || 'img') + ' titled "' +
							img.getAttribute('title') + '"';
					});

				expect(offenders.join(', '), 'images carrying a native title').to.be.empty;
			});
		});

		if (moreTests)
			moreTests({
				win: function () { return win; },
				symbolOnly: symbolOnly,
			});
	});
}

module.exports.sweepDecorativeGlyphs = sweepDecorativeGlyphs;
