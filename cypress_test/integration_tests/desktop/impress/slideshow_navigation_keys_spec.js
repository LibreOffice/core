/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// Both slides of the document have one on-click effect that plays for 10 seconds.
describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slideshow navigation keys', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slideshow-effects.fodp');
		cy.cGet('.notebookbar #Slideshow-tab-label').click();
		cy.cGet('.notebookbar #slide-fullscreen-presentation-button').click();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function currentSlide(win) {
		return cy.wrap(win).its('app.map.slideShowPresenter._slideShowNavigator.currentSlideIndex');
	}

	function effectPlaying(win) {
		return cy.wrap(win).its('app.map.slideShowPresenter._slideShowHandler')
			.invoke('isAnyEffectPlaying');
	}

	it('Alt+PageDown goes to the next slide while an effect plays', function() {
		impressHelper.waitForSlideShowIdle(this.win);
		currentSlide(this.win).should('equal', 0);

		impressHelper.sendSlideShowKey(this.win, 'Space');
		effectPlaying(this.win).should('equal', true);

		impressHelper.sendSlideShowKey(this.win, 'PageDown', { altKey: true });
		currentSlide(this.win).should('equal', 1);
		effectPlaying(this.win).should('equal', false);
	});

	it('PageDown while an effect plays ends the effect and stays on the slide', function() {
		impressHelper.waitForSlideShowIdle(this.win);

		impressHelper.sendSlideShowKey(this.win, 'Space');
		effectPlaying(this.win).should('equal', true);

		impressHelper.sendSlideShowKey(this.win, 'PageDown');
		effectPlaying(this.win).should('equal', false);
		currentSlide(this.win).should('equal', 0);
	});

	it('Alt+PageUp goes to the previous slide before its first effect', function() {
		impressHelper.waitForSlideShowIdle(this.win);

		impressHelper.sendSlideShowKey(this.win, 'Space');
		impressHelper.sendSlideShowKey(this.win, 'PageDown', { altKey: true });
		currentSlide(this.win).should('equal', 1);

		impressHelper.sendSlideShowKey(this.win, 'PageUp', { altKey: true });
		currentSlide(this.win).should('equal', 0);
		cy.wrap(this.win).its('app.map.slideShowPresenter._slideShowHandler')
			.invoke('getCurrentEffect').should('equal', 0);
	});
});
