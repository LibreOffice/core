/* global cy */

function stubParentPostMessage(aliasName = 'postMessage') {
	cy.getFrameWindow().then(win => {
		cy.stub(win.parent, 'postMessage').as(aliasName);
	});
}

const _stubParentPostMessage = stubParentPostMessage;
export { _stubParentPostMessage as stubParentPostMessage };
