/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI design-template picker', function() {

	// A single-slide presentation: a fresh deck, the case where the
	// assistant offers a design template before building anything.
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.fodp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function sendRequest(text) {
		aichatHelper.typeIntoAIInput(text);
		aichatHelper.clickSend();
	}

	it('The pick rides the deck request', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {});
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		// The picker comes first: cards are on screen and no request is out.
		cy.cGet('.aichat-template-card').should('exist');
		cy.then(() => {
			expect(this.win.__aichatPayloads).to.have.length(0);
		});

		cy.cGet('.aichat-template-card[title="Cobalt"]').click();

		// The transcript records the outcome instead of the dangling prompt.
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Design template: Cobalt');

		// The request carries the picked template name. The template's designs
		// are no longer sent: the backend fetches them from the document itself.
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');
		cy.then(() => {
			expect(this.win.__aichatPayloads).to.have.length(1);
			var payload = this.win.__aichatPayloads[0];
			expect(payload.designTemplate).to.equal('Cobalt');
			expect(payload.designTemplateMasters).to.be.undefined;
			expect(payload.designTemplateLayouts).to.be.undefined;
		});
	});

	it('Skipping sends the request with no template', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {});
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-template-card').should('exist');
		cy.cGet('.aichat-template-skip').click();

		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Continuing without a design template.');
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');
		cy.then(() => {
			expect(this.win.__aichatPayloads).to.have.length(1);
			expect(this.win.__aichatPayloads[0].designTemplate).to.be.undefined;
			expect(this.win.__aichatPayloads[0].designTemplateLayouts).to.be
				.undefined;
		});
	});

	it('A question does not use up the picker offer', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {});
		aichatHelper.openAIChat();
		sendRequest('What is on this slide?');

		cy.cGet('.aichat-template-card').should('exist');
		cy.cGet('.aichat-template-skip').click();
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');

		// The request modified nothing, so while the deck stays fresh the
		// next request offers the picker again.
		sendRequest('Now build a deck about dogs');
		cy.cGet('.aichat-template-card').should('exist');
		cy.then(() => {
			// Only the first request went out; the second awaits the picker.
			expect(this.win.__aichatPayloads).to.have.length(1);
		});
	});

	it('Building deck content ends the picker offer', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
		});
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-template-card').should('exist');
		cy.cGet('.aichat-template-skip').click();

		// The model proposes a change and the user approves it, which is
		// what actually builds deck content.
		cy.cGet('.aichat-approve-btn').click();
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');

		// The approved change built deck content, so the offer is spent:
		// the next request goes straight out with no picker.
		sendRequest('Add one more slide');
		cy.wrap(null).should(() => {
			expect(this.win.__aichatPayloads).to.have.length(2);
		});
		cy.cGet('.aichat-template-card').should('not.exist');
	});

	it('A rejected change does not use up the picker offer', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
		});
		aichatHelper.openAIChat();
		sendRequest('Create a deck about cats');

		cy.cGet('.aichat-template-card').should('exist');
		cy.cGet('.aichat-template-skip').click();

		// The user rejects the proposed change: nothing was built.
		cy.cGet('.aichat-reject-btn').click();
		cy.cGet('#aichat-messages-list').should(
			'contain.text', 'Mock AI response');

		// The deck is still fresh, so the next request offers the picker
		// again and is held until a choice is made.
		sendRequest('Try a different deck');
		cy.cGet('.aichat-template-card').should('exist');
		cy.then(() => {
			expect(this.win.__aichatPayloads).to.have.length(1);
		});
	});
});
