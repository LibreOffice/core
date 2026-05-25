/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Regression for the OverflowManager initial-measurement bug: in compact
// mode the first onRefresh used to run before the toolbar's parent had
// been laid out, scrollWidth was 0, hasOverflow() took its "unknown
// width -> assume overflow" branch and every group folded. The fix
// (one-shot ResizeObserver + hasOverflow returning false on
// requiredWidth==0) is what these tests guard.
describe(['tagdesktop'], 'Compact toolbar overflow groups', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('starts with every overflow group unfolded at a wide viewport', function() {
		helper.processToIdle(this.win);

		// There must be at least one overflow group to validate.
		cy.cGet('#toolbar-up .ui-overflow-group').should('have.length.gte', 1);

		cy.cGet('#toolbar-up .ui-overflow-group').each(($el) => {
			cy.wrap($el)
				.should('have.class', 'ui-overflow-group-container-with-label');
			cy.wrap($el).find('[id^="overflow-button-"]')
				.should('have.css', 'display', 'none');
		});
	});

	it('folds groups at a narrow viewport (observer drives real fold)', function() {
		// Sanity: after the constructor-time observer fires onRefresh with
		// a measurable width, a viewport too small for the toolbar must
		// produce at least one folded group.
		cy.viewport(640, 1080);
		helper.processToIdle(this.win);
		cy.cGet('#toolbar-up [id^="overflow-button-"]:visible')
			.should('have.length.gte', 1);
	});
});
