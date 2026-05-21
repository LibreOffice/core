/* global cy require expect Cypress */

var helper = require('./helper');

// Assert that Impress is *not* in Text Edit Mode.
function assertNotInTextEditMode() {
	cy.log('>> assertNotInTextEditMode - start');

	// In edit mode, we should have the blinking cursor.
	cy.cGet('.leaflet-cursor.blinking-cursor').should('not.exist');
	cy.cGet('.leaflet-cursor-container').should('not.exist');
	helper.assertNoKeyboardInput();

	cy.log('<< assertNotInTextEditMode - end');
}

// Assert that Impress is in Text Edit Mode.
function assertInTextEditMode() {
	cy.log('>> assertInTextEditMode - start');

	// In edit mode, we should have the edit container.
	cy.cGet('#doc-clipboard-container').should('exist');
	cy.cGet('#document-container svg g').should('exist');
	helper.assertCursorAndFocus();

	cy.log('<< assertInTextEditMode - end');
}

// Enter some text and confirm we get it back.
function typeTextAndVerify(text, expected, copy) {
	cy.log('>> typeTextAndVerify - start');

	if (!expected)
		expected = text;

	assertInTextEditMode();

	// Type some text.
	helper.typeIntoDocument(text);

	// Still in edit mode.
	assertInTextEditMode();

	helper.selectAllText();
	if (copy === true) {
		helper.copy();
	}

	helper.expectTextForClipboard(expected);

	cy.log('<< typeTextAndVerify - end');
}

// Make sure we have the right number of slides in the document.
// We use the number of slide previews as indicators.
// Parameters:
// slides - number of expected slides
function assertSlidePreviewCountAfterIdle(win, slides) {
	cy.log('>> assertSlidePreviewCountAfterIdle - start');

	helper.processToIdle(win);
	// +1 to account for #first-drop-site which also has the .preview-frame class
	cy.cGet('#slide-sorter .preview-frame')
		.should('have.length', slides + 1);

	cy.log('<< assertSlidePreviewCountAfterIdle - end');
}

// Trigger mouse click on center of the screen
// Does no checking as to what is or isn't found there
function clickCenterOfSlide(modifiers) {
	cy.cGet('#document-container')
		.then(function(items) {
			expect(items).to.have.length(1);
			var XPos = (items[0].getBoundingClientRect().left + items[0].getBoundingClientRect().right) / 2;
			var YPos = (items[0].getBoundingClientRect().top + items[0].getBoundingClientRect().bottom) / 2;
			cy.cGet('body').click(XPos, YPos, modifiers);
		});
}

// Select a text shape at the center of the slide / view.
// This method triggers mouse click in the center to achive
// a shape selection. It fails, if there is no shape there.
function selectTextShapeInTheCenter() {
	cy.log('>> selectTextShapeInTheCenter - start');

	// Click on the center of the slide to select the text shape there
	clickCenterOfSlide( { } );

	cy.cGet('#test-div-shape-handle-rotation').should('exist');
	cy.cGet('#document-container svg g.Page g').should('exist');

	cy.log('<< selectTextShapeInTheCenter - end');
}

function selectTableInTheCenter(win) {
	cy.log('>> selectTableInTheCenter - start');

	// First click selects the table as a shape.
	clickCenterOfSlide();
	helper.processToIdle(win);

	// Second click enters the table and places the cursor in a cell.
	clickCenterOfSlide();
	helper.processToIdle(win);

	cy.cGet('.leaflet-cursor-container').should('be.visible');
	cy.cGet('.table-row-resize-marker').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).should('be.visible');
	cy.cGet('#document-container svg g.Page g').should('exist');

	cy.log('<< selectTableInTheCenter - end');
}

// Remove existing shape selection by clicking outside of the shape.
function removeShapeSelection() {
	cy.log('>> removeShapeSelection - start');

	// Remove selection with clicking on the top-left corner of the slide
	cy.cGet('.leaflet-canvas-container canvas')
		.then(function(items) {
			var XPos = items[0].getBoundingClientRect().left + 10;
			var YPos = items[0].getBoundingClientRect().top + 10;
			cy.cGet('body').click(XPos, YPos);
		});

	// The click triggers MouseControl.onClick which calls focus(false),
	// blurring the textarea on mobile. Re-focus depends on INCOMING
	// invalidatecursor from core. processToIdle ensures that message
	// has been received before we send the Esc keys.
	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	helper.typeIntoDocument('{esc}');

	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	helper.typeIntoDocument('{esc}');

	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	cy.cGet('#document-container')
		.should(function(overlay) {
			expect(overlay.children('svg').length).to.equal(0);
		});

	cy.cGet('#test-div-shapeHandlesSection').should('not.exist');

	cy.log('<< removeShapeSelection - end');
}


// We use an SVG representation of the Impress shapes
// to check it's content, it's shape, etc. We can use
// this method to trigger the update of the SVG representation
// so we can be sure that it's in an updated state.
function triggerNewSVGForShapeInTheCenter() {
	cy.log('>> triggerNewSVGForShapeInTheCenter - start');

	removeShapeSelection();

	// If we click too fast on the shape again
	// then it steps into edit mode, might be a bug
	cy.wait(200);

	// Select text shape again which will retrigger a new SVG from core
	selectTextShapeInTheCenter();

	cy.log('<< triggerNewSVGForShapeInTheCenter - end');
}

// Select the text inside a preselected shape. So we assume
// we have already a shape selected. We try to select the
// text of this shape by double clicking into it, until the
// cursor becomes visible and the text is selected
function selectTextOfShape() {
	cy.log('>> selectTextOfShape - start');

	dblclickOnSelectedShape();
	helper.typeIntoDocument('{ctrl}a');
	cy.cGet('.text-selection-handle-start, .text-selection-handle-end')
		.should('exist');
	cy.cGet('.leaflet-cursor-container, .text-selection-handle-start')
		.should('exist');

	cy.log('<< selectTextOfShape - end');
}

// Step into text editing of the preselected shape. So we assume
// we have already a shape selected. We try to make the document
// to switch text editing mode by double clicking into the shape.
function dblclickOnSelectedShape() {
	cy.log('>> dblclickOnSelectedShape - start');

	helper.getShapeSVGCenter().then(function(pos) {
		cy.cGet('#document-canvas').dblclick(pos.x, pos.y, { force: true });
	});

	// check if any of text input markers exist
	cy.cGet('.leaflet-cursor-container, .text-selection-handle-start, .leaflet-cursor.blinking-cursor')
		.should('exist');

	cy.log('<< dblclickOnSelectedShape - end');
}

//add multiple slides
function addSlide(numberOfSlides) {
	cy.log('>> addSlide - start');

	cy.cGet('.preview-frame').then(function (result) {
		var origSlides = result.length;
		for (let i = 0; i < numberOfSlides; i++) {
			cy.cGet('#presentation-toolbar #insertpage')
				.should('not.have.class', 'disabled')
				.click();
		}
		cy.cGet('.preview-frame')
			.should('have.length',origSlides+numberOfSlides);
	});

	cy.log('<< addSlide - end');
}

//change multiple slides
function changeSlide(changeNum,direction) {
	cy.log('>> changeSlide - start');

	var slideButton;
	if (direction === 'next') {
		slideButton = cy.cGet('#nextpage-button');
	} else if (direction === 'previous') {
		slideButton = cy.cGet('#prevpage-button');
	}
	if (slideButton) {
		for (var n = 0; n < changeNum; n++) {
			slideButton.click();
		}
	}

	cy.log('<< changeSlide - end');
}

function getSlideShow() {
	return cy.cGet('#slideshow-cypress-iframe');
}

function getSlideShowContent() {
	return getSlideShow().its('0.contentDocument');
}

function getSlideShowCanvas() {
	return getSlideShowContent().find('#slideshow-canvas');
}

// Wait for the slideshow to have loaded a slide and for any
// animations/transitions to finish. Waits for the navigator's
// currentSlideIndex to be set (slide loaded via fetchAndRun)
// then waits for no active 'slideshowupdate' timers.
function waitForSlideShowIdle(win) {
	cy.waitUntil(() => {
		var presenter = win.app && win.app.map && win.app.map.slideShowPresenter;
		if (!presenter || !presenter._slideShowNavigator)
			return false;
		return presenter._slideShowNavigator.currentSlideIndex !== undefined;
	}, { interval: 50 });
	helper.waitForTimers(win, 'slideshowupdate');
}

// Click a slideshow-nav-container button. The nav container auto-hides after
// 3s of inactivity via _showSlideControls' setTimeout. Under heavy CI load the
// gap between cypress steps can be more than that and cypress's own retry
// doesn't know about the auto-hide timer.
// So re-call _showSlideControls each iteration and click as soon as the button
// is visible.
function clickSlideShowNav(win, buttonSelector) {
	var presenter = win.app && win.app.map && win.app.map.slideShowPresenter;
	cy.waitUntil(function () {
		if (presenter && presenter._showSlideControls)
			presenter._showSlideControls();
		return getSlideShowContent()
			.find('.slideshow-nav-container ' + buttonSelector)
			.then(function ($el) {
				if (!Cypress.dom.isVisible($el)) return false;
				return cy.wrap($el).click().then(function () { return true; });
			});
	}, { interval: 500,
		errorMsg: 'slideshow nav button "' + buttonSelector + '" never reachable' });
}

module.exports.assertNotInTextEditMode = assertNotInTextEditMode;
module.exports.assertInTextEditMode = assertInTextEditMode;
module.exports.typeTextAndVerify = typeTextAndVerify;
module.exports.assertSlidePreviewCountAfterIdle = assertSlidePreviewCountAfterIdle;
module.exports.clickCenterOfSlide = clickCenterOfSlide;
module.exports.selectTextShapeInTheCenter = selectTextShapeInTheCenter;
module.exports.triggerNewSVGForShapeInTheCenter = triggerNewSVGForShapeInTheCenter;
module.exports.removeShapeSelection = removeShapeSelection;
module.exports.selectTextOfShape = selectTextOfShape;
module.exports.dblclickOnSelectedShape = dblclickOnSelectedShape;
module.exports.addSlide = addSlide;
module.exports.changeSlide = changeSlide;
module.exports.selectTableInTheCenter = selectTableInTheCenter;
module.exports.getSlideShow = getSlideShow;
module.exports.getSlideShowContent = getSlideShowContent;
module.exports.getSlideShowCanvas = getSlideShowCanvas;
module.exports.waitForSlideShowIdle = waitForSlideShowIdle;
module.exports.clickSlideShowNav = clickSlideShowNav;
