/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Shapes deck tooltips', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('impress/help_dialog.odp', false, false, undefined,
			'tooltips=true');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.cGet('#View-tab-label').click();
		cy.cGet('#View-container').should('be.visible');
		cy.cGet('#View-container [modelId="view-shapes-deck"] button').click();
		cy.cGet('#DefaultShapesPanel').should('exist');
		cy.cGet('#DefaultShapesPanel .ui-iconview-entry img').should('exist');
	});

	it('every shape offers the name of the shape it inserts', function () {
		cy.then(function () {
			const panel = win.document.querySelector('#DefaultShapesPanel');
			const images = Array.from(panel.querySelectorAll('.ui-iconview-entry img'));
			const named = images.filter(function (image) {
				const text = image.getAttribute('data-cooltip');
				return text && text.trim() !== '';
			});

			const titled = images.filter(function (image) {
				return image.hasAttribute('title');
			});

			expect(images, 'shapes drawn in the panel').to.not.be.empty;
			expect(named.length, 'shapes offering a tooltip').to.equal(images.length);
			expect(titled, 'shapes offering a second, native tooltip').to.be.empty;
		});
	});

	it('the pointer shows the tooltip of the shape it rests on', function () {
		let expected;

		cy.cGet('#BasicShapes .ui-iconview-entry img').first().then(function ($image) {
			expected = $image.attr('data-cooltip');
			expect(expected, 'the shape names itself').to.not.be.empty;
			cy.wrap($image).trigger('mouseenter');
		});

		cy.cGet('#cooltip').should('be.visible').and(function ($tooltip) {
			expect($tooltip.text().trim(), 'what the tooltip says').to.equal(expected);
		});

		cy.cGet('#BasicShapes .ui-iconview-entry img').first().trigger('mouseleave');
		cy.cGet('#cooltip').should('not.be.visible');
	});
});
