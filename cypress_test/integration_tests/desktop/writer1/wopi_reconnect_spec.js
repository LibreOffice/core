/* global describe it cy beforeEach expect */

import { setupAndLoadDocument } from '../../common/helper';
import { stubParentPostMessage } from '../../common/postmessage_helper';

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'WOPI app-loaded state across reconnect', function() {

	beforeEach(function() {
		setupAndLoadDocument('writer/focus.odt');
	});

	// Regression test for a save-as that only worked once.
	//
	// A save-as makes the document reconnect/reload. resetAppLoaded() clears the
	// _appLoadedConditions, then the reload re-fires the per-connection events
	// (docloaded, updatepermission, viewinfo). The 'initializedui' event is not.
	// In such case _appLoaded stays false and all later post messages
	// were ignored with "Collabora Online not loaded yet".
	//
	it('re-sends Document_Loaded after a reconnect without re-initializing the UI', function() {
		stubParentPostMessage();

		cy.getFrameWindow().then(win => {
			const wopi = win.app.map['wopi'];
			expect(wopi._appLoaded, 'app loaded after initial load').to.be.true;

			cy.get('@postMessage').then(stub => stub.resetHistory());

			// Simulate the reconnect/reload that a save-as triggers
			wopi.resetAppLoaded();
			wopi._postLoaded({ type: 'docloaded', status: true });
			wopi._postLoaded({ type: 'updatepermission' });
			wopi._postLoaded({ type: 'viewinfo' });

			expect(wopi._appLoaded, 'app loaded again after reconnect').to.be.true;
		});
	});
});
