/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser vector dark mode', function() {
	var win1, win2;

	// The stock load checks wait for bitmap tiles, which a vector
	// view never paints, so wait for the document connection directly.
	function waitForVectorView(win) {
		cy.wrap(null, {timeout: 60000}).should(function() {
			expect(win.app).to.exist;
			expect(win.app.map).to.exist;
			expect(win.app.map._docLayer).to.exist;
			expect(win.app.impress.partList).to.have.length.greaterThan(0);
		});
	}

	// The primitive tree of the first slide, or undefined while the
	// request for it is out.
	function firstSlide(win) {
		return win.RenderManager.requestPart(0, win.cool.VectorMode.Slides);
	}

	// Wait until the first slide's primitive tree is in the cache and
	// hand it over. requestPart asks for it while it is missing.
	function waitForFirstSlide(win) {
		cy.wrap(null, {timeout: 60000}).should(function() {
			expect(firstSlide(win)).to.exist;
		});
	}

	// The primitive tree of the first slide as text, with the objects
	// listed in paint order, so two views can be compared.
	function firstSlideAsText(win) {
		const data = firstSlide(win);
		return JSON.stringify({
			version: data.version,
			slideWidth: data.slideWidth,
			slideHeight: data.slideHeight,
			order: data.order,
			objects: data.order.map(function(id) { return data.objects.get(id); }),
		});
	}

	// The color the slide background is painted in, read back from a
	// pixel near the top left corner of the slide.
	function slideBackgroundPixel(win) {
		const data = firstSlide(win);
		const canvas = win.document.createElement('canvas');
		canvas.width = 40;
		canvas.height = 30;
		const context = canvas.getContext('2d');
		context.scale(40 / data.slideWidth, 30 / data.slideHeight);
		win.RenderManager.renderInto(context, data);
		const pixel = context.getImageData(1, 1, 1, 1).data;
		return [pixel[0], pixel[1], pixel[2]];
	}

	beforeEach(function() {
		var newFilePath = helper.setupDocument('impress/slide_operations.odp');
		helper.loadDocument(newFilePath, true, true, undefined, 'vector=true');
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	it('two views with different backgrounds paint one slide differently', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForVectorView(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForVectorView(win2);

		// User A turns the document background dark, user B keeps it light.
		cy.cSetActiveFrame('#iframe1');
		cy.then(function() {
			win1.app.map.uiManager.setDarkBackground(true);
		});
		helper.processToIdle(win1);

		cy.cSetActiveFrame('#iframe2');
		cy.then(function() {
			win2.app.map.uiManager.setDarkBackground(false);
		});
		helper.processToIdle(win2);

		waitForFirstSlide(win1);
		waitForFirstSlide(win2);

		// The slide reads dark for A and light for B.
		cy.then(function() {
			const dark = slideBackgroundPixel(win1);
			const light = slideBackgroundPixel(win2);
			expect(dark[0], 'dark view red channel').to.be.lessThan(128);
			expect(light[0], 'light view red channel').to.be.greaterThan(128);
		});

		// Both views hold the same primitive tree: what the automatic
		// colors mean belongs to the view, so it is not in the tree.
		cy.then(function() {
			expect(firstSlideAsText(win1)).to.equal(firstSlideAsText(win2));
		});
	});
});
