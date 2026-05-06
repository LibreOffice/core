/* global describe it cy require beforeEach */
var helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Delete Objects', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/delete_objects.ods');
		desktopHelper.switchUIToCompact();
	});

	it('Delete Text', function() {
		helper.setDummyClipboardForCopy();

		helper.typeIntoDocument('text');
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('text');
		helper.typeIntoDocument('{del}');
		helper.typeIntoDocument('{ctrl}a');
		helper.textSelectionShouldNotExist();
	});

	it('Delete Shapes', function() {
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIcon('BasicShapes').click();

		cy.cGet('.col.w2ui-icon.symbolshapes').click();

		cy.cGet('#test-div-shapeHandlesSection')
			.should('exist');

		//delete
		helper.typeIntoDocument('{del}');

		cy.cGet('#test-div-shapeHandlesSection')
			.should('not.exist');
	});

	it('Delete Chart' , function() {
		// Insert chart button not visible yet so click on the overflow button.
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		// Click on insert chart button.
		desktopHelper.getCompactIcon('InsertObjectChart').click();

		// Click on the ok button of chart jsdialog.
		cy.cGet('#CHART2_HID_SCH_WIZARD_ROADMAP').should('exist');
		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();
		cy.cGet('#CHART2_HID_SCH_WIZARD_ROADMAP').should('not.exist');

		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		// delete
		helper.typeIntoDocument('{del}');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	});
});
