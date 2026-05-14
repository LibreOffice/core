/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'JSDialog widgets visual tests', function() {
	beforeEach(function() {
		cy.viewport(1600, helper.maxScreenshotableViewportHeight);
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			cy.cGet('#Help-tab-label').click();
			desktopHelper.getNbIcon('About', 'Help').click();

			cy.cGet('#modal-dialog-about-dialog-box')
				.should('be.visible')
				.should('not.be.empty')
				.contains('#js-dialog a', 'View widgets')
				.click();

			cy.cGet('.ui-dialog[aria-labelledby="Test Widgets"]').should('be.visible');
			// Wait for fadein animation to complete
			cy.cGet('.jsdialog-window.fadein').should('have.css', 'opacity', '1');
		});
	});

	it('Combobox', function() {
		cy.cGet('#combo_box_enable').compareSnapshot('combobox_enable', 0.1);
		cy.cGet('#combo_box_disable').compareSnapshot('combobox_disable', 0.1);
	});

	it('Edit field', function() {
		cy.cGet('#entry_box_1').compareSnapshot('edit_enabled', 0.1);
		cy.cGet('#entry_box_2').compareSnapshot('edit_disabled', 0.1);
	});

	it('Checkbox', function() {
		cy.cGet('#check_btn_1-input').should('have.prop', 'checked', true);
		cy.cGet('#check_btn_1').compareSnapshot('checkbox_checked', 0.1);

		cy.cGet('#check_btn_2-input').should('have.prop', 'checked', false);
		cy.cGet('#check_btn_2').compareSnapshot('checkbox', 0.1);

		cy.cGet('#check_btn_3').scrollIntoView();
		cy.cGet('#check_btn_3-input')
			.should('have.prop', 'checked', true)
			.and('be.disabled');
		helper.processToIdle(this.win);
		cy.cGet('#check_btn_3').compareSnapshot('checkbox_disabled_checked', 0.1);

		cy.cGet('#check_btn_4-input')
			.should('have.prop', 'checked', false)
			.and('be.disabled');
		cy.cGet('#check_btn_4').compareSnapshot('checkbox_disabled', 0.1);
	});

	it('Radio button', function() {
		cy.cGet('#radio_btn_1-input').should('have.prop', 'checked', true);
		cy.cGet('#radio_btn_1').compareSnapshot('radio_checked', 0.1);

		cy.cGet('#radio_btn_2-input').should('have.prop', 'checked', false);
		cy.cGet('#radio_btn_2').compareSnapshot('radio', 0.1);

		cy.cGet('#radio_btn_3').scrollIntoView();
		cy.cGet('#radio_btn_3-input')
			.should('have.prop', 'checked', true)
			.and('be.disabled');
		helper.processToIdle(this.win);
		cy.cGet('#radio_btn_3').compareSnapshot('radio_disabled_checked', 0.1);

		cy.cGet('#radio_btn_4-input')
			.should('have.prop', 'checked', false)
			.and('be.disabled');
		cy.cGet('#radio_btn_4').compareSnapshot('radio_disabled', 0.1);
	});

	it('Treelistbox no-headers', function() {
		cy.cGet('#contenttree').compareSnapshot('treeview_no_headers', 0.05);
	});

	it('Treelistbox focus', function() {
		// with no selection the first entry should be the only tab-focusable entry
		cy.cGet('#contenttree_0').should('have.attr', 'tabindex', '0');
		cy.cGet('#contenttree_1').should('have.attr', 'tabindex', '-1');
		cy.cGet('#contenttree_2').should('have.attr', 'tabindex', '-1');
		cy.cGet('#contenttree').should('not.have.attr', 'tabindex');

		cy.cGet('#contenttree_0').focus();
		helper.assertFocus('id', 'contenttree_0');

		// check that we can navigate inside the widget
		cy.realPress('ArrowDown');
		cy.cGet('#contenttree_1').should('have.focus');
		cy.realPress('ArrowDown');
		cy.cGet('#contenttree_2').should('have.focus');

		// select the third entry
		cy.realPress('Space');
		cy.cGet('#contenttree_2').should('have.class', 'selected');

		// after selection, the selected entry becomes the only tab-focusable entry
		cy.cGet('#contenttree_0').should('have.attr', 'tabindex', '-1');
		cy.cGet('#contenttree_2').should('have.attr', 'tabindex', '0');
		cy.cGet('#contenttree').should('not.have.attr', 'tabindex');
	});

	it.skip('Treelistbox with-headers', function() {
		cy.cGet('#contenttree2').compareSnapshot('treeview_headers', 0.12);

		// use sort feature
		cy.cGet('#contenttree2 .ui-treeview-header-sort-icon').should('be.not.visible');
		cy.cGet('#contenttree2 .ui-treeview-header-button').contains('Column 2').click();
		cy.cGet('#contenttree2 .ui-treeview-header-sort-icon').should('be.visible');
		cy.cGet('#contenttree2 .ui-treeview-header-button').contains('Column 2').click();
		cy.cGet('#contenttree2').compareSnapshot('treeview_headers_sort', 0.1);

		// use filter feature
		cy.cGet('#contenttree2').then(
			(trees) => {
				trees[0].filterEntries('Row 2');
				helper.processToIdle(this.win);
				cy.cGet('#contenttree2').compareSnapshot('treeview_headers_filter', 0.12);
			});
	});
});
