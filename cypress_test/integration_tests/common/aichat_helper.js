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

module.exports.enableAIAndStubSocket = enableAIAndStubSocket;
module.exports.openAIChat = openAIChat;
module.exports.closeAIChat = closeAIChat;
module.exports.typeIntoAIInput = typeIntoAIInput;
module.exports.clickSend = clickSend;
