/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe.skip('Spell checking menu.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/spellchecking.odt', false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	function openContextMenu() {
		// Do a new selection
		writerHelper.selectAllTextOfDoc();

		// Open context menu
		cy.cGet('.leaflet-marker-icon')
			.then(function(markers) {
				expect(markers.length).to.have.greaterThan(1);
				for (var i = 0; i < markers.length; i++) {
					if (markers[i].classList.contains('text-selection-handle-start')) {
						var startPos = markers[i].getBoundingClientRect();
					} else if (markers[i].classList.contains('text-selection-handle-end')) {
						var endPos = markers[i].getBoundingClientRect();
					}
				}

				// Remove selection
				helper.typeIntoDocument('{downarrow}');

				helper.textSelectionShouldNotExist();

				var XPos = startPos.right + 10;
				var YPos = endPos.top - 10;
				mobileHelper.longPressOnDocument(XPos, YPos);
			});

		cy.cGet('#mobile-wizard-content').should('be.visible');
	}

	it('Apply suggestion.', function() {
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'hello').click();
		writerHelper.selectAllTextOfDoc();
		helper.expectTextForClipboard('hello');
	});

	it('Ignore one.', function() {
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'Ignore').click();
		openContextMenu();
		// We don't get the spell check context menu any more
		cy.cGet('body').contains('.context-menu-link', 'Paste');
	});

	it('Ignore all.', function() {
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'Ignore All').click();
		openContextMenu();
		// We don't get the spell check context menu any more
		cy.cGet('body').contains('.context-menu-link', 'Paste').should('be.visible');
	});
});
