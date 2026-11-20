/* global describe it cy require beforeEach*/

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Slide operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_operations.odp', false, false, undefined, 'vector=false');

		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Add slides', function() {
		cy.cGet('.leaflet-control-zoom-in').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Remove Slides', function() {
		//add slides
		cy.cGet('.leaflet-control-zoom-in').click();
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
		//remove slides
		mobileHelper.openHamburgerMenu();
		cy.cGet('.menu-entry-icon.slidemenu').parent().click();
		cy.cGet('.menu-entry-icon.deletepage').parent().click();
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);
	});

	it('Duplicate Slide', function() {
		mobileHelper.openHamburgerMenu();

		cy.cGet('.menu-entry-icon.slidemenu').parent().click();
		cy.cGet('.menu-entry-icon.duplicatepage').parent().click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

});
