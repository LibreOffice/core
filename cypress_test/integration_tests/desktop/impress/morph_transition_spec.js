/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// The second, third and fourth slide of the document enter with a morph: by object, by word and
// by character. A slideshow drawn from vector primitives draws each of them object by object.
describe(['tagdesktop'], 'Morph transition in a vector slideshow', function() {
	var win;

	beforeEach(function() {
		var path = helper.setupDocument('impress/MorphTransition.pptx');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		helper.documentChecks(true);
		cy.getFrameWindow().then(function(w) {
			win = w;
			// A vector view has no tiles, so it waits for the slides instead.
			cy.wrap(null).should(function() {
				expect(win.app.impress.partList).to.have.length.greaterThan(0);
			});
			return helper.processToIdle(win);
		});
		cy.then(function() {
			// The slideshow takes WebGL2 only when it runs fast, and plays no transition on
			// the 2D canvas it falls back to. A test machine without a GPU has a software
			// WebGL2, which the slideshow takes here, so the morph plays there too.
			var getContext = win.HTMLCanvasElement.prototype.getContext;
			win.HTMLCanvasElement.prototype.getContext = function(type, attributes) {
				if (type === 'webgl2')
					attributes = Object.assign({}, attributes,
						{ failIfMajorPerformanceCaveat: false });
				return getContext.call(this, type, attributes);
			};
		});
		cy.cGet('.notebookbar #Slideshow-tab-label').click();
		cy.cGet('.notebookbar #slide-fullscreen-presentation-button').click();
		cy.then(function() {
			impressHelper.waitForSlideShowIdle(win);
		});
		cy.wrap(null).should(function() {
			expect(win.app.map.slideShowPresenter._slideRenderer._context.is2dGl(),
				'the slideshow draws with WebGL2').to.be.false;
		});
	});

	it('Plays the morph between the slides', function() {
		var morph;
		cy.then(function() {
			morph = cy.spy(win.app.map.slideShowPresenter._slideCompositor, 'drawMorphFrame');
		});

		[1, 2, 3].forEach(function(slide) {
			cy.then(function() {
				morph.resetHistory();
				impressHelper.clickSlideShowNav(win, '#next');
				impressHelper.waitForSlideShowIdle(win);
			});
			cy.wrap(null).should(function() {
				expect(win.app.map.slideShowPresenter._slideShowNavigator.currentSlideIndex)
					.to.equal(slide);
				// A frame that could not be drawn object by object returns false.
				expect(morph, 'morph frames').to.have.returned(true);
			});
		});
	});
});
