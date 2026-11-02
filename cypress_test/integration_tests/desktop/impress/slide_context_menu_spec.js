/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide pane context menu', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slideshow.odp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function slideImage(position) {
		return cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site) .preview-img')
			.eq(position);
	}

	function openSlideContextMenu(position) {
		slideImage(position).rightclick();
		cy.cGet('#slide-img-menu-dropdown').should('be.visible');
	}

	function clickContextMenuItem(text) {
		cy.cGet('#slide-img-menu-dropdown').contains('.ui-combobox-entry', text).click();
	}

	it('Layout submenu changes the layout of the slide', function() {
		openSlideContextMenu(0);
		clickContextMenuItem('Layout');

		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid').should('be.visible');
		// Layout 18 is "Title, 4 Content".
		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid [class*="WhatLayout:long=18"] button')
			.click();
		cy.cGet('#slide-img-menu-dropdown').should('not.exist');
		helper.processToIdle(this.win);

		cy.window().should(function() {
			var state = this.win.app.map['stateChangeHandler'].getItemValue('.uno:AssignLayout');
			expect(String(state)).to.equal('18');
		}.bind(this));
	});

	it('Start Slideshow from This Slide presents from the clicked slide', function() {
		openSlideContextMenu(0);
		clickContextMenuItem('New Slide');
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		// Select both slides with the first one current, so the clicked slide is a
		// selected slide that is not the current one.
		slideImage(0).click();
		slideImage(1).click({ ctrlKey: true });
		helper.processToIdle(this.win);
		cy.window().should(function() {
			expect(this.win.app.map.getCurrentPartNumber()).to.equal(0);
			expect(this.win.app.impress.isSlideSelected(1)).to.equal(true);
		}.bind(this));

		openSlideContextMenu(1);
		clickContextMenuItem('Start Slideshow from This Slide');

		cy.cGet('#slideshow-cypress-iframe').should('be.visible');
		impressHelper.waitForSlideShowIdle(this.win);
		cy.window().should(function() {
			var navigator = this.win.app.map.slideShowPresenter._slideShowNavigator;
			expect(navigator.currentSlideIndex).to.equal(1);
		}.bind(this));
	});
});
