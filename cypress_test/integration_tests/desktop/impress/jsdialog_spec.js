/* global describe it cy require beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'JSDialog Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/jsdialog.odp');
	});

	// fails on Panel6
	it.skip('Check disabled state in animation sidebar', function() {
		// open animation deck
		cy.cGet('#options-custom-animation-button').should('not.have.class', 'selected');
		cy.cGet('#options-custom-animation-button').click();
		cy.cGet('#options-custom-animation-button').should('have.class', 'selected');

		cy.cGet('#Panel6').should('be.visible');

		// all options are disabled
		cy.cGet('#start_effect_list-input').should('be.disabled');
		cy.cGet('#combo-input').should('be.disabled');
		cy.cGet('#anim_duration-input').should('be.disabled');
		cy.cGet('#delay_value-input').should('be.disabled');

		// select animation entry
		cy.cGet('#categorylb-input').should('be.disabled');

		cy.cGet('#custom_animation_list')
			.contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 1').click();

		cy.cGet('#categorylb-input').should('not.be.disabled');

		cy.wait(1000);

		// some options are enabled
		cy.cGet('#start_effect_list-input').should('not.be.disabled');
		cy.cGet('#combo-input').should('not.be.visible');
		cy.cGet('#anim_duration-input').should('not.be.disabled');
		cy.cGet('#delay_value-input').should('not.be.disabled');

		// use different type of animation
		cy.cGet('#categorylb-input').should('be.visible').select('Entrance');
		cy.cGet('#effect_list').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Fly In').click();

		// all options are enabled
		cy.cGet('#start_effect_list-input').should('not.be.disabled');
		cy.cGet('#combo-input').should('not.be.disabled');
		cy.cGet('#anim_duration-input').should('not.be.disabled');
		cy.cGet('#delay_value-input').should('not.be.disabled');
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Master Page Preview', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/masterpagepreview.odp');
	});

	it('To - from master page switch should update previews.', function() {
		cy.cGet('#masterslidebutton').click();
		cy.getFrameWindow().then(function (win) {
			return helper.processToIdle(win);
		});
		cy.cGet('#preview-img-part-0').compareSnapshot('master_page_preview_0', 0.25);
	});
});
