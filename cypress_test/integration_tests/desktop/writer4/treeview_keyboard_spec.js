/* global describe it cy require beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Treeview keyboard navigation', function () {
	var win;

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/navigator.odt');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.cGet('#navigator-floating-icon').click();
		cy.cGet('#contenttree').should('be.visible');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	function visibleEntries() {
		return cy.cGet('#contenttree .ui-treeview-entry:not(.page-divider-row):visible');
	}

	function focusEntryByText(text) {
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', text)
			.click();
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function expandSectionAndWaitForChild(name, childText) {
		cy.cGet('#contenttree')
			.contains('.jsdialog.sidebar.ui-treeview-cell-text', name)
			.parent()
			.parent()
			.parent()
			.find('.ui-treeview-expander-column')
			.click();
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', childText)
			.should('be.visible');
	}

	it('Navigator treeview has role tree', function () {
		cy.cGet('#contenttree').should('have.attr', 'role', 'tree');
	});

	it('Tabindex: only one (first by default) entry has tabindex 0', function () {
		visibleEntries().each(function (el, index) {
			if (index === 0)
				cy.wrap(el).should('have.attr', 'tabindex', '0');
			else
				cy.wrap(el).should('have.attr', 'tabindex', '-1');
		});
	});

	it('Tabindex: selected entry keeps tabindex 0 after focus leaves', function () {
		focusEntryByText('Headings');

		// Move focus out of the tree.
		cy.cGet('div.clipboard').focus();

		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Headings')
			.closest('.ui-treeview-entry')
			.should('have.attr', 'tabindex', '0');
	});

	it('Home key moves focus to the first entry', function () {
		focusEntryByText('Tables');
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Tables')
			.closest('.ui-treeview-entry')
			.should('have.focus');

		cy.realPress('Home');

		visibleEntries().first().should('have.focus');
	});

	it('End key moves focus to the last entry', function () {
		focusEntryByText('Headings');
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Headings')
			.closest('.ui-treeview-entry')
			.should('have.focus');

		cy.realPress('End');

		visibleEntries().last().should('have.focus');
	});

	it('ArrowRight on expanded node moves focus to first child', function () {
		// Expand Tables so children are visible.
		expandSectionAndWaitForChild('Tables', 'Table15');

		// Focus on the Tables entry (the parent).
		focusEntryByText('Tables');

		// ArrowRight on an expanded node should move to the first child.
		cy.realPress('ArrowRight');

		// The first child of Tables should have focus.
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Tables')
			.closest('.ui-treeview-entry')
			.next('.ui-treeview-expanded-content')
			.find('.ui-treeview-entry')
			.first()
			.should('have.focus');
	});

	it('ArrowLeft on child node moves focus to parent', function () {
		// Expand Tables so children are visible.
		expandSectionAndWaitForChild('Tables', 'Table15');

		// Focus on the Tables entry.
		focusEntryByText('Tables');

		// Press ArrowDown two times to move to second child, 
		// then ArrowLeft moves back focus to parent.
		cy.realPress('ArrowDown');
		cy.realPress('ArrowDown');
		cy.realPress('ArrowLeft');

		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Tables')
			.closest('.ui-treeview-entry')
			.should('have.focus');
	});

	it('ArrowLeft on expanded node collapses it', function () {
		// Expand Tables so children are visible.
		expandSectionAndWaitForChild('Tables', 'Table15');

		// Focus on the Tables entry.
		focusEntryByText('Tables');

		// ArrowLeft should collapse the expanded node.
		cy.realPress('ArrowLeft');

		// After collapsing, Table15 should not be visible.
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Table15')
			.should('not.be.visible');
	});

	it('Asterisk expands all collapsed siblings', function () {
		focusEntryByText('Headings');

		cy.realPress('*');

		// Focus should not move from Headings.
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Headings')
			.closest('.ui-treeview-entry')
			.should('have.focus');

		// Children of Headings should become visible (Headings was expanded).
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Contributors')
			.should('be.visible');

		// Children of Tables should also be present (sibling at the same
		// level was expanded). Scroll into view since the expanded tree
		// may be taller than the navigator viewport.
		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', 'Table15')
			.scrollIntoView()
			.should('be.visible');
	});

	it('Type-ahead search jumps to matching entry', function () {
		// Click on the tree container to ensure it can receive focus.
		cy.cGet('#contenttree .ui-treeview-entry:visible').first().click();

		// Type 't' to jump to next entry starting with 'T'.
		cy.realPress('t');

		// The focused entry should start with 'T'.
		cy.cGet('#contenttree .ui-treeview-entry:focus')
			.find('.ui-treeview-cell-text')
			.first()
			.invoke('text')
			.should('match', /^T/);
	});
});
