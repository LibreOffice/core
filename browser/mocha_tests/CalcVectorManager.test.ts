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

describe('CalcVectorManager requests', function () {
	let recorder: SocketRecorder;
	let savedInstance: any;
	let manager: CalcVectorManager;
	let changes: number;

	beforeEach(function () {
		recorder = new SocketRecorder();
		// The first content that arrives reports the first visuals.
		savedInstance = (RenderManager as any)._instance;
		(RenderManager as any)._instance = { markVisualsReady() {} };
		manager = new CalcVectorManager();
		changes = 0;
		manager.onChanged(() => changes++);
	});
	afterEach(function () {
		(manager as any)._disarmExpiry();
		recorder.restore();
		(RenderManager as any)._instance = savedInstance;
	});

	// The request ids named by the messages sent, in their order.
	function requestIds(): number[] {
		return recorder.sent.map((message) =>
			Number(message.split('requestid=')[1].split('&')[0]),
		);
	}

	// The parts named by the messages sent, in their order.
	function parts(): number[] {
		return recorder.sent.map((message) =>
			Number(message.split('partid=')[1].split('&')[0]),
		);
	}

	// The block that each message asks for, or '' for a request without one.
	function blocksAsked(): string[] {
		return recorder.sent.map(
			(message) => message.split('blocks=')[1]?.split('&')[0] ?? '',
		);
	}

	function wholeBlock(
		blockRow: number,
		blockColumn: number,
	): cool.CalcBlockUpdate {
		return {
			kind: 'cells',
			blockRow,
			blockColumn,
			startRow: blockRow * 64,
			startColumn: blockColumn * 16,
			endRow: (blockRow + 1) * 64 - 1,
			endColumn: (blockColumn + 1) * 16 - 1,
			version: 1,
			from: 0,
			bounds: { x: [0, 0], y: [0, 0], right: [16, 0], bottom: [64, 0] },
			rows: [],
		};
	}

	// A whole answer for the sheet that carries the given blocks.
	function answer(
		part: number,
		requestId: number,
		blocks: cool.CalcBlockUpdate[],
	): cool.CalcVectorResponse {
		return {
			type: 'vectorprimitives',
			docType: 'spreadsheet',
			mode: 0,
			partId: String(part),
			epoch: 7,
			requestId,
			version: 1,
			blockRows: 64,
			blockColumns: 16,
			objects: blocks,
		};
	}

	// Move the time every request was sent back past the timeout.
	function ageRequests(): void {
		for (const request of (manager as any)._requests.values())
			request.sent -= 2 * (CalcVectorManager as any).requestTimeout;
	}

	it('asks for one block in each request, two at a time', function () {
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
			[1, 0],
		]);

		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:1']);
	});

	it('asks for the next block once an answer arrives', function () {
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
			[1, 0],
		]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(0, 0)]),
		);

		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:1', '1:0']);
		nodeassert.strictEqual(manager.getSheet(0, 0)?.blocks.has('0:0'), true);
	});

	it('keeps an answer that comes after its request gave its place up', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		const late = requestIds()[0];
		ageRequests();
		(manager as any)._retireExpired();

		manager.handleVectorPrimitives(answer(0, late, [wholeBlock(0, 0)]));

		nodeassert.strictEqual(manager.getSheet(0, 0)?.blocks.has('0:0'), true);
	});

	it('drops an answer for a sheet it no longer holds', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		const requestId = requestIds()[0];
		manager.clearAll();

		manager.handleVectorPrimitives(answer(0, requestId, [wholeBlock(0, 0)]));

		nodeassert.strictEqual(manager.getSheet(0, 0), undefined);
	});

	it('asks again for the blocks of a request that ran out of time', function () {
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
			[1, 0],
		]);
		ageRequests();
		const before = changes;
		(manager as any)._retireExpired();

		// The waiting block takes the freed place, and the view asks to redraw.
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:1', '1:0']);
		nodeassert.strictEqual(changes, before + 1);
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
			[1, 0],
		]);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:1', '1:0', '0:0']);
		nodeassert.deepStrictEqual(parts(), [0, 0, 0, 0]);
	});

	it('keeps an object that moved to a block named before the one it left', function () {
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
		]);
		const ids = requestIds();
		const object = (version: number): cool.CalcDrawingEntry => ({
			kind: 'drawing',
			id: 42,
			version,
			content: { primitives: [] } as unknown as cool.CalcDrawingContent,
		});
		manager.handleVectorPrimitives(
			answer(0, ids[0], [{ ...wholeBlock(0, 0), objects: [] }]),
		);
		manager.handleVectorPrimitives(
			answer(0, ids[1], [{ ...wholeBlock(0, 1), objects: [object(1)] }]),
		);

		// The push names the block the object joined before the one it left.
		const push = answer(0, undefined as unknown as number, [
			{ ...wholeBlock(0, 0), version: 2, from: 1, objects: [object(2)] },
			{ ...wholeBlock(0, 1), version: 2, from: 1, removed: [42] },
		]);
		push.version = 2;
		push.from = 1;
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		const sheet = manager.getSheet(0, 0);
		nodeassert.strictEqual(sheet?.objects.get(42)?.version, 2);
		nodeassert.strictEqual(sheet?.blocks.get('0:0')?.objects.has(42), true);
		nodeassert.strictEqual(sheet?.blocks.get('0:1')?.objects.has(42), false);
	});

	it('keeps an object that left for a block not held and asks for that block', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		const object = (version: number): cool.CalcDrawingEntry => ({
			kind: 'drawing',
			id: 42,
			version,
			content: { primitives: [] } as unknown as cool.CalcDrawingContent,
		});
		const extent = (blockColumn: number): cool.CalcObjectBlock => ({
			blockRow: 0,
			blockColumn,
			startColumn: blockColumn * 16,
			startRow: 0,
			endColumn: blockColumn * 16 + 2,
			endRow: 4,
		});
		const first = answer(0, requestIds()[0], [
			{ ...wholeBlock(0, 0), objects: [object(1)] },
		]);
		first.objectBlocks = [extent(0)];
		manager.handleVectorPrimitives(first);

		// The push names only the block left: the engine has not built the other.
		const push = answer(0, 0, [
			{ ...wholeBlock(0, 0), version: 2, from: 1, removed: [42] },
		]);
		push.version = 2;
		push.from = 1;
		push.objectBlocks = [extent(1)];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		const sheet = manager.getSheet(0, 0);
		nodeassert.strictEqual(sheet?.objects.get(42)?.version, 1);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:1']);

		manager.handleVectorPrimitives(
			answer(0, requestIds()[1], [
				{ ...wholeBlock(0, 1), version: 2, objects: [object(2)] },
			]),
		);
		nodeassert.strictEqual(sheet?.objects.get(42)?.version, 2);
		nodeassert.strictEqual(sheet?.blocks.get('0:1')?.objects.has(42), true);
	});

	it('serves the sheet shown ahead of the requests of another sheet', function () {
		manager.requestBlocks(0, 0, [
			[0, 0],
			[0, 1],
		]);
		const waiting = requestIds();

		manager.requestBlocks(1, 0, [[0, 0]]);

		nodeassert.deepStrictEqual(parts(), [0, 0, 1]);
		// The answer of the request that gave its place up is still taken.
		manager.handleVectorPrimitives(answer(0, waiting[0], [wholeBlock(0, 0)]));
		nodeassert.strictEqual(manager.getSheet(0, 0)?.blocks.has('0:0'), true);
	});

	it('asks again for a block that went stale while it was asked for', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(0, 0)]),
		);
		manager.markPartStale(0);
		manager.requestBlocks(0, 0, [[0, 0]]);
		const before = requestIds()[1];

		// The sheet changes again before the answer to the request comes.
		manager.markPartStale(0);
		const late = answer(0, before, []);
		late.blocks = ['0:0'];
		manager.handleVectorPrimitives(late);

		nodeassert.strictEqual(
			manager.getSheet(0, 0)?.staleBlocks.has('0:0'),
			true,
		);
		manager.requestBlocks(0, 0, [[0, 0]]);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:0:1', '0:0:1']);

		// The answer to the request sent after the change brings the block up to date.
		const current = answer(0, requestIds()[2], []);
		current.blocks = ['0:0'];
		manager.handleVectorPrimitives(current);
		nodeassert.strictEqual(
			manager.getSheet(0, 0)?.staleBlocks.has('0:0'),
			false,
		);
	});

	it('takes the pushed changes of a stale sheet before it is asked for again', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(0, 0)]),
		);
		manager.markPartStale(0);

		const push = answer(0, 0, [{ ...wholeBlock(0, 0), version: 2, from: 1 }]);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		nodeassert.strictEqual(
			manager.getSheet(0, 0)?.blocks.get('0:0')?.version,
			2,
		);
	});

	it('asks again for a block that a push names stale', function () {
		// After a change to many blocks, the engine names them stale instead of building them.
		manager.requestBlocks(0, 0, [[0, 0]]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(0, 0)]),
		);

		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['0:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		const sheet = manager.getSheet(0, 0);
		nodeassert.strictEqual(sheet?.staleBlocks.has('0:0'), true);
		// The block is still drawn as held, and is not taken as current up to the push.
		nodeassert.strictEqual(sheet?.blocks.get('0:0')?.current, 1);
		manager.requestBlocks(0, 0, [[0, 0]]);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '0:0:1']);
	});

	it('refreshes the stale blocks near the view once the view has all it needs', function () {
		// Hold blocks 0:0, 2:0 and 9:0, then a push names 2:0 and 9:0 stale.
		for (const blockRow of [0, 2, 9]) {
			manager.requestBlocks(0, 0, [[blockRow, 0]]);
			const ids = requestIds();
			manager.handleVectorPrimitives(
				answer(0, ids[ids.length - 1], [wholeBlock(blockRow, 0)]),
			);
		}
		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['2:0', '9:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		// The view shows block 0:0, which is current. Block 2:0 is near it, 9:0 is not.
		manager.requestBlocks(0, 0, [[0, 0]]);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '2:0', '9:0', '2:0:1']);
	});

	it('refreshes the stale blocks the view scrolls towards first, and further', function () {
		// The view shows block 5:0. Blocks 3:0 above it and 7:0 and 10:0 below it are stale.
		for (const blockRow of [5, 3, 7, 10]) {
			manager.requestBlocks(0, 0, [[blockRow, 0]]);
			const ids = requestIds();
			manager.handleVectorPrimitives(
				answer(0, ids[ids.length - 1], [wholeBlock(blockRow, 0)]),
			);
		}
		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['3:0', '7:0', '10:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		// Scrolling down, 7:0 and then 10:0 below come before 3:0 above, and 10:0 is within the
		// longer reach. Held blocks travel together in one request.
		manager.requestBlocks(0, 0, [[5, 0]], [0, 1]);
		nodeassert.deepStrictEqual(blocksAsked().slice(4), ['7:0:1,10:0:1,3:0:1']);
	});

	it('leaves a request free for the view while it refreshes the stale blocks around it', function () {
		// Hold blocks 0:0 and 2:0, then a push names 2:0 stale.
		for (const blockRow of [0, 2]) {
			manager.requestBlocks(0, 0, [[blockRow, 0]]);
			const ids = requestIds();
			manager.handleVectorPrimitives(
				answer(0, ids[ids.length - 1], [wholeBlock(blockRow, 0)]),
			);
		}
		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['2:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		// The view shows 0:0 and 1:0, which is missing and asked for. While its answer is on its
		// way the refresh of 2:0 waits, and a block the view scrolls to goes out at once.
		manager.requestBlocks(0, 0, [
			[0, 0],
			[1, 0],
		]);
		manager.requestBlocks(0, 0, [
			[0, 0],
			[1, 0],
		]);
		nodeassert.deepStrictEqual(blocksAsked().slice(2), ['1:0']);
		manager.requestBlocks(0, 0, [[3, 0]]);
		nodeassert.deepStrictEqual(blocksAsked().slice(2), ['1:0', '3:0']);

		// With both answered, the refresh goes out.
		const ids = requestIds();
		manager.handleVectorPrimitives(answer(0, ids[2], [wholeBlock(1, 0)]));
		manager.handleVectorPrimitives(answer(0, ids[3], [wholeBlock(3, 0)]));
		manager.requestBlocks(0, 0, [
			[0, 0],
			[1, 0],
		]);
		nodeassert.deepStrictEqual(blocksAsked().slice(2), ['1:0', '3:0', '2:0:1']);
	});

	it('asks for a few missing blocks in one request when the view spans many blocks', function () {
		// At a low zoom the view spans 4 by 8 blocks, so missing blocks go four to a request.
		manager.requestBlocks(
			0,
			0,
			[
				[0, 0],
				[1, 0],
				[2, 0],
				[3, 0],
				[4, 0],
				[5, 0],
			],
			[0, 0],
			{ rows: 4, columns: 8 },
		);
		nodeassert.deepStrictEqual(blocksAsked(), ['0:0,1:0,2:0,3:0', '4:0,5:0']);
	});

	it('refreshes stale blocks farther away when the view spans more block rows', function () {
		// Hold blocks 0:0 and 7:0, then a push names 7:0 stale.
		for (const blockRow of [0, 7]) {
			manager.requestBlocks(0, 0, [[blockRow, 0]]);
			const ids = requestIds();
			manager.handleVectorPrimitives(
				answer(0, ids[ids.length - 1], [wholeBlock(blockRow, 0)]),
			);
		}
		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['7:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		// A view of one block row reaches three block rows, which leaves 7:0 out. A view of four
		// block rows reaches eight.
		manager.requestBlocks(0, 0, [[0, 0]], [0, 0], { rows: 1, columns: 1 });
		nodeassert.deepStrictEqual(blocksAsked().slice(2), []);
		manager.requestBlocks(0, 0, [[0, 0]], [0, 0], { rows: 4, columns: 1 });
		nodeassert.deepStrictEqual(blocksAsked().slice(2), ['7:0:1']);
	});

	it('asks for a few held blocks in each request', function () {
		const held = [0, 1, 2, 3, 4, 5];
		for (const blockRow of held) {
			manager.requestBlocks(0, 0, [[blockRow, 0]]);
			const ids = requestIds();
			manager.handleVectorPrimitives(
				answer(0, ids[ids.length - 1], [wholeBlock(blockRow, 0)]),
			);
		}
		manager.markPartStale(0);

		manager.requestBlocks(
			0,
			0,
			held.map((blockRow) => [blockRow, 0]),
		);
		nodeassert.deepStrictEqual(blocksAsked().slice(held.length), [
			'0:0:1,1:0:1,2:0:1,3:0:1',
			'4:0:1,5:0:1',
		]);
	});

	it('asks for a missing block in view before a stale block near it', function () {
		manager.requestBlocks(0, 0, [[2, 0]]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(2, 0)]),
		);
		const push = answer(0, 0, []);
		push.type = 'vectorprimitivesdelta';
		push.version = 2;
		push.from = 1;
		push.stale = ['2:0'];
		delete (push as any).requestId;
		manager.handleVectorPrimitives(push);

		manager.requestBlocks(0, 0, [[0, 0]]);
		nodeassert.deepStrictEqual(blocksAsked(), ['2:0', '0:0']);
	});

	it('asks again for the objects and settings of a stale sheet with no block in view', function () {
		manager.requestBlocks(0, 0, [[0, 0]]);
		manager.handleVectorPrimitives(
			answer(0, requestIds()[0], [wholeBlock(0, 0)]),
		);
		manager.markPartStale(0);

		manager.requestBlocks(0, 0, []);
		manager.requestBlocks(0, 0, []);

		nodeassert.deepStrictEqual(blocksAsked(), ['0:0', '']);
	});
});

describe('CalcVectorManager eviction', function () {
	let recorder: SocketRecorder;
	let savedInstance: any;
	let manager: CalcVectorManager;

	beforeEach(function () {
		recorder = new SocketRecorder();
		savedInstance = (RenderManager as any)._instance;
		(RenderManager as any)._instance = { markVisualsReady() {} };
		manager = new CalcVectorManager();
	});
	afterEach(function () {
		(manager as any)._disarmExpiry();
		recorder.restore();
		(RenderManager as any)._instance = savedInstance;
	});

	// Holds the sheet with its first block in view.
	function hold(part: number): void {
		manager.requestBlocks(part, 0, [[0, 0]]);
		const asked = recorder.sent[recorder.sent.length - 1];
		const requestId = Number(asked.split('requestid=')[1].split('&')[0]);
		manager.handleVectorPrimitives({
			type: 'vectorprimitives',
			docType: 'spreadsheet',
			mode: 0,
			partId: String(part),
			epoch: 7,
			requestId,
			version: 1,
			objects: [
				{
					kind: 'cells',
					blockRow: 0,
					blockColumn: 0,
					startRow: 0,
					startColumn: 0,
					endRow: 63,
					endColumn: 15,
					version: 1,
					from: 0,
					bounds: { x: [0, 0], y: [0, 0], right: [16, 0], bottom: [64, 0] },
					rows: [],
				},
			],
		});
	}

	function heldParts(): number[] {
		return Array.from((manager as any)._sheets.values()).map(
			(sheet: any) => sheet.part,
		);
	}

	it('drops the sheets shown longest ago down to the limit', function () {
		const limit = (CalcVectorManager as any).maxSheets;
		for (let part = 0; part <= limit; part++) hold(part);

		manager.evict(limit, 0, new Set(['0:0']));

		nodeassert.strictEqual(manager.getSheet(0, 0), undefined);
		nodeassert.deepStrictEqual(heldParts().length, limit);
		nodeassert.strictEqual(manager.getSheet(limit, 0) !== undefined, true);
	});

	it('keeps a sheet shown again ahead of the ones shown before', function () {
		const limit = (CalcVectorManager as any).maxSheets;
		for (let part = 0; part < limit; part++) hold(part);
		manager.evict(0, 0, new Set(['0:0']));
		hold(limit);

		manager.evict(limit, 0, new Set(['0:0']));

		nodeassert.strictEqual(manager.getSheet(0, 0) !== undefined, true);
		nodeassert.strictEqual(manager.getSheet(1, 0), undefined);
	});
});
