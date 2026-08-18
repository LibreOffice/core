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
 * JSDialog.ContentEditable - caret and selection helpers for widgets that render text into a
 *                            contenteditable element.
 *
 * Offsets are plain character counts over the text of one root element, which is what the
 * engine-side text models use. The root is a single paragraph for a multi-paragraph widget, or
 * the whole widget for a single line one.
 */

declare var JSDialog: any;

interface FlatSelection {
	start: number;
	end: number;
}

/// Character count from the start of root up to the given DOM position.
function offsetWithin(root: Element, node: Node, offset: number): number {
	const range = document.createRange();
	range.selectNodeContents(root);
	try {
		range.setEnd(node, offset);
	} catch {
		return 0;
	}
	return range.toString().length;
}

/// The current selection as character offsets, or null when it is outside root.
function getFlatSelection(root: Element): FlatSelection | null {
	const selection = window.getSelection();
	if (!selection || selection.rangeCount === 0) return null;

	const range = selection.getRangeAt(0);
	if (
		!root.contains(range.startContainer) ||
		!root.contains(range.endContainer)
	)
		return null;

	const anchor = offsetWithin(root, range.startContainer, range.startOffset);
	const focus = offsetWithin(root, range.endContainer, range.endOffset);
	return { start: Math.min(anchor, focus), end: Math.max(anchor, focus) };
}

/// The DOM position a character offset lands on. Clamps to the end of the last text node.
function findNodeAtOffset(root: Element, target: number): [Node, number] {
	const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
	let node = walker.nextNode();
	if (!node) return [root, 0];

	let remaining = target;
	for (;;) {
		const length = node.textContent ? node.textContent.length : 0;
		if (remaining <= length) return [node, remaining];
		remaining -= length;
		const next = walker.nextNode();
		if (!next) return [node, length];
		node = next;
	}
}

/// Places the caret or selection at the given character offsets of a single root.
function setFlatCaret(root: Element, start: number, end: number): void {
	const [startNode, startOffset] = findNodeAtOffset(root, start);
	const [endNode, endOffset] = findNodeAtOffset(root, end);
	setCaretRange(startNode, startOffset, endNode, endOffset);
}

/// Places the caret or selection at DOM positions that may sit in different roots.
function setCaretRange(
	startNode: Node,
	startOffset: number,
	endNode: Node,
	endOffset: number,
): void {
	const selection = window.getSelection();
	if (!selection) return;

	const range = document.createRange();
	try {
		range.setStart(startNode, startOffset);
		range.setEnd(endNode, endOffset);
	} catch {
		return;
	}
	selection.removeAllRanges();
	selection.addRange(range);
}

JSDialog.ContentEditable = {
	offsetWithin: offsetWithin,
	getFlatSelection: getFlatSelection,
	findNodeAtOffset: findNodeAtOffset,
	setFlatCaret: setFlatCaret,
	setCaretRange: setCaretRange,
};
