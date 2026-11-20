/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Slide sorter orientation', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_operations.odp', false, false, undefined, 'vector=false');
		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Slide previews stay available when the device rotates', function() {
		// Portrait: the slide strip sits below the document.
		cy.cGet('#presentation-controls-wrapper').should('be.visible').should('have.class', 'portrait');
		cy.cGet('body > #presentation-controls-wrapper').should('exist');

		// A second slide, so there is something to switch to. The new slide becomes current.
		cy.cGet('.leaflet-control-zoom-in').click();
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
		cy.cGet('#slide-sorter .preview-img').eq(1).should('have.class', 'preview-img-currentpart');

		// Landscape: the slide list moves next to the document and is still usable.
		cy.viewport('iphone-6', 'landscape');
		cy.cGet('#presentation-controls-wrapper').should('be.visible').should('have.class', 'landscape');
		cy.cGet('#main-document-content > #presentation-controls-wrapper').should('exist');
		cy.cGet('#slide-sorter .preview-img').should('have.length', 2).each(($img) => {
			cy.wrap($img).should('be.visible');
		});

		// Tapping a preview in landscape changes the current slide.
		cy.cGet('#slide-sorter .preview-img').eq(0).click();
		cy.cGet('#slide-sorter .preview-img').eq(0).should('have.class', 'preview-img-currentpart');
		cy.cGet('#slide-sorter .preview-img').eq(1).should('not.have.class', 'preview-img-currentpart');

		// Back to portrait: the strip returns below the document.
		cy.viewport('iphone-6', 'portrait');
		cy.cGet('#presentation-controls-wrapper').should('be.visible').should('have.class', 'portrait');
		cy.cGet('body > #presentation-controls-wrapper').should('exist');
		cy.cGet('#slide-sorter .preview-img').should('have.length', 2).each(($img) => {
			cy.wrap($img).should('be.visible');
		});
	});
});
