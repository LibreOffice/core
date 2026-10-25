/* global describe it cy require beforeEach expect Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Delete Objects', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('impress/delete_objects.odp', {
		setup: function() {
			desktopHelper.switchUIToCompact();

			cy.getFrameWindow().then(function(win) {
				helper.processToIdle(win);
			});
		},
	});

	beforeEach(function() {
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('Delete Text', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#document-container').dblclick('center');
		cy.cGet('#document-container svg g').should('exist');
		helper.processToIdle(this.win);
		helper.typeIntoDocument('text');
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('text');
		helper.typeIntoDocument('{del}');
		helper.typeIntoDocument('{ctrl}a');
		helper.textSelectionShouldNotExist();
	});

	it('Delete Text From Second Page', function() {
		// Insert second page.
		cy.cGet('#insertpage-button').click();

		// Check / wait for the inserted page.
		cy.cGet('#preview-img-part-1').should('exist');
		helper.processToIdle(this.win);

		// Activate the inserted page.
		cy.cGet('#preview-img-part-1').click();
		helper.processToIdle(this.win);

		// Click on canvas to activate the document.
		cy.cGet('#document-canvas').click(100, 100);
		helper.processToIdle(this.win);

		// Press delete button. If the document is correctly activated, the second page shouldn't be deleted.
		helper.typeIntoDocument('{del}');

		// Core reports the new part count before it reports idle, so the count read after
		// idle already includes a slide deletion if the key deleted one. The preview strip
		// drops a deleted slide seconds later, so the test reads the count.
		helper.processToIdle(this.win);
		cy.getFrameWindow().its('app.map._docLayer._parts').should('equal', 2);
		cy.cGet('#preview-img-part-1').should('exist');
	});

	it('Delete Shapes', function() {
		//insert
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIconArrow('BasicShapes').click();
		cy.cGet('.col.w2ui-icon.symbolshapes').should($el => { expect(Cypress.dom.isDetached($el)).to.eq(false); }).click();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.processToIdle(this.win);

		//delete
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it('Delete Chart' , function() {
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		//insert
		desktopHelper.getCompactIcon('InsertObjectChart').click();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.processToIdle(this.win);

		//delete
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it('Delete Fontwork', function() {
		cy.cGet('#menu-insert').click();
		cy.cGet('body').contains('a','Fontwork...').click();
		cy.cGet('#ok').click();
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.processToIdle(this.win);

		//delete
		helper.typeIntoDocument('{del}');

		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});

	it('Delete Shape In Master View', function() {
		var win = this.win;

		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SlideMasterPage');
		});
		helper.processToIdle(win);

		// The insert-shapes toolbar item is not offered in master view, so
		// insert through the same uno command the toolbar would send.
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SymbolShapes.smiley');
		});
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.processToIdle(win);

		//delete - master view used to swallow the key, leaving the shape behind
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');

		// leave the document in normal view for whatever runs after this
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:CloseMasterView');
		});
		helper.processToIdle(win);
	});

	// This one comes last because it zooms out to 50 percent, which is not part
	// of the document and stays for whatever runs after it.
	it('Delete Table',function() {
		desktopHelper.selectZoomLevel('50', false);

		cy.cGet('#menu-table').click();
		cy.cGet('body').contains('Insert Table...').click();

		// Insert Table is now a common JSDialog rather than a lokdialog
		// canvas, accept the default table via its OK button.
		cy.cGet('#NewTableDialog').should('exist');
		helper.processToIdle(this.win);
		cy.cGet('#NewTableDialog #ok-button').click();
		helper.processToIdle(this.win);

		// Table is inserted with the markers shown
		cy.cGet('.table-column-resize-marker').should('exist');
		cy.cGet('#test-div-shapeHandlesSection').then(function(element) {
			const x = element[0].getBoundingClientRect().left;
			const y = element[0].getBoundingClientRect().top;

			cy.cGet('body').rightclick(x + 20, y + 20);
		});

		helper.getContextMenuItem('Delete').click();
		cy.cGet('.table-column-resize-marker').should('not.exist');
	});
});
