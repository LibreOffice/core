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
		frameOfSlide(part).find('img.preview-img').scrollIntoView().click();
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
		// B closes their page, which ends their session for good.
		cy.then(function() {
			win2.frameElement.remove();
		});

		// No processToIdle here: the other view is gone, so asking the server
		// to report when idle is answered by nobody. Let the assertion retry
		// until the removeview lands.
		cy.cSetActiveFrame('#iframe1');
		cy.wrap(null).should(function() {
			expect(avatarCounts(win1)[1].faces).to.equal(0);
		});
	});

	it('hides the mark while the other user is parked and brings it back', function() {
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
			expect(Object.keys(win1.app.map._viewInfo)).to.have.length(2);
		});

		// One row each for A and B, neither away.
		cy.cGet('#userListHeader').click();
		cy.cGet('#userlist-dropdown').should('exist');
		cy.cGet('.user-list-item').should('have.length', 2);
		cy.cGet('.user-list-item.parked').should('not.exist');
		cy.then(function() {
			win1.JSDialog.CloseDropdown('userlist');
		});
		cy.cGet('#userlist-dropdown').should('not.exist');

		// A third view would come to A as an addview, so count them.
		let addedViews = 0;
		let otherViewId;
		cy.then(function() {
			win1.app.map.on('addview', function() { addedViews++; });
			otherViewId = win2.app.map._docLayer._viewId;
			// B's connection drops without a close frame, as through a bad
			// proxy. Reconnecting would follow within milliseconds, so B is
			// held in the state it uses while the server has it idle, which
			// stops _activate() from opening a new socket until released.
			win2.app.idleHandler._documentIdle = true;
			win2.app.socket.sendMessage('debugdropconnection');
		});

		// No processToIdle on win2 here: its socket is down. A hears about
		// the parking through viewinfo, so let the assertion retry until it
		// lands. The view stays known, so the count stays at two.
		cy.cSetActiveFrame('#iframe1');
		cy.wrap(null).should(function() {
			expect(win1.app.map.isViewParked(otherViewId)).to.equal(true);
			expect(avatarCounts(win1)[1].faces).to.equal(0);
			expect(Object.keys(win1.app.map._viewInfo)).to.have.length(2);
		});
		cy.cGet('#userListHeader').click();
		cy.cGet('#userlist-dropdown').should('exist');
		cy.cGet('.user-list-item').should('have.length', 2);
		cy.cGet('.user-list-item.parked').should('have.length', 1);
		cy.then(function() {
			win1.JSDialog.CloseDropdown('userlist');
		});
		cy.cGet('#userlist-dropdown').should('not.exist');

		// Release B. Its socket carries the same token, so the server attaches
		// it to the parked view and B keeps its view id.
		cy.then(function() {
			win2.app.idleHandler._documentIdle = false;
			win2.app.idleHandler._activate();
		});
		cy.cSetActiveFrame('#iframe2');
		cy.wrap(null).should(function() {
			expect(win2.app.socket._reconnecting).to.equal(false);
			expect(win2.app.socket.connected()).to.equal(true);
			expect(win2.app.map._docLayer._viewId).to.equal(otherViewId);
		});
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(win1.app.map.isViewParked(otherViewId)).to.equal(false);
			expect(avatarCounts(win1)[1].faces).to.equal(1);
			expect(Object.keys(win1.app.map._viewInfo)).to.have.length(2);
			expect(addedViews).to.equal(0);
		});
		cy.cGet('#userListHeader').click();
		cy.cGet('#userlist-dropdown').should('exist');
		cy.cGet('.user-list-item').should('have.length', 2);
		cy.cGet('.user-list-item.parked').should('not.exist');
	});

	// The name the other view is known by, however the test host set it.
	function otherName(win) {
		var mine = win.app.map._docLayer._viewId;
		var info = win.app.map._viewInfo;
		for (var id in info)
			if (parseInt(id) !== mine) return info[id].username;
		return null;
	}

	it('names the people on a slide in the slide\'s own label', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(1);
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);

		// The strip is aria-hidden, so the slide's own label is the only place
		// a name can reach a screen reader.
		cy.wrap(null).should(function() {
			var name = otherName(win1);
			expect(name).to.not.be.null;
			var frames = win1.document.querySelectorAll('.preview-frame');
			var here = frames[2].querySelector('.preview-img');
			var elsewhere = frames[1].querySelector('.preview-img');
			expect(here.getAttribute('alt')).to.contain(name);
			expect(here.getAttribute('data-cooltip')).to.contain(name);
			expect(elsewhere.getAttribute('alt')).to.not.contain(name);
		});
	});

	it('names the user behind a face, and the face follows that user', function() {
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
			var name = otherName(win1);
			var face = win1.document.querySelectorAll('.preview-frame')[2]
				.querySelector('.preview-avatars .avatar-img');
			expect(face).to.not.be.null;
			expect(face.getAttribute('data-cooltip')).to.equal(name);
		});

		// The element is kept and moved, so the one on the new slide has to be
		// the one that names its user there, not the slide it started on.
		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(2);
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			var name = otherName(win1);
			var frames = win1.document.querySelectorAll('.preview-frame');
			var face = frames[3].querySelector('.preview-avatars .avatar-img');
			expect(face).to.not.be.null;
			expect(face.getAttribute('data-cooltip')).to.equal(name);
			expect(face.closest('.preview-frame')).to.equal(frames[3]);
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
