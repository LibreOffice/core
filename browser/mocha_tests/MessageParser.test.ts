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

	it('reads the uno command of a uno or commandvalues message', function () {
		// Read a bare command; it should have no parameters and no JSON.
		const bold = new MessageParser('uno .uno:Bold').uno;
		nodeassert.strictEqual(bold.name, '.uno:Bold');
		nodeassert.deepStrictEqual(bold.params(), new Map());
		nodeassert.strictEqual(bold.json(), undefined);

		// Read a command with a JSON argument; the spaces in it should
		// not cut it short.
		const freeze = new MessageParser(
			'uno .uno:FreezePanesColumn {"Index": {"type": "int32", "value": 3}}',
		).uno;
		nodeassert.strictEqual(freeze.name, '.uno:FreezePanesColumn');
		nodeassert.deepStrictEqual(freeze.json(), {
			Index: { type: 'int32', value: 3 },
		});

		// Read a typed query parameter; it should be keyed by its name.
		const mode = new MessageParser(
			'uno .uno:ToolbarMode?Mode:string=notebookbar_online.ui',
		).uno;
		nodeassert.strictEqual(mode.name, '.uno:ToolbarMode');
		nodeassert.strictEqual(mode.params().get('Mode'), 'notebookbar_online.ui');

		// Read a commandvalues query; the parameters should be decoded.
		const geometry = new MessageParser(
			'commandvalues command=.uno:SheetGeometryData?columns=1&rows=1',
		).uno;
		nodeassert.strictEqual(geometry.name, '.uno:SheetGeometryData');
		nodeassert.deepStrictEqual(
			geometry.params(),
			new Map([
				['columns', '1'],
				['rows', '1'],
			]),
		);
		const fields = new MessageParser(
			'commandvalues command=.uno:Fields?namePrefix=ZOTERO_ITEM%20CSL_CITATION',
		).uno;
		nodeassert.strictEqual(
			fields.params().get('namePrefix'),
			'ZOTERO_ITEM CSL_CITATION',
		);

		// Read another message; it should have no uno command.
		nodeassert.strictEqual(
			new MessageParser('tilecombine nviewid=0 part=0').uno,
			undefined,
		);
	});

	it('tells a hover from a drag', function () {
		const hover = 'mouse type=move x=100 y=200 count=1 buttons=0 modifier=0';
		nodeassert.strictEqual(new MessageParser(hover).isHover, true);
		const drag = 'mouse type=move x=100 y=200 count=1 buttons=1 modifier=0';
		nodeassert.strictEqual(new MessageParser(drag).isHover, false);
		const click =
			'mouse type=buttondown x=100 y=200 count=1 buttons=1 modifier=0';
		nodeassert.strictEqual(new MessageParser(click).isHover, false);
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
