/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

// Browser-level validation of the security-label visual pipeline (banner + host
// postMessage) for Impress. See the Writer spec for the rationale: the in-document
// markings are core-rendered and covered by C++ tests, and a real apply needs a
// provisioned SPIF policy, so this feeds the enriched .uno:SecurityLabel statechanged
// message that wsd broadcasts on apply/change/remove through the real client parser.

describe(['tagdesktop'], 'Impress security-label banner', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.pptx');
	});

	function pushLabelChange(win, state) {
		win.app.map._docLayer._onStateChangedMsg(
			'statechanged: ' + JSON.stringify({
				commandName: '.uno:SecurityLabel',
				state: state,
			}));
	}

	it('shows, updates and clears the banner and fires the host postMessage', function() {
		cy.cGet('#security-label-banner').should('not.be.visible');

		cy.getFrameWindow().then(function(win) {
			cy.spy(win.app.map, 'fire').as('mapFire');
			pushLabelChange(win, {
				action: 'applied',
				classification: 'SECRET',
				marking: 'SECRET//REL',
				oldClassification: '',
				oldMarking: '',
			});
		});

		cy.cGet('#security-label-banner').should('be.visible').should('have.text', 'SECRET//REL');
		cy.get('@mapFire').should('have.been.calledWithMatch', 'postMessage', {
			msgId: 'Security_Label_Changed',
			args: { action: 'applied', classification: 'SECRET', marking: 'SECRET//REL' },
		});

		cy.getFrameWindow().then(function(win) {
			pushLabelChange(win, {
				action: 'changed',
				classification: 'CONFIDENTIAL',
				marking: 'CONFIDENTIAL//REL',
				oldClassification: 'SECRET',
				oldMarking: 'SECRET//REL',
			});
		});

		cy.cGet('#security-label-banner').should('be.visible').should('have.text', 'CONFIDENTIAL//REL');
		cy.get('@mapFire').should('have.been.calledWithMatch', 'postMessage', {
			msgId: 'Security_Label_Changed',
			args: { action: 'changed', oldMarking: 'SECRET//REL', marking: 'CONFIDENTIAL//REL' },
		});

		cy.getFrameWindow().then(function(win) {
			pushLabelChange(win, {
				action: 'removed',
				classification: '',
				marking: '',
				oldClassification: 'CONFIDENTIAL',
				oldMarking: 'CONFIDENTIAL//REL',
			});
		});

		cy.cGet('#security-label-banner').should('not.be.visible');
		cy.get('@mapFire').should('have.been.calledWithMatch', 'postMessage', {
			msgId: 'Security_Label_Changed',
			args: { action: 'removed', oldMarking: 'CONFIDENTIAL//REL', marking: '' },
		});
	});
});
