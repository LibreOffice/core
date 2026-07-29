/* -*- js-indent-level: 8 -*- */
/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Calc Focus Cell overlay', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	});

	it('Draws the overlay the first time the feature is turned on', function() {
		// The feature has never been enabled, so the section does not exist yet.
		cy.getFrameWindow().should(function(win) {
			expect(win.app.sectionContainer.getSectionWithName(
				win.app.CSections.FocusCell.name),
				'focus cell section absent before first enable').to.not.exist;
		});

		// Turn on View - Focus Cell through the same action the menu dispatches.
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('columnrowhighlight');
			helper.processToIdle(win);
		});

		// isVisible is what decides the overlay is drawn. It must be computed on
		// the first enable, without waiting for a later scroll to recompute it.
		cy.getFrameWindow().should(function(win) {
			var section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.FocusCell.name);
			expect(section, 'focus cell section present after enable').to.exist;
			expect(section.isVisible,
				'overlay visible right after the first enable').to.be.true;
		});
	});
});
