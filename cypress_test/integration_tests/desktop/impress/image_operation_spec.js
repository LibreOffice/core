/* global describe it require cy beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var { triggerNewSVGForShapeInTheCenter } = require('../../common/impress_helper');

describe(['tagdesktop'], 'Image Operation Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/image_operation.odp');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
	});

	it('Insert/Delete image',function() {
		desktopHelper.insertImage();

		//make sure that image is in focus
		cy.cGet('#document-container svg g')
			.should('exist');

		desktopHelper.deleteImage();
	});

	it("Insert multimedia", function () {
		desktopHelper.insertVideo();

		// The video foreignObject lives inside a nested <svg> wrapper.
		// Verify the wrapper has explicit dimensions so it does not
		// fall back to the SVG default 300x150 and clip the video.
		cy.cGet('#document-container svg svg').should('have.attr', 'width');
		cy.cGet('#document-container svg svg').should('have.attr', 'height');
		cy.cGet('#document-container svg svg foreignObject').then(function ($fo) {
			var foWidth = $fo.attr('width');
			var foHeight = $fo.attr('height');
			cy.cGet('#document-container svg svg').should('have.attr', 'width', foWidth);
			cy.cGet('#document-container svg svg').should('have.attr', 'height', foHeight);
		});
	});

	it.skip('Crop Image', function () {
		desktopHelper.insertImage();
		helper.assertImageSize(438, 111);

		cy.cGet('#Crop').should('be.visible');
		cy.cGet('#Crop').click();

		cy.cGet('#test-div-shape-handle-3').then(($handle) => {
			const rect = $handle[0].getBoundingClientRect();
			const startX = rect.left + rect.width / 2;
			const startY = rect.top + rect.height / 2;
			const moveX = 20;

			cy.cGet('body').realMouseDown({ x: startX, y: startY });
			cy.cGet('body').realMouseMove(startX + moveX, startY);
			cy.cGet('body').realMouseUp();
		});

		cy.wait(1000);

		cy.cGet('#canvas-container > svg').should('exist');
		cy.cGet('#test-div-shape-handle-3').should('exist');
		helper.assertImageSize(418, 111);
	});


	it('Resize image when keep ratio option enabled and disabled', function() {
		cy.cGet('#optionstoolboxdown .unoModifyPage button').click();	
		cy.cGet('#sidebar-panel').should('not.be.visible');
		
		desktopHelper.insertImage();
		//when Keep ratio is unchecked
		helper.assertImageSize(438, 111);
		cy.viewport(1000,660);

		cy.cGet('#optionstoolboxdown .unoModifyPage button').click();
		cy.cGet('#sidebar-panel').should('be.visible');

		//sidebar needs more time
		cy.cGet('#sidebar-dock-wrapper').should('be.visible').wait(2000).scrollTo('bottom');

		cy.cGet('.ui-expander-label').contains('Position and Size').should('be.visible').click();

		cy.cGet('#selectwidth input').type('{selectAll}{backspace}10{enter}');

		cy.cGet('#selectheight input').type('{selectAll}{backspace}4{enter}');

		triggerNewSVGForShapeInTheCenter();

		helper.assertImageSize(322, 129);

		//Keep ratio checked
		//sidebar needs more time
		cy.cGet('#sidebar-dock-wrapper').should('be.visible').wait(2000).scrollTo('bottom');

		cy.cGet('.ui-expander-label').contains('Position and Size').should('be.visible').click();

		cy.cGet('#ratio input').check();

		cy.cGet('#selectheight input').type('{selectAll}{backspace}5{enter}');

		triggerNewSVGForShapeInTheCenter();

		helper.assertImageSize(402, 161);
	});
});
