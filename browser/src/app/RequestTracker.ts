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

namespace cool {
	/// A request that is out, with the keys it asks for and the data noted with it. id counts
	/// from 1 in its tracker. sent is a performance.now time. limited counts towards the limit.
	export interface RequestOut<K, D> {
		id: number;
		keys: K[];
		data: D | undefined;
		sent: number;
		limited: boolean;
	}

	/// limit is how many limited requests may be out at a time, any number when absent. timeout
	/// is in milliseconds, none when absent. expired gets the requests that were out longer.
	export interface RequestTrackerOptions<K, D> {
		limit?: number;
		timeout?: number;
		expired?: (requests: RequestOut<K, D>[]) => void;
	}

	/// limited counts the request towards the limit. needed says whether the request is still
	/// wanted as it goes out, always when absent. data is noted with the request.
	export interface RequestOptions<D> {
		limited?: boolean;
		needed?: () => boolean;
		data?: D;
	}

	/*
		Tracks the requests that were sent and have no answer yet, so that no key is
		requested twice. A request asks for one key or for several, and comes with the
		function that sends its message.

		Limited requests over the limit wait in a queue, and go out as earlier ones are
		answered. A request out for longer than the timeout is given up, so its keys can be
		requested again.

		All trackers share the one connection to the server. forgetEverywhere forgets the
		requests of every tracker when that connection drops.
	*/
	export class RequestTracker<K, D = undefined> {
		// The trackers that have a request out.
		private static _withRequestsOut: Set<{ forgetAll(): void }> = new Set();

		/// Forget the requests of every tracker. Nothing is sent.
		static forgetEverywhere(): void {
			for (const tracker of Array.from(RequestTracker._withRequestsOut))
				tracker.forgetAll();
		}

		private _requests: Map<number, RequestOut<K, D>> = new Map();
		// Each key that is out, with the id of its request.
		private _requestOfKey: Map<K, number> = new Map();
		// The limited requests over the limit, oldest first.
		private _waiting: {
			key: K;
			send: () => void;
			options: RequestOptions<D>;
		}[] = [];
		private _nextRequestId = 1;
		private _limitedOut = 0;
		// Runs when the oldest request reaches the timeout.
		private _timer: ReturnType<typeof setTimeout> | undefined = undefined;
		private readonly _limit: number;
		private readonly _timeout: number | undefined;
		private readonly _expired?: (requests: RequestOut<K, D>[]) => void;

		constructor(options: RequestTrackerOptions<K, D> = {}) {
			this._limit = options.limit ?? Infinity;
			this._timeout = options.timeout;
			this._expired = options.expired;
		}

		isOut(key: K): boolean {
			return this._requestOfKey.has(key);
		}

		get requestCount(): number {
			return this._requests.size;
		}

		/// The id of the next request. The requests that are out have smaller ids.
		get nextRequestId(): number {
			return this._nextRequestId;
		}

		/// The requests that are out, oldest first.
		requests(): IterableIterator<RequestOut<K, D>> {
			return this._requests.values();
		}

		outKeys(): IterableIterator<K> {
			return this._requestOfKey.keys();
		}

		/// Ask for the key, unless it is out. A limited request over the limit waits. Any
		/// other request goes out now, also for a key that waited.
		request(key: K, send: () => void, options: RequestOptions<D> = {}): void {
			if (this._requestOfKey.has(key)) return;
			if (!options.limited) {
				this._dropWaiting(key);
				this._sendKey(key, send, options);
				return;
			}
			if (this._waiting.some((one) => one.key === key)) return;
			if (this._limitedOut < this._limit) this._sendKey(key, send, options);
			else this._waiting.push({ key: key, send: send, options: options });
		}

		/// Send one request for all the keys. Gives its id to send, and returns it.
		requestKeys(keys: K[], data: D, send: (requestId: number) => void): number {
			const request = this._add(keys.slice(), data, false);
			send(request.id);
			return request.id;
		}

		/// The answer for the key came. Its request closes with its last key.
		answered(key: K): void {
			const id = this._requestOfKey.get(key);
			const request = id === undefined ? undefined : this._requests.get(id);
			if (!request) return;
			this._requestOfKey.delete(key);
			request.keys = request.keys.filter((other) => other !== key);
			if (request.keys.length === 0) this._remove(request);
			this._sendWaiting();
		}

		/// Close the request with the id and return it, undefined when it is not out.
		takeRequest(id: number): RequestOut<K, D> | undefined {
			const request = this._requests.get(id);
			if (!request) return undefined;
			this._remove(request);
			this._sendWaiting();
			return request;
		}

		/// Forget the key, out or waiting.
		forget(key: K): void {
			this._dropWaiting(key);
			this.answered(key);
		}

		/// Forget the requests out that the check accepts.
		forgetWhere(accepts: (request: RequestOut<K, D>) => boolean): void {
			for (const request of Array.from(this._requests.values()))
				if (accepts(request)) this._remove(request);
			this._sendWaiting();
		}

		/// Forget every request, out or waiting. Nothing is sent.
		forgetAll(): void {
			this._requests.clear();
			this._requestOfKey.clear();
			this._waiting = [];
			this._limitedOut = 0;
			this._stopTimer();
			RequestTracker._withRequestsOut.delete(this);
		}

		/// Give up the requests out for longer than the timeout. True when there was one.
		dropLate(): boolean {
			const late = this._takeLate();
			this._sendWaiting();
			return late.length > 0;
		}

		/// Give up the requests out for longer than the timeout, and pass them to expired.
		expire(): void {
			const late = this._takeLate();
			this._sendWaiting();
			if (late.length > 0) this._expired?.(late);
			this._startTimer();
		}

		private _takeLate(): RequestOut<K, D>[] {
			const timeout = this._timeout;
			if (timeout === undefined) return [];
			const now = performance.now();
			const late = Array.from(this._requests.values()).filter(
				(request) => now - request.sent >= timeout,
			);
			for (const request of late) this._remove(request);
			return late;
		}

		private _startTimer(): void {
			const timeout = this._timeout;
			if (timeout === undefined || this._timer !== undefined) return;
			if (this._requests.size === 0) return;
			let oldest = Infinity;
			for (const request of this._requests.values())
				oldest = Math.min(oldest, request.sent);
			const wait = Math.max(0, oldest + timeout - performance.now());
			this._timer = setTimeout(() => {
				this._timer = undefined;
				this.expire();
			}, wait + 1);
		}

		private _stopTimer(): void {
			if (this._timer === undefined) return;
			clearTimeout(this._timer);
			this._timer = undefined;
		}

		private _add(
			keys: K[],
			data: D | undefined,
			limited: boolean,
		): RequestOut<K, D> {
			const request: RequestOut<K, D> = {
				id: this._nextRequestId++,
				keys: keys,
				data: data,
				sent: performance.now(),
				limited: limited,
			};
			this._requests.set(request.id, request);
			for (const key of keys) this._requestOfKey.set(key, request.id);
			if (limited) this._limitedOut++;
			RequestTracker._withRequestsOut.add(this);
			this._startTimer();
			return request;
		}

		// A key asked for again while it was out belongs to the later request.
		private _remove(request: RequestOut<K, D>): void {
			this._requests.delete(request.id);
			for (const key of request.keys)
				if (this._requestOfKey.get(key) === request.id)
					this._requestOfKey.delete(key);
			if (request.limited) this._limitedOut--;
			if (this._requests.size > 0) return;
			this._stopTimer();
			RequestTracker._withRequestsOut.delete(this);
		}

		private _dropWaiting(key: K): void {
			if (this._waiting.length === 0) return;
			this._waiting = this._waiting.filter((one) => one.key !== key);
		}

		private _sendKey(
			key: K,
			send: () => void,
			options: RequestOptions<D>,
		): void {
			if (options.needed && !options.needed()) return;
			this._add([key], options.data, options.limited === true);
			send();
		}

		private _sendWaiting(): void {
			while (this._limitedOut < this._limit && this._waiting.length > 0) {
				const next = this._waiting.shift();
				if (next) this._sendKey(next.key, next.send, next.options);
			}
		}
	}
}
