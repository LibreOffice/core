/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser slide move', function() {
	var win1, win2;

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide-move.fodp', true);
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	// The slide list arrives after the document is otherwise ready, so wait for
	// the known 3-slide layout.
	function waitForLayout(win) {
		cy.wrap(null).should(function() {
			expect(win.app.impress.partList).to.have.length(3);
		});
	}

	it('moving a slide keeps the other view in place', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		// User A parks on the last slide, index 2, which User B's move leaves in
		// place because B only reorders the two slides above it.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#preview-img-part-2').scrollIntoView().click();
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(win1.app.map.getCurrentPartNumber()).to.equal(2);
		});

		// Watch every slide index User A's status handler settles on from here
		// on. A correct run only ever sees A's own slide, index 2.
		cy.then(function() {
			var docLayer = win1.app.map._docLayer;
			win1.__seenParts = [];
			var original = docLayer._onStatusMsg;
			docLayer._onStatusMsg = function(textMsg) {
				original.call(docLayer, textMsg);
				win1.__seenParts.push(docLayer._selectedPart);
			};
		});

		// User B selects slide index 0 and moves it to position 1, so B ends up
		// on slide index 1. This is the wire message a slide-sorter drop sends.
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#preview-img-part-0').scrollIntoView().click();
		helper.processToIdle(win2);
		cy.then(function() {
			win2.__orderBefore = win2.app.impress.partList.map(function(part) { return part.hash; });
			win2.app.socket.sendMessage('moveselectedclientparts position=1');
		});
		helper.processToIdle(win2);

		// Confirm the move actually happened: the slide order changed, so the
		// test cannot pass on a silent no-op.
		cy.wrap(null).should(function() {
			var orderAfter = win2.app.impress.partList.map(function(part) { return part.hash; });
			expect(orderAfter).to.not.deep.equal(win2.__orderBefore);
		});

		// Let User A finish reacting to the broadcast.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);

		// A must have processed at least one status from the move (otherwise
		// the check below is vacuous), and every one of them must have left A
		// on its own slide. Adopting B's slide, index 1, here is the bug.
		cy.wrap(null).should(function() {
			expect(win1.__seenParts.length).to.be.greaterThan(0);
			expect(win1.__seenParts).to.not.include(1);
			win1.__seenParts.forEach(function(part) {
				expect(part).to.equal(2);
			});
		});
	});
});
