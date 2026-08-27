/* global describe expect it cy before beforeEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Draw bars accessibility tree', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');
	});

	function assertEveryStopAnnounces(selector, what) {
		cy.then(function () {
			const root = win.document.querySelector(selector);
			expect(root, what + ' exists').to.not.equal(null);

			const count = (win.JSDialog.GetFocusableElements(root) || []).length;
			expect(count, what + ' has focusable widgets').to.be.greaterThan(0);

			for (let at = 0; at < count; at++) {
				cy.then(function () {
					const target = win.JSDialog.GetFocusableElements(
						win.document.querySelector(selector))[at];
					const where = what + ' widget ' + at + ' ' +
						a11yHelper.describeFocusable(target);
					target.focus();
					// The tree is read asynchronously, so a focus that never
					// landed would have us assert about somebody else's node.
					expect(win.document.activeElement, where + ' takes the focus')
						.to.equal(target);

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
			}
		});
	}

	beforeEach(function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}
		helper.typeIntoDocument('{esc}');
	});

	it('the notebookbar tab strip', function () {
		assertEveryStopAnnounces('.notebookbar-tabs-container', 'tab strip');
	});

	it('the notebookbar Home tab', function () {
		assertEveryStopAnnounces('#Home-container', 'Home tab');
	});

	it('the navigation sidebar', function () {
		assertEveryStopAnnounces('#navigation-sidebar', 'navigation sidebar');
	});

	it('the status bar', function () {
		assertEveryStopAnnounces('#toolbar-down', 'status bar');
	});

	it('the status bar zoom announces more than its number', function () {
		let shown = null;

		cy.then(function () {
			const button = win.document.querySelector('#toolbar-down #zoom-button');
			expect(button, 'the zoom button exists').to.not.equal(null);
			shown = button.textContent.trim();
			button.focus();
			// The tree is read asynchronously, so a focus that never landed
			// would have us assert about somebody else's node.
			expect(win.document.activeElement, 'the zoom button takes the focus')
				.to.equal(button);

			a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'the zoom button is in the tree').to.not.equal(null);
				expect(node.name, 'the zoom name carries the number')
					.to.contain(shown);
				expect(node.name.trim(), 'the zoom name says more than the number')
					.to.not.equal(shown);
			});
		});

		cy.cGet('#toolbar-down #zoomin-button').click();

		// The status bar rebuilds the zoom widget from a layouting task, so the
		// new number reaches the screen a frame or two after the click. Retry
		// until it is there, then read the button it was rebuilt into.
		cy.cGet('#toolbar-down #zoom .unolabel').should(function ($label) {
			expect($label.text().trim(), 'the zoom changed').to.not.equal(shown);
		});

		cy.then(function () {
			const button = win.document.querySelector('#toolbar-down #zoom-button');
			const now = button.textContent.trim();
			button.focus();
			expect(win.document.activeElement, 'the zoom button takes the focus')
				.to.equal(button);

			a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node.name, 'the name followed the zoom').to.contain(now);
			});
		});
	});

	// The focusable helper does not reach these buttons, so read the label
	// they carry rather than the tree entry of a focus that never lands.
	it('an overflow group is labelled with its name, not its id', function () {
		cy.cGet('#Home-container [id$="-more-button"]').should('have.length.above', 0);
		cy.cGet('#Home-container [id$="-more-button"]').each(function ($button) {
			const id = $button.attr('id');
			expect($button.attr('aria-label'), 'aria-label of ' + id)
				.to.not.contain(id.replace(/-more-button$/, ''));
		});
	});

});
