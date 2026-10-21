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

describe('InputSequence', function () {
	const assert = require('assert');

	let savedFile: any;
	beforeEach(function () {
		savedFile = (app as any).file;
		(app as any).file = { inputSequence: 10 };
		cool.InputSequence.forget();
	});
	afterEach(function () {
		(app as any).file = savedFile;
		cool.InputSequence.forget();
	});

	it('numbers each message one above the last', function () {
		assert.strictEqual(
			cool.InputSequence.stamp('key type=input'),
			'key type=input sequence=11',
		);
		assert.strictEqual(cool.InputSequence.stamp('mouse'), 'mouse sequence=12');
		assert.strictEqual(app.file.inputSequence, 12);
	});

	it('gives a ticket for the next message the number of that message only', function () {
		const ticket = cool.InputSequence.ticketForTheNextMessage();
		assert.strictEqual(ticket.sentAs(), undefined);

		cool.InputSequence.stamp('textinput');
		cool.InputSequence.stamp('textinput');
		assert.strictEqual(ticket.sentAs(), 11);
	});

	it('keeps a ticket waiting past a message that carries no change', function () {
		const ticket = cool.InputSequence.ticketForTheNextMessage();
		assert.strictEqual(cool.InputSequence.next([], false), 11);
		assert.strictEqual(ticket.sentAs(), undefined);

		cool.InputSequence.stamp('textinput');
		assert.strictEqual(ticket.sentAs(), 12);
	});

	it('gives the tickets named the number of the message they go out with', function () {
		const first = new cool.InputTicket();
		const second = new cool.InputTicket();
		cool.InputSequence.stamp('setobjecttransform', [first, second]);
		assert.strictEqual(first.sentAs(), 11);
		assert.strictEqual(second.sentAs(), 11);
	});

	it('counts an input as applied by its own number and the ones after it', function () {
		const ticket = cool.InputSequence.ticketForTheNextMessage();
		assert.ok(!ticket.appliedBy(100));

		cool.InputSequence.stamp('textinput');
		assert.ok(!ticket.appliedBy(10));
		assert.ok(ticket.appliedBy(11));
		assert.ok(ticket.appliedBy(12));
	});

	it('takes the number back from the message stamped last, with its tickets', function () {
		const ticket = cool.InputSequence.ticketForTheNextMessage();
		const key = cool.InputSequence.stamp('key type=input') + '\n';
		const unstamped = cool.InputSequence.unstamp(key);
		assert.deepStrictEqual(unstamped, {
			message: 'key type=input\n',
			tickets: [ticket],
		});
		assert.strictEqual(ticket.sentAs(), undefined);
		assert.strictEqual(app.file.inputSequence, 10);

		// The number goes to the next message stamped, and the tickets follow their own.
		assert.strictEqual(cool.InputSequence.stamp('mouse'), 'mouse sequence=11');
		assert.strictEqual(
			cool.InputSequence.stamp(unstamped!.message, unstamped!.tickets),
			'key type=input sequence=12\n',
		);
		assert.strictEqual(ticket.sentAs(), 12);
	});

	it('leaves a message alone that is not the one stamped last', function () {
		cool.InputSequence.stamp('key type=input');
		assert.strictEqual(cool.InputSequence.unstamp('mouse sequence=5'), null);
		assert.strictEqual(cool.InputSequence.unstamp('uno .uno:Bold'), null);
		assert.strictEqual(app.file.inputSequence, 11);
	});

	it('reads the newest input applied from a header line', function () {
		assert.strictEqual(
			cool.InputSequence.readFrom(
				'zstdvectorprimitivesdelta: inputSequence=42',
			),
			42,
		);
		assert.strictEqual(
			cool.InputSequence.readFrom('commandvalues: inputSequence=7 '),
			7,
		);
		assert.strictEqual(
			cool.InputSequence.readFrom('commandvalues: '),
			undefined,
		);
		assert.strictEqual(
			cool.InputSequence.readFrom('zstdvectorprimitives: inputSequence=0'),
			undefined,
		);
	});
});
