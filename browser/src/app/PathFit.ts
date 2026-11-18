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
	Fits curves to the places a hand drew, by the method Philip J. Schneider gave in "An Algorithm
	for Automatically Fitting Digitized Curves" (Graphics Gems, 1990): a cubic curve is fitted to
	the places by least squares, and where it strays further than the tolerance from any of them,
	the places are split where it strays most and each half is fitted on its own, running on
	smoothly through the split.

	The drawing is first cut at its corners, where it turns sharper than the corner angle, so a
	corner drawn stays a corner and the curves on both sides of it are fitted apart.

	The answer is the points of a path with their weights, in the unit of the places: every point
	where two curves meet, with the weight behind it and the one ahead.
*/
class PathFit {
	/// How sharp a turn is a corner, in radians.
	private static readonly cornerAngle: number = Math.PI / 3;

	/// How many times the places are given new parameters before a curve is split instead.
	private static readonly mostRounds: number = 4;

	/*
		The curves through those places, straying no further than the tolerance from any of them.
		Places that coincide count once. Where closed is set, the last place is taken to be the
		first again, and the answer does not repeat the first point. Fewer than two places give
		those places as points.
	*/
	public static fit(
		drawn: ReadonlyArray<cool.Point>,
		tolerance: number,
		cornerReach: number,
		closed: boolean,
	): cool.ObjectPathPoint[] {
		const places = PathFit.withoutRepeats(drawn);
		if (closed && places.length > 2) places.push(places[0]);
		if (places.length < 2)
			return places.map((place: cool.Point) => ({ x: place.x, y: place.y }));

		const corners = PathFit.cornersOf(places, cornerReach);
		const pieces = corners.length - 1;
		const starts = corners
			.slice(0, pieces)
			.map((first: number, piece: number) =>
				PathFit.tangentAt(places, first, corners[piece + 1], cornerReach),
			);
		const ends = corners
			.slice(1)
			.map((last: number, piece: number) =>
				PathFit.tangentAt(places, last, corners[piece], cornerReach),
			);

		// a closed path runs through its first point smoothly unless it turns a corner there
		if (closed) {
			const coming = ends[pieces - 1].multiplyBy(-1);
			const leaving = starts[0];
			const turn = Math.abs(
				Math.atan2(coming.cross(leaving), coming.dot(leaving)),
			);
			const through = coming.add(leaving);
			if (turn <= PathFit.cornerAngle && through.length()) {
				starts[0] = through.divideBy(through.length());
				ends[pieces - 1] = starts[0].multiplyBy(-1);
			}
		}

		const points: cool.ObjectPathPoint[] = [{ x: places[0].x, y: places[0].y }];
		for (let piece = 0; piece < pieces; ++piece)
			PathFit.fitPiece(
				places,
				corners[piece],
				corners[piece + 1],
				starts[piece],
				ends[piece],
				tolerance * tolerance,
				points,
			);

		// the last point of a closed path is its first one again, and hands it its weight behind
		if (closed && places.length > 3) {
			const last = points.pop() as cool.ObjectPathPoint;
			if (last.behindX !== undefined) {
				points[0].behindX = last.behindX;
				points[0].behindY = last.behindY;
			}
		}
		return points;
	}

	/// The places with each one that lies on the one before left out.
	private static withoutRepeats(
		drawn: ReadonlyArray<cool.Point>,
	): cool.Point[] {
		const places: cool.Point[] = [];
		for (const place of drawn) {
			const before = places[places.length - 1];
			if (!before || before.x !== place.x || before.y !== place.y)
				places.push(place);
		}
		return places;
	}

	/*
		The indices where the drawing is cut, the first and the last place included: every place
		where the way in and the way out, each looked at over the corner reach, turn by more than
		the corner angle, the sharpest one of a run of such places.
	*/
	private static cornersOf(
		places: ReadonlyArray<cool.Point>,
		reach: number,
	): number[] {
		const turnAt = (index: number): number => {
			const before = PathFit.reachedFrom(places, index, -1, reach);
			const after = PathFit.reachedFrom(places, index, 1, reach);
			if (before === index || after === index) return 0;
			const inward = places[index].subtract(places[before]);
			const outward = places[after].subtract(places[index]);
			const angle =
				Math.atan2(outward.y, outward.x) - Math.atan2(inward.y, inward.x);
			return Math.abs(Math.atan2(Math.sin(angle), Math.cos(angle)));
		};

		const corners = [0];
		let sharpest = -1;
		let sharpestTurn = 0;
		for (let index = 1; index + 1 < places.length; ++index) {
			const turn = turnAt(index);
			if (turn > PathFit.cornerAngle) {
				if (turn > sharpestTurn) {
					sharpest = index;
					sharpestTurn = turn;
				}
			} else if (sharpest >= 0) {
				corners.push(sharpest);
				sharpest = -1;
				sharpestTurn = 0;
			}
		}
		if (sharpest >= 0) corners.push(sharpest);
		corners.push(places.length - 1);
		return corners;
	}

	/// The index of the first place that way from that one which lies the reach away from it, or
	/// the last one there is.
	private static reachedFrom(
		places: ReadonlyArray<cool.Point>,
		index: number,
		way: number,
		reach: number,
	): number {
		let at = index;
		while (
			at + way >= 0 &&
			at + way < places.length &&
			places[at].distanceTo(places[index]) < reach
		)
			at += way;
		return at;
	}

	/*
		The way the drawing leaves that place towards the other end of its piece, or comes into it
		where the other end lies before it, as a vector of length one. It is taken towards the
		first place the reach away, or the other end where that is nearer, so a shake of the hand
		or a short step at the place does not turn it aside.
	*/
	private static tangentAt(
		places: ReadonlyArray<cool.Point>,
		index: number,
		otherEnd: number,
		reach: number,
	): cool.Point {
		const way = otherEnd > index ? 1 : -1;
		let at = index + way;
		while (at !== otherEnd && places[at].distanceTo(places[index]) < reach)
			at += way;
		const vector = places[at].subtract(places[index]);
		const length = vector.length();
		return length ? vector.divideBy(length) : new cool.Point(1, 0);
	}

	/*
		Fits the places from first to last with curves, starting and ending along those tangents,
		and adds the points after the first to the answer: the weight ahead goes to the point that
		is there already, the new point gets its weight behind.
	*/
	private static fitPiece(
		places: ReadonlyArray<cool.Point>,
		first: number,
		last: number,
		startTangent: cool.Point,
		endTangent: cool.Point,
		toleranceSquared: number,
		points: cool.ObjectPathPoint[],
	): void {
		const start = places[first];
		const end = places[last];

		if (last - first === 1) {
			const third = start.distanceTo(end) / 3;
			PathFit.addCurve(points, [
				start,
				start.add(startTangent.multiplyBy(third)),
				end.add(endTangent.multiplyBy(third)),
				end,
			]);
			return;
		}

		let parameters = PathFit.byChordLength(places, first, last);
		let curve = PathFit.curveFor(
			places,
			first,
			last,
			parameters,
			startTangent,
			endTangent,
		);
		let worst = PathFit.worstStray(places, first, last, curve, parameters);
		if (worst.squared < toleranceSquared) {
			PathFit.addCurve(points, curve);
			return;
		}

		// A curve that strays not too far is given better parameters a few times before it is split.
		if (worst.squared < 4 * toleranceSquared) {
			for (let round = 0; round < PathFit.mostRounds; ++round) {
				parameters = PathFit.newParameters(
					places,
					first,
					last,
					parameters,
					curve,
				);
				curve = PathFit.curveFor(
					places,
					first,
					last,
					parameters,
					startTangent,
					endTangent,
				);
				worst = PathFit.worstStray(places, first, last, curve, parameters);
				if (worst.squared < toleranceSquared) {
					PathFit.addCurve(points, curve);
					return;
				}
			}
		}

		// The two halves run on smoothly through the place where the curve strayed most.
		const split = worst.index;
		const through = places[split - 1].subtract(places[split + 1]);
		const length = through.length();
		const centre = length ? through.divideBy(length) : startTangent;
		PathFit.fitPiece(
			places,
			first,
			split,
			startTangent,
			centre,
			toleranceSquared,
			points,
		);
		PathFit.fitPiece(
			places,
			split,
			last,
			centre.multiplyBy(-1),
			endTangent,
			toleranceSquared,
			points,
		);
	}

	/// Adds a curve given by its four control points to the answer.
	private static addCurve(
		points: cool.ObjectPathPoint[],
		curve: cool.Point[],
	): void {
		const from = points[points.length - 1];
		from.aheadX = curve[1].x;
		from.aheadY = curve[1].y;
		points.push({
			x: curve[3].x,
			y: curve[3].y,
			behindX: curve[2].x,
			behindY: curve[2].y,
		});
	}

	/// For every place from first to last, how far along the drawn line it lies, from 0 to 1.
	private static byChordLength(
		places: ReadonlyArray<cool.Point>,
		first: number,
		last: number,
	): number[] {
		const lengths = [0];
		for (let index = first + 1; index <= last; ++index)
			lengths.push(
				lengths[lengths.length - 1] +
					places[index].distanceTo(places[index - 1]),
			);
		const whole = lengths[lengths.length - 1];
		return lengths.map((length: number) => (whole ? length / whole : 0));
	}

	/// The point of a curve given by its four control points at that parameter.
	private static pointOn(curve: cool.Point[], t: number): cool.Point {
		const s = 1 - t;
		return curve[0]
			.multiplyBy(s * s * s)
			.add(curve[1].multiplyBy(3 * s * s * t))
			.add(curve[2].multiplyBy(3 * s * t * t))
			.add(curve[3].multiplyBy(t * t * t));
	}

	/*
		The curve from the first place to the last, leaving and arriving along the tangents, whose
		weights are as long as least squares over the places at those parameters makes them. Where
		that gives a weight that is too short to say anything, both are a third of the distance
		between the ends.
	*/
	private static curveFor(
		places: ReadonlyArray<cool.Point>,
		first: number,
		last: number,
		parameters: number[],
		startTangent: cool.Point,
		endTangent: cool.Point,
	): cool.Point[] {
		const start = places[first];
		const end = places[last];

		let c00 = 0;
		let c01 = 0;
		let c11 = 0;
		let x0 = 0;
		let x1 = 0;
		for (let index = first; index <= last; ++index) {
			const t = parameters[index - first];
			const s = 1 - t;
			const b0 = s * s * s;
			const b1 = 3 * s * s * t;
			const b2 = 3 * s * t * t;
			const b3 = t * t * t;
			const a1 = startTangent.multiplyBy(b1);
			const a2 = endTangent.multiplyBy(b2);
			c00 += a1.dot(a1);
			c01 += a1.dot(a2);
			c11 += a2.dot(a2);
			const rest = places[index].subtract(
				start.multiplyBy(b0 + b1).add(end.multiplyBy(b2 + b3)),
			);
			x0 += a1.dot(rest);
			x1 += a2.dot(rest);
		}

		const determinant = c00 * c11 - c01 * c01;
		let alpha1 = determinant ? (x0 * c11 - x1 * c01) / determinant : 0;
		let alpha2 = determinant ? (c00 * x1 - c01 * x0) / determinant : 0;

		const length = start.distanceTo(end);
		const least = 1e-6 * length;
		if (alpha1 < least || alpha2 < least) {
			alpha1 = length / 3;
			alpha2 = length / 3;
		}
		return [
			start,
			start.add(startTangent.multiplyBy(alpha1)),
			end.add(endTangent.multiplyBy(alpha2)),
			end,
		];
	}

	/// The place that lies furthest from where the curve has it, and the square of how far.
	private static worstStray(
		places: ReadonlyArray<cool.Point>,
		first: number,
		last: number,
		curve: cool.Point[],
		parameters: number[],
	): { index: number; squared: number } {
		let worst = { index: Math.floor((first + last) / 2), squared: 0 };
		for (let index = first + 1; index < last; ++index) {
			const stray = PathFit.pointOn(curve, parameters[index - first]).subtract(
				places[index],
			);
			const squared = stray.x * stray.x + stray.y * stray.y;
			if (squared >= worst.squared) worst = { index: index, squared: squared };
		}
		return worst;
	}

	/// Better parameters: each moved by one step of Newton's method towards the point of the
	/// curve nearest to its place.
	private static newParameters(
		places: ReadonlyArray<cool.Point>,
		first: number,
		last: number,
		parameters: number[],
		curve: cool.Point[],
	): number[] {
		const firstDerivative = [1, 2, 3].map((index: number) =>
			curve[index].subtract(curve[index - 1]).multiplyBy(3),
		);
		const secondDerivative = [1, 2].map((index: number) =>
			firstDerivative[index].subtract(firstDerivative[index - 1]).multiplyBy(2),
		);
		const quadratic = (control: cool.Point[], t: number) =>
			control[0]
				.multiplyBy((1 - t) * (1 - t))
				.add(control[1].multiplyBy(2 * (1 - t) * t))
				.add(control[2].multiplyBy(t * t));
		const linear = (control: cool.Point[], t: number) =>
			control[0].multiplyBy(1 - t).add(control[1].multiplyBy(t));

		return parameters.map((t: number, offset: number) => {
			const place = places[first + offset];
			const off = PathFit.pointOn(curve, t).subtract(place);
			const first1 = quadratic(firstDerivative, t);
			const second = linear(secondDerivative, t);
			const numerator = off.x * first1.x + off.y * first1.y;
			const denominator =
				first1.x * first1.x +
				first1.y * first1.y +
				off.x * second.x +
				off.y * second.y;
			return denominator ? t - numerator / denominator : t;
		});
	}
}
