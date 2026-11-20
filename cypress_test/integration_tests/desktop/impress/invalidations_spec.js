/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress invalidation tests.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
	});

	// Switching to a slide with an empty outline placeholder should not
	// produce full-slide (EMPTY) invalidations.
	it('Slide switch invalidation count.', function() {
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		}).then(() => {
			helper.processToIdle(this.win);
		}).then(() => {
			var docLayer = this.win.app.map._docLayer;
			var emptyCount = 0;
			var origFn = docLayer.handleInvalidateTilesMsg.bind(docLayer);

			docLayer.handleInvalidateTilesMsg = function(textMsg) {
				if (textMsg.substring('invalidatetiles:'.length + 1).startsWith('EMPTY'))
					emptyCount++;
				origFn(textMsg);
			};

			// Switch from slide 1 to slide 2 (which has an empty outline placeholder)
			cy.cGet('#preview-img-part-1').click();
			return cy.wrap(null).then(() => {
				helper.processToIdle(this.win);
			}).then(() => {
				cy.log('Slide 1->2: ' + emptyCount + ' EMPTY invalidations');
				expect(emptyCount,
					'slide 1->2 should have no EMPTY invalidations, got ' + emptyCount)
					.to.equal(0);
			});
		});
	});
});
