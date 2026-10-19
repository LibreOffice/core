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
});
