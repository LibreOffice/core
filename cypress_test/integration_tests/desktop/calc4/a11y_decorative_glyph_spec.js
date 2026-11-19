/* global expect it cy require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');
const sweep = require('../../common/a11y_decorative_glyph');

sweep.sweepDecorativeGlyphs('Calc', 'calc/switch.ods', function (app) {
	function openStandardFilter() {
		cy.then(function () {
			app.win().app.map.sendUnoCommand('.uno:DataFilterStandardFilter');
		});
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () { return helper.processToIdle(app.win()); });
	}

	// cypress-tags drops an untagged test that is not inside a tagged describe of
	// this file, so the test carries the tag of the describe it joins.
	it(['tagdesktop'], 'and the Standard Filter expander names itself with a word', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		openStandardFilter();

		cy.then(function () { return a11yHelper.getAXNodesWithin('.ui-dialog'); })
			.then(function (nodes) { return a11yHelper.withDomNames(nodes); })
			.then(function (nodes) {
				expect(app.symbolOnly(nodes).join(', '),
					'controls named only with symbols in the dialog').to.be.empty;
			});
	});
});
