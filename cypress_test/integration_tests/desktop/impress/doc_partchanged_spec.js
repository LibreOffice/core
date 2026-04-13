/* global describe it cy beforeEach expect */

import { setupAndLoadDocument } from '../../common/helper';
import { stubParentPostMessage, expectDocPartChanged, resetDocPartChangedCache } from '../../common/postmessage_helper';

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress Doc_PartChanged postMessage', function() {

	beforeEach(function() {
		setupAndLoadDocument('impress/slide_navigation.odp');
	});

	it('emits for setpart, updateparts and pagenumberchanged event paths', function() {
		stubParentPostMessage();

		cy.get('@postMessage').then(stub => {
			stub.resetHistory();
		});

		cy.getFrameWindow().then(win => {
			var map = win.app.map;
			var docLayer = map._docLayer;

			expect(docLayer._parts).to.be.gte(2);
			expect(docLayer._docType).to.equal('presentation');

			resetDocPartChangedCache(map);

			map.fire('setpart', {
				selectedPart: 1,
				parts: docLayer._parts,
				docType: docLayer._docType
			});

			map.fire('setpart', {
				selectedPart: 1,
				parts: docLayer._parts,
				docType: docLayer._docType
			});

			map.fire('setpart', {
				selectedPart: 1
			});

			map.fire('updateparts', {
				selectedPart: 0,
				parts: docLayer._parts,
				docType: docLayer._docType
			});

			map.fire('updateparts', {});

			map.fire('pagenumberchanged', {
				currentPage: 1,
				pages: docLayer._parts,
				docType: docLayer._docType
			});
		});

		expectDocPartChanged({
			count: 3,
			parts: [1, 2],
			minPartCount: 2,
			docType: 'presentation'
		});
	});
});
