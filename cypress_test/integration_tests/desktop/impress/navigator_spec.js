/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe.skip(['tagdesktop'], 'Scroll through document, insert/delete items', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/navigator.odp');

		desktopHelper.selectZoomLevel('100');
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-navigator').click();
	});

	function checkIfItemNotExist(itemName) {
		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', itemName).should('not.exist');
	}

	function checkIfItemSelectedAndVisible(itemName) {
		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', itemName).should('be.visible');
		cy.cGet('#tree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-notexpandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text',itemName);
	}

	function checkIfItemExSelectedAndVisible(itemName) {
		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', itemName).should('be.visible');
		cy.cGet('#tree').find('.jsdialog.sidebar.ui-treeview-entry.ui-treeview-expandable.selected').find('.jsdialog.sidebar.ui-treeview-cell-text').should('have.text',itemName);
	}

	it('Jump to element. Navigator -> Document', function() {
		// Click items in navigator, and check if it goes to the right slide.
		// items should be visible (scrolled right), but that is not tested yet
		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 2 (Text Frame \'Text1\')').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 2 of 4');

		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Object 2').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 4 of 4');

		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 1 (Title text \'Title1\')').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 1 of 4');

		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 5 (Image)').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 4 of 4');

		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 4 (Text Frame \'T3\')').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 3 of 4');

		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Shape 1 (SVG)').dblclick();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 4 of 4');
	});

	it('Jump to element. Document -> Navigator', function() {
		// Click some items in document, and check if Navigator will jump to it
		// Risky: it clicks x,y coordinates, if scroll or layout change it may need to be changed
		cy.cGet('#toolbar-down #nextpage').click();
		cy.cGet('body').click(600,360);
		checkIfItemSelectedAndVisible('Shape 6 (Text Frame \'Text3\')');

		cy.cGet('#toolbar-down #nextpage').click();
		cy.cGet('#toolbar-down #nextpage').click();
		cy.cGet('body').click(370,270);
		checkIfItemSelectedAndVisible('Shape 1 (SVG)');

		cy.cGet('#toolbar-down #prevpage').click();
		cy.cGet('body').click(355,435);
		// T2 is part of a group. The group will be selected in Navigator.
		checkIfItemExSelectedAndVisible('Shape 3 (Group object)');

		cy.cGet('#toolbar-down #nextpage').click();
		cy.cGet('body').click(500,520);	//Object 3
		checkIfItemSelectedAndVisible('Object 3');

		cy.cGet('#toolbar-down #prevpage').click();
		cy.cGet('#toolbar-down #prevpage').click();
		cy.cGet('#toolbar-down #prevpage').click();
		cy.cGet('body').click(630,330);	//Some text
		checkIfItemSelectedAndVisible('Shape 2 (Text Frame \'Some text\')');
	});

	it('Insert/delete updated on Navigator', function() {
		//create a new sheet
		cy.cGet('#presentation-toolbar #insertpage').click();
		cy.cGet('#tree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Slide 5').should('exist');
		cy.cGet('#toolbar-up > .ui-scroll-right').click();

		//Insert Shape
		cy.cGet('#insertshapes').click();
		cy.cGet('.col.w2ui-icon.symbolshapes').click();
		checkIfItemSelectedAndVisible('Shape 3 (Shape)');
		//delete
		helper.typeIntoDocument('{del}');
		checkIfItemNotExist('Shape 3 (Shape)');

		//Insert Chart
		cy.cGet('#insertobjectchart').click();
		checkIfItemSelectedAndVisible('Object 4');
		//delete
		helper.typeIntoDocument('{del}');
		checkIfItemNotExist('Object 4');

		//Insert Table
		cy.cGet('#menu-table').click();
		cy.cGet('body').contains('Insert Table...').click();
		cy.cGet('.lokdialog_canvas').click();
		helper.typeIntoDocument('{shift}{enter}');
		checkIfItemSelectedAndVisible('Shape 2 (Table)');
		// Table is inserted with the markers shown
		cy.cGet('.leaflet-marker-icon.table-column-resize-marker').should('exist');
		cy.cGet('path.leaflet-interactive').rightclick({force:true});
		cy.cGet('body').contains('.ui-combobox-entry', 'Delete').click();
		checkIfItemNotExist('Shape 2 (Table)');

		//Insert Fontwork
		cy.cGet('#menu-insert').click();
		cy.cGet('body').contains('a','Fontwork...').click();
		cy.cGet('#ok').click();
		checkIfItemSelectedAndVisible('Simple');
		//delete
		helper.typeIntoDocument('{del}');
		checkIfItemNotExist('Simple');
	});
});
