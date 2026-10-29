/* global describe it cy before require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['taga11yenabled'], 'Screen reading in a drawing document', { testIsolation: false }, function () {
	var win;

	before(function () {
		helper.setupAndLoadDocument('draw/to_curve.fodg');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.cGet('div.leaflet-layer').should('exist');
	});

	it('keeps the plain text input when screen reading is turned on', function () {
		cy.then(function () {
			expect(win.app.map._textInput.hasAccessibilitySupport()).to.eq(false);

			win.app.map.setAccessibilityState(true);
			expect(win.prefs.getBoolean('accessibilityState')).to.eq(true);
			expect(win.app.map._textInput.hasAccessibilitySupport()).to.eq(false);

			win.app.map.lockAccessibilityOn();
			expect(win.app.map._lockAccessibilityOn).to.eq(true);
			expect(win.app.map._textInput.hasAccessibilitySupport()).to.eq(false);
		});
	});

	it('does not offer the Screen Reading button in the Help tab', function () {
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.selectNotebookbarTab('Help');
		cy.cGet('#togglea11ystate').should('not.exist');
	});
});
