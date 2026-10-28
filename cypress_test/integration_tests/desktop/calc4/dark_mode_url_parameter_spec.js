/* global describe it cy require expect */

var helper = require('../../common/helper');

// The darkTheme URL parameter accepts 'true' and 'false'. Any other value is ignored, and the
// parameter changes only the dark theme default, so the other UI defaults stay in place. The
// UITheme query is a debug.html aid that stands in for an integrator's ui_defaults, with
// SavedUIState=false so that the default decides the theme.
describe(['tagdesktop'], 'darkTheme URL parameter', function() {

	function loadWithQuery(query) {
		helper.setupAndLoadDocument('calc/cell_cursor.ods', false, false, undefined, query);
	}

	function assertThemeIs(theme) {
		cy.cframe().find('html').should('have.attr', 'data-theme', theme);
	}

	function assertZoomDefaultsKept() {
		// coolwsd.xml turns smart_zoom on by default, which arrives as a UI default.
		cy.getFrameWindow().then(function(win) {
			expect(win.uiDefaults.smartZoom).to.equal('true');
		});
	}

	it('darkTheme=true opens dark and keeps the other UI defaults', function() {
		loadWithQuery('UITheme=light&darkTheme=true');
		assertThemeIs('dark');
		assertZoomDefaultsKept();
	});

	it('darkTheme=false opens light and keeps the other UI defaults', function() {
		loadWithQuery('UITheme=dark&darkTheme=false');
		assertThemeIs('light');
		assertZoomDefaultsKept();
	});

	it('Any other darkTheme value leaves the theme default unchanged', function() {
		loadWithQuery('UITheme=light&darkTheme=0');
		assertThemeIs('light');
		assertZoomDefaultsKept();
	});
});
