/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Table Design tab', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/table_operation.odp');
		cy.viewport(1920, 1080);

		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('Applies a style clicked in the gallery', function() {
		impressHelper.selectTableInTheCenter(this.win);

		cy.getFrameWindow().then(function(win) {
			cy.wrap(cy.stub(win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');
		});

		cy.cGet('#TableDesign-tab-label').click();
		cy.cGet('#TableDesign-tab-label').should('have.class', 'selected');

		// The gallery is populated asynchronously from the engine's style
		// list, so wait for the first swatch to render before clicking it.
		cy.cGet('#table-design-styles_0').should('be.visible').click();

		cy.get('@sendUnoCommand').should(function(sendUnoCommand) {
			var call = sendUnoCommand.getCalls().find(function(c) {
				return c.args[0] === '.uno:TableStyle';
			});
			expect(call, '.uno:TableStyle was sent').to.not.be.undefined;
			expect(call.args[1].TableStyle.type).to.equal('string');
			expect(call.args[1].TableStyle.value).to.be.a('string').and.not.empty;
		});
	});

	it('The gallery is announced with the name the tab asks for', function() {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		impressHelper.selectTableInTheCenter(this.win);

		cy.cGet('#TableDesign-tab-label').click();
		cy.cGet('#TableDesign-tab-label').should('have.class', 'selected');
		cy.cGet('#table-design-styles_0').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			const wanted = win.app.impressTableStyles.generateTableStylesJSON().aria.label;
			expect(wanted, 'the name the tab asks for').to.not.be.empty;

			a11yHelper.getAXNodes().then(function(nodes) {
				const names = nodes.filter(function(node) {
					return node.role === 'radiogroup' && !node.ignored;
				}).map(function(node) {
					return node.name;
				});

				expect(names, 'accessible names of the exposed radiogroups')
					.to.include(wanted);
			});
		});
	});
});
