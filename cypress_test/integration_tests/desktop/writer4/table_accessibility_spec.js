/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
// var desktopHelper = require('../../common/desktop_helper');
var ceHelper = require('../../common/contenteditable_helper');

describe(['taga11yenabled'], 'Table accessibility', function() {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/table_accessibility.odt');
		cy.cGet('div.clipboard').as('clipboard');
	});

	function checkCellDescription(expectedDescription) {
		cy.wait(400);
		cy.get('@clipboard').should('have.attr', 'aria-description', expectedDescription);
	}

	it('Table content and cell address on navigation', function () {
		ceHelper.checkPlainContent('Paragraph above.');
		ceHelper.moveCaret('down');
		checkCellDescription('Table with 4 rows and 3 columns. Row 1. Column 1. ');
		ceHelper.checkPlainContent('Item 1.1');
		ceHelper.moveCaret('down');
		checkCellDescription('Row 2. ');
		ceHelper.checkPlainContent('Item 2.1');
		ceHelper.moveCaret('down');
		checkCellDescription('Row 3 through 4. ');
		ceHelper.checkPlainContent('Item 3.1');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Row 3. Column 2 through 3. ');
		ceHelper.checkPlainContent('Item 3.2');
		ceHelper.moveCaret('down');
		checkCellDescription('Row 4. ');
		ceHelper.checkPlainContent('Item 4.2');
		ceHelper.moveCaret('down');
		checkCellDescription('Out of table. ');
		ceHelper.checkPlainContent('Paragraph below.');
		ceHelper.moveCaret('up');
		checkCellDescription('Table with 4 rows and 3 columns. Row 3 through 4. Column 1. ');
		ceHelper.checkPlainContent('Item 3.1');
	});

	it('Nested table content and cell address on navigation', function () {
		ceHelper.checkPlainContent('Paragraph above.');
		ceHelper.moveCaret('down');
		checkCellDescription('Table with 4 rows and 3 columns. Row 1. Column 1. ');
		ceHelper.checkPlainContent('Item 1.1');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Column 2. ');
		ceHelper.checkPlainContent('Item 1.2');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Column 3. ');
		ceHelper.checkPlainContent('Item 1.3');
		ceHelper.moveCaret('down');
		checkCellDescription('Table with 2 rows and 2 columns. Row 1. Column 2. ');
		ceHelper.checkPlainContent('Nested Cell 1.2');
		ceHelper.moveCaret('left');
		checkCellDescription('Column 1. ');
		ceHelper.checkPlainContent('Nested Cell 1.1');
		ceHelper.moveCaret('home');
		ceHelper.moveCaret('left');
		checkCellDescription('Out of table. Row 2. Column 2. ');
		ceHelper.checkPlainContent('Item 2.2');
		ceHelper.moveCaret('down');
		checkCellDescription('Row 3. Column 2 through 3. ');
		ceHelper.checkPlainContent('Item 3.2');
		ceHelper.moveCaret('up');
		checkCellDescription('Row 2. Column 3. ');
		ceHelper.checkPlainContent('');
		ceHelper.moveCaret('up');
		checkCellDescription('Table with 2 rows and 2 columns. Row 2. Column 1. ');
		ceHelper.checkPlainContent('Nested Cell 2.1');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Column 2. ');
		ceHelper.checkPlainContent('Nested Cell 2.2');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Out of table. Row 2. Column 3. ');
		ceHelper.checkPlainContent('');
		ceHelper.moveCaret('end');
		ceHelper.moveCaret('right');
		checkCellDescription('Row 3 through 4. Column 1. ');
		ceHelper.checkPlainContent('Item 3.1');
	});
});
