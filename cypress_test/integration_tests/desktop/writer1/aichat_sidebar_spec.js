/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI Chat Sidebar', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	describe('Open/Close', function() {
		it('Sidebar opens when AI is configured', function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Toggle closes sidebar', function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			// Second dispatch should close
			cy.then(() => {
				this.win.app.dispatcher.dispatch('aichat');
			});
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');
		});

		it('Close button closes sidebar', function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.closeAIChat();
		});

		it('Shows snackbar when AI is not configured', function() {
			this.win.app.map.isAIConfigured = false;
			this.win.app.dispatcher.dispatch('aichat');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#snackbar-container').should('be.visible');
			cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');
		});
	});

	describe('Empty State', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Shows empty state text', function() {
			cy.cGet('#aichat-empty-state').should('contain.text', 'Ask AI anything');
		});

		it('Shows initial prompt chips', function() {
			cy.cGet('#aichat-chip-0').should('exist');
			cy.cGet('#aichat-chip-1').should('exist');
			cy.cGet('#aichat-chip-2').should('exist');
			cy.cGet('#aichat-chip-3').should('not.exist');
		});

		it('Header shows title', function() {
			cy.cGet('#aichat-title').should('contain.text', 'AI Assistant');
		});

		it('Clear button is disabled when empty', function() {
			cy.cGet('#aichat-clear-btn button').should('have.attr', 'disabled');
		});
	});

	describe('Input Behavior', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Send button is disabled when empty', function() {
			cy.cGet('#aichat-send-btn button').should('have.attr', 'disabled');
		});

		it('Send button enables on text input', function() {
			aichatHelper.typeIntoAIInput('Hello');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-send-btn button').should('not.have.attr', 'disabled');
		});

		it('Send button re-disables on clear', function() {
			aichatHelper.typeIntoAIInput('Hello');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-send-btn button').should('not.have.attr', 'disabled');
			// Drive the clear like a real user: keyup events from the
			// selectall+backspace are what Widget.MultilineEdit listens
			// for to notify the sidebar of an input change.
			cy.cGet('#aichat-input.ui-textarea').type('{selectall}{backspace}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-send-btn button').should('have.attr', 'disabled');
		});

		it('Textarea has placeholder text', function() {
			cy.cGet('#aichat-input.ui-textarea').should('have.attr', 'placeholder', 'Ask AI...');
		});
	});

	describe('Send & Response', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, { content: 'Hello from AI' });
			aichatHelper.openAIChat();
		});

		it('User message appears after send', function() {
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-0').should('exist');
			cy.cGet('#aichat-msg-0').should('have.class', 'aichat-msg-user');
			cy.cGet('#aichat-msg-0').should('contain.text', 'Hello');
		});

		it('Assistant response appears', function() {
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-1').should('exist');
			cy.cGet('#aichat-msg-1').should('have.class', 'aichat-msg-assistant');
			cy.cGet('#aichat-msg-1').should('contain.text', 'Hello from AI');
		});

		it('Input is cleared after send', function() {
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-0').should('exist');
			cy.cGet('#aichat-input.ui-textarea').should('have.value', '');
		});

		it('Clear button enables after messages exist', function() {
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			cy.cGet('#aichat-msg-1').should('exist');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-clear-btn button').should('not.have.attr', 'disabled');
		});
	});

	describe('Prompt Chips', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, { content: 'Chip response' });
			aichatHelper.openAIChat();
		});

		it('Clicking a chip sends the prompt', function() {
			cy.cGet('#aichat-chip-0 button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-0').should('exist');
			cy.cGet('#aichat-msg-0').should('have.class', 'aichat-msg-user');
			// The chip at index 0 sends "Make it more concise"
			cy.cGet('#aichat-msg-0').should('contain.text', 'Make it more concise');
		});

		it('Chips disappear after send', function() {
			cy.cGet('#aichat-chip-0 button').click();
			cy.cGet('#aichat-msg-0').should('exist');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-chips').should('not.exist');
			cy.cGet('#aichat-empty-state').should('not.exist');
		});
	});

	describe('Clear Conversation', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, { content: 'Response' });
			aichatHelper.openAIChat();
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			cy.cGet('#aichat-msg-1').should('exist');
			helper.waitUntilLayoutingIsIdle(this.win);
		});

		it('Clear resets to empty state', function() {
			cy.cGet('#aichat-clear-btn button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-empty-state').should('exist');
			cy.cGet('#aichat-chips').should('exist');
			cy.cGet('#aichat-msg-0').should('not.exist');
		});

		it('Clear disables clear button', function() {
			cy.cGet('#aichat-clear-btn button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-clear-btn button').should('have.attr', 'disabled');
		});
	});

	describe('Action Buttons', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, { content: 'Copyable text' });
			aichatHelper.openAIChat();
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			cy.cGet('#aichat-msg-1').should('exist');
			helper.waitUntilLayoutingIsIdle(this.win);
		});

		it('Copy button exists on assistant messages', function() {
			cy.cGet('#aichat-copy-text-1').should('exist');
		});

		it('Insert button exists on assistant messages', function() {
			cy.cGet('#aichat-insert-text-1').should('exist');
		});

		it('No action buttons on user messages', function() {
			cy.cGet('#aichat-copy-text-0').should('not.exist');
			cy.cGet('#aichat-insert-text-0').should('not.exist');
		});
	});

	describe('Error Handling', function() {
		it('Shows error message with error class', function() {
			aichatHelper.enableAIAndStubSocket(this.win, { success: false, error: 'Something went wrong' });
			aichatHelper.openAIChat();
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-1').should('exist');
			cy.cGet('#aichat-msg-1').should('have.class', 'aichat-msg-error');
			cy.cGet('#aichat-msg-1').should('contain.text', 'Error:');
		});

		it('Retry button appears on error messages', function() {
			aichatHelper.enableAIAndStubSocket(this.win, { success: false, error: 'Fail' });
			aichatHelper.openAIChat();
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-1').should('exist');
			cy.cGet('#aichat-retry-1').should('exist');
		});
	});

	describe('See More/Less', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('See more expands chips', function() {
			cy.cGet('#aichat-chip-3').should('not.exist');
			cy.cGet('#aichat-see-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-chip-3').should('exist');
			cy.cGet('#aichat-chip-4').should('exist');
			cy.cGet('#aichat-chip-5').should('exist');
			cy.cGet('#aichat-see-more').should('contain.text', 'See less');
		});

		it('See less collapses chips', function() {
			cy.cGet('#aichat-see-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-chip-3').should('exist');
			cy.cGet('#aichat-see-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-chip-3').should('not.exist');
			cy.cGet('#aichat-see-more').should('contain.text', 'See more');
		});
	});

	describe('Keyboard Navigation', function() {
		beforeEach(function() {
			aichatHelper.enableAIAndStubSocket(this.win, { content: 'Keyboard response' });
			aichatHelper.openAIChat();
		});

		it('Escape closes sidebar', function() {
			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');
		});

		it('Enter sends message', function() {
			aichatHelper.typeIntoAIInput('Hello');
			cy.cGet('#aichat-input.ui-textarea').type('{enter}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-0').should('exist');
			cy.cGet('#aichat-msg-0').should('have.class', 'aichat-msg-user');
		});

		it('Shift-Enter does not send message', function() {
			aichatHelper.typeIntoAIInput('Hello');
			cy.cGet('#aichat-input.ui-textarea').type('{shift}{enter}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-0').should('not.exist');
			cy.cGet('#aichat-input.ui-textarea').should('not.have.value', '');
		});
	});
});
