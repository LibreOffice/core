/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach */
var helper = require('../../common/helper');

// A dialog tabcontrol switches to a vertical rail once it has more than four
// tabs. These tests drive the builder directly with injected dialog JSON so
// the tab count is exact and does not depend on a particular core dialog.
describe(['tagdesktop'], 'Dialog vertical tabs', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		cy.viewport(1920, 1080);
	});

	// Build a dialog whose single tabcontrol has the requested number of tabs.
	function injectTabDialog(win, tabCount) {
		var tabs = [];
		var children = [];
		for (var i = 0; i < tabCount; i++) {
			tabs.push({ text: 'Tab ' + i, id: i, name: 'tabpage' + i });
			children.push({
				id: 'tabpage' + i,
				type: 'tabpage',
				enabled: true,
				children: [{ id: 'label' + i, type: 'fixedtext', text: 'content ' + i }],
			});
		}

		var jsonDialog = {
			id: 'tabdialog',
			type: 'dialog',
			text: 'Vertical tabs test',
			children: [{
				id: 'tabcontrol',
				type: 'tabcontrol',
				selected: 0,
				tabs: tabs,
				children: children,
			}],
		};

		var dialog = win.L.control.jsDialog();
		dialog.onJSDialog({ data: jsonDialog, callback: function() {} });
	}

	it('keeps four tabs in a horizontal row', function() {
		cy.getFrameWindow().then(function(win) {
			injectTabDialog(win, 4);
		});

		cy.cGet('#tabcontrol').should('be.visible');
		cy.cGet('#tabcontrol').should('not.have.class', 'vertical');
		cy.cGet('#tabcontrol [role="tablist"]')
			.should('have.attr', 'aria-orientation', 'horizontal')
			.should('not.have.class', 'vertical');

		cy.getFrameWindow().then(function(win) {
			win.L.control.jsDialog().closeAll(false);
		});
	});

	it('stacks more than four tabs in a vertical rail', function() {
		cy.getFrameWindow().then(function(win) {
			injectTabDialog(win, 5);
		});

		cy.cGet('#tabcontrol').should('be.visible');
		cy.cGet('#tabcontrol').should('have.class', 'vertical');
		cy.cGet('#tabcontrol [role="tab"]').should('have.length', 5);
		cy.cGet('#tabcontrol [role="tablist"]')
			.should('have.attr', 'aria-orientation', 'vertical')
			.should('have.class', 'vertical');

		// The panel of the selected tab must keep a real width beside the rail,
		// not collapse to nothing.
		cy.cGet('#tabpage0').should('be.visible');
		cy.cGet('#label0').should('be.visible');
		cy.cGet('#tabpage0').invoke('outerWidth').should('be.greaterThan', 0);

		cy.getFrameWindow().then(function(win) {
			win.L.control.jsDialog().closeAll(false);
		});
	});

	it('switches the active tab when another tab is clicked', function() {
		cy.getFrameWindow().then(function(win) {
			injectTabDialog(win, 5);
		});

		// The first tab is selected on open.
		cy.cGet('#tabcontrol-0').should('have.attr', 'aria-selected', 'true');

		cy.cGet('#tabcontrol-2').click();

		// Selecting another tab moves the active state and reveals its panel.
		cy.cGet('#tabcontrol-2').should('have.attr', 'aria-selected', 'true');
		cy.cGet('#tabcontrol-0').should('have.attr', 'aria-selected', 'false');
		cy.cGet('#tabpage2').should('not.have.class', 'hidden');
		cy.cGet('#tabpage0').should('have.class', 'hidden');

		cy.getFrameWindow().then(function(win) {
			win.L.control.jsDialog().closeAll(false);
		});
	});
});
