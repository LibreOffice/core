/* global expect describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop'], 'Notebookbar tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showSidebar();
		}

		writerHelper.selectAllTextOfDoc();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function checkCollapsedGroups() {
		// check if dropdown arrows exist for groups
		desktopHelper.getNbIconArrow('Grow');
		desktopHelper.getNbIconArrow('DefaultBullet');
		desktopHelper.getNbIconArrow('InsertTable');
		cy.cGet('.home-search .arrowbackground');
	}

	it('Check collapsed state after mode switch', function() {
		cy.viewport(1280, 600);
		helper.processToIdle(this.win); // stabilize

		checkCollapsedGroups();
		desktopHelper.switchUIToCompact();
		desktopHelper.switchUIToNotebookbar();
		checkCollapsedGroups();
	});

	it('Overflow groups should not folded after switching from tabbed to compact mode', function() {
		// Ensure wide viewport where all groups fit without folding
		cy.viewport(1920, 1080);

		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');

		cy.getFrameWindow().then((win) => {
			helper.processToIdle(win);
		});

		// All foldable groups should be expanded in compact mode
		cy.cGet('#toolbar-up .ui-overflow-group:not(.nofold):not(.ui-overflow-group-container-with-label)')
			.should('have.length', 0);
	});

	it('OverflowGroup collapse state preserved after mode switch', function() {
		cy.viewport(1280, 600);
		helper.processToIdle(this.win);

		// At this width some groups should be collapsed and some expanded
		cy.cGet('.notebookbar .ui-overflow-group:not(.nofold):not(.ui-overflow-group-container-with-label)')
			.should('have.length.greaterThan', 0);
		cy.cGet('.notebookbar .ui-overflow-group.ui-overflow-group-container-with-label')
			.should('have.length.greaterThan', 0);

		// Save the number of expanded groups before switching
		cy.cGet('.notebookbar .ui-overflow-group.ui-overflow-group-container-with-label')
			.then($expanded => cy.wrap($expanded.length).as('expandedCount'));

		// Switch to compact UI via View tab
		desktopHelper.switchUIToCompact();
		// Switch back to notebookbar via View menu
		desktopHelper.switchUIToNotebookbar();

		// Wait for layout stabilization after mode switch
		cy.getFrameWindow().then((win) => {
			helper.processToIdle(win);
		});

		// Verify the number of expanded groups is preserved (bug: all were collapsed)
		cy.get('@expandedCount').then(expandedCount => {
			cy.cGet('.notebookbar .ui-overflow-group.ui-overflow-group-container-with-label')
				.should('have.length', expandedCount);
		});
	});

	it('Apply bold font from dropdown in Format tab', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('.notebookbar #Format-tab-label').click();
		desktopHelper.getNbIconArrow('FormatMenu', 'Format').click();
		desktopHelper.getDropdown('format-FormatMenu').should('exist');
		desktopHelper.getDropdown('format-FormatMenu').contains('.ui-combobox-entry', 'Bold').click();
		desktopHelper.getDropdown('format-FormatMenu').should('not.exist');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Check label showing heuristic', function() {
		// no label
		cy.cGet('.notebookbar .unoBold').should('be.visible');
		cy.cGet('.notebookbar .unoBold span').should('not.exist');

		// with label
		cy.cGet('.notebookbar #Review-tab-label').click();
		cy.cGet('.notebookbar .unoSpellOnline').should('be.visible');
		cy.cGet('.notebookbar .unoSpellOnline span').contains('Auto Spell Check');
	});
});

describe(['tagdesktop'], 'Notebookbar checkbox widgets', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('writer/notebookbar.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#View-tab-label').click();
	});

	it('Ruler Toggle', function() {
		cy.cGet('#showruler').should('be.visible');
		cy.cGet('#showruler-input').should('be.visible').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');

		cy.cGet('#showruler-input').check();
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		cy.cGet('#lo-fline-marker').should('exist');

		// Move the indentation marker.
		cy.cGet('#lo-fline-marker').then(function(items) {
			expect(items).to.have.lengthOf(1);
			const boundingRectangle = items[0].getBoundingClientRect();
			const x1 = boundingRectangle.left;
			const y1 = boundingRectangle.top;

			cy.wrap(x1).as('x1');

			cy.cGet('#lo-fline-marker').realMouseDown(x1, y1); // Press mouse button.
			cy.wait(500);
			cy.cGet('#lo-fline-marker').realMouseMove(x1 + 100, y1); // Move mouse.
			cy.wait(500);
			cy.cGet('#lo-fline-marker').realMouseUp(x1, y1); // Release mouse button.
			cy.wait(500);
			cy.cGet('#lo-fline-marker').realMouseMove(x1, y1); // Move mouse back.
		});

		cy.cGet('#lo-fline-marker').should('be.visible');
		cy.cGet('#lo-fline-marker').then(function(items) {
			expect(items).to.have.lengthOf(1);
			const boundingRectangle = items[0].getBoundingClientRect();
			const x = boundingRectangle.left;

			cy.wait(1000);
			cy.get('@x1').should('not.be.equal', x);
		});

		// Check that there are no tab stops.
		cy.cGet('.cool-ruler-tabstop-left').should('not.exist');
		// Add a tab stop with a double click.
		cy.cGet('.cool-ruler-horizontal-tabstopcontainer').dblclick();
		// Check that a new tab stop is added.
		cy.cGet('.cool-ruler-tabstop-left').should('exist');

		cy.cGet('#showruler-input').uncheck();
		cy.cGet('#showruler-input').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');
	});

	it('Ruler visible after reload', function() {
		cy.cGet('#showruler').should('be.visible');

		// Enable if not checked
		cy.cGet('#showruler-input').then(($input) => {
			if (!$input.is(':checked')) {
				cy.wrap($input).check();
			}
		});

		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// Reload
		helper.reloadDocument(newFilePath);

		// Verify ruler is still visible
		cy.cGet('.cool-ruler').should('be.visible');
	});

	it('StatusBar Toggle', function() {
		cy.cGet('#showstatusbar').should('be.visible');
		cy.cGet('#showstatusbar-input').should('be.visible').should('be.checked');
		cy.cGet('#toolbar-down').should('be.visible');

		cy.cGet('#showstatusbar-input').uncheck();
		cy.cGet('#showstatusbar-input').should('not.be.checked');
		cy.cGet('#toolbar-down').should('not.be.visible');

		cy.cGet('#showstatusbar-input').check();
		cy.cGet('#showstatusbar-input').should('be.checked');
		cy.cGet('#toolbar-down').should('be.visible');
	});
});

describe(['tagdesktop'], 'Notebookbar review operations.', function() {
	it.skip('Go to the next change', function() {
		// Given a document where the first redline is inside a table:
		helper.setupAndLoadDocument('writer/notebookbar-redline.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);

		// When going to the next redline:
		cy.cGet('#Review-tab-label').click();
		cy.cGet('#Review-tab-label').should('have.class', 'selected');
		cy.cGet('#overflow-button-review-tracking .arrowbackground').click();
		cy.cGet('#review-next-tracked-change-button').click();
		cy.cGet('#Table-tab-label').should('not.have.class', 'hidden');

		// Then make sure that by the time the Table tab is visible, the Review tab is still
		// selected:
		// Without the accompanying fix in place, this test would have failed with:
		// AssertionError: Timed out retrying after 10000ms: expected '<button#Review-tab-label.ui-tab.notebookbar>' to have class 'selected'
		cy.cGet('#Review-tab-label').should('have.class', 'selected');
	});
});

describe(['tagdesktop'], 'HideChangeTrackingControls mode tests.', function() {
	it('Check that track change controls are not shown', function() {
		helper.setupAndLoadDocument('writer/hide_change_tracking_controls.odt', /* isMultiUser */ false, /* copyCertificates copies .wopi.json */ true);
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);

		cy.cGet('.notebookbar #Review-tab-label').click();
		cy.cGet('.notebookbar #Review-container').should('exist');
		cy.cGet('.notebookbar #review-tracking').should('not.exist');
		cy.cGet('.notebookbar .unoTrackChanges').should('not.exist');
		cy.cGet('.notebookbar .unoShowTrackedChanges').should('not.exist');
		cy.cGet('.notebookbar .unoNextTrackedChange').should('not.exist');
		cy.cGet('.notebookbar .unoPreviousTrackedChange').should('not.exist');
		cy.cGet('.notebookbar .unoAcceptTrackedChangeToNext').should('not.exist');
		cy.cGet('.notebookbar .unoRejectTrackedChangeToNext').should('not.exist');
		cy.cGet('.notebookbar .unoReinstateTrackedChange').should('not.exist');
		cy.cGet('.notebookbar .unoAcceptTrackedChanges').should('not.exist');
	});
});
