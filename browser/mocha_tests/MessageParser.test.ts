/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('MessageParser', function () {
	it('names the command and the parameters of a message', function () {
		const parser = new MessageParser('textinput id=0 text=a sequence=11');
		nodeassert.strictEqual(parser.command, 'textinput');
		nodeassert.strictEqual(parser.length, 4);
		nodeassert.strictEqual(parser.value('id'), '0');
		nodeassert.strictEqual(parser.value('text'), 'a');
		nodeassert.strictEqual(parser.value('part'), undefined);
	});

	it('reads the sequence number of a key message that ends with a line break', function () {
		const parser = new MessageParser(
			'key type=input char=0 key=1286 sequence=14\n',
		);
		nodeassert.strictEqual(parser.length, 5);
		nodeassert.strictEqual(parser.sequence, 14);
	});

	it('counts only a non-negative integer', function () {
		nodeassert.strictEqual(
			new MessageParser('removetextcontext id=0 before=1 after=0').count(
				'before',
			),
			1,
		);
		nodeassert.strictEqual(
			new MessageParser('textinput id=0 text=a sequence=-1').sequence,
			undefined,
		);
		nodeassert.strictEqual(
			new MessageParser('textinput id=0 text=a sequence=x').sequence,
			undefined,
		);
		nodeassert.strictEqual(
			new MessageParser('textinput id=0 text=a sequence=').sequence,
			undefined,
		);
		nodeassert.strictEqual(
			new MessageParser('tilecombine nviewid=0 part=0').sequence,
			undefined,
		);
	});

	it('sets a parameter and keeps the line break of a key message', function () {
		const parser = new MessageParser(
			'key type=input char=0 key=1286 sequence=14\n',
		);
		nodeassert.strictEqual(
			parser.with('sequence', 15),
			'key type=input char=0 key=1286 sequence=15\n',
		);
		nodeassert.strictEqual(
			new MessageParser('textcursor object=5').with('sequence', 3),
			'textcursor object=5 sequence=3',
		);
	});
});
