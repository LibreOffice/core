/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Scroll through document', function() {

	beforeEach(function() {
		cy.viewport(1500, 600);
		helper.setupAndLoadDocument('impress/scrolling.odp');
		desktopHelper.switchUIToCompact();
		// close the default slide-sorter navigation sidebar
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.getCompactIcon('ModifyPage').click();
		desktopHelper.selectZoomLevel('200', false);
	});

	function clickOnTheCenter() {
		cy.cGet('#document-container')
			.then(function(items) {
				expect(items).to.have.length(1);
				var XPos = (items[0].getBoundingClientRect().left + items[0].getBoundingClientRect().right) / 2;
				var YPos = (items[0].getBoundingClientRect().top + items[0].getBoundingClientRect().bottom) / 2;
				cy.cGet('body')
					.dblclick(XPos, YPos)
					.wait(500);
			});
	}
	it('Scrolling to bottom/top', function() {
		//show vertical scrollbar
		cy.cGet('#document-container').click('right');
		cy.getFrameWindow().then(function (win) {
			return helper.processToIdle(win);
		});
		clickOnTheCenter();
		desktopHelper.pressKey(15,'uparrow');
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
		desktopHelper.pressKey(18,'downarrow');
		desktopHelper.assertScrollbarPosition('vertical', 280, 330);
	});

	it('Scrolling to left/right', function() {
		//show horizontal scrollbar
		cy.cGet('#document-container').click('bottom');
		cy.getFrameWindow().then(function (win) {
			return helper.processToIdle(win);
		});
		clickOnTheCenter();
		cy.getFrameWindow().then(function (win) {
			return helper.processToIdle(win);
		});
		helper.typeIntoDocument('{home}');
		desktopHelper.assertScrollbarPosition('horizontal', 0, 1);
		helper.typeIntoDocument('{end}');
		desktopHelper.assertScrollbarPosition('horizontal', 340, 660);
	});
});
