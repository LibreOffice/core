/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Image Operation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/image_operation.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
	});

	it('Insert/Delete Image',function() {
		desktopHelper.insertImage('calc');

		//make sure that image is in focus
		cy.cGet('#document-container svg g').should('exist');

		desktopHelper.deleteImage();
	});

	it('Crop Image', function() {
		const moveY = 50;

		desktopHelper.insertImage('calc');

		helper.assertImageSize(248, 63);

		// use bottom handle
		cy.cGet('#test-div-shape-handle-6').should('exist');
		cy.cGet('.unoCrop').should('be.visible').click();

		cy.cGet('#test-div-shape-handle-6').then(($handle) => {
			const rect = $handle[0].getBoundingClientRect();
			const startX = rect.left + rect.width / 2;
			const startY = rect.top + rect.height / 2;

			cy.cGet('body').realMouseDown({ x: startX, y: startY });

			cy.cGet('body').realMouseMove(startX, startY - moveY);

			cy.cGet('body').realMouseUp({ x: startX, y: startY - moveY });
			cy.cGet('body').realMouseUp();
		});

		cy.wait(1000);
		helper.assertImageSize(248, 63 - moveY);
	});

	it.skip('Resize image when keep ratio option enabled and disabled', function() {
		desktopHelper.insertImage('calc');
		//when Keep ratio is unchecked
		helper.assertImageSize(248, 63);

		cy.cGet().contains('.ui-expander-label', 'Position and Size')
			.click();

		cy.cGet('#selectwidth input').type('{selectAll}{backspace}3{enter}');

		cy.cGet('#selectheight input').type('{selectAll}{backspace}2{enter}');

		helper.assertImageSize(139, 93);

		//Keep ratio checked
		cy.cGet('#ratio input').check();

		cy.cGet('#selectheight input').type('{selectAll}{backspace}5{enter}');

		helper.assertImageSize(347, 232);
	});
});
