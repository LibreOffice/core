/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Insert a slide from the slide sorter', function() {

	function previewImage(position) {
		return cy.cGet('#slide-sorter .preview-img').eq(position);
	}

	it('the new slide becomes the current one and the arrow keys move on from it', function() {
		helper.setupAndLoadDocument('impress/colors.odp');

		var win;
		// The part numbers of the slides the insertion moves, read before it
		// happens. A part number stays with its slide for the slide's whole
		// lifetime, so it tells which slide a position holds afterwards.
		var greenPart;
		var bluePart;

		cy.getFrameWindow().then(function(frameWindow) {
			win = frameWindow;
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});

		// The document has five slides: red, green, blue, yellow and orange.
		cy.then(function() {
			expect(win.app.impress.partList).to.have.length(5);
			greenPart = win.app.impress.partList[1].part;
			bluePart = win.app.impress.partList[2].part;
		});

		// Make the second slide, green, the current one.
		previewImage(1).click();
		cy.then(function() {
			return helper.processToIdle(win);
		});
		cy.then(function() {
			expect(win.app.map._docLayer._selectedPart).to.equal(1);
		});

		// Insert a slide from the green slide's own context menu.
		previewImage(1).rightclick();
		cy.cGet('[id$="-dropdown"]:visible')
			.contains('.ui-combobox-entry', 'New Slide').click();

		cy.then(function() {
			impressHelper.assertSlidePreviewCountAfterIdle(win, 6);
		});

		// The new slide sits at the third position. The slides that followed
		// green keep their own part numbers at their new positions.
		cy.getFrameWindow().should(function() {
			var partList = win.app.impress.partList;
			expect(partList).to.have.length(6);
			expect(partList[1].part).to.equal(greenPart);
			expect(partList[3].part).to.equal(bluePart);
			expect(partList[2].part).to.not.equal(bluePart);
		});

		// The new slide is the current one, in the browser and in the engine.
		cy.getFrameWindow().should(function() {
			var docLayer = win.app.map._docLayer;
			expect(docLayer._selectedPart).to.equal(2);
			expect(docLayer.getSelectedPart()).to.equal(win.app.impress.partList[2].part);
		});

		// The sorter marks the new slide as the current one.
		previewImage(2).should('have.class', 'preview-img-currentpart');

		// The arrow keys move from the new slide, so pressing Down goes on to
		// blue at the fourth position.
		cy.then(function() {
			win.document.activeElement.dispatchEvent(
				new win.KeyboardEvent('keydown',
					{ key: 'ArrowDown', keyCode: 40, which: 40, bubbles: true }));
		});
		cy.getFrameWindow().should(function() {
			expect(win.app.map._docLayer._selectedPart).to.equal(3);
			expect(win.app.map._docLayer.getSelectedPart()).to.equal(bluePart);
		});
	});
});
