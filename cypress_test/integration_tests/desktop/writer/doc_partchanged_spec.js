/* global describe it cy beforeEach */

import { setupAndLoadDocument } from '../../common/helper';
import { stubParentPostMessage, expectDocPartChanged, resetDocPartChangedCache } from '../../common/postmessage_helper';

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Writer Doc_PartChanged postMessage', function() {

	beforeEach(function() {
		setupAndLoadDocument('writer/focus.odt');
	});

	it('emits on pagenumberchanged', function() {
		stubParentPostMessage();

		cy.get('@postMessage').then(stub => {
			stub.resetHistory();
		});

		cy.getFrameWindow().then(win => {
			var map = win.app.map;
			var docLayer = map._docLayer;
			var pageCount = Number.isInteger(docLayer._pages) && docLayer._pages > 0 ? docLayer._pages : 1;

			resetDocPartChangedCache(map);

			map.fire('pagenumberchanged', {
				currentPage: 0,
				pages: pageCount,
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
