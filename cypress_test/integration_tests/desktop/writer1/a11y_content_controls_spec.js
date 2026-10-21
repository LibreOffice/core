/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer content controls for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_content_controls.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('a checkbox is read as a checkbox, with its title', function () {
		cy.cGet('#clipboard-area [role="checkbox"]').should(function ($checkboxes) {
			expect($checkboxes, 'the checkboxes').to.have.length(1);
			expect($checkboxes.attr('aria-checked'), 'the state').to.equal('false');
			expect($checkboxes.attr('aria-description'), 'the title').to.equal('Agreement');
			expect($checkboxes.attr('contenteditable'), 'one unit for the caret').to.equal('false');
			expect($checkboxes.text(), 'the character drawn').to.equal('o');
		});
		// the accessibility tree follows the DOM later
		cy.waitUntil(function () {
			return a11yHelper.getAXNodesWithin('#clipboard-area').then(function (nodes) {
				const checkboxes = nodes.filter(function (node) {
					return !node.ignored && node.role === 'checkbox';
				});
				return checkboxes.length === 1 && checkboxes[0].description === 'Agreement';
			});
		});
	});

	it('the other controls are read by their type and title', function () {
		cy.cGet('#clipboard-area [role="mark"]').should(function ($marks) {
			expect($marks, 'the marks').to.have.length(3);
			expect($marks.toArray().map(function (mark) { return mark.textContent; }),
				'the content').to.deep.equal(['red', 'Type a name', 'Some notes']);
			expect($marks.toArray().map(function (mark) {
				return mark.getAttribute('aria-roledescription');
			}), 'the types').to.deep.equal(['Drop-down list', 'Content control', 'Content control']);
			expect($marks.toArray().map(function (mark) {
				return mark.getAttribute('aria-description');
			}), 'the titles').to.deep.equal(['Colour', 'Placeholder', 'Notes']);
		});
	});

	it('the text keeps no character marking the ends of a control', function () {
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the paragraph text')
				.to.contain('Agree: o, colour: red, name: Type a name, notes: Some notes.');
		});
	});

	it('Alt+Down opens the list of a drop-down list, and Enter chooses from it', function () {
		// into the drop-down list starting the paragraph
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}{rightarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.realPress(['Alt', 'ArrowDown']);
		cy.cGet('.jsdialog-window.modalpopup .ui-treeview-entry').should(function ($entries) {
			expect($entries.toArray().map(function (entry) { return entry.textContent; }),
				'the items').to.deep.equal(['small', 'big']);
			expect($entries.closest('.modalpopup')[0].contains($entries[0].ownerDocument.activeElement),
				'the focus in the list').to.equal(true);
		});

		// an arrow moves through the list without choosing
		cy.realPress('ArrowDown');
		cy.cGet('.jsdialog-window.modalpopup .ui-treeview-entry').should(function ($entries) {
			expect($entries[1].ownerDocument.activeElement, 'the focus on the second item')
				.to.equal($entries[1]);
		});
		cy.realPress('Enter');
		cy.cGet('.jsdialog-window.modalpopup').should('not.exist');
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the item chosen').to.contain('big size.');
		});
	});

	it('Space toggles a checkbox entered after a drop-down list', function () {
		// out of the drop-down list the previous test chose from, into the checkbox; Ctrl stays
		// held for the rest of a typeIntoDocument()
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(8));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.realPress('Space');
		cy.cGet('#clipboard-area [role="checkbox"]').should('have.attr', 'aria-checked', 'true');
		cy.cGet('.jsdialog-window.modalpopup').should('not.exist');
	});

	it('Alt+Down opens the calendar of a date, on its date, and the keyboard chooses another', function () {
		// into the date starting the third paragraph
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{downarrow}{downarrow}{rightarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.realPress(['Alt', 'ArrowDown']);
		cy.cGet('#datepicker').should('be.visible');
		// each day by its full date
		cy.cGet('#datepicker').should(function ($picker) {
			const day = $picker[0].ownerDocument.activeElement;
			expect($picker[0].contains(day), 'the focus in the calendar').to.equal(true);
			expect(day.getAttribute('aria-label'), 'the date of the control').to.contain('March 14, 2025');
		});

		cy.realPress('ArrowRight');
		cy.cGet('#datepicker').should(function ($picker) {
			expect($picker[0].ownerDocument.activeElement.getAttribute('aria-label'), 'the next day')
				.to.contain('March 15, 2025');
		});

		// a month on, then closed: opened again, the calendar is back on the date of the control
		cy.realPress('PageDown');
		cy.cGet('#datepicker').should(function ($picker) {
			expect($picker[0].ownerDocument.activeElement.getAttribute('aria-label'), 'a month on')
				.to.contain('April 15, 2025');
		});
		cy.realPress('Escape');
		cy.cGet('#datepicker').should('not.be.visible');
		cy.realPress(['Alt', 'ArrowDown']);
		cy.cGet('#datepicker').should(function ($picker) {
			const day = $picker[0].ownerDocument.activeElement;
			expect(day.getAttribute('aria-label'), 'the date of the control').to.contain('March 14, 2025');
			expect(day.getAttribute('aria-selected'), 'the date of the control').to.equal('true');
		});
		cy.realPress('ArrowRight');
		cy.cGet('#datepicker').should(function ($picker) {
			expect($picker[0].ownerDocument.activeElement.getAttribute('aria-label'), 'the next day')
				.to.contain('March 15, 2025');
		});

		// a year on, and back
		cy.realPress(['Shift', 'PageDown']);
		cy.cGet('#datepicker').should(function ($picker) {
			expect($picker[0].ownerDocument.activeElement.getAttribute('aria-label'), 'a year on')
				.to.contain('March 15, 2026');
		});
		cy.realPress(['Shift', 'PageUp']);
		cy.cGet('#datepicker').should(function ($picker) {
			expect($picker[0].ownerDocument.activeElement.getAttribute('aria-label'), 'back')
				.to.contain('March 15, 2025');
		});

		cy.realPress('Enter');
		cy.cGet('#datepicker').should('not.be.visible');
		cy.cGet('#clipboard-area').should(function ($area) {
			expect($area.text(), 'the date chosen').to.contain('2025-03-15 day.');
		});
	});
});
