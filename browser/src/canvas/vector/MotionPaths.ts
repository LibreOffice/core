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
	/// One motion path of an object laid onto the slide, and the colour it is shown in.
	export interface PlacedMotionPath {
		polygons: ObjectPathPolygon[];
		colour: string;
	}

	/*
		The motion paths of an object where they lie on the slide. A path comes relative to the
		middle of what it moves, as the slide show plays it, so it goes wherever the object goes:
		a drag moves the middle and the path with it, and turning or sizing the object leaves
		the path as it is, only moved.
	*/
	export class MotionPaths {
		/// The first path is shown as a plain gray; every further one gets a colour of its own
		/// so they can be told apart.
		private static readonly colours: string[] = [
			'#77767b',
			'#e01b24',
			'#1c71d8',
			'#2ec27e',
			'#e66100',
			'#9141ac',
			'#c64600',
			'#0e9fae',
		];

		/// The colour of the path at that place in the order the paths play.
		public static colourOf(index: number): string {
			const colours = MotionPaths.colours;
			return index === 0
				? colours[0]
				: colours[1 + ((index - 1) % (colours.length - 1))];
		}

		/// The paths of that object on the slide, in twips, with the middles of what they move
		/// taken through that drag matrix where one is given.
		public static placedOf(
			object: SlideObject,
			drag?: Matrix2D | null,
		): PlacedMotionPath[] {
			const placed: PlacedMotionPath[] = [];
			const paths = object.motionPaths ?? [];
			paths.forEach((motionPath: ObjectMotionPath, index: number) => {
				const middle = MotionPaths.middleOf(object, motionPath.paragraph);
				if (!middle) return;
				const start = drag ? drag.apply(middle.x, middle.y) : middle;
				placed.push({
					polygons: motionPath.path.map((polygon: ObjectPathPolygon) =>
						MotionPaths.moved(polygon, start),
					),
					colour: MotionPaths.colourOf(index),
				});
			});
			return placed;
		}

		/// The middle of the object's box, or of the box of that paragraph of its text. Nothing
		/// where the object has no box or no such paragraph.
		private static middleOf(
			object: SlideObject,
			paragraph?: number,
		): Point | null {
			if (paragraph !== undefined) {
				const bounds = TextHierarchyParagraphPrimitive.collect(
					object.primitives,
				)[paragraph]?.bounds;
				return bounds
					? new Point((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2)
					: null;
			}
			if (object.x === undefined || object.width === undefined) return null;
			return new Point(
				object.x + object.width / 2,
				(object.y ?? 0) + (object.height ?? 0) / 2,
			);
		}

		private static moved(
			polygon: ObjectPathPolygon,
			by: Point,
		): ObjectPathPolygon {
			const shift = (x: number | undefined, along: number) =>
				x === undefined ? undefined : x + along;
			return {
				closed: polygon.closed,
				points: polygon.points.map((point: ObjectPathPoint) => ({
					x: point.x + by.x,
					y: point.y + by.y,
					behindX: shift(point.behindX, by.x),
					behindY: shift(point.behindY, by.y),
					aheadX: shift(point.aheadX, by.x),
					aheadY: shift(point.aheadY, by.y),
				})),
			};
		}
	}
}
