/* global describe it cy beforeEach require Cypress expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress hyperlink popup tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/top_toolbar.odp');
		desktopHelper.switchUIToCompact();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebar();
		} else {
			desktopHelper.hideSidebarImpress();
		}

		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});

		impressHelper.selectTextShapeInTheCenter();
		impressHelper.dblclickOnSelectedShape();

		helper.typeIntoDocument('{ctrl}a');
		helper.typeIntoDocument('{del}');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument('before ');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument('{ctrl}k');
		cy.cGet('#target-input').should('be.visible');
		cy.cGet('#indication-input').clear();
		cy.cGet('#indication-input').type('linktext');
		cy.cGet('#target-input').type('http://www.example.com/');
		cy.cGet('#ok').click();
		cy.cGet('#target-input').should('not.exist');
		cy.then(() => helper.processToIdle(this.win));

		// InsertURLField selects the newly inserted field. Press right
		// arrow to collapse the selection past the field so that typing
		// does not replace the URL field.
		helper.typeIntoDocument('{rightArrow}');
		cy.then(() => helper.processToIdle(this.win));

		helper.typeIntoDocument(' after');
		cy.then(() => helper.processToIdle(this.win));
	});

	it('Popup appears when cursor is at the start of hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Move right to reach the start of the hyperlink field.
		// "before " is 7 characters, so 7 right-arrow presses
		// lands at the field boundary.
		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up').should('have.text', 'http://www.example.com/');
	});

	it('Popup appears when cursor is at the end of hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Move right past the hyperlink field. "before " is 7 chars,
		// the field is 1 dummy char, so 8 right-arrow presses lands
		// just after the field.
		for (var i = 0; i < 8; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up').should('have.text', 'http://www.example.com/');
	});

	it('Copy button writes the URL directly to the clipboard.', function() {
		cy.getFrameWindow().then(function(win) {
			cy.stub(win.navigator.clipboard, 'writeText').as('writeText');
		});

		helper.typeIntoDocument('{home}');
		cy.then(() => helper.processToIdle(this.win));

		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		cy.then(() => helper.processToIdle(this.win));

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up-copy').click();

		cy.get('@writeText').should('have.been.calledOnceWith', 'http://www.example.com/');
	});

	it('In readonly mode, edit and remove buttons are hidden and copy button is visible.', function() {
		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('readonly');
		});

		helper.typeIntoDocument('{home}');
		cy.then(() => helper.processToIdle(this.win));

		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		cy.then(() => helper.processToIdle(this.win));

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');
		cy.cGet('#hyperlink-pop-up-copy').should('be.visible');
		cy.cGet('#hyperlink-pop-up-edit').should('not.be.visible');
		cy.cGet('#hyperlink-pop-up-remove').should('not.be.visible');
	});

	it('Popup disappears after navigating away from hyperlink.', function() {
		helper.typeIntoDocument('{home}');
		helper.processToIdle(this.win);

		// Navigate into the hyperlink field.
		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('be.visible');

		// Navigate past the field and the trailing text to a position
		// that is clearly outside the hyperlink.
		helper.typeIntoDocument('{end}');
		helper.processToIdle(this.win);

		cy.cGet('.hyperlink-pop-up-container').should('not.exist');
	});

	// Filled in by recordNextAuxclick before each middle click.
	var auxclickRecord = null;

	// Record whether the next middle-button click reaches the page, and
	// whether it arrives cancelled. A middle click consumed on the canvas
	// never shows up here; one left to the browser shows up uncancelled.
	function recordNextAuxclick(win) {
		cy.then(() => {
			auxclickRecord = { seen: false, prevented: false };
			win.document.addEventListener('auxclick', function(e) {
				auxclickRecord.seen = true;
				auxclickRecord.prevented = e.defaultPrevented;
			}, { once: true });
		});
	}


	it('Middle click on the hyperlink opens the link.', function() {
		// Find the on-screen middle of the URL field from the caret
		// positions at its two ends.
		helper.typeIntoDocument('{home}');
		cy.then(() => helper.processToIdle(this.win));
		for (var i = 0; i < 7; i++) {
			helper.typeIntoDocument('{rightArrow}');
		}
		cy.then(() => helper.processToIdle(this.win));
		helper.getBlinkingCursorPosition('fieldStart');

		helper.typeIntoDocument('{rightArrow}');
		cy.then(() => helper.processToIdle(this.win));
		helper.getBlinkingCursorPosition('fieldEnd');

		cy.get('@fieldStart').then((start) => {
			cy.get('@fieldEnd').then((end) => {
				cy.wrap({
					x: (start.x + end.x) / 2,
					y: (start.y + end.y) / 2,
				}).as('linkPos');
			});
		});

		// Leave text edit; over an unmarked shape the engine reports the
		// hand pointer for the hyperlink under the mouse.
		impressHelper.removeShapeSelection();

		// Hover the link until the engine reports the hand pointer for it.
		cy.get('@linkPos').then((point) => {
			cy.cGet('body').realMouseMove(point.x, point.y);
		});
		cy.cGet('#document-canvas').should('have.css', 'cursor', 'pointer');

		recordNextAuxclick(this.win);
		cy.get('@linkPos').then((point) => {
			cy.cGet('body').realClick({ x: point.x, y: point.y, button: 'middle' });
		});

		// The external-link confirmation shows the link's URL.
		cy.cGet('#openlink-response').should('exist');
		cy.cGet('[id^="info-modal-label2"]').should('have.text', 'http://www.example.com/');

		// The click was consumed on the canvas, so the browser saw no
		// middle click to paste from.
		cy.then(() => {
			expect(auxclickRecord.seen).to.be.false;
		});
	});

	it('Middle click away from the hyperlink is left to the browser.', function() {
		impressHelper.removeShapeSelection();

		// A spot near the top-left corner of the view, away from the shape.
		cy.cGet('#document-canvas').then(function(items) {
			var rect = items[0].getBoundingClientRect();
			cy.wrap({ x: rect.left + 20, y: rect.top + 20 }).as('awayPos');
		});

		// Hover the spot; the engine does not report the hand pointer
		// there. The pointer only updates on change, so the canvas may
		// keep its initial cursor rather than settle on a specific one.
		cy.get('@awayPos').then((point) => {
			cy.cGet('body').realMouseMove(point.x, point.y);
		});
		cy.cGet('#document-canvas').should(($canvas) => {
			expect($canvas[0].style.cursor).to.not.equal('pointer');
		});

		recordNextAuxclick(this.win);
		cy.get('@awayPos').then((point) => {
			cy.cGet('body').realClick({ x: point.x, y: point.y, button: 'middle' });
		});

		// The click reaches the page uncancelled, so pasting the primary
		// selection stays available to the browser.
		cy.then(() => {
			expect(auxclickRecord.seen).to.be.true;
			expect(auxclickRecord.prevented).to.be.false;
		});

		// And no link dialog opens for it.
		cy.then(() => helper.processToIdle(this.win));
		cy.cGet('#openlink-response').should('not.exist');
	});
});
