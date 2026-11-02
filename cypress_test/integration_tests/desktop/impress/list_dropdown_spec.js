/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

function selectText(win) {
	impressHelper.triggerNewSVGForShapeInTheCenter();
	impressHelper.selectTextOfShape();
	helper.processToIdle(win);
}

// At the default test window width the list buttons sit in an overflow menu. Open it, open the
// bullet popup from the dropdown button in it, and check that the popup opens below that button,
// starting at its left edge. The button is measured before the click, because the overflow menu
// closes when the popup opens.
function checkBulletPopupOpensUnderButton(overflowButton) {
	cy.cGet(overflowButton + ' > .arrowbackground').click();

	var bulletItem = '.modalpopup .unoDefaultBullet.has-dropdown';
	cy.cGet(bulletItem).should('be.visible').should('not.have.attr', 'disabled');
	cy.cGet(bulletItem).then(function(item) {
		var buttonRect = item[0].getBoundingClientRect();
		cy.cGet(bulletItem + ' > .arrowbackground').click();
		cy.cGet('.modalpopup #numbering_window_iconview')
			.should('be.visible')
			.parents('.modalpopup')
			.should(function(popup) {
				var popupRect = popup[0].getBoundingClientRect();
				expect(Math.abs(popupRect.left - buttonRect.left)).to.be.lessThan(5);
				expect(Math.abs(popupRect.top - buttonRect.bottom)).to.be.lessThan(5);
			});
	});
}

describe(['tagdesktop'], 'Bullet dropdown in the compact toolbar', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/apply_paragraph_props_text.odp');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('Bullet popup opens under the toolbar button', function() {
		selectText(this.win);
		checkBulletPopupOpensUnderButton('#toolbar-up [id^="overflow-button-other-toptoolbar"]');
	});
});

describe(['tagdesktop'], 'Bullet dropdown in the notebookbar', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/apply_paragraph_props_text.odp');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('Bullet popup opens under the notebookbar button', function() {
		selectText(this.win);
		checkBulletPopupOpensUnderButton('#Home-container [id^="overflow-button-home-paragraph"]');
	});
});
