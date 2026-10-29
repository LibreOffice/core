/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// Set how long the label waits, in milliseconds, in the given window. The label reads these
// times on each update, so they apply to a label that already shows.
function setTimes(win, times) {
	cy.then(function () {
		Object.assign(win.app.definitions.shapeLockLabelSection, times);
	});
}

// Show the second slide, whose outline placeholder covers the center of the slide.
function goToSecondSlide(win) {
	cy.cGet('#preview-img-part-1').click();
	helper.processToIdle(win);
	cy.wrap(win).its('app.map._docLayer._selectedPart').should('equal', 1);
}

// Select the outline shape and press Enter to edit its text.
function editShape(win) {
	impressHelper.clickCenterOfSlide();
	helper.waitForTimers(win, 'clicktimer');
	helper.processToIdle(win);
	cy.cGet('#test-div-shape-handle-rotation').should('exist');
	helper.typeIntoDocument('{enter}');
	helper.processToIdle(win);
}

// The label shows the avatar of the user of the other window, with the given status.
function assertLabel(win, otherWin, status) {
	cy.cGet('.shape-lock-label').should('be.visible');
	cy.cGet('.shape-lock-label .shape-lock-label-status').should('have.text', status);
	cy.cGet('.shape-lock-label .visuallyhidden').should(function (name) {
		var otherName = win.app.map.getViewName(otherWin.app.map._docLayer._viewId);
		expect(otherName).to.be.a('string').and.not.be.empty;
		expect(name.text()).to.equal(otherName);
	});
}

// The snackbar shows the given message, with %1 for the name of the user of the other window.
// The messages have no action button, so the label has the id of a label without one.
function assertMessage(win, otherWin, text) {
	cy.cGet('#snackbar-container #label-no-action').should(function (label) {
		var otherName = win.app.map.getViewName(otherWin.app.map._docLayer._viewId);
		expect(label.text()).to.equal(text.replace('%1', otherName));
	});
}

// Long enough that no countdown or phase change happens while a test runs.
var longTime = 600000;

// A shape whose text one user edits is locked for the other users. They see a label with the
// avatar of that user and how recently that user typed. Once that user is active, Take over asks
// that user for the shape. Once that user is inactive, Take over gives the shape at once, and it
// can be given back for a short time.
describe(['tagmultiuser'], 'Impress label of the user who edits a shape', function () {
	var win1, win2;

	beforeEach(function () {
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp', true);
		cy.getFrameWindow('#iframe1').then(function (win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function (win) { win2 = win; });
	});

	// User A edits the text of the outline shape on the second slide, and user B shows that slide.
	// The active frame is the one of user B afterwards.
	function startWithUserAEditing() {
		cy.cSetActiveFrame('#iframe2');
		goToSecondSlide(win2);

		cy.cSetActiveFrame('#iframe1');
		goToSecondSlide(win1);
		editShape(win1);
		impressHelper.assertInTextEditMode();

		cy.cSetActiveFrame('#iframe2');
		helper.processToIdle(win2);
	}

	it('while a user types, the others cannot take the shape over', function () {
		setTimes(win2, { activeAfter: longTime, inactiveAfter: longTime });
		startWithUserAEditing();

		// User B sees that user A is editing, with no way to take the shape over.
		assertLabel(win2, win1, 'is editing');
		cy.cGet('.shape-lock-label .shape-lock-button').should('not.be.visible');

		// User B tries to edit the shape, and is told why that is not possible.
		editShape(win2);
		assertMessage(win2, win1, 'You can\'t edit right now, %1 is actively editing.');
		cy.cGet('.leaflet-cursor.blinking-cursor').should('not.exist');
	});

	it('a user who cannot edit sees who edits the shape, with no way to take it over', function () {
		setTimes(win2, { activeAfter: 0, inactiveAfter: 0 });
		cy.then(function () {
			win2.app.map.setPermission('readonly');
		});
		startWithUserAEditing();

		assertLabel(win2, win1, 'is inactive');
		cy.cGet('.shape-lock-label .shape-lock-button').should('not.be.visible');
	});

	it('an active user can reject a request, and gets one without an answer taken over', function () {
		// User A is told about a take over without a request only after undoTime, so the message
		// that comes at once below shows that the take over answers the request.
		setTimes(win1, { activeAfter: longTime, approvalTime: longTime, undoTime: longTime });
		setTimes(win2, { activeAfter: 0, inactiveAfter: longTime, approvalTime: longTime });
		startWithUserAEditing();

		// User B asks user A for the shape, and waits for the answer.
		assertLabel(win2, win1, 'is active');
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible').click();
		helper.processToIdle(win2);
		cy.cGet('.shape-lock-label .shape-lock-button').should('not.be.visible');
		cy.cGet('.shape-lock-label .shape-lock-label-wait').should('be.visible')
			.and('contain.text', 'Take over in');

		// User A sees the request and rejects it.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-callout').should('be.visible').and('contain.text', 'wants to edit');
		cy.cGet('.shape-lock-callout .shape-lock-button').should('have.text', 'Reject').click();
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-callout').should('not.exist');

		// User B is told so, and can ask again.
		cy.cSetActiveFrame('#iframe2');
		helper.processToIdle(win2);
		assertMessage(win2, win1, '%1 rejected your request.');
		cy.cGet('.shape-lock-label .shape-lock-label-wait').should('not.be.visible');
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible');

		// User B asks again, and user A does not answer, so user B edits the shape once the wait
		// is over.
		setTimes(win2, { approvalTime: 1000 });
		cy.cGet('.shape-lock-label .shape-lock-button').click();
		helper.processToIdle(win2);
		cy.cGet('.shape-lock-label').should('not.exist');
		cy.cGet('.leaflet-cursor.blinking-cursor').should('exist');

		// User A is told that user B edits the shape now, and sees the label of user B.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-callout').should('not.exist');
		assertMessage(win1, win2, '%1 has taken over editing. Your changes were saved.');
		assertLabel(win1, win2, 'is editing');
	});

	it('an inactive user loses the shape at once, and gets it back on Cancel', function () {
		setTimes(win1, { undoTime: longTime });
		setTimes(win2, { activeAfter: 0, inactiveAfter: 0, undoTime: longTime });
		startWithUserAEditing();

		// User B takes the shape over at once, and can give it back.
		assertLabel(win2, win1, 'is inactive');
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible').click();
		helper.processToIdle(win2);
		cy.cGet('.shape-lock-label').should('not.exist');
		cy.cGet('.leaflet-cursor.blinking-cursor').should('exist');
		cy.cGet('.shape-lock-callout').should('be.visible')
			.and('contain.text', 'You took over. Made a mistake?');

		// User B gives the shape back. User A edits it again, and user B sees the label again.
		cy.cGet('.shape-lock-callout .shape-lock-button').should('have.text', 'Cancel').click();
		helper.processToIdle(win2);
		cy.cGet('.shape-lock-callout').should('not.exist');
		assertLabel(win2, win1, 'is inactive');

		// User A edits the shape again.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.wrap(win1).its('app.file.textCursor.visible').should('be.true');
		cy.cGet('.shape-lock-label').should('not.exist');
	});

	it('a user who lost the shape is told about the changes once the other user leaves it', function () {
		setTimes(win1, { undoTime: 0 });
		setTimes(win2, { activeAfter: 0, inactiveAfter: 0, undoTime: 0 });
		startWithUserAEditing();

		// User B takes the shape over and types into it. User A is told about the take over.
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible').click();
		helper.processToIdle(win2);
		cy.cGet('.leaflet-cursor.blinking-cursor').should('exist');
		helper.typeIntoDocument('x');
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		assertMessage(win1, win2, '%1 has taken over editing. Your changes were saved.');
		// User A may have been away, so the message stays until user A dismisses it.
		cy.cGet('#snackbar-container #snackbar-dismiss-button').should('be.visible');

		// User B leaves the text, and user A is told that user B made changes.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{esc}');
		helper.processToIdle(win2);

		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-label').should('not.exist');
		assertMessage(win1, win2, '%1 made changes.');
	});
});

describe(['tagmultiuser'], 'Impress label of the user who edits a shape, with three users', function () {
	var win1, win2, win3;

	beforeEach(function () {
		// A third user opens the document in iframe3. The frames share the width, so a wider window
		// gives each frame the width it has with two users.
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp', true, false, undefined,
			'userid3=test3');
		cy.viewport(3000, 660);
		cy.cSetActiveFrame('#iframe3');
		helper.documentChecks(true);
		cy.getFrameWindow('#iframe1').then(function (win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function (win) { win2 = win; });
		cy.getFrameWindow('#iframe3').then(function (win) { win3 = win; });
	});

	// The user of the given frame asks for the shape, and waits for the answer.
	function askForShape(frame, win) {
		cy.cSetActiveFrame(frame);
		helper.processToIdle(win);
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible').click();
		helper.processToIdle(win);
		cy.cGet('.shape-lock-label .shape-lock-label-wait').should('be.visible');
	}

	// The user of the given frame was told about the rejection, and can ask again.
	function assertRejected(frame, win) {
		cy.cSetActiveFrame(frame);
		helper.processToIdle(win);
		assertMessage(win, win1, '%1 rejected your request.');
		cy.cGet('.shape-lock-label .shape-lock-label-wait').should('not.be.visible');
		cy.cGet('.shape-lock-label .shape-lock-button').should('be.visible');
	}

	it('a rejection reaches every user who asked for the shape', function () {
		setTimes(win1, { activeAfter: longTime, approvalTime: longTime });
		setTimes(win2, { activeAfter: 0, inactiveAfter: longTime, approvalTime: longTime });
		setTimes(win3, { activeAfter: 0, inactiveAfter: longTime, approvalTime: longTime });

		cy.cSetActiveFrame('#iframe2');
		goToSecondSlide(win2);
		cy.cSetActiveFrame('#iframe3');
		goToSecondSlide(win3);
		cy.cSetActiveFrame('#iframe1');
		goToSecondSlide(win1);
		editShape(win1);
		impressHelper.assertInTextEditMode();

		// User B asks for the shape, then user C.
		askForShape('#iframe2', win2);
		askForShape('#iframe3', win3);

		// User A sees the request of user C, the last one, and rejects it.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-callout .visuallyhidden').should(function (name) {
			expect(name.text()).to.equal(win1.app.map.getViewName(win3.app.map._docLayer._viewId));
		});
		cy.cGet('.shape-lock-callout .shape-lock-button').should('have.text', 'Reject').click();
		helper.processToIdle(win1);
		cy.cGet('.shape-lock-callout').should('not.exist');

		// Both users are told, so neither takes the shape once its wait is over.
		assertRejected('#iframe2', win2);
		assertRejected('#iframe3', win3);
	});
});
