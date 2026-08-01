/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');

// The document holds three rectangles of the same size on one row. The left one
// starts at the left edge of the row, the right one ends at its right edge and
// the middle one is bunched up next to the left one. Spreading them evenly
// therefore moves the middle rectangle onto the centre of the slide.
describe(['tagdesktop'], 'Distribute objects', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/object_distribute.fodp');
		desktopHelper.switchUIToNotebookbar();
		// A wide viewport leaves the Shape tab room for every group, so the
		// Distribute buttons stay on the toolbar instead of collapsing into
		// an overflow dropdown.
		cy.viewport(1920, 1080);

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	const distributeButtons = [
		'shape-distribute-horz-left',
		'shape-distribute-horz-center',
		'shape-distribute-horz-distance',
		'shape-distribute-horz-right',
		'shape-distribute-vert-top',
		'shape-distribute-vert-center',
		'shape-distribute-vert-distance',
		'shape-distribute-vert-bottom',
	];

	// The toolbar item keeps the id from the notebookbar definition in its
	// modelid attribute, while its own id carries a uniquifying suffix.
	function distributeButton(id) {
		return cy.cGet('#Shape-container [modelid="' + id + '"]');
	}

	function selectAllObjects() {
		helper.typeIntoDocument('{ctrl}a');
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	}

	// Selecting objects already brings the contextual Shape tab to the front.
	// Clicking a tab that is already selected collapses the notebookbar, so the
	// tab is only clicked when some other tab is showing.
	function openShapeTab() {
		cy.cGet('#Shape-tab-label').should('be.visible').then(function($tab) {
			if (!$tab.hasClass('selected'))
				cy.wrap($tab).click();
		});
		cy.cGet('#Shape-tab-label').should('have.class', 'selected');
		cy.cGet('#Shape-container').should('be.visible');
	}

	it('Shape tab offers a Distribute button for every direction', function() {
		selectAllObjects();
		openShapeTab();

		cy.cGet('#shape-distribute').should('exist');
		distributeButtons.forEach(function(id) {
			distributeButton(id).should('exist').and('not.have.attr', 'disabled');
		});
	});

	it('Distribute is refused for a single object', function() {
		selectAllObjects();

		// Note where the leftmost rectangle is on screen. It covers the first
		// seventh of the row of three, so a point a fourteenth of the way into
		// the row is on it.
		const leftRectangle = {};
		cy.cGet('#test-div-shapeHandlesSection').then(function($handles) {
			const row = $handles[0].getBoundingClientRect();
			leftRectangle.x = row.left + row.width / 14;
			leftRectangle.y = row.top + row.height / 2;
		});

		// Selecting that one rectangle on its own leaves too few objects to
		// spread out.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		cy.then(function() {
			cy.cGet('body').click(leftRectangle.x, leftRectangle.y);
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		openShapeTab();
		distributeButtons.forEach(function(id) {
			distributeButton(id).should('have.attr', 'disabled');
		});
	});

	it('Even horizontal spacing moves the middle object into the middle', function() {
		// The middle of the slide starts out empty, so a click there selects
		// nothing.
		impressHelper.clickCenterOfSlide();
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');

		selectAllObjects();
		openShapeTab();

		distributeButton('shape-distribute-horz-distance')
			.should('not.have.attr', 'disabled');
		distributeButton('shape-distribute-horz-distance').find('button').click();
		cy.then(() => {
			return helper.processToIdle(this.win);
		});

		// The middle rectangle now sits halfway between the outer two, which is
		// the middle of the slide, so a click there picks it up.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		impressHelper.clickCenterOfSlide();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	});
});
