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

/*
 * The morph from one slide to the next. MorphMatcher pairs the objects of the leaving slide with
 * the objects of the entering slide. Both copies of a pair move, turn and resize together from
 * where the object is on the leaving slide to where it is on the entering one, while the entering
 * copy fades in over the leaving one. An object without a pair fades out, or in.
 *
 * The text of a pair keeps its size while the object is resized. Each word has a place in the text
 * of its object, which moves and turns with the object at the size of the text, and goes to the
 * place of the same word in the text of the other copy. A word that the other copy does not have
 * keeps its place in the text and fades out or in.
 *
 * A morph by word or by character pairs no objects. The objects stay where they are and fade out
 * or in without their text, while a word or a character that reads the same on both slides moves
 * from its place on the leaving slide to its place on the entering one, also from one object into
 * another. A piece on one slide only fades out or in where it is.
 */

/// A piece of the text of an object on one slide of a morph, and the key that its layer takes its
/// state by.
interface MorphSlidePiece {
	key: number;
	owner: number;
	piece: MorphPiece;
}

/// A piece of the text of a paired object, which moves with its object at its own size. pair is
/// the position of the pair of its object. from and to are the frames of the piece in the space
/// of the anchor of its object, at the start and at the end of the morph. paired is false for a
/// piece that the other copy of the object does not have.
interface MorphAnchoredPiece {
	entry: MorphSlidePiece;
	pair: number;
	from: cool.Matrix2D;
	to: cool.Matrix2D;
	paired: boolean;
}

/// The pieces of the texts of the two slides of a morph. splitsText is true for a morph by word
/// or by character, which moves the pieces across the slides and no objects.
interface MorphTextPieces {
	leaving: MorphSlidePiece[];
	entering: MorphSlidePiece[];
	splitsText: boolean;
}

/// The object ids of a leaving object and of the entering object it becomes.
interface MorphPair {
	leaving: number;
	entering: number;
}

/// One object to draw in a frame of the morph: from the leaving slide or from the entering one.
interface MorphStep {
	leaving: boolean;
	id: number;
}

/// Where an object is: the middle of its box, its size and the angle it is turned by, in
/// twips and radians. The height is negative for an object that is mirrored.
interface MorphPlace {
	centerX: number;
	centerY: number;
	width: number;
	height: number;
	angle: number;
}

class SlideMorph {
	public readonly pairs: MorphPair[] = [];
	/// The objects of each slide that have no pair, by object id.
	public readonly leavingAlone: number[] = [];
	public readonly enteringAlone: number[] = [];
	/// The objects of both slides in the order to draw them: the entering slide's paint order,
	/// with the leaving copy of a pair right under its entering copy, and each object of the
	/// leaving slide alone under the next leaving copy that follows it there.
	public readonly paintOrder: MorphStep[] = [];
	/// The objects of each slide that are drawn without their text, while their pieces draw it.
	public readonly leavingSplit = new Set<number>();
	public readonly enteringSplit = new Set<number>();

	private readonly leaving: cool.VectorPrimitivesData;
	private readonly entering: cool.VectorPrimitivesData;
	private readonly text: MorphTextPieces | undefined;
	// The place of each pair on the leaving and on the entering slide, by position in pairs.
	private readonly places: [MorphPlace, MorphPlace][] = [];
	// The pieces that read the same on both slides, and the pieces on one slide only.
	private readonly piecePairs: [MorphSlidePiece, MorphSlidePiece][] = [];
	private readonly leavingLonePieces: MorphSlidePiece[] = [];
	private readonly enteringLonePieces: MorphSlidePiece[] = [];
	// The pieces of the text of the paired objects.
	private readonly anchoredPieces: [
		MorphAnchoredPiece[],
		MorphAnchoredPiece[],
	] = [[], []];

	/// leavingHidden and enteringHidden hold the objects that an animation holds hidden on
	/// either slide, which take no part in the morph. text holds the pieces of the texts of the
	/// objects of both slides.
	constructor(
		leaving: cool.VectorPrimitivesData,
		entering: cool.VectorPrimitivesData,
		leavingHidden?: Set<number>,
		enteringHidden?: Set<number>,
		text?: MorphTextPieces,
	) {
		this.leaving = leaving;
		this.entering = entering;
		this.text = text;
		const leavingObjects = MorphMatcher.candidates(leaving, leavingHidden);
		const enteringObjects = MorphMatcher.candidates(entering, enteringHidden);
		const match: MorphMatch = text?.splitsText
			? { pairs: [], leavingOnly: [], enteringOnly: [] }
			: MorphMatcher.match(leaving, entering, leavingHidden, enteringHidden);
		if (text?.splitsText)
			this._matchPieces(text, leavingObjects, enteringObjects);

		// The pairs in the paint order of the leaving slide.
		const partners = new Map<number, number>();
		const pairedLeaving = new Set<number>();
		for (const [id, partner] of match.pairs
			.slice()
			.sort(
				(one, other) =>
					leavingObjects.indexOf(one[0]) - leavingObjects.indexOf(other[0]),
			)) {
			const from = SlideMorph._placeOf(leaving, id);
			const to = SlideMorph._placeOf(entering, partner);
			if (!from || !to) continue;
			this.pairs.push({ leaving: id, entering: partner });
			this.places.push([from, to]);
			partners.set(partner, id);
			pairedLeaving.add(id);
		}
		for (const id of leavingObjects)
			if (!pairedLeaving.has(id)) this.leavingAlone.push(id);
		for (const id of enteringObjects)
			if (!partners.has(id)) this.enteringAlone.push(id);

		const leavingAlone = new Set(this.leavingAlone);
		let next = 0;
		for (const id of enteringObjects) {
			const partner = partners.get(id);
			if (partner !== undefined) {
				const at = leavingObjects.indexOf(partner);
				for (; next < at; ++next)
					if (leavingAlone.has(leavingObjects[next]))
						this.paintOrder.push({ leaving: true, id: leavingObjects[next] });
				next = Math.max(next, at + 1);
				this.paintOrder.push({ leaving: true, id: partner });
			}
			this.paintOrder.push({ leaving: false, id });
		}
		for (; next < leavingObjects.length; ++next)
			if (leavingAlone.has(leavingObjects[next]))
				this.paintOrder.push({ leaving: true, id: leavingObjects[next] });

		if (text && !text.splitsText) this._anchorPieces(text);
	}

	// Pair the pieces of each pair of objects with each other only, so a word stays in its
	// object, and place each piece in the space of the anchor of its object. The pieces of the
	// entering copy are placed against the anchor that the move ends at.
	private _anchorPieces(text: MorphTextPieces): void {
		const piecesOf = (pieces: MorphSlidePiece[], owner: number) =>
			pieces.filter((entry) => entry.owner === owner);
		const [leavingAnchored, enteringAnchored] = this.anchoredPieces;
		this.pairs.forEach((pair, index) => {
			const leavingPieces = piecesOf(text.leaving, pair.leaving);
			const enteringPieces = piecesOf(text.entering, pair.entering);
			if (leavingPieces.length) this.leavingSplit.add(pair.leaving);
			if (enteringPieces.length) this.enteringSplit.add(pair.entering);
			const [from, to] = this.places[index];
			const start = SlideMorph._anchorOf(from);
			const end = SlideMorph._anchorOf(SlideMorph._targetOf(from, to));
			const place = (entry: MorphSlidePiece, anchor: cool.Matrix2D) =>
				entry.piece.frame.then(anchor.invert() ?? cool.Matrix2D.IDENTITY);

			const match = MorphMatcher.matchPieces(
				leavingPieces.map((entry) => entry.piece),
				enteringPieces.map((entry) => entry.piece),
			);
			for (const [leavingIndex, enteringIndex] of match.pairs) {
				const leavingEntry = leavingPieces[leavingIndex];
				const enteringEntry = enteringPieces[enteringIndex];
				const pieceFrom = place(leavingEntry, start);
				const pieceTo = place(enteringEntry, end);
				leavingAnchored.push({
					entry: leavingEntry,
					pair: index,
					from: pieceFrom,
					to: pieceTo,
					paired: true,
				});
				enteringAnchored.push({
					entry: enteringEntry,
					pair: index,
					from: pieceFrom,
					to: pieceTo,
					paired: true,
				});
			}
			for (const at of match.leavingOnly) {
				const entry = leavingPieces[at];
				const own = place(entry, start);
				leavingAnchored.push({
					entry,
					pair: index,
					from: own,
					to: own,
					paired: false,
				});
			}
			for (const at of match.enteringOnly) {
				const entry = enteringPieces[at];
				const own = place(entry, end);
				enteringAnchored.push({
					entry,
					pair: index,
					from: own,
					to: own,
					paired: false,
				});
			}
		});
	}

	/// True when the morph goes from the leaving tree to the entering tree, with the pieces of
	/// text given.
	public isBetween(
		leaving: cool.VectorPrimitivesData,
		entering: cool.VectorPrimitivesData,
		text?: MorphTextPieces,
	): boolean {
		return (
			this.leaving === leaving &&
			this.entering === entering &&
			this.text?.leaving === text?.leaving &&
			this.text?.entering === text?.entering &&
			this.text?.splitsText === text?.splitsText
		);
	}

	// Pair the pieces of the objects that take part across the two slides. Every such object
	// that has pieces is drawn without its text.
	private _matchPieces(
		text: MorphTextPieces,
		leavingObjects: number[],
		enteringObjects: number[],
	): void {
		const takingPart = (pieces: MorphSlidePiece[], objects: number[]) => {
			const ids = new Set(objects);
			return pieces.filter((entry) => ids.has(entry.owner));
		};
		const leavingPieces = takingPart(text.leaving, leavingObjects);
		const enteringPieces = takingPart(text.entering, enteringObjects);
		for (const entry of leavingPieces) this.leavingSplit.add(entry.owner);
		for (const entry of enteringPieces) this.enteringSplit.add(entry.owner);

		const match = MorphMatcher.matchPieces(
			leavingPieces.map((entry) => entry.piece),
			enteringPieces.map((entry) => entry.piece),
		);
		for (const [leavingIndex, enteringIndex] of match.pairs)
			this.piecePairs.push([
				leavingPieces[leavingIndex],
				enteringPieces[enteringIndex],
			]);
		for (const index of match.leavingOnly)
			this.leavingLonePieces.push(leavingPieces[index]);
		for (const index of match.enteringOnly)
			this.enteringLonePieces.push(enteringPieces[index]);
	}

	/// The ids of the objects directly on the slide, in paint order. The members of a group
	/// move with it, and the page is the background.
	public static topObjects(data: cool.VectorPrimitivesData): number[] {
		return data.order.filter((id) => {
			const object = data.objects.get(id);
			return object && object.parent === 0 && object.kind === undefined;
		});
	}

	/// The draw state of each object of the leaving and of the entering slide, by object id,
	/// and of each piece of text, by its key, at time, from 0 at the start of the morph to 1 at
	/// its end.
	public states(
		time: number,
	): [
		Map<number, cool.ObjectRenderState>,
		Map<number, cool.ObjectRenderState>,
	] {
		const leavingStates = new Map<number, cool.ObjectRenderState>();
		const enteringStates = new Map<number, cool.ObjectRenderState>();

		// The two copies of a pair cover each other fully in the middle of the morph.
		const leavingAlpha = Math.min(1, 2 * (1 - time));
		const enteringAlpha = Math.min(1, 2 * time);
		const placesNow: MorphPlace[] = [];
		this.pairs.forEach((pair, index) => {
			const [from, to] = this.places[index];
			const now = SlideMorph._placeAt(from, to, time);
			placesNow.push(now);
			leavingStates.set(pair.leaving, {
				transform: SlideMorph._moving(from, now),
				alpha: leavingAlpha,
			});
			enteringStates.set(pair.entering, {
				transform: SlideMorph._moving(to, now),
				alpha: enteringAlpha,
			});
		});
		for (const id of this.leavingAlone)
			leavingStates.set(id, { alpha: 1 - time });
		for (const id of this.enteringAlone)
			enteringStates.set(id, { alpha: time });

		// A pair of pieces is drawn at the same frame on both slides.
		for (const [leavingPiece, enteringPiece] of this.piecePairs) {
			const from = leavingPiece.piece.frame;
			const to = enteringPiece.piece.frame;
			const now = MorphMatcher.interpolate(from, to, time);
			leavingStates.set(leavingPiece.key, {
				transform: MorphMatcher.mapping(from, now)?.toArray(),
				alpha: leavingAlpha,
			});
			enteringStates.set(enteringPiece.key, {
				transform: MorphMatcher.mapping(to, now)?.toArray(),
				alpha: enteringAlpha,
			});
		}
		// A piece on one slide only is gone by the middle of the morph on the leaving slide and
		// shows from the middle on the entering one, so the copy of a word that moves is the
		// only one seen in the middle.
		for (const entry of this.leavingLonePieces)
			leavingStates.set(entry.key, { alpha: Math.max(0, 1 - 2 * time) });
		for (const entry of this.enteringLonePieces)
			enteringStates.set(entry.key, { alpha: Math.max(0, 2 * time - 1) });

		// A piece of a paired object is drawn in its place in the text, in the anchor of the
		// object where it is now.
		const [leavingAnchored, enteringAnchored] = this.anchoredPieces;
		const anchored = (
			pieces: MorphAnchoredPiece[],
			states: Map<number, cool.ObjectRenderState>,
			pairedAlpha: number,
			loneAlpha: number,
		) => {
			for (const piece of pieces) {
				const frame = MorphMatcher.interpolate(piece.from, piece.to, time).then(
					SlideMorph._anchorOf(placesNow[piece.pair]),
				);
				states.set(piece.entry.key, {
					transform: MorphMatcher.mapping(
						piece.entry.piece.frame,
						frame,
					)?.toArray(),
					alpha: piece.paired ? pairedAlpha : loneAlpha,
				});
			}
		};
		anchored(
			leavingAnchored,
			leavingStates,
			leavingAlpha,
			Math.max(0, 1 - 2 * time),
		);
		anchored(
			enteringAnchored,
			enteringStates,
			enteringAlpha,
			Math.max(0, 2 * time - 1),
		);
		return [leavingStates, enteringStates];
	}

	// The place of the object from the mapping of the unit square onto it, or from its box
	// where it has no mapping. Null for an object with neither.
	private static _placeOf(
		data: cool.VectorPrimitivesData,
		id: number,
	): MorphPlace | null {
		const object = data.objects.get(id);
		if (!object) return null;
		let matrix = cool.Matrix2D.fromArray(object.transform);
		if (
			!matrix &&
			object.x !== undefined &&
			object.y !== undefined &&
			object.width !== undefined &&
			object.height !== undefined
		)
			matrix = new cool.Matrix2D(
				object.width,
				0,
				0,
				object.height,
				object.x,
				object.y,
			);
		if (!matrix) return null;
		const center = matrix.apply(0.5, 0.5);
		const isMirrored = matrix.a * matrix.d - matrix.b * matrix.c < 0;
		return {
			centerX: center.x,
			centerY: center.y,
			width: matrix.lengthOfXAxis(),
			height: isMirrored ? -matrix.lengthOfYAxis() : matrix.lengthOfYAxis(),
			angle: matrix.rotation(),
		};
	}

	// The place a share of time of the way from one place to the other. The angle turns the
	// shorter way round. A place that is mirrored on one side only is also that place turned
	// half round and mirrored along its other axis, and of the two the one that turns least is
	// taken, so the object flips over without spinning.
	private static _placeAt(
		from: MorphPlace,
		to: MorphPlace,
		time: number,
	): MorphPlace {
		const target = SlideMorph._targetOf(from, to);
		return {
			centerX: SlideMorph._between(from.centerX, target.centerX, time),
			centerY: SlideMorph._between(from.centerY, target.centerY, time),
			width: SlideMorph._between(from.width, target.width, time),
			height: SlideMorph._between(from.height, target.height, time),
			angle: from.angle + SlideMorph._turn(from, target) * time,
		};
	}

	// The place that the move from one place to the other ends at: the other place, or that
	// place turned half round and mirrored along its other axis where that turns less.
	private static _targetOf(from: MorphPlace, to: MorphPlace): MorphPlace {
		if (from.height * to.height >= 0) return to;
		const flipped: MorphPlace = {
			...to,
			width: -to.width,
			height: -to.height,
			angle: to.angle + Math.PI,
		};
		return Math.abs(SlideMorph._turn(from, flipped)) <
			Math.abs(SlideMorph._turn(from, to))
			? flipped
			: to;
	}

	// The frame that has its origin at the middle of the place and is turned like it, at a scale
	// of 1. A piece of text placed in it keeps its size and turns with the place.
	private static _anchorOf(place: MorphPlace): cool.Matrix2D {
		return cool.Matrix2D.IDENTITY.rotateAround(0, 0, place.angle).translate(
			place.centerX,
			place.centerY,
		);
	}

	// The angle from one place to the other, the shorter way round.
	private static _turn(from: MorphPlace, to: MorphPlace): number {
		let turn = (to.angle - from.angle) % (2 * Math.PI);
		if (turn > Math.PI) turn -= 2 * Math.PI;
		else if (turn < -Math.PI) turn += 2 * Math.PI;
		return turn;
	}

	private static _between(from: number, to: number, time: number): number {
		return from + (to - from) * time;
	}

	// The transform that takes an object drawn at one place to the other place, as the six
	// canvas matrix values. A place without width or height keeps its size along that side.
	private static _moving(from: MorphPlace, to: MorphPlace): number[] {
		const scaleX = from.width !== 0 ? to.width / from.width : 1;
		const scaleY = from.height !== 0 ? to.height / from.height : 1;
		return cool.Matrix2D.IDENTITY.translate(-from.centerX, -from.centerY)
			.rotateAround(0, 0, -from.angle)
			.scale(scaleX, scaleY)
			.rotateAround(0, 0, to.angle)
			.translate(to.centerX, to.centerY)
			.toArray();
	}
}
