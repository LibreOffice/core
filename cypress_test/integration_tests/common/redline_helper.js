/* global cy Cypress expect require */

var helper = require('./helper');

function confirmChange(win, action) {
	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).click();
	cy.cGet('#menu-changesmenu').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).contains(action).click();
	cy.cGet('body').type('{esc}');
	cy.cGet('#menu-changesmenu').should('not.be.visible');
	// Wait for core to apply the change before checking comment DOM state.
	helper.processToIdle(win);
}

//enable record for track changes
function enableRecord(win) {
	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').click();
	cy.cGet('#menu-changesmenu').contains('Record').click();

	cy.cGet('body').type('{esc}');

	cy.cGet('#menu-editmenu').click();
	cy.cGet('#menu-changesmenu').click();
	cy.cGet('#menu-changesmenu').contains('Record').should('have.class', 'lo-menu-item-checked');

	//to close
	cy.cGet('#menu-changesmenu').click();
	cy.cGet('body').type('{esc}');
	cy.cGet('#menu-changesmenu').should('not.be.visible');
	helper.processToIdle(win);
}

module.exports.confirmChange = confirmChange;
module.exports.enableRecord = enableRecord;
