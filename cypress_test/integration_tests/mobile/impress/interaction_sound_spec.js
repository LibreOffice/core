/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Sound interaction of a shape.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/apply_font_shape.odp');
		mobileHelper.enableEditingMobile();
	});

	// A small WAVE file: the 44 byte header and a few silent samples.
	function silentWave() {
		const samples = 8;
		const buffer = new ArrayBuffer(44 + samples * 2);
		const view = new DataView(buffer);
		const writeText = function(offset, text) {
			for (let i = 0; i < text.length; i++)
				view.setUint8(offset + i, text.charCodeAt(i));
		};
		writeText(0, 'RIFF');
		view.setUint32(4, 36 + samples * 2, true);
		writeText(8, 'WAVE');
		writeText(12, 'fmt ');
		view.setUint32(16, 16, true);
		view.setUint16(20, 1, true);
		view.setUint16(22, 1, true);
		view.setUint32(24, 8000, true);
		view.setUint32(28, 16000, true);
		view.setUint16(32, 2, true);
		view.setUint16(34, 16, true);
		writeText(36, 'data');
		view.setUint32(40, samples * 2, true);
		return Cypress.Buffer.from(buffer);
	}

	it('Browse picks a sound file that goes into the document.', function() {
		impressHelper.selectTextShapeInTheCenter();

		// The file input the Browse button opens would show the browser's own
		// picker; the test feeds the input a file instead.
		cy.getFrameWindow().then(function(win) {
			cy.stub(win.HTMLInputElement.prototype, 'click').as('openPicker');
		});

		cy.cGet('#document-container').trigger('contextmenu', 'center');
		cy.cGet('body').contains('.menu-entry-with-icon', 'Interaction...')
			.scrollIntoView().should('be.visible').click();

		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('#listbox').should('exist');
		cy.cGet('#listbox').click();
		cy.cGet('#listbox .ui-combobox-text').contains('Play audio').click();

		cy.cGet('#browse').should('be.visible').click();
		cy.get('@openPicker').should('have.been.called');

		// The picker offers the audio types the dialog lists, not every file.
		cy.cGet('#filepicker').should('have.attr', 'accept').and('include', '.mp3')
			.and('include', '.wav').and('not.include', '.*');

		cy.cGet('#filepicker').selectFile(
			{ contents: silentWave(), fileName: 'silence.wav', mimeType: 'audio/wav' },
			{ force: true });

		// The engine put the sound inside the document and names it there.
		cy.cGet('#sound-input').invoke('val')
			.should('match', /^vnd\.sun\.star\.Package:Media\/.*silence\.wav$/);
	});
});
