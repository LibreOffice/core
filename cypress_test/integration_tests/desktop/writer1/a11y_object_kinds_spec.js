/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer shapes and objects for the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_object_kinds.fodt');

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

	it('a shape and a formula are read as what they are, not as images', function () {
		cy.cGet('#clipboard-area [role="img"]').should(function ($images) {
			expect($images, 'the images').to.have.length(2);
			expect($images.eq(0).attr('aria-label'), 'the shape name').to.equal('Shape 1 Blue box');
			expect($images.eq(0).attr('aria-roledescription'), 'the shape role').to.equal('Rectangle');
			expect($images.eq(1).attr('aria-label'), 'the formula name').to.equal('Half of b');
			expect($images.eq(1).attr('aria-roledescription'), 'the formula role').to.equal('Formula');
		});
		cy.then(function () {
			return a11yHelper.getAXNodesWithin('#clipboard-area').then(function (nodes) {
				const images = nodes.filter(function (node) {
					return !node.ignored && (node.role === 'image' || node.role === 'img');
				});
				expect(images.map(function (node) { return node.name; }), 'the objects read')
					.to.deep.equal(['Shape 1 Blue box', 'Half of b']);
				expect(images.map(function (node) { return node.properties.roledescription; }),
					'the roles read').to.deep.equal(['Rectangle', 'Formula']);
			});
		});
	});

	it('a shape selected from the keyboard is announced by its name', function () {
		// nothing lies below the caret, so Shift+F4 wraps around to the topmost object, the shape
		cy.realPress(['Shift', 'F4']);
		cy.cGet('#clipboard-area')
			.should('have.attr', 'aria-description', 'Shape 1 Blue box selected. ');
		cy.realPress('Escape');
		// typing is blocked while a selection is held, though not under Cypress
		cy.cGet('#clipboard-area').should(function () {
			expect(win.app.map._textInput._hasAnySelection, 'a selection held').to.equal(false);
		});
	});
});
