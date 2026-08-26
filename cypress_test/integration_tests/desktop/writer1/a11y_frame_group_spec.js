/* global describe expect it cy beforeEach require */

var helper = require('../../common/helper');

// A frame that groups real widgets is announced as a group, so the label of
// the frame is read with them. buildFrame decides that while the dialog is
// still detached, which is why it has to ask the markup and not the live
// focus state.
describe(['tagdesktop'], 'Writer dialog frame grouping', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('a frame holding focusable widgets is announced as a group', function () {
		cy.then(() => {
			this.win.app.map.sendUnoCommand('.uno:ParagraphDialog');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		helper.processToIdle(this.win);

		cy.cGet('.ui-dialog[role="dialog"]').should(($dialog) => {
			const selector = this.win.JSDialog.FocusableSelector;
			// A real fieldset is already a group through its own element, so
			// only the div form of a group frame carries the role. And only
			// when it holds a widget: a frame of decorative images is
			// deliberately left ungrouped.
			const frames = Array.from(
				$dialog[0].querySelectorAll('div.ui-frame-container.ui-fieldset'));
			const withWidgets = frames.filter((frame) => {
				const content = frame.querySelector('.ui-expander-content');
				return content && content.querySelector(selector);
			});

			expect(withWidgets.length, 'frames holding a focusable widget')
				.to.be.greaterThan(0);

			// buildFrame reads the markup, so the grouping survives being
			// decided while the dialog is still detached. Asking the live
			// focus state instead loses it for every frame at once.
			const grouped = withWidgets
				.filter((frame) => frame.getAttribute('role') === 'group')
				.map((frame) => frame.id || '<no id>');
			expect(grouped, 'frames announced as a group').to.not.be.empty;
		});

		helper.typeIntoDocument('{esc}');
	});
});
