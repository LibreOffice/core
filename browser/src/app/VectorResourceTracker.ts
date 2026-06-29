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
/// demand: the loaded values, the ids in flight, the ids that turned
/// out unavailable, and which parts use which id.
class VectorResourceTracker<K, V> {
	private _cache: Map<K, V> = new Map();
	private _inFlight: Set<K> = new Set();
	private _unavailable: Set<K> = new Set();
	private _idToParts: Map<K, Set<number>> = new Map();

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
	indexForPart(part: number, ids: Set<K>): void {
		for (const id of ids) {
			let parts = this._idToParts.get(id);
			if (!parts) {
				parts = new Set<number>();
				this._idToParts.set(id, parts);
			}
			parts.add(part);
		}
	}

	/// Parts that use the id.
	partsFor(id: K): Set<number> | undefined {
		return this._idToParts.get(id);
	}

	/// Request every id that is neither loaded, nor in flight, nor
	/// unavailable.
	requestMissing(ids: Set<K>): void {
		for (const id of ids) {
			if (this._cache.has(id)) continue;
			if (this._inFlight.has(id)) continue;
			if (this._unavailable.has(id)) continue;
			this._inFlight.add(id);
			app.socket.sendMessage(this._requestMessage(id));
		}
	}

	/// Store a loaded value and clear the in-flight mark.
	setLoaded(id: K, value: V): void {
		this._cache.set(id, value);
		this._inFlight.delete(id);
	}

	/// Record an id no usable data could be obtained for, so it is not
	/// requested again.
	setUnavailable(id: K): void {
		this._unavailable.add(id);
		this._inFlight.delete(id);
	}

	clearInFlight(id: K): void {
		this._inFlight.delete(id);
	}

	clear(): void {
		this._cache.clear();
		this._inFlight.clear();
		this._unavailable.clear();
		this._idToParts.clear();
	}
}
