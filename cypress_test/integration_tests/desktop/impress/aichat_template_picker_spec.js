/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI design picker on the deck decision cards', function() {

	// A single-slide presentation: a fresh deck, the case where a deck is
	// about to be built and a design can still be chosen for it.
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.fodp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	var outline = {
		title: 'Cats',
		slides: [
			{ part: 'opening', intent: 'title', title: 'Cats',
				gist: 'Set the stage.' },
			{ part: 'closing', intent: 'closing', title: 'Thanks',
				gist: 'Wrap up.' },
		],
	};

	function sendRequest(text) {
		aichatHelper.typeIntoAIInput(text);
		aichatHelper.clickSend();
	}

	it('A question goes out with no design picker in the way', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {});
		aichatHelper.openAIChat();
		sendRequest('What does this slide say?');

		// The request goes out straight away and is answered; no picker
		// appears anywhere in the conversation.
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');
		cy.then(() => {
			expect(this.win.__aichatPayloads).to.have.length(1);
			expect(this.win.__aichatPayloads[0].designTemplate).to.be.undefined;
		});
		cy.cGet('.aichat-design-section').should('not.exist');
	});

	it('The outline card offers the designs and the pick rides the approval', function() {
		aichatHelper.enableAIWithOutlineSocket(this.win, { outline: outline });
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		// The outline card carries the design section, with the plain option
		// preselected and the catalog loaded from the document.
		cy.cGet('.aichat-outline-card .aichat-design-section').should('exist');
		cy.cGet('.aichat-outline-card')
			.children()
			.first()
			.should('have.class', 'aichat-design-section');
		cy.cGet('.aichat-template-skip[aria-pressed="true"]').should('exist');
		cy.cGet('.aichat-template-card[title="Cobalt"]').should('exist');

		cy.cGet('.aichat-template-card[title="Cobalt"]').click();
		cy.cGet('.aichat-template-card[title="Cobalt"]')
			.should('have.attr', 'aria-pressed', 'true');
		cy.cGet('.aichat-approve-btn').click();

		cy.wrap(null).should(() => {
			expect(this.win.__aichatApprovePayloads).to.have.length(1);
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.action).to.equal('approve');
			expect(decision.designTemplate).to.equal('Cobalt');
		});
	});

	it('A design picked before the sidebar is reopened still rides the approval', function() {
		aichatHelper.enableAIWithOutlineSocket(this.win, { outline: outline });
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-template-card[title="Cobalt"]').click();

		aichatHelper.closeAIChat();
		aichatHelper.openAIChat();

		// The rebuilt picker shows the chosen design as the pressed chip, and
		// the plain option is no longer the one selected.
		cy.cGet('.aichat-template-card[title="Cobalt"]')
			.should('have.attr', 'aria-pressed', 'true');
		cy.cGet('.aichat-template-skip')
			.should('have.attr', 'aria-pressed', 'false');

		cy.cGet('.aichat-approve-btn').click();
		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.designTemplate).to.equal('Cobalt');
		});
	});

	it('No design is the default and the approval carries none', function() {
		aichatHelper.enableAIWithOutlineSocket(this.win, { outline: outline });
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-outline-card .aichat-design-section').should('exist');
		cy.cGet('.aichat-approve-btn').click();

		cy.wrap(null).should(() => {
			var decision = this.win.__aichatApprovePayloads[0];
			expect(decision.action).to.equal('approve');
			expect(decision.designTemplate).to.be.undefined;
		});
	});

	it('A recorded pick rides later requests and ends the offer', function() {
		aichatHelper.enableAIWithOutlineSocket(this.win, { outline: outline });
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-template-card[title="Cobalt"]').click();
		cy.cGet('.aichat-approve-btn').click();
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Outline approved');

		// Land the build result so the first request finishes.
		cy.then(() => {
			this.win.__deliverOutlineResult();
		});
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');

		// A later request in the same conversation carries the pick and its
		// outline card no longer offers the catalog.
		sendRequest('Add a slide about kittens');
		cy.cGet('.aichat-outline-card').should('exist');
		cy.cGet('.aichat-design-section').should('not.exist');
		cy.wrap(null).should(() => {
			expect(this.win.__aichatPayloads).to.have.length(2);
			expect(this.win.__aichatPayloads[1].designTemplate).to.equal('Cobalt');
		});
	});

	it('An edit approval offers no designs', function() {
		var transform = JSON.stringify({
			Transforms: {
				SlideCommands: [
					{ 'DeleteSlide': 1 },
				],
			},
		});
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
			approvalTransformJson: transform,
		});
		aichatHelper.openAIChat();
		sendRequest('Delete the second slide');

		cy.cGet('.aichat-approve-btn').should('exist');
		cy.cGet('.aichat-design-section').should('not.exist');
	});
});
