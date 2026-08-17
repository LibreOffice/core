/* global describe it cy beforeEach require expect */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Insert position', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/insert_position.fodg');
		desktopHelper.switchUIToNotebookbar();
		// A viewport shorter than the A4 page, so the page does not fit and an
		// object placed by the page geometry alone lands outside the view.
		cy.viewport(1920, 800);
	});

	function assertSelectionIsInView() {
		cy.getFrameWindow().its('app.definitions.graphicSelection.rectangle')
			.should('not.be.null');

		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win).then(function() {
				const object = win.app.definitions.graphicSelection.rectangle;
				const visible = win.app.activeDocument.activeLayout.viewedRectangle;

				expect(object.x1 + object.width / 2,
					'inserted object centre x').to.be.within(visible.x1, visible.x2);
				expect(object.y1 + object.height / 2,
					'inserted object centre y').to.be.within(visible.y1, visible.y2);
			});
		});
	}

	it('An inserted image lands where the user is looking', function() {
		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert-container .unoInsertGraphic').filter(':visible').click();
		cy.cGet('#insertgraphic[type=file]')
			.attachFile('/desktop/writer/image_to_insert.png');

		assertSelectionIsInView();
	});
});
