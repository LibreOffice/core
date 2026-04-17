/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide sections', function() {

	function assertSectionHeaders(names) {
		cy.cGet('.slide-section-header .slide-section-name').should('have.length', names.length);
		for (var i = 0; i < names.length; i++) {
			cy.cGet('.slide-section-header .slide-section-name').eq(i).should('have.text', names[i]);
		}
	}

	function rightClickSectionHeader(index) {
		cy.cGet('.slide-section-header').eq(index).rightclick();
	}

	function clickContextMenuItem(text) {
		cy.cGet('[id$="-dropdown"]:visible')
			.contains('.ui-combobox-entry', text).click();
	}

	function renameSection(sectionIndex, newName) {
		rightClickSectionHeader(sectionIndex);
		clickContextMenuItem('Rename Section');
		cy.cGet('#input-modal input').clear().type(newName);
		cy.cGet('#response-ok').click();
	}

	describe('PPTX format', function() {

		beforeEach(function() {
			this.newFilePath = helper.setupAndLoadDocument('impress/slide-section-test.pptx');
			desktopHelper.switchUIToNotebookbar();
			cy.getFrameWindow().then((win) => {
				this.win = win;
			});
		});

		it('Open PPTX file with sections', function() {
			helper.processToIdle(this.win);

			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);
		});

		it('Add section in PPTX', function() {
			helper.processToIdle(this.win);

			// Right-click on the last slide which is not a section start
			cy.cGet('.preview-img').last().rightclick();
			clickContextMenuItem('Add Section');
			helper.processToIdle(this.win);

			// Verify new section added with default name
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3', 'Untitled Section']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3', 'Untitled Section']);
		});

		it('Rename section in PPTX', function() {
			helper.processToIdle(this.win);

			// Rename the first section
			renameSection(0, 'Renamed Section');
			helper.processToIdle(this.win);

			assertSectionHeaders(['Renamed Section', 'Section-2', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Renamed Section', 'Section-2', 'Section-3']);
		});

		it('Move section up in PPTX', function() {
			helper.processToIdle(this.win);

			// Move second section up
			rightClickSectionHeader(1);
			clickContextMenuItem('Move Section Up');
			helper.processToIdle(this.win);

			// Verify Section-2 is now first, Section-1 is second
			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);
		});

		it('Move section down in PPTX', function() {
			helper.processToIdle(this.win);

			// Move first section down
			rightClickSectionHeader(0);
			clickContextMenuItem('Move Section Down');
			helper.processToIdle(this.win);

			// Verify Section-2 is now first, Section-1 is second
			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);
		});

		it('Collapse section hides its slides', function() {
			helper.processToIdle(this.win);

			// Starting layout (13 slides, 3 sections): Section-1 (1-4), Section-2 (5-11), Section-3 (12-13)
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);

			// Add a 4th section starting at slide 3
			cy.cGet('#preview-img-part-2').scrollIntoView().rightclick();
			clickContextMenuItem('Add Section');
			helper.processToIdle(this.win);
			// Now: Section-1 (1-2), Untitled Section (3-4), Section-2 (5-11), Section-3 (12-13)
			assertSectionHeaders(['Section-1', 'Untitled Section', 'Section-2', 'Section-3']);

			// Collapse the new Untitled Section (index 1)
			cy.cGet('.slide-section-header').eq(1).find('.slide-section-toggle').click();

			[2, 3].forEach(function (i) {
				cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
			});

			// Collapse Section-3 (index 3)
			cy.cGet('.slide-section-header').eq(3).find('.slide-section-toggle').click();
			[2, 3, 11, 12].forEach(function (i) {
				cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
			});

			// Add another section at slide 8 (index 7, inside Section-2). Slide 8 is
			// inside Section-2 which is visible.
			cy.cGet('#preview-img-part-7').scrollIntoView();
			cy.cGet('#preview-img-part-7').rightclick();
			clickContextMenuItem('Add Section');
			helper.processToIdle(this.win);
			// Layout: Section-1 (1-2), Untitled (3-4), Section-2 (5-7), Untitled (8-11), Section-3 (12-13)
			// The second Untitled Section auto-collapses because it shares the name
			// "Untitled Section" with the already-collapsed first one.

			// Check the .section-collapsed class on each slide's preview-frame
			// rather than visibility - off-screen slides also fail be.visible.
			// Visible: Section-1 (slides 1-2 = index 0,1), Section-2 (slides 5-7 = index 4,5,6)
			[0, 1, 4, 5, 6].forEach(function (i) {
				cy.cGet('#preview-frame-part-' + i).should('not.have.class', 'section-collapsed');
			});

			// Hidden: both Untitled Sections (indices 2,3,7,8,9,10) and Section-3 (indices 11,12)
			[2, 3, 7, 8, 9, 10, 11, 12].forEach(function (i) {
				cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
			});
		});
	});

	describe('ODP format', function() {

		beforeEach(function() {
			this.newFilePath = helper.setupAndLoadDocument('impress/slide-section-test.odp');
			desktopHelper.switchUIToNotebookbar();
			cy.getFrameWindow().then((win) => {
				this.win = win;
			});
		});

		it('Open ODP file with sections', function() {
			helper.processToIdle(this.win);

			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);
		});

		it('Add section in ODP', function() {
			helper.processToIdle(this.win);

			// Right-click on the last slide which is not a section start
			cy.cGet('.preview-img').last().rightclick();
			clickContextMenuItem('Add Section');
			helper.processToIdle(this.win);

			// Verify new section added with default name
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3', 'Untitled Section']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3', 'Untitled Section']);
		});

		it('Rename section in ODP', function() {
			helper.processToIdle(this.win);

			// Rename the first section
			renameSection(0, 'Renamed Section');
			helper.processToIdle(this.win);

			assertSectionHeaders(['Renamed Section', 'Section-2', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Renamed Section', 'Section-2', 'Section-3']);
		});

		it('Move section up in ODP', function() {
			helper.processToIdle(this.win);

			// Move second section up
			rightClickSectionHeader(1);
			clickContextMenuItem('Move Section Up');
			helper.processToIdle(this.win);

			// Verify Section-2 is now first, Section-1 is second
			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);
		});

		it('Move section down in ODP', function() {
			helper.processToIdle(this.win);

			// Move first section down
			rightClickSectionHeader(0);
			clickContextMenuItem('Move Section Down');
			helper.processToIdle(this.win);

			// Verify Section-2 is now first, Section-1 is second
			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);

			// Reload and verify persistence
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);

			assertSectionHeaders(['Section-2', 'Section-1', 'Section-3']);
		});
	});
});
