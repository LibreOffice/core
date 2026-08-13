/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// The smooth scrolling setting in the Options dialog lets someone who finds
// the scroll animation uncomfortable scroll in discrete steps instead. The
// test seeds the saved setting for an explicit &userid, which the test WOPI
// server serves back on load (see userPresetDir() in
// test/TestWopiFileServer.hpp).
describe(['tagdesktop'], 'Smooth scrolling can be turned off', function() {
	var USER = 'smooth-scroll-off';
	var userQuery = 'userid=' + USER;

	beforeEach(function() {
		cy.task('writeUserSetting', { userId: USER, settings: { smoothScroll: 'false' } });
		helper.setupAndLoadDocument('writer/scrolling.odt', false, false, undefined, userQuery);
	});

	it('The wheel scrolls in discrete steps when the setting is off', function() {
		cy.getFrameWindow().then(function(win) {
			var scroll = win.app.sectionContainer.getSectionWithName(win.app.CSections.Scroll.name);
			expect(scroll.sectionProperties.animateWheelScroll).to.be.false;
		});
	});

	it('The wheel still scrolls the document when the setting is off', function() {
		var layout;
		var topBeforeScroll;

		cy.getFrameWindow().then(function(win) {
			layout = win.app.activeDocument.activeLayout;
			topBeforeScroll = layout.viewedRectangle.pY1;

			// The wheel event carries a position, and the canvas hands it to
			// whichever section covers that point. Aim at the middle of the
			// canvas so the document area gets the event rather than a ruler or
			// a header along the edges.
			var canvas = win.document.querySelector('#document-canvas');
			var rect = canvas.getBoundingClientRect();

			canvas.dispatchEvent(new win.WheelEvent('wheel', {
				deltaY: 120,
				deltaMode: win.WheelEvent.DOM_DELTA_PIXEL,
				clientX: rect.left + rect.width / 2,
				clientY: rect.top + rect.height / 2,
				bubbles: true,
				cancelable: true,
			}));

			return helper.processToIdle(win);
		});

		cy.wrap(null).should(function() {
			expect(layout.viewedRectangle.pY1).to.be.greaterThan(topBeforeScroll);
		});
	});
});
