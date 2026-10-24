/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Page 1 holds three long threads, "Test line 1" (comments 1 to 6), "Test line 2"
// (7 to 13) and "Test line 3" (14 to 18). They are far taller than the page.
describe(['tagdesktop'], 'Long Writer comment threads', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/long_comment_threads.odt');
		// At 100% zoom, with the sidebar closed, the box is tall enough for the
		// longest reply, but page 1 is taller than the view.
		cy.viewport(1400, 900);
		desktopHelper.sidebarToggle();
		desktopHelper.selectZoomLevel('100', false);
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		cy.cGet('#comment-container-18').should('exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	});

	function commentSection(win) {
		return win.app.sectionContainer.getSectionWithName(win.app.CSections.CommentList.name);
	}

	// Cypress scrolls a target into view before a click, which would move the
	// cards, so the clicks here go where the target already is.
	function openMenuAndChoose(id, entry) {
		cy.cGet('#comment-annotation-menu-' + id).click({ scrollBehavior: false });
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', entry).click();
	}

	// The box that scrolls the threads of page 1.
	function pageBox() {
		return cy.cGet('.cool-annotation-page-scroller');
	}

	// A card, once the layout has put it in its page's box.
	function cardInBox(id) {
		return cy.cGet('#comment-container-' + id).should(function (card) {
			expect(card[0].parentElement.classList.contains('cool-annotation-page-scroller'), 'card in a box').to.equal(true);
		});
	}

	// Scrolls the box as a user would to read a card, until the card's top is
	// just below the top of the part of the box on screen.
	function scrollBoxTo(id) {
		cy.cGet('#document-container').then((container) => {
			var viewTop = container[0].getBoundingClientRect().top;
			cardInBox(id).then(function (card) {
				var box = card[0].parentElement;
				var visibleTop = Math.max(box.getBoundingClientRect().top, viewTop);
				box.scrollTop += card[0].getBoundingClientRect().top - visibleTop - 20;
			});
		});
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	}

	// Scrolls the box until the bottom of a card is at the bottom of the part
	// of the box on screen, as when a user scrolls down to read a thread's
	// last reply.
	function scrollBoxToBottomOf(id) {
		cy.cGet('#document-container').then((container) => {
			var viewBottom = container[0].getBoundingClientRect().bottom;
			cardInBox(id).then(function (card) {
				var box = card[0].parentElement;
				var visibleBottom = Math.min(box.getBoundingClientRect().bottom, viewBottom);
				box.scrollTop += card[0].getBoundingClientRect().bottom - visibleBottom;
			});
		});
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	}

	// Clicks in the text that a thread comments on, which selects the thread.
	// The document scrolls first when the text is out of view.
	function clickCommentedText(id) {
		var textPoint = () => {
			var data = commentSection(this.win).getComment(id).sectionProperties.data;
			return new this.win.cool.SimplePoint(data.anchorPos[0] + 50, data.anchorPos[1] + data.anchorPos[3] / 2);
		};
		cy.cGet('#document-canvas').then((canvas) => {
			var y = textPoint().vY / this.win.app.dpiScale;
			var height = canvas[0].getBoundingClientRect().height;
			if (y > 20 && y < height - 20)
				return;
			this.win.app.sectionContainer.getSectionWithName(this.win.app.CSections.Scroll.name)
				.scrollVerticalWithOffset((y - height / 2) * this.win.app.dpiScale);
			return helper.processToIdle(this.win);
		});
		cy.then(() => {
			var point = textPoint();
			cy.cGet('#document-canvas').click(point.vX / this.win.app.dpiScale, point.vY / this.win.app.dpiScale);
		});
		cy.cGet('#comment-container-' + id).should('have.class', 'annotation-active');
		// The box scrolls a newly selected thread into view in a later layout pass, which
		// would undo a scroll made before it.
		cy.wrap(null).should(() => {
			var section = commentSection(this.win);
			expect(section.selectionShownInBox, 'selection laid out in the box').to.equal(section.sectionProperties.selectedComment);
		});
	}

	// Runs check with the top of a card on screen, retried until it holds.
	function cardTop(id, check) {
		cy.cGet('#comment-container-' + id).should(function (card) {
			check(card[0].getBoundingClientRect().top);
		});
	}

	it('Removing a reply does not select its thread', function() {
		// Core puts the text cursor where the removed reply was, which is inside
		// the text that "Test line 1" comments on. The call is the one that the
		// Remove entry of the comment menu makes.
		cy.then(() => {
			commentSection(this.win).remove('6');
		});
		cy.cGet('#comment-container-6').should('not.exist');
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
		cy.then(() => {
			var selected = commentSection(this.win).sectionProperties.selectedComment;
			expect(selected ? selected.sectionProperties.data.id : null, 'selected thread').to.equal(null);
		});
	});

	it('Threads that do not fit beside their page scroll in a box', function() {
		cy.cGet('#document-container').then((container) => {
			var win = this.win;
			var page = win.app.file.writer.pageRectangleList[0];
			var top = new win.cool.SimplePoint(page[0], page[1]).vY / win.app.dpiScale;
			var bottom = new win.cool.SimplePoint(page[0], page[1] + page[3]).vY / win.app.dpiScale;
			var origin = container[0].getBoundingClientRect().top;
			pageBox().should(function (box) {
				var rect = box[0].getBoundingClientRect();
				expect(rect.top, 'box top against the page top').to.be.closeTo(origin + top, 1);
				expect(rect.height, 'box height against the page height').to.be.closeTo(bottom - top, 1);
				expect(box[0].scrollHeight, 'box content height').to.be.greaterThan(box[0].clientHeight);
			});
		});
		for (var id = 1; id <= 18; id++)
			cy.cGet('#comment-container-' + id).parent().should('have.class', 'cool-annotation-page-scroller');
	});

	it('A thread that the cursor selects lines up with its text', function() {
		clickCommentedText.call(this, '7');
		cy.cGet('#comment-arrow-container line').should('exist');
		cy.cGet('#comment-container-7').should(function (card) {
			var line = card[0].ownerDocument.querySelector('#comment-arrow-container line');
			expect(card[0].getBoundingClientRect().top, 'card top against the line to its text')
				.to.be.closeTo(line.getBoundingClientRect().top, 2);
		});
	});

	it('A click on a reply keeps the reply in place', function() {
		// The last reply of "Test line 2", far below the start of its thread.
		scrollBoxTo.call(this, 13);
		cy.cGet('#comment-container-13').then((card) => {
			var before = card[0].getBoundingClientRect().top;
			cy.cGet('#comment-container-13 .cool-annotation-content').click({ scrollBehavior: false });
			cy.cGet('#comment-container-7').should('have.class', 'annotation-active');
			cy.then(() => {
				return helper.processToIdle(this.win);
			});
			cardTop(13, function (top) {
				expect(top, 'top of the clicked reply').to.be.closeTo(before, 1);
			});
		});
	});

	it('A posted reply shows in the box', function() {
		// The last reply ends at the bottom of what the box shows, so the new
		// reply goes below it.
		scrollBoxToBottomOf.call(this, 18);
		openMenuAndChoose(18, 'Reply');
		cy.cGet('#annotation-reply-textarea-18').should('have.focus').type('a new reply');
		cy.cGet('#annotation-reply-18').click({ scrollBehavior: false });
		cy.then(() => {
			return helper.processToIdle(this.win);
		});
		cy.cGet('#document-container').then((container) => {
			var view = container[0].getBoundingClientRect();
			pageBox().contains('.cool-annotation', 'a new reply').should(function (card) {
				var rect = card[0].getBoundingClientRect();
				var box = card[0].parentElement.getBoundingClientRect();
				expect(rect.top, 'reply top').to.be.at.least(Math.max(box.top, view.top));
				expect(rect.bottom, 'reply bottom').to.be.at.most(Math.min(box.bottom, view.bottom) + 1);
			});
		});
	});

	it('Removing the last reply keeps the reply above it in place', function() {
		// The box shows reply 17 and the top of reply 18, which runs on below
		// what the box shows.
		clickCommentedText.call(this, '14');
		scrollBoxTo.call(this, 17);
		cy.cGet('#comment-container-17').then((card) => {
			var before = card[0].getBoundingClientRect().top;
			openMenuAndChoose(18, 'Remove');
			cy.cGet('#comment-container-18').should('not.exist');
			cy.then(() => {
				return helper.processToIdle(this.win);
			});
			cardTop(17, function (top) {
				expect(top, 'top of the reply above the removed one').to.be.closeTo(before, 1);
			});
		});
	});

	it('A resolved thread leaves the box while resolved comments are hidden', function() {
		cy.then(() => {
			this.win.app.map.showResolvedComments(false);
			return helper.processToIdle(this.win);
		});
		cy.then(() => {
			var section = commentSection(this.win);
			section.resolve(section.getComment('7'));
		});
		cy.cGet('#comment-container-7').parent().should('not.have.class', 'cool-annotation-page-scroller');
		// The box ends at the last card it shows.
		pageBox().should(function (box) {
			var cards = Array.from(box[0].children).filter(function (child) {
				return child.classList.contains('cool-annotation');
			});
			var end = Math.max.apply(null, cards.map(function (card) {
				return card.offsetTop + card.offsetHeight;
			}));
			expect(box[0].scrollHeight, 'box content height').to.be.at.most(end + 1);
		});
	});
});
