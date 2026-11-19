/* global describe expect it cy beforeEach require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer link popup while the mouse selects text', function () {

	beforeEach(function () {
		// The first paragraph is "Visit the Collabora website for details.", with a link on
		// "the Collabora website".
		helper.setupAndLoadDocument('writer/a11y_hyperlink.fodt');
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

	it('shows no popup until the button comes up, then the context toolbar', function () {
		let start;
		let onLink;
		let nearLinkStart;
		// The caret at two places in the link gives the point to drag to and the point to click.
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(8));
		cy.cGet('#hyperlink-pop-up').should('be.visible');
		helper.processToIdle(this.win);
		cy.then(() => {
			nearLinkStart = caretPoint(this.win);
		});
		helper.typeIntoDocument('{rightarrow}'.repeat(4));
		helper.processToIdle(this.win);
		cy.then(() => {
			onLink = caretPoint(this.win);
		});
		helper.typeIntoDocument('{ctrl}{home}');
		cy.cGet('#hyperlink-pop-up').should('not.exist');
		helper.processToIdle(this.win);
		cy.then(() => {
			start = caretPoint(this.win);
			cy.cGet('body').realMouseDown({ x: start.x + 2, y: start.y });
		});
		helper.processToIdle(this.win);

		// A single realMouseMove can be dropped under CI load, so send the move again until
		// the caret reaches the link with text selected. The target is a fixed point.
		cy.then(() => {
			helper.retryUntil(
				() => { cy.cGet('body').realMouseMove(onLink.x, onLink.y); },
				() => {
					const link = this.win.app.map.hyperlinkUnderCursor;
					return this.win.TextSelections.isActive() && !!link && !!link.link;
				});
		});
		helper.processToIdle(this.win);

		// The caret is in the link while the button is held, and the popup stays closed.
		cy.cGet('#hyperlink-pop-up').should('not.exist');

		cy.then(() => {
			cy.cGet('body').realMouseUp({ x: onLink.x, y: onLink.y });
		});
		helper.processToIdle(this.win);

		cy.cGet('#context-toolbar').should('be.visible');
		cy.cGet('#hyperlink-pop-up').should('not.exist');
		cy.getFrameWindow().should((win) => {
			expect(win.TextSelections.isActive(), 'the text stays selected').to.equal(true);
		});

		// A plain click on the link still opens its popup.
		helper.typeIntoDocument('{ctrl}{home}');
		cy.getFrameWindow().should((win) => {
			expect(win.TextSelections.isActive(), 'no text selected').to.equal(false);
		});
		helper.waitForTimers(this.win, 'clicktimer');
		cy.then(() => {
			cy.cGet('body').realClick({ x: nearLinkStart.x, y: nearLinkStart.y });
		});
		helper.processToIdle(this.win);
		cy.cGet('#hyperlink-pop-up').should('be.visible');
	});
});
