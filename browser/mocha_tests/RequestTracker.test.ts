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

describe('RequestTracker', function () {
	// Move the time every request was sent back past a timeout of 1000 milliseconds.
	function ageRequests<K, D>(tracker: cool.RequestTracker<K, D>): void {
		for (const request of tracker.requests()) request.sent -= 2000;
	}

	it('sends for a key once while its request is out', function () {
		const tracker = new cool.RequestTracker<string>();
		const sent: string[] = [];
		tracker.request('a', () => sent.push('a'));
		tracker.request('a', () => sent.push('a'));
		nodeassert.deepStrictEqual(sent, ['a']);
		nodeassert.ok(tracker.isOut('a'));

		tracker.answered('a');
		nodeassert.ok(!tracker.isOut('a'));
		tracker.request('a', () => sent.push('a'));
		nodeassert.deepStrictEqual(sent, ['a', 'a']);
	});

	it('lets the limited requests over the limit go out one per answer, in the order asked', function () {
		const tracker = new cool.RequestTracker<string>({ limit: 2 });
		const sent: string[] = [];
		for (const key of ['a', 'b', 'c', 'd'])
			tracker.request(key, () => sent.push(key), { limited: true });
		// A second ask for a waiting key does not make it wait twice.
		tracker.request('c', () => sent.push('c'), { limited: true });
		nodeassert.deepStrictEqual(sent, ['a', 'b']);
		nodeassert.ok(!tracker.isOut('c'));

		tracker.answered('b');
		nodeassert.deepStrictEqual(sent, ['a', 'b', 'c']);
		tracker.answered('a');
		nodeassert.deepStrictEqual(sent, ['a', 'b', 'c', 'd']);
		tracker.answered('c');
		tracker.answered('d');
		nodeassert.strictEqual(tracker.requestCount, 0);
	});

	it('sends a request that is not limited whatever the limit', function () {
		const tracker = new cool.RequestTracker<string>({ limit: 1 });
		const sent: string[] = [];
		tracker.request('a', () => sent.push('a'), { limited: true });
		tracker.request('b', () => sent.push('b'));
		tracker.request('c', () => sent.push('c'), { limited: true });
		nodeassert.deepStrictEqual(sent, ['a', 'b']);

		// The answer of the request that is not limited frees no place.
		tracker.answered('b');
		nodeassert.deepStrictEqual(sent, ['a', 'b']);
		tracker.answered('a');
		nodeassert.deepStrictEqual(sent, ['a', 'b', 'c']);
	});

	it('passes over a waiting request that is no longer needed', function () {
		const tracker = new cool.RequestTracker<string>({ limit: 1 });
		const sent: string[] = [];
		let needed = true;
		tracker.request('a', () => sent.push('a'), { limited: true });
		tracker.request('b', () => sent.push('b'), {
			limited: true,
			needed: () => needed,
		});
		tracker.request('c', () => sent.push('c'), { limited: true });

		needed = false;
		tracker.answered('a');
		nodeassert.deepStrictEqual(sent, ['a', 'c']);
		nodeassert.ok(!tracker.isOut('b'));
	});

	it('sends a waiting key at once when it is asked for without the limit', function () {
		const tracker = new cool.RequestTracker<string>({ limit: 1 });
		const sent: string[] = [];
		tracker.request('a', () => sent.push('a'), { limited: true });
		tracker.request('b', () => sent.push('waited'), { limited: true });

		tracker.request('b', () => sent.push('b'));
		tracker.answered('a');
		nodeassert.deepStrictEqual(sent, ['a', 'b']);
	});

	it('forgets a key out or waiting, and lets the next waiting key go out', function () {
		const tracker = new cool.RequestTracker<string>({ limit: 1 });
		const sent: string[] = [];
		for (const key of ['a', 'b', 'c'])
			tracker.request(key, () => sent.push(key), { limited: true });

		tracker.forget('b');
		nodeassert.deepStrictEqual(sent, ['a']);

		tracker.forget('a');
		nodeassert.deepStrictEqual(sent, ['a', 'c']);
	});

	it('sends one request for several keys, and closes it by its id', function () {
		const tracker = new cool.RequestTracker<string, string>();
		const ids: number[] = [];
		const first = tracker.requestKeys(['a', 'b'], 'one', (id) => ids.push(id));
		const second = tracker.requestKeys([], 'two', (id) => ids.push(id));
		nodeassert.deepStrictEqual(ids, [first, second]);
		nodeassert.ok(second > first);
		nodeassert.strictEqual(tracker.nextRequestId, second + 1);
		nodeassert.ok(tracker.isOut('a') && tracker.isOut('b'));
		nodeassert.strictEqual(tracker.requestCount, 2);

		const taken = tracker.takeRequest(first);
		nodeassert.deepStrictEqual(taken?.keys, ['a', 'b']);
		nodeassert.strictEqual(taken?.data, 'one');
		nodeassert.ok(!tracker.isOut('a') && !tracker.isOut('b'));
		nodeassert.strictEqual(tracker.takeRequest(first), undefined);
		nodeassert.strictEqual(tracker.requestCount, 1);
	});

	it('forgets the requests a given check accepts', function () {
		const tracker = new cool.RequestTracker<string, string>();
		tracker.requestKeys(['x/1', 'x/2'], 'x', () => undefined);
		tracker.requestKeys(['y/1'], 'y', () => undefined);

		tracker.forgetWhere((request) => request.data === 'x');
		nodeassert.deepStrictEqual(Array.from(tracker.outKeys()), ['y/1']);
	});

	it('gives up a request that is out for longer than the timeout', function () {
		const expired: string[] = [];
		const tracker = new cool.RequestTracker<string>({
			timeout: 1000,
			expired: (requests) =>
				requests.forEach((request) => expired.push(...request.keys)),
		});
		const sent: string[] = [];
		tracker.request('a', () => sent.push('a'));
		tracker.expire();
		nodeassert.deepStrictEqual(expired, []);

		ageRequests(tracker);
		tracker.request('b', () => sent.push('b'));
		tracker.expire();
		nodeassert.deepStrictEqual(expired, ['a']);
		nodeassert.ok(!tracker.isOut('a') && tracker.isOut('b'));

		// The key can be asked for again, and a late answer changes nothing.
		tracker.request('a', () => sent.push('a'));
		nodeassert.deepStrictEqual(sent, ['a', 'b', 'a']);
		tracker.forgetAll();
		tracker.answered('a');
	});

	it('lets a waiting request go out in place of one that was given up', function () {
		const tracker = new cool.RequestTracker<string>({
			limit: 1,
			timeout: 1000,
		});
		const sent: string[] = [];
		tracker.request('a', () => sent.push('a'), { limited: true });
		tracker.request('b', () => sent.push('b'), { limited: true });

		ageRequests(tracker);
		nodeassert.ok(tracker.dropLate());
		nodeassert.deepStrictEqual(sent, ['a', 'b']);
		tracker.forgetAll();
	});

	describe('with a clock', function () {
		beforeEach(function () {
			installFakeClock();
		});
		afterEach(function () {
			restoreClock();
		});

		it('gives up a late request by itself once the timeout passed', function () {
			let expired = 0;
			const tracker = new cool.RequestTracker<string>({
				timeout: 1000,
				expired: () => expired++,
			});
			tracker.request('a', () => undefined);
			ageRequests(tracker);

			advanceClock(500);
			nodeassert.strictEqual(expired, 0);
			advanceClock(600);
			nodeassert.strictEqual(expired, 1);
			nodeassert.ok(!tracker.isOut('a'));
		});
	});

	it('forgets the requests of every tracker at once', function () {
		const one = new cool.RequestTracker<string>({ limit: 1 });
		const other = new cool.RequestTracker<number>();
		const sent: string[] = [];
		one.request('a', () => sent.push('a'), { limited: true });
		one.request('b', () => sent.push('b'), { limited: true });
		other.request(1, () => undefined);

		cool.RequestTracker.forgetEverywhere();
		nodeassert.ok(!one.isOut('a') && !other.isOut(1));
		nodeassert.deepStrictEqual(sent, ['a']);
		// The waiting key was forgotten too, so it goes out when asked for.
		one.request('b', () => sent.push('b'), { limited: true });
		nodeassert.deepStrictEqual(sent, ['a', 'b']);
	});
});
