/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Shapes deck icon theme', { testIsolation: false }, function() {
	let win;

	before(function() {
		helper.setupAndLoadDocument('impress/help_dialog.odp');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function(frameWindow) {
			win = frameWindow;
		});

		cy.cGet('#View-tab-label').then(function($tab) {
			if (!$tab.hasClass('selected'))
				cy.wrap($tab).click();
		});
		cy.cGet('#View-container').should('be.visible');
		cy.cGet('#View-container [modelId="view-shapes-deck"] button').click();
		cy.then(function() {
			helper.processToIdle(win);
		});
		cy.cGet('#DefaultShapesPanel').should('exist');
	});

	function themeIcons() {
		return cy.getFrameWindow().then(function(w) {
			const images = w.document.querySelectorAll('#DefaultShapesPanel .ui-iconview-entry img');
			return Array.from(images)
				.map(function(image) { return image.getAttribute('src') || ''; })
				.filter(function(source) { return source.indexOf('/images/') !== -1; });
		});
	}

	function fromDarkSet(sources) {
		return sources.filter(function(source) { return source.indexOf('/images/dark/') !== -1; });
	}

	function setDarkMode(dark) {
		cy.getFrameWindow().then(function(w) {
			if (w.prefs.getBoolean('darkTheme') !== dark)
				w.app.map.uiManager.toggleDarkMode();
		});
		cy.cGet('html').should(dark ? 'have.attr' : 'not.have.attr', 'data-theme', 'dark');
		cy.then(function() {
			helper.processToIdle(win);
		});
	}

	it('the shape icons come from the set the theme asks for', function() {
		setDarkMode(false);
		themeIcons().then(function(sources) {
			expect(sources.length, 'shapes drawn from the icon theme').to.be.greaterThan(0);
			expect(fromDarkSet(sources).join(', '), 'dark icons while the theme is light').to.be.empty;
		});

		setDarkMode(true);
		themeIcons().then(function(sources) {
			expect(sources.length, 'shapes drawn from the icon theme').to.be.greaterThan(0);
			const light = sources.filter(function(source) { return source.indexOf('/images/dark/') === -1; });
			expect(light.join(', '), 'light icons while the theme is dark').to.be.empty;
		});

		setDarkMode(false);
		themeIcons().then(function(sources) {
			expect(fromDarkSet(sources).join(', '), 'dark icons after switching back').to.be.empty;
		});
	});
});
