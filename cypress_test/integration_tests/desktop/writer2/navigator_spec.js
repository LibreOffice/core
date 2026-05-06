/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Scroll through document, modify heading', function() {

	function expandSection(name) {
		cy.cGet('#contenttree')
			.contains('.jsdialog.sidebar.ui-treeview-cell-text', name)
			.parent() // .ui-treeview-cell
			.parent() // div - column
			.parent() // .ui-treeview-entry
			.find('.ui-treeview-expander-column')
			.click();
	}

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/navigator.odt');
		cy.cGet('#navigator-floating-icon').click(); // Ensure it's open
	});

	it('Navigator visual test', function() {
		cy.wait(500); // wait to make fully rendered
		cy.cGet('#navigator-dock-wrapper').scrollTo(0,0,{ ensureScrollable: false });
		cy.wait(500); // wait for animations
		cy.cGet('#navigator-dock-wrapper').compareSnapshot('navigator_writer', 0.065);
	});

	it('Jump to element. Navigator -> Document', function() {
		// Expand Tables, Frames, Images
		// Note click()/dblclick() scrolls the contenttree even if it would be not needed to click
		expandSection('Tables');
		expandSection('Frames');
		expandSection('Images');

		cy.wait(500);

		//Scroll back to Top
		cy.cGet('#navigator-dock-wrapper').scrollTo(0,0, { ensureScrollable: false });
		cy.wait(500);

		// Doubleclick several items, and check if the document is scrolled to the right page
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Feedback').dblclick();
		desktopHelper.assertVisiblePage(2, 2, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Text').dblclick();
		desktopHelper.assertVisiblePage(5, 6, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Replacing').dblclick();
		desktopHelper.assertVisiblePage(7, 7, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Table15').dblclick();
		desktopHelper.assertVisiblePage(2, 2, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Frame39').dblclick();
		desktopHelper.assertVisiblePage(4, 4, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Frame27').dblclick();
		desktopHelper.assertVisiblePage(6, 6, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'graphics3').dblclick();
		desktopHelper.assertVisiblePage(1, 1, 8);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'graphics10').dblclick();
		desktopHelper.assertVisiblePage(5, 5, 8);
	});

	it('Jump to element even when cursor not visible', function() {
		// Expand Tables, Frames, Images
		// Note click()/dblclick() scrolls the contenttree even if it would be not needed to click
		expandSection('Tables');
		expandSection('Frames');
		expandSection('Images');

		//Scroll back to Top
		cy.cGet('#navigator-dock-wrapper').scrollTo(0,0, { ensureScrollable: false });

		// Doubleclick several items, and check if the document is scrolled to the right page
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Feedback').dblclick();
		desktopHelper.assertVisiblePage(2, 2, 8);

		desktopHelper.assertScrollbarPosition('vertical', 45, 75);

		// Scroll document to the top so cursor is no longer visible, that turns following off
		desktopHelper.scrollWriterDocumentToTop();
		desktopHelper.updateFollowingUsers();

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Text').dblclick();
		desktopHelper.assertVisiblePage(5, 6, 8);

		desktopHelper.assertScrollbarPosition('vertical', 225, 255);

		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Replacing').dblclick();
		desktopHelper.assertVisiblePage(7, 7, 8);

		desktopHelper.assertScrollbarPosition('vertical', 325, 360);
	});

	it('Jump to image when cursor not visible', function() {
		// Regression test: when the text cursor is hidden, double-clicking
		// an image in the navigator must still scroll the viewport to it.
		// assertVisiblePage alone is not sufficient as #StatePageNumber
		// reflects the selection's page (which core updates regardless of
		// whether the viewport scrolls), so check the actual scrollbar.
		expandSection('Images');

		cy.cGet('#navigator-dock-wrapper').scrollTo(0,0, { ensureScrollable: false });

		// Jump to a heading so a cursor exists at a known position.
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Feedback').dblclick();
		desktopHelper.assertVisiblePage(2, 2, 8);
		desktopHelper.assertScrollbarPosition('vertical', 45, 75);

		// Scroll document to the top so the text cursor is no longer visible.
		desktopHelper.scrollWriterDocumentToTop();
		desktopHelper.updateFollowingUsers();
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);

		// Double-click an image on a different page (graphics10 is on page 5).
		// Without the fix, _onUpdateCursor returns early because the text
		// cursor is invisible, so the viewport never scrolls and the
		// scrollbar stays near 0.
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'graphics10').dblclick();
		desktopHelper.assertVisiblePage(5, 5, 8);
		desktopHelper.assertScrollbarPosition('vertical', 100, 400);
	});

	it.skip('Jump to element. Document -> Navigator', function() {
		// Move the cursor into elements in Document, and check
		// if navigator contentTree scroll to the element and select that,
		// and if necessary expand contentypes, to make the element visible.

		// Move into a hyperlink
		desktopHelper.pressKey(2, 'pagedown');
		desktopHelper.pressKey(3, 'downArrow');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'http://www.gnu.org/licenses/gpl.html').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','http://www.gnu.org/licenses/gpl.html');

		// Move into a Table
		desktopHelper.pressKey(2, 'pagedown');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Table15').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','Table15');

		// Move into a Headings
		// Previous headings was in a section, and navigator selected the section instead.
		desktopHelper.pressKey(3, 'pagedown');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Introduction').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','Introduction');
		// hyperlinks should be not visible, as they are so far from Introduction
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'http://www.gnu.org/licenses/gpl.html').should('be.not.visible');

		desktopHelper.pressKey(1, 'pagedown');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Leading zeroes').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','Leading zeroes');

		// Risky: blind click into a big image, because cursor avoid images.
		cy.cGet('#document-container').click(450,450);
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'graphics36').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','graphics36');

		desktopHelper.pressKey(2, 'pagedown');
		desktopHelper.pressKey(1, 'downArrow');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Table14').should('be.visible');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','Table14');
	});

	it.skip('Rewrite Heading', function() {
		// Write into a heading, and check if it changed in navigator contentTree.
		desktopHelper.pressKey(7, 'pagedown');
		desktopHelper.pressKey(1, 'A');
		cy.cGet('#contenttree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text','IntroAduction');
	});
});
