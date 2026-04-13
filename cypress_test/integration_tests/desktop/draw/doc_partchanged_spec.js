/* global describe it cy beforeEach */

import { setupAndLoadDocument } from '../../common/helper';
import { stubParentPostMessage, expectDocPartChanged, resetDocPartChangedCache } from '../../common/postmessage_helper';

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Draw Doc_PartChanged postMessage', function() {

	beforeEach(function() {
		setupAndLoadDocument('draw/navigator.odg');
	});

	it('emits on setpart', function() {
		stubParentPostMessage();

		cy.get('@postMessage').then(stub => {
			stub.resetHistory();
		});

		cy.getFrameWindow().then(win => {
			var map = win.app.map;
			var docLayer = map._docLayer;
			var partCount = Number.isInteger(docLayer._parts) && docLayer._parts > 0 ? docLayer._parts : 1;

			resetDocPartChangedCache(map);

			map.fire('setpart', {
				selectedPart: 0,
				parts: partCount,
				docType: docLayer._docType
			});
		});

		expectDocPartChanged({
			count: 1,
			parts: [1],
			minPartCount: 1
		});
	});
});
