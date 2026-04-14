/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagproxy'], 'Save progress bar tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/focus.odt');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			return helper.processToIdle(win);
		});
	});

	it('Progress bar appears below document title', function() {
		cy.then(function() {
			this.win.app.map.uiManager.documentNameInput.showProgressBar();
		});

		cy.cGet('#document-name-input-progress-bar').should('be.visible');

		cy.cGet('.document-title').should(function($title) {
			var titleBottom = $title[0].getBoundingClientRect().bottom;
			var barRect = $title[0].querySelector('.progress-bar').getBoundingClientRect();
			expect(barRect.top, 'progress bar top vs title bottom').to.be.at.least(titleBottom - 1);
			expect(barRect.top, 'progress bar not too far below title').to.be.at.most(titleBottom + 5);
			expect(barRect.height, 'progress bar height').to.equal(2);
		});

		cy.then(function() {
			this.win.app.map.uiManager.documentNameInput.hideProgressBar();
		});

		cy.cGet('#document-name-input-progress-bar').should('not.be.visible');
	});

	it('Loading animation appears below document title', function() {
		cy.then(function() {
			this.win.app.map.uiManager.documentNameInput.showLoadingAnimation();
		});

		cy.cGet('#document-name-input-loading-bar').should('be.visible');

		cy.cGet('.document-title').should(function($title) {
			var titleBottom = $title[0].getBoundingClientRect().bottom;
			var barRect = $title[0].querySelector('#document-name-input-loading-bar').getBoundingClientRect();
			expect(barRect.top, 'loading bar top vs title bottom').to.be.at.least(titleBottom - 1);
			expect(barRect.top, 'loading bar not too far below title').to.be.at.most(titleBottom + 5);
			expect(barRect.height, 'loading bar height').to.equal(2);
		});

		cy.then(function() {
			this.win.app.map.uiManager.documentNameInput.hideLoadingAnimation();
		});

		cy.cGet('#document-name-input-loading-bar').should('not.be.visible');
	});
});
