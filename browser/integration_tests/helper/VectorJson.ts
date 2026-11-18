/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// UNO key codes of the keys that the tests press.
export const Key = {
	RIGHT: 1027,
};

/// One centimeter in twips, the unit of the boxes in an answer.
export const TWIPS_PER_CM = 1440 / 2.54;

/// The entries of an answer, or none when it has no objects.
export function entriesOf(answer: any): any[] {
	return Array.isArray(answer.objects) ? answer.objects : [];
}

/// The first entry whose properties name the given fill color, such as "#4472c4".
export function entryWithFill(answer: any, color: string): any {
	return entriesOf(answer).find(
		(entry) => entry.properties && entry.properties.fillColor === color,
	);
}

/// The first entry of the given kind, such as "page".
export function entryOfKind(answer: any, kind: string): any {
	return entriesOf(answer).find((entry) => entry.kind === kind);
}

/// The first object anywhere under node that matches the predicate, node itself included, in
/// the order the tree holds them.
function findNode(node: any, predicate: (node: any) => boolean): any {
	if (node === null || typeof node !== 'object') return undefined;
	if (predicate(node)) return node;
	for (const key of Object.keys(node)) {
		const found = findNode(node[key], predicate);
		if (found) return found;
	}
	return undefined;
}

/// The first text portion anywhere under node whose text contains text.
export function findTextPortion(node: any, text: string): any {
	return findNode(
		node,
		(candidate) =>
			(candidate.type === 'textSimplePortion' ||
				candidate.type === 'textDecoratedPortion') &&
			typeof candidate.text === 'string' &&
			candidate.text.includes(text),
	);
}

/// The first value of the given key anywhere under node.
export function findValueOf(node: any, key: string): any {
	const holder = findNode(
		node,
		(candidate) =>
			Object.prototype.hasOwnProperty.call(candidate, key) &&
			candidate[key] !== undefined,
	);
	return holder ? holder[key] : undefined;
}

/// The point in the middle of the box of an entry, in twips.
export function centerOf(entry: any): { x: number; y: number } {
	return { x: entry.x + entry.width / 2, y: entry.y + entry.height / 2 };
}
