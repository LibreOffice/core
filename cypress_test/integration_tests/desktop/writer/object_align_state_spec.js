/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Object alignment buttons follow the selection', function() {
	var commands = ['ObjectAlignLeft', 'AlignCenter', 'ObjectAlignRight',
		'AlignUp', 'AlignMiddle', 'AlignDown'];

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/image_operation.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			return helper.processToIdle(win);
		}.bind(this));
	});

	it('are disabled while no object is selected', function() {
		cy.cGet('#Layout-tab-label').click();
		commands.forEach(function(command) {
			desktopHelper.getNbItem(command, 'Layout').should('have.attr', 'disabled');
		});
	});

	it('are enabled while an image is selected', function() {
		desktopHelper.insertImage();
		cy.cGet('#document-container svg g').should('exist');
		helper.processToIdle(this.win);

		cy.cGet('#Layout-tab-label').click();
		commands.forEach(function(command) {
			desktopHelper.getNbItem(command, 'Layout').should('not.have.attr', 'disabled');
		});
	});
});
