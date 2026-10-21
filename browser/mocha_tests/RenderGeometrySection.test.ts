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

describe('RenderGeometrySection', function () {
	// An object keeps its color for as long as it lives, so its rectangle
	// stays recognizable as it moves, and two neighboring ids come out far
	// apart rather than nearly the same.
	it('gives an object a color of its own', function () {
		const first = RenderGeometrySection.colorOfObject(7);
		nodeassert.strictEqual(first, RenderGeometrySection.colorOfObject(7));
		nodeassert.notStrictEqual(first, RenderGeometrySection.colorOfObject(8));

		const hueOf = (color: string) => parseFloat(color.slice(4));
		const apart = Math.abs(
			hueOf(first) - hueOf(RenderGeometrySection.colorOfObject(8)),
		);
		nodeassert.ok(
			Math.min(apart, 360 - apart) > 60,
			'neighboring ids are far apart in hue, got ' + apart,
		);
	});

	// The outline follows the object's own axes, so a rotated object gets a
	// rotated rectangle rather than the upright box around it.
	it('walks the unit rectangle corners through the transform', function () {
		const cornersOf = (values: number[]) =>
			RenderGeometrySection.unitRectangleCorners(
				cool.Matrix2D.fromArray(values),
			).map((corner: cool.Point) => [corner.x, corner.y]);

		nodeassert.deepStrictEqual(cornersOf([300, 0, 0, 200, 10, 20]), [
			[10, 20],
			[310, 20],
			[310, 220],
			[10, 220],
		]);

		// A quarter turn of a 100 by 100 square anchored at (100, 0).
		nodeassert.deepStrictEqual(cornersOf([0, 100, -100, 0, 100, 0]), [
			[100, 0],
			[100, 100],
			[0, 100],
			[0, 0],
		]);
	});

	describe('answers to the changes made here', function () {
		let object: any;
		let notForeseen: number[];
		let savedObjectOf: any;
		let savedUnpredicted: any;
		let savedInstance: any;

		// One 100 by 100 object, id 5, standing at the origin.
		beforeEach(function () {
			object = {
				transform: [100, 0, 0, 100, 0, 0],
				x: 0,
				y: 0,
				width: 100,
				height: 100,
				primitives: [],
			};
			notForeseen = [];
			savedObjectOf = RenderGeometrySection.objectOf;
			(RenderGeometrySection as any).objectOf = (objectId: number) =>
				objectId === 5 ? object : undefined;
			savedUnpredicted = GraphicSelection.unpredictedAnswer;
			(GraphicSelection as any).unpredictedAnswer = (objectId?: number) => {
				if (objectId !== undefined) notForeseen.push(objectId);
			};
			savedInstance = (RenderManager as any)._instance;
			(RenderManager as any)._instance = new VectorManager();
			(RenderGeometrySection as any).pending.clear();
		});
		afterEach(function () {
			(RenderGeometrySection as any).objectOf = savedObjectOf;
			(GraphicSelection as any).unpredictedAnswer = savedUnpredicted;
			(RenderManager as any)._instance = savedInstance;
			(RenderGeometrySection as any).pending.clear();
			(ChangeOutbox as any).lastTicket = null;
		});

		// A ticket for a message that went out with that number.
		function sentAs(sequence: number): cool.InputTicket {
			const ticket = new cool.InputTicket();
			ticket.goesOutAs(sequence);
			return ticket;
		}

		// Moves the object by that much here, in a change with that ticket.
		function move(by: number, ticket: cool.InputTicket | null): void {
			RenderGeometrySection.changeObject(
				5,
				cool.Matrix2D.IDENTITY.translate(by, 0),
				ticket,
			);
		}

		// The engine's answer with the object at that place and that newest applied input.
		function answer(at: number, inputSequence: number): void {
			object.transform = [100, 0, 0, 100, at, 0];
			object.x = at;
			(RenderGeometrySection as any).takeUpTheAnswers([5], inputSequence);
		}

		it('shows the changes the engine has not applied yet on top of its answer', function () {
			move(100, sentAs(11));
			move(100, sentAs(12));
			answer(100, 11);
			nodeassert.strictEqual(object.transform[4], 200);
			nodeassert.ok(RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, []);
		});

		it('answers every change up to the newest by number, not by place', function () {
			move(100, sentAs(11));
			move(-100, sentAs(12));
			move(100, sentAs(13));
			answer(100, 13);
			nodeassert.strictEqual(object.transform[4], 100);
			nodeassert.ok(!RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, []);
		});

		it("shows the engine's answer where it differs from the prediction", function () {
			move(100, sentAs(11));
			answer(150, 11);
			nodeassert.strictEqual(object.transform[4], 150);
			nodeassert.ok(!RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, [5]);
		});

		it('keeps a change whose message waits to go out on top of the answer', function () {
			move(100, new cool.InputTicket());
			answer(0, 11);
			nodeassert.strictEqual(object.transform[4], 100);
			nodeassert.ok(RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, []);
		});

		it('takes an answer older than every change as one for none of them', function () {
			move(100, sentAs(12));
			move(100, sentAs(13));
			answer(0, 11);
			nodeassert.strictEqual(object.transform[4], 200);
			nodeassert.ok(RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, []);
		});

		it('answers a change without a number with the first answer', function () {
			move(100, null);
			answer(100, 11);
			nodeassert.strictEqual(object.transform[4], 100);
			nodeassert.ok(!RenderGeometrySection.awaitsAnAnswer(5));
			nodeassert.deepStrictEqual(notForeseen, []);
		});
	});
});
