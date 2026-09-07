/* global describe before beforeEach it cy expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

// The checks are the ones the property deck runs, read from the DOM.
describe(['tagdesktop'], 'Impress shapes deck keyboard navigation',
	{ testIsolation: false }, function () {
	let win;

	function getWin() {
		return win;
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('impress/help_dialog.odp');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		// The View tab entry is the only way the interface offers to this deck.
		cy.cGet('#View-tab-label').click();
		cy.cGet('#View-container').should('be.visible');
		cy.cGet('#View-container [modelId="view-shapes-deck"] button').click();

		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		cy.cGet('#DefaultShapesPanel').should('exist');
		// An entry redrawn while the focus walks would move the ground under it.
		cy.cGet('#DefaultShapesPanel .ui-iconview-entry img').should('exist');

		cy.then(function () {
			helper.waitUntilLayoutingIsIdle(win);
		});
	});

	beforeEach(function () {
		helper.typeIntoDocument('{esc}');
		helper.assertFocus('className', 'clipboard');
	});

	it('every gallery of the deck is one tab stop', function () {
		cy.then(function () {
			const panel = win.document.querySelector('#DefaultShapesPanel');
			const galleries = panel.querySelectorAll('.ui-iconview');
			const entries = panel.querySelectorAll('.ui-iconview-entry');
			const tabbable = panel.querySelectorAll('.ui-iconview-entry[tabindex="0"]');

			expect(galleries.length, 'galleries in the panel').to.not.equal(0);
			expect(entries.length, 'shapes across the galleries')
				.to.be.greaterThan(galleries.length);
			// A gallery is one stop, not one per shape: the arrows move inside it.
			expect(tabbable.length, 'shapes that Tab stops on')
				.to.equal(galleries.length);
		});
	});

	it('no widget of the deck forces its own tab position', function () {
		a11yHelper.sidebarKeyboard.assertNoForcedTabPosition(getWin);
	});

	it('the focusable helper reports the tab order of the deck', function () {
		a11yHelper.sidebarKeyboard.assertHelperReportsTabOrder(getWin);
	});

	it('F6 enters the deck on its first widget', function () {
		a11yHelper.sidebarKeyboard.assertRingEntersOnFirstWidget(getWin);
	});

	it('Tab walks the deck in order and Shift+Tab comes back', function () {
		a11yHelper.sidebarKeyboard.assertTabWalksTheDeck(getWin);
	});

	it('F6 leaves the deck from a widget inside it', function () {
		a11yHelper.sidebarKeyboard.assertRingLeavesTheDeck(getWin);
	});
	it('every shape of a gallery is focusable, so the arrows reach it', function () {
		cy.then(function () {
			const panel = win.document.querySelector('#DefaultShapesPanel');
			const galleries = Array.from(panel.querySelectorAll('.ui-iconview[id]'));

			expect(galleries, 'galleries in the panel').to.not.be.empty;

			galleries.forEach(function (gallery) {
				const entries = Array.from(gallery.children);
				expect(entries, 'shapes of ' + gallery.id).to.not.be.empty;
				entries.forEach(function (entry) {
					expect(win.JSDialog.IsFocusable(entry),
						entry.id + ' is focusable').to.equal(true);
				});
			});
		});
	});

	it('the arrows walk a gallery to its last shape', function () {
		let entries;
		let gallery;

		cy.then(function () {
			const panel = win.document.querySelector('#DefaultShapesPanel');
			// The widest gallery, so the walk crosses several rows.
			gallery = Array.from(panel.querySelectorAll('.ui-iconview[id]'))
				.reduce(function (widest, candidate) {
					return candidate.children.length > widest.children.length
						? candidate : widest;
				});
			entries = Array.from(gallery.children);
			expect(entries.length, 'shapes of ' + gallery.id).to.be.greaterThan(6);

			entries[0].focus();
			expect(win.document.activeElement.id, 'the walk starts on the first shape')
				.to.equal(entries[0].id);
		});

		cy.then(function () {
			for (let at = 1; at < entries.length; at++) {
				cy.realPress('ArrowRight');
				cy.then(function () {
					expect(win.document.activeElement.id, 'shape ' + at + ' of the gallery')
						.to.equal(entries[at].id);
				});
			}
		});
	});

	it('ArrowDown steps by the columns the gallery is laid out in', function () {
		cy.then(function () {
			const gallery = win.document.querySelector('#DefaultShapesPanel .ui-iconview[id]');
			const columns = win.getComputedStyle(gallery).gridTemplateColumns
				.split(' ').filter(function (track) { return track.length > 0; }).length;
			const entries = Array.from(gallery.children);

			expect(columns, 'columns of ' + gallery.id).to.be.greaterThan(1);
			expect(entries.length, 'shapes of ' + gallery.id).to.be.greaterThan(columns);

			entries[0].focus();
			cy.realPress('ArrowDown');
			cy.then(function () {
				expect(win.document.activeElement.id, 'the shape one row down')
					.to.equal(entries[columns].id);
			});
		});
	});

	// Last: showing the accelerators puts info boxes on the page.
	it('every gallery carries the accelerator the definitions give it', function () {
		cy.wrap(null, { timeout: 20000 }).should(function () {
			if (!win.app.UI.notebookbarAccessibility.initialized)
				throw new Error('accessibility not initialized yet');
		});

		// The attribute is put on when the accelerators are shown.
		cy.then(function () {
			const a11y = win.app.UI.notebookbarAccessibility;
			a11y.mayShowAcceleratorInfoBoxes = true;
			a11y.onDocumentKeyUp({ keyCode: 18 });
		});

		cy.then(function () {
			const combinations = win.app.UI.notebookbarAccessibility.definitions
				.sidebarCombinations[win.app.map.getDocType()] || {};
			const panel = win.document.querySelector('#DefaultShapesPanel');
			const galleries = Array.from(panel.querySelectorAll('.ui-iconview[id]'));

			expect(galleries, 'galleries in the panel').to.not.be.empty;

			galleries.forEach(function (gallery) {
				expect(combinations, 'a combination for ' + gallery.id)
					.to.have.property(gallery.id);
				expect(gallery.getAttribute('accesskey'),
					'accesskey of ' + gallery.id)
					.to.equal(combinations[gallery.id]);
			});
		});
	});
	// The key being drawn is not the same as the key doing something.
	it('the accelerator of a gallery reaches the gallery', function () {
		cy.wrap(null, { timeout: 20000 }).should(function () {
			if (!win.app.UI.notebookbarAccessibility.initialized)
				throw new Error('accessibility not initialized yet');
		});

		cy.realPress('Alt');
		cy.realPress('K');
		cy.realPress('L');

		cy.cGet('#LinesArrows').should(function () {
			const gallery = win.document.getElementById('LinesArrows');
			const active = win.document.activeElement;
			const where = active ? active.tagName + '#' + active.id : 'nothing';

			expect(gallery.contains(active),
				'KL left the focus at ' + where + ', not inside LinesArrows')
				.to.be.true;
		});
	});
});
