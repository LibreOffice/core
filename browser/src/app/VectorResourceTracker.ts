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

/// Bookkeeping for one kind of content-addressed resource fetched on
/// demand: the loaded values, the ids asked for, the ids that turned
/// out unavailable, and which parts use which id. A part is named by its
/// cool.VectorPartGuid, so the same index in two modes is tracked apart.
class VectorResourceTracker<K, V> {
	/// How long a request waits for its answer, in milliseconds. After that the id is asked for
	/// again while a part uses it.
	private static readonly requestTimeout = 10000;

	private _cache: Map<K, V> = new Map();
	private _requests = new cool.RequestTracker<K>({
		timeout: VectorResourceTracker.requestTimeout,
		expired: (requests) => this._requestAgain(requests),
	});
	private _unavailable: Set<K> = new Set();
	private _idToParts: Map<K, Set<cool.VectorPartGuid>> = new Map();

	// Builds the outgoing request message for an id.
	private _requestMessage: (id: K) => string;

	constructor(requestMessage: (id: K) => string) {
		this._requestMessage = requestMessage;
	}

	has(id: K): boolean {
		return this._cache.has(id);
	}

	get(id: K): V | undefined {
		return this._cache.get(id);
	}

	values(): IterableIterator<V> {
		return this._cache.values();
	}

	/// Remember that the part uses the ids.
	indexForPart(partId: cool.VectorPartGuid, ids: Set<K>): void {
		for (const id of ids) {
			let parts = this._idToParts.get(id);
			if (!parts) {
				parts = new Set<cool.VectorPartGuid>();
				this._idToParts.set(id, parts);
			}
			parts.add(partId);
		}
	}

	/// Parts that use the id.
	partsFor(id: K): Set<cool.VectorPartGuid> | undefined {
		return this._idToParts.get(id);
	}

	/// Forget that the part uses anything, and drop the values nothing uses
	/// any more.
	releasePart(partId: cool.VectorPartGuid): void {
		for (const [id, parts] of Array.from(this._idToParts.entries())) {
			if (!parts.delete(partId) || parts.size > 0) continue;
			this._idToParts.delete(id);
			this._cache.delete(id);
		}
	}

	/// Make the ids all that the part uses. The part lets go of the ids it no longer uses, and
	/// with dropUnused a value that nothing uses any more goes. An id the part uses before and
	/// after keeps its value.
	replacePart(
		partId: cool.VectorPartGuid,
		ids: Set<K>,
		dropUnused: boolean,
	): void {
		for (const [id, parts] of Array.from(this._idToParts.entries())) {
			if (ids.has(id) || !parts.delete(partId) || parts.size > 0) continue;
			this._idToParts.delete(id);
			if (dropUnused) this._cache.delete(id);
		}
		this.indexForPart(partId, ids);
	}

	/// Forget that the part uses anything, keeping the values.
	forgetPart(partId: cool.VectorPartGuid): void {
		for (const [id, parts] of Array.from(this._idToParts.entries())) {
			if (parts.delete(partId) && parts.size === 0) this._idToParts.delete(id);
		}
	}

	/// Forget which parts use what, keeping the values.
	forgetAllParts(): void {
		this._idToParts.clear();
	}

	/// Request every id that is neither loaded, nor asked for, nor
	/// unavailable.
	requestMissing(ids: Set<K>): void {
		for (const id of ids) {
			if (this._cache.has(id)) continue;
			if (this._unavailable.has(id)) continue;
			this._requests.request(id, () =>
				app.socket.sendMessage(this._requestMessage(id)),
			);
		}
	}

	// The requests got no answer in time. Their ids that a part still uses are asked for again.
	private _requestAgain(requests: cool.RequestOut<K, undefined>[]): void {
		const ids = new Set<K>();
		for (const request of requests)
			for (const id of request.keys) if (this._idToParts.has(id)) ids.add(id);
		this.requestMissing(ids);
	}

	/// Store a loaded value, which answers the request for the id.
	setLoaded(id: K, value: V): void {
		this._cache.set(id, value);
		this._requests.answered(id);
	}

	/// Record an id no usable data could be obtained for, so it is not
	/// requested again.
	setUnavailable(id: K): void {
		this._unavailable.add(id);
		this._requests.answered(id);
	}

	/// The request for the id is answered without a value, so the id can be asked for again.
	clearInFlight(id: K): void {
		this._requests.answered(id);
	}

	/// Drop the loaded values, keeping which parts use which id. A dropped id is requested
	/// again as any missing one.
	dropValues(): void {
		this._cache.clear();
		this._requests.forgetAll();
	}

	clear(): void {
		this._cache.clear();
		this._requests.forgetAll();
		this._unavailable.clear();
		this._idToParts.clear();
	}
}
