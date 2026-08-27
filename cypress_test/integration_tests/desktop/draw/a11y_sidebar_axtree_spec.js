/* global describe expect it cy before require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Draw sidebar accessibility tree', { testIsolation: false }, function () {
	let win;

	function getWin() {
		return win;
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		a11yHelper.openSidebarPropertyDeck(getWin);
	});

	it('a panel title announces its text, not the chevron glyph', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		helper.typeIntoDocument('{esc}');

		cy.cGet('.ui-expander-label').first().then(function ($label) {
			const button = $label[0].closest('[tabindex], button') || $label[0];
			button.focus();

			a11yHelper.getFocusedAXNode().then(function (node) {
				// The name comes from the contents, so it has to match them: a
				// decorative pseudo-element must not leak into it.
				expect(node.name.trim(), 'accessible name of the panel title')
					.to.equal($label[0].textContent.trim());
			});
		});
	});

	it('every stop of the deck announces a role and a name', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		helper.typeIntoDocument('{esc}');

		cy.then(function () {
			const order = a11yHelper.sidebarTabOrder(win);
			expect(order.length, 'focusable widgets in the deck').to.be.greaterThan(1);

			order.forEach(function (widget, at) {
				cy.then(function () {
					const target = a11yHelper.sidebarTabOrder(win)[at];
					const where = 'deck widget ' + at + ' ' +
						a11yHelper.describeFocusable(target);
					target.focus();

					a11yHelper.getFocusedAXNode().then(function (node) {
						expect(node, where).to.not.equal(null);
						expect(node.roleType, where + ' exposes an ARIA role')
							.to.equal('role');
						expect(node.name.trim(), where + ' accessible name')
							.to.not.equal('');
						expect(node.ignored, where + ' is in the tree')
							.to.equal(false);
					});
				});
			});
		});
	});
});
