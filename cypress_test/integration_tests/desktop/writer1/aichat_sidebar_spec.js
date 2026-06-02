/* global describe it cy beforeEach require expect */

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
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-1').should('exist');
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
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-msg-1').should('exist');
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

		it('Retry does not duplicate or unstyle the re-sent message', function() {
			aichatHelper.enableAIAndStubSocket(this.win, { success: false, error: 'boom' });
			aichatHelper.openAIChat();
			// Two failing sends so the prior [user, error] pair survives the
			// splice and the incremental render path runs (length stays > 0).
			aichatHelper.typeIntoAIInput('First');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			aichatHelper.typeIntoAIInput('Second');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			// messages: user0, error1, user2, error3
			cy.cGet('#aichat-retry-3 button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			// After retry the failed turn (user2, error3) is removed and 'Second'
			// is re-sent, landing at index 2 as a styled user bubble.
			cy.cGet('[id="aichat-msg-2"]').should('have.length', 1);
			cy.cGet('#aichat-msg-2').should('have.class', 'aichat-msg-user');
		});

		it('Retry after an approval failure clears the approval bubble and re-sends', function() {
			var win = this.win;
			win.app.map.isAIConfigured = true;
			var original = win.app.socket.sendMessage.bind(win.app.socket);
			cy.stub(win.app.socket, 'sendMessage').callsFake(function(msg) {
				if (typeof msg === 'string' && msg.startsWith('aichat: ')) {
					// First leg: the model asks for approval to read the document.
					var p = JSON.parse(msg.substring('aichat: '.length));
					win.app.layoutingService.onDrain(function() {
						win.app.map.fire('aichatapproval', {
							requestId: p.requestId,
							toolName: 'extract_document_structure',
							summary: 'Read the full text of your document',
						});
					});
				} else if (typeof msg === 'string' && msg.startsWith('aichatapprove: ')) {
					// After approval the run fails.
					var a = JSON.parse(msg.substring('aichatapprove: '.length));
					win.app.layoutingService.onDrain(function() {
						win.app.map.fire('aichatresult', {
							requestId: a.requestId,
							success: false,
							error: 'boom',
						});
					});
				} else if (typeof msg === 'string' && msg.startsWith('aichatcancel: ')) {
					// swallow cancel messages
				} else {
					original(msg);
				}
			});
			aichatHelper.openAIChat();
			aichatHelper.typeIntoAIInput('Summarize');
			aichatHelper.clickSend();
			// messages: user0, approval1
			cy.cGet('#aichat-msg-1').should('have.class', 'aichat-msg-approval');
			cy.cGet('.aichat-approve-btn').click();
			// messages: user0, approval1, error2
			cy.cGet('#aichat-msg-2').should('have.class', 'aichat-msg-error');
			cy.cGet('#aichat-retry-2 button').click();
			// Retry removes the whole failed turn and re-sends, which triggers a
			// fresh approval. The list should hold exactly the re-rendered user
			// prompt and approval - no orphaned approval, no leftover error, no
			// duplicate bubbles.
			cy.cGet('#aichat-msg-1').should('have.class', 'aichat-msg-approval');
			cy.cGet('#aichat-msg-0').should('have.class', 'aichat-msg-user');
			cy.cGet('#aichat-messages-list > [id^="aichat-msg-"]').should('have.length', 2);
			cy.cGet('.aichat-msg-error').should('not.exist');
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

	describe('Tone picker - presets', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Picker opens on tone chip click', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-natural').should('exist');
			cy.cGet('#aichat-tone-preset-formal').should('exist');
			cy.cGet('#aichat-tone-preset-short').should('exist');
			cy.cGet('#aichat-tone-preset-friendly').should('exist');
			cy.cGet('#aichat-tone-preset-professional').should('exist');
			cy.cGet('#aichat-tone-preset-casual').should('exist');
		});

		it('Selecting a preset closes the picker and updates the chip', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-formal .ui-chip-main').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('not.exist');
			cy.cGet('#aichat-tone-chip button.ui-pushbutton').should('contain.text', 'Formal');
		});

		it('Selection writes to localStorage', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-friendly .ui-chip-main').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.then(() => {
				expect(this.win.localStorage.getItem('aichat.selectedTone')).to.equal('friendly');
			});
		});

		it('Selected preset is marked aria-pressed in the picker', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-professional .ui-chip-main').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-professional .ui-chip-main')
				.should('have.attr', 'aria-pressed', 'true');
			cy.cGet('#aichat-tone-preset-natural .ui-chip-main')
				.should('have.attr', 'aria-pressed', 'false');
		});
	});

	describe('Tone picker - emojify', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Toggling Emojify flips aria-pressed', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-emojify button.ui-pushbutton')
				.should('have.attr', 'aria-pressed', 'false');
			cy.cGet('#aichat-tone-emojify button.ui-pushbutton').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-emojify button.ui-pushbutton')
				.should('have.attr', 'aria-pressed', 'true');
			cy.then(() => {
				expect(this.win.localStorage.getItem('aichat.emojify')).to.equal('true');
			});
		});
	});

	describe('Tone picker - docload-failure regression', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		// Direct regression guard for the resetVoiceState split:
		// onDocLoaded(status: false) must NOT erase the persisted tone.
		it('docloaded:false does not wipe persisted tone', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-formal .ui-chip-main').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.then(() => {
				expect(this.win.localStorage.getItem('aichat.selectedTone')).to.equal('formal');
				this.win.app.map.fire('docloaded', { status: false });
			});
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.then(() => {
				expect(this.win.localStorage.getItem('aichat.selectedTone')).to.equal('formal');
			});
		});
	});

	describe('Tone picker - custom tones', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.openTonePicker();
		});

		it('Add my own voice opens the form', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form').should('exist');
			cy.cGet('#aichat-tone-form-name input').should('exist');
			cy.cGet('#aichat-tone-form-save button').should('have.attr', 'disabled');
		});

		it('Save enables once name and description are filled', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-save button').should('not.have.attr', 'disabled');
		});

		// After saveToneForm the picker stays open, so we do NOT re-click
		// the tone chip (which would toggle it closed).
		it('Saving adds a chip selected by aria-pressed', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-save button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			// Constrain to .ui-chip - the chip's main button also carries
			// an id that starts with aichat-tone-custom-, so without the
			// class filter the selector picks up both wrapper and button.
			cy.cGet('.ui-chip[id^="aichat-tone-custom-"]').should('have.length', 1);
			cy.cGet('.ui-chip[id^="aichat-tone-custom-"] .ui-chip-main')
				.should('have.attr', 'aria-pressed', 'true');
		});

		it('Edit pencil reopens the form with prepopulated values', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			cy.cGet('#aichat-tone-form-save button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('[id^="aichat-tone-edit-"]').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').should('have.value', 'Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').should('have.value', 'Be terse');
			cy.cGet('#aichat-tone-form-delete button').should('exist');
		});

		it('Delete removes the custom tone', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			cy.cGet('#aichat-tone-form-save button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('[id^="aichat-tone-edit-"]').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-delete button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-delete-confirm').should('exist');
			cy.cGet('#aichat-tone-delete-confirm-ok button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('.ui-chip[id^="aichat-tone-custom-"]').should('not.exist');
		});

		it('Cancelling the delete confirm returns to the edit form', function() {
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			cy.cGet('#aichat-tone-form-save button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('[id^="aichat-tone-edit-"]').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-delete button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-delete-confirm-cancel button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-delete-confirm').should('not.exist');
			cy.cGet('#aichat-tone-form-delete button').should('exist');
			cy.cGet('#aichat-tone-form-name input').should('have.value', 'Curt');
		});
	});

	describe('Tone picker - emoji picker inside form', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
		});

		it('Plus button opens the emoji picker', function() {
			cy.cGet('#aichat-tone-form-emoji-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker').should('exist');
			cy.cGet('#aichat-tone-form-emoji-picker .ui-emoji-picker-search').should('exist');
		});

		it('Search filters cells by keyword', function() {
			cy.cGet('#aichat-tone-form-emoji-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker .ui-emoji-picker-search').type('smile');
			helper.waitUntilLayoutingIsIdle(this.win);
			// Smileys category - 😊 is keyworded "smile happy blush".
			cy.cGet('#aichat-tone-form-emoji-picker .ui-emoji-picker-cell')
				.contains('\u{1F60A}')
				.should('exist');
		});

		it('Picking an emoji closes the picker and highlights the first slot', function() {
			cy.cGet('#aichat-tone-form-emoji-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker .ui-emoji-picker-search').type('smile');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker .ui-emoji-picker-cell')
				.contains('\u{1F60A}')
				.click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker').should('not.exist');
			cy.cGet('#aichat-tone-form-icon-0 button.ui-pushbutton')
				.should('have.attr', 'aria-pressed', 'true')
				.and('contain.text', '\u{1F60A}');
		});
	});

	describe('Tone picker - request payload', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIWithCaptureSocket(this.win, { content: 'Reply' });
			aichatHelper.openAIChat();
		});

		it('Selected preset is sent as tone in the payload', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-preset-formal .ui-chip-main').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.then(() => {
				var last = this.win.__aichatPayloads[this.win.__aichatPayloads.length - 1];
				expect(last.tone).to.equal('formal');
				expect(last.customToneDescription).to.equal(undefined);
				expect(last.emojify).to.equal(false);
			});
		});

		it('Custom tone is sent as tone=custom with the description', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-name input').type('Curt');
			cy.cGet('#aichat-tone-form-desc.ui-textarea').type('Be terse');
			cy.cGet('#aichat-tone-form-save button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			aichatHelper.typeIntoAIInput('Hello');
			aichatHelper.clickSend();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.then(() => {
				var last = this.win.__aichatPayloads[this.win.__aichatPayloads.length - 1];
				expect(last.tone).to.equal('custom');
				expect(last.customToneDescription).to.equal('Be terse');
			});
		});
	});

	describe('Tone picker - escape layering', function() {
		beforeEach(function() {
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
		});

		it('Escape closes picker without closing the sidebar', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('not.exist');
			cy.cGet('#aichat-dock-wrapper.visible').should('exist');
		});

		it('Escape unwinds emoji picker -> form -> picker -> sidebar', function() {
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker').should('exist');

			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form-emoji-picker').should('not.exist');
			cy.cGet('#aichat-tone-form').should('exist');

			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form').should('not.exist');
			cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('exist');

			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('not.exist');
			cy.cGet('#aichat-dock-wrapper.visible').should('exist');

			cy.cGet('#aichat-input.ui-textarea').type('{esc}');
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');
		});
	});
});
