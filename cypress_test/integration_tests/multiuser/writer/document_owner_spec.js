/* global describe it cy require */

const helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser document owner', function() {

	function checkOwners(owner1, owner2) {
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().its('app.map.wopi.IsOwner').should('eq', owner1);

		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().its('app.map.wopi.IsOwner').should('eq', owner2);
	}

	it('Only the first user owns the document', function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt', true, false, undefined, 'vector=false');
		checkOwners(true, false);
	});

	it('Users with ids of their own do not own the document', function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt', true, false, undefined,
			'userid1=owner-spec-first&userid2=owner-spec-second&vector=false');
		checkOwners(false, false);
	});

	it('The user named by ownerid owns the document', function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt', true, false, undefined,
			'userid1=owner-spec-first&userid2=owner-spec-second&ownerid=owner-spec-second&vector=false');
		checkOwners(false, true);
	});
});
