/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser slide sorter avatars', function() {
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

	// Frame 0 is the reorder drop-site, so frame k is slide k.
	function frameOfSlide(part) {
		return cy.cGet('.preview-frame').eq(part + 1);
	}

	function clickSlideThumbnail(part) {
		frameOfSlide(part).find('img').scrollIntoView().click();
	}

	// How many avatars and counters the active frame draws on each slide.
	function avatarCounts(win) {
		var counts = [];
		var frames = win.document.querySelectorAll('.preview-frame');
		for (var i = 1; i < frames.length; i++) {
			var strip = frames[i].querySelector('.preview-avatars');
			counts.push({
				faces: strip ? strip.querySelectorAll('.avatar-img').length : 0,
				counters: strip ? strip.querySelectorAll('.preview-avatars-more').length : 0,
			});
		}
		return counts;
	}

	it('marks the slide the other user is on, and only that slide', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		// User B goes to slide 1 and User A stays on slide 0, so the two are on
		// different slides and neither can be reading its own avatar.
		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(1);
		helper.processToIdle(win2);
		cy.wrap(null).should(function() {
			expect(win2.app.map.getCurrentPartNumber()).to.equal(1);
		});

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);

		// A sees one face on slide 1 and nothing anywhere else. Two views put
		// one other person on a slide, so the counter must not appear at all.
		cy.wrap(null).should(function() {
			var counts = avatarCounts(win1);
			expect(counts[1].faces).to.equal(1);
			expect(counts[1].counters).to.equal(0);
			expect(counts[0].faces).to.equal(0);
			expect(counts[2].faces).to.equal(0);
		});

		// B does not draw itself on the slide it is on.
		cy.cSetActiveFrame('#iframe2');
		cy.wrap(null).should(function() {
			expect(avatarCounts(win2)[1].faces).to.equal(0);
		});
	});

	it('drops the mark when the other user leaves', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(1);
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(avatarCounts(win1)[1].faces).to.equal(1);
		});

		// User B goes away. A has to hear it and clear the mark without a
		// reload, otherwise the slide keeps showing someone who has left.
		cy.then(function() {
			win2.app.socket.close();
		});

		// No processToIdle here: the other view is gone, so asking the server
		// to report when idle is answered by nobody. Let the assertion retry
		// until the removeview lands.
		cy.cSetActiveFrame('#iframe1');
		cy.wrap(null, { timeout: 30000 }).should(function() {
			expect(avatarCounts(win1)[1].faces).to.equal(0);
		});
	});

	it('moves the mark when the other user changes slide', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(1);
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(avatarCounts(win1)[1].faces).to.equal(1);
		});

		// The part change has to reach A on its own, without a reload.
		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(2);
		helper.processToIdle(win2);
		cy.wrap(null).should(function() {
			expect(win2.app.map.getCurrentPartNumber()).to.equal(2);
		});

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			var counts = avatarCounts(win1);
			expect(counts[2].faces).to.equal(1);
			expect(counts[1].faces).to.equal(0);
		});
	});
});
