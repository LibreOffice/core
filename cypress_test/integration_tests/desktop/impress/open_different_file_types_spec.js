/* global describe it cy require expect */
var helper = require('../../common/helper');
const { selectZoomLevel } = require('../../common/desktop_helper');
var desktopHelper = require('../../common/desktop_helper');
// const { selectTextShapeInTheCenter } = require('../../common/impress_helper');

describe.skip(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Open different file types', function() {

	function before(filePath) {
		helper.setupAndLoadDocument(filePath);

		selectZoomLevel('50');

		cy.cGet('#modifypage').click();
	}

	function assertData() {
		//select all the content of doc
		helper.typeIntoDocument('{ctrl}{a}');

		//assert image and size
		cy.cGet('.leaflet-overlay-pane g.Graphic image').should('exist')
		    .then($ele => {
		        const width = parseInt($ele.attr('width'));
		        const height = parseInt($ele.attr('height'));
		        expect(width).to.be.closeTo(18969, 10);
		        expect(height).to.be.closeTo(7397, 10);
		    });

		var selector = '#document-container g.Page';

		cy.cGet(selector + ' g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of table cells
		cy.cGet(selector + ' path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(6);
			});

		//assert text properties
		cy.cGet(selector + ' g')
			.should('have.class', 'com.sun.star.drawing.CustomShape');

		cy.cGet(selector + ' .TextParagraph')
			.should('have.attr', 'font-family', 'Calibri, sans-serif');

		cy.cGet(selector + ' .TextParagraph')
			.should('have.attr', 'font-size', '1552px');

		cy.cGet(selector + ' .TextParagraph .TextPosition tspan')
			.should('have.text', 'LibreOffice');
	}

	it('Open pptx file', function() {
		before('impress/testfile.pptx');

		assertData();
	});

	it('Open ppt file', function() {
		before('impress/testfile.ppt');

		assertData();
	});

	it('Open pptm file', function() {
		before('impress/testfile.pptm');

		assertData();
	});

	it('Open pot file', function() {
		desktopHelper.openReadOnlyFile('impress/testfile.pot');
	});

	it('Open potx file', function() {
		desktopHelper.openReadOnlyFile('impress/testfile.potx');
	});

	it('Open potm file', function() {
		desktopHelper.openReadOnlyFile('impress/testfile.potm');
	});

	it('Open fodp file', function() {
		before('impress/testfile.fodp');

		assertData();
	});

	it('Open ppsx file', function() {
		before('impress/testfile.ppsx');

		assertData();
	});
});
