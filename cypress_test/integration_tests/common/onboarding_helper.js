/* -*- js-indent-level: 8 -*- */
/* global cy expect */

// Helpers for the tests of the tour that introduces the user interface.

// The tour remembers per document type that it has already been shown. The
// document type names are the ones the application uses: 'text',
// 'spreadsheet', 'presentation' and 'drawing'.
function tourSeenKey(docType) {
	return docType + '.onboardingTourSeen';
}

// Open the tour from its first card and wait until that card is on screen.
function startTour(win) {
	cy.log('>> startTour - start');

	cy.then(function() {
		win.app.onboardingTour.start();
	});
	cy.cGet('#modal-dialog-onboarding-tour').should('be.visible');

	cy.log('<< startTour - end');
}

// The card on screen is the one named by stepId.
function assertStep(win, stepId) {
	cy.cGet('#modal-dialog-onboarding-tour').should('be.visible');
	cy.wrap(null).should(function() {
		expect(win.app.onboardingTour.currentStepId()).to.equal(stepId);
	});
}

// Move forward until the card on screen is the one named by stepId. A tour
// that never reaches that card fails here, after maxSteps cards, rather than
// walking off the end of the tour.
function stepForwardTo(win, stepId, maxSteps) {
	cy.cGet('#modal-dialog-onboarding-tour').should('be.visible').then(function() {
		if (win.app.onboardingTour.currentStepId() === stepId)
			return;

		expect(maxSteps, 'cards left while looking for the ' + stepId + ' card').to.be.greaterThan(0);

		cy.cGet('#onboarding-next-button').click();
		stepForwardTo(win, stepId, maxSteps - 1);
	});
}

// Once the tour has been left, none of it is on screen any more and the
// document is the user's again.
function assertTourEnded(win) {
	cy.cGet('#modal-dialog-onboarding-tour').should('not.exist');
	cy.cGet('#modal-dialog-onboarding-tour-overlay').should('not.exist');
	cy.cGet('.onboarding-highlight').should('not.exist');
	cy.then(function() {
		expect(win.app.onboardingTour.isRunning()).to.be.false;
	});
}

// The highlight is on the element the card is talking about, and on nothing
// else.
function assertHighlighted(selector) {
	cy.log('>> assertHighlighted - start');

	cy.cGet(selector).should('have.class', 'onboarding-highlight');
	cy.cGet('.onboarding-highlight').should('have.length', 1);

	cy.log('<< assertHighlighted - end');
}

module.exports.tourSeenKey = tourSeenKey;
module.exports.startTour = startTour;
module.exports.assertStep = assertStep;
module.exports.stepForwardTo = stepForwardTo;
module.exports.assertTourEnded = assertTourEnded;
module.exports.assertHighlighted = assertHighlighted;
