/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The panel is built once the document has loaded. Wait for it and for the queued layout work,
// so a panel that is hidden really stays hidden.
function assertPanelShown(shown) {
	cy.cGet('#tab-slide-sorter').should('exist');
	cy.getFrameWindow().then(function(win) {
		return helper.processToIdle(win);
	});
	cy.cGet('#navigation-sidebar').should(shown ? 'have.class' : 'not.have.class', 'visible');
}

describe(['tagdesktop'], 'Navigator option in Draw', function() {
	var filePath;

	beforeEach(function() {
		filePath = helper.setupAndLoadDocument('draw/navigator.odg');
	});

	it('the Navigator option hides and shows the slide panel, also after a reload', function() {
		assertPanelShown(true);

		desktopHelper.toggleInterfaceSetting('drawing', 'ShowNavigator', true);
		assertPanelShown(false);

		helper.reloadDocument(filePath);
		assertPanelShown(false);

		// The test WOPI server does not hand the saved settings back to the reloaded document, so
		// the dialog would open on the defaults. Apply the setting the way the dialog's Save does.
		cy.getFrameWindow().then(function(win) {
			win.app.map.settings.applyBrowserSettings({ 'drawing.ShowNavigator': 'true' });
		});
		assertPanelShown(true);
		cy.cGet('#tab-slide-sorter').should('have.class', 'selected');

		helper.reloadDocument(filePath);
		assertPanelShown(true);
	});

	it('the slide panel closed or reopened by hand stays that way after a reload', function() {
		assertPanelShown(true);

		desktopHelper.closeNavigatorSidebar();
		assertPanelShown(false);

		helper.reloadDocument(filePath);
		assertPanelShown(false);

		cy.cGet('#floating-navigator').click();
		assertPanelShown(true);

		helper.reloadDocument(filePath);
		assertPanelShown(true);
	});
});
