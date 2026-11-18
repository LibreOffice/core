/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

function getSlideShowContent() {
	return cy.cGet('#slideshow-cypress-iframe');
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Some app', function() {
	beforeEach(function() {
		helper.withoutBrandTheme();
		helper.setupAndLoadDocument('impress/slideshow.odp');
		cy.cGet('.notebookbar #Slideshow-tab-label').click();
		cy.cGet('.notebookbar #slide-fullscreen-presentation-button').click();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Should see an empty slideshow', function () {
		getSlideShowContent().should('be.visible');
		// Wait for the slideshow navigator to set currentSlideIndex and
		// for slideshowupdate timers to drain, so compareSnapshot does
		// not race the canvas's initial render.
		impressHelper.waitForSlideShowIdle(this.win);
		getSlideShowContent().compareSnapshot('slideshow', 0.15);
	});

	it('B and W blank the screen until the next key or click', function () {
		getSlideShowContent().should('be.visible');
		impressHelper.waitForSlideShowIdle(this.win);
		cy.wrap(this.win).its('app.map.slideShowPresenter._slideShowNavigator.currentSlideIndex')
			.should('equal', 0);

		// B covers the slide with a black screen.
		impressHelper.sendSlideShowKey(this.win, 'KeyB');
		impressHelper.getSlideShowContent().find('.slideshow-blank-screen')
			.should('have.css', 'background-color', 'rgb(0, 0, 0)');

		// Space only brings the slide back, it does not also advance.
		impressHelper.sendSlideShowKey(this.win, 'Space');
		impressHelper.getSlideShowContent().find('.slideshow-blank-screen').should('not.exist');
		impressHelper.waitForSlideShowIdle(this.win);
		cy.wrap(this.win).its('app.map.slideShowPresenter._slideShowNavigator.currentSlideIndex')
			.should('equal', 0);

		// W covers the slide with a white screen, a click brings the slide back.
		impressHelper.sendSlideShowKey(this.win, 'KeyW');
		impressHelper.getSlideShowContent().find('.slideshow-blank-screen')
			.should('have.css', 'background-color', 'rgb(255, 255, 255)')
			.click();
		impressHelper.getSlideShowContent().find('.slideshow-blank-screen').should('not.exist');
		impressHelper.waitForSlideShowIdle(this.win);
		cy.wrap(this.win).its('app.map.slideShowPresenter._slideShowNavigator.currentSlideIndex')
			.should('equal', 0);
	});
});
