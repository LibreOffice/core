/* global describe it cy require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Insert a new slide from the slide panel', function () {
	it('pressing Enter with the slide panel focused creates a new slide', function () {
		helper.setupAndLoadDocument('impress/comment_switching.odp');

		cy.cGet('.preview-img-currentpart').click();
		cy.cGet('.preview-img-currentpart').should('have.focus');

		cy.getFrameWindow().then(function (win) {
			cy.wrap(win.app.map._docLayer._parts).as('partsBefore');
		});

		cy.realPress('Enter');

		cy.get('@partsBefore').then(function (partsBefore) {
			cy.getFrameWindow().should(function (win) {
				expect(win.app.map._docLayer._parts, 'slide count after Enter').to.equal(partsBefore + 1);
				expect(win.app.map._docLayer._selectedPart, 'the new slide becomes selected').to.equal(partsBefore);
			});
		});
	});
});
