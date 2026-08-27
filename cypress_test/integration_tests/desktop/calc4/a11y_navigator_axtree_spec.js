/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Navigator accessibility tree', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/top_toolbar.ods');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:Navigator');
			return helper.processToIdle(win);
		});
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
	});

	it('an expandable row announces its text, not the chevron glyph', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		cy.cGet('.ui-treeview-entry.ui-treeview-expandable').first().then(function ($row) {
			expect($row[0].querySelector('.ui-treeview-expander'),
				'the row carries an expander').to.not.equal(null);
			$row[0].focus();

			a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node.name.trim(), 'accessible name of the expandable row')
					.to.equal($row[0].textContent.trim());
			});
		});
	});
});
