/* global describe expect it cy beforeEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Writer internal link jump', function () {

	beforeEach(function () {
		// The first paragraph has a link to the bookmark "target" at the start of page 3.
		helper.setupAndLoadDocument('writer/internal_link_jump.fodt', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// The middle of the caret, in the coordinates of the document frame.
	function caretPoint(win) {
		const rectangle = win.app.file.textCursor.rectangle;
		const point = new win.cool.SimplePoint(rectangle.x1, rectangle.y1 + rectangle.height / 2);
		const canvas = win.app.sectionContainer.getCanvasBoundingClientRect();
		const anchor = win.app.sectionContainer.getDocumentAnchor();
		return {
			x: canvas.left + (anchor[0] + point.vX) / win.app.dpiScale,
			y: canvas.top + (anchor[1] + point.vY) / win.app.dpiScale,
		};
	}

	function caretIsInView(win) {
		const rectangle = win.app.file.textCursor.rectangle;
		return win.app.isPointVisibleInTheDisplayedArea([rectangle.x1, rectangle.y1]);
	}

	it('shows the target on a second Ctrl+click when the caret is already there', function () {
		let onLink;
		let linkY;
		// The caret inside the link text gives the point to click.
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(8));
		cy.cGet('#hyperlink-pop-up').should('be.visible');
		helper.processToIdle(this.win);
		cy.then(() => {
			onLink = caretPoint(this.win);
			linkY = this.win.app.file.textCursor.rectangle.y1;
		});

		// The first Ctrl+click moves the caret to the target on page 3, and the view goes there.
		cy.then(() => {
			cy.cGet('body').realClick({ x: onLink.x, y: onLink.y, ctrlKey: true });
		});
		helper.processToIdle(this.win);
		cy.getFrameWindow().should((win) => {
			expect(win.app.file.textCursor.rectangle.y1, 'the caret is at the target')
				.to.be.greaterThan(linkY + 20000);
			expect(caretIsInView(win), 'the target is in view').to.equal(true);
		});

		// Scrolling back up leaves the caret at the target, out of view.
		desktopHelper.scrollWriterDocumentToTop();
		helper.processToIdle(this.win);
		cy.getFrameWindow().should((win) => {
			expect(caretIsInView(win), 'the target is out of view').to.equal(false);
		});

		// The second Ctrl+click on the same link puts the caret where it already is, and the view
		// still goes to the target.
		helper.waitForTimers(this.win, 'clicktimer');
		cy.then(() => {
			cy.cGet('body').realClick({ x: onLink.x, y: onLink.y, ctrlKey: true });
		});
		helper.processToIdle(this.win);
		cy.getFrameWindow().should((win) => {
			expect(caretIsInView(win), 'the target is in view again').to.equal(true);
		});
	});
});
