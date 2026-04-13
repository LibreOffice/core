/* global cy expect */

function stubParentPostMessage(aliasName = 'postMessage') {
	cy.getFrameWindow().then(win => {
		cy.stub(win.parent, 'postMessage').as(aliasName);
	});
}

function parsePostedMessage(rawMessage) {
	if (typeof rawMessage === 'string') {
		try {
			return JSON.parse(rawMessage);
		} catch (e) {
			return null;
		}
	}

	return rawMessage;
}

function resetDocPartChangedCache(map) {
	map._lastPart = -1;
	map._lastPartCount = -1;
	map._lastPartDocType = '';
}

function expectDocPartChanged(options = {}, aliasName = 'postMessage') {
	const expectedParts = options.parts || [];
	const minPartCount = options.minPartCount || 1;
	const expectedDocType = options.docType;
	const expectedCount = options.count;

	cy.get('@' + aliasName).should(stub => {
		const docPartMessages = stub.getCalls()
			.map(call => parsePostedMessage(call.args[0]))
			.filter(msg => msg && msg.MessageId === 'Doc_PartChanged' && msg.Values);

		if (expectedCount !== undefined)
			expect(docPartMessages.length, 'unexpected Doc_PartChanged count').to.equal(expectedCount);

		expectedParts.forEach(function(part) {
			const found = docPartMessages.some(function(msg) {
				return msg.Values.Part === part
					&& Number.isInteger(msg.Values.PartCount)
					&& msg.Values.PartCount >= minPartCount
					&& (expectedDocType === undefined || msg.Values.DocType === expectedDocType);
			});

			expect(found, 'Doc_PartChanged for part ' + part + ' was not posted').to.be.true;
		});
	});
}

const _stubParentPostMessage = stubParentPostMessage;
export { _stubParentPostMessage as stubParentPostMessage };
const _expectDocPartChanged = expectDocPartChanged;
export { _expectDocPartChanged as expectDocPartChanged };
const _resetDocPartChangedCache = resetDocPartChangedCache;
export { _resetDocPartChangedCache as resetDocPartChangedCache };
