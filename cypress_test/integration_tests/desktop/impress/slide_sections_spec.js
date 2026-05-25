/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach expect */

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

		it('Section slide selection', function() {
			helper.processToIdle(this.win);

			// 3 sections: Section-1 (slides 1-4), Section-2 (5-11), Section-3 (12-13).
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);

			// Wait until that the has the expected shape before clicking.
			// The section header DOM can be present while the sections array is
			// still being filled in from the late-arriving .uno:SlideSections state.
			cy.window().should((win) => {
				var sections = win['0'].app.impress.sections;
				expect(sections).to.have.length(3);
				expect(sections[0].startIndex).to.equal(0);
				expect(sections[1].startIndex).to.equal(4);
				expect(sections[2].startIndex).to.equal(11);
				expect(win['0'].app.impress.partList).to.have.length(13);
			});

			// Click the body of Section-2's header (anywhere that isn't the toggle).
			cy.cGet('.slide-section-header').eq(1)
				.find('.slide-section-name').click();
			helper.processToIdle(this.win);

			// Use should() so the assertion block retries.
			cy.window().should((win) => {
				var impress = win['0'].app.impress;
				// Slides in Section-2 (indices 4-10) should be selected.
				for (var i = 4; i <= 10; i++)
					expect(impress.isSlideSelected(i)).to.be.true;

				// Slides outside Section-2 should not be selected.
				[0, 1, 2, 3, 11, 12].forEach(function (i) {
					expect(impress.isSlideSelected(i)).to.be.false;
				});
			});
		});

		describe('Drop slide onto section header', function() {

			function dropOnSectionHeader(sectionIndex) {
				cy.cGet('.slide-section-header').eq(sectionIndex).then(function($el) {
					var evt = new Event('drop', { bubbles: true, cancelable: true });
					$el[0].dispatchEvent(evt);
				});
			}

			// Wait until app.impress.sections reflects [0, 4, 11] - the section
			// header DOM can render before the late-arriving SlideSections state
			// has populated app.impress.sections.
			function waitForInitialSections() {
				cy.window().should(function(win) {
					var s = win['0'].app.impress.sections;
					expect(s).to.have.length(3);
					expect(s.map(function(x) { return x.startIndex; }))
						.to.deep.equal([0, 4, 11]);
					expect(win['0'].app.impress.partList).to.have.length(13);
				});
			}

			// TODO: fix and re-enable
			it.skip('Drop sends correct message', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Stub sendMessage so we can inspect what the drop handler emits
				// without letting the message round-trip to the server.
				cy.window().then(function(win) {
					cy.stub(win['0'].app.socket, 'sendMessage').as('sendMessage');
				});

				// Section-2 starts at index 4 -> position = 3, intoSection = 1.
				dropOnSectionHeader(1);
				cy.get('@sendMessage').should('have.been.calledWith',
					'moveselectedclientparts position=3 intoSection=1');

				// Section-1 starts at index 0 -> position = -1, intoSection = 0.
				dropOnSectionHeader(0);
				cy.get('@sendMessage').should('have.been.calledWith',
					'moveselectedclientparts position=-1 intoSection=0');
			});

			it('Drop clears dropsite class', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Simulate the dragover state by adding the class directly.
				cy.cGet('.slide-section-header').eq(1).then(function($el) {
					$el[0].classList.add('slide-section-dropsite');
				});
				cy.cGet('.slide-section-header').eq(1)
					.should('have.class', 'slide-section-dropsite');

				dropOnSectionHeader(1);

				cy.cGet('.slide-section-header').eq(1)
					.should('not.have.class', 'slide-section-dropsite');
			});

			// TODO: fix and re-enable
			it.skip('Drop anchors section to moved slide', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				var slide0Hash;
				cy.window().then(function(win) {
					slide0Hash = win['0'].app.impress.partList[0].hash;
				});

				// Select slide 0 only.
				cy.cGet('#preview-img-part-0').click();
				helper.processToIdle(this.win);
				cy.window().should(function(win) {
					expect(win['0'].app.impress.isSlideSelected(0)).to.be.true;
				});

				// Drop onto Section-2 header.
				dropOnSectionHeader(1);
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					// Section-1 shrinks (loses slide 0), Section-2 anchors to the
					// moved slide, Section-3 stays put.
					expect(impress.sections.map(function(s) { return s.startIndex; }))
						.to.deep.equal([0, 3, 11]);
					expect(impress.partList[3].hash).to.equal(slide0Hash);
				});
			});

			// TODO: fix and re-enable
			it.skip('Emptying a section removes it', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Section-3 owns slides 11-12. Select both.
				cy.cGet('#preview-img-part-11').scrollIntoView().click();
				cy.cGet('#preview-img-part-12').scrollIntoView().click({ ctrlKey: true });
				helper.processToIdle(this.win);
				cy.window().should(function(win) {
					expect(win['0'].app.impress.isSlideSelected(11)).to.be.true;
					expect(win['0'].app.impress.isSlideSelected(12)).to.be.true;
				});

				// Drop on Section-1 header.
				dropOnSectionHeader(0);
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					// Section-3 is gone because all its slides moved out.
					expect(impress.sections).to.have.length(2);
					expect(impress.sections.map(function(s) { return s.name; }))
						.to.deep.equal(['Section-1', 'Section-2']);
				});
			});

			// TODO: fix and re-enable
			it.skip('Multi-slide drop anchors to first selected', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				var slide0Hash;
				cy.window().then(function(win) {
					slide0Hash = win['0'].app.impress.partList[0].hash;
				});

				// Select slides 0 and 1.
				cy.cGet('#preview-img-part-0').click();
				cy.cGet('#preview-img-part-1').click({ ctrlKey: true });
				helper.processToIdle(this.win);
				cy.window().should(function(win) {
					expect(win['0'].app.impress.isSlideSelected(0)).to.be.true;
					expect(win['0'].app.impress.isSlideSelected(1)).to.be.true;
				});

				// Drop on Section-3 header (startIndex 11 -> position 10).
				dropOnSectionHeader(2);
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					// Section-3 anchors to the first moved slide; Section-1 and
					// Section-2 shift up to fill the gap left by the moved slides.
					expect(impress.sections.map(function(s) { return s.startIndex; }))
						.to.deep.equal([0, 2, 9]);
					expect(impress.partList[9].hash).to.equal(slide0Hash);
				});
			});

			// TODO: fix and re-enable
			it.skip('Anchoring persists across reload', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Drop slide 0 onto Section-2 header.
				cy.cGet('#preview-img-part-0').click();
				helper.processToIdle(this.win);
				dropOnSectionHeader(1);
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					expect(win['0'].app.impress.sections.map(function(s) { return s.startIndex; }))
						.to.deep.equal([0, 3, 11]);
				});

				helper.processToIdle(this.win);
				helper.reloadDocument(this.newFilePath);
				cy.getFrameWindow().then((win) => {
					this.win = win;
				});
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					expect(win['0'].app.impress.sections.map(function(s) { return s.startIndex; }))
						.to.deep.equal([0, 3, 11]);
				});
			});
		});

		describe('Section anchors follow insert and delete', function() {
			function waitForInitialSections() {
				cy.window().should(function(win) {
					var s = win['0'].app.impress.sections;
					expect(s).to.have.length(3);
					expect(s.map(function(x) { return x.startIndex; }))
						.to.deep.equal([0, 4, 11]);
					expect(win['0'].app.impress.partList).to.have.length(13);
				});
			}

			// Reproduce the original bug: duplicating the last slide of a
			// section used to push the duplicate into the next section
			// because that section's startIndex did not shift. The new slide
			// must stay in the same section, and the following sections'
			// startIndex must move forward by one.
			it('Duplicating last slide of Section-1 keeps duplicate in Section-1', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Slide 3 (preview index 3) is the last slide of Section-1.
				cy.cGet('#preview-img-part-3').scrollIntoView().rightclick();
				clickContextMenuItem('Duplicate Slide');
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					expect(impress.partList).to.have.length(14);
					// Section-1 grew by one slide; Section-2 / Section-3 shift up.
					expect(impress.sections.map(function(s) { return s.startIndex; }))
						.to.deep.equal([0, 5, 12]);
				});
			});

			// Deleting the only slide of a singleton section drops the
			// section entirely.
			// TODO: fix and re-enable
			it.skip('Deleting only slide of a singleton section drops the section', function() {
				helper.processToIdle(this.win);
				waitForInitialSections();

				// Insert a section starting at slide 5 so the original
				// Section-2 is reduced to a single slide (slide 4).
				cy.cGet('#preview-img-part-4').scrollIntoView().rightclick();
				clickContextMenuItem('Add Section');
				helper.processToIdle(this.win);
				cy.window().should(function(win) {
					expect(win['0'].app.impress.sections).to.have.length(4);
				});

				// Delete slide 4 (the only slide left in Section-2).
				cy.cGet('#preview-img-part-4').scrollIntoView().rightclick();
				clickContextMenuItem('Delete Slide');
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					expect(impress.partList).to.have.length(12);
					expect(impress.sections).to.have.length(3);
					expect(impress.sections.map(function(s) { return s.name; }))
						.to.deep.equal(['Section-1', 'Untitled Section', 'Section-3']);
				});

				// Reload to confirm the dropped section survives a PPTX
				// round-trip and the remaining sections persist correctly.
				helper.processToIdle(this.win);
				helper.reloadDocument(this.newFilePath);
				cy.getFrameWindow().then((win) => {
					this.win = win;
				});
				helper.processToIdle(this.win);

				cy.window().should(function(win) {
					var impress = win['0'].app.impress;
					expect(impress.partList).to.have.length(12);
					expect(impress.sections).to.have.length(3);
					expect(impress.sections.map(function(s) { return s.name; }))
						.to.deep.equal(['Section-1', 'Untitled Section', 'Section-3']);
				});
			});
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

		it('Expand All / Collapse All sections via context menu', function() {
			helper.processToIdle(this.win);

			// 3 sections, 13 slides. Section-1 (0-3), Section-2 (4-10), Section-3 (11-12).
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);

			// Collapse all sections from the first section's context menu.
			rightClickSectionHeader(0);
			clickContextMenuItem('Collapse All Sections');
			helper.processToIdle(this.win);

			// Every slide frame should now have the section-collapsed class.
			for (var i = 0; i < 13; i++) {
				cy.cGet('#preview-frame-part-' + i).should('have.class', 'section-collapsed');
			}

			// Now "Collapse All Sections" should no longer be offered.
			helper.processToIdle(this.win);
			rightClickSectionHeader(0);
			cy.cGet('[id$="-dropdown"]:visible').should('not.contain', 'Collapse All Sections');
			// Pick "Expand All Sections" to undo.
			clickContextMenuItem('Expand All Sections');
			helper.processToIdle(this.win);

			// No slide frame should carry the section-collapsed class.
			for (var j = 0; j < 13; j++) {
				cy.cGet('#preview-frame-part-' + j).should('not.have.class', 'section-collapsed');
			}

			// And the menu should once again offer Collapse All Sections, not Expand All.
			helper.processToIdle(this.win);
			rightClickSectionHeader(0);
			cy.cGet('[id$="-dropdown"]:visible').should('contain', 'Collapse All Sections');
			cy.cGet('[id$="-dropdown"]:visible').should('not.contain', 'Expand All Sections');
		});

		it('Remove Section and Slides via context menu in PPTX', function() {
			helper.processToIdle(this.win);

			// 3 sections, 13 slides: Section-1 (0-3, 4 slides), Section-2 (4-10, 7 slides), Section-3 (11-12, 2 slides).
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);

			// Wait until app.impress.sections and partList reflect the known layout.
			cy.window().should(function(win) {
				var s = win['0'].app.impress.sections;
				expect(s).to.have.length(3);
				expect(s[1].startIndex).to.equal(4);
				expect(s[1].slideCount).to.equal(7);
				expect(win['0'].app.impress.partList).to.have.length(13);
			});

			// Remove Section-2 (index 1) and its 7 slides.
			rightClickSectionHeader(1);
			clickContextMenuItem('Remove Section & Slides');
			cy.cGet('#remove-section-slides-modal-response').click();
			helper.processToIdle(this.win);

			// Section count drops from 3 to 2, slide count drops from 13 to 6.
			cy.window().should(function(win) {
				expect(win['0'].app.impress.sections).to.have.length(2);
				expect(win['0'].app.impress.partList).to.have.length(6);
			});
			assertSectionHeaders(['Section-1', 'Section-3']);

			// Reload and verify the deletion persisted.
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);
			assertSectionHeaders(['Section-1', 'Section-3']);
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

		it('Remove Section and Slides via context menu in ODP', function() {
			helper.processToIdle(this.win);

			// 3 sections, 13 slides: Section-1 (0-3, 4 slides), Section-2 (4-10, 7 slides), Section-3 (11-12, 2 slides).
			assertSectionHeaders(['Section-1', 'Section-2', 'Section-3']);

			// Wait until app.impress.sections and partList reflect the known layout.
			cy.window().should(function(win) {
				var s = win['0'].app.impress.sections;
				expect(s).to.have.length(3);
				expect(s[1].startIndex).to.equal(4);
				expect(s[1].slideCount).to.equal(7);
				expect(win['0'].app.impress.partList).to.have.length(13);
			});

			// Remove Section-2 (index 1) and its 7 slides.
			rightClickSectionHeader(1);
			clickContextMenuItem('Remove Section & Slides');
			cy.cGet('#remove-section-slides-modal-response').click();
			helper.processToIdle(this.win);

			// Section count drops from 3 to 2, slide count drops from 13 to 6.
			cy.window().should(function(win) {
				expect(win['0'].app.impress.sections).to.have.length(2);
				expect(win['0'].app.impress.partList).to.have.length(6);
			});
			assertSectionHeaders(['Section-1', 'Section-3']);

			// Reload and verify the deletion persisted.
			helper.processToIdle(this.win);
			helper.reloadDocument(this.newFilePath);
			assertSectionHeaders(['Section-1', 'Section-3']);
		});
	});
});
