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
