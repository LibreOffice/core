/* global describe it cy beforeEach require expect */

// Live coverage for the Online-native (browser-drawn) Calc Table Design tab:
// the gallery is populated and grouped, the New and Clear entries are present,
// and right-clicking a style offers Set as Default, which the next table uses.

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc Table Design tab (Online-native).', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/testfile.xlsx');
		desktopHelper.switchUIToNotebookbar();
	});

	// Turn a range into a styled table, which switches the context to Table and
	// reveals the contextual Table Design tab.
	function insertTable(range) {
		calcHelper.selectCellsInRange(range || 'A1:C3');
		cy.getFrameWindow().then(function(win) {
			win.app.socket.sendMessage('uno .uno:InsertCalcTable');
		});
		cy.cGet('#tablerangedialog').should('be.visible');
		cy.cGet('#tablerangedialog #ok').click();
		cy.cGet('#tablerangedialog').should('not.exist');
		cy.cGet('#Table-tab-label').should('be.visible');
	}

	it('the browser-drawn gallery is populated and grouped into families', function() {
		insertTable();
		cy.cGet('#tablestyles_design .ui-iconview-entry').should('have.length.greaterThan', 10);
		cy.cGet('#tablestyles_design').contains('.ui-iconview-separator', 'Light').should('exist');
		cy.cGet('#tablestyles_design').contains('.ui-iconview-separator', 'Medium').should('exist');
		cy.cGet('#tablestyles_design').contains('.ui-iconview-separator', 'Dark').should('exist');
	});

	it('the New and Clear Table Style entries are present in the expanded gallery', function() {
		insertTable();
		// Let the gallery updates that follow the table insert settle, so the
		// expanded dropdown is not rebuilt away while the test uses it.
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
		cy.cGet('#tablestyles_design-iconview-list-expand-button').click();
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="new-table-style"]').should('exist');
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="clear-table-style"]').should('exist');
		// Each entry is one menu-style row: the label sits on a single line next
		// to the icon and is not clipped.
		// Each entry is one menu-style row: icon left of a single-line label.
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="new-table-style"] button').then(function(btn) {
			var el = btn[0];
			expect(el.innerText.trim()).to.contain('New Table Style');
			// A wrapped two-line label would roughly double the row height.
			expect(el.getBoundingClientRect().height).to.be.below(40);
			// The label sits beside the icon, not stacked under it.
			expect(window.getComputedStyle(el).flexDirection).to.equal('row');
		});
		cy.cGet('body').type('{esc}');
	});

	it('right-clicking a style offers Set as Default at the pointer', function() {
		insertTable();
		cy.cGet('#tablestyles_design .ui-iconview-entry:visible').eq(1).rightclick();
		cy.cGet('body').contains('Set as Default').should('be.visible');
		// The menu opens at the pointer, which rightclick places at the
		// entry's center; allow a small placement margin.
		cy.cGet('#tablestyles_design .ui-iconview-entry:visible').eq(1).then(function(entry) {
			var rect = entry[0].getBoundingClientRect();
			cy.cGet('body').contains('Set as Default').then(function(item) {
				var menuRect = item[0].getBoundingClientRect();
				var centerX = rect.left + rect.width / 2;
				var centerY = rect.top + rect.height / 2;
				expect(Math.abs(menuRect.left - centerX)).to.be.below(60);
				expect(Math.abs(menuRect.top - centerY)).to.be.below(60);
			});
		});
		// Dismiss the menu so it does not linger into the next test.
		cy.cGet('body').type('{esc}');
	});

	it('right-clicking a style inside the expanded gallery offers Set as Default', function() {
		insertTable();
		// Let the gallery updates that follow the table insert settle, so the
		// expanded dropdown is not rebuilt away while the test uses it.
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
		cy.cGet('#tablestyles_design-iconview-list-expand-button').click();
		// Capture the entry position first: opening the context menu closes the
		// expanded gallery, so the entry cannot be measured afterwards.
		var entryRect = null;
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] .ui-iconview-entry:visible').eq(1).then(function(entry) {
			entryRect = entry[0].getBoundingClientRect();
		});
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] .ui-iconview-entry:visible').eq(1).rightclick();
		cy.cGet('body').contains('Set as Default').should('be.visible');
		// The menu opens at the pointer inside the expanded gallery too.
		cy.cGet('body').contains('Set as Default').then(function(item) {
			var menuRect = item[0].getBoundingClientRect();
			var centerX = entryRect.left + entryRect.width / 2;
			var centerY = entryRect.top + entryRect.height / 2;
			expect(Math.abs(menuRect.left - centerX)).to.be.below(60);
			expect(Math.abs(menuRect.top - centerY)).to.be.below(60);
		});
		// Dismiss the menu.
		cy.cGet('body').type('{esc}');
	});

	it('a table inserted after Set as Default comes up in that style', function() {
		insertTable();
		// The gallery marks the style the table at the cursor uses. Pick a
		// different one as the default, so the check cannot pass by accident.
		var startingEntryId = null;
		cy.cGet('#tablestyles_design .ui-iconview-entry.selected').then(function(entry) {
			startingEntryId = entry[0].id;
		});
		var defaultEntryId = null;
		cy.cGet('#tablestyles_design .ui-iconview-entry:visible').eq(1).then(function(entry) {
			defaultEntryId = entry[0].id;
			expect(defaultEntryId).to.not.equal(startingEntryId);
		});
		cy.cGet('#tablestyles_design .ui-iconview-entry:visible').eq(1).rightclick();
		cy.cGet('body').contains('Set as Default').should('be.visible').click();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// A second table, clear of the first, comes up in the style just chosen.
		insertTable('E1:G3');
		cy.cGet('#tablestyles_design .ui-iconview-entry.selected').should(function(entry) {
			expect(entry[0].id).to.equal(defaultEntryId);
		});
	});

	it('Clear Style leaves the table with no style applied', function() {
		insertTable();
		// The table starts out with a style, which is the state Clear Style undoes.
		// The None entry is the one the gallery marks when nothing is applied.
		var noneEntryId = 'tablestyles_design_-1';
		cy.cGet('#tablestyles_design .ui-iconview-entry.selected').should(function(entry) {
			expect(entry[0].id).to.not.equal(noneEntryId);
		});
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
		cy.cGet('#tablestyles_design-iconview-list-expand-button').click();
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="clear-table-style"] button').click();

		cy.cGet('#tablestyles_design .ui-iconview-entry.selected').should(function(entry) {
			expect(entry[0].id).to.equal(noneEntryId);
		});
	});

	it('the New Table Style entry opens the create dialog', function() {
		insertTable();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
		cy.cGet('#tablestyles_design-iconview-list-expand-button').click();
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="new-table-style"] button').click();
		cy.cGet('#NewTableStyleDialog').should('be.visible');
	});
});
