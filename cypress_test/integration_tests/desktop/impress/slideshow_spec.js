/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

function getSlideShowContent() {
	return cy.cGet('#slideshow-cypress-iframe');
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Some app', function() {
	beforeEach(function() {
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
});
