/* global describe it cy require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Save indicator', function() {

	function editAndSave(fixture) {
		helper.setupAndLoadDocument(fixture);
		cy.cGet('[id^="save"].unotoolbutton').should('be.visible');

		helper.typeIntoDocument('hello');
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');

		helper.typeIntoDocument('{ctrl+s}');
	}

	it('reports a finished save on an ODF document', function() {
		editAndSave('writer/annotation.odt');
		cy.cGet('[id^="save"].unotoolbutton', { timeout: 20000 }).should('have.class', 'saved');
	});

	it('reports a finished save on a DOCX document', function() {
		editAndSave('writer/testfile.docx');
		cy.cGet('[id^="save"].unotoolbutton', { timeout: 20000 }).should('have.class', 'saved');
	});

	it('reports save progress from the engine on a DOCX document', function() {
		helper.setupAndLoadDocument('writer/testfile.docx');
		cy.cGet('[id^="save"].unotoolbutton').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			win.recordedSaveProgress = [];
			win.app.map.on('statusindicator', function(e) {
				if (e.background)
					win.recordedSaveProgress.push(e.statusType);
			});
		});

		helper.typeIntoDocument('hello');
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');

		helper.typeIntoDocument('{ctrl+s}');

		cy.getFrameWindow().should(function(win) {
			expect(win.recordedSaveProgress).to.include('start');
			expect(win.recordedSaveProgress).to.include('finish');
		});
	});
});
