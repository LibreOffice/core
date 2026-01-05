/* global cy Cypress expect */

function confirmChange(action) {
	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).click();
	cy.cGet('#menu-changesmenu').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).contains(action).click();
	cy.cGet('body').type('{esc}');
	cy.cGet('#menu-changesmenu').should('not.be.visible');
}

//enable record for track changes
function enableRecord() {
	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').click();
	cy.cGet('#menu-changesmenu').contains('Record').click();

	cy.cGet('body').type('{esc}');

	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').click();
	cy.cGet('#menu-changesmenu').contains('Record').should('have.class', 'lo-menu-item-checked');

	//to close
	cy.cGet('#menu-changesmenu').click();
}

module.exports.confirmChange = confirmChange;
module.exports.enableRecord = enableRecord;
