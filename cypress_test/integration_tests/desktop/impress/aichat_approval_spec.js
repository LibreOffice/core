/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI approval card', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.fodp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('The card counts the change from the commands, not the model summary', function() {
		// The summary says something harmless; the commands also delete a
		// slide. The card must say so anyway.
		var transform = JSON.stringify({
			Transforms: {
				SlideCommands: [
					{ 'InsertMasterSlide': 0 },
					{ 'SetText.0': 'Title' },
					{ 'SetText.1': 'Body' },
					{ 'DeleteSlide': 1 },
					{ 'ApplyTemplate': 'Mint' },
				],
			},
		});
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
			approvalTransformJson: transform,
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Tidy up my deck');
		aichatHelper.clickSend();

		// A fresh deck offers the design picker first; continue without one.
		cy.cGet('.aichat-template-skip').click();

		cy.cGet('#aichat-messages-list').should(
			'contain.text',
			'This will: insert 1 slide, delete 1 slide, set text in 2 places, ' +
				"apply the design template 'Mint'.");
	});

	it('No badge is shown when the approval carries no slide commands', function() {
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Tidy up my deck');
		aichatHelper.clickSend();
		cy.cGet('.aichat-template-skip').click();

		cy.cGet('#aichat-messages-list').should('contain.text', 'Mock change');
		cy.cGet('#aichat-messages-list').should(
			'not.contain.text',
			'This will:');
	});

	it('A compiler-shaped deck transform is described like any other', function() {
		// The write_slides tool compiles a deck spec into slide commands on
		// the server: an ApplyTemplate spliced in front, then a
		// ChangeLayoutByName, SetText and SetSlidePart run per slide, plus a
		// GenerateImage for an image slide. The browser never sees the deck
		// spec, only this transform, so its badge must count it like any
		// other transform.
		var transform = JSON.stringify({
			Transforms: {
				SlideCommands: [
					{ 'ApplyTemplate': 'Cobalt' },
					{ 'ChangeLayoutByName': 'AUTOLAYOUT_TITLE' },
					{ 'SetText.0': 'Renewable Energy' },
					{ 'SetText.1': 'An overview' },
					{ 'SetSlidePart': 'opening' },
					{ 'InsertMasterSlide': 0 },
					{ 'ChangeLayoutByName': 'AUTOLAYOUT_TITLE_CONTENT' },
					{ 'SetText.0': 'Sources' },
					{ 'SetText.1': 'Solar\nWind\nHydro' },
					{ 'SetSlidePart': 'body' },
					{ 'InsertMasterSlide': 0 },
					{ 'ChangeLayoutByName': 'AUTOLAYOUT_TITLE_CONTENT' },
					{ 'SetText.0': 'Growth' },
					{
						'GenerateImage.1': {
							prompt: 'a line chart of renewable capacity',
							alt: 'Line chart of renewable capacity',
						},
					},
					{ 'SetSlidePart': 'body' },
				],
			},
		});
		// The server presents the compiled deck as an ordinary transform, so
		// the browser receives a transform_document_structure approval.
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
			approvalTransformJson: transform,
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('An overview of renewable energy');
		aichatHelper.clickSend();
		cy.cGet('.aichat-template-skip').click();

		cy.cGet('#aichat-messages-list').should(
			'contain.text',
			'This will: insert 2 slides, change 3 layouts, set text in 5 places, ' +
				"generate 1 image, run 3 SetSlidePart commands, " +
				"apply the design template 'Cobalt'.");
	});

	it('An unknown command is reported by its raw name', function() {
		// The badge has no description for this command, but it must still
		// show up - the counted line may never under-describe what runs.
		var transform = JSON.stringify({
			Transforms: {
				SlideCommands: [
					{ 'ReplaceEverything': 1 },
				],
			},
		});
		aichatHelper.enableAIWithCaptureSocket(this.win, {
			approvalToolName: 'transform_document_structure',
			approvalTransformJson: transform,
		});
		aichatHelper.openAIChat();
		aichatHelper.typeIntoAIInput('Tidy up my deck');
		aichatHelper.clickSend();
		cy.cGet('.aichat-template-skip').click();

		cy.cGet('#aichat-messages-list').should(
			'contain.text',
			'This will: run 1 ReplaceEverything command.');
	});
});
