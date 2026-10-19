/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Stylesview Iconview Tests', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('writer/styles.odt', {
		notebookbar: true,
		// wide enough for the styles view to stay in the notebookbar, unfolded, and narrow enough
		// for it to show only some of the styles, so it scrolls
		viewport: [1600, 720],
	});

	// check expand button visibility and click on it
	const openExpander = () => {
		cy.cGet('#stylesview-iconview-list-expand-button').should('exist').should('be.visible');
		cy.cGet('#stylesview-iconview-list-expand-button').click();
		desktopHelper.getDropdown('stylesview-iconview-list').should('exist');
		cy.cGet('.jsdialog #stylesview').should('exist').should('be.visible');
	}

	beforeEach(function() {
		// The styles view these tests read is the one in the notebookbar, so the
		// sidebar dock stays out of the way. Opening the styles deck brings it
		// back, and so does the notebookbar being built again after a view mode
		// switch, which is why the state is read rather than toggled.
		desktopHelper.ensureSidebarHidden();

		cy.cGet('.notebookbar #stylesview').should('exist').should('be.visible').should('not.be.empty');
	});

	// The styles come from core, the layout the client generates for the
	// notebookbar has the styles view empty. Anything that builds the
	// notebookbar again after load, for example the extensions becoming known,
	// goes through refresh(), and the styles have to be there afterwards.
	it('Styles are still listed after the notebookbar is built again', function() {
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry').should('have.length.greaterThan', 1);

		cy.cGet('.notebookbar #stylesview .ui-iconview-entry').then(function($entries) {
			const entryCountBefore = $entries.length;

			cy.getFrameWindow().then(function(win) {
				win.app.map.uiManager.notebookbar.impl.refresh();
			});

			cy.cGet('.notebookbar #stylesview .ui-iconview-entry')
				.should('have.length', entryCountBefore);
			cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should('be.visible');
		});
	});

	it('Scroll Up/Down Buttons', function() {
		cy.cGet('#stylesview-iconview-list-scroll-up').should('exist').should('be.visible');
		cy.cGet('#stylesview-iconview-list-scroll-down').should('exist').should('be.visible');
		cy.cGet('#stylesview_0').should('exist').scrollIntoView().should('be.visible');

		cy.cGet('#stylesview-iconview-list-scroll-down').click();
		cy.cGet('#stylesview_0').should('exist').should('not.be.visible');
		cy.cGet('#stylesview-iconview-list-scroll-up').click();
		cy.cGet('#stylesview_0').should('exist').scrollIntoView().should('be.visible');
	});

	it('Expander Button', function() {
		cy.cGet('.jsdialog #stylesview_9').should('not.exist');
		openExpander();
		cy.cGet('.jsdialog #stylesview_9').should('exist').should('be.visible');
	});

	// Switching to viewing mode tears down the notebookbar (which marks it
	// de-initialized in core). Switching back to editing must restore it,
	// otherwise core stops answering the stylesview preview render requests and
	// the entries fall back to plain text names. See the de-initialize-on-remove
	// fix in Control.Notebookbar. The expander is opened only after the switch so
	// its entries are rendered fresh (and so expose stale core state).
	it('Previews still render after view -> edit mode switch', function() {
		// Baseline: a style preview is rendered in the notebookbar frame.
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should('be.visible');

		// Switch to viewing mode.
		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('readonly');
		});
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Viewing');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });

		// Switch back to editing mode.
		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('edit');
		});
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Editing');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });

		// Open the expander: its entries were not rendered before the switch, so
		// they are requested fresh from core now. Scroll every entry into view to
		// trigger its on-demand render, then require that none stays a plain text
		// placeholder - a rendered preview always has an <img>. If core ignores
		// any of these requests the entry has no image and this fails.
		openExpander();
		cy.cGet('.jsdialog #stylesview .ui-iconview-entry').should('have.length.greaterThan', 1);
		cy.cGet('.jsdialog #stylesview .ui-iconview-entry').each(function($entry) {
			cy.wrap($entry).scrollIntoView();
			cy.wrap($entry).find('img').should('exist');
		});
	});

	// Each switch to viewing mode and back builds the notebookbar again. The
	// previews the engine drew for the earlier build stay in a cache, and from
	// the second switch on the entries are built from that cache instead of
	// being requested again. An entry built that way has to look like a freshly
	// rendered one: the preview at its device-independent size, which keeps it
	// inside its cell on a high-density display, and the style name as its
	// tooltip.
	it('Previews from the cache keep their size and tooltip after a second view -> edit switch', function() {
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should('be.visible');

		for (var round = 0; round < 2; round++) {
			cy.getFrameWindow().its('app').then(function(app) {
				app.map.setPermission('readonly');
			});
			cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Viewing');
			cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });

			cy.getFrameWindow().its('app').then(function(app) {
				app.map.setPermission('edit');
			});
			cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Editing');
			cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		}
		cy.getFrameWindow().then(function(win) { return helper.waitForOnDemandRenders(win); });

		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should('have.length.greaterThan', 1);
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should(function($images) {
			var ratio = $images[0].ownerDocument.defaultView.devicePixelRatio;
			$images.each(function(index, img) {
				expect(img.naturalWidth, 'preview ' + index + ' is loaded').to.be.greaterThan(0);
				expect(img.getBoundingClientRect().width, 'width of preview ' + index)
					.to.be.closeTo(img.naturalWidth / ratio, 1);
				expect(img.hasAttribute('data-cooltip'), 'preview ' + index + ' has a tooltip').to.equal(true);
			});
		});
	});

	it('Resize', function() {
		// the dropdown should close on resize
		openExpander();
		cy.cGet('#stylesview-iconview-list-dropdown').should('exist').should('be.visible');
		cy.viewport(650, 1080);
		cy.cGet('#stylesview-iconview-list-dropdown').should('not.exist');

		// re-open after the resize, the narrow window folds the styles view into a button
		cy.cGet('#overflow-button-home-stylesview-group').should('be.visible').click();
		desktopHelper.getDropdown('stylesview-iconview-list').should('exist');
		cy.cGet('.jsdialog #stylesview').should('exist').should('be.visible');

		// with reduced height:
		// - only a few rows should be visible.
		// - the open styles sidebar button should be visible.
		cy.viewport(650, 454);
		cy.cGet('.jsdialog #stylesview_4').should('exist').should('be.visible');
		cy.cGet('#format-style-list-dialog-button').should('exist').should('be.visible');
	});

	// This one comes last because it leaves the styles deck showing in the
	// sidebar. From there the button that hides the dock switches the deck
	// instead, so the tests above get the sidebar as the file started.
	it('Open Styles Sidebar Button', function() {
		openExpander();

		// open sidebar
		cy.cGet('#format-style-list-dialog-button').should('exist').should('be.visible');
		cy.cGet('#format-style-list-dialog-button').click();

		// close dropdown on button click
		desktopHelper.getDropdown('stylesview').should('not.exist');

		cy.cGet('#StyleListDeck').should('exist').should('be.visible');

		// The panel container is laid out by the stylesheet as a flex column,
		// not forced into a grid by an inline style.
		cy.cGet('#TemplatePanel').should('have.css', 'display', 'flex');
	});
});
