/* global describe cy it beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Image Operation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/image_operation.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	it('Insert Image', function() {
		mobileHelper.insertImage();
	});

	it('Delete Image', function() {
		mobileHelper.insertImage();

		cy.cGet('#document-container').then(function(items) {
			const rect = items[0].getBoundingClientRect();
			const centerX = rect.left + rect.width / 2;
			const centerY = rect.top + rect.height / 2;

			cy.cGet('#document-container').rightclick(centerX, centerY);
		})

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
			.should('be.visible').click();

		cy.cGet('#document-container svg g').should('not.exist');
	});

	it.skip('Crop Image', function () {
		mobileHelper.insertImage();

		cy.cGet('.mobile-wizard-back.close-button').click();

		helper.assertImageSize(514, 130);

		cy.cGet('#test-div-shape-handle-3').should('exist');
		cy.cGet('#toolbar-down .ui-scroll-right').click({ force: true });
		cy.cGet('#toolbar-down .ui-scroll-right').click({ force: true });
		cy.cGet('#crop-button').click({ force: true });

		cy.cGet('#test-div-shape-handle-3').then(($handle) => {
			const rect = $handle[0].getBoundingClientRect();
			const startX = rect.left + rect.width / 2;
			const startY = rect.top + rect.height / 2;
			const moveX = 20;

			cy.cGet('#document-canvas').realSwipe("toRight", { x: startX, y: startY, length: moveX });
		});

		helper.assertImageSize(494, 115);
	});
});
