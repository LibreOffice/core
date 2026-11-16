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
 * Which objects of the leaving slide of a morph become which objects of the entering slide.
 *
 * Only the objects directly on a slide take part. A group moves with its members.
 */

declare var SlideShow: any;

/// The objects of two slides, by their ids. A pair holds the leaving object first. An object of
/// a slide that is in no pair is only on its own slide.
interface MorphMatch {
	pairs: [number, number][];
	leavingOnly: number[];
	enteringOnly: number[];
}

class MorphMatcher {
	/// The name prefix that pairs two objects by name alone, whatever kind they are.
	public static readonly forcedNamePrefix = '!!';

	/// The objects of the slide that take part in a morph, in paint order: the objects directly
	/// on the slide that are drawn. The page entry, the entries of text edits, empty
	/// placeholders and the objects in hidden are left out.
	public static candidates(
		data: cool.VectorPrimitivesData,
		hidden?: Set<number>,
	): number[] {
		const candidates: number[] = [];
		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object || object.kind !== undefined) continue;
			if (object.parent !== undefined && object.parent !== 0) continue;
			if (object.emptyPlaceholder) continue;
			if (hidden && hidden.has(id)) continue;
			candidates.push(id);
		}
		return candidates;
	}

	/// Pair the objects of the leaving slide with the objects of the entering slide. Each pass
	/// takes the objects it pairs out of the following passes:
	/// 1. Equal names that start with "!!".
	/// 2. Equal names, between objects of the same kind.
	/// 3. The same picture or the same text, between objects of the same kind.
	/// 4. The same kind. The pair with the nearest centers is taken first.
	public static match(
		leaving: cool.VectorPrimitivesData,
		entering: cool.VectorPrimitivesData,
		leavingHidden?: Set<number>,
		enteringHidden?: Set<number>,
	): MorphMatch {
		const leavingIds = MorphMatcher.candidates(leaving, leavingHidden);
		const enteringIds = MorphMatcher.candidates(entering, enteringHidden);
		const pairs: [number, number][] = [];
		const leavingTaken = new Set<number>();
		const enteringTaken = new Set<number>();

		const take = (leavingId: number, enteringId: number) => {
			pairs.push([leavingId, enteringId]);
			leavingTaken.add(leavingId);
			enteringTaken.add(enteringId);
		};

		// Pair the objects whose keys are equal, in paint order. An object without a key takes
		// no part in the pass.
		const pairByKey = (
			keyOf: (object: cool.SlideObject) => string | undefined,
		) => {
			const waiting = new Map<string, number[]>();
			for (const id of enteringIds) {
				if (enteringTaken.has(id)) continue;
				const key = keyOf(MorphMatcher.objectOf(entering, id));
				if (key === undefined) continue;
				const ids = waiting.get(key);
				if (ids) ids.push(id);
				else waiting.set(key, [id]);
			}
			for (const id of leavingIds) {
				if (leavingTaken.has(id)) continue;
				const key = keyOf(MorphMatcher.objectOf(leaving, id));
				if (key === undefined) continue;
				const enteringId = waiting.get(key)?.shift();
				if (enteringId !== undefined) take(id, enteringId);
			}
		};

		pairByKey((object) =>
			object.name && object.name.indexOf(MorphMatcher.forcedNamePrefix) === 0
				? object.name
				: undefined,
		);
		pairByKey((object) =>
			object.name
				? MorphMatcher.kindOf(object) + '\n' + object.name
				: undefined,
		);
		pairByKey((object) => {
			const content = MorphMatcher.contentOf(object);
			return content === undefined
				? undefined
				: MorphMatcher.kindOf(object) + '\n' + content;
		});

		// The remaining objects of the same kind, nearest first.
		const nearby: {
			distance: number;
			leavingId: number;
			enteringId: number;
		}[] = [];
		for (const leavingId of leavingIds) {
			if (leavingTaken.has(leavingId)) continue;
			const leavingObject = MorphMatcher.objectOf(leaving, leavingId);
			const leavingKind = MorphMatcher.kindOf(leavingObject);
			for (const enteringId of enteringIds) {
				if (enteringTaken.has(enteringId)) continue;
				const enteringObject = MorphMatcher.objectOf(entering, enteringId);
				if (MorphMatcher.kindOf(enteringObject) !== leavingKind) continue;
				nearby.push({
					distance: MorphMatcher.centerDistance(leavingObject, enteringObject),
					leavingId,
					enteringId,
				});
			}
		}
		// The sort is stable, so equal distances keep the paint order.
		nearby.sort((one, other) => one.distance - other.distance);
		for (const candidate of nearby) {
			if (leavingTaken.has(candidate.leavingId)) continue;
			if (enteringTaken.has(candidate.enteringId)) continue;
			take(candidate.leavingId, candidate.enteringId);
		}

		return {
			pairs,
			leavingOnly: leavingIds.filter((id) => !leavingTaken.has(id)),
			enteringOnly: enteringIds.filter((id) => !enteringTaken.has(id)),
		};
	}

	// The object of a candidate id, which is always in the slide's objects.
	private static objectOf(
		data: cool.VectorPrimitivesData,
		id: number,
	): cool.SlideObject {
		return data.objects.get(id) as cool.SlideObject;
	}

	/// The kind of the object as a key: its object kind, and for a custom shape its geometry
	/// type, so a rectangle and an ellipse are of different kinds.
	public static kindOf(object: cool.SlideObject): string {
		return (object.objectKind ?? '') + ':' + (object.shapeType ?? '');
	}

	/// The content of the object as a key: the checksum of its picture, or its text with the
	/// paragraphs on lines of their own. Undefined for an object with neither.
	public static contentOf(object: cool.SlideObject): string | undefined {
		if (object.graphicChecksum !== undefined)
			return 'picture ' + object.graphicChecksum;
		const paragraphs = cool.TextHierarchyParagraphPrimitive.collect(
			object.primitives,
		);
		if (!paragraphs.length) return undefined;
		const text = paragraphs
			.map((paragraph) => MorphMatcher.textOf(paragraph))
			.join('\n');
		return text.trim() ? 'text ' + text : undefined;
	}

	// The text the portions under the primitive show. A bullet is not part of the text.
	private static textOf(primitive: cool.Primitive): string {
		if (primitive.type === cool.TextHierarchyBulletPrimitive.type) return '';
		if (
			primitive.type === cool.TextSimplePortionPrimitive.type ||
			primitive.type === cool.TextDecoratedPortionPrimitive.type
		) {
			const portion = primitive as cool.TextSimplePortionPrimitive;
			const text = portion.text ?? '';
			if (portion.textPosition === undefined) return text;
			return text.substring(
				portion.textPosition,
				portion.textPosition + (portion.textLength ?? text.length),
			);
		}
		let text = '';
		for (const child of primitive.children ?? [])
			text += MorphMatcher.textOf(child);
		return text;
	}

	// The distance between the centers of the rectangles the two objects paint.
	private static centerDistance(
		one: cool.SlideObject,
		other: cool.SlideObject,
	): number {
		const distanceX =
			(one.x ?? 0) +
			(one.width ?? 0) / 2 -
			((other.x ?? 0) + (other.width ?? 0) / 2);
		const distanceY =
			(one.y ?? 0) +
			(one.height ?? 0) / 2 -
			((other.y ?? 0) + (other.height ?? 0) / 2);
		return Math.sqrt(distanceX * distanceX + distanceY * distanceY);
	}
}

SlideShow.MorphMatcher = MorphMatcher;
