/* global describe it require cy beforeEach */

var helper = require('../../common/helper');
var { insertImage, deleteImage } = require('../../common/desktop_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Image Operation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/image_operation.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);

	});

	it('Insert Image',function() {
		insertImage();

		//make sure that image is in focus
		cy.cGet('#document-container svg g').should('exist');

		deleteImage();
	});

	it('Crop', function () {
		insertImage();
		helper.assertImageSize(248, 63);
		cy.cGet('#test-div-shape-handle-3').should('exist');
		desktopHelper.getNbIcon('Crop', 'Picture').click();

		const moveX = 20;

		cy.cGet('#test-div-shape-handle-3').then(($handle) => {
			const rect = $handle[0].getBoundingClientRect();
			const startX = rect.left + rect.width / 2;
			const startY = rect.top + rect.height / 2;

			cy.cGet('body').realMouseDown({ x: startX, y: startY });

			cy.cGet('body').realMouseMove(startX + moveX, startY);

			// for some reason even after moving the crop marker 20 px, on realMouseUp crop marker moves a lot more than expected
			// but it seems to related to how realMouseUp is implemented
			cy.cGet('body').realMouseUp({ x: startX + moveX, y: startY });
			cy.cGet('body').realMouseUp();
		});

		cy.wait(1000);
		helper.assertImageSize(248 - moveX, 63);
	});

	it('Resize image when keep ratio option enabled and disabled', function() {
		insertImage();
		//when Keep ratio is unchecked
		helper.assertImageSize(248, 63);
		// if window is too small sidebar won't popup

		cy.cGet('#selectwidth input').type('{selectAll}{backspace}3{enter}');

		cy.cGet('#selectheight input').type('{selectAll}{backspace}2{enter}');

		cy.wait(1000);

		helper.assertImageSize(139, 93);

		//Keep ratio checked
		cy.cGet('#ratio input').check();

		cy.cGet('#selectheight input').type('{selectAll}{backspace}5{enter}');

		cy.wait(1000);

		helper.assertImageSize(347, 232);
	});
});
