/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Compact view: the document button brings back a hidden menu bar', function() {

	beforeEach(function() {
		cy.viewport(1280, 800);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function hideMenubarFromViewMenu(win) {
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-togglemenubar').click();
		helper.processToIdle(win);
		cy.cGet('.main-nav').should('not.be.visible');
	}

	it('shows no document button while the menu bar is shown', function() {
		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
		cy.cGet('#documentmenubreak').should('not.be.visible');
	});

	it('shows the document button first in the toolbar once the menu bar is hidden', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('have.class', 'writer-icon-img');
		cy.cGet('#documentmenubreak').should('be.visible');
		cy.cGet('#overflow-manager-toptoolbar > :visible').first()
			.should('have.class', 'documentmenu');
	});

	it('keeps the document button out of the overflow at a narrow width', function() {
		hideMenubarFromViewMenu(this.win);
		cy.viewport(640, 800);
		helper.processToIdle(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
		cy.cGet('.hidden-overflow-container .documentmenu').should('not.exist');
	});

	it('brings the menu bar back from Show Menu Bar and moves focus to File', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').click();
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').click();
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
		cy.cGet('#documentmenubreak').should('not.be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
	});

	it('brings the menu bar back with Enter, Enter and moves focus to File', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu button').focus();
		cy.cGet('#toolbar-up .documentmenu button').should('have.focus');
		cy.realPress('Enter');
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').should('have.focus');
		cy.realPress('Enter');
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
	});

	it('shows no document button when the integrator hid it', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.showButton('documentmenu', false);
		});
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
	});

	function setPermission(permission) {
		cy.getFrameWindow().then(function(win) {
			win.app.map.setPermission(permission);
		});
	}

	it('shows a menu bar hidden by the user in read-only and hides it again in edit', function() {
		hideMenubarFromViewMenu(this.win);

		setPermission('readonly');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('be.visible');

		setPermission('edit');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
	});

	it('moves focus from the Editing switch to the document button when the header hides', function() {
		hideMenubarFromViewMenu(this.win);
		setPermission('readonly');
		helper.processToIdle(this.win);

		cy.cGet('#viewModeDropdownButton-button').focus();
		cy.cGet('#viewModeDropdownButton-button').should('have.focus');
		setPermission('edit');
		helper.processToIdle(this.win);

		cy.cGet('#toolbar-up .documentmenu button').should('have.focus');
	});

	it('keeps a menu bar hidden by the integrator hidden in read-only', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.hideMenubar();
		});
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');

		setPermission('readonly');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');
	});

	it('lists the same File entries as the menu bar', function() {
		cy.cGet('#menu-file').click();
		cy.cGet('#menu-file > ul > li > a:visible').should('have.length.greaterThan', 3);

		cy.getFrameWindow().should(function(win) {
			var expected = win.app.map.menubar.getFileMenuEntries()
				.filter(function(entry) { return entry.type !== 'separator'; })
				.map(function(entry) { return entry.text; });
			var shown = Array.from(win.document.querySelectorAll('#menu-file > ul > li > a:not(.separator)'))
				.filter(function(link) { return link.checkVisibility(); })
				.map(function(link) { return link.firstChild.textContent.trim(); });
			expect(shown).to.deep.equal(expected);
		});
	});

	it('opens a menu with Show Menu Bar and the File entries', function() {
		hideMenubarFromViewMenu(this.win);
		cy.cGet('#toolbar-up .documentmenu button').should('have.attr', 'aria-haspopup', 'true');
		cy.cGet('#toolbar-up .documentmenu button').should('have.attr', 'aria-label', 'File');

		cy.cGet('#toolbar-up .documentmenu').click();
		cy.cGet('#toolbar-up .documentmenu button').should('have.attr', 'aria-expanded', 'true');
		cy.cGet('#documentmenu-entries > [role="option"]').first()
			.should('have.attr', 'data-filter-text', 'show menu bar');
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"] + hr').should('exist');

		cy.getFrameWindow().should(function(win) {
			var expected = win.app.map.menubar.getFileMenuEntries()
				.filter(function(entry) { return entry.type !== 'separator'; })
				.map(function(entry) { return entry.text; });
			var shown = Array.from(win.document.querySelectorAll('#documentmenu-entries > [role="option"]'))
				.slice(1)
				.map(function(entry) { return entry.textContent.trim(); });
			expect(shown).to.deep.equal(expected);
		});
	});

	it('runs a File entry from the menu', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').click();
		cy.cGet('#documentmenu-entries [data-filter-text="options"]').click();

		cy.cGet('#documentmenu-dropdown').should('not.exist');
		cy.cGet('.iframe-settings-wrap').should('be.visible');
	});

	it('offers Show Menu Bar when the integrator hid the menu bar', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.hideMenubar();
		});
		helper.processToIdle(this.win);

		cy.cGet('#toolbar-up .documentmenu').click();
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').click();
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('be.visible');
	});

	it('shows the file name above the menu entries', function() {
		hideMenubarFromViewMenu(this.win);
		cy.cGet('#toolbar-up .documentmenu').click();

		cy.getFrameWindow().should(function(win) {
			var name = win.app.map.wopi.BreadcrumbDocName || win.app.map.wopi.BaseFileName;
			expect(name).to.not.be.empty;
			expect(win.document.getElementById('documentmenu-header').textContent).to.equal(name);
		});
		cy.cGet('#documentmenu-entries').should('have.attr', 'aria-label', 'File');
		cy.cGet('#documentmenu-entries').should('have.attr', 'aria-describedby', 'documentmenu-header');

		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').should('have.focus');
		cy.realPress('ArrowUp');
		cy.cGet('#documentmenu-header').should('not.have.focus');
		cy.cGet('#documentmenu-entries [role="option"]:focus').should('have.length', 1);
	});

	it('keeps the menu at 240px and cuts a long file name in the middle', function() {
		var longName = 'Quarterly report for the Collabora Online product team with the full list of open items and decisions from 2026.odt';
		hideMenubarFromViewMenu(this.win);
		cy.cGet('#toolbar-up .documentmenu').click();
		cy.cGet('#documentmenu-content').invoke('outerWidth').should('be.gte', 240);
		cy.cGet('#documentmenu-dropdown').invoke('outerWidth').as('shortNameWidth');
		cy.realPress('Escape');
		cy.cGet('#documentmenu-dropdown').should('not.exist');

		cy.getFrameWindow().then(function(win) {
			win.app.map.wopi.BreadcrumbDocName = longName;
			win.app.map.topToolbar.updateDocumentFileMenu();
		});
		cy.cGet('#toolbar-up .documentmenu').click();
		cy.get('@shortNameWidth').then(function(shortNameWidth) {
			cy.cGet('#documentmenu-dropdown').invoke('outerWidth').should('equal', shortNameWidth);
		});

		cy.cGet('#documentmenu-header').should('have.attr', 'data-cooltip', longName);
		cy.cGet('#documentmenu-header-end').should('have.text', 'from 2026.odt');
		cy.cGet('#documentmenu-header-end').should(function($end) {
			expect($end[0].scrollWidth).to.be.at.most($end[0].clientWidth);
		});
		cy.cGet('#documentmenu-header-start').should(function($start) {
			expect($start[0].scrollWidth).to.be.greaterThan($start[0].clientWidth);
		});
	});

	it('limits the menu height to the window height minus 38px', function() {
		hideMenubarFromViewMenu(this.win);
		cy.cGet('#toolbar-up .documentmenu').click();

		cy.getFrameWindow().should(function(win) {
			var dropdown = win.document.getElementById('documentmenu-dropdown');
			expect(parseFloat(win.getComputedStyle(dropdown).maxHeight)).to.equal(win.innerHeight - 38);
			expect(win.getComputedStyle(win.document.getElementById('documentmenu-entries')).maxHeight).to.equal('none');
		});
	});

	function pressAltMF() {
		cy.getFrameWindow().then(function(win) {
			win.app.UI.compactViewAccessibility.onDocumentKeyDown({ key: 'Alt' });
			win.app.UI.compactViewAccessibility.onDocumentKeyUp({ key: 'Alt' });
		});
		cy.cGet('body').should('have.class', 'activate-info-boxes');
		cy.realPress('m');
		cy.realPress('f');
	}

	it('opens the document menu with Alt M F while the menu bar is hidden', function() {
		hideMenubarFromViewMenu(this.win);

		pressAltMF();

		cy.cGet('#documentmenu-dropdown').should('be.visible');
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').should('have.focus');
	});

	it('opens File in the menu bar with Alt M F after the menu bar came back', function() {
		hideMenubarFromViewMenu(this.win);
		pressAltMF();
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').click();
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('be.visible');

		pressAltMF();

		cy.cGet('#menu-file > ul').should('be.visible');
		cy.cGet('#documentmenu-dropdown').should('not.exist');
	});
});
