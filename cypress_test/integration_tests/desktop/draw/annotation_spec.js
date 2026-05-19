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

	// Insert Comment on a PDF defers comment creation until the user clicks
	// on the page: the toolbar/menu action enters a placement mode (crosshair
	// cursor, capture-phase mousedown listener) and the captured click is
	// converted to document twips for both the on-screen anchor and the
	// .uno:InsertAnnotation PositionX/Y args.
	it('Insert Comment in PDF enters placement mode, pins the anchor to the click, and round-trips through core', { env: { 'pdf-view': true } }, function() {
		const commentText = 'placement-mode-test ' + Date.now();
		let initialCount = 0;

		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			initialCount = section.sectionProperties.commentList.length;
			win.app.map.insertComment();
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'placement mode must switch the canvas cursor to crosshair')
				.to.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'no comment should be created until the user picks a position')
				.to.equal(initialCount);
		});

		let expectedX = 0, expectedY = 0;
		cy.getFrameWindow().then(function(win) {
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();

			// Aim for ~25% across page 1 in document twips, then map back to
			// canvas-relative CSS pixels.
			const docLayer = win.app.map._docLayer;
			const targetTwipsX = docLayer._partWidthTwips / 4;
			const targetTwipsY = docLayer._partHeightTwips / 4;
			const canvasClickX = (targetTwipsX * win.app.twipsToPixels) / win.app.dpiScale;
			const canvasClickY = (targetTwipsY * win.app.twipsToPixels) / win.app.dpiScale;

			// canvasToDocumentPoint mutates its input, so build a fresh
			// SimplePoint here that matches finishCommentPlacement's local point.
			const expectedPoint = new win.cool.SimplePoint(0, 0);
			expectedPoint.cX = canvasClickX;
			expectedPoint.cY = canvasClickY;
			const docPoint = win.app.activeDocument.activeLayout.canvasToDocumentPoint(expectedPoint);
			expectedX = docPoint.x;
			expectedY = docPoint.y;

			// Capture-phase listeners inside startCommentPlacement consume
			// these; bubbles:true is needed so the capture path runs. A
			// mouseup at the same point with no mousemove between keeps the
			// gesture below DRAG_THRESHOLD_PX, so finishCommentPlacement
			// treats it as a point placement (no Width/Height arg).
			const downEv = new win.MouseEvent('mousedown', {
				clientX: rect.left + canvasClickX,
				clientY: rect.top + canvasClickY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(downEv);
			const upEv = new win.MouseEvent('mouseup', {
				clientX: rect.left + canvasClickX,
				clientY: rect.top + canvasClickY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(upEv);
		});

		// Local 'new' comment exists with the expected on-screen anchor.
		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'cursor must be restored after placement finishes')
				.to.not.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const newComment = section.sectionProperties.commentList.find(
				function(c) { return c.sectionProperties.data.id === 'new'; });
			expect(newComment, 'new comment was not created after the click').to.exist;

			const anchorPos = newComment.sectionProperties.data.anchorPos;
			expect(anchorPos[0],
				'on-screen anchor X must match canvasToDocumentPoint of the click').to.be.closeTo(expectedX, 1);
			expect(anchorPos[1],
				'on-screen anchor Y must match canvasToDocumentPoint of the click').to.be.closeTo(expectedY, 1);
		});

		// Wait for the editor to render with a textarea, then settle so its
		// id has flipped from 'new' to the final value.
		cy.cGet('.cool-annotation').last({log: false})
			.find('#annotation-modify-textarea-new').should('exist');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.type(commentText);
		cy.cGet('.cool-annotation').last({log: false})
			.find('[value="Save"]').click();

		// After core's acknowledgement: the commentList grew by one, the new
		// comment carries a non-'new' id assigned by core, and its anchor
		// survived the round-trip - the position core stored matches the
		// position the placement-mode click captured.
		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'commentList must have one more entry after save')
				.to.equal(initialCount + 1);

			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === commentText;
			});
			expect(acked, 'a comment with the typed text must be present').to.exist;
			expect(acked.sectionProperties.data.id,
				'core must assign a real (non-\'new\') id').to.not.equal('new');

			// Round-trip goes twips -> mm/100 (core) -> twips, so allow a few
			// twips of rounding slack rather than asserting exact equality.
			const anchorPos = acked.sectionProperties.data.anchorPos;
			expect(anchorPos[0],
				'round-tripped anchor X must match the captured click').to.be.closeTo(expectedX, 5);
			expect(anchorPos[1],
				'round-tripped anchor Y must match the captured click').to.be.closeTo(expectedY, 5);
		});
	});

	// A WOPI host can request a new comment in PDF by posting Send_UNO_Command
	// with .uno:InsertAnnotation. That should enter the in-browser handling,
	// instead of sending the command to engine.
	it('Send_UNO_Command .uno:InsertAnnotation enters click-to-place mode', { env: { 'pdf-view': true } }, function() {
		let initialCount = 0;

		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			initialCount = section.sectionProperties.commentList.length;

			const message = {
				MessageId: 'Send_UNO_Command',
				Values: { Command: '.uno:InsertAnnotation' }
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'placement mode must switch the canvas cursor to crosshair')
				.to.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'no comment should be created until the user picks a position')
				.to.equal(initialCount);
		});
	});

	// Negative path through placement mode: enter placement, miss the page
	// (no comment), then click on the page (anchor visible at the click
	// point), then cancel the editor before saving (no comment ends up in
	// the document at all).
	it('Insert Comment placement mode: off-page miss, on-page anchor, cancel', { env: { 'pdf-view': true } }, function() {
		let initialCount = 0;

		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			initialCount = section.sectionProperties.commentList.length;
			win.app.map.insertComment();
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor)
				.to.equal('crosshair');
		});

		// Off-page click. Compute an X past the page's right edge from
		// _partWidthTwips so it's guaranteed off-page regardless of zoom or
		// scroll. mousedown+mouseup at the same point keeps the gesture
		// below DRAG_THRESHOLD_PX so finishCommentPlacement runs and
		// rejects the off-page point.
		cy.getFrameWindow().then(function(win) {
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();
			const docLayer = win.app.map._docLayer;
			const offPageCssX = (docLayer._partWidthTwips
				* win.app.twipsToPixels) / win.app.dpiScale + 50;
			const downEv = new win.MouseEvent('mousedown', {
				clientX: rect.left + offPageCssX,
				clientY: rect.top + 200,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(downEv);
			const upEv = new win.MouseEvent('mouseup', {
				clientX: rect.left + offPageCssX,
				clientY: rect.top + 200,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(upEv);
		});

		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'placement mode should remain active after an off-page click')
				.to.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'no comment should be added on an off-page click')
				.to.equal(initialCount);
		});

		// On-page click - placement mode exits, a 'new' comment gets the
		// expected anchor and the editor opens.
		let expectedX = 0, expectedY = 0;
		cy.getFrameWindow().then(function(win) {
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();

			// Same target-twips-back-to-CSS-pixels strategy as the happy-path
			// test - aims for ~25% across page 1 regardless of zoom.
			const docLayer = win.app.map._docLayer;
			const targetTwipsX = docLayer._partWidthTwips / 4;
			const targetTwipsY = docLayer._partHeightTwips / 4;
			const canvasClickX = (targetTwipsX * win.app.twipsToPixels) / win.app.dpiScale;
			const canvasClickY = (targetTwipsY * win.app.twipsToPixels) / win.app.dpiScale;

			const expectedPoint = new win.cool.SimplePoint(0, 0);
			expectedPoint.cX = canvasClickX;
			expectedPoint.cY = canvasClickY;
			const docPoint = win.app.activeDocument.activeLayout.canvasToDocumentPoint(expectedPoint);
			expectedX = docPoint.x;
			expectedY = docPoint.y;

			const downEv = new win.MouseEvent('mousedown', {
				clientX: rect.left + canvasClickX,
				clientY: rect.top + canvasClickY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(downEv);
			const upEv = new win.MouseEvent('mouseup', {
				clientX: rect.left + canvasClickX,
				clientY: rect.top + canvasClickY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(upEv);
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'cursor must be restored after the on-page click')
				.to.not.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const newComment = section.sectionProperties.commentList.find(
				function(c) { return c.sectionProperties.data.id === 'new'; });
			expect(newComment, 'new comment must be created at the click').to.exist;
			const anchorPos = newComment.sectionProperties.data.anchorPos;
			expect(anchorPos[0],
				'temporary anchor X must match the click').to.be.closeTo(expectedX, 1);
			expect(anchorPos[1],
				'temporary anchor Y must match the click').to.be.closeTo(expectedY, 1);
		});

		// Cancel the editor before typing/saving.
		cy.cGet('.cool-annotation').last({log: false})
			.find('#annotation-modify-textarea-new').should('exist');
		cy.cGet('.cool-annotation').last({log: false})
			.find('#annotation-cancel-new').click();

		// Settle and verify the document carries no extra comment.
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'no comment should remain after cancelling the editor')
				.to.equal(initialCount);
		});
	});

	// Drag-to-area placement: while in placement mode, the user clicks and
	// drags a rectangle on the page. The anchor lands at the top-left of the
	// rectangle and the dragged Width/Height reach core, so the new comment's
	// marker spans the dragged area instead of the default 5x5 mm marker.
	it('Insert Comment in PDF supports drag-to-area selection', { env: { 'pdf-view': true } }, function() {
		const commentText = 'drag-to-area-test ' + Date.now();
		let initialCount = 0;

		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			initialCount = section.sectionProperties.commentList.length;
			win.app.map.insertComment();
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'placement mode must switch the canvas cursor to crosshair')
				.to.equal('crosshair');
		});

		// Drag a rectangle from ~25%/25% to ~50%/40% of page 1. Compute the
		// CSS-pixel positions from twips so the test is zoom-independent.
		let expectedTLX = 0, expectedTLY = 0;
		let expectedW = 0, expectedH = 0;
		cy.getFrameWindow().then(function(win) {
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();
			const docLayer = win.app.map._docLayer;

			const startTwipsX = docLayer._partWidthTwips / 4;
			const startTwipsY = docLayer._partHeightTwips / 4;
			const endTwipsX = docLayer._partWidthTwips / 2;
			const endTwipsY = docLayer._partHeightTwips * 0.4;

			const startCssX = (startTwipsX * win.app.twipsToPixels) / win.app.dpiScale;
			const startCssY = (startTwipsY * win.app.twipsToPixels) / win.app.dpiScale;
			const endCssX = (endTwipsX * win.app.twipsToPixels) / win.app.dpiScale;
			const endCssY = (endTwipsY * win.app.twipsToPixels) / win.app.dpiScale;

			// Top-left of the dragged rectangle (= the anchor).
			const tlPoint = new win.cool.SimplePoint(0, 0);
			tlPoint.cX = Math.min(startCssX, endCssX);
			tlPoint.cY = Math.min(startCssY, endCssY);
			const docTL = win.app.activeDocument.activeLayout.canvasToDocumentPoint(tlPoint);
			expectedTLX = docTL.x;
			expectedTLY = docTL.y;
			// Bottom-right; canvasToDocumentPoint mutates so build a fresh point.
			const brPoint = new win.cool.SimplePoint(0, 0);
			brPoint.cX = Math.max(startCssX, endCssX);
			brPoint.cY = Math.max(startCssY, endCssY);
			const docBR = win.app.activeDocument.activeLayout.canvasToDocumentPoint(brPoint);
			expectedW = docBR.x - expectedTLX;
			expectedH = docBR.y - expectedTLY;

			const downEv = new win.MouseEvent('mousedown', {
				clientX: rect.left + startCssX,
				clientY: rect.top + startCssY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(downEv);
			const moveEv = new win.MouseEvent('mousemove', {
				clientX: rect.left + endCssX,
				clientY: rect.top + endCssY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(moveEv);
			const upEv = new win.MouseEvent('mouseup', {
				clientX: rect.left + endCssX,
				clientY: rect.top + endCssY,
				button: 0,
				bubbles: true,
			});
			canvas.dispatchEvent(upEv);
		});

		// Local 'new' comment exists with anchor at the rectangle's top-left
		// and a marker rectangle that reflects the dragged size.
		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'cursor must be restored after placement finishes')
				.to.not.equal('crosshair');

			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const newComment = section.sectionProperties.commentList.find(
				function(c) { return c.sectionProperties.data.id === 'new'; });
			expect(newComment, 'new comment was not created after drag').to.exist;
			const data = newComment.sectionProperties.data;
			expect(data.size, 'drag must capture a size').to.exist;
			expect(data.size[0], 'captured width must match dragged width')
				.to.be.closeTo(expectedW, 1);
			expect(data.size[1], 'captured height must match dragged height')
				.to.be.closeTo(expectedH, 1);
			expect(data.anchorPos[0],
				'anchor X must be the rectangle top-left').to.be.closeTo(expectedTLX, 1);
			expect(data.anchorPos[1],
				'anchor Y must be the rectangle top-left').to.be.closeTo(expectedTLY, 1);
			// rectangle = [x, y, width, height] in twips; width/height come from data.size.
			expect(data.rectangle[2], 'marker rectangle width must reflect drag')
				.to.be.closeTo(expectedW, 1);
			expect(data.rectangle[3], 'marker rectangle height must reflect drag')
				.to.be.closeTo(expectedH, 1);
		});

		// Type and save - core stores the area as PDF /Rect.
		cy.cGet('.cool-annotation').last({log: false})
			.find('#annotation-modify-textarea-new').should('exist');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.type(commentText);
		cy.cGet('.cool-annotation').last({log: false})
			.find('[value="Save"]').click();

		// After core's acknowledgement: the area survived twips -> mm/100 -> twips.
		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'commentList must have one more entry after save')
				.to.equal(initialCount + 1);

			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === commentText;
			});
			expect(acked, 'a comment with the typed text must be present').to.exist;
			expect(acked.sectionProperties.data.id,
				'core must assign a real (non-\'new\') id').to.not.equal('new');

			const ackRect = acked.sectionProperties.data.rectangle;
			// Round-trip goes twips -> mm/100 (core) -> twips, so allow a few
			// twips of rounding slack.
			expect(ackRect[2],
				'round-tripped marker width must match the dragged area').to.be.closeTo(expectedW, 5);
			expect(ackRect[3],
				'round-tripped marker height must match the dragged area').to.be.closeTo(expectedH, 5);
			// Core flagged the size as explicit, so Online creates the
			// anchor-area sub-section underneath the comment marker.
			expect(acked.sectionProperties.data.hasArea,
				'core must report hasArea for an explicit-size comment').to.equal('true');
			expect(acked.sectionProperties.commentAnchorAreaSubSection,
				'comment must own an anchor-area sub-section').to.exist;
		});

		// Click on the anchor-area div and verify the comment becomes
		// the selected one - the area's div has pointer-events:auto and
		// listens for click directly, no canvas section routing involved.
		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === commentText;
			});
			// Deselect first so the click below has something observable to
			// flip; selectedComment may already point at our comment after
			// save.
			if (section.sectionProperties.selectedComment === acked)
				acked.onCancelClick(null);
			const div = acked.sectionProperties.commentAnchorAreaSubSection
				.sectionProperties.objectDiv;
			const rect = div.getBoundingClientRect();
			const cx = rect.left + rect.width / 2;
			const cy = rect.top + rect.height / 2;
			['mousedown', 'mouseup', 'click'].forEach(function(t) {
				div.dispatchEvent(new win.MouseEvent(t, {
					clientX: cx, clientY: cy, button: 0, bubbles: true,
				}));
			});
		});

		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === commentText;
			});
			expect(section.sectionProperties.selectedComment,
				'clicking on the anchor-area must select the comment')
				.to.equal(acked);
		});
	});

	// Starting a drag-to-area gesture on top of an existing area-anchored
	// comment must not create a new comment: the area sub-section catches
	// mousedown and stops propagation, so the canvas placement gesture is
	// never armed. Note: this tests the status quo; if you decide to change
	// the behavior, just change the test.
	it('Insert Comment starting over an existing area does not create a new one', { env: { 'pdf-view': true } }, function() {
		const existingText = 'over-existing-area ' + Date.now();
		let countWithExisting = 0;

		// Create an existing area-anchored comment in the upper-left of page 1.
		cy.getFrameWindow().then(function(win) {
			win.app.map.insertComment();
		});

		cy.getFrameWindow().then(function(win) {
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();
			const docLayer = win.app.map._docLayer;
			const sx = (docLayer._partWidthTwips * 0.10 * win.app.twipsToPixels) / win.app.dpiScale;
			const sy = (docLayer._partHeightTwips * 0.10 * win.app.twipsToPixels) / win.app.dpiScale;
			const ex = (docLayer._partWidthTwips * 0.30 * win.app.twipsToPixels) / win.app.dpiScale;
			const ey = (docLayer._partHeightTwips * 0.20 * win.app.twipsToPixels) / win.app.dpiScale;
			canvas.dispatchEvent(new win.MouseEvent('mousedown', {
				clientX: rect.left + sx, clientY: rect.top + sy, button: 0, bubbles: true,
			}));
			canvas.dispatchEvent(new win.MouseEvent('mousemove', {
				clientX: rect.left + ex, clientY: rect.top + ey, button: 0, bubbles: true,
			}));
			canvas.dispatchEvent(new win.MouseEvent('mouseup', {
				clientX: rect.left + ex, clientY: rect.top + ey, button: 0, bubbles: true,
			}));
		});

		cy.cGet('.cool-annotation').last({log: false})
			.find('#annotation-modify-textarea-new').should('exist');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.should('not.have.attr', 'disabled');
		cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea')
			.type(existingText);
		cy.cGet('.cool-annotation').last({log: false})
			.find('[value="Save"]').click();

		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === existingText;
			});
			expect(acked, 'first comment must be saved').to.exist;
			expect(acked.sectionProperties.commentAnchorAreaSubSection,
				'first comment must own an anchor-area sub-section').to.exist;
			countWithExisting = section.sectionProperties.commentList.length;
		});

		// Re-enter placement mode and start a drag whose mousedown lands on
		// the existing area. mousemove and mouseup are dispatched on the
		// canvas to mimic a real drag - the area's mousedown stopPropagation
		// should keep this from arming the gesture.
		cy.getFrameWindow().then(function(win) {
			win.app.map.insertComment();
		});

		cy.getFrameWindow().should(function(win) {
			expect(win.document.getElementById('document-canvas').style.cursor,
				'placement mode must be active').to.equal('crosshair');
		});

		cy.getFrameWindow().then(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			const acked = section.sectionProperties.commentList.find(function(c) {
				return c.sectionProperties.data.text === existingText;
			});
			const div = acked.sectionProperties.commentAnchorAreaSubSection
				.sectionProperties.objectDiv;
			const r = div.getBoundingClientRect();
			const canvas = win.document.getElementById('document-canvas');
			const downX = r.left + r.width / 2;
			const downY = r.top + r.height / 2;
			const upX = r.right + 100;
			const upY = r.bottom + 100;
			div.dispatchEvent(new win.MouseEvent('mousedown', {
				clientX: downX, clientY: downY, button: 0, bubbles: true,
			}));
			canvas.dispatchEvent(new win.MouseEvent('mousemove', {
				clientX: upX, clientY: upY, button: 0, bubbles: true,
			}));
			canvas.dispatchEvent(new win.MouseEvent('mouseup', {
				clientX: upX, clientY: upY, button: 0, bubbles: true,
			}));
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().should(function(win) {
			const section = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.CommentList.name);
			expect(section.sectionProperties.commentList.length,
				'starting drag-to-area on an existing area must not add a comment')
				.to.equal(countWithExisting);
		});
	});
});
