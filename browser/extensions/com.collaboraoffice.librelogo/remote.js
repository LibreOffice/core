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

// Runs in the kit (QuickJS with UNO): apply a batch of operations to the
// active Writer document.
remote.kitApply = function kitApply(ops) {
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
		// While a shape, a frame or a positioned image is selected instead of text, the drawing goes
		// at the start of the document:
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
				// For a selected table, and for a selected table cell that starts with a nested
				// table, the table's first cell holds the position:
				let target = element;
				while (target.getType() === DocumentApp.ElementType.TABLE
					|| (target.getType() === DocumentApp.ElementType.TABLE_CELL
						&& target.getChild(0).getType() === DocumentApp.ElementType.TABLE)) {
					const table = target.getType() === DocumentApp.ElementType.TABLE
						? target : target.getChild(0);
					target = table.getRow(0).getCell(0);
				}
				position = doc.newPosition(
					target,
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
};

// Runs in the kit: the page size of the document, in points.
remote.kitPageSize = function kitPageSize() {
	const body = DocumentApp.getActiveDocument().getBody();
	return [body.getPageWidth(), body.getPageHeight()];
};

// Runs in the kit: the language the program is written in, taken from the
// document like desktop LibreLogo does (LibreLogo.py's __setlang__): the
// character locale at the cursor, or at the start of the selection, Western
// first, then Asian and Complex, skipping the "no language" locale zxx, else
// the document's language. Returns a BCP-47 code (nl-NL, hu-HU, ca-ES-valencia),
// a GAS language code (pt_BR, de), or null when the document has no usable
// language, so the caller can fall back.
remote.kitDocumentLanguage = function kitDocumentLanguage() {
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
			// For a selected table, and for a selected table cell that starts with a nested table,
			// the table's first cell holds the position:
			let target = element;
			while (target.getType() === DocumentApp.ElementType.TABLE
				|| (target.getType() === DocumentApp.ElementType.TABLE_CELL
					&& target.getChild(0).getType() === DocumentApp.ElementType.TABLE)) {
				const table = target.getType() === DocumentApp.ElementType.TABLE
					? target : target.getChild(0);
				target = table.getRow(0).getCell(0);
			}
			cursor = doc.newPosition(target, first.isPartial() ? first.getStartOffset() : 0);
		}
	}
	for (const getter of ['getLocale', 'getLocaleAsian', 'getLocaleComplex']) {
		const c = code(cursor[getter]());
		if (c) return c;
	}
	return gasLanguage();
};

// Runs in the kit: the program text - the selection if there is one, else the body.
remote.kitProgramText = function kitProgramText() {
	const doc = DocumentApp.getActiveDocument();
	const selection = doc.getSelection();
	const text = selection ? selection.getText() : '';
	if (text.trim().length > 0) return { text: text, selection: true };
	return { text: doc.getBody().getText(), selection: false };
};

// Runs in the kit, writing the program over the selection, or at the cursor:
remote.kitWriteProgram = function kitWriteProgram(program) {
	const doc = DocumentApp.getActiveDocument();
	const selection = doc.getSelection();
	if (selection) selection.replace(program);
	else doc.getCursor().insertText(program);
};
