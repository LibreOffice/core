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
 * The part of a shape that a transition filter shows at a moment, as an
 * area a 2D canvas can clip to. It follows the masks of the WebGL shape
 * transitions: the same shapes over the unit square, the same direction
 * and mode rules, the same sweep of the progress.
 *
 * Mask coordinates are the unit square of the shape. A transition that
 * scales isotropically spans 0 to 1 along the longer side only, so a
 * circle stays round.
 */

// The extent of a piece across the whole mask. The frame clips it.
const maskLow = -1;
const maskHigh = 2;
// The radius of a slice that covers the whole mask from its center.
const maskReach = 2;

class ShapeTransitionMask {
	private readonly info: TransitionFilterInfo;
	private readonly isClipping: boolean;
	private forwardSweep = true;
	private invertMask = false;
	private scaleIsotropically = false;

	constructor(info: TransitionFilterInfo) {
		this.info = info;
		this.isClipping = ShapeTransitionMask.isClippingType(info);
		if (this.isClipping) this.readTransitionInfo();
	}

	/// True when the filter draws as an area. Any other filter fades.
	public hasArea(): boolean {
		return this.isClipping || this.isCut();
	}

	/// The opacity at the time, 0 to 1, for a filter that fades.
	public fadeAt(time: number): number {
		return this.info.isModeIn ? time : 1 - time;
	}

	/// The part drawn at the time over the box in twips. Null for an empty
	/// box or a filter that fades.
	public clipAt(time: number, box: BoundingBoxType): cool.DrawClip | null {
		if (!this.hasArea() || !box || box.width <= 0 || box.height <= 0)
			return null;

		const { transform, frame } = this.frameFor(box);
		if (this.isCut()) {
			// A cut shows nothing until the end, then all. An exit cut the
			// reverse.
			const ended = time >= 1;
			return {
				transform,
				frame,
				pieces: [],
				inverted: this.info.isModeIn ? ended : !ended,
			};
		}

		const progress = this.forwardSweep ? time : 1 - time;
		return {
			transform,
			frame,
			pieces: this.piecesAt(clampN(progress, 0, 1)),
			inverted: this.invertMask,
		};
	}

	private isCut(): boolean {
		return (
			this.info.transitionType === TransitionType.BARWIPE &&
			this.info.transitionSubtype === TransitionSubType.FADEOVERCOLOR
		);
	}

	private static isClippingType(info: TransitionFilterInfo): boolean {
		switch (info.transitionType) {
			case TransitionType.BARWIPE:
				return info.transitionSubtype !== TransitionSubType.FADEOVERCOLOR;
			case TransitionType.PINWHEELWIPE:
			case TransitionType.RANDOMBARWIPE:
			case TransitionType.CHECKERBOARDWIPE:
			case TransitionType.FOURBOXWIPE:
			case TransitionType.IRISWIPE:
			case TransitionType.ELLIPSEWIPE:
			case TransitionType.FANWIPE:
			case TransitionType.BLINDSWIPE:
			case TransitionType.DISSOLVE:
			case TransitionType.BARNDOORWIPE:
			case TransitionType.WATERFALLWIPE:
				return true;
			default:
				return false;
		}
	}

	// Not in the table: forward, the mask as it is, scaled with the shape.
	private readTransitionInfo(): void {
		const transitionInfo =
			aTransitionInfoTable[this.info.transitionType]?.[
				this.info.transitionSubtype
			];
		if (!transitionInfo) return;

		this.scaleIsotropically = !!transitionInfo.scaleIsotropically;

		// A reverse by rotation or flip is part of the mask where the type
		// has one.
		if (
			!this.info.isDirectionForward &&
			transitionInfo.reverseMethod === TransitionReverseMethod.SubtractAndInvert
		) {
			this.forwardSweep = !this.forwardSweep;
			this.invertMask = !this.invertMask;
		}

		if (!this.info.isModeIn) {
			if (transitionInfo.outInvertsSweep)
				this.forwardSweep = !this.forwardSweep;
			else this.invertMask = !this.invertMask;
		}
	}

	// Mask coordinates onto the box in twips, and the box in mask
	// coordinates.
	private frameFor(box: BoundingBoxType): {
		transform: number[];
		frame: number[];
	} {
		const { x, y, width, height } = box;
		if (!this.scaleIsotropically || width === height)
			return {
				transform: [width, 0, 0, height, x, y],
				frame: [0, 0, 1, 1],
			};

		if (width > height) {
			// Width 0 to 1, height its share of the width, centered.
			const share = height / width;
			return {
				transform: [width, 0, 0, width, x, y + height / 2 - width / 2],
				frame: [0, 0.5 - share / 2, 1, share],
			};
		}
		const share = width / height;
		return {
			transform: [height, 0, 0, height, x + width / 2 - height / 2, y],
			frame: [0.5 - share / 2, 0, share, 1],
		};
	}

	// The pieces the mask covers at the progress.
	private piecesAt(progress: number): cool.AreaPiece[] {
		const subtype = this.info.transitionSubtype;
		const forward = this.info.isDirectionForward;
		switch (this.info.transitionType) {
			case TransitionType.BARWIPE:
				return ShapeTransitionMask.wipe(subtype, forward, progress);
			case TransitionType.PINWHEELWIPE:
				return ShapeTransitionMask.wheel(subtype, progress);
			case TransitionType.RANDOMBARWIPE:
				return ShapeTransitionMask.randomBars(subtype, progress);
			case TransitionType.CHECKERBOARDWIPE:
				return ShapeTransitionMask.checkers(subtype, progress);
			case TransitionType.FOURBOXWIPE:
				return ShapeTransitionMask.fourBoxes(subtype, progress);
			case TransitionType.IRISWIPE:
				return ShapeTransitionMask.iris(subtype, progress);
			case TransitionType.ELLIPSEWIPE:
				return ShapeTransitionMask.ellipse(subtype, progress);
			case TransitionType.FANWIPE:
				return ShapeTransitionMask.fan(progress);
			case TransitionType.BLINDSWIPE:
				return ShapeTransitionMask.blinds(subtype, progress);
			case TransitionType.DISSOLVE:
				return ShapeTransitionMask.dissolve(progress);
			case TransitionType.BARNDOORWIPE:
				return ShapeTransitionMask.barnDoor(subtype, progress);
			case TransitionType.WATERFALLWIPE:
				return ShapeTransitionMask.waterfall(subtype, forward, progress);
		}
		return [];
	}

	// The same pseudo-random value for a cell as the WebGL masks use.
	private static random(x: number, y: number): number {
		const value = Math.sin(x * 12.9898 + y * 78.233) * 43758.5453;
		return value - Math.floor(value);
	}

	// A band across the mask, position to position + size, along x when
	// across.
	private static band(
		across: boolean,
		position: number,
		size: number,
	): cool.AreaPiece {
		return across
			? { rectangle: [position, maskLow, size, maskHigh - maskLow] }
			: { rectangle: [maskLow, position, maskHigh - maskLow, size] };
	}

	// Uncovered from one edge: the top, or the bottom in reverse, for top
	// to bottom. Else the left, or the right in reverse.
	private static wipe(
		subtype: TransitionSubType,
		forward: boolean,
		progress: number,
	): cool.AreaPiece[] {
		const down = subtype === TransitionSubType.TOPTOBOTTOM;
		const fromStart = down
			? forward
			: subtype === TransitionSubType.LEFTTORIGHT && forward;
		return fromStart
			? [ShapeTransitionMask.band(!down, maskLow, progress - maskLow)]
			: [
					ShapeTransitionMask.band(
						!down,
						1 - progress,
						maskHigh - 1 + progress,
					),
				];
	}

	// Blades about the center, each turning clockwise from the top to the
	// next.
	private static wheel(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		let blades = 1;
		if (subtype === TransitionSubType.TWOBLADEVERTICAL) blades = 2;
		else if (subtype === TransitionSubType.THREEBLADE) blades = 3;
		else if (subtype === TransitionSubType.FOURBLADE) blades = 4;
		else if (subtype === TransitionSubType.EIGHTBLADE) blades = 8;

		if (progress <= 0) return [];
		const blade = (2 * Math.PI) / blades;
		const pieces: cool.AreaPiece[] = [];
		for (let index = 0; index < blades; ++index) {
			const start = index * blade - Math.PI / 2;
			pieces.push({
				slice: [0.5, 0.5, maskReach, start, start + blade * progress],
			});
		}
		return pieces;
	}

	// 128 thin bars in random order. Adjacent bars form one piece.
	private static randomBars(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		const across = subtype === TransitionSubType.VERTICAL;
		const count = 128;
		const pieces: cool.AreaPiece[] = [];
		let runStart = -1;
		for (let index = 0; index <= count; ++index) {
			const shown =
				index < count && ShapeTransitionMask.random(index, 0) < progress;
			if (shown && runStart < 0) runStart = index;
			if (!shown && runStart >= 0) {
				pieces.push(
					ShapeTransitionMask.band(
						across,
						runStart / count,
						(index - runStart) / count,
					),
				);
				runStart = -1;
			}
		}
		return pieces;
	}

	// An 8 by 8 board: one color in the first half, the other in the
	// second, across or down.
	private static checkers(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		const across = subtype === TransitionSubType.ACROSS;
		const count = 8;
		const edge = 1 / count;
		const pieces: cool.AreaPiece[] = [];
		for (let row = 0; row < count; ++row) {
			for (let column = 0; column < count; ++column) {
				const first = (row + column) % 2 === 0;
				const filled = clampN(2 * progress - (first ? 0 : 1), 0, 1);
				if (filled <= 0) continue;
				pieces.push({
					rectangle: across
						? [column * edge, row * edge, filled * edge, edge]
						: [column * edge, row * edge, edge, filled * edge],
				});
			}
		}
		return pieces;
	}

	// Four boxes growing from each quarter's middle, or the gap between
	// the corners shrinking.
	private static fourBoxes(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		const pieces: cool.AreaPiece[] = [];
		if (subtype === TransitionSubType.CORNERSIN) {
			const gap = (1.01 * (1 - progress)) / 2;
			const near = 0.5 - gap - maskLow;
			const far = maskHigh - 0.5 - gap;
			for (const [left, width] of [
				[maskLow, near],
				[0.5 + gap, far],
			])
				for (const [top, height] of [
					[maskLow, near],
					[0.5 + gap, far],
				])
					pieces.push({ rectangle: [left, top, width, height] });
			return pieces;
		}

		const inner = 0.25 - progress / 4;
		const size = progress / 2;
		if (size <= 0) return pieces;
		for (const left of [0.5 - inner - size, 0.5 + inner])
			for (const top of [0.5 - inner - size, 0.5 + inner])
				pieces.push({ rectangle: [left, top, size, size] });
		return pieces;
	}

	// A diamond or a box growing from the center.
	private static iris(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		if (progress <= 0) return [];
		if (subtype === TransitionSubType.DIAMOND) {
			const reach = 1.5 * progress;
			return [
				{
					polygon: [
						0.5,
						0.5 - reach,
						0.5 + reach,
						0.5,
						0.5,
						0.5 + reach,
						0.5 - reach,
						0.5,
					],
				},
			];
		}
		const half = 0.75 * progress;
		return [{ rectangle: [0.5 - half, 0.5 - half, 2 * half, 2 * half] }];
	}

	// A circle from the center, or an oval twice as tall for the vertical
	// subtype.
	private static ellipse(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		if (progress <= 0) return [];
		if (subtype === TransitionSubType.VERTICAL) {
			const radius = 1.2 * progress;
			return [{ ellipse: [0.5, 0.5, radius / 2, radius] }];
		}
		const radius = 1.5 * progress;
		return [{ ellipse: [0.5, 0.5, radius, radius] }];
	}

	// A fan opening from the bottom center to both sides.
	private static fan(progress: number): cool.AreaPiece[] {
		if (progress <= 0) return [];
		const opening = Math.PI * progress;
		return [
			{
				slice: [
					0.5,
					0.5,
					maskReach,
					Math.PI / 2 - opening,
					Math.PI / 2 + opening,
				],
			},
		];
	}

	// Six blinds opening together, across for horizontal, else down.
	private static blinds(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		if (progress <= 0) return [];
		const across = subtype !== TransitionSubType.HORIZONTAL;
		const count = 6;
		const pieces: cool.AreaPiece[] = [];
		for (let index = 0; index < count; ++index)
			pieces.push(
				ShapeTransitionMask.band(across, index / count, progress / count),
			);
		return pieces;
	}

	// Squares a sixteenth of the mask wide, in random order. Adjacent
	// squares in a row form one piece.
	private static dissolve(progress: number): cool.AreaPiece[] {
		const count = 16;
		const edge = 1 / count;
		const pieces: cool.AreaPiece[] = [];
		for (let row = 0; row < count; ++row) {
			let runStart: number | null = null;
			for (let column = 0; column <= count; ++column) {
				const shown =
					column < count && progress > ShapeTransitionMask.random(column, row);
				if (shown && runStart === null) runStart = column;
				if (!shown && runStart !== null) {
					pieces.push({
						rectangle: [
							runStart * edge,
							row * edge,
							(column - runStart) * edge,
							edge,
						],
					});
					runStart = null;
				}
			}
		}
		return pieces;
	}

	// A band opening from the middle, across for horizontal, else down.
	private static barnDoor(
		subtype: TransitionSubType,
		progress: number,
	): cool.AreaPiece[] {
		if (progress <= 0) return [];
		const half = 0.75 * progress;
		return [
			ShapeTransitionMask.band(
				subtype !== TransitionSubType.HORIZONTAL,
				0.5 - half,
				2 * half,
			),
		];
	}

	// Ten rows uncovered from one side, each a twentieth of the effect
	// after the one before. Rows start at the top, or the bottom for left or
	// right in reverse. From the right for right forward and left in
	// reverse, else from the left.
	private static waterfall(
		subtype: TransitionSubType,
		forward: boolean,
		progress: number,
	): cool.AreaPiece[] {
		const right = subtype === TransitionSubType.HORIZONTALRIGHT;
		const left = subtype === TransitionSubType.HORIZONTALLEFT;
		const fromBottom = !forward && (right || left);
		const fromRight = right ? forward : left && !forward;
		const count = 10;
		const pieces: cool.AreaPiece[] = [];
		for (let row = 0; row < count; ++row) {
			const step = fromBottom ? 1 - row / count : row / count;
			const reach = 2 * progress - step;
			if (reach <= 0) continue;
			pieces.push({
				rectangle: fromRight
					? [1 - reach, row / count, maskHigh - 1 + reach, 1 / count]
					: [maskLow, row / count, reach - maskLow, 1 / count],
			});
		}
		return pieces;
	}
}
