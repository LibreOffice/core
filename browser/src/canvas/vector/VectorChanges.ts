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
	/// The callbacks to call when the vector content held changes, in the order they came.
	export class VectorChangeListeners {
		private listeners: VectorChangeListener[] = [];

		add(listener: VectorChangeListener): void {
			this.listeners.push(listener);
		}

		remove(listener: VectorChangeListener): void {
			this.listeners = this.listeners.filter((other) => other !== listener);
		}

		/// Calls each listener with the ids of the objects an answer of the engine
		/// replaced, and the newest input of this view the answer reflects.
		fire(answered?: number[], inputSequence?: number): void {
			for (const listener of this.listeners) listener(answered, inputSequence);
		}
	}

	/// The ids of the objects drawn apart from the rest of the content, such as the selection
	/// during a drag, and a version that counts up each time the ids change.
	export class DrawnElsewhere {
		private ids: Set<number> = new Set();
		private count: number = 0;

		/// Takes the ids. An empty list leaves nothing out.
		set(objectIds: number[]): void {
			const next = new Set(objectIds);
			const same =
				next.size === this.ids.size &&
				objectIds.every((id) => this.ids.has(id));
			if (same) return;
			this.ids = next;
			this.count++;
		}

		has(objectId: number): boolean {
			return this.ids.has(objectId);
		}

		version(): number {
			return this.count;
		}
	}
}
