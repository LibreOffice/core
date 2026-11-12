/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect Cypress */

var helper = require('../../common/helper');
var formHelper = require('../../common/form_controls_helper');

// The keyboard focus among the form controls of a slide in a view drawn from vector primitives.
// The slide lists its controls in another order than their tab order.
describe(['tagdesktop'], 'Keyboard focus in the form controls of a vector slide', function() {

	beforeEach(function() {
		formHelper.loadFormDocument(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('The keyboard moves through the controls in their tab order and comes back to the document.', function() {
		var win = this.win;
		formHelper.waitForDesignMode(false);
		cy.cGet('.vector-form-controls .vector-form-control').should('have.length', 14);

		// With the automatic control focus on, leaving the design mode gives the first control
		// in the tab order the focus.
		formHelper.clickFormTabButton(win, 'AutoControlFocus');
		helper.waitForMapState('.uno:AutoControlFocus', 'true');
		formHelper.switchDesignMode(win, true);
		formHelper.switchDesignMode(win, false);
		formHelper.expectFocusIn(win, 'name');

		// Tab goes through the controls by their tab index. It passes over the push button, at
		// which Tab does not stop, the disabled field, the label and the read-only list that
		// drops down, and from the last control it goes round to the first one.
		['amount', 'agree', 'yes', 'colour', 'count', 'day', 'hour', 'fixed', 'phone', 'name']
			.forEach(function(name) {
				cy.realPress('Tab');
				formHelper.expectFocusIn(win, name);
			});
		cy.realPress(['Shift', 'Tab']);
		formHelper.expectFocusIn(win, 'phone');
		cy.realPress('Tab');
		formHelper.expectFocusIn(win, 'name');

		// Return sends the text of a field, which keeps the focus. Escape gives the keyboard to
		// the document.
		formHelper.forgetSent(win);
		formHelper.typeInto(win, 'name', 'Bob', true);
		cy.realPress('Enter');
		formHelper.expectSent(win, 'text', 'Bob');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'name');
		cy.realPress('Escape');
		cy.wrap(null).should(function() {
			var element = formHelper.focused(win);
			expect(element.closest('.vector-form-controls'), 'the focus in a control').to.equal(null);
			expect(element.classList.contains('clipboard'), 'the focus in the document').to.equal(true);
		});

		// A click on a check box gives it the focus, which stays after the server sends the
		// control back. Space checks it again from the keyboard.
		formHelper.forgetSent(win);
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').click();
		formHelper.expectSent(win, 'state', '1');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'agree');
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('be.checked');
		cy.realPress('Space');
		formHelper.expectSent(win, 'state', '0');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'agree');
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('not.be.checked');

		// A click on a push button leaves the focus where it was.
		formHelper.getPart(win, 'press', 'button').click();
		formHelper.expectSent(win, 'press', '');
		formHelper.expectFocusIn(win, 'agree');

		// A click on an option button gives it the focus.
		formHelper.getPart(win, 'yes', 'input[type="radio"]').click();
		formHelper.expectFocusIn(win, 'yes');

		// A list that shows its entries takes the focus by a click and shows an outline. The
		// arrow keys select the next entry, and the focus stays in the list.
		formHelper.forgetSent(win);
		formHelper.getControl(win, 'count').then(function(count) {
			cy.wrap(count).contains('.ui-listbox > div', 'One').click();
		});
		formHelper.expectFocusIn(win, 'count');
		formHelper.getPart(win, 'count', '.vector-form-list').should(function(list) {
			var style = win.getComputedStyle(Cypress.$(list)[0]);
			expect(style.outlineStyle, 'the outline').to.not.equal('none');
		});
		cy.realPress('ArrowDown');
		formHelper.expectSent(win, 'select', '1');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'count');
		cy.realPress('End');
		formHelper.expectSent(win, 'select', '2');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'count');

		// A list that drops down keeps the focus after a pick, and its drawn field shows the
		// outline.
		formHelper.forgetSent(win);
		formHelper.getPart(win, 'colour', 'select').select('Blue');
		formHelper.expectSent(win, 'select', '2');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'colour');
		formHelper.getPart(win, 'colour', '.ui-edit').should(function(field) {
			var style = win.getComputedStyle(Cypress.$(field)[0]);
			expect(style.outlineStyle, 'the outline').to.not.equal('none');
		});

		// A day picked in the calendar of a date field gives the field the focus.
		formHelper.getPart(win, 'day', '.ui-edit > :nth-child(2)').click();
		formHelper.getPart(win, 'day', '.vector-form-open-part').then(function(calendar) {
			cy.wrap(calendar).contains('span', /^20$/).click();
		});
		formHelper.expectSent(win, 'date', /\d{4}-\d{2}-20/);
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.expectFocusIn(win, 'day');

		// Tab closes the calendar that is open under the date field and goes on to the next
		// control.
		formHelper.getPart(win, 'day', '.ui-edit > :nth-child(2)').click();
		formHelper.getPart(win, 'day', '.vector-form-open-part');
		formHelper.expectFocusIn(win, 'day');
		cy.realPress('Tab');
		formHelper.expectFocusIn(win, 'hour');
		cy.cGet('.vector-form-open-part').should('not.exist');

		// In design mode Delete removes the selected control, also after the automatic control
		// focus has run.
		formHelper.switchDesignMode(win, true);
		cy.wrap(null).should(function() {
			expect(formHelper.formObjects(win), 'form controls of the slide').to.have.length(14);
		});
		formHelper.clickOnCanvas(win, 'name');
		cy.wrap(null).should(function() {
			var selected = formHelper.selectedObject(win);
			expect(selected, 'selected object').to.not.equal(undefined);
			expect(formHelper.isFormControl(win, selected), 'a form control selected').to.equal(true);
		});
		cy.wrap(null).should(function() {
			expect(formHelper.focused(win).classList.contains('clipboard'),
				'the focus in the document').to.equal(true);
		});
		cy.realPress('Delete');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(formHelper.formObjects(win), 'form controls of the slide').to.have.length(13);
		});
	});
});
