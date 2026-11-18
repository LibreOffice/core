/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { strict as nodeassert } from 'assert';
import { SocketClient } from './helper/SocketClient';
import { BASE_URL, sampleUrl } from './helper/Environment';
import { findTextPortion, findValueOf } from './helper/VectorJson';

describe('Integration: vector primitives of slide text', function () {
	this.timeout(60000);

	let client: SocketClient;
	/// The answer for the one slide of the sample, which has a text box for each test.
	let answer: any;

	before(async function () {
		client = await SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress-vector-text.fodp'),
		);
		const status = await client.status();
		answer = await client.vectorPrimitives(String(status.parts[0].part));
	});

	after(function () {
		if (client) client.close();
	});

	/// The text portion with the given text, which the test cannot go on without.
	function portion(text: string): any {
		const found = findTextPortion(answer, text);
		nodeassert.ok(found, 'no text portion holds "' + text + '"');
		return found;
	}

	/// Every text portion carries the index of the paragraph it belongs to.
	it('names the paragraph of each text portion', function () {
		nodeassert.strictEqual(portion('First paragraph').paragraph, 0);
		nodeassert.strictEqual(portion('Second paragraph').paragraph, 1);
	});

	/// A centered paragraph names its alignment on its text portions.
	it('names the alignment of a paragraph', function () {
		nodeassert.strictEqual(portion('Centered text').align, 'center');
	});

	/// Each advance is the step from the glyph before it, not the distance from the start of
	/// the run, so the numbers stay small however long the run is.
	it('gives each advance from the glyph before it', function () {
		const advances: number[] = portion('Advances').advances;
		nodeassert.strictEqual(advances.length, 8);
		for (const advance of advances)
			nodeassert.ok(advance > 0, 'an advance is not a forward step');

		// Measured from the start of the run, the eighth value would be about eight times the
		// first one.
		nodeassert.ok(advances[7] < advances[0] * 3);
	});

	/// Laid-out text names the height of its line and the ascent within it.
	it('names the height and ascent of the line', function () {
		const found = portion('Line metrics');
		nodeassert.ok(found.lineHeight > 0, 'line height missing');
		nodeassert.ok(found.lineAscent > 0, 'line ascent missing');
		// The ascent is where the baseline lies in the line, so it fits inside the height.
		nodeassert.ok(found.lineAscent <= found.lineHeight);
	});

	/// Laid-out text names the size of the area that it wraps in.
	it('names the size of the text area', function () {
		const found = portion('Box size');
		nodeassert.ok(found.textAreaWidth > 0, 'text area width missing');
		nodeassert.ok(found.textAreaHeight > 0, 'text area height missing');
	});

	/// An indented paragraph names its line indent and line width, which add up to the width of
	/// the text area.
	it('names the indent and width of an indented line', function () {
		const found = portion('Indented line');
		const values =
			'line indent ' +
			found.lineIndent +
			', line width ' +
			found.lineWidth +
			', text area width ' +
			found.textAreaWidth;
		nodeassert.ok(found.lineWidth > 0, values);
		nodeassert.ok(found.lineIndent > 0, values);
		nodeassert.ok(found.lineWidth < found.textAreaWidth, values);
		nodeassert.ok(
			Math.abs(found.lineIndent + found.lineWidth - found.textAreaWidth) <= 1,
			values,
		);
	});

	/// A text portion names its face by an id, and the font command answers with the file of
	/// that face, so a client draws with the exact font.
	it('serves the font file that a text portion names', async function () {
		const fontId = findValueOf(answer, 'fontId');
		nodeassert.ok(typeof fontId === 'string', 'no text portion names a face');
		nodeassert.notStrictEqual(parseInt(fontId, 16), 0);

		const font = await client.vectorFont(fontId);
		nodeassert.strictEqual(font.type, 'vectorrenderingfont');
		nodeassert.strictEqual(font.fontId, fontId);
		nodeassert.strictEqual(font.data, undefined);
		// A real font file is far longer. The bound catches an empty answer only.
		nodeassert.ok(font.bytes instanceof Uint8Array && font.bytes.length > 1000);
	});
});
