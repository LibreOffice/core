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
//
// TODO: this uses the legacy com.sun.star UNO API directly. Port each use to
// the cool/cpo facade, or better to the GAS-compatible replacement, once those
// cover what LibreLogo needs. Touchpoints to migrate:
//   - createInstance('com.sun.star.text.TextGraphicObject') (replaceCanvas, insertAtCursor)
//   - css.io.TempFile, css.graphic.GraphicProvider (loadGraphic)
//   - css.beans.PropertyValue / PropertyState (loadGraphic)
//   - css.text.* enums: TextContentAnchorType, HoriOrientation, VertOrientation,
//     RelOrientation, WrapTextMode (replaceCanvas, insertAtCursor)
function kitApply(ops) {
	const PT_TO_MM100 = 2540 / 72;
	function utf8(str) {
		const out = [];
		for (let i = 0; i < str.length; i++) {
			let c = str.charCodeAt(i);
			if (c >= 0xd800 && c <= 0xdbff && i + 1 < str.length) {
				c = 0x10000 + ((c - 0xd800) << 10) + (str.charCodeAt(++i) - 0xdc00);
			}
			if (c < 0x80) out.push(c);
			else if (c < 0x800) out.push(0xc0 | (c >> 6), 0x80 | (c & 0x3f));
			else if (c < 0x10000) out.push(0xe0 | (c >> 12), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f));
			else out.push(0xf0 | (c >> 18), 0x80 | ((c >> 12) & 0x3f), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f));
		}
		// UNO bytes are signed
		return out.map((b) => (b > 127 ? b - 256 : b));
	}
	const css = uno.idl.com.sun.star;
	const ctx = uno.componentContext;
	const model = cool.getActiveUnoModel();
	const CANVAS = 'LibreLogo';
	// Properties typed as short need an explicit Any; a JS number becomes a long.
	const short = (v) => new uno.Any(uno.type.short, v);

	function loadGraphic(svg) {
		const tmp = css.io.TempFile.create(ctx);
		const stream = tmp.getOutputStream();
		stream.writeBytes(utf8(svg));
		stream.closeOutput();
		const provider = css.graphic.GraphicProvider.create(ctx);
		// XTempFile's Uri is a UNO attribute; the binding exposes attributes
		// as properties, older builds as getUri().
		const uri = typeof tmp.getUri === 'function' ? tmp.getUri() : tmp.Uri;
		const arg = new css.beans.PropertyValue({ Name: 'URL', Handle: 0, Value: new uno.Any(uno.type.string, uri), State: css.beans.PropertyState.DIRECT_VALUE });
		return provider.queryGraphic([arg]);
	}

	function removeCanvas() {
		const objects = model.getGraphicObjects();
		if (objects.hasByName(CANVAS)) {
			const old = objects.getByName(CANVAS);
			model.getText().removeTextContent(old);
			return true;
		}
		return false;
	}

	function anchorRange() {
		try {
			return model.getCurrentController().getViewCursor().getStart();
		} catch (e) {
			return model.getText().getStart();
		}
	}

	// The drawing as one page-anchored image on the first page, so LibreLogo
	// coordinates keep meaning "points from the top-left corner of the page".
	function replaceCanvas(op) {
		removeCanvas();
		const graphic = loadGraphic(op.svg);
		const obj = model.createInstance('com.sun.star.text.TextGraphicObject');
		obj.setPropertyValue('Graphic', graphic);
		obj.setName(CANVAS); // XNamed, not a property
		obj.setPropertyValue('AnchorType', css.text.TextContentAnchorType.AT_PAGE);
		obj.setPropertyValue('AnchorPageNo', short(1));
		obj.setPropertyValue('Width', Math.round(op.w * PT_TO_MM100));
		obj.setPropertyValue('Height', Math.round(op.h * PT_TO_MM100));
		obj.setPropertyValue('HoriOrient', short(css.text.HoriOrientation.NONE));
		obj.setPropertyValue('VertOrient', short(css.text.VertOrientation.NONE));
		obj.setPropertyValue('HoriOrientRelation', short(css.text.RelOrientation.PAGE_FRAME));
		obj.setPropertyValue('VertOrientRelation', short(css.text.RelOrientation.PAGE_FRAME));
		obj.setPropertyValue('HoriOrientPosition', Math.round(op.x * PT_TO_MM100));
		obj.setPropertyValue('VertOrientPosition', Math.round(op.y * PT_TO_MM100));
		obj.setPropertyValue('TextWrap', css.text.WrapTextMode.THROUGH);
		obj.setPropertyValue('Opaque', true);
		model.getText().insertTextContent(anchorRange(), obj, false);
	}

	// The drawing as an inline image at the cursor.
	function insertAtCursor(op) {
		const graphic = loadGraphic(op.svg);
		const obj = model.createInstance('com.sun.star.text.TextGraphicObject');
		obj.setPropertyValue('Graphic', graphic);
		obj.setPropertyValue('Width', Math.round(op.w * PT_TO_MM100));
		obj.setPropertyValue('Height', Math.round(op.h * PT_TO_MM100));
		obj.setPropertyValue('AnchorType', css.text.TextContentAnchorType.AS_CHARACTER);
		model.getText().insertTextContent(anchorRange(), obj, false);
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

// Runs in the kit: the page size of the page the cursor is on, in points.
function kitPageSize() {
	const model = cool.getActiveUnoModel();
	let name = 'Standard';
	try {
		name = model.getCurrentController().getViewCursor().getPropertyValue('PageStyleName');
	} catch (e) {
		// no view cursor: use the default page style
	}
	const style = model.getStyleFamilies().getByName('PageStyles').getByName(name);
	const w = style.getPropertyValue('Width'), h = style.getPropertyValue('Height');
	return [w * 72 / 2540, h * 72 / 2540];
}

// Runs in the kit: the language the program is written in, taken from the
// document like desktop LibreLogo does (LibreLogo.py's __setlang__): the view
// cursor's character locale, Western first, then Asian and Complex, skipping
// the "no language" locale zxx. Returns a BCP-47 code (nl-NL, hu-HU, en-GB) or
// null when the document has no usable language, so the caller can fall back.
function kitDocumentLanguage() {
	const model = cool.getActiveUnoModel();
	const code = (loc) => {
		if (!loc || !loc.Language || loc.Language === 'zxx') return null;
		return loc.Country ? loc.Language + '-' + loc.Country : loc.Language;
	};
	try {
		const cursor = model.getCurrentController().getViewCursor();
		for (const prop of ['CharLocale', 'CharLocaleAsian', 'CharLocaleComplex']) {
			try {
				const c = code(cursor.getPropertyValue(prop));
				if (c) return c;
			} catch (e) {
				// property absent on this cursor: try the next
			}
		}
	} catch (e) {
		// no view cursor
	}
	return null;
}

// Runs in the kit: the program text - the selection if there is one, else the body.
function kitProgramText() {
	const doc = cool.getActiveDocument();
	const selection = doc.getSelection().getText();
	if (selection && selection.trim().length > 0) return { text: selection, selection: true };
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
