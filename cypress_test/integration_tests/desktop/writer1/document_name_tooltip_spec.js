/* global describe it cy require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Document name input tooltip', function () {
	it('shows only the filename, not the technical WOPISrc path', function () {
		helper.setupAndLoadDocument('writer/testfile.fodt');

		cy.getFrameWindow().then(function (win) {
			var title = win.document.getElementById('document-name-input').title;

			// Before the fix, the tooltip appended "\nPath: <WOPISrc pathname>" -
			// a technical, opaque file identifier on a real WOPI host, not a
			// folder path, so it should not appear here at all.
			expect(title).to.equal(win.app.map['wopi'].BaseFileName);
			expect(title).to.not.contain('Path');
		});
	});
});
