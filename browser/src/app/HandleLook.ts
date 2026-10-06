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

/// The kinds of handle a selection shows, each drawn its own way.
type HandleKind =
	| 'framing'
	| 'turning'
	| 'shaping'
	| 'tiePoint'
	| 'pathPoint'
	| 'weight'
	| 'crop';

/*
	The kinds of band that can be drawn round a selection. A selection takes one where it is a
	single thing holding others, so that what is selected can be told apart from one object.
*/
type SurroundKind = 'group' | 'diagram';

/*
	How the handles of a selection look: how large each kind is, what shape it has and what color
	it takes. One place, so that changing any of it is changing one number rather than hunting
	through the groups that draw them.

	There is one size for all of them, and each kind takes a share of it. A share of one is that
	size; a point of a path is two thirds of it. Changing the size alone keeps every kind in the
	same relation to the others.
*/
class HandleLook {
	/// How wide a handle of a share of one is, before the screen's own scale is taken in.
	public static size: number = 12;

	/// What share of that size each kind takes.
	public static readonly shareOfTheSize: { [kind in HandleKind]: number } = {
		framing: 1,
		turning: 1,
		shaping: 1,
		tiePoint: 5 / 6,
		pathPoint: 2 / 3,
		weight: 1 / 2,
		crop: 1,
	};

	/** Whether each kind is drawn as a square or as a circle.

	 * A point to tie a connector to is a circle here and draws more of its own besides: the
	 * noses that say which ways out it offers, and the marks that say which side of the object
	 * its place is measured from. Those depend on what the point holds, so the group that knows
	 * it draws them and takes only the size and the colors from here.
	 */
	public static readonly shapeOfTheKind: {
		[kind in HandleKind]: 'square' | 'circle';
	} = {
		framing: 'square',
		turning: 'circle',
		shaping: 'circle',
		tiePoint: 'circle',
		pathPoint: 'square',
		weight: 'circle',
		/*
			A crop mark is neither: an L at a corner of the picture and a bar along a side, each
			a handle long and half a handle thick, laid in the picture's own axes by the group
			that draws them. The size and the color come from here.
		*/
		crop: 'square',
	};

	/// What each kind is filled with.
	public static readonly colourOfTheKind: { [kind in HandleKind]: string } = {
		framing: '#FFFFFF',
		turning: '#FFFFFF',
		shaping: '#FFFF00',
		/*
			A point to tie a connector to is drawn in a dimmed red: it is offered while something
			else is being done and should not shout over the handles beside it.
		*/
		tiePoint: '#B03A3A',
		pathPoint: '#1C99E0',
		weight: '#1C99E0',
		// A crop mark is drawn in the handle color darkened by cropMarkShade. This is what it
		// falls back on where that cannot be worked out.
		crop: '#FFFFFF',
	};

	/*
		The band drawn round a selection that holds other objects stands off from what it goes
		round, and has a width of its own. Both are shares of the same size the handles take, so
		making the handles larger moves the band out and widens it in step. Every kind of band is
		the same size, and only the color tells one from another.
	*/
	public static shareOfTheSizeBeforeASurround: number = 2 / 5;
	public static shareOfTheSizeOfASurround: number = 2 / 5;

	/*
		One color for each kind of band. The lines that bound it and the gradient it is filled
		with are all worked out from that one color, lighter on one side and darker on the
		other, so a band is changed by changing a single value.
	*/
	public static colourOfTheSurround: {
		[kind in SurroundKind]: string;
	} = {
		group: '#CFE0F4',
		diagram: '#F0F0F0',
	};

	/// How far the lighter and the darker of that color stand from it, as a share of the whole
	/// range a color has.
	public static surroundBlend: number = 0.1;

	/// Whether a group that is not a diagram is drawn with a band round it.
	public static bandRoundAGroup: boolean = true;

	/*
		That color moved towards white by that share of the whole range, or towards black where
		the share is negative.
	*/
	public static blended(color: string, by: number): string {
		const part = (at: number) => parseInt(color.substr(at, 2), 16);
		const moved = (value: number) =>
			Math.round(Math.min(255, Math.max(0, value + by * 255)))
				.toString(16)
				.padStart(2, '0');

		return '#' + moved(part(1)) + moved(part(3)) + moved(part(5));
	}

	/*
		How far the eight that frame a selection are drawn outside the places they stand on, as a
		share of half a handle. At one, a corner handle has moved by half of itself along each
		axis, so the corner of it that faces the selection lands on the place it stands on and
		the handle lies wholly outside the frame; at nought it is drawn centered on that place, as
		it always was; at minus one it lies wholly inside. Anything beyond either end would leave
		the frame behind altogether, so the share is held between the two.

		It is what keeps the eight off the points of a path, which lie on the corners of the
		frame and halfway along its sides. Only the drawing and the hit test know of it: a drag
		still puts the edge the handle names where the mouse is.
	*/
	public static shareTheEightMoveOut: number = 0.5;

	/// Whether the eight are drawn turned with the selection, so that a square handle stands at
	/// the object's own angle rather than upright. Only the turn: a sheared object still gets
	/// square handles.
	public static turnTheEightWithTheObject: boolean = true;

	/// How much darker than the handle color a crop mark is drawn, as a share of the whole
	/// range a color has, so that the marks and the eight can be told apart at a glance.
	public static cropMarkShade: number = 0.12;

	/// The line every handle is drawn round with.
	public static readonly outlineColour: string = 'black';

	/// A point to tie to that keeps a distance of its own, and one of the four an object falls
	/// back on, which takes the same red darker again so the given ones stand out from it.
	public static readonly tiePointKeepingItsDistance: string = '#CC6666';
	public static readonly tiePointFallenBackOn: string = '#6A2323';

	/// A handle that is shown and does nothing.
	public static readonly deactivatedColour: string = '#E0E0E0';
	public static readonly deactivatedOutlineColour: string = '#909090';

	/// The line a drag of a point leaves behind, and the one a path is drawn along while it is
	/// edited.
	public static readonly outlineOfADrag: string = '#1C99E0';

	/// The line that ties a weight of a curve to its point, drawn in two colors one over the
	/// other with a dash, so it shows on a light background and on a dark one alike.
	public static readonly helperLineColour: string = '#1C99E0';
	public static readonly helperLineDarkColour: string = '#0B3F63';
	public static readonly helperLineAlpha: number = 0.7;
	/// The length of a dash of that line, in CSS pixels.
	public static readonly helperLineDash: number = 4;

	/// How wide a handle of that kind is drawn, in core pixels.
	public static widthOf(kind: HandleKind): number {
		return HandleLook.size * HandleLook.shareOfTheSize[kind] * app.dpiScale;
	}

	/// How far the eight are drawn outside the places they stand on, in core pixels.
	public static distanceTheEightMoveOut(): number {
		const share = Math.max(-1, Math.min(1, HandleLook.shareTheEightMoveOut));

		return share * HandleLook.widthOf('framing') * 0.5;
	}

	/*
		Where a handle is drawn, in twips, which is where it stands unless it is one of the eight
		and those are drawn outside the frame.
	*/
	public static drawnAt(handle: SelectionHandle): cool.SimplePoint {
		const away = handle.awayFromItsPlace;
		const distance = away
			? (handle.awayBy ?? HandleLook.distanceTheEightMoveOut()) *
				app.pixelsToTwips
			: 0;

		return new cool.SimplePoint(
			handle.point.x + (away ? away.x * distance : 0),
			handle.point.y + (away ? away.y * distance : 0),
		);
	}

	/// How far a band stands off from what it goes round, in core pixels.
	public static distanceBeforeASurround(): number {
		return (
			HandleLook.size * HandleLook.shareOfTheSizeBeforeASurround * app.dpiScale
		);
	}

	/// How wide a band is drawn, in core pixels.
	public static widthOfASurround(): number {
		return (
			HandleLook.size * HandleLook.shareOfTheSizeOfASurround * app.dpiScale
		);
	}

	/// How far the outer edge of a band lies from what it goes round, in core pixels.
	public static reachOfASurround(): number {
		return HandleLook.distanceBeforeASurround() + HandleLook.widthOfASurround();
	}

	/*
		Lays the outline of a handle of that kind into the path being built, grown by however much
		the handle is being shown larger, and answers the color it is filled with.
	*/
	public static lay(
		context: CanvasRenderingContext2D,
		kind: HandleKind,
		at: cool.SimplePoint,
		grown: number,
		turnedBy?: number,
	): string {
		const across = HandleLook.widthOf(kind) + 2 * grown;

		if (HandleLook.shapeOfTheKind[kind] !== 'square')
			context.arc(at.vX, at.vY, across * 0.5, 0, Math.PI * 2);
		else if (turnedBy && HandleLook.turnTheEightWithTheObject) {
			// The corners of the square are laid into the path under the turn and stay where
			// they were laid once the transform is put back.
			context.save();
			context.translate(at.vX, at.vY);
			context.rotate(turnedBy);
			context.rect(-across * 0.5, -across * 0.5, across, across);
			context.restore();
		} else
			context.rect(at.vX - across * 0.5, at.vY - across * 0.5, across, across);

		return HandleLook.colourOfTheKind[kind];
	}
}
