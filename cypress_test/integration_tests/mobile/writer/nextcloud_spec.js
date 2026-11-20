/* global describe it cy require Cypress */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var nextcloudHelper = require('../../common/nextcloud_helper');

describe(['tagnextcloud'], 'Nextcloud specific tests.', function() {

	it('Insert image from storage.', function() {
		helper.setupAndLoadDocument('writer/nextcloud.odt', false, false, undefined, 'vector=false');
		mobileHelper.enableEditingMobile();

		helper.upLoadFileToNextCloud('writer/image_to_insert.png');
		nextcloudHelper.insertImageFromStorage('image_to_insert.png');

		cy.get('#document-container svg g.Graphic')
			.should('exist');
	});

	it('Save as.', function() {
		var newFilePath = helper.setupAndLoadDocument('writer/nextcloud.odt', false, false, undefined, 'vector=false');
		var newFileName = helper.getFileName(newFilePath);
		mobileHelper.enableEditingMobile();

		nextcloudHelper.saveFileAs('1' + newFileName);

		// Close the document
		cy.get('#mobile-edit-button')
			.should('be.visible');

		cy.get('##toolbar-mobile-back')
			.then(function(item) {
				cy.wrap(item)
					.click();
				Cypress.env('IFRAME_LEVEL', '');
			});

		cy.get('tr[data-file=\'1' + newFileName + '\']')
			.should('be.visible');

		cy.get('tr[data-file=\'' + newFileName + '\']')
			.should('be.visible');
	});

	it('Share.', function() {
		helper.setupAndLoadDocument('writer/nextcloud.odt', false, false, undefined, 'vector=false');
		mobileHelper.enableEditingMobile();

		nextcloudHelper.checkAndCloseSharing();
	});
});

