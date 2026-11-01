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

/*
 * TimerRegistry - A registry for tagged timers to support Cypress testing.
 *
 * Allows timers to be registered with a tag, and provides methods to check
 * whether timers with a given tag have fired or are still active.
 */

interface TimerEntry {
	tag: string;
	type: 'timeout' | 'interval';
	id: ReturnType<typeof setTimeout>;
	fireCount: number;
	cancelled: boolean;
}

interface FrameEntry {
	tag: string;
	type: 'frame';
	id: number;
	fireCount: number;
	cancelled: boolean;
	// The window whose animation frame the callback waits for.
	owner: Window;
}

class TimerRegistry {
	private timers: Map<ReturnType<typeof setTimeout>, TimerEntry> = new Map();
	// Animation frames count their ids on their own, so an id can equal the id of a timer.
	private frames: Map<number, FrameEntry> = new Map();

	setTimeout(
		tag: string,
		callback: () => void,
		delay: number,
	): ReturnType<typeof setTimeout> {
		const id = setTimeout(() => {
			const entry = this.timers.get(id);
			if (entry) {
				entry.fireCount++;
			}
			callback();
			// Auto-remove completed timeouts
			this.timers.delete(id);
		}, delay);

		this.timers.set(id, {
			tag,
			type: 'timeout',
			id,
			fireCount: 0,
			cancelled: false,
		});

		return id;
	}

	setInterval(
		tag: string,
		callback: () => void,
		delay: number,
	): ReturnType<typeof setInterval> {
		const id = setInterval(() => {
			const entry = this.timers.get(id);
			if (entry) {
				entry.fireCount++;
			}
			callback();
		}, delay);

		this.timers.set(id, {
			tag,
			type: 'interval',
			id,
			fireCount: 0,
			cancelled: false,
		});

		return id;
	}

	requestAnimationFrame(
		tag: string,
		callback: () => void,
		owner: Window = window,
	): number {
		const id = owner.requestAnimationFrame(() => {
			const entry = this.frames.get(id);
			if (entry) {
				entry.fireCount++;
			}
			callback();
			this.frames.delete(id);
		});

		this.frames.set(id, {
			tag,
			type: 'frame',
			id,
			fireCount: 0,
			cancelled: false,
			owner,
		});

		return id;
	}

	cancelAnimationFrame(id: number): void {
		const entry = this.frames.get(id);
		if (entry) {
			entry.cancelled = true;
			entry.owner.cancelAnimationFrame(id);
			this.frames.delete(id);
		}
	}

	clearTimeout(id: ReturnType<typeof setTimeout>): void {
		const entry = this.timers.get(id);
		if (entry) {
			entry.cancelled = true;
			clearTimeout(id);
			this.timers.delete(id);
		}
	}

	clearInterval(id: ReturnType<typeof setInterval>): void {
		const entry = this.timers.get(id);
		if (entry) {
			entry.cancelled = true;
			clearInterval(id);
			this.timers.delete(id);
		}
	}

	// For testing: check if all active timers with the given tag have fired at least once
	hasFired(tag: string): boolean {
		for (const entry of this.entries()) {
			if (entry.tag === tag && entry.fireCount === 0) {
				return false;
			}
		}
		return true;
	}

	// For testing: check if there are any active (not cancelled) timers with the given tag
	hasActive(tag: string): boolean {
		for (const entry of this.entries()) {
			if (entry.tag === tag) {
				return true;
			}
		}
		return false;
	}

	// For testing: get the number of times timers with the given tag have fired
	getFireCount(tag: string): number {
		let count = 0;
		for (const entry of this.entries()) {
			if (entry.tag === tag) {
				count += entry.fireCount;
			}
		}
		return count;
	}

	private entries(): (TimerEntry | FrameEntry)[] {
		return [...this.timers.values(), ...this.frames.values()];
	}
}

// Global instance
(window as any).app = (window as any).app || {};
window.app.timerRegistry = new TimerRegistry();
