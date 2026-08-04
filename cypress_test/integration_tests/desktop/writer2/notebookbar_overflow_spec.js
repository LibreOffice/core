/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// WCAG 2.1 SC 1.4.10 (Reflow) at 400% browser zoom, which on a 1920px screen
// leaves a 480px viewport. The notebookbar holds more than it can show at that
// width, and what does not fit still has to be reachable. eba379710fd9 put that
// on the <nav>: it scrolls horizontally, and at 480px and below its scrollbar
// stops being hidden so there is something to say more is there and something
// to drag.
describe(['tagdesktop'], 'Notebookbar at 400% zoom', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(480, 660);
		cy.getFrameWindow().then(function(win) {
			helper.waitUntilLayoutingIsIdle(win);
		});
	});

	it('what does not fit in the nav can be reached by scrolling', function() {
		cy.cGet('nav.main-nav.hasnotebookbar').then(function($nav) {
			const nav = $nav[0];
			const style = getComputedStyle(nav);
			const bar = nav.offsetHeight - nav.clientHeight -
			          parseFloat(style.borderTopWidth) -
			          parseFloat(style.borderBottomWidth);

			expect(nav.scrollWidth, 'nav holds more than it can show, clientWidth ' +
			       nav.clientWidth).to.be.greaterThan(nav.clientWidth);

			expect(style.overflowX, 'nav overflow-x').to.be.oneOf(['scroll', 'auto']);

			expect(style.scrollbarWidth, 'nav scrollbar-width at this viewport')
				.to.not.equal('none');

			expect(bar, 'nav scrollbar height, offsetHeight ' + nav.offsetHeight +
			       ' against clientHeight ' + nav.clientHeight).to.be.greaterThan(0);

			nav.scrollLeft = nav.scrollWidth;
			expect(nav.scrollLeft, 'nav scrollLeft after scrolling to the end')
				.to.be.greaterThan(0);
		});
	});
});
