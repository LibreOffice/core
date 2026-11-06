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

// The bridge between the LibreLogo panel and the document. Everything that
// touches the document runs inside the kit through cool.remote; this
// module queues those operations and ships them in batches so a run costs a
// handful of round trips however much it draws.

// Queues document operations from the panel and ships them to the kit in
// batches, one flush at a time.
export class KitBridge {
	constructor(options) {
		this.maxOpsPerCall = (options && options.maxOpsPerCall) || 100;
		this.queue = [];
		this.flushing = null;
	}

	enqueue(op) {
		this.queue.push(op);
	}

	// Send the queued operations, at most maxOpsPerCall per round trip, in
	// order. Concurrent callers are serialized: each flush claims the ops that
	// are queued when it is called (so results map to the caller's own ops) and
	// waits for the previous flush to finish before sending, so batches from
	// overlapping flush() calls never interleave or race on this.flushing.
	async flush() {
		// Claim the current queue synchronously; a later flush() gets a fresh one.
		const mine = this.queue;
		this.queue = [];
		const prev = this.flushing;
		let done;
		this.flushing = new Promise((resolve) => { done = resolve; });
		try {
			if (prev) await prev;
			const results = [];
			for (let i = 0; i < mine.length; i += this.maxOpsPerCall) {
				const batch = mine.slice(i, i + this.maxOpsPerCall);
				const r = await window.cool.remote.kitApply(batch);
				if (Array.isArray(r)) results.push(...r);
			}
			return results;
		} finally {
			done();
		}
	}

	async pageSize() {
		return window.cool.remote.kitPageSize();
	}

	async programText() {
		return window.cool.remote.kitProgramText();
	}

	async documentLanguage() {
		return window.cool.remote.kitDocumentLanguage();
	}

	// Put the drawing into the document. bounds = [minX, minY, maxX, maxY] in
	// points, page = [w, h]; mode 'page' anchors it where the turtle drew,
	// 'cursor' inserts it inline at the cursor.
	insertDrawing(svg, bounds, page, mode) {
		// The image always carries the whole drawing: its viewBox is the
		// drawing's own bounding box, never clipped to the page.
		const m = 2;
		const dx = bounds[0] - m, dy = bounds[1] - m;
		const dw = Math.max(4, bounds[2] - bounds[0] + 2 * m);
		const dh = Math.max(4, bounds[3] - bounds[1] + 2 * m);
		const cropped = svg.replace(/viewBox="[^"]*"/, 'viewBox="' + dx + ' ' + dy + ' ' + dw + ' ' + dh + '"')
			.replace(/width="[^"]*pt"/, 'width="' + dw + 'pt"').replace(/height="[^"]*pt"/, 'height="' + dh + 'pt"');
		let type, x, y, w, h;
		if (mode === 'cursor') {
			// Inline at the cursor, at the drawing's own size.
			type = 'insertAtCursor';
			x = 0;
			y = 0;
			w = dw;
			h = dh;
		} else {
			// Page-anchored where the turtle drew. If the drawing fits the
			// page, keep it 1:1 and shift it inward so it never overhangs; if
			// it is larger than the page, scale it down to fit and centre it,
			// so the whole drawing is always visible.
			type = 'replaceCanvas';
			const pw = page[0], ph = page[1];
			if (dw <= pw && dh <= ph) {
				w = dw;
				h = dh;
				x = Math.max(0, Math.min(dx, pw - dw));
				y = Math.max(0, Math.min(dy, ph - dh));
			} else {
				const s = Math.min(pw / dw, ph / dh);
				w = dw * s;
				h = dh * s;
				x = (pw - w) / 2;
				y = (ph - h) / 2;
			}
		}
		this.enqueue({ type, svg: cropped, x, y, w, h });
		return this.flush();
	}

	removeDrawing() {
		this.enqueue({ type: 'removeCanvas' });
		return this.flush();
	}
}
