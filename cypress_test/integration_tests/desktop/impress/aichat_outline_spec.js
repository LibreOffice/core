/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI deck outline card', function() {

	// A single-slide presentation: a fresh deck, the case where the backend
	// proposes a slide outline for the user to edit before the deck is built.
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.fodp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	var outline = {
		title: 'Renewable Energy',
		slides: [
			{ part: 'opening', intent: 'title', title: 'Renewable Energy',
				gist: 'Set the stage for the whole deck.' },
			{ part: 'body', intent: 'bullets', title: 'Solar Power',
				gist: 'Cover photovoltaic basics.' },
			{ part: 'closing', intent: 'closing', title: 'Thank You',
				gist: 'Wrap up and invite questions.' },
		],
	};

	// Send a deck request; the backend answers with an outline card.
	function requestOutline(win) {
		aichatHelper.enableAIWithOutlineSocket(win, { outline: outline });
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Create a deck about renewable energy');
		aichatHelper.clickSend();
		cy.cGet('.aichat-outline-card').should('exist');
	}

	it('The card shows one editable row per proposed slide', function() {
		requestOutline(this.win);

		// One row per slide, each numbered, with a title input and a delete
		// button; the inputs carry the proposed titles.
		cy.cGet('.aichat-outline-row').should('have.length', 3);
		cy.cGet('.aichat-outline-row').eq(0)
			.find('.aichat-outline-num').should('have.text', '1.');
		cy.cGet('.aichat-outline-row').eq(2)
			.find('.aichat-outline-num').should('have.text', '3.');
		cy.cGet('input.aichat-outline-title').should('have.length', 3);
		cy.cGet('button.aichat-outline-delete').should('have.length', 3);
		cy.cGet('.aichat-outline-row').eq(0)
			.find('.aichat-outline-title').should('have.value', 'Renewable Energy');
		cy.cGet('.aichat-outline-row').eq(1)
			.find('.aichat-outline-title').should('have.value', 'Solar Power');
	});

	it('Approving sends the edited outline with a deleted slide dropped', function() {
		requestOutline(this.win);

		// Rename the first slide and remove the middle one, then approve.
		cy.cGet('.aichat-outline-row').eq(0).find('.aichat-outline-title')
			.clear().type('Clean Energy Today');
		cy.cGet('.aichat-outline-row').eq(1).find('.aichat-outline-delete').click();
		cy.cGet('.aichat-outline-row').should('have.length', 2);
		cy.cGet('.aichat-approve-btn').click();

		// The approved outline keeps DOM order, carries the edited title, and
		// no longer contains the deleted slide.
		cy.wrap(null).should(() => {
			expect(this.win.__aichatApprovePayloads).to.have.length(1);
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.action).to.equal('approve');
			expect(decision.outline.title).to.equal('Renewable Energy');
			expect(decision.outline.slides).to.have.length(2);
			expect(decision.outline.slides[0].title).to.equal('Clean Energy Today');
			expect(decision.outline.slides[0].part).to.equal('opening');
			expect(decision.outline.slides[1].title).to.equal('Thank You');
			var titles = decision.outline.slides.map((s) => s.title);
			expect(titles).to.not.include('Solar Power');
		});

		// The card collapses to a record of the decision.
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Outline approved - building 2 slides.');
		cy.cGet('.aichat-outline-card').should('not.exist');
	});

	it('An emptied title falls back to the proposed title', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-outline-row').eq(1).find('.aichat-outline-title').clear();
		cy.cGet('.aichat-approve-btn').click();

		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.outline.slides[1].title).to.equal('Solar Power');
		});
	});

	it('The gist of each slide is editable', function() {
		requestOutline(this.win);

		// The gists render as editable textareas carrying the proposed text.
		cy.cGet('textarea.aichat-outline-gist').should('have.length', 3);
		cy.cGet('.aichat-outline-row').eq(0)
			.find('.aichat-outline-gist')
			.should('have.value', 'Set the stage for the whole deck.');
	});

	it('An edited gist rides the approved outline', function() {
		requestOutline(this.win);

		// Rewrite the first slide's description; leave the others untouched.
		cy.cGet('.aichat-outline-row').eq(0).find('.aichat-outline-gist')
			.clear().type('Frame why renewables matter now.');
		cy.cGet('.aichat-approve-btn').click();

		// The edited description is sent, and untouched gists keep their text.
		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.outline.slides[0].gist)
				.to.equal('Frame why renewables matter now.');
			expect(decision.outline.slides[1].gist)
				.to.equal('Cover photovoltaic basics.');
		});
	});

	it('An emptied gist falls back to the proposed description', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-outline-row').eq(1).find('.aichat-outline-gist').clear();
		cy.cGet('.aichat-approve-btn').click();

		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.outline.slides[1].gist).to.equal('Cover photovoltaic basics.');
		});
	});

	it('Rejecting sends the plain decision and collapses the card', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-reject-btn').click();

		cy.wrap(null).should(() => {
			expect(this.win.__aichatApprovePayloads).to.have.length(1);
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.action).to.equal('reject');
			expect(decision.outline).to.be.undefined;
		});

		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Outline rejected.');
		cy.cGet('.aichat-outline-card').should('not.exist');
	});

	it('Progress updates while the deck builds, then the result lands', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-approve-btn').click();

		// The per-slide progress from the server replaces the initial text.
		cy.cGet('#aichat-progress-text').should(
			'contain.text', 'Building slide 1 of 2');

		// The built deck arrives once the server sends its final result.
		cy.then(() => {
			this.win.__deliverOutlineResult();
		});
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');
	});

	it('Adding a slide appends an empty numbered row and blocks approve', function() {
		requestOutline(this.win);

		// With three named slides the card is ready to approve.
		cy.cGet('.aichat-approve-btn').should('be.enabled');

		cy.cGet('.aichat-outline-add').click();

		// A fourth row appears, numbered 4, with an empty title and the title
		// and description placeholders inviting the user to fill them in.
		cy.cGet('.aichat-outline-row').should('have.length', 4);
		cy.cGet('.aichat-outline-row').eq(3)
			.find('.aichat-outline-num').should('have.text', '4.');
		cy.cGet('.aichat-outline-row').eq(3)
			.find('.aichat-outline-title')
			.should('have.value', '')
			.and('have.attr', 'placeholder', 'Slide title');
		cy.cGet('.aichat-outline-row').eq(3)
			.find('.aichat-outline-gist')
			.should('have.value', '')
			.and('have.attr', 'placeholder', 'What this slide should cover');

		// The untitled added slide holds approve back until it is named.
		cy.cGet('.aichat-approve-btn').should('be.disabled');
	});

	it('A titled added slide rides the approved outline at the end', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-outline-add').click();
		cy.cGet('.aichat-approve-btn').should('be.disabled');

		// Naming the added slide re-enables approve.
		cy.cGet('.aichat-outline-row').eq(3).find('.aichat-outline-title')
			.type('Wind Power');
		cy.cGet('.aichat-outline-row').eq(3).find('.aichat-outline-gist')
			.type('Cover turbines and offshore farms.');
		cy.cGet('.aichat-approve-btn').should('be.enabled').click();

		// The approved outline keeps DOM order, so the added slide lands last
		// as a body slide that uses bullet points, carrying the typed title
		// and description.
		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.outline.slides).to.have.length(4);
			var last = decision.outline.slides[3];
			expect(last.part).to.equal('body');
			expect(last.intent).to.equal('bullets');
			expect(last.title).to.equal('Wind Power');
			expect(last.gist).to.equal('Cover turbines and offshore farms.');
		});
	});

	it('Deleting the untitled added slide re-enables approve', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-outline-add').click();
		cy.cGet('.aichat-outline-row').should('have.length', 4);
		cy.cGet('.aichat-approve-btn').should('be.disabled');

		// Removing the still-empty added row leaves only named slides, so
		// approve becomes available again.
		cy.cGet('.aichat-outline-row').eq(3).find('.aichat-outline-delete').click();
		cy.cGet('.aichat-outline-row').should('have.length', 3);
		cy.cGet('.aichat-approve-btn').should('be.enabled');
	});

	it('The card and its edits come back after the sidebar is reopened', function() {
		requestOutline(this.win);

		// Edit a title and a description, remove a slide and add one, so the
		// card carries every kind of change a user can make.
		cy.cGet('.aichat-outline-row').eq(0).find('.aichat-outline-title')
			.clear().type('Clean Energy Today');
		cy.cGet('.aichat-outline-row').eq(0).find('.aichat-outline-gist')
			.clear().type('Frame why renewables matter now.');
		cy.cGet('.aichat-outline-row').eq(1).find('.aichat-outline-delete').click();
		cy.cGet('.aichat-outline-add').click();
		cy.cGet('.aichat-outline-row').eq(2).find('.aichat-outline-title')
			.type('Wind Power');
		cy.cGet('.aichat-outline-row').should('have.length', 3);

		aichatHelper.closeAIChat();
		aichatHelper.openAIChat();

		// The card is rebuilt with the rows as they were left, renumbered, and
		// approve is still offered.
		cy.cGet('.aichat-outline-card').should('exist');
		cy.cGet('.aichat-outline-row').should('have.length', 3);
		cy.cGet('.aichat-outline-row').eq(0)
			.find('.aichat-outline-title').should('have.value', 'Clean Energy Today');
		cy.cGet('.aichat-outline-row').eq(0)
			.find('.aichat-outline-gist')
			.should('have.value', 'Frame why renewables matter now.');
		cy.cGet('.aichat-outline-row').eq(1)
			.find('.aichat-outline-title').should('have.value', 'Thank You');
		cy.cGet('.aichat-outline-row').eq(2)
			.find('.aichat-outline-title').should('have.value', 'Wind Power');
		cy.cGet('.aichat-outline-row').eq(2)
			.find('.aichat-outline-num').should('have.text', '3.');
		cy.cGet('.aichat-approve-btn').should('be.enabled');

		// Approving after the reopen sends exactly the edited outline.
		cy.cGet('.aichat-approve-btn').click();
		cy.wrap(null).should(() => {
			expect(this.win.__aichatApprovePayloads).to.have.length(1);
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.action).to.equal('approve');
			var titles = decision.outline.slides.map((s) => s.title);
			expect(titles).to.deep.equal(
				['Clean Energy Today', 'Thank You', 'Wind Power']);
			expect(decision.outline.slides[0].gist)
				.to.equal('Frame why renewables matter now.');
		});
	});

	it('A decided outline is not offered a second time after a reopen', function() {
		requestOutline(this.win);

		cy.cGet('.aichat-approve-btn').click();
		cy.cGet('.aichat-outline-card').should('not.exist');

		aichatHelper.closeAIChat();
		aichatHelper.openAIChat();

		// The transcript keeps the outcome text and offers no second decision.
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Outline approved - building 3 slides.');
		cy.cGet('.aichat-outline-card').should('not.exist');
		cy.cGet('.aichat-approve-btn').should('not.exist');
	});
});
