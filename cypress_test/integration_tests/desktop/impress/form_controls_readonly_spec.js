/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var formHelper = require('../../common/form_controls_helper');

// The form controls of a slide in a read-only document drawn from vector primitives. The document
// shows its slides one below the other, each in the middle of the view, and the controls are drawn
// as part of the slide.
describe(['tagdesktop'], 'Form controls on a vector slide of a read-only document', function() {

	beforeEach(function() {
		formHelper.loadFormDocument(function(win) {
			this.win = win;
		}.bind(this), true);
	});

	it('The slide is drawn with its controls where the view places it, and takes no input.', function() {
		var win = this.win;

		// The place on the document canvas where each slide was drawn last, and whether the
		// form controls were drawn with it, by the id of the slide.
		var drawn = {};
		cy.then(function() {
			expect(win.app.file.fileBasedView, 'the slides one below the other').to.equal(true);
			var renderInto = win.RenderManager.renderInto;
			win.RenderManager.renderInto = function(context, data, options) {
				if (context.canvas.id === 'document-canvas') {
					var transform = context.getTransform();
					drawn[data.partId] = {
						x: transform.e,
						y: transform.f,
						withControls: !(options && options.withoutFormControls),
					};
				}
				return renderInto.apply(this, arguments);
			};
			win.app.sectionContainer.requestReDraw();
		});

		// The slide lies where the view lays out its page, which is in the middle of the view.
		cy.wrap(null).should(function() {
			var layout = win.app.activeDocument.activeLayout;
			var page = layout.viewRectangles[0];
			var anchor = win.app.sectionContainer.getDocumentAnchor();
			var slide = drawn[win.RenderManager.partIdAt(0, win.cool.VectorMode.Slides)];
			expect(slide, 'the drawn slide').to.not.equal(undefined);
			expect(slide.x, 'the left edge of the slide')
				.to.be.closeTo(anchor[0] + page.pX1 - layout.scrollProperties.viewX, 1);
			expect(slide.y, 'the top edge of the slide')
				.to.be.closeTo(anchor[1] + page.pY1 - layout.scrollProperties.viewY, 1);
			expect(slide.withControls, 'the controls drawn on the slide').to.equal(true);
		});

		// No control is an element over the page, so a click on a control changes nothing.
		cy.cGet('.vector-form-controls .vector-form-control').should('have.length', 0);
		cy.then(function() {
			var layout = win.app.activeDocument.activeLayout;
			var page = layout.viewRectangles[0];
			var twipsPerCentimetre = 1440 / 2.54;
			var scale = win.app.twipsToPixels / win.app.dpiScale;
			var anchor = win.app.sectionContainer.getDocumentAnchor();
			// the middle of the check box "agree", 4.5 cm from the left and 2 cm from the top
			var x = (anchor[0] + page.pX1 - layout.scrollProperties.viewX) / win.app.dpiScale +
				4.5 * twipsPerCentimetre * scale;
			var y = (anchor[1] + page.pY1 - layout.scrollProperties.viewY) / win.app.dpiScale +
				2 * twipsPerCentimetre * scale;
			cy.cGet('#document-canvas').click(x, y);
		});
		formHelper.expectNoFormControlSent(win);
	});
});
