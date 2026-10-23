/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('Doc_PartChanged message', function () {
	let posted: { msgId: string; args: any }[];
	let partChanged: DocPartChanged;

	beforeEach(function () {
		posted = [];
		partChanged = new DocPartChanged((message) => posted.push(message));
	});

	// The values of every message posted so far, after checking that each one is a
	// Doc_PartChanged.
	function postedValues(): any[] {
		return posted.map((message) => {
			nodeassert.strictEqual(message.msgId, 'Doc_PartChanged');
			return message.args;
		});
	}

	it('is posted once when updateparts reports a part', function () {
		partChanged.onUpdateParts({
			selectedPart: 0,
			parts: 3,
			docType: 'spreadsheet',
		});

		nodeassert.deepStrictEqual(postedValues(), [
			{ Part: 1, PartCount: 3, DocType: 'spreadsheet' },
		]);
	});

	it('is posted for setpart, updateparts and pagenumberchanged, and not for repeats or incomplete events', function () {
		const parts = 3;
		const docType = 'presentation';

		partChanged.onSetPart({ selectedPart: 1, parts: parts, docType: docType });
		partChanged.onSetPart({ selectedPart: 1, parts: parts, docType: docType });
		partChanged.onSetPart({ selectedPart: 1 });
		partChanged.onUpdateParts({
			selectedPart: 0,
			parts: parts,
			docType: docType,
		});
		partChanged.onUpdateParts({});
		partChanged.onPageNumberChanged({
			currentPage: 1,
			pages: parts,
			docType: docType,
		});

		nodeassert.deepStrictEqual(postedValues(), [
			{ Part: 2, PartCount: 3, DocType: 'presentation' },
			{ Part: 1, PartCount: 3, DocType: 'presentation' },
			{ Part: 2, PartCount: 3, DocType: 'presentation' },
		]);
	});

	it('is posted on setpart', function () {
		partChanged.onSetPart({ selectedPart: 0, parts: 2, docType: 'drawing' });

		nodeassert.deepStrictEqual(postedValues(), [
			{ Part: 1, PartCount: 2, DocType: 'drawing' },
		]);
	});

	it('is posted on pagenumberchanged', function () {
		partChanged.onPageNumberChanged({
			currentPage: 0,
			pages: 4,
			docType: 'text',
		});

		nodeassert.deepStrictEqual(postedValues(), [
			{ Part: 1, PartCount: 4, DocType: 'text' },
		]);
	});

	describe('on a map', function () {
		let map: Evented;

		beforeEach(function () {
			map = new Evented();
			partChanged.listenTo(map);
		});

		it('is posted for each part event the map fires', function () {
			map.fire('setpart', { selectedPart: 0, parts: 3, docType: 'drawing' });
			map.fire('updateparts', {
				selectedPart: 1,
				parts: 3,
				docType: 'drawing',
			});
			map.fire('pagenumberchanged', {
				currentPage: 2,
				pages: 3,
				docType: 'drawing',
			});

			nodeassert.deepStrictEqual(postedValues(), [
				{ Part: 1, PartCount: 3, DocType: 'drawing' },
				{ Part: 2, PartCount: 3, DocType: 'drawing' },
				{ Part: 3, PartCount: 3, DocType: 'drawing' },
			]);
		});

		it('is posted before the setpart listeners added later run', function () {
			let postedBeforeLaterListener: number = null;
			map.on('setpart', () => {
				postedBeforeLaterListener = posted.length;
			});

			map.fire('setpart', { selectedPart: 0, parts: 2, docType: 'drawing' });

			nodeassert.strictEqual(postedBeforeLaterListener, 1);
		});
	});
});
