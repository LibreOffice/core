/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach*/

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

// The visible slide number is rendered by a CSS counter (see
// partsPreviewControl.css): counter-reset on the container, counter-increment
// on each .preview-slide-number, content: counter(...) painting the digit.
// getComputedStyle never resolves counter() to the digit it paints - the
// resolved value depends on the element's position among its layout
// siblings, which is a rendering-time concern the computed style spec
// deliberately excludes - so this checks the counter wiring itself rather
// than trying to read a rendered number.
function assertSlideNumberCounterWiring(items) {
	for (var i = 0; i < items.length; i++) {
		var view = items[i].ownerDocument.defaultView;
		expect(view.getComputedStyle(items[i]).counterIncrement).to.contain('slide-number');
		expect(view.getComputedStyle(items[i], '::before').content).to.equal('counter(slide-number)');
	}
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide operations', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('impress/slide_operations.odp', {
		notebookbar: true,
	});

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// A frame id carries the part number the slide was created with, and that number
	// keeps climbing for as long as the document stays open. What the tests below mean
	// is where a slide sits in the sorter, so they ask for it by position.
	function slideFrame(position) {
		return cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site)').eq(position);
	}

	it('Add slides', function() {
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Remove slides', function() {
		// Add slides
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		// Remove Slides
		cy.cGet('#presentation-toolbar #deletepage')
			.should('not.have.attr', 'disabled');

		cy.cGet('#presentation-toolbar #deletepage')
			.click();

		cy.cGet('#modal-dialog-deleteslide-modal .button-primary').click();

		cy.cGet('#presentation-toolbar #deletepage')
			.should('have.attr', 'disabled')

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

	});

	it('Slide number counter stays wired one-to-one with slides after insert and delete', function() {
		// Add two slides.
		cy.cGet('#presentation-toolbar #insertpage').click();
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 3);

		cy.cGet('#slide-sorter').should(function(container) {
			var view = container[0].ownerDocument.defaultView;
			expect(view.getComputedStyle(container[0]).counterReset).to.contain('slide-number');
		});

		// One number span per slide - not more, not fewer - each still
		// wired to the shared counter.
		cy.cGet('#slide-sorter .preview-slide-number').should(function(items) {
			expect(items).to.have.length(3);
			assertSlideNumberCounterWiring(items);
		});

		// Remember the slide sitting second, which the delete below moves to the front.
		var secondFrameId;
		cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site)').then(function(frames) {
			secondFrameId = frames[1].id;
		});

		// Delete the first slide: its number span must go with its frame,
		// not linger and throw off every later slide's count.
		slideFrame(0).click();

		cy.cGet('#presentation-toolbar #deletepage').click();
		cy.cGet('#modal-dialog-deleteslide-modal .button-primary').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		cy.cGet('#slide-sorter .preview-slide-number').should(function(items) {
			expect(items).to.have.length(2);
			assertSlideNumberCounterWiring(items);
		});

		// The slide that used to be second is now first in DOM order, so
		// the counter will paint it as slide 1.
		cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site)').should(function(frames) {
			expect(frames[0].id).to.equal(secondFrameId);
		});
	});

	it('Slide alt text and tooltip track position after insert and delete', function() {
		function assertPositionLabels(items) {
			for (var i = 0; i < items.length; i++) {
				expect(items[i].getAttribute('alt')).to.equal('preview of page ' + (i + 1));
				expect(items[i].getAttribute('data-cooltip')).to.equal('Slide ' + (i + 1));
			}
		}

		// Add two slides.
		cy.cGet('#presentation-toolbar #insertpage').click();
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 3);

		cy.cGet('#slide-sorter .preview-img').should(function(items) {
			expect(items).to.have.length(3);
			assertPositionLabels(items);
		});

		// Delete the first slide: every later slide moves up one position, so
		// its alt text and tooltip must be relabelled to match, unlike the
		// visible number, they are plain attributes and do not update on
		// their own just because the frame moved in the DOM.
		slideFrame(0).click();

		cy.cGet('#presentation-toolbar #deletepage').click();
		cy.cGet('#modal-dialog-deleteslide-modal .button-primary').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		cy.cGet('#slide-sorter .preview-img').should(function(items) {
			expect(items).to.have.length(2);
			assertPositionLabels(items);
		});

		// Insert a slide after the first one: the new last slide must pick
		// up label 2 even though it was created as label 1 in a different
		// position earlier in the test.
		slideFrame(1).click();
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 3);

		cy.cGet('#slide-sorter .preview-img').should(function(items) {
			expect(items).to.have.length(3);
			assertPositionLabels(items);
		});
	});

	it('Check slide sorter focus', function() {
		cy.cGet('#insertpage-button').click();
		helper.processToIdle(this.win);

		// Set the focus to slide sorter.
		slideFrame(0).click();
		slideFrame(1).click();

		// Slide sorter should keep focus while user clicks on different slides.
		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		cy.cGet('#toolbar-up').click();
		// Slide sorter should have lost the focus after user clicked somewhere.
		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(false);
		});
	});

	it('Slide sorter keeps focus when Alt or Meta is pressed', function() {
		cy.cGet('#insertpage-button').click();
		helper.processToIdle(this.win);

		slideFrame(0).click();

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		slideFrame(0).trigger('keydown', { key: 'Alt', code: 'AltLeft', which: 18 });

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		slideFrame(0).trigger('keydown', { key: 'Meta', code: 'MetaLeft', which: 91 });

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});
	});

	it('Undo reaches core after inserting on a non-last slide', function() {
		var win = this.win;

		// Add a second slide so the first slide is no longer the last one.
		cy.cGet('#insertpage-button').click();
		impressHelper.assertSlidePreviewCountAfterIdle(win, 2);

		// Select the first (non-last) slide in the sorter.
		slideFrame(0).click();
		cy.then(() => {
			expect(win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		// Insert after the first slide; focus moves to the new slide's preview.
		cy.cGet('#insertpage-button').click();
		helper.processToIdle(win);

		cy.then(() => {
			// The slide sorter must still be considered focused.
			expect(win.app.map._docLayer._preview.partsFocused).to.equal(true);
			cy.spy(win.app.socket, 'sendMessage').as('sendMessage');
		});

		cy.cGet('#slide-sorter').trigger('keydown', {
			ctrlKey: true, key: 'z', code: 'KeyZ', keyCode: 90, which: 90, bubbles: true,
		});

		cy.get('@sendMessage').should('have.been.calledWith', 'uno .uno:Undo');
	});

	it('Duplicate slide', function() {
		// Also check if comments are getting duplicated
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.getNbIcon('ModifyPage').click();
		desktopHelper.insertComment();
		cy.cGet('[id^=annotation-content-area-]').should('include.text', 'some text0');
		desktopHelper.getNbIcon('DuplicatePage', 'Insert').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
		cy.cGet('#SlideStatus').should('have.text', 'Slide 2 of 2');
		cy.cGet('[id^=annotation-content-area-]').should('include.text', 'some text0');

	});

	it('Navigator height test', function() {
		var navigationContainer, navOptionContainer, presentationWrapper, navHeading;

		cy.cGet('.navigation-header')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navHeading = items[0].getBoundingClientRect();
			});

		cy.cGet('.navigation-options-container')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navOptionContainer = items[0].getBoundingClientRect();
			});


		cy.cGet('#presentation-controls-wrapper')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				presentationWrapper = items[0].getBoundingClientRect();
			});

		cy.cGet('#navigation-sidebar')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navigationContainer = items[0].getBoundingClientRect();
				expect(navigationContainer.height).equal(navHeading.height + navOptionContainer.height + presentationWrapper.height);
			});
	});
});
