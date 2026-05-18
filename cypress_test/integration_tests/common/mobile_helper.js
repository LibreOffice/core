/* global cy expect require Cypress */

var helper = require('./helper');

// Enable editing if we are in read-only mode.
function enableEditingMobile() {
	cy.log('>> enableEditingMobile - start');

	cy.cGet('#mobile-edit-button').click();

	cy.cGet('#toolbar-mobile-back').should('have.class', 'editmode-on');
	cy.cGet('#toolbar-mobile-back').should('not.have.class', 'editmode-off');

	cy.log('Enabling editing mode - now editable.');

	// Wait until all UI update is finished.
	cy.cGet('#toolbar-down').should('be.visible');

	cy.log('Enabling editing mode - toolbar update done.');

	helper.doIfInCalc(function() {
		cy.cGet('#formulabar')
			.should('be.visible');
	});

	// In writer, we should have the blinking cursor visible
	// after stepping into editing mode.
	helper.doIfInWriter(function() {
		cy.cGet('.blinking-cursor')
			.should('be.visible');
	});

	// Wait until core has processed outstanding input and has returned to idle.
	cy.getFrameWindow().then((win) => {
		helper.processToIdle(win);
	});

	cy.log('<< enableEditingMobile - end');
}

function longPressOnDocument(posX, posY) {
	cy.log('>> longPressOnDocument - start');
	cy.log('Param - posX: ' + posX);
	cy.log('Param - posY: ' + posY);

	cy.cGet('.leaflet-pane.leaflet-map-pane')
		.then(function(items) {
			expect(items).have.length(1);

			var eventOptions = {
				force: true,
				button: 0,
				pointerType: 'mouse',
				x: posX - items[0].getBoundingClientRect().left,
				y: posY - items[0].getBoundingClientRect().top
			};

			cy.cGet('.leaflet-pane.leaflet-map-pane')
				.trigger('pointerdown', eventOptions)
				.trigger('pointermove', eventOptions);

			// Wait for long press
			// This value is set in Map.TouchGesture.js.
			cy.wait(500);

			cy.cGet('.leaflet-pane.leaflet-map-pane')
				.trigger('pointerup', eventOptions);
		});

	cy.log('<< longPressOnDocument - end');
}

function openHamburgerMenu() {
	cy.log('>> openHamburgerMenu - start');

	cy.cGet('#toolbar-hamburger')
		.should('not.have.class', 'menuwizard-opened');

	cy.cGet('#toolbar-hamburger .main-menu-btn-icon')
		.click({force: true});

	cy.cGet('#toolbar-hamburger')
		.should('have.class', 'menuwizard-opened');

	cy.cGet('#mobile-wizard-content-menubar')
		.should('not.be.empty');

	cy.log('<< openHamburgerMenu - end');
}

function closeHamburgerMenu() {
	cy.log('>> closeHamburgerMenu - start');

	cy.cGet('#toolbar-hamburger')
		.should('have.class', 'menuwizard-opened');

	cy.cGet('#toolbar-hamburger .main-menu-btn-icon')
		.click({force: true});

	cy.cGet('#toolbar-hamburger')
		.should('not.have.class', 'menuwizard-opened');

	cy.cGet('#mobile-wizard-content-menubar')
		.should('not.exist');

	cy.log('<< closeHamburgerMenu - end');
}

function openMobileWizard() {
	cy.log('>> openMobileWizard - start');

	// Drain any in-flight core work first so the toggle isn't racing
	// against a pending sidebar update.
	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	// Open mobile wizard
	cy.cGet('#toolbar-up #mobile_wizard')
		.should('not.have.class', 'disabled')
		.click();

	// Mobile wizard is opened and it has content
	cy.cGet('#mobile-wizard-content')
		.should('not.be.empty');
	cy.cGet('#toolbar-up #mobile_wizard')
		.should('have.class', 'selected');

	cy.log('<< openMobileWizard - end');
}

function closeMobileWizard() {
	cy.log('>> closeMobileWizard - start');

	cy.cGet('#toolbar-up #mobile_wizard')
		.should('have.class', 'selected');

	cy.cGet('#toolbar-up #mobile_wizard')
		.click();

	cy.cGet('#mobile-wizard')
		.should('not.be.visible');
	cy.cGet('#toolbar-up #mobile_wizard')
		.should('not.have.class', 'selected');

	cy.log('<< closeMobileWizard - end');
}

function openInsertionWizard() {
	cy.log('>> openInsertionWizard - start');

	// Drain any in-flight core work first so the toggle isn't racing
	// against a pending update.
	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	cy.cGet('#toolbar-up #insertion_mobile_wizard')
		.should('not.have.class', 'disabled');

	cy.cGet('#toolbar-up #insertion_mobile_wizard button')
		.click();

	cy.cGet('#mobile-wizard-content')
		.should('not.be.empty');

	cy.cGet('#toolbar-up #insertion_mobile_wizard')
		.should('have.class', 'selected');

	cy.log('<< openInsertionWizard - end');
}

function openCommentWizard() {
	cy.log('>> openCommentWizard - start');

	cy.cGet('#toolbar-up #comment_wizard')
		.should('not.have.class', 'disabled')

	cy.cGet('#toolbar-up #comment_wizard button')
		.click();

	cy.cGet('#toolbar-up #comment_wizard')
		.should('have.class', 'selected');

	cy.log('<< openCommentWizard - end');
}

function closeInsertionWizard() {
	cy.log('>> closeInsertionWizard - start');

	cy.cGet('#toolbar-up #insertion_mobile_wizard')
		.should('have.class', 'selected');

	cy.cGet('#toolbar-up #insertion_mobile_wizard')
		.click();

	cy.cGet('#mobile-wizard')
		.should('not.be.visible');

	cy.cGet('#toolbar-up #insertion_mobile_wizard')
		.should('not.have.class', 'selected');

	cy.log('<< closeInsertionWizard - end');
}

/// deprecated: see selectFromColorPicker function instead
function selectFromColorPalette(paletteNum, groupNum, paletteAfterChangeNum, colorNum) {
	cy.log('>> selectFromColorPalette - start');

	cy.cGet('#color-picker-' + paletteNum.toString() + '-basic-color-' + groupNum.toString()).click();
	if (paletteAfterChangeNum !== undefined && colorNum !== undefined) {
		cy.cGet('#color-picker-' + paletteAfterChangeNum.toString() + '-tint-' + colorNum.toString()).click();
	}
	cy.cGet('#mobile-wizard-back').click();

	cy.log('<< selectFromColorPalette - end');
}

function selectFromColorPicker(pickerId, groupNum, colorNum) {
	cy.log('>> selectFromColorPicker - start');

	cy.cGet(pickerId + ' [id^=color-picker-][id$=-basic-color-' + groupNum.toString() + ']')
		.click();

	if (colorNum !== undefined) {
		cy.cGet(pickerId + ' [id^=color-picker-][id$=-tint-' + colorNum.toString() + ']')
			.click();
	}

	// TODO: Verify color is selected in picker
	// See: https://github.com/CollaboraOnline/online/issues/9036

	cy.cGet('#mobile-wizard-back')
		.click();

	// TODO: Verify color is selected, as in:
	//cy.cGet(pickerId + ' .color-sample-selected')
	//    .should('have.attr', 'style', 'background-color: rgb(204, 0, 0);');

	cy.log('<< selectFromColorPicker - end');
}

function openTextPropertiesPanel() {
	cy.log('>> openTextPropertiesPanel - start');

	openMobileWizard();

	cy.cGet('#TextPropertyPanel, .TextPropertyPanel').click();

	cy.cGet('.unoBold').should('be.visible');

	cy.log('<< openTextPropertiesPanel - end');
}

function selectHamburgerMenuItem(menuItems) {
	cy.log('>> selectHamburgerMenuItem - start');
	cy.log('Param - menuItems: ' + menuItems);

	openHamburgerMenu();

	for (var i = 0; i < menuItems.length; i++) {
		cy.cGet('body').contains('.menu-entry-with-icon', menuItems[i])
			.click();

		if (Cypress.env('INTEGRATION') !== 'nextcloud') {
			if (Cypress.$('.menu-entry-with-icon').length) {
				cy.cGet('.menu-entry-with-icon')
					.should('not.have.text', menuItems[i]);
			}
		}
	}

	cy.log('<< selectHamburgerMenuItem - end');
}

function selectAnnotationMenuItem(menuItem) {
	cy.log('>> selectAnnotationMenuItem - start');

	cy.cGet('#mobile-wizard .wizard-comment-box .cool-annotation-menu')
		.click({force: true});

	cy.cGet('body').contains('.ui-header.mobile-wizard.ui-widget', menuItem)
		.click();

	cy.log('<< selectAnnotationMenuItem - end');
}

function selectListBoxItem(listboxSelector, item) {
	cy.log('>> selectListBoxItem - start');

	cy.cGet(listboxSelector).click();

	cy.cGet('.mobile-wizard.ui-combobox-text', item).click();

	// Combobox entry contains the selected item
	cy.cGet(listboxSelector + ' .ui-header-right .entry-value')
		.should('have.text', item);

	cy.log('<< selectListBoxItem - end');
}

function selectListBoxItem2(listboxSelector, item) {
	cy.log('>> selectListBoxItem2 - start');

	cy.cGet(listboxSelector).click();

	var endPos = listboxSelector.indexOf(' ');
	if (endPos < 0)
		endPos = listboxSelector.length;
	var parentId = listboxSelector.substring(0, endPos);

	cy.cGet(parentId + ' .ui-combobox-text', item).click();

	cy.cGet(listboxSelector + ' .ui-header-left')
		.should('have.text', item);

	cy.log('<< selectListBoxItem2 - end');
}
function insertComment(skipCommentCheck = false, menuLabel = 'Comment') {
	cy.log('>> insertComment - start');

	openInsertionWizard();
	cy.cGet('body').contains('.menu-entry-with-icon', menuLabel).click();
	cy.cGet('.cool-annotation-table').should('exist');
	cy.cGet('#input-modal-input').type('some text');
	cy.cGet('#response-ok').click();

	// Wait for core to process the comment insertion
	cy.getFrameWindow().then((win) => {
		helper.processToIdle(win);
	});

	if (!skipCommentCheck) {
		cy.cGet('[id^=comment-container-]').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('have.text', 'some text');
	}

	cy.log('<< insertComment - end');
}

function insertImage() {
	cy.log('>> insertImage - start');

	openInsertionWizard();

	// We can't use the menu item directly, because it would open file picker.
	cy.cGet('body').contains('.menu-entry-with-icon', 'Local Image...')
		.should('be.visible');

	cy.cGet('#insertgraphic[type=file]')
		.attachFile('/mobile/writer/image_to_insert.png');

	// The upload + insert round-trip is asynchronous.
	// CPU load the default 10s wait on the SVG might be not enough

	cy.wait(1000);

	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	cy.cGet('#document-container svg g')
		.should('exist');

	cy.log('<< insertImage - end');
}

function deleteImage() {
	cy.log('>> deleteImage - start');

	cy.cGet('#document-container').then(function(items) {
		const boundingRect = items[0].getBoundingClientRect();
		const x = boundingRect.left + boundingRect.width / 2;
		const y = boundingRect.top + boundingRect.height / 2;
		cy.cGet('#document-container').rightclick(x, y);
	});

	cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
		.should('be.visible').click();

	cy.cGet('#document-container svg g')
		.should('not.exist');

	cy.log('<< deleteImage - end');
}

function pressPushButtonOfDialog(name) {
	cy.log('>> pressPushButtonOfDialog - start');

	cy.cGet('body').contains('.ui-pushbutton', name).click();

	cy.log('<< pressPushButtonOfDialog - end');
}

function getCompactIcon(unoCommand) {
	return cy.cGet('#toolbar-down .uno' + unoCommand + ':visible');
}

module.exports.enableEditingMobile = enableEditingMobile;
module.exports.longPressOnDocument = longPressOnDocument;
module.exports.openHamburgerMenu = openHamburgerMenu;
module.exports.selectHamburgerMenuItem = selectHamburgerMenuItem;
module.exports.selectAnnotationMenuItem = selectAnnotationMenuItem;
module.exports.closeHamburgerMenu = closeHamburgerMenu;
module.exports.openMobileWizard = openMobileWizard;
module.exports.closeMobileWizard = closeMobileWizard;
module.exports.openInsertionWizard = openInsertionWizard;
module.exports.closeInsertionWizard = closeInsertionWizard;
module.exports.selectFromColorPalette = selectFromColorPalette;
module.exports.selectFromColorPicker = selectFromColorPicker;
module.exports.openTextPropertiesPanel = openTextPropertiesPanel;
module.exports.selectListBoxItem = selectListBoxItem;
module.exports.selectListBoxItem2 = selectListBoxItem2;
module.exports.openCommentWizard = openCommentWizard;
module.exports.insertImage = insertImage;
module.exports.deleteImage = deleteImage;
module.exports.insertComment = insertComment;
module.exports.pressPushButtonOfDialog = pressPushButtonOfDialog;
module.exports.getCompactIcon = getCompactIcon;
