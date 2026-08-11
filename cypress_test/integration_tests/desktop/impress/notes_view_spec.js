/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// Count the dark pixels on the main tile canvas to detect notes typing.
function countDarkPixels(canvasElement) {
	var context = canvasElement.getContext('2d');
	var pixels = context.getImageData(0, 0, canvasElement.width, canvasElement.height).data;
	var dark = 0;
	for (var i = 0; i < pixels.length; i += 4) {
		// Opaque and clearly darker than the light page background.
		if (pixels[i + 3] > 0 && pixels[i] + pixels[i + 1] + pixels[i + 2] < 384)
			dark++;
	}
	return dark;
}

describe(['tagdesktop'], 'Impress notes view editing.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Typed notes text is painted onto the tiles while editing.', function() {
		cy.then(() => {
			helper.processToIdle(this.win);
			this.win.app.map.sendUnoCommand('.uno:NotesMode');
		}).then(() => {
			helper.processToIdle(this.win);
		});

		// Double click low on the page, over the notes text placeholder, to
		// start editing it.
		cy.cGet('#document-container').then((items) => {
			expect(items).to.have.length(1);
			var rect = items[0].getBoundingClientRect();
			var x = (rect.left + rect.right) / 2;
			var y = rect.top + (rect.bottom - rect.top) * 0.75;
			cy.cGet('body').dblclick(x, y);
		});

		impressHelper.assertInTextEditMode();

		// The empty notes box paints no text yet.
		cy.then(() => {
			helper.processToIdle(this.win);
		});
		cy.cGet('#document-canvas').should('exist').then((canvas) => {
			this.baselineDark = countDarkPixels(canvas[0]);
		});

		cy.then(() => {
			helper.typeIntoDocument(
				'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod...');
		}).then(() => {
			helper.processToIdle(this.win);
		});

		cy.cGet('#document-canvas').should((canvas) => {
			var afterDark = countDarkPixels(canvas[0]);
			expect(afterDark - this.baselineDark,
				'dark pixels added by the typed notes text').to.be.greaterThan(50);
		});
	});
});
