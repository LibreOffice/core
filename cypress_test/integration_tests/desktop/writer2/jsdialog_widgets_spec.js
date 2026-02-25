/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'JSDialog widgets visual tests', function() {
	beforeEach(function() {
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
		cy.cGet('#check_btn_1').compareSnapshot('checkbox_checked', 0.1);
		cy.cGet('#check_btn_2').compareSnapshot('checkbox', 0.1);
		cy.cGet('#check_btn_3').compareSnapshot('checkbox_disabled_checked', 0.1);
		cy.cGet('#check_btn_4').compareSnapshot('checkbox_disabled', 0.1);
	});

	it('Radio button', function() {
		cy.cGet('#radio_btn_1').compareSnapshot('radio_checked', 0.1);
		cy.cGet('#radio_btn_2').compareSnapshot('radio', 0.1);
		cy.cGet('#radio_btn_3').compareSnapshot('radio_disabled_checked', 0.1);
		cy.cGet('#radio_btn_4').compareSnapshot('radio_disabled', 0.1);
	});

	it('Treelistbox no-headers', function() {
		cy.cGet('#contenttree').compareSnapshot('treeview_no_headers', 0.05);
	});

	it('Treelistbox focus', function() {
		cy.cGet('#link_btn_2').click();
		helper.assertFocus('id','link_btn_2');
		// since no entry is selected the first entry should get focused
		cy.realPress('Tab');
		cy.cGet('#contenttree .ui-treeview-entry:nth-child(1)').should('have.focus');

		// check that we can navigate inside the widget
		cy.realPress('ArrowDown');
		cy.cGet('#contenttree .ui-treeview-entry:nth-child(2)').should('have.focus');
		cy.realPress('ArrowDown');
		cy.cGet('#contenttree .ui-treeview-entry:nth-child(3)').should('have.focus');

		// select the second entry
		cy.realPress('Space');
		cy.cGet('#contenttree .ui-treeview-entry:nth-child(3)').should('have.class', 'selected');

		// check that now the whole widget is no more focusable and tab-indexes are restored and
		// that the next focusable element is the first entry
		cy.cGet('#link_btn_2').click();
		helper.assertFocus('id','link_btn_2');
		cy.realPress('Tab');
		cy.cGet('#contenttree .ui-treeview-entry:nth-child(1)').should('have.focus');
		cy.cGet('#contenttree').should('not.have.attr', 'tabindex');
	});

	it('Treelistbox with-headers', function() {
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
