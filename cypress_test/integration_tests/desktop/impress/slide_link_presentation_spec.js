/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Linked presentation in the slide sorter', function() {

	// The document holds six slides. Slides 2 to 5 are a copy of the whole of "Q3 deck.odp",
	// linked to it as one presentation over three sections: the section named after the deck,
	// which holds the slide the deck keeps outside any section, "Opening" of two slides and
	// "Closing" of one. The first slide sits in no section and the last in a section of its
	// own. The deck is no remote link of this document here, so nothing can be read from it.
	var presentationId = '{30000000-0000-4000-8000-000000000001}';
	var openingId = '{20000000-0000-4000-8000-000000000002}';

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide-link-presentation.odp');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	// The headers come from the sections state and the link list, which arrive after the
	// document, so a test waits for both before it reads the sorter.
	function waitForLinkedPresentation() {
		cy.getFrameWindow().should(function(win) {
			expect(win.app.impress.partList).to.have.length(6);
			var sectionNames = win.app.impress.sections.map(function(section) {
				return section.name;
			});
			expect(sectionNames).to.deep.equal(['Q3 deck.odp', 'Opening', 'Closing', 'Afterword']);
			var presentationIds = win.app.map.slideLinks.getPresentations().map(function(link) {
				return link.id;
			});
			expect(presentationIds).to.deep.equal([presentationId]);
		});
		cy.cGet('.slide-presentation-header').should('have.length', 1);
	}

	function clickContextMenuItem(text) {
		cy.cGet('[id$="-dropdown"]:visible')
			.contains('.ui-combobox-entry', text).click();
	}

	function assertRunCollapsed(collapsed) {
		[2, 3, 4].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i)
				.should(collapsed ? 'have.class' : 'not.have.class', 'presentation-collapsed');
		});
		cy.cGet('.slide-presentation-header')
			.should(collapsed ? 'have.class' : 'not.have.class', 'collapsed');
	}

	it('shows the linked presentation as one tile under a header with its name', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header .slide-section-name').should('have.text', 'Q3 deck.odp');
		cy.cGet('.slide-presentation-header .slide-section-count').should('have.text', '4 slides');
		cy.cGet('.slide-presentation-header .slide-section-link').should('exist');

		// The header stands above the header of the first section of the presentation.
		cy.cGet('.slide-presentation-header + .slide-section-header .slide-section-name')
			.should('have.text', 'Q3 deck.odp');

		// Collapsed from the start: the first slide of the run shows as the tile, with its
		// own number, and the rest of the run and the headers of its sections are hidden.
		assertRunCollapsed(true);
		cy.cGet('#preview-frame-part-1').should('have.class', 'presentation-tile');
		cy.cGet('#preview-frame-part-1').should('not.have.class', 'presentation-collapsed');
		cy.cGet('#preview-frame-part-1 .preview-slide-number').should('have.text', '2');
		[0, 5].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('not.have.class', 'presentation-collapsed');
		});
		cy.cGet('.slide-section-header.in-presentation').should('have.length', 3);
		cy.cGet('.slide-section-header.in-presentation.presentation-collapsed')
			.should('have.length', 3);
		cy.cGet('.slide-section-header:not(.in-presentation):not(.slide-presentation-header)')
			.should('have.length', 1);

		// The slide after the presentation keeps its number, and the sections of the
		// presentation carry marks of their own.
		cy.cGet('#preview-frame-part-5 .preview-slide-number').should('have.text', '6');
		cy.cGet('.slide-section-header.in-presentation .slide-section-link').should('have.length', 3);
	});

	it('expands and collapses the presentation from its toggle', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header .slide-section-toggle').click();
		assertRunCollapsed(false);
		cy.cGet('#preview-frame-part-1').should('not.have.class', 'presentation-tile');
		cy.cGet('.slide-section-header.in-presentation').should('have.length', 3);
		cy.cGet('.slide-section-header.in-presentation.presentation-collapsed').should('not.exist');

		// A section inside the expanded presentation collapses on its own.
		cy.cGet('.slide-section-header.in-presentation').eq(1).find('.slide-section-toggle').click();
		[2, 3].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
		});
		[1, 4].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('not.have.class', 'section-collapsed');
		});

		cy.cGet('.slide-presentation-header .slide-section-toggle').click();
		assertRunCollapsed(true);
		cy.cGet('.slide-section-header.in-presentation.presentation-collapsed').should('have.length', 3);
	});

	it('selects every slide of the presentation from its header', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header .slide-section-name').click();
		helper.processToIdle(this.win);

		cy.getFrameWindow().should(function(win) {
			var impress = win.app.impress;
			expect(win.app.map._docLayer._selectedPart).to.equal(1);
			[1, 2, 3, 4].forEach(function(i) {
				expect(impress.isSlideSelected(i), 'slide ' + i).to.be.true;
			});
			[0, 5].forEach(function(i) {
				expect(impress.isSlideSelected(i), 'slide ' + i).to.be.false;
			});
		});
	});

	// The panel beside the sorter holds every frame and section header of the presentation,
	// and the sorter holds the frames of the other slides, as many as given.
	function assertPanelOpen(sorterFrames) {
		cy.cGet('.slide-link-panel').should('exist');
		cy.cGet('.slide-link-panel-title .slide-section-name').should('have.text', 'Q3 deck.odp');
		cy.cGet('.slide-link-panel-title .slide-section-link').should('exist');
		[1, 2, 3, 4].forEach(function(i) {
			cy.cGet('.slide-link-panel-deck #preview-frame-part-' + i).should('exist');
			cy.cGet('.slide-link-panel-deck #preview-frame-part-' + i)
				.should('not.have.class', 'presentation-collapsed');
		});
		cy.cGet('.slide-link-panel-deck .slide-section-header').should('have.length', 3);
		cy.cGet('.slide-link-panel-deck .slide-section-header.presentation-collapsed')
			.should('not.exist');
		cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site):not(.link-panel-tile)')
			.should('have.length', sorterFrames);
		// The tile of the presentation stays under its header, with the picture and the number of
		// the first slide.
		cy.cGet('#slide-sorter .slide-presentation-header + .link-panel-tile').should('have.length', 1);
		cy.cGet('#slide-sorter .link-panel-tile .preview-slide-number').should('have.text', '2');
		cy.cGet('#preview-img-part-1').invoke('attr', 'src').then(function(src) {
			cy.cGet('#slide-sorter .link-panel-tile .preview-img').should('have.attr', 'src', src);
		});
		cy.cGet('.slide-presentation-header').should('have.class', 'panel-open');
		cy.cGet('#presentation-controls-wrapper').should('have.class', 'slide-link-panel-open');
	}

	// The panel is gone and the sorter holds every frame in slide order again, with the
	// presentation collapsed to its tile.
	function assertPanelClosed() {
		cy.cGet('.slide-link-panel').should('not.exist');
		cy.cGet('#presentation-controls-wrapper').should('not.have.class', 'slide-link-panel-open');
		cy.cGet('.slide-presentation-header').should('not.have.class', 'panel-open');
		cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site)').should(function(frames) {
			var ids = frames.toArray().map(function(frame) {
				return frame.id;
			});
			expect(ids).to.deep.equal([0, 1, 2, 3, 4, 5].map(function(i) {
				return 'preview-frame-part-' + i;
			}));
		});
		cy.cGet('.slide-presentation-header + .slide-section-header .slide-section-name')
			.should('have.text', 'Q3 deck.odp');
		assertRunCollapsed(true);
		cy.cGet('#preview-frame-part-1').should('have.class', 'presentation-tile');
	}

	it('opens the panel on the presentation when the slide on show lands inside it', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();
		assertRunCollapsed(true);
		cy.cGet('.slide-link-panel').should('not.exist');

		// A hidden slide of the run becomes the current one, by whatever route.
		cy.getFrameWindow().then(function(win) {
			win.app.map.setPart(3);
		});
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(3);
		});
		assertPanelOpen(2);
		cy.cGet('.slide-link-panel-deck #preview-img-part-3')
			.should('have.class', 'preview-img-currentpart');

		// The sidebar grew by a column for the panel.
		cy.cGet('#navigation-sidebar').should(function(sidebar) {
			expect(sidebar.outerWidth()).to.be.greaterThan(400);
		});

		// A slide outside the presentation leaves the panel as it is.
		cy.getFrameWindow().then(function(win) {
			win.app.map.setPart(5);
		});
		helper.processToIdle(this.win);
		assertPanelOpen(2);
	});

	it('opens the panel from a click on the tile and closes it from its button', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('#preview-img-part-1').click();
		helper.processToIdle(this.win);
		assertPanelOpen(2);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(1);
		});

		// A click on a slide in the panel makes it the current one.
		cy.cGet('.slide-link-panel-deck #preview-img-part-3').click();
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(3);
		});
		cy.cGet('.slide-link-panel-deck #preview-img-part-3')
			.should('have.class', 'preview-img-currentpart');

		// A click on the tile in the sorter makes its slide the current one again, and the
		// panel stays open.
		cy.cGet('#slide-sorter .link-panel-tile .preview-img').click();
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(1);
		});
		assertPanelOpen(2);

		// A section in the panel collapses on its own.
		cy.cGet('.slide-link-panel-deck .slide-section-header').eq(1)
			.find('.slide-section-toggle').click();
		[2, 3].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
		});
		cy.cGet('.slide-link-panel-deck .slide-section-header').eq(1)
			.find('.slide-section-toggle').click();
		[2, 3].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('not.have.class', 'section-collapsed');
		});

		cy.cGet('.slide-link-panel-close').click();
		assertPanelClosed();
	});

	it('opens and closes the panel from the header menu and its toggle', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header').rightclick();
		cy.cGet('[id$="-dropdown"]:visible').should('not.contain', 'Close Panel');
		clickContextMenuItem('Open in Panel');
		assertPanelOpen(2);

		cy.cGet('.slide-presentation-header').rightclick();
		cy.cGet('[id$="-dropdown"]:visible').should('not.contain', 'Open in Panel');
		clickContextMenuItem('Close Panel');
		assertPanelClosed();

		// The toggle of the header collapses a presentation shown in the panel.
		cy.cGet('.slide-presentation-header').rightclick();
		clickContextMenuItem('Open in Panel');
		assertPanelOpen(2);
		cy.cGet('.slide-presentation-header .slide-section-toggle').click();
		assertPanelClosed();
	});

	it('keeps the panel open while a slide is inserted after the presentation', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('#preview-img-part-1').click();
		helper.processToIdle(this.win);
		assertPanelOpen(2);

		// The new slide follows the last slide, outside the presentation.
		cy.cGet('#preview-img-part-5').click();
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(5);
		});
		cy.getFrameWindow().then(function(win) {
			win.app.map.insertPage();
		});
		helper.processToIdle(this.win);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.impress.partList).to.have.length(7);
			expect(win.app.map._docLayer._selectedPart).to.equal(6);
		});
		assertPanelOpen(3);
		cy.cGet('.slide-presentation-header .slide-section-count').should('have.text', '4 slides');
	});

	it('unlinks the presentation from its header menu and keeps the links inside it', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header').rightclick();
		// The deck cannot be read, so no update is offered.
		cy.cGet('[id$="-dropdown"]:visible').should('not.contain', 'Update Presentation');
		cy.cGet('[id$="-dropdown"]:visible').should('contain', 'Unlink Presentation & Contents');
		// The entry is matched whole, since the next entry starts with the same words.
		clickContextMenuItem(/^Unlink Presentation$/);
		helper.processToIdle(this.win);

		// The presentation header is gone and its slides show as the sections they are in,
		// each section and slide still linked on its own.
		cy.cGet('.slide-presentation-header').should('not.exist');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map.slideLinks.getPresentations()).to.have.length(0);
			expect(win.app.map.slideLinks.getLink(openingId)).to.not.equal(null);
		});
		[1, 2, 3, 4].forEach(function(i) {
			cy.cGet('#preview-frame-part-' + i).should('not.have.class', 'presentation-collapsed');
		});
		cy.cGet('.slide-section-header.in-presentation').should('not.exist');
		cy.cGet('.slide-section-header .slide-section-link').should('have.length', 3);
		cy.cGet('.preview-link-badge.linked').should('have.length', 4);
	});

	it('takes every link off with the presentation when asked', function() {
		helper.processToIdle(this.win);
		waitForLinkedPresentation();

		cy.cGet('.slide-presentation-header').rightclick();
		clickContextMenuItem('Unlink Presentation & Contents');
		helper.processToIdle(this.win);

		cy.getFrameWindow().should(function(win) {
			expect(win.app.map.slideLinks.hasLinks()).to.equal(false);
		});
		cy.cGet('.slide-presentation-header').should('not.exist');
		cy.cGet('.slide-section-link').should('not.exist');
		cy.cGet('.preview-link-badge.linked').should('not.exist');
		// The sections stay, as sections of this document alone.
		cy.cGet('.slide-section-header .slide-section-name').should('have.length', 4);
	});
});
