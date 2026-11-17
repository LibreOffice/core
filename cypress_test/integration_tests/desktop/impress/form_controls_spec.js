/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect Cypress */

var helper = require('../../common/helper');
var formHelper = require('../../common/form_controls_helper');

// The form controls of a slide in a view drawn from vector primitives. Out of design mode they
// work as elements over the page, and what is done with them goes to the server. In design mode
// they are objects of the slide. A view that cannot edit shows them read-only.
describe(['tagdesktop'], 'Form controls on a vector slide', function() {

	beforeEach(function() {
		formHelper.loadFormDocument(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('The controls work in the browser, in design mode they are objects to edit, and a view that cannot edit shows them read-only.', function() {
		var win = this.win;

		// The document opens out of design mode, and every control of the slide is an element
		// over the page.
		formHelper.waitForDesignMode(false);
		cy.cGet('.vector-form-controls .vector-form-control').should('have.length', 14);
		formHelper.getPart(win, 'fixed', 'input').should('have.prop', 'readOnly', true);
		formHelper.getPart(win, 'off', 'input').should('be.disabled');
		// a read-only list that drops down offers no choice
		formHelper.getPart(win, 'size', 'select').should('be.disabled');
		formHelper.getControl(win, 'caption').then(function(caption) {
			expect(Cypress.$(caption)[0].querySelector('input, select, textarea, button'),
				'a part of the label that takes input').to.equal(null);
		});

		// A text field sends what was typed when it is left, and shows what the server holds.
		formHelper.getPart(win, 'name', 'input').should('have.value', 'Alice');
		formHelper.typeInto(win, 'name', 'Bob', true);
		formHelper.leaveField(win, 'name');
		formHelper.expectSent(win, 'text', 'Bob');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'name', 'input').should('have.value', 'Bob');

		// A numeric field holds a number. The server keeps the number where it is sent text.
		formHelper.typeInto(win, 'amount', '12', true);
		formHelper.leaveField(win, 'amount');
		formHelper.expectSent(win, 'text', '12').then(function(amountId) {
			cy.then(function() {
				return helper.processToIdle(win);
			});
			formHelper.getPart(win, 'amount', 'input').should(function(input) {
				expect(parseFloat(Cypress.$(input)[0].value), 'the number of the field').to.equal(12);
			});
			cy.then(function() {
				win.app.socket.sendMessage('formcontrol id=' + amountId + ' action=text value=abc');
			});
			cy.then(function() {
				return helper.processToIdle(win);
			});
			formHelper.getPart(win, 'amount', 'input').should(function(input) {
				expect(parseFloat(Cypress.$(input)[0].value), 'the number after a text').to.equal(12);
			});
		});

		// A check box and an option button take a click.
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('not.be.checked');
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').click();
		formHelper.expectSent(win, 'state', '1');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('be.checked');

		formHelper.getPart(win, 'yes', 'input[type="radio"]').should('not.be.checked');
		formHelper.getPart(win, 'yes', 'input[type="radio"]').click();
		formHelper.expectSent(win, 'check', '');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'yes', 'input[type="radio"]').should('be.checked');

		// A list that drops down selects the entry chosen in the browser's list.
		formHelper.getPart(win, 'colour', 'select').should('have.prop', 'selectedIndex', 0);
		formHelper.getPart(win, 'colour', 'select').select('Green');
		formHelper.expectSent(win, 'select', '1');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'colour', 'select').should('have.prop', 'selectedIndex', 1);

		// A list that shows its entries selects the entry clicked.
		formHelper.getControl(win, 'count').then(function(count) {
			cy.wrap(count).contains('.ui-listbox > div', 'Two').click();
		});
		formHelper.expectSent(win, 'select', '1');

		// A push button is pressed on the server.
		formHelper.getPart(win, 'press', 'button').click();
		formHelper.expectSent(win, 'press', '');

		// A date field opens a calendar, where a day is picked.
		formHelper.getPart(win, 'day', '.ui-edit > :nth-child(2)').click();
		formHelper.getPart(win, 'day', '.vector-form-open-part').then(function(calendar) {
			cy.wrap(calendar).contains('span', /^15$/).click();
		});
		formHelper.expectSent(win, 'date', /\d{4}-\d{2}-15/);
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'day', 'input').should('not.have.value', '');

		// A time field takes the time in the browser's time input.
		formHelper.getPart(win, 'hour', 'input').should('have.attr', 'type', 'time');
		// The browser's time input shows the time in 12 or 24 hours by the language, so the time
		// goes in as its value, which is the same in every language.
		formHelper.getPart(win, 'hour', 'input').type('10:30');
		formHelper.leaveField(win, 'hour');
		formHelper.expectSent(win, 'time', '10:30:00');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'hour', 'input').should('have.value', '10:30');

		/*
			A pattern field takes its text through two masks of the same length, one position of
			the text for each position of the masks. The edit mask says what a position takes: L a
			fixed character, N a digit, n a digit or a space, a a letter, A a letter in upper case,
			c a letter or a digit, C the same in upper case, x any character, X the same in upper
			case. The literal mask gives the fixed characters at the L positions and what the other
			positions show while they are empty. With strict format on, the field takes nothing
			that the edit mask does not allow.

			The phone field of the slide has the edit mask LNNNLLNNNLNNNN and the literal mask
			"(___) ___-____", so it holds a number such as "(555) 123-4567". The digits typed fill
			the free positions in turn, and the caret jumps over the fixed characters.
		*/
		formHelper.getPart(win, 'phone', 'input').click();
		cy.realPress(['Control', 'a']);
		cy.realType('5551234567');
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 123-4567');

		// A letter, which the edit mask does not take at a digit position, is dropped.
		formHelper.getPart(win, 'phone', 'input').then(function(input) {
			Cypress.$(input)[0].setSelectionRange(6, 6);
		});
		cy.realType('a');
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 123-4567');

		// Backspace empties the position before the caret, which shows the literal mask again.
		formHelper.getPart(win, 'phone', 'input').then(function(input) {
			Cypress.$(input)[0].setSelectionRange(14, 14);
		});
		cy.realPress('Backspace');
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 123-456_');
		cy.realType('8');
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 123-4568');

		// The fixed characters can be typed too, so the number goes in as it is written.
		cy.realPress(['Control', 'a']);
		cy.realType('(555) 987-6543');
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 987-6543');

		// Leaving the field sends the text, which the server keeps as it is.
		formHelper.leaveField(win, 'phone');
		formHelper.expectSent(win, 'text', '(555) 987-6543');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		formHelper.getPart(win, 'phone', 'input').should('have.value', '(555) 987-6543');

		// In design mode the controls are objects of the slide: the elements go, and a click
		// selects a control.
		formHelper.switchDesignMode(win, true);
		cy.cGet('.vector-form-controls .vector-form-control').should('have.length', 0);
		cy.wrap(null).should(function() {
			expect(formHelper.formObjects(win), 'form controls of the slide').to.have.length(14);
		});
		formHelper.clickOnCanvas(win, 'name');
		cy.wrap(null).should(function() {
			var selected = formHelper.selectedObject(win);
			expect(selected, 'selected object').to.not.equal(undefined);
			expect(formHelper.isFormControl(win, selected), 'a form control selected').to.equal(true);
		});

		// A control of the Form tab is dragged out in the empty part below the others, and is
		// selected.
		formHelper.clickFormTabButton(win, 'CheckBox');
		formHelper.dragOnCanvas(win, { x: 1701, y: 6236 }, { x: 4535, y: 6803 });
		cy.wrap(null).should(function() {
			expect(formHelper.formObjects(win), 'form controls of the slide').to.have.length(15);
			var selected = formHelper.selectedObject(win);
			expect(selected, 'selected object').to.not.equal(undefined);
			expect(formHelper.isFormControl(win, selected), 'a form control selected').to.equal(true);
		});

		// Out of design mode the new control works as the others do.
		formHelper.switchDesignMode(win, false);
		cy.cGet('.vector-form-controls .vector-form-control').should('have.length', 15);

		// A view that cannot edit shows every control read-only. A field takes no typing, a check
		// box keeps its state, a list offers no choice, and nothing goes to the server.
		cy.then(function() {
			win.app.map.setPermission('readonly');
		});
		// The office answers the new permission with the state of the design mode.
		cy.then(function() {
			return helper.processToIdle(win);
		});
		// The controls are made anew together for the new permission, so the field is looked up
		// again on each try.
		cy.wrap(null).should(function() {
			var control = formHelper.controlAt(win, 'name');
			expect(control, 'the element of name').to.not.equal(null);
			expect(control.querySelector('input').readOnly, 'the name field read-only').to.equal(true);
		});
		formHelper.getPart(win, 'colour', 'select').should('be.disabled');
		formHelper.forgetSent(win);
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('be.checked');
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').click();
		formHelper.getPart(win, 'agree', 'input[type="checkbox"]').should('be.checked');
		formHelper.expectNoFormControlSent(win);
	});
});
