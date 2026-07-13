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

	it('what does not fit in the tab strip can be reached by scrolling', function() {
		cy.cGet('.ui-tabs.notebookbar').then(function($tabStrip) {
			const tabStrip = $tabStrip[0];
			const style = getComputedStyle(tabStrip);
			const bar = tabStrip.offsetHeight - tabStrip.clientHeight -
			          parseFloat(style.borderTopWidth) -
			          parseFloat(style.borderBottomWidth);

			expect(tabStrip.scrollWidth, 'tab strip holds more than it can show, clientWidth ' +
			       tabStrip.clientWidth).to.be.greaterThan(tabStrip.clientWidth);

			expect(style.overflowX, 'tab strip overflow-x').to.be.oneOf(['scroll', 'auto']);

			expect(style.scrollbarWidth, 'tab strip scrollbar-width at this viewport')
				.to.not.equal('none');

			expect(bar, 'tab strip scrollbar height, offsetHeight ' + tabStrip.offsetHeight +
			       ' against clientHeight ' + tabStrip.clientHeight).to.be.greaterThan(0);

			tabStrip.scrollLeft = tabStrip.scrollWidth;
			expect(tabStrip.scrollLeft, 'tab strip scrollLeft after scrolling to the end')
				.to.be.greaterThan(0);
		});
	});
});
