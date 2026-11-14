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
 * The morph from one slide to the next. An object of the leaving slide and an object of the
 * entering slide with the same name are one pair. Both copies of a pair move, turn and resize
 * together from where the object is on the leaving slide to where it is on the entering one,
 * while the entering copy fades in over the leaving one. An object without a pair fades out, or
 * in.
 */

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

	private readonly leaving: cool.VectorPrimitivesData;
	private readonly entering: cool.VectorPrimitivesData;
	// The place of each pair on the leaving and on the entering slide, by position in pairs.
	private readonly places: [MorphPlace, MorphPlace][] = [];

	constructor(
		leaving: cool.VectorPrimitivesData,
		entering: cool.VectorPrimitivesData,
	) {
		this.leaving = leaving;
		this.entering = entering;
		const leavingObjects = SlideMorph.topObjects(leaving);
		const enteringObjects = SlideMorph.topObjects(entering);

		const enteringByName = new Map<string, number[]>();
		for (const id of enteringObjects) {
			const name = entering.objects.get(id)?.name;
			if (!name) continue;
			const named = enteringByName.get(name);
			if (named) named.push(id);
			else enteringByName.set(name, [id]);
		}

		// Objects that share a name on one slide pair in paint order.
		const partners = new Map<number, number>();
		for (const id of leavingObjects) {
			const name = leaving.objects.get(id)?.name;
			const partner = name ? enteringByName.get(name)?.shift() : undefined;
			const from = SlideMorph._placeOf(leaving, id);
			const to =
				partner !== undefined ? SlideMorph._placeOf(entering, partner) : null;
			if (partner === undefined || !from || !to) {
				this.leavingAlone.push(id);
				continue;
			}
			this.pairs.push({ leaving: id, entering: partner });
			this.places.push([from, to]);
			partners.set(partner, id);
		}
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
	}

	/// True when the morph goes from the leaving tree to the entering tree.
	public isBetween(
		leaving: cool.VectorPrimitivesData,
		entering: cool.VectorPrimitivesData,
	): boolean {
		return this.leaving === leaving && this.entering === entering;
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
	/// at time, from 0 at the start of the morph to 1 at its end.
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
		this.pairs.forEach((pair, index) => {
			const [from, to] = this.places[index];
			const now = SlideMorph._placeAt(from, to, time);
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
		let target = to;
		if (from.height * to.height < 0) {
			const flipped: MorphPlace = {
				...to,
				width: -to.width,
				height: -to.height,
				angle: to.angle + Math.PI,
			};
			if (
				Math.abs(SlideMorph._turn(from, flipped)) <
				Math.abs(SlideMorph._turn(from, to))
			)
				target = flipped;
		}
		return {
			centerX: SlideMorph._between(from.centerX, target.centerX, time),
			centerY: SlideMorph._between(from.centerY, target.centerY, time),
			width: SlideMorph._between(from.width, target.width, time),
			height: SlideMorph._between(from.height, target.height, time),
			angle: from.angle + SlideMorph._turn(from, target) * time,
		};
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
