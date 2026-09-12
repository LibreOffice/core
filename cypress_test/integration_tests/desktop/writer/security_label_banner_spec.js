/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

// Browser-level validation of the security-label visual pipeline: the on-screen
// classification banner and the Security_Label_Changed host postMessage react to a
// label being applied, changed and removed.
//
// The in-document header/footer/watermark markings are core-rendered and covered by
// the C++ round-trip tests (sw/qa/.../ooxmlexport3.cxx). Driving a real apply through
// the dialog needs a SPIF policy provisioned into the jail, which the Cypress harness
// does not set up; so here we load a plain OOXML document (which is enough for the
// command to be supported) and feed the exact enriched .uno:SecurityLabel statechanged
// message that wsd broadcasts on apply/change/remove, through the real client parser.

describe(['tagdesktop'], 'Writer security-label banner', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/testfile.docx');
	});

	// The message wsd broadcasts to every client when a label is applied/changed/removed.
	function pushLabelChange(win, state) {
		win.app.map._docLayer._onStateChangedMsg(
			'statechanged: ' + JSON.stringify({
				commandName: '.uno:SecurityLabel',
				state: state,
			}));
	}

	it('shows, updates and clears the banner and fires the host postMessage', function() {
		// No label yet: the banner exists (seeded by the on-load commandvalues) but is hidden.
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

		// Apply: the banner shows the marking and the host is notified.
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

		// Change: the banner text updates; the postMessage carries both old and new.
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

		// Remove: the banner hides again; the postMessage carries the removed marking.
		cy.cGet('#security-label-banner').should('not.be.visible');
		cy.get('@mapFire').should('have.been.calledWithMatch', 'postMessage', {
			msgId: 'Security_Label_Changed',
			args: { action: 'removed', oldMarking: 'CONFIDENTIAL//REL', marking: '' },
		});
	});
});
