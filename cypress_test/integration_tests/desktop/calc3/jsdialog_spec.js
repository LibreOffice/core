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

	// A right-click that reopens a popup sends the close (on mousedown) and the
	// open (on contextmenu) for the same id in one message batch, before the
	// first open's layouting task has put it on screen. The close must cancel
	// that pending open and drop its state at once, so the reopened popup is
	// left intact instead of being torn down by leftover deferred work.
	it('Popup reopened in the same batch survives the close', function() {
		cy.getFrameWindow().then(function(win) {
			var makePopup = function(label) {
				return {
					id: 'testpopup',
					jsontype: 'dialog',
					type: 'modalpopup',
					children: [{
						id: 'popupcontainer',
						type: 'container',
						vertical: 'true',
						children: [{
							id: 'popuplabel',
							type: 'fixedtext',
							text: label }]
					}]
				};
			};

			var dialog = win.L.control.jsDialog();
			// The task-id and cancel checks below deliberately pin the cancel
			// mechanism, not just the end state, so a regression in how the
			// pending open is unwound is caught and not silently reimplemented.
			var cancelSpy = cy.spy(win.app.layoutingService, 'cancelLayoutingTask');

			// open: the popup is only queued as a layouting task, not shown yet
			var firstOpen = makePopup('first popup');
			dialog.onJSDialog({data: firstOpen, callback: function() {}});
			var firstTaskId = firstOpen.openLayoutingTaskId;
			expect(firstTaskId, 'open schedules a cancellable task').to.be.a('string');

			// close arrives before that task runs
			dialog.onJSDialog({data: {id: 'testpopup', jsontype: 'dialog', action: 'close'}, callback: function() {}});

			// the pending open is cancelled and the entry dropped synchronously
			expect(cancelSpy).to.be.calledWith(firstTaskId);
			expect(dialog.dialogs['testpopup'], 'state cleared at once, not deferred').to.be.undefined;

			// the same id reopens in the same batch and stays the live popup
			var secondOpen = makePopup('second popup');
			dialog.onJSDialog({data: secondOpen, callback: function() {}});
			expect(dialog.dialogs['testpopup']).to.equal(secondOpen);

			// drop the still-pending second open so its task does not run later
			dialog.close('testpopup', false);
		});
	});

	// Two full updates for the same id can arrive before either has been put on
	// screen, then a close. The earlier open must be dropped when the second one
	// takes its place, so that no leftover task quietly attaches a popup after
	// the close was meant to clear everything.
	it('Second open cancels the first so nothing is left on screen after close', function() {
		cy.getFrameWindow().then(function(win) {
			var makePopup = function(label) {
				return {
					id: 'testpopup',
					jsontype: 'dialog',
					type: 'modalpopup',
					children: [{
						id: 'popupcontainer',
						type: 'container',
						vertical: 'true',
						children: [{
							id: 'popuplabel',
							type: 'fixedtext',
							text: label }]
					}]
				};
			};

			var dialog = win.L.control.jsDialog();
			var cancelSpy = cy.spy(win.app.layoutingService, 'cancelLayoutingTask');

			// first open is only queued, not shown yet
			var firstOpen = makePopup('first popup');
			dialog.onJSDialog({data: firstOpen, callback: function() {}});
			var firstTaskId = firstOpen.openLayoutingTaskId;
			expect(firstTaskId, 'first open schedules a task').to.be.a('string');

			// a second full update for the same id supersedes the first while it
			// is still queued, so the first open's task is cancelled
			var secondOpen = makePopup('second popup');
			dialog.onJSDialog({data: secondOpen, callback: function() {}});
			var secondTaskId = secondOpen.openLayoutingTaskId;
			expect(cancelSpy, 'superseded open cancelled').to.be.calledWith(firstTaskId);
			expect(firstOpen.openLayoutingTaskId, 'first open no longer cancellable').to.be.null;
			expect(dialog.dialogs['testpopup']).to.equal(secondOpen);

			// the close clears the surviving open before it lays out
			dialog.onJSDialog({data: {id: 'testpopup', jsontype: 'dialog', action: 'close'}, callback: function() {}});
			expect(cancelSpy).to.be.calledWith(secondTaskId);
			expect(dialog.dialogs['testpopup'], 'entry dropped').to.be.undefined;
		});

		// neither cancelled open may resurface once the queue drains
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			expect(win.document.getElementById('testpopup'), 'no popup attached after drain').to.be.null;
		});
	});

	// A popup that is already on screen can get a full update that rebuilds it,
	// then a close before the rebuild lays out. The close abandons the rebuild
	// and removes the container that is actually attached, not the rebuilt one
	// that never made it onto the page, so the old popup does not stay behind.
	it('Closing a popup mid-update also removes the popup still on screen', function() {
		var makePopup = function(label) {
			return {
				id: 'testpopup',
				jsontype: 'dialog',
				type: 'modalpopup',
				children: [{
					id: 'popupcontainer',
					type: 'container',
					vertical: 'true',
					children: [{
						id: 'popuplabel',
						type: 'fixedtext',
						text: label }]
				}]
			};
		};

		// drive the real, map-attached control so the first popup truly shows
		cy.getFrameWindow().then(function(win) {
			win.app.map.jsdialog.onJSDialog({data: makePopup('first popup'), callback: function() {}});
		});
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			var dialog = win.app.map.jsdialog;
			var cancelSpy = cy.spy(win.app.layoutingService, 'cancelLayoutingTask');
			expect(win.document.getElementById('testpopup'), 'popup is on screen').to.not.be.null;

			// a full update rebuilds the popup but its task has not run yet
			var update = makePopup('second popup');
			dialog.onJSDialog({data: update, callback: function() {}});
			var rebuildTaskId = update.openLayoutingTaskId;
			expect(rebuildTaskId, 'update schedules a task').to.be.a('string');

			// the close arrives before the rebuild is attached
			dialog.onJSDialog({data: {id: 'testpopup', jsontype: 'dialog', action: 'close'}, callback: function() {}});

			// the close abandons the in-flight rebuild: its task is cancelled
			// and can no longer be unwound a second time
			expect(cancelSpy, 'pending rebuild cancelled').to.be.calledWith(rebuildTaskId);
			expect(update.openLayoutingTaskId, 'rebuild no longer cancellable').to.be.null;

			// the popup that was on screen is gone and no entry lingers
			expect(win.document.getElementById('testpopup'), 'no popup left on screen').to.be.null;
			expect(dialog.dialogs['testpopup'], 'entry dropped').to.be.undefined;
		});

		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			expect(win.document.getElementById('testpopup'), 'still nothing after drain').to.be.null;
		});
	});

	// Closing a popup that is on screen removes it at once. When the same id
	// reopens right after, that reopen is the popup left on screen, so the user
	// is not left with a popup that vanishes a moment after it appears.
	it('Reopening a popup right after closing it keeps the reopened popup', function() {
		var makePopup = function(label) {
			return {
				id: 'testpopup',
				jsontype: 'dialog',
				type: 'modalpopup',
				children: [{
					id: 'popupcontainer',
					type: 'container',
					vertical: 'true',
					children: [{
						id: 'popuplabel',
						type: 'fixedtext',
						text: label }]
				}]
			};
		};

		// drive the real, map-attached control so the first popup truly shows
		cy.getFrameWindow().then(function(win) {
			win.app.map.jsdialog.onJSDialog({data: makePopup('first popup'), callback: function() {}});
		});
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			var dialog = win.app.map.jsdialog;
			expect(win.document.getElementById('testpopup'), 'popup is on screen').to.not.be.null;

			// the popup is already shown, so the close removes it at once
			dialog.onJSDialog({data: {id: 'testpopup', jsontype: 'dialog', action: 'close'}, callback: function() {}});

			// the same id reopens right after
			var reopen = makePopup('second popup');
			dialog.onJSDialog({data: reopen, callback: function() {}});
			win.__reopenInstance = reopen;

			// the reopen is the live entry
			expect(dialog.dialogs['testpopup']).to.equal(reopen);
		});

		// the reopen lays out
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			var dialog = win.app.map.jsdialog;
			expect(win.document.getElementById('testpopup'), 'reopened popup is on screen').to.not.be.null;
			expect(dialog.dialogs['testpopup'], 'reopen kept, not torn down').to.equal(win.__reopenInstance);

			dialog.close('testpopup', false);
		});
	});

	// Closing a popup that is already on screen tears it down in the same
	// message that arrives, not on a later layouting frame: the container, its
	// overlay and the entry are all gone before anything drains.
	it('Closing a shown popup removes it synchronously', function() {
		var makePopup = function(label) {
			return {
				id: 'testpopup',
				jsontype: 'dialog',
				type: 'modalpopup',
				children: [{
					id: 'popupcontainer',
					type: 'container',
					vertical: 'true',
					children: [{
						id: 'popuplabel',
						type: 'fixedtext',
						text: label }]
				}]
			};
		};

		// drive the real, map-attached control and let the popup fully show
		cy.getFrameWindow().then(function(win) {
			win.app.map.jsdialog.onJSDialog({data: makePopup('shown popup'), callback: function() {}});
		});
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			var dialog = win.app.map.jsdialog;
			expect(win.document.getElementById('testpopup'), 'popup is on screen').to.not.be.null;
			expect(win.document.getElementById('testpopup-overlay'), 'overlay is on screen').to.not.be.null;

			// a shown popup has no pending open task, so this goes through the
			// plain close path
			dialog.onJSDialog({data: {id: 'testpopup', jsontype: 'dialog', action: 'close'}, callback: function() {}});

			// container, overlay and entry are all gone in this same tick, with
			// no layouting drain in between
			expect(win.document.getElementById('testpopup'), 'container removed at once').to.be.null;
			expect(win.document.getElementById('testpopup-overlay'), 'overlay removed at once').to.be.null;
			expect(dialog.dialogs['testpopup'], 'entry dropped at once').to.be.undefined;
		});
	});
});
