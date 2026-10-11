/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Writer multi image selection', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/image_operation.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920, 1080);
		cy.getFrameWindow().then((win) => {
			this.win = win;
			return helper.processToIdle(win);
		});
	});

	// The rectangle of the selection handles on screen, which is the rectangle around
	// every selected image.
	function readSelectionRect(target) {
		cy.cGet('#test-div-shapeHandlesSection').should('exist').then(function($handles) {
			const rect = $handles[0].getBoundingClientRect();
			target.left = rect.left;
			target.top = rect.top;
			target.right = rect.right;
			target.bottom = rect.bottom;
			target.centerX = rect.left + rect.width / 2;
			target.centerY = rect.top + rect.height / 2;
		});
	}

	function clickAt(win, rect, modifiers) {
		// realClick sends input through the browser itself, so a modifier key is seen the way
		// a user's key press is.
		cy.then(function() {
			cy.cGet('body').realClick(Object.assign({ x: rect.centerX, y: rect.centerY }, modifiers || {}));
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});
	}

	it('Shift-click adds a second image and Delete removes both', function() {
		const win = this.win;
		const first = {};
		const second = {};
		const both = {};
		const bothAgain = {};

		// The first image is selected right after it is inserted.
		desktopHelper.insertImage();
		readSelectionRect(first);

		// The second image lands on the same anchor paragraph, so move it below the first one
		// with the arrow keys while it is still selected.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		desktopHelper.insertImage();
		helper.typeIntoDocument('{downArrow}'.repeat(24));
		cy.then(function() {
			return helper.processToIdle(win);
		});
		readSelectionRect(second);
		cy.then(function() {
			expect(second.top).to.be.greaterThan(first.bottom);
		});

		// Start from no selection, select the first image, then add the second one.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		clickAt(win, first);
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		// The second click waits for the click merge timer, so it stays a click of its own.
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		clickAt(win, second, { shiftKey: true });

		// One handles rectangle now spans both images.
		readSelectionRect(both);
		cy.then(function() {
			expect(both.top).to.be.closeTo(first.top, 5);
			expect(both.bottom).to.be.closeTo(second.bottom, 5);
			expect(both.left).to.be.closeTo(Math.min(first.left, second.left), 5);
			expect(both.right).to.be.closeTo(Math.max(first.right, second.right), 5);
		});

		// Delete removes both images: the handles go, and a click where each image was selects
		// nothing.
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		clickAt(win, first);
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		clickAt(win, second);
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');

		// Undo brings both images back, and they can be selected together again.
		helper.typeIntoDocument('{ctrl}z');
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		clickAt(win, first);
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		cy.then(function() {
			return helper.waitForTimers(win, 'clicktimer');
		});
		clickAt(win, second, { shiftKey: true });
		readSelectionRect(bothAgain);
		cy.then(function() {
			expect(bothAgain.top).to.be.closeTo(both.top, 5);
			expect(bothAgain.bottom).to.be.closeTo(both.bottom, 5);
		});
	});
});
