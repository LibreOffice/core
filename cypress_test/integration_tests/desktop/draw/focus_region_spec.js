/* global describe expect it cy before beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// F6 moves the keyboard focus one stop along the ring of application
// regions and wraps around; Shift+F6 walks it backwards. The ring is what
// the client describes for the interface in use, so it is read from
// app.dispatcher.getFocusRegions() rather than listed here: what the tests
// assert is the rule, not a copy of the table the dispatcher already holds.
//
// The document is loaded once and the interface switched in place, so the
// classic tests run first and the notebookbar ones after them.
describe(['tagdesktop'], 'Draw F6 region navigation', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.cGet('#main-menu').should('be.visible');

		preparePanels();
	});

	// The page panel and the sidebar are stops of the ring, so both have to
	// be open. Switching the interface rebuilds them, hence a function.
	function preparePanels() {
		cy.cGet('body').then(function ($body) {
			if ($body.find('#floating-navigator:visible').length)
				cy.cGet('#floating-navigator').click();
		});

		cy.cGet('#sidebar-dock-wrapper').then(function ($dock) {
			if (!$dock.is(':visible')) {
				cy.then(function () {
					win.app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
					return helper.processToIdle(win);
				});
			}
		});
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		// let the page panel and sidebar focus-grabs settle, so neither can
		// steal the focus back mid-test
		cy.then(function () {
			helper.waitUntilLayoutingIsIdle(win);
			helper.waitForTimers(win, 'sidebarstealfocus');
		});
	}

	// Every test walks the ring from the document, so start there.
	beforeEach(function () {
		helper.typeIntoDocument('{esc}');
		assertFocusedRegion('documentArea');
	});

	// The ring as the client describes it for the interface in use.
	function ring() {
		return win.app.dispatcher.getFocusRegions().filter(function (region) {
			return region.available();
		});
	}

	function ringNames() {
		return ring().map(function (region) {
			return region.name;
		});
	}

	// The current stop is the first region claiming the focus, which is how
	// the dispatcher itself finds where it is in the ring.
	function focusedRegion() {
		const current = ring().find(function (region) {
			return region.hasFocus();
		});
		return current ? current.name : 'none';
	}

	function assertFocusedRegion(name) {
		cy.cGet('#main-document-content').should(function () {
			expect(focusedRegion(), 'region holding the keyboard focus').to.equal(name);
		});
	}

	// A region has to hand the focus to a widget, never leave it on the
	// container it is made of.
	function assertFocusOnWidget() {
		cy.cGet('#main-document-content').should(function () {
			const active = win.document.activeElement;
			const focusableOnItsOwn =
				active.isContentEditable ||
				active.hasAttribute('tabindex') ||
				['BUTTON', 'INPUT', 'SELECT', 'TEXTAREA', 'A'].includes(active.tagName);
			expect(focusableOnItsOwn, 'focused ' + active.tagName + '#' + active.id +
				' is focusable on its own').to.be.true;
		});
	}

	// The tab strip is a stop of the ring, so walk to it the way the ring
	// describes rather than counting key presses.
	function focusTheTabStrip() {
		cy.then(function () {
			const names = ringNames();
			const start = names.indexOf('documentArea');
			const target = names.indexOf('topBar');
			expect(target, 'the tab strip is a stop of the ring').to.be.greaterThan(-1);

			const steps = (((target - start) % names.length) + names.length) % names.length;
			for (let i = 0; i < steps; i++) cy.realPress('F6');
		});

		assertFocusedRegion('topBar');
	}

	// One full turn of the ring in the given direction, from the document
	// back to it.
	function walkFullRing(step) {
		cy.then(function () {
			const names = ringNames();
			const start = names.indexOf('documentArea');
			expect(start, 'the document is a stop of the ring').to.be.greaterThan(-1);

			const wrap = function (index) {
				return ((index % names.length) + names.length) % names.length;
			};

			for (let i = 1; i <= names.length; i++) {
				const expected = names[wrap(start + step * i)];
				cy.then(function () {
					cy.realPress(step > 0 ? 'F6' : ['Shift', 'F6']);
					assertFocusedRegion(expected);
					if (expected !== 'documentArea') assertFocusOnWidget();
				});
			}
		});
	}

	describe('classic interface', function () {
		it('offers the toolbar as the stop right after the menubar', function () {
			cy.then(function () {
				const names = ringNames();
				expect(names, 'ring of the classic interface')
					.to.include.members(['topBar', 'topToolbar']);
				expect(names[names.indexOf('topBar') + 1],
					'the stop after the menubar').to.equal('topToolbar');
			});
		});

		it('walks every stop of the ring in both directions', function () {
			walkFullRing(1);
			walkFullRing(-1);
		});
	});

	describe('notebookbar interface', function () {
		before(function () {
			desktopHelper.switchUIToNotebookbar();
			cy.cGet('.notebookbar-tabs-container').should('be.visible');
			preparePanels();
		});

		it('offers the tab strip and not the toolbar', function () {
			cy.then(function () {
				expect(ringNames(), 'ring of the notebookbar interface')
					.to.include('topBar').and.to.not.include('topToolbar');
			});
		});

		it('walks every stop of the ring in both directions', function () {
			walkFullRing(1);
			walkFullRing(-1);
		});

		it('goes back to the selected tab', function () {
			var tabId;

			cy.then(function () {
				const notebookbar = win.app.map.uiManager.notebookbar;
				const tab = notebookbar.getTabs().find(function (candidate) {
					const label = win.document.getElementById(candidate.id);
					return (!candidate.context || candidate.context.includes('default')) &&
						label && !label.classList.contains('selected');
				});
				expect(tab, 'an unselected non-context tab').to.not.be.undefined;
				tabId = tab.id;
			});

			cy.then(function () {
				cy.cGet('#' + tabId).should('be.visible').click();
				cy.cGet('#' + tabId).should('have.class', 'selected');
			});

			helper.typeIntoDocument('{esc}');
			assertFocusedRegion('documentArea');

			cy.realPress(['Shift', 'F6']);
			assertFocusedRegion('navigationSidebar');

			cy.realPress(['Shift', 'F6']);
			assertFocusedRegion('topBar');
			cy.cGet().its('activeElement.id').should(function (id) {
				expect(id, 'focused notebookbar tab').to.equal(tabId);
			});
		});

		it('Tab from the selected tab enters its own page, not the title bar', function () {
			focusTheTabStrip();

			cy.cGet('#main-document-content').should(function () {
				const tab = win.document.activeElement;
				expect(tab.getAttribute('role'), 'the ring landed on a tab').to.equal('tab');
				expect(tab.classList.contains('selected'),
					'focused ' + tab.tagName + '#' + tab.id + ' is the selected tab').to.be.true;
			});

			cy.realPress('Tab');

			cy.cGet('#main-document-content').should(function () {
				const tab = win.document.querySelector('.notebookbar [role="tab"].selected');
				const page = win.document.getElementById(tab.getAttribute('aria-controls'));
				expect(page, 'the panel the selected tab controls').to.not.equal(null);

				const expected = win.JSDialog.GetFocusableElements(page)[0];
				const active = win.document.activeElement;
				expect(active.tagName + '#' + active.id, 'the widget Tab reached')
					.to.equal(expected.tagName + '#' + expected.id);
			});
		});
	});
});
