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
 * Which objects of the leaving slide of a morph become which objects of the entering slide, and
 * which words or characters of their texts become which.
 *
 * Only the objects directly on a slide take part. A group moves with its members.
 *
 * A frame is the transform that maps the unit square onto a piece of text, in slide twips.
 */

declare var SlideShow: any;

/// The objects of two slides, by their ids. A pair holds the leaving object first. An object of
/// a slide that is in no pair is only on its own slide.
interface MorphMatch {
	pairs: [number, number][];
	leavingOnly: number[];
	enteringOnly: number[];
}

/// A word or a character of an object's text: what it reads, the paragraph it is in, counted from
/// 0, and its frame, which maps the unit square onto the box around its glyphs in slide twips.
/// The box spans the advances of its glyphs, so the same word in the same font has a box of the
/// same size wherever it stands. clip is the frame of the part of the drawn text that the piece
/// takes. It reaches past frame into the blanks next to a word.
interface MorphPiece {
	text: string;
	paragraph: number;
	frame: cool.Matrix2D;
	clip: cool.Matrix2D;
}

/// The words or characters of an object's text, and how many paragraphs the text has.
interface MorphText {
	pieces: MorphPiece[];
	paragraphCount: number;
}

/// The pieces of two slides, by their positions in the lists of pieces. A pair holds the
/// leaving piece first.
interface MorphPieceMatch {
	pairs: [number, number][];
	leavingOnly: number[];
	enteringOnly: number[];
}

/// A frame taken apart. The x axis turns by rotation and is scaled by scaleX. The y axis leans
/// over by shear along the x axis and is scaled by scaleY, which is negative for a frame that is
/// mirrored.
interface MorphFrameParts {
	x: number;
	y: number;
	rotation: number;
	scaleX: number;
	scaleY: number;
	shear: number;
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

	/// Take a frame apart into its position, rotation, scales and shear.
	public static partsOf(frame: cool.Matrix2D): MorphFrameParts {
		const scaleX = frame.lengthOfXAxis();
		const rotation = scaleX ? frame.rotation() : 0;
		const cosine = Math.cos(rotation);
		const sine = Math.sin(rotation);
		// The y axis measured along the turned x axis and across it.
		const along = frame.c * cosine + frame.d * sine;
		const scaleY = frame.d * cosine - frame.c * sine;
		return {
			x: frame.e,
			y: frame.f,
			rotation,
			scaleX,
			scaleY,
			shear: scaleY ? along / scaleY : 0,
		};
	}

	/// The frame that the parts make up.
	public static frameOf(parts: MorphFrameParts): cool.Matrix2D {
		const cosine = Math.cos(parts.rotation);
		const sine = Math.sin(parts.rotation);
		const along = parts.shear * parts.scaleY;
		return new cool.Matrix2D(
			parts.scaleX * cosine,
			parts.scaleX * sine,
			along * cosine - parts.scaleY * sine,
			along * sine + parts.scaleY * cosine,
			parts.x,
			parts.y,
		);
	}

	/// The frame at time t between from, at 0, and to, at 1. Each part moves on its own, and the
	/// rotation turns the shorter way round.
	public static interpolate(
		from: cool.Matrix2D,
		to: cool.Matrix2D,
		t: number,
	): cool.Matrix2D {
		const start = MorphMatcher.partsOf(from);
		const end = MorphMatcher.partsOf(to);
		let turn = end.rotation - start.rotation;
		while (turn > Math.PI) turn -= 2 * Math.PI;
		while (turn <= -Math.PI) turn += 2 * Math.PI;
		const between = (one: number, other: number) => one + (other - one) * t;
		return MorphMatcher.frameOf({
			x: between(start.x, end.x),
			y: between(start.y, end.y),
			rotation: start.rotation + turn * t,
			scaleX: between(start.scaleX, end.scaleX),
			scaleY: between(start.scaleY, end.scaleY),
			shear: between(start.shear, end.shear),
		});
	}

	/// The transform that carries what is drawn in the frame source onto the frame target, or
	/// null when source flattens the unit square.
	public static mapping(
		source: cool.Matrix2D,
		target: cool.Matrix2D,
	): cool.Matrix2D | null {
		return source.invert()?.then(target) ?? null;
	}

	// The box of a piece reaches this far above the baseline and below it, in units of the font
	// size, so it holds the accents and the descenders.
	private static readonly pieceAscent = 1.15;
	private static readonly pieceDescent = 0.4;

	/// The words of the object's text, or its characters when characters is true. Undefined for
	/// an object without text, and for a text that a run without glyph advances or a run from
	/// right to left is part of, since such a run has no place for each character. Blanks are
	/// no piece of their own. A bullet is a piece like the text after it.
	public static textPiecesOf(
		object: cool.SlideObject,
		characters: boolean,
	): MorphText | undefined {
		const found = MorphMatcher.findTextBlock(
			object.primitives,
			cool.Matrix2D.IDENTITY,
		);
		if (!found) return undefined;
		const pieces: MorphPiece[] = [];
		let paragraphCount = 0;
		for (const child of found.block.children ?? []) {
			if (child.type !== cool.TextHierarchyParagraphPrimitive.type) continue;
			if (
				!MorphMatcher.collectPieces(
					child,
					found.matrix,
					paragraphCount,
					characters,
					pieces,
				)
			)
				return undefined;
			++paragraphCount;
		}
		return paragraphCount ? { pieces, paragraphCount } : undefined;
	}

	// The block that holds the object's own text, with the transform from its space to the
	// slide. The copy in a shadow, hidden geometry and edit-view content are passed over.
	private static findTextBlock(
		primitives: cool.Primitive[] | undefined,
		matrix: cool.Matrix2D,
	): { block: cool.Primitive; matrix: cool.Matrix2D } | undefined {
		for (const primitive of primitives ?? []) {
			if (primitive.type === cool.TextHierarchyBlockPrimitive.type)
				return { block: primitive, matrix };
			if (
				primitive.type === cool.ShadowPrimitive.type ||
				primitive.type === cool.HiddenGeometryPrimitive.type ||
				primitive.type === cool.ExclusiveEditViewPrimitive.type
			)
				continue;
			const found = MorphMatcher.findTextBlock(
				primitive.children,
				MorphMatcher.within(primitive, matrix),
			);
			if (found) return found;
		}
		return undefined;
	}

	// The transform from the space of the primitive's children to the slide, where matrix is
	// the one from the primitive's own space.
	private static within(
		primitive: cool.Primitive,
		matrix: cool.Matrix2D,
	): cool.Matrix2D {
		if (primitive.type !== cool.TransformPrimitive.type) return matrix;
		const own = cool.Matrix2D.fromArray(
			(primitive as cool.TransformPrimitive).matrix,
		);
		return own ? own.then(matrix) : matrix;
	}

	// Add the pieces of the runs under the primitive to pieces. False when a run has no place
	// for each of its characters.
	private static collectPieces(
		primitive: cool.Primitive,
		matrix: cool.Matrix2D,
		paragraph: number,
		characters: boolean,
		pieces: MorphPiece[],
	): boolean {
		if (
			primitive.type === cool.TextSimplePortionPrimitive.type ||
			primitive.type === cool.TextDecoratedPortionPrimitive.type
		)
			return MorphMatcher.collectRunPieces(
				primitive as cool.TextSimplePortionPrimitive,
				matrix,
				paragraph,
				characters,
				pieces,
			);
		const inner = MorphMatcher.within(primitive, matrix);
		for (const child of primitive.children ?? []) {
			if (
				!MorphMatcher.collectPieces(child, inner, paragraph, characters, pieces)
			)
				return false;
		}
		return true;
	}

	// Add the pieces of one run. The run's matrix maps its glyphs at a font size of 1 onto the
	// space of its parent, and the advances step along its x axis in the parent's units.
	private static collectRunPieces(
		run: cool.TextSimplePortionPrimitive,
		matrix: cool.Matrix2D,
		paragraph: number,
		characters: boolean,
		pieces: MorphPiece[],
	): boolean {
		const whole = run.text ?? '';
		const start = run.textPosition ?? 0;
		const text = whole.substring(
			start,
			start + (run.textLength ?? whole.length - start),
		);
		if (!text) return true;
		const advances = run.advances;
		const own = cool.Matrix2D.fromArray(run.matrix);
		if (run.rtl || !own) return false;
		if (!advances || advances.length < text.length) return false;
		const xLength = own.lengthOfXAxis();
		if (!xLength) return false;

		const distances = [0];
		for (let k = 0; k < text.length; k++)
			distances.push(distances[k] + advances[k]);
		const runFrame = own.then(matrix);
		// The box from left to right along the run.
		const box = (left: number, right: number) =>
			new cool.Matrix2D(
				(right - left) / xLength,
				0,
				0,
				MorphMatcher.pieceAscent + MorphMatcher.pieceDescent,
				left / xLength,
				-MorphMatcher.pieceAscent,
			).then(runFrame);

		for (const [from, to] of MorphMatcher.rangesOf(text, characters)) {
			const left = distances[from];
			const right = distances[to];
			if (!(right > left)) continue;
			// A word takes the drawing halfway into the blank on either side, so the slant of an
			// italic letter at its end stays with it.
			let clipLeft = left;
			let clipRight = right;
			if (!characters) {
				if (from > 0) clipLeft -= (distances[from] - distances[from - 1]) / 2;
				if (to < text.length)
					clipRight += (distances[to + 1] - distances[to]) / 2;
			}
			pieces.push({
				text: text.substring(from, to),
				paragraph,
				frame: box(left, right),
				clip: box(clipLeft, clipRight),
			});
		}
		return true;
	}

	/// The words of the text, or its characters, as [from, to) ranges of UTF-16 units. Blanks
	/// are left out, and a character outside the basic plane is one range with both its units.
	public static rangesOf(
		text: string,
		characters: boolean,
	): [number, number][] {
		const ranges: [number, number][] = [];
		const blank = (k: number) => /\s/.test(text[k]);
		if (characters) {
			for (let k = 0; k < text.length; k++) {
				const code = text.charCodeAt(k);
				const pair = code >= 0xd800 && code <= 0xdbff && k + 1 < text.length;
				const end = pair ? k + 2 : k + 1;
				if (!blank(k)) ranges.push([k, end]);
				k = end - 1;
			}
			return ranges;
		}
		let from = -1;
		for (let k = 0; k <= text.length; k++) {
			const isBlank = k === text.length || blank(k);
			if (isBlank && from >= 0) {
				ranges.push([from, k]);
				from = -1;
			} else if (!isBlank && from < 0) {
				from = k;
			}
		}
		return ranges;
	}

	/// Pair the pieces that read the same. Pieces of different texts pair whatever their order,
	/// so the letters of a word can trade places. The pieces of one text pair in the order of
	/// the text, so equal words keep their order. Where one slide has more of them, the ones
	/// that pair are the ones nearest to each other in that order.
	public static matchPieces(
		leaving: MorphPiece[],
		entering: MorphPiece[],
	): MorphPieceMatch {
		const byText = new Map<string, { leaving: number[]; entering: number[] }>();
		const indicesOf = (text: string) => {
			let indices = byText.get(text);
			if (!indices) {
				indices = { leaving: [], entering: [] };
				byText.set(text, indices);
			}
			return indices;
		};
		leaving.forEach((piece, index) =>
			indicesOf(piece.text).leaving.push(index),
		);
		entering.forEach((piece, index) =>
			indicesOf(piece.text).entering.push(index),
		);

		const leavingTaken = new Set<number>();
		const enteringTaken = new Set<number>();
		const pairs: [number, number][] = [];
		byText.forEach((indices) => {
			for (const [leavingIndex, enteringIndex] of MorphMatcher.pairInOrder(
				indices.leaving.map((index) => leaving[index].frame.apply(0.5, 0.5)),
				indices.entering.map((index) => entering[index].frame.apply(0.5, 0.5)),
			)) {
				pairs.push([
					indices.leaving[leavingIndex],
					indices.entering[enteringIndex],
				]);
				leavingTaken.add(indices.leaving[leavingIndex]);
				enteringTaken.add(indices.entering[enteringIndex]);
			}
		});
		pairs.sort((one, other) => one[0] - other[0]);
		const left = (pieces: MorphPiece[], taken: Set<number>) =>
			pieces.map((_piece, index) => index).filter((index) => !taken.has(index));
		return {
			pairs,
			leavingOnly: left(leaving, leavingTaken),
			enteringOnly: left(entering, enteringTaken),
		};
	}

	// Pair as many of the points of one list with points of the other as the shorter list has,
	// keeping the order of both lists, with the smallest sum of distances. The pairs are
	// positions in the two lists.
	private static pairInOrder(
		one: cool.Point[],
		other: cool.Point[],
	): [number, number][] {
		if (one.length > other.length)
			return MorphMatcher.pairInOrder(other, one).map(
				([otherIndex, oneIndex]) => [oneIndex, otherIndex] as [number, number],
			);
		const distance = (i: number, j: number) => one[i].distanceTo(other[j]);
		// cost[i][j] is the smallest sum for pairing the first i points of one among the first j
		// points of other.
		const width = other.length + 1;
		const cost = new Float64Array((one.length + 1) * width);
		for (let i = 1; i <= one.length; ++i) {
			cost[i * width + i - 1] = Infinity;
			for (let j = i; j <= other.length; ++j) {
				const paired = cost[(i - 1) * width + j - 1] + distance(i - 1, j - 1);
				const skipped = j > i ? cost[i * width + j - 1] : Infinity;
				cost[i * width + j] = Math.min(paired, skipped);
			}
		}

		const pairs: [number, number][] = [];
		let j = other.length;
		for (let i = one.length; i > 0; --i) {
			while (j > i && cost[i * width + j] === cost[i * width + j - 1]) --j;
			pairs.push([i - 1, j - 1]);
			--j;
		}
		return pairs.reverse();
	}
}

SlideShow.MorphMatcher = MorphMatcher;
