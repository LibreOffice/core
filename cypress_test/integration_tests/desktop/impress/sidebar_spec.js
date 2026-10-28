/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Sidebar Tests', function() {

	beforeEach(function() {
		helper.withoutBrandTheme();
		helper.setupAndLoadDocument('impress/sidebar.odp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('Set gradient background color', function() {
		cy.cGet('#fillattr2').should('not.be.visible');

		cy.cGet('#fillstyle select').select('Gradient');
		cy.cGet('#fillattr2').should('be.visible');
		cy.cGet('#fillattr3').should('be.visible');
		cy.cGet('#fillattr2').click();
		cy.cGet('.modalpopup .jsdialog-container').should('be.visible');
		cy.cGet('#colorwindow_iv_colors.ui-iconview').should('be.visible');
	});

	it('Hidden gradient color buttons take no room in the panel', function() {
		cy.cGet('#fillstyle select').select('Color');
		cy.cGet('#fillattr').should('be.visible');
		helper.processToIdle(this.win);

		// Only the border of a hidden grid cell is left.
		cy.cGet('#fillattr2').should('have.class', 'hidden').invoke('outerWidth').should('be.lessThan', 5);
		cy.cGet('#fillattr3').should('have.class', 'hidden').invoke('outerWidth').should('be.lessThan', 5);
	});

	it.skip('Set underline using popup', function() {
		cy.cGet('#layoutvalueset').should('be.visible');
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();
		cy.cGet('#layoutvalueset').should('not.be.visible');
		cy.cGet('#Underline .arrowbackground').click();
		cy.cGet('.modalpopup .jsdialog-container').should('be.visible');
		cy.cGet('#single').click();
		impressHelper.triggerNewSVGForShapeInTheCenter();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition tspan')
			.should('have.attr', 'text-decoration', 'underline');
	});

	it('Sidebar menubutton visual test', function() {
		cy.cGet('#fillstyle select').select('Color');
		cy.cGet('#fillattr').should('be.visible');
		cy.cGet('#fillattr').should('contain.text', 'Light Blue');
		helper.processToIdle(this.win);
		// The slide layout iconview above #fillattr uses OnDemandRenderer
		// to fetch its preview images. Those round-trips can land after
		// processToIdle returns and shift #fillattr down between the
		// bbox computation and the chrome screenshot. Wait for every
		// in-flight render_entry to be answered first.
		helper.waitForOnDemandRenders(this.win);
		cy.cGet('#fillattr').compareSnapshot('sidebar_menubutton_color', 0.1);
	});

	it('Slide Transition from the View menu shows the panel and activating it again hides it', function() {
		desktopHelper.switchUIToCompact();
		helper.processToIdle(this.win);

		// Impress opens with a deck that comes from core, so the sidebar starts showing.
		cy.cGet('#sidebar-dock-wrapper').should('have.class', 'visible');
		cy.cGet('#sidebar-dock-wrapper').should('have.class', 'coreBased');

		desktopHelper.hideSidebarImpress();

		cy.cGet('#menu-view').click();
		cy.cGet('#menu-transitiondeck').click();

		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		cy.cGet('#sidebar-dock-wrapper').should('have.class', 'visible');
		// The panel is not core content, so the sidebar is no longer marked as core based.
		cy.cGet('#sidebar-dock-wrapper').should('not.have.class', 'coreBased');
		cy.cGet('#transitions-deck').should('be.visible');
		cy.cGet('#transitions-panel').should('be.visible');

		// The same menu entry closes the panel when it is already showing.
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-transitiondeck').click();

		cy.cGet('#sidebar-dock-wrapper').should('not.have.class', 'visible');
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Sidebar decks in compact view', function() {

	beforeEach(function() {
		// The compact toolbar shows the deck buttons only when it is this wide.
		cy.viewport(1920, 1080);
		helper.withoutBrandTheme();
		helper.setupAndLoadDocument('impress/sidebar.odp');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('A deck opened while the sidebar is closed is the only deck shown', function() {
		desktopHelper.hideSidebarImpress();

		// Record each set of decks the dock shows from here on, so a deck that is shown only
		// for a moment is recorded too.
		cy.getFrameWindow().then(function(win) {
			var wrapper = win.document.getElementById('sidebar-dock-wrapper');
			var container = win.document.getElementById('sidebar-container');
			win.shownDecks = [];
			var recordShownDecks = function() {
				if (!wrapper.classList.contains('visible'))
					return;
				// Core sends every deck and the inactive ones are built but not displayed.
				var decks = Array.from(container.querySelectorAll('[id$="Deck"]'))
					.filter(function(deck) { return deck.offsetParent !== null; })
					.map(function(deck) { return deck.id; })
					.filter(function(id, index, ids) { return ids.indexOf(id) === index; })
					.join(',');
				if (win.shownDecks[win.shownDecks.length - 1] !== decks)
					win.shownDecks.push(decks);
			};
			win.shownDecksObserver = new win.MutationObserver(recordShownDecks);
			win.shownDecksObserver.observe(wrapper, { attributes: true, attributeFilter: ['class'] });
			win.shownDecksObserver.observe(container, { childList: true, subtree: true });
		});

		desktopHelper.getCompactIcon('CustomAnimation').find('button').should('not.be.disabled').click();
		helper.processToIdle(this.win);
		cy.cGet('#sidebar-container [id="SdCustomAnimationDeck"]').should('be.visible');

		cy.getFrameWindow().should(function(win) {
			expect(win.shownDecks.join(' then ')).to.equal('SdCustomAnimationDeck');
		}).then(function(win) {
			win.shownDecksObserver.disconnect();
		});
	});
});
