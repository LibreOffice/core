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
// touches the document runs inside the kit through cool.callRemote; this
// module queues those operations and ships them in batches so a run costs a
// handful of round trips however much it draws.

// Runs in the kit (QuickJS with UNO): apply a batch of operations to the
// active Writer document. Must stay self-contained - only its source is sent.
function kitApply(ops) {
	const PT_TO_PX = 96 / 72;
	const CANVAS = 'LibreLogo';

	function bodyParagraphs(body) {
		const paragraphs = [];
		for (let i = 0; i < body.getNumChildren(); ++i) {
			const child = body.getChild(i);
			const type = child.getType();
			if (type === DocumentApp.ElementType.PARAGRAPH) {
				paragraphs.push(child.asParagraph());
			} else if (type === DocumentApp.ElementType.LIST_ITEM) {
				paragraphs.push(child.asListItem());
			}
		}
		return paragraphs;
	}

	function removeCanvas() {
		const body = DocumentApp.getActiveDocument().getBody();
		if (body.getPositionedImages().some((image) => image.getId() === CANVAS)) {
			return body.removePositionedImage(CANVAS);
		}
		for (const paragraph of bodyParagraphs(body)) {
			if (paragraph.getPositionedImages().some((image) => image.getId() === CANVAS)) {
				return paragraph.removePositionedImage(CANVAS);
			}
		}
		return false;
	}

	// The drawing as one page-anchored image on the first page, so LibreLogo
	// coordinates keep meaning "points from the top-left corner of the page".
	function replaceCanvas(op) {
		removeCanvas();
		const body = DocumentApp.getActiveDocument().getBody();
		bodyParagraphs(body)[0]
			.addPositionedImage(Utilities.newBlob(op.svg, 'image/svg+xml'))
			.setId(CANVAS)
			.setPageAnchor(1)
			.setLayout(DocumentApp.PositionedLayout.ABOVE_TEXT)
			.setWidth(Math.round(op.w * PT_TO_PX))
			.setHeight(Math.round(op.h * PT_TO_PX))
			.setLeftOffset(op.x)
			.setTopOffset(op.y);
	}

	// The drawing as an inline image at the cursor.
	function insertAtCursor(op) {
		const doc = DocumentApp.getActiveDocument();
		let position = doc.getCursor();
		const selection = doc.getSelection();
		// While a shape, a frame, a positioned image or table cells are selected instead of text,
		// the drawing goes at the start of the document:
		if (!position && !selection) {
			const body = doc.getBody();
			let first = body.getChild(0);
			for (let i = 1; first.getType() === DocumentApp.ElementType.TABLE; ++i) {
				first = body.getChild(i);
			}
			position = doc.newPosition(first, 0);
		} else if (!position) {
			const first = selection.getRangeElements()[0];
			const element = first.getElement();
			if (element.getType() === DocumentApp.ElementType.INLINE_IMAGE) {
				const parent = element.getParent();
				position = doc.newPosition(parent, parent.getChildIndex(element));
			} else {
				position = doc.newPosition(
					element,
					first.isPartial() ? first.getStartOffset() : 0,
				);
			}
		}
		position.insertInlineImage(Utilities.newBlob(op.svg, 'image/svg+xml'));
	}

	const results = [];
	for (const op of ops) {
		switch (op.type) {
		case 'replaceCanvas': replaceCanvas(op); results.push(true); break;
		case 'insertAtCursor': insertAtCursor(op); results.push(true); break;
		case 'removeCanvas': results.push(removeCanvas()); break;
		default: results.push(null); break;
		}
	}
	return results;
}

// Runs in the kit: the page size of the document, in points.
function kitPageSize() {
	const body = DocumentApp.getActiveDocument().getBody();
	return [body.getPageWidth(), body.getPageHeight()];
}

// Runs in the kit: the language the program is written in, taken from the
// document like desktop LibreLogo does (LibreLogo.py's __setlang__): the
// character locale at the cursor, or at the start of the selection, Western
// first, then Asian and Complex, skipping the "no language" locale zxx, else
// the document's language. Returns a BCP-47 code (nl-NL, hu-HU, ca-ES-valencia),
// a GAS language code (pt_BR, de), or null when the document has no usable
// language, so the caller can fall back.
function kitDocumentLanguage() {
	const doc = DocumentApp.getActiveDocument();
	const code = (tag) => (tag && tag !== 'zxx' ? tag : null);
	// GAS has the old code iw for Hebrew, and no for Norwegian Bokmal:
	const gasLanguage = () => {
		const language = doc.getLanguage();
		return language === 'iw' ? 'he' : language === 'no' ? 'nb' : language;
	};
	let cursor = doc.getCursor();
	const selection = doc.getSelection();
	if (!cursor && !selection) return gasLanguage();
	if (!cursor) {
		const first = selection.getRangeElements()[0];
		const element = first.getElement();
		if (element.getType() === DocumentApp.ElementType.INLINE_IMAGE) {
			const parent = element.getParent();
			cursor = doc.newPosition(parent, parent.getChildIndex(element));
		} else {
			cursor = doc.newPosition(element, first.isPartial() ? first.getStartOffset() : 0);
		}
	}
	for (const getter of ['getLocale', 'getLocaleAsian', 'getLocaleComplex']) {
		const c = code(cursor[getter]());
		if (c) return c;
	}
	return gasLanguage();
}

// Runs in the kit: the program text - the selection if there is one, else the body.
function kitProgramText() {
	const doc = DocumentApp.getActiveDocument();
	const selection = doc.getSelection();
	const text = selection ? selection.getText() : '';
	if (text.trim().length > 0) return { text: text, selection: true };
	return { text: doc.getBody().getText(), selection: false };
}

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
				const r = await window.cool.callRemote(kitApply, batch);
				if (Array.isArray(r)) results.push(...r);
			}
			return results;
		} finally {
			done();
		}
	}

	async pageSize() {
		return window.cool.callRemote(kitPageSize);
	}

	async programText() {
		return window.cool.callRemote(kitProgramText);
	}

	async documentLanguage() {
		return window.cool.callRemote(kitDocumentLanguage);
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
