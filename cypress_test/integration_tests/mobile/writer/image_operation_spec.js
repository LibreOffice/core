/* global describe it beforeEach require cy */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile'],'Image Operation Tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/image_operation.odt');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	it('Insert Image', function() {
		mobileHelper.insertImage();
	});

	it('Delete Image', function() {
		mobileHelper.insertImage();
		mobileHelper.deleteImage();
	});

	it.skip('Crop', function() {
		mobileHelper.insertImage();

		cy.cGet('.mobile-wizard-back.close-button').click();

		helper.assertImageSize(512, 130);
		cy.cGet('#test-div-shape-handle-3').should('exist');
		cy.cGet('#crop').should('be.visible');
		cy.cGet('#crop').click();

		cy.cGet('#test-div-shape-handle-3').then(($handle) => {
			const rect = $handle[0].getBoundingClientRect();
			const startX = rect.left + rect.width / 2;
			const startY = rect.top + rect.height / 2;
			const moveX = 20;

			cy.cGet('body').realSwipe("toRight", { x: startX, y: startY, length: moveX });
		});

		helper.assertImageSize(492, 115);
	});
});
