/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Accessibility menu button keyboard tests', { testIsolation: false }, function () {
	let win;
	let menu;

	before(function () {
		helper.setupAndLoadDocument('draw/insert_position.fodg');

		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			menu = desktopHelper.findStatelessMenuButton(win, 'Home');
			expect(menu, 'a Home tab menu button whose entries carry no state')
				.to.not.equal(null);
		});
	});

	it('Arrow keys move through a menu popup that has no selected entry', function () {
		// A folded group keeps its buttons in an overflow popup.
		cy.then(function () {
			if (desktopHelper.isWidgetButtonShown(win, menu.id)) return;
			desktopHelper.getWidgetButton(menu.overflow).click();
		});

		desktopHelper.getWidgetButton(menu.id).filter(':visible').click();

		desktopHelper.getOpenMenuEntries().should('have.length.above', 1);
		desktopHelper.getOpenMenuEntries().first().should('have.focus');

		// Nothing is selected on open, so the first Down selects before moving.
		cy.realPress('ArrowDown');
		desktopHelper.getOpenMenuEntries().first()
			.should('have.class', 'selected').and('have.focus');

		cy.realPress('ArrowDown');
		desktopHelper.getOpenMenuEntries().eq(1).should('have.focus');

		cy.realPress('ArrowUp');
		desktopHelper.getOpenMenuEntries().first().should('have.focus');
	});
});
