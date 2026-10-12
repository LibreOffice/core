/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer comments for the reader', { testIsolation: false }, function () {
	let win;

	// the text the details of a mark point at, which a reader gives on demand; NVDA reads only the
	// first of several targets, and none hidden
	function detailsText(mark) {
		const ids = (mark.getAttribute('aria-details') || '').split(' ').filter(Boolean);
		expect(ids, 'the targets').to.have.length(1);
		const target = win.document.getElementById(ids[0]);
		expect(target, 'the target').to.not.equal(null);
		expect(target.checkVisibility({ visibilityProperty: true }), 'the target shown').to.equal(true);
		expect(target.closest('[aria-hidden="true"]'), 'the target hidden').to.equal(null);
		expect(target.getAttribute('role'), 'the role of the target').to.equal('comment');
		return target.innerText.replace(/\s+/g, ' ').trim();
	}

	function expectComment(selector, text, details) {
		cy.cGet(selector + ' [role="mark"]').should(function ($marks) {
			expect($marks, 'the marks').to.have.length(1);
			expect($marks.attr('aria-roledescription'), 'the role read').to.equal('Comment');
			expect($marks.text(), 'the commented text').to.equal(text);
			expect(detailsText($marks[0]), 'the details').to.equal(details);
		});
		// the accessibility tree follows the DOM later
		cy.waitUntil(function () {
			return a11yHelper.getAXNodesWithin(selector).then(function (nodes) {
				return nodes.filter(function (node) {
					return !node.ignored && node.role === 'mark';
				}).length === 1;
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_comment.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('a comment is marked at its anchor, with its author and text as its details', function () {
		expectComment('#clipboard-area', '\ufffc', 'Alice Check this word.');
		cy.then(function () {
			expect(win.app.map._textInput.getPlainTextContent(), 'the editable text')
				.to.equal('Before \ufffcafter.');
		});
	});

	it('a range comment of a paragraph around the caret marks its range', function () {
		expectComment('#a11y-context-after > :first-child', 'marked range\ufffc', 'Bob About the marked words.');
	});

	it('a comment hidden in the panel keeps its details', function () {
		cy.then(function () {
			win.app.map.showComments(false);
		});
		expectComment('#clipboard-area', '\ufffc', 'Alice Check this word.');
		// a paragraph reached with comments switched off has them too
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		expectComment('#clipboard-area', 'marked range\ufffc', 'Bob About the marked words.');
		helper.typeIntoDocument('{uparrow}');
		cy.then(function () {
			win.app.map.showComments(true);
			return helper.processToIdle(win);
		});
		expectComment('#clipboard-area', '\ufffc', 'Alice Check this word.');
	});

	it('a comment inside a commented range is a mark in its mark', function () {
		helper.typeIntoDocument('{downarrow}{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('#clipboard-area [role="mark"]').should(function ($marks) {
			expect($marks, 'the marks').to.have.length(2);
			expect($marks.eq(0).text(), 'the outer range').to.equal('outer \ufffcwords\ufffc');
			expect($marks.eq(1).text(), 'the inner comment').to.equal('\ufffc');
			expect($marks[0].contains($marks[1]), 'the inner mark in the outer one').to.equal(true);
			expect(detailsText($marks[0]), 'the outer details').to.equal('Carol About the outer words.');
			expect(detailsText($marks[1]), 'the inner details').to.equal('Dave About this one.');
			// one copy per comment shown, none left from the paragraphs before
			const ids = new Set(Array.from(win.document.querySelectorAll('[data-comment-id]'),
				function (mark) { return mark.dataset.commentId; }));
			expect(win.document.querySelectorAll('[id^="a11y-comment-"]').length, 'the copies')
				.to.equal(ids.size);
			expect(win.app.map._textInput.getPlainTextContent(), 'the editable text')
				.to.equal('Some outer \ufffcwords\ufffc end.');
		});
	});
});
