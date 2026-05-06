/* global describe it cy require */
var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Open different file types', function() {

	function assertData() {
		//select all the content of doc
		helper.typeIntoDocument('{shift}{end}');

		//assert text and its properties
		var container = '#copy-paste-container';

		cy.cGet(container + ' p font').should('have.attr', 'size', '6');
		cy.cGet(container + ' p font b').should('exist');
		cy.cGet(container + ' p font b').should('exist').and('have.text', 'LibreOffice');
		//assert table cells
		helper.typeIntoDocument('{home}{downarrow}');
		helper.typeIntoDocument('{shift}{rightarrow}{rightarrow}{rightarrow}{rightarrow}');
		cy.cGet(container + ' table').should('exist');
		cy.cGet('#copy-paste-container colgroup').find('col').should('have.length', 4);
		cy.cGet('#copy-paste-container tbody').find('tr').should('have.length', 1);
		helper.typeIntoDocument('{downarrow}');

		// Click right to the blinking cursor position.
		cy.cGet('.leaflet-cursor.blinking-cursor')
			.then(function(cursor) {
				var boundRect = cursor[0].getBoundingClientRect();
				var XPos = boundRect.right + 10;
				var YPos = (boundRect.top + boundRect.bottom) / 2;

				cy.cGet('body').click(XPos, YPos);
			});

		//assert image
		helper.assertImageSize(480, 122);
	}

	it.skip('Open doc file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('writer/testfile.doc');
		assertData();
	});

	it.skip('Open docx file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('writer/testfile.docx');
		assertData();
	});

	it.skip('Open docm file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('writer/testfile.docm');
		assertData();
	});

	it.skip('Open fodt file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('writer/testfile.fodt');
		assertData();
	});

	it.skip('Open dot file', { defaultCommandTimeout: 60000 }, function() {
		desktopHelper.openReadOnlyFile('writer/testfile.dot');
	});

	it.skip('Open dotm file', { defaultCommandTimeout: 60000 }, function() {
		desktopHelper.openReadOnlyFile('writer/testfile.dotm');
	});

	it('Open dotx file', function() {
		helper.setupAndLoadDocument('writer/testfile.dotx');
		// Template files should open in edit mode, not read-only.
		// Without the accompanying fix in place, this test would have failed,
		// the server sent filemode with readOnly: true for dotx files.
		// This resulted in having no notebookbar:
		// assertexpected .notebookbar-scroll-wrapper to have class initialized
		cy.cGet('#viewModeDropdownButton-button').should('be.visible').should('have.text', 'Editing');
	});
});
