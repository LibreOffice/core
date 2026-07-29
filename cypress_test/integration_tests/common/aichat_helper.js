/* global cy require */

var helper = require('./helper');

/**
 * Enable AI configuration and stub the socket to intercept aichat: messages.
 *
 * @param {Window} win  - the frame window (from cy.getFrameWindow())
 * @param {object} opts - mock response options
 * @param {string} [opts.content='Mock AI response']  - assistant text
 * @param {boolean} [opts.success=true]                - whether the response succeeds
 * @param {string} [opts.error]                        - error text when success=false
 */
function enableAIAndStubSocket(win, opts) {
	var content = (opts && opts.content) || 'Mock AI response';
	var success = opts && opts.success !== undefined ? opts.success : true;
	var error   = (opts && opts.error) || '';

	win.app.map.isAIConfigured = true;

	var original = win.app.socket.sendMessage.bind(win.app.socket);
	cy.stub(win.app.socket, 'sendMessage').callsFake(function(msg) {
		if (typeof msg === 'string' && msg.startsWith('aichat: ')) {
			var payload = JSON.parse(msg.substring('aichat: '.length));
			var requestId = payload.requestId;
			win.app.layoutingService.onDrain(function() {
				var result = { requestId: requestId, success: success };
				if (success) {
					result.content = content;
				} else {
					result.error = error || 'Mock error';
				}
				win.app.map.fire('aichatresult', result);
			});
		} else if (typeof msg === 'string' && msg.startsWith('aichatcancel: ')) {
			// swallow cancel messages
		} else {
			original(msg);
		}
	});
}

/**
 * Open the AI chat sidebar via the dispatcher.
 */
function openAIChat() {
	cy.getFrameWindow().then(function(win) {
		win.app.dispatcher.dispatch('aichat');
	});
	cy.cGet('#aichat-dock-wrapper.visible').should('exist');
	cy.cGet('#aichat-panel').should('not.be.empty');
}

/**
 * Close the AI chat sidebar by clicking the close button.
 */
function closeAIChat() {
	cy.cGet('#aichat-close-btn button').click();
	cy.getFrameWindow().then(function(win) {
		return helper.waitUntilLayoutingIsIdle(win);
	});
	cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');
}

/**
 * Type text into the AI chat textarea.
 */
function typeIntoAIInput(text) {
	cy.cGet('#aichat-input.ui-textarea').type(text);
}

/**
 * Click the send button.
 */
function clickSend() {
	cy.cGet('#aichat-send-btn button').click();
}

/**
 * Like enableAIAndStubSocket, but also records every aichat: payload
 * on win.__aichatPayloads so the test can assert on what was actually
 * sent (tone, customToneDescription, emojify, ...).
 */
function enableAIWithCaptureSocket(win, opts) {
	var content = (opts && opts.content) || 'Mock AI response';
	var success = opts && opts.success !== undefined ? opts.success : true;
	var error   = (opts && opts.error) || '';

	win.app.map.isAIConfigured = true;
	win.__aichatPayloads = [];

	var original = win.app.socket.sendMessage.bind(win.app.socket);
	cy.stub(win.app.socket, 'sendMessage').callsFake(function(msg) {
		if (typeof msg === 'string' && msg.startsWith('aichat: ')) {
			var payload = JSON.parse(msg.substring('aichat: '.length));
			win.__aichatPayloads.push(payload);
			var requestId = payload.requestId;
			win.app.layoutingService.onDrain(function() {
				var result = { requestId: requestId, success: success };
				if (success) {
					result.content = content;
				} else {
					result.error = error || 'Mock error';
				}
				win.app.map.fire('aichatresult', result);
			});
		} else if (typeof msg === 'string' && msg.startsWith('aichatcancel: ')) {
			// swallow cancel messages
		} else {
			original(msg);
		}
	});
}

/**
 * Open the tone-of-voice picker by clicking the tone chip below the
 * input. Waits for the picker container to gain the open class.
 */
function openTonePicker() {
	cy.cGet('#aichat-tone-chip button.ui-pushbutton').click();
	cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('exist');
}

/**
 * Close the tone picker by sending Escape to the sidebar root.
 */
function closeTonePicker() {
	cy.cGet('#aichat-input.ui-textarea').type('{esc}');
	cy.cGet('#aichat-tone-picker.aichat-tone-picker-open').should('not.exist');
}

/**
 * Give the sidebar back the state a freshly loaded document has it in: no
 * conversation, no saved voice, no tone picker, and the panel closed.
 *
 * The message elements are numbered by their place in the conversation, so a
 * conversation left behind would move every later message's id. A saved tone stays
 * in the preferences rather than in local storage, and it comes back as one more
 * chip, so both the stored copy and the one the sidebar is holding have to go: the
 * preferences are removed and the sidebar's own loader is asked to read them again.
 * The tone chip toggles the picker, so a picker left open turns the next open into a
 * close. Only the state is set here, and the panel is built again from it the next
 * time a test opens the sidebar.
 *
 * @param {Window} win - the frame window (from cy.getFrameWindow())
 */
function resetAIChat(win) {
	cy.log('>> resetAIChat - start');

	var sidebar = win.JSDialog.AIChatSidebar;
	if (sidebar) {
		[sidebar.PREFS_CUSTOM_TONES, sidebar.PREFS_RECENT_ICONS,
			sidebar.PREFS_SELECTED_TONE, sidebar.PREFS_EMOJIFY].forEach(function(key) {
			win.prefs.remove(key);
		});
		sidebar.loadPrefs();

		sidebar.tonePickerOpen = false;
		sidebar.clearConversation();
	}

	// Opening and closing go through the same dispatch, so it is sent only when
	// something is on screen to close.
	cy.cGet('body').then(function($body) {
		if ($body.find('#aichat-dock-wrapper.visible').length)
			win.app.dispatcher.dispatch('aichat');
	});

	cy.cGet('#aichat-dock-wrapper.visible').should('not.exist');

	cy.log('<< resetAIChat - end');
}

module.exports.enableAIAndStubSocket = enableAIAndStubSocket;
module.exports.resetAIChat = resetAIChat;
module.exports.enableAIWithCaptureSocket = enableAIWithCaptureSocket;
module.exports.openAIChat = openAIChat;
module.exports.closeAIChat = closeAIChat;
module.exports.typeIntoAIInput = typeIntoAIInput;
module.exports.clickSend = clickSend;
module.exports.openTonePicker = openTonePicker;
module.exports.closeTonePicker = closeTonePicker;
