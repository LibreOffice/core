/* -*- js-indent-level: 8 -*- */

/* global describe it cy require expect beforeEach */
var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'JSDialog unit test', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		cy.viewport(1920,1080);
	});

	it('JSDialog popup dialog', function() {
		cy.getFrameWindow()
			.its('L')
			.then(function(L) {
				var jsonDialog = {
					id: 'testpopup',
					jsontype: 'dialog',
					type: 'modalpopup',
					children: [{
						id: 'busycontainer',
						type: 'container',
						vertical: 'true',
						children: [{
							id: 'busylabel',
							type: 'fixedtext',
							text: 'test popup dialog'}]
					}]
				};
				var dialog = L.control.jsDialog();
				var fnClosePopup = cy.spy(dialog, 'closePopover');
				dialog.onJSDialog({data: jsonDialog, callback: function() {}});
				expect(Object.keys(dialog.dialogs)).to.have.length(1);
				dialog.closeAll(false);
				expect(fnClosePopup).to.be.called;
			});
	});

	it('JSDialog child focus', function() {
		cy.getFrameWindow().then(function(win) {
			var smile = win.document.querySelector('meta[name="previewSmile"]').content;
			var jsonDialog = {
				id: 'testfocus',
				type: 'dialog',
				text: 'Focus test',
				children: [{
					id: 'tabcontrol',
					type: 'tabcontrol',
					selected: 1,
					tabs: [{
						text: 'Test Focus',
						id: 1,
						name: 'testfocus'}],
					children: [{
						id: 'tabpage',
						type: 'tabpage',
						enabled: true,
						text: 'Focus',
						children: [{
							id: 'container',
							type: 'container',
							children: [{
								id: 'colorsetwin',
								type: 'scrollwindow',
								children: [{
									id: 'colorset',
									type: 'drawingarea',
									canFocus: true,
									enabled: true,
									imagewidth: 216,
									imageheight: 180,
									image: smile }]}, {
								id: 'testcheck',
								type: 'checkbox',
								text: 'checkbox' }]
						}]
					}]
				}]};

			var dialog = win.L.control.jsDialog();
			dialog.onJSDialog({data: jsonDialog, callback: function() {}});
			expect(Object.keys(dialog.dialogs)).to.have.length(1);
		});

		cy.cGet('#tabcontrol').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			var dialog = win.L.control.jsDialog();
			var current = win.document.activeElement;
			expect(current.id).to.equal('tabcontrol-1');

			cy.realPress('Tab').then(function() {
				var next = win.document.activeElement;
				expect(next.id).to.equal('colorset-img');
				dialog.closeAll(false);
			});
		});
	});

	it('Open hybrid help dialog', function() {
		cy.cGet('#Help-tab-label').click();
		cy.cGet('.unoOnlineHelp').click();
		cy.cGet('#online-help-content').should('exist');
	});

	it('JSDialog dropdown', function() {
		cy.cGet('#toolbar-up #Home .unoConditionalFormatMenu:visible').click();

		desktopHelper.getDropdown('home-conditional-format-menu').should('exist');

		// Click on overlay to close
		cy.cGet('.jsdialog-overlay').click();

		// Dropdown should be closed
		cy.cGet('.jsdialog-overlay').should('not.exist');
		desktopHelper.getDropdown('home-conditional-format-menu').should('not.exist');
	});

	it('JSDialog check enable edit input', function() {
		cy.cGet('#File-tab-label').click();
		cy.cGet('#File-container .unodownloadas button').click();

		// open "PDF options JsDialog"
		cy.cGet('.exportpdf-submenu-icon').click();

		// check watermark checkbox to enable watermark entry input
		cy.cGet('#watermark-input').check();
		// after enable watermark checkbox the input field beside should also be in enabled state
		cy.cGet('#watermarkentry-input').should('not.be.disabled');

	});

	it('JSDialog check data validity options', function() {
		cy.cGet('#Data-tab-label').click();
		cy.cGet('.unoValidation').click();

		// On changing options other fields should toggle enable and disable
		cy.cGet('#data-input').should('be.disabled');
		cy.cGet('#allow-input').select("1");

		cy.cGet('#data-input').should('not.be.disabled');
	});

	it('Sparkline dialog listboxes allow narrow width', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:InsertSparkline');
		});
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.cGet('#cbType').should('be.visible');
		cy.cGet('#cbEmptyCells').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			var typeEl = win.document.getElementById('cbType');
			var emptyEl = win.document.getElementById('cbEmptyCells');
			expect(typeEl.getBoundingClientRect().width).to.be.equal(75);
			expect(emptyEl.getBoundingClientRect().width).to.be.equal(75);
		});

		cy.cGet('.ui-dialog-titlebar-close').click();
		cy.cGet('.ui-dialog[role="dialog"]').should('not.exist');
	});

	it('QuerySelector Syntax error', function(){

		cy.getFrameWindow().then(function(win) {
			cy.spy(win.console, 'error').as('consoleError');
		})

		cy.cGet('#Format-tab-label').click();
		// FIXME: below button has class with "." inside, best to rework it
		cy.cGet('#Format [id^="format-style-dialog"]:visible button').click();
		cy.cGet('#filter-input').select('4');
		cy.wait(500);
		cy.cGet('#flatview .ui-treeview-entry').eq(6).rightclick();
		cy.get('@consoleError').should('not.be.called');
	});
});
