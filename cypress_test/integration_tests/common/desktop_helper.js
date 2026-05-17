/* global cy  require Cypress expect */

var helper = require('./helper');

// Make the sidebar visible by clicking on the corresponding toolbar item.
// We assume that the sidebar is hidden, when this method is called.

function showSidebar() {
	cy.log('>> showSidebar - start');

	cy.cGet('#sidebar').should('not.have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
	cy.cGet('#sidebar').click();
	cy.cGet('#sidebar').should('have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('be.visible').should('not.be.empty');

	cy.log('<< showSidebar - end');
}

// Hide the sidebar by clicking on the corresponding toolbar item.
// We assume that the sidebar is visible, when this method is called.
function hideSidebar() {
	cy.log('>> hideSidebar - start');

	cy.cGet('#sidebar').should('have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('be.visible').should('not.be.empty');
	cy.cGet('#sidebar').click();
	cy.cGet('#sidebar').should('not.have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

	cy.log('<< hideSidebar - end');
}

function hideSidebarImpress() {
	cy.log('>> hideSidebarImpress - start');

	cy.cGet('.unoModifyPage').should('have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('be.visible').should('not.be.empty');
	cy.cGet('.unoModifyPage button').click();
	cy.cGet('.unoModifyPage').should('not.have.class', 'selected');
	cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

	cy.log('<< hideSidebarImpress - end');
}

function sidebarToggle() {
	cy.log('>> sidebarToggle');
	// for notebookbar
	cy.cGet('#optionscontainer [id^="SidebarDeck.PropertyDeck"] button').click();
}

// Ensure the sidebar dock is hidden, regardless of starting state.
function ensureSidebarHidden() {
	cy.log('>> ensureSidebarHidden');
	cy.cGet('#sidebar-dock-wrapper').then(function($dock) {
		if ($dock.is(':visible')) {
			cy.cGet('#optionscontainer [id^="SidebarDeck.PropertyDeck"] button').click();
		}
	});
	cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
	cy.log('<< ensureSidebarHidden - end');
}

// Wait for the sidebar to reshow and grab focus to its first focusable
// element via the 'sidebarstealfocus' timer scheduled in
// Control.Sidebar.ts.
function assertSidebarStealsFocus(win) {
	cy.log('>> assertSidebarStealsFocus - start');

	cy.cGet('#sidebar-dock-wrapper').should('be.visible').then(function(sidebar) {
		// This first waitUntilLayoutingIsIdle ensures the sidebar
		// update task has run and registered the timer.
		helper.waitUntilLayoutingIsIdle(win);
		// Wait for that timer to complete
		helper.waitForTimers(win, 'sidebarstealfocus');
		// Wait until dispatched tasks get done
		helper.waitUntilLayoutingIsIdle(win);
		helper.containsFocusElement(sidebar[0], true);
	});

	cy.log('<< assertSidebarStealsFocus - end');
}

// Make the status bar visible if it's hidden at the moment.
// We use the menu option under 'View' menu to make it visible.
function showStatusBarIfHidden() {
	cy.log('>> showStatusBarIfHidden - start');

	cy.cGet('#toolbar-down')
		.then(function(statusbar) {
			if (!Cypress.dom.isVisible(statusbar[0])) {
				cy.get('#menu-view')
					.click();

				cy.get('#menu-showstatusbar')
					.click();
			}
		});

	cy.cGet('#toolbar-down').should('be.visible');

	cy.log('<< showStatusBarIfHidden - end');
}

// Select a color from colour palette widget used on top toolbar.
// Parameters:
// color - a hexadecimal color code without the '#' mark (e.g. 'FF011B')
function selectColorFromPalette(color) {
	cy.log('>> selectColorFromPalette - start');

	cy.cGet('.ui-color-picker').should('be.visible');
	cy.cGet('.ui-color-picker-entry[value="' + color + '"]').click();
	cy.cGet('.ui-color-picker').should('not.exist');

	cy.log('<< selectColorFromPalette - end');
}

// Select an item from a listbox/combobox widget used on top toolbar.
// Parameters:
// item - item string, that we use a selector to find the right list item.
function selectFromListbox(item) {
	cy.log('>> selectFromListbox - start');

	cy.cGet('[id$="-dropdown"].modalpopup').should('be.visible');
	cy.cGet('[id$="-dropdown"].modalpopup').contains('span', item).click();
	cy.cGet('[id$="-dropdown"].modalpopup').should('not.exist');

	cy.log('<< selectFromListbox - end');
}

// Select an item from a JSDialog dropdown widget used on top toolbar.
// Parameters:
// item - item string, that we use a selector to find the right list item.
function selectFromJSDialogListbox(item, isImage) {
	cy.log('>> selectFromJSDialogListbox - start');

	cy.cGet('[id$="-dropdown"].modalpopup').should('be.visible');
	if (isImage) {
		cy.wait(1000); // We need some time to render custom entries
		cy.cGet('[id$="-dropdown"].modalpopup img[alt="' + item + '"]').click();
	} else
		cy.cGet('[id$="-dropdown"].modalpopup').contains('span', item).click();

	cy.cGet('[id$="-dropdown"].modalpopup').should('not.exist');

	cy.log('<< selectFromJSDialogListbox - end');
}

// Make sure the right dialog is opened and then we close it.
// Used for tunneled dialogs. We don't interact with this dialogs
// now, because they are just images. We mostly just check that
// the dialog is open.
// Parameters:
// dialogTitle - a title string to make sure the right dialog was opened.
function checkDialogAndClose(dialogTitle) {
	cy.log('>> checkDialogAndClose - start');

	// Dialog is opened
	cy.cGet('.lokdialog_canvas').should('be.visible');
	cy.cGet('.ui-dialog-title').should('have.text', dialogTitle);

	// Close the dialog
	cy.cGet('body').type('{esc}');
	cy.cGet('.lokdialog_canvas').should('not.exist');

	cy.log('<< checkDialogAndClose - end');
}

// Checks wether the document has the given zoom level according to the status bar.
// Parameters:
// zoomLevel        the expected zoom level (e.g. '100' means 100%).
function shouldHaveZoomLevel(zoomLevel) {
	cy.log('>> shouldHaveZoomLevel - start');

	cy.cGet('#toolbar-down #zoom .unolabel').should('have.text', zoomLevel);

	cy.log('<< shouldHaveZoomLevel - end');
}

// Make the zoom related status bar items visible if they are hidden.
// The status bar can be long to not fit on the screen. We have a scroll
// item for navigation in this case.
function makeZoomItemsVisible() {
	cy.log('>> makeZoomItemsVisible - start');

	// Ensure the #zoomin element is visible
	cy.cGet('#toolbar-down #zoomin').should('be.visible');

	cy.log('<< makeZoomItemsVisible - end');
}

// Increase / decrease the zoom level using the status bar related items.
// Parameters:
// zoomIn - do a zoom in (e.g. increase zoom level) or zoom out.
function doZoom(zoomIn) {
	cy.log('>> doZoom - start');

	var prevZoom = '';
	cy.cGet('#toolbar-down #zoom .unolabel')
		.should(function(zoomLevel) {
			prevZoom = zoomLevel.text();
			expect(prevZoom).to.not.equal('');
		});

	// Force because sometimes the icons are scrolled off the screen to the right
	if (zoomIn) {
		cy.cGet('#toolbar-down #zoomin').click();
	} else {
		cy.cGet('#toolbar-down #zoomout').click();
	}

	// Wait for animation to complete
	cy.wait(500);

	cy.cGet('#toolbar-down #zoom .unolabel')
		.should(function(zoomLevel) {
			expect(zoomLevel.text()).to.not.equal(prevZoom);
		});

	cy.log('<< doZoom - end');
}

// Zoom in the document.
function zoomIn() {
	doZoom(true);
}

// Zoom out of the document.
function zoomOut() {
	doZoom(false);
}

// Similarly to zoomIn() and zoomOut() we can change the zoom level.
// However, in this case we can specify the exact zoom level.
// Parameters:
// zoomLevel - a number specifing the zoom level  (e.g. '100' means 100%).
//             See also the status bar's zoom level list for possible values.
function selectZoomLevel(zoomLevel, makeZoomVisible = true) {
	cy.log('>> selectZoomLevel - start');

	// Force because sometimes the icons are scrolled off the screen to the right
	if (makeZoomVisible)
		makeZoomItemsVisible();
	cy.cGet('#toolbar-down #zoom .arrowbackground').click();
	cy.cGet('#zoom-dropdown').contains('.ui-combobox-entry', zoomLevel).click();
	shouldHaveZoomLevel(zoomLevel);

	cy.log('<< selectZoomLevel - end');
}

// Reset zoom level to 100%.
// WARN: doesn't work for writer, use fitWidthZoom for that.
function resetZoomLevel() {
	cy.log('>> resetZoomLevel - start');

	// Force because sometimes the icons are scrolled off the screen to the right
	cy.cGet('#toolbar-down #zoomreset').click();
	shouldHaveZoomLevel('100');

	cy.log('<< resetZoomLevel - end');
}

function fitWidthZoom() {
	cy.log('>> fitWidthZoom - start');

	cy.cGet('#toolbar-down #fitwidthzoom').click();

	cy.log('<< fitWidthZoom - end');
}

function insertImage() {
	cy.log('>> insertImage - start');

	selectZoomLevel('50', false);

	const mode = Cypress.env('USER_INTERFACE');

	if (mode === 'notebookbar') {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoInsertGraphic').filter(':visible').click();
	} else {
		cy.cGet('#toolbar-up .unoInsertGraphic').filter(':visible').click();
	}

	cy.cGet('#insertgraphic[type=file]').attachFile('/desktop/writer/image_to_insert.png');
	cy.cGet('#document-container svg g').should('exist');

	cy.log('<< insertImage - end');
}

function insertVideo() {
	cy.log('>> insertVideo - start');

	selectZoomLevel('50', false);

	cy.cGet('#Insert-tab-label').click();
	cy.cGet('#Insert-container .inline.insertmultimedia').click();

	cy.cGet('#insertmultimedia[type=file]').attachFile(
		'/desktop/impress/video_to_insert.mp4'
	);
	cy.cGet('#document-container svg foreignObject video').should('exist');

	cy.log('<< insertVideo - end');
}

function deleteImage() {
	cy.log('>> deleteImage - start');

	helper.typeIntoDocument('{del}');
	cy.cGet('#document-container svg g').should('not.exist');

	cy.log('<< deleteImage - end');
}

function closeNavigatorSidebar () {
	cy.cGet('.close-navigation-button').click();
}

function insertComment(text = 'some text0', save = true) {
	cy.log('>> insertComment - start');

	cy.getFrameWindow().then(function(win) {
		return helper.processToIdle(win);
	});

	var mode = Cypress.env('USER_INTERFACE');
	if (mode === 'notebookbar') {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert .unoInsertAnnotation').click();
	} else {
		cy.cGet('#menu-insert').click();
		cy.cGet('#menu-insertcomment').click();
	}

	// Wait for the annotation to be created
	cy.cGet('.cool-annotation').last({log: false}).find('#annotation-modify-textarea-new').should('exist');
	// Wait for core to process and layouting to settle so the textarea has its final ID
	cy.getFrameWindow().then(function(win) {
		return helper.processToIdle(win);
	});

	// Use class selector since processToIdle may have caused the textarea ID to change from 'new' to a number
	cy.cGet('.cool-annotation').last({log: false}).find('.modify-annotation .cool-annotation-textarea').should('not.have.attr','disabled');
	cy.cGet('.cool-annotation').last({log: false}).find('.modify-annotation .cool-annotation-textarea').type(text);
	// Check that comment exists
	cy.cGet('.cool-annotation').last({log: false}).find('.cool-annotation-textarea').should('contain',text);

	if (save) {
		cy.cGet('.cool-annotation').last({log: false}).find('[value="Save"]').click();
		cy.cGet('.cool-annotation').last({log: false}).find('.modify-annotation').should('not.be.visible');
		cy.cGet('.cool-annotation').last({log: false}).find('.cool-annotation-content').should('contain',text);
		// Comments can be automatically hidden after save in some cases,
		// so we can't check that the final content is visible
		// cy.cGet('.cool-annotation').last({log: false}).find('.cool-annotation-content').should('be.visible');

		// Wait for the animation to stop
		cy.cGet('.cool-annotation').last({log: false}).invoke('attr','style').should('not.contain','transition');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
	} else {
		cy.cGet('.cool-annotation').last({log: false}).find('.cool-annotation-content').should('not.be.visible');
		cy.cGet('.cool-annotation').last({log: false}).find('.modify-annotation').should('be.visible');
	}

	cy.log('<< insertComment - end');
}


function toggleComments(resolved = false) {
	cy.log('>> toggleComments - start');

	var mode = Cypress.env('USER_INTERFACE');
	if (mode === 'notebookbar') {
		cy.cGet('#Review-tab-label').click();
		if (resolved) getNbIcon('ShowResolvedAnnotations', 'Review').click();
		else cy.cGet('#showannotations').click();
		// to avoid notebookbar collapse in subsequent calls to toggleComments.
		cy.cGet('#Home-tab-label').click();
	} else {
		cy.cGet('#menu-view').click();
		if (resolved) cy.cGet('#menu-showresolved').click();
		else cy.cGet('#menu-showannotations').click();
	}

	cy.log('>> toggleComments - end');
}

function switchUIToNotebookbar() {
	cy.log('>> switchUIToNotebookbar - start');

	cy.window().then(win => {
		var userInterfaceMode = win['0'].userInterfaceMode;
		if (userInterfaceMode !== 'notebookbar') {
			cy.cGet('#menu-view').click();
			cy.cGet('#menu-toggleuimode').click();
		}
		Cypress.env('USER_INTERFACE', 'notebookbar');
	});

	cy.log('<< switchUIToNotebookbar - end');
}

function switchUIToCompact() {
	cy.log('>> switchUIToCompact - start');

	cy.window().then(win => {
		var userInterfaceMode = win['0'].userInterfaceMode;
		if (userInterfaceMode === 'notebookbar') {
			cy.cGet('#View-tab-label').click();
			getNbIcon('toggleuimode', 'View').click();
			// Wait for the compact toolbar to load
			cy.cGet('#toolbar-up').should('be.visible');
		}
	});

	cy.log('<< switchUIToCompact - end');
}

function actionOnSelector(name, func) {
	cy.task('getSelectors', {
		mode: Cypress.env('USER_INTERFACE'),
		name: name,
	}).then((selector) => {
		func(selector);
	});
}

//type represent horizontal/vertical scrollbar
//arr : In both cypress GUI and CLI the scrollposition are slightly different
//so we are passing both in array and assert using oneOf
function assertScrollbarPosition(type, lowerBound, upperBound) {
	cy.log('>> assertScrollbarPosition - start');

	cy.cGet('#test-div-' + type + '-scrollbar')
		.should(function($item) {
			const x = parseInt($item.text());
			expect(x).to.be.within(lowerBound, upperBound);
		});

	cy.log('<< assertScrollbarPosition - end');
}

function pressKey(n, key) {
	cy.log('>> pressKey - start');

	for (let i=0; i<n; i++) {
		helper.typeIntoDocument('{' + key + '}');
	}

	cy.log('<< pressKey - end');
}

function openReadOnlyFile(filePath) {
	cy.log('>> openReadOnlyFile - start');

	var newFilePath = helper.setupDocument(filePath);
	// Skip document checks because sidebar does not appear
	helper.loadDocument(filePath, true);

	//check doc is loaded
	cy.cGet('.leaflet-canvas-container canvas', {timeout : Cypress.config('defaultCommandTimeout') * 2.0});

	helper.isCanvasWhite(false);

	cy.cGet('#PermissionMode').should('be.visible').should('have.text', ' Read-only ');

	cy.log('<< openReadOnlyFile - end');
	return newFilePath;
}

function checkAccessibilityEnabledToBe(state) {
	cy.log('>> checkAccessibilityEnabledToBe - start');

	cy.window().then(win => {
		cy.log('check accessibility enabled to be: ' + state);
		var isAccessibilityEnabledAtServerLevel = win['0'].enableAccessibility;
		// expect(isAccessibilityEnabledAtServerLevel).to.eq(true);
		if (isAccessibilityEnabledAtServerLevel) {
			var userInterfaceMode = win['0'].userInterfaceMode;
			if (userInterfaceMode === 'notebookbar') {
				if (state) {
					cy.cGet('#togglea11ystate').should('have.class', 'selected');
				} else {
					cy.cGet('#togglea11ystate').should('not.have.class', 'selected');
				}
			} else {
				cy.cGet('#menu-tools').click();
				if (state) {
					cy.cGet('#menu-togglea11ystate a').should('have.class', 'lo-menu-item-checked');
				} else {
					cy.cGet('#menu-togglea11ystate a').should('not.have.class', 'lo-menu-item-checked');
				}
				cy.cGet('div.clipboard').type('{esc}', {force: true});
			}
			cy.cGet('div.clipboard').then((clipboard) => {
				expect(clipboard.get(0)._hasAccessibilitySupport()).to.eq(state);
			});
		} else {
			cy.log('accessibility disabled at server level');
		}
	});

	cy.log('<< checkAccessibilityEnabledToBe - end');
}

function setAccessibilityState(enable) {
	cy.log('>> setAccessibilityState - start');

	cy.window().then(win => {
		cy.log('set accessibility state to: ' + enable);
		var a11yEnabled = win['0'].enableAccessibility;
		if (a11yEnabled) {
			var userInterfaceMode = win['0'].userInterfaceMode;
			if (userInterfaceMode === 'notebookbar') {
				cy.cGet('#Help-tab-label').click();
				cy.cGet('#togglea11ystate').then((button) => {
					//var currentState = button.get(0).classList.contains('selected');
					var currentState = button.hasClass('selected');
					if (currentState !== enable) {
						button.click();
						cy.log('accessibility state changed: ' + enable);
					} else {
						cy.log('accessibility already in requested state: ' + enable);
					}
				});
			} else  {
				cy.cGet('#menu-tools').click();
				cy.cGet('#menu-togglea11ystate a').then((item) => {
					var currentState = item.hasClass('lo-menu-item-checked');
					if (currentState !== enable) {
						cy.cGet('#menu-togglea11ystate').click();
						cy.log('accessibility state changed: ' + enable);
					} else {
						cy.log('accessibility already in requested state: ' + enable);
					}
				});
			}
			cy.cGet('div.clipboard').then((clipboard) => {
				expect(clipboard.get(0)._hasAccessibilitySupport()).to.eq(enable);
			});
		} else {
			cy.log('accessibility disabled at server level');
		}
	});

	cy.log('<< setAccessibilityState - end');
}

// best way to simulate scrolling using mouse wheel I found -> no click performed
function scrollWriterDocumentToTop() {
	cy.getFrameWindow()
		.its('L')
		.then(function(L) {
			L.Map.THIS.panTo({lat: -58.62652658901402, lng: 68.96288389396415});
		});
	assertScrollbarPosition('vertical', 0, 10);
}

function scrollViewDown(win) {
	cy.then(function() {
		win.L.Map.THIS.panBy({x: 0, y: 4000});
		win.app.updateFollowingUsers();
	});
}

function updateFollowingUsers() {
	cy.getFrameWindow()
		.its('app')
		.then(function(app) {
			app.updateFollowingUsers();
		});
}

function assertVisiblePage(min, max, allPages) {
	const expectedArray = [];

	if (min === max) {
		expectedArray.push('Page ' + min + ' of ' + allPages);
	} else {
		expectedArray.push('Page ' + min + ' of ' + allPages);
		expectedArray.push('Pages ' + min + ' and ' + max + ' of ' + allPages);
		expectedArray.push('Page ' + max + ' of ' + allPages);
	}

	cy.cGet('#StatePageNumber').invoke('text').should('be.oneOf', expectedArray);
}

/// get icon for given uno command from classic toolbar
function getCompactIcon(unoCommand) {
	return cy.cGet('#toolbar-up .uno' + unoCommand + ':visible');
}

/// get icon for given uno command from notebookbar
function getNbIcon(unoCommand, tabName) {
	return cy.cGet((tabName ? '#' + tabName + '-container' : '') + '.notebookbar  .uno' + unoCommand + ' > button.unobutton:visible');
}

/// get icon arrow for given uno command from classic toolbar to open the dropdown
function getCompactIconArrow(unoCommand) {
	return cy.cGet('#toolbar-up .uno' + unoCommand + ' > .arrowbackground:visible');
}

/// get icon arrow for given uno command from notebookbar to open the dropdown
function getNbIconArrow(unoCommand, tabName) {
	return cy.cGet((tabName ? '#' + tabName + '-container' : '') + '.notebookbar  .uno' + unoCommand + ' > .arrowbackground:visible');
}

/// get gropdown element for menu with given id
function getDropdown(dropdownId) {
	return cy.cGet('[id^="' + dropdownId + '"].modalpopup');
}

// Undo all changes until undo is no longer possible
function undoAll() {
	cy.log('>> undoAll - start');

	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
		cy.cGet('#Home-container .unoUndo').then(function undoStep($undo) {
			if ($undo.attr('disabled') === undefined) {
				cy.cGet('#Home-container .unoUndo button').click({force: true});
				helper.processToIdle(win);
				cy.cGet('#Home-container .unoUndo').then(undoStep);
			}
		});
	});

	cy.cGet('#Home-container .unoUndo').should('not.have','disabled');

	cy.log('<< undoAll - end');
}

module.exports.showSidebar = showSidebar;
module.exports.hideSidebar = hideSidebar;
module.exports.hideSidebarImpress = hideSidebarImpress;
module.exports.showStatusBarIfHidden = showStatusBarIfHidden;
module.exports.selectColorFromPalette = selectColorFromPalette;
module.exports.selectFromListbox = selectFromListbox;
module.exports.selectFromJSDialogListbox = selectFromJSDialogListbox;
module.exports.checkDialogAndClose = checkDialogAndClose;
module.exports.makeZoomItemsVisible = makeZoomItemsVisible;
module.exports.zoomIn = zoomIn;
module.exports.zoomOut = zoomOut;
module.exports.shouldHaveZoomLevel = shouldHaveZoomLevel;
module.exports.selectZoomLevel = selectZoomLevel;
module.exports.resetZoomLevel = resetZoomLevel;
module.exports.fitWidthZoom = fitWidthZoom;
module.exports.insertImage = insertImage;
module.exports.insertVideo = insertVideo;
module.exports.deleteImage = deleteImage;
module.exports.insertComment = insertComment;
module.exports.toggleComments = toggleComments;
module.exports.actionOnSelector = actionOnSelector;
module.exports.assertScrollbarPosition = assertScrollbarPosition;
module.exports.pressKey = pressKey;
module.exports.openReadOnlyFile = openReadOnlyFile;
module.exports.switchUIToNotebookbar = switchUIToNotebookbar;
module.exports.switchUIToCompact = switchUIToCompact;
module.exports.checkAccessibilityEnabledToBe = checkAccessibilityEnabledToBe;
module.exports.setAccessibilityState = setAccessibilityState;
module.exports.scrollWriterDocumentToTop = scrollWriterDocumentToTop;
module.exports.scrollViewDown = scrollViewDown;
module.exports.updateFollowingUsers = updateFollowingUsers;
module.exports.assertVisiblePage = assertVisiblePage;
module.exports.closeNavigatorSidebar = closeNavigatorSidebar;
module.exports.sidebarToggle = sidebarToggle;
module.exports.ensureSidebarHidden = ensureSidebarHidden;
module.exports.assertSidebarStealsFocus = assertSidebarStealsFocus;
module.exports.getCompactIcon = getCompactIcon;
module.exports.getNbIcon = getNbIcon;
module.exports.getCompactIconArrow = getCompactIconArrow;
module.exports.getNbIconArrow = getNbIconArrow;
module.exports.getDropdown = getDropdown;
module.exports.undoAll = undoAll;
