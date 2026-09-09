/* global describe it cy beforeEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Notebookbar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/insert_position.fodg');
		desktopHelper.switchUIToNotebookbar();
		// Wide enough that the View tab does not push the zoom group into
		// its overflow menu.
		cy.viewport(1920, 1080);
	});

	it('The View tab Full Screen button toggles full screen', function() {
		// The browser only enters full screen on a real user gesture, which
		// Cypress cannot produce, so watch the call the button has to make.
		cy.getFrameWindow().then(function(win) {
			cy.stub(win.app.util, 'toggleFullScreen').as('toggleFullScreen');
		});

		desktopHelper.selectNotebookbarTab('View');
		// The zoom overflow group renders a second, hidden copy of the button.
		cy.cGet('#View-container .unotoolbutton.unoFullScreen')
			.filter(':visible').click();

		cy.get('@toggleFullScreen').should('have.been.calledOnce');
	});

	it('The folded zoom group lists its buttons while the page is full screen', function() {
		// Narrow enough that the View tab folds the zoom group into its
		// overflow menu, the layout a zoomed-in browser produces.
		cy.viewport(800, 1080);

		// Cypress cannot enter full screen, so report the page root as the
		// fullscreen element the way the browser does after View > Full Screen.
		cy.getFrameWindow().then(function(win) {
			Object.defineProperty(win.document, 'fullscreenElement', {
				configurable: true,
				get: function() { return win.document.documentElement; },
			});
		});

		desktopHelper.selectNotebookbarTab('View');
		// The button id carries a uniqueness suffix, so match on its prefix.
		cy.cGet('#View-container [id^="overflow-button-view-zoom"].menubutton')
			.should('be.visible')
			.find('.arrowbackground').click();

		// The menu holds the real Full Screen button moved over from the folded
		// group.
		cy.cGet('[id^="overflow-button-view-zoom-dropdown"] .unotoolbutton.unoFullScreen')
			.should('be.visible');
	});
});
