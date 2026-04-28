/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* global describe it cy require beforeEach expect */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'PDF Threaded Comments', function() {

	beforeEach(function() {
		// Lock the viewport so scrolling is actually required: Alice's anchor
		// is ~803px down page 1 and Zoe's is on page 2, both outside a 600-tall
		// viewport. Without this, a larger default could make the anchors
		// visible without any scroll, hiding regressions in the scroll logic.
		cy.viewport(1000, 600);
		helper.setupAndLoadDocument('draw/threaded_comments.pdf');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	});

	function findCommentByAuthor(win, author) {
		const commentSection = win.app.sectionContainer.getSectionWithName(
			win.app.CSections.CommentList.name);
		return commentSection.sectionProperties.commentList.find(
			function(c) { return c.sectionProperties.data.author === author; });
	}

	it('Get_Comments returns threaded fields for PDF comments', { env: { 'pdf-view': true } }, function() {
		cy.getFrameWindow().then(function(win) {
			cy.stub(win.parent, 'postMessage').as('postMessage');
			win.postMessage(JSON.stringify({ MessageId: 'Get_Comments' }), '*');
		});

		cy.get('@postMessage').should(function(stub) {
			const calls = stub.getCalls().filter(function(call) {
				try {
					const msg = typeof call.args[0] === 'string'
						? JSON.parse(call.args[0]) : call.args[0];
					return msg.MessageId === 'Get_Comments_Resp';
				} catch (e) { return false; }
			});
			expect(calls.length, 'Get_Comments_Resp was not posted').to.be.greaterThan(0);
			const resp = typeof calls[0].args[0] === 'string'
				? JSON.parse(calls[0].args[0]) : calls[0].args[0];
			const comments = resp.Values.Comments;
			// Alice, Bob, Dave (page 1) + Zoe (page 2). Charlie collapsed on import
			// into Alice's resolved flag; Eve dropped as a malformed state-change.
			expect(comments.length).to.equal(4);
			const byAuthor = {};
			comments.forEach(function(c) { byAuthor[c.Author] = c; });
			expect(byAuthor.Alice, 'Alice missing').to.exist;
			expect(byAuthor.Bob, 'Bob missing').to.exist;
			expect(byAuthor.Dave, 'Dave missing').to.exist;
			expect(byAuthor.Zoe, 'Zoe missing').to.exist;
			expect(byAuthor.Alice.Threaded).to.equal('true');
			expect(byAuthor.Alice.Resolved).to.equal('true');
			expect(byAuthor.Bob.Threaded).to.equal('true');
			expect(byAuthor.Bob.Resolved).to.equal('false');
			expect(byAuthor.Bob.Parent).to.equal(String(byAuthor.Alice.Id));
			expect(byAuthor.Dave.Threaded).to.equal('true');
			expect(byAuthor.Dave.Resolved).to.equal('false');
			expect(byAuthor.Zoe.Threaded).to.equal('true');
			expect(byAuthor.Zoe.Resolved).to.equal('false');
		});
	});

	it('Resolve menu item flips an unresolved comment to resolved', { env: { 'pdf-view': true } }, function() {
		let daveId;
		cy.getFrameWindow().then(function(win) {
			const dave = findCommentByAuthor(win, 'Dave');
			expect(dave, 'Dave not found').to.exist;
			expect(dave.sectionProperties.data.resolved).to.not.equal('true');
			daveId = dave.sectionProperties.data.id;
		});

		cy.then(function() {
			cy.cGet('#comment-annotation-menu-' + daveId).click({ force: true });
			cy.cGet('#comment-menu-' + daveId + '-dropdown')
				.contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Resolve')
				.should('exist')
				.click({ force: true });
		});

		cy.getFrameWindow().should(function(win) {
			const dave = findCommentByAuthor(win, 'Dave');
			expect(dave.sectionProperties.data.resolved).to.equal('true');
		});
	});

	// Verifies that after Action_GoToComment:
	//   (a) the anchor's Y coordinate is inside the scrolled-to document area
	//   (b) the comment dialog is actually on-screen (bounding rect intersects
	//       the iframe viewport), not merely display != none - a dialog
	//       positioned off-canvas would otherwise pass a naive display check.
	function assertCommentShownAndAnchorVisible(win, author) {
		const comment = findCommentByAuthor(win, author);
		expect(comment, author + ' not found').to.exist;
		const container = comment.sectionProperties.container;
		expect(container, author + ' has no container DOM node').to.exist;
		expect(getComputedStyle(container).display,
			author + ' comment dialog display=none').to.not.equal('none');

		const anchorY = comment.sectionProperties.data.anchorPos[1];
		expect(win.app.isYVisibleInTheDisplayedArea(anchorY),
			author + ' anchor not scrolled into view (anchorY=' + anchorY + ')').to.be.true;

		const rect = container.getBoundingClientRect();
		const onScreen = rect.width > 0 && rect.height > 0
			&& rect.bottom > 0 && rect.top < win.innerHeight
			&& rect.right > 0 && rect.left < win.innerWidth;
		expect(onScreen,
			author + ' comment dialog is off-screen'
			+ ' (rect=' + JSON.stringify({top: rect.top, left: rect.left, w: rect.width, h: rect.height})
			+ ' viewport=' + win.innerWidth + 'x' + win.innerHeight + ')').to.be.true;
	}

	it('Action_GoToComment navigates within the current page', { env: { 'pdf-view': true } }, function() {
		let aliceId;
		cy.getFrameWindow().then(function(win) {
			const alice = findCommentByAuthor(win, 'Alice');
			aliceId = alice.sectionProperties.data.id;
			expect(win.app.map._docLayer._selectedPart).to.equal(0);
		});

		cy.then(function() {
			cy.getFrameWindow().then(function(win) {
				cy.stub(win.parent, 'postMessage').as('postMessage');
				win.postMessage(JSON.stringify({
					MessageId: 'Action_GoToComment',
					Values: { Id: aliceId },
				}), '*');
				helper.processToIdle(win);
			});
		});

		cy.get('@postMessage').should(function(stub) {
			const calls = stub.getCalls().filter(function(call) {
				try {
					const msg = typeof call.args[0] === 'string'
						? JSON.parse(call.args[0]) : call.args[0];
					return msg.MessageId === 'Action_GoToComment_Resp';
				} catch (e) { return false; }
			});
			expect(calls.length, 'Action_GoToComment_Resp was not posted').to.be.greaterThan(0);
			const resp = typeof calls[0].args[0] === 'string'
				? JSON.parse(calls[0].args[0]) : calls[0].args[0];
			expect(resp.Values.success,
				'Action_GoToComment_Resp reported error: ' + resp.Values.errorMsg).to.be.true;
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart,
				'current part unexpectedly changed').to.equal(0);
			assertCommentShownAndAnchorVisible(win, 'Alice');
		});
	});

	it('Action_GoToComment switches page for a comment on another page', { env: { 'pdf-view': true } }, function() {
		let zoeId;
		cy.getFrameWindow().then(function(win) {
			const zoe = findCommentByAuthor(win, 'Zoe');
			zoeId = zoe.sectionProperties.data.id;
			expect(win.app.map._docLayer._selectedPart,
				'test should start on page 1').to.equal(0);
		});

		cy.then(function() {
			cy.getFrameWindow().then(function(win) {
				win.postMessage(JSON.stringify({
					MessageId: 'Action_GoToComment',
					Values: { Id: zoeId },
				}), '*');
				helper.processToIdle(win);
			});
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart,
				'current part did not switch to page 2').to.equal(1);
			assertCommentShownAndAnchorVisible(win, 'Zoe');
		});
	});
});
