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

namespace cool {
	/*
		The places a freeform line goes through, in twips on the page, as the mouse draws it. A
		place nearer than the least step to the place kept before it is left out, so a mouse that
		moves slowly does not pile up places on top of each other.

		The line the engine is given is a polygon: it keeps the corners of the drawing and drops
		the places that lie on a nearly straight run between them.
	*/
	export class FreeformStroke {
		private readonly places: cool.Point[] = [];

		/// The least distance from one place kept to the next, in twips.
		private readonly leastStep: number;

		constructor(leastStep: number) {
			this.leastStep = leastStep;
		}

		/// Adds the place the mouse is at, unless it is nearer than the least step to the place
		/// kept last. Answers whether it was kept.
		public add(at: cool.Point): boolean {
			const last = this.places[this.places.length - 1];
			if (last && last.distanceTo(at) < this.leastStep) return false;

			this.places.push(at);
			return true;
		}

		/// The places kept so far, in the order the mouse went through them.
		public placesSoFar(): ReadonlyArray<cool.Point> {
			return this.places;
		}

		/// Whether the places kept so far draw a line, which takes two of them.
		public drawsALine(): boolean {
			return this.places.length >= 2;
		}

		/*
			The places with those left out that lie nearer than the tolerance, in twips, to the
			straight line between the places kept around them. The first and the last place
			always stay. This is the method of Ramer, Douglas and Peucker: a run of places is kept
			as one straight edge where all of them lie near enough to it, and is otherwise split
			at the place that lies furthest away from it, which is kept.
		*/
		public static simplified(
			places: ReadonlyArray<cool.Point>,
			tolerance: number,
		): cool.Point[] {
			if (places.length <= 2) return places.slice();

			const kept: boolean[] = places.map(() => false);
			kept[0] = true;
			kept[places.length - 1] = true;

			// The runs still to look at, each as the index of its first and its last place. A
			// stack keeps a long drawing from going too deep.
			const runs: [number, number][] = [[0, places.length - 1]];
			while (runs.length) {
				const [first, last] = runs.pop() as [number, number];

				let furthest = -1;
				let furthestDistance = tolerance;
				for (let at = first + 1; at < last; ++at) {
					const distance = FreeformStroke.distanceToEdge(
						places[at],
						places[first],
						places[last],
					);
					if (distance > furthestDistance) {
						furthest = at;
						furthestDistance = distance;
					}
				}

				if (furthest < 0) continue;

				kept[furthest] = true;
				runs.push([first, furthest], [furthest, last]);
			}

			return places.filter((_place: cool.Point, at: number) => kept[at]);
		}

		/// How far the place lies from the straight edge between the two ends, in the unit of
		/// the places. An edge whose ends coincide is a point.
		private static distanceToEdge(
			place: cool.Point,
			start: cool.Point,
			end: cool.Point,
		): number {
			const alongX = end.x - start.x;
			const alongY = end.y - start.y;
			const lengthSquared = alongX * alongX + alongY * alongY;
			if (lengthSquared === 0) return place.distanceTo(start);

			// How far along the edge the place falls, from 0 at its start to 1 at its end.
			const along =
				((place.x - start.x) * alongX + (place.y - start.y) * alongY) /
				lengthSquared;
			const share = Math.min(1, Math.max(0, along));
			return place.distanceTo(
				new cool.Point(start.x + share * alongX, start.y + share * alongY),
			);
		}

		/// The message that asks the engine to add an object drawn along the places, in whole
		/// twips: a closed polygon where closed is set, and an open polyline otherwise.
		public static message(
			places: ReadonlyArray<cool.Point>,
			closed: boolean,
		): string {
			const points = places.map(
				(place: cool.Point) =>
					String(Math.round(place.x)) + ',' + String(Math.round(place.y)),
			);
			return (
				'insertpolygonobject closed=' +
				String(closed) +
				' points=' +
				points.join(';')
			);
		}
	}
}
