/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

var dimDialogSelector = '#modal-dialog-inactive_user_message-overlay';

// The position of the first other view's cursor drawn in the frame window.
function otherViewCursorPosition(win) {
	var sections = win.app.sectionContainer.sections.filter(function(section) {
		return section.name.startsWith('OtherViewCursor ');
	});
	expect(sections.length).to.be.greaterThan(0);
	return sections[0].position.slice();
}

describe(['tagmultiuser'], 'Idle view in a shared document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/simultaneous_typing.odt', true);
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow('#iframe1').then(function(win) {
			this.win1 = win;
		});
		cy.getFrameWindow('#iframe2').then(function(win) {
			this.win2 = win;
		});
	});

	it('An idle view stops getting updates until its user comes back', function() {
		// The second user moves first, so the first view draws that cursor.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{rightArrow}');
		helper.processToIdle(this.win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(this.win1);
		cy.getFrameWindow().then(function(win) {
			this.cursorBeforeIdle = otherViewCursorPosition(win);

			// Record the activity messages this view sends and the other view's
			// cursor updates it receives.
			win.idleTest = { viewCursors: 0, activity: [] };
			win.app.socket.socket.addEventListener('message', function(event) {
				if (typeof event.data === 'string' && event.data.startsWith('invalidateviewcursor:'))
					win.idleTest.viewCursors++;
			});
			var send = win.app.socket.socket.send.bind(win.app.socket.socket);
			win.app.socket.socket.send = function(message) {
				if (message === 'useractive' || message === 'userinactive')
					win.idleTest.activity.push(message);
				return send(message);
			};

			// The view counts as idle after a second instead of the configured minutes.
			win.idleTimeoutSecs = 1;
		}.bind(this));

		// The idle popup comes up and the view tells the server it is away.
		cy.cGet(dimDialogSelector).should('exist');
		helper.waitUntilLayoutingIsIdle(this.win1);
		cy.getFrameWindow().should(function(win) {
			expect(win.app.idleHandler._active).to.be.false;
			expect(win.idleTest.activity).to.deep.equal(['userinactive']);
		});

		// The second user keeps working.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{rightArrow}{rightArrow}{rightArrow}');
		helper.processToIdle(this.win2);
		// The idle view answers no round trip, so this is a check that nothing arrives
		// within a time: the other view's input was processed above, and anything sent to
		// the idle view was on its way before that answer came back.
		cy.wait(1000);

		// The idle view saw none of it and still shows the popup.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet(dimDialogSelector).should('exist');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.idleHandler._active).to.be.false;
			expect(win.idleTest.activity).to.deep.equal(['userinactive']);
			expect(win.idleTest.viewCursors).to.equal(0);
			expect(otherViewCursorPosition(win)).to.deep.equal(this.cursorBeforeIdle);
		}.bind(this));

		// The first user comes back: the popup goes and the missed state arrives.
		cy.getFrameWindow().then(function(win) {
			win.idleTimeoutSecs = 100000;
			win.app.idleHandler.notifyActive();
		});
		cy.cGet(dimDialogSelector).should('not.exist');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.idleHandler._active).to.be.true;
			expect(win.idleTest.activity).to.deep.equal(['userinactive', 'useractive']);
			expect(win.idleTest.viewCursors).to.be.greaterThan(0);
			expect(otherViewCursorPosition(win)).to.not.deep.equal(this.cursorBeforeIdle);
		}.bind(this));
	});
});
