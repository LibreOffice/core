/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Treeview keyboard navigation', { testIsolation: false }, function () {
	var win;

	desktopHelper.shareDocumentAcrossTests('writer/navigator.odt');

	beforeEach(function () {
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		openNavigator();
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	// The navigator icon toggles, so it is clicked only while the tree is away.
	function openNavigator() {
		cy.cGet('body').then(function ($body) {
			if ($body.find('#contenttree:visible').length > 0)
				return;

			cy.cGet('#navigator-floating-icon').click();
		});

		cy.cGet('#contenttree').should('be.visible');
	}

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

	// The expander toggles, so a section an earlier test left open is taken as is and
	// only a closed one is clicked open. Core sends the content tree again whenever the
	// document selection changes, and the tree comes back with its sections closed, so
	// the state is read once the pending updates have been handled and the click is
	// repeated if such an update lands on top of it.
	function expandSectionAndWaitForChild(name, childText) {
		function expandWhileClosed(attemptsLeft) {
			cy.then(function () {
				return helper.processToIdle(win);
			});

			cy.cGet('#contenttree').then(function ($tree) {
				if ($tree.find('.ui-treeview-cell-text:visible:contains("' + childText + '")').length > 0)
					return;

				expect(attemptsLeft, 'attempts left to open ' + name).to.be.greaterThan(0);

				cy.cGet('#contenttree')
					.contains('.jsdialog.sidebar.ui-treeview-cell-text', name)
					.parent()
					.parent()
					.parent()
					.find('.ui-treeview-expander-column')
					.click();

				expandWhileClosed(attemptsLeft - 1);
			});
		}

		expandWhileClosed(3);

		cy.cGet('#contenttree')
			.contains('.ui-treeview-cell-text', childText)
			.should('be.visible');
	}

	it('Navigator treeview has role tree', function () {
		cy.cGet('#contenttree').should('have.attr', 'role', 'tree');
	});

	// The default this checks is the tree as it was built, and clicking an entry moves
	// the tabindex for good, so this test belongs above the ones that click entries.
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

	// The two tests below need the sections closed, which is what the tree looks like
	// when it is built: the star has to find a closed sibling to open, and the letter
	// picks the entry it reaches among the entries on screen. The tests after them
	// leave sections open, and file order is the only thing keeping these two above.
	it('Type-ahead search jumps to matching entry', function () {
		// Start from an entry that carries a name: the tree also holds page divider
		// rows, and their text is empty.
		visibleEntries().first().click();
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// Type 't' to jump to next entry starting with 'T'.
		cy.realPress('t');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// The focused entry should start with 'T'.
		cy.cGet('#contenttree').then(function ($tree) {
			var focused = $tree.find('.ui-treeview-entry:focus');
			var name = focused.find('.ui-treeview-cell-text').first().text();

			expect(name, 'focused entry, class ' + focused.attr('class')).to.match(/^T/);
		});
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

});
