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
	/** One handle of an object that a reader cannot work out from what the object draws: the
	 * corner radius of a rectangle, kind 11, and the points a custom shape is shaped by, kind 22.
	 *
	 * The kind, with the polygon and the point it belongs to, is what names the handle wherever it
	 * is spoken about. The position is in twips.
	 */
	export interface ObjectHandle {
		kind: number;
		polygon?: number;
		point?: number;
		/// The number the object itself gives the handle, which tells apart the handles that
		/// are alike in every other way. Absent where the kind, the polygon and the point
		/// already name it.
		at?: number;
		x: number;
		y: number;
		/// How far the handle may be moved, for a point a custom shape is shaped by. Absent for a
		/// handle of another kind.
		rails?: ObjectHandleRails;
	}

	/** How far a point a custom shape is shaped by may be moved, given as the places the point
	 * itself would stand at, on the object's own square: 0 and 0 is its upper left corner and 1
	 * and 1 its lower right one.
	 *
	 * A handle moves across the shape, down it, both, or around a point of it. A place that is
	 * not given leaves that side open. The names say which end of the shape's own value a place
	 * comes from, not which side of the shape it lies on: a shape that counts the other way
	 * round puts the smallest one on the right.
	 */
	export interface ObjectHandleRails {
		/// A handle that runs on the object's two ways: whether it moves on each of them, and
		/// the places it is held between on each. A place that is not given leaves that side
		/// open, and an axis it does not move on keeps the place it had.
		movesAcross?: boolean;
		movesDown?: boolean;
		leastAcross?: ObjectPlace;
		mostAcross?: ObjectPlace;
		leastDown?: ObjectPlace;
		mostDown?: ObjectPlace;
		/// A handle that turns about a point: the point, the nearest and furthest it may come to
		/// it, and the two ends of the piece of the way round it may take.
		movesAround?: boolean;
		around?: ObjectPlace;
		nearest?: ObjectPlace;
		furthest?: ObjectPlace;
		from?: ObjectPlace;
		to?: ObjectPlace;
		/// Whether the handle keeps a length of its own when the object is made larger or
		/// smaller, rather than travelling with it.
		keepsItsLength?: boolean;
	}

	/// A place on the object's own square, where 0 and 0 is its upper left corner and 1 and 1 its
	/// lower right one.
	export interface ObjectPlace {
		x: number;
		y: number;
	}

	/** One point of a path, in twips, with the weights of the curve it lies on where they are
	 * in use: the one behind the point and the one ahead of it, named as a handle names them.
	 */
	export interface ObjectPathPoint {
		x: number;
		y: number;
		behindX?: number;
		behindY?: number;
		aheadX?: number;
		aheadY?: number;
		/// How the curve carries on through the point: 1 when the two weights lie on one line
		/// through it, 2 when they lie on one line and are the same length as well. Absent when
		/// the curve turns a corner there. It is worked out by the engine on the model's own
		/// numbers, which are finer than the twips here.
		continuity?: 1 | 2;
	}

	/**
	 * How one end of a connector holds on to its object, or where it lies when it holds on to
	 * none. A tied end has the number of the glue point it is tied to, or none while the
	 * connector picks the best point itself, and the flags say how that pick goes: whether the
	 * best of the four sides is searched for, and whether the pick landed on a side or on a
	 * corner. A free end has its place on the page instead, in twips.
	 */
	export interface ConnectorTie {
		point?: number;
		bestVertex?: boolean;
		autoVertex?: boolean;
		autoCorner?: boolean;
		x?: number;
		y?: number;
	}

	/**
	 * The numbers a connector's run is worked out from, in twips where they are lengths. The kind
	 * names the shape of the run: "ortho" for lines meeting at right angles, "three" for three
	 * straight lines, "one" for one straight line, "bezier" for a curve and "arc" for an arc. The
	 * curve follows the OOXML way of routing when the flag says so. The node distances are how
	 * far the run keeps away from the first object, across and down, then from the second. The
	 * line deltas are how far the user moved the lines of an orthogonal run from where the
	 * routing would put them, and the count says how many of the three are set. The run is laid
	 * out against the objects' snap rectangles when the flag says so, their bound rectangles
	 * otherwise.
	 */
	export interface ConnectorRouting {
		kind: 'ortho' | 'three' | 'one' | 'bezier' | 'arc';
		curveFollowsOOXML?: boolean;
		nodeDistances: number[];
		lineDeltaCount: number;
		lineDeltas: number[];
		againstSnapRectangles?: boolean;
		/// True for a run that was set from outside, by the file or by hand, and is kept as it
		/// is rather than laid out from the numbers here.
		trackUserDefined?: boolean;
		tieAtStart: ConnectorTie;
		tieAtEnd: ConnectorTie;
	}

	/** One point an object was given for a connector to tie itself to, where it lies on the
	 * object: 0 and 0 is its upper left corner and 1 and 1 its lower right one.
	 */
	export interface ObjectGluePoint {
		x: number;
		y: number;
		/// The number the point is known by on the object, which is how a connector tied to it
		/// names it.
		id?: number;
		/// True for a point the object holds as a distance of its own rather than as a share of
		/// its size: making the object larger leaves such a point where it is.
		keepsItsDistance?: boolean;
		/// The way a connector leaves the object at this point: one side of it, or the two that
		/// face each other. Absent where the point names none and the way is worked out from
		/// where the point lies. These six are all a point can hold: the model can hold any mix
		/// of the four sides, but no document can carry one, so a mix never arrives.
		wayOut?: 'left' | 'right' | 'top' | 'bottom' | 'horizontal' | 'vertical';
		/// The side the place of the point is measured from, across the object and down it.
		/// Absent where it is measured from the middle.
		fromAcross?: 'left' | 'right';
		fromDown?: 'top' | 'bottom';
	}

	/** One polygon of the path of an object, as the model holds it. The polygons and their
	 * points keep the model's order, which is the order that names a point when it is moved.
	 */
	export interface ObjectPathPolygon {
		closed?: boolean;
		points: ObjectPathPoint[];
	}

	/// One drawable object on a slide, carrying its primitive tree.
	export interface SlideObject {
		/// Stable identity of the object: the engine's SdrObject unique
		/// id, unchanged across edits to the same object.
		id?: number;
		/// The aids that mark out a placeholder: a dashed boundary around
		/// the area it occupies and, on a master page, the name of the
		/// area. They are drawn apart from the object's own content.
		aids?: Primitive[];
		/// "page" for the entry that stands for the slide itself: it is
		/// drawn first and holds the background, the page fill and the
		/// master page content, and its box is the slide.
		/// "texteditoverlay" for an entry that carries the text of a
		/// running text edit: it is drawn last, over the object it runs
		/// on, which hides its own text while the edit runs. There is one
		/// per view that is editing, and two of them can name the same
		/// object. Absent for a drawing object.
		kind?: 'page' | 'texteditoverlay';
		/// Which view's text edit an entry of kind "texteditoverlay"
		/// carries, so a reader can tell its own from another user's.
		viewId?: number;
		/// Id of the group the object sits in, 0 for an object directly
		/// on the slide and -1 on the entry of kind "page", which sits
		/// under nothing, so a walk up the parents ends there. A group's
		/// members follow it in the object list and draw its content, so
		/// a group with members has no primitives of its own.
		parent?: number;
		/// Id of the layer the object is on.
		layer?: number;
		/// True for a placeholder that holds no content of its own yet.
		emptyPlaceholder?: boolean;
		/// On the entry of kind "page" of a slide: the id of the master part
		/// the slide draws under itself. Absent when the page carries its
		/// master content inline.
		masterPartId?: VectorPartGuid;
		/// On the entry of kind "page" of a slide: the ids of the layers of
		/// its master the slide does not show. Absent when it shows them all.
		masterHiddenLayers?: number[];
		/// On an object of a slide: it is the slide's own copy of a master
		/// placeholder, rendered for this slide.
		masterContent?: boolean;
		/// On a master object: it is a layout prototype, or an empty
		/// placeholder with neither fill nor line.
		hiddenBehindSlide?: boolean;
		/// On a master object: its content differs per slide. Each slide
		/// carries its own copy of it, under this object's id.
		slideDependent?: boolean;
		/// True while a text edit is running on the object. It shows none of
		/// its own text then, and the entry of kind "texteditoverlay"
		/// carries what has been typed.
		textEdit?: boolean;
		/// Rectangle the object paints, in twips: the primitives' range,
		/// so it takes in the line width and a shadow.
		x?: number;
		y?: number;
		width?: number;
		height?: number;
		/// Mapping of the unit square onto the object, in twips, as the
		/// six canvas matrix values [a, b, c, d, e, f].
		transform?: number[];
		/// The handles that shape the object, empty for an object that has none of them.
		handles?: ObjectHandle[];
		/// The path the object is drawn from, as the model holds it. Absent for an object
		/// that has no path of its own.
		path?: ObjectPathPolygon[];
		/// What kind of object it is, as the drawing layer numbers the kinds: 2 a line, 24 a
		/// connector, 25 a caption, 29 a measurement, 33 a custom shape, 35 a table.
		objectKind?: number;
		/// True where the object is a diagram, which is a set of shapes the office lays out from
		/// data it holds beside them.
		isDiagram?: boolean;
		/// The file name ending of the picture the object holds, absent where it holds none.
		graphicExtension?: string;
		/// True while the document is being signed and a certificate has been chosen. The
		/// document is read-only then, and the object may still be moved and resized.
		signing?: boolean;
		/** For a picture: the size the graphic has by its own resolution, and the four amounts
		 * cropped away from it, left, top, right and bottom, all in twips of that size. The
		 * object's box is the window onto what is left. Absent for anything but a picture that
		 * shows a graphic.
		 */
		graphicChecksum?: number;
		/// How the graphic is turned over as it is drawn: bit 0 across, bit 1 down. Absent
		/// where it is drawn as it is.
		graphicMirror?: number;
		graphicWidth?: number;
		graphicHeight?: number;
		cropLeft?: number;
		cropTop?: number;
		cropRight?: number;
		cropBottom?: number;
		/** What may not be done to the object. Absent means it may: most objects allow
		 * everything, so only the refusals travel. Whether it may be moved and whether it may be
		 * made larger or smaller are each held against being changed on their own. Whether it may
		 * be turned is something the object answers about itself, and a media object will not.
		 */
		cannotBeMoved?: boolean;
		cannotBeResized?: boolean;
		cannotBeTurned?: boolean;
		/// True where the object allows a resize in ratio only: a text frame turned off the right
		/// angles, a shape turned at all. A selection holding one keeps the ratio as a whole.
		cannotBeResizedFreely?: boolean;
		/// True where the object may not be turned over, a media object for one.
		cannotBeMirrored?: boolean;
		/// True where the object cannot hold a shear at all, a text frame for one.
		cannotBeSheared?: boolean;
		gluePoints?: ObjectGluePoint[];
		/** The four glue points the object has at the middles of its sides - the upper, the
		 * right, the lower and the left one - where they lie on the object, as gluePoints are
		 * given. The object places them itself, turned and sheared with it on some objects, put
		 * out by half its line on others. Absent for a connector.
		 */
		sideGluePoints?: ObjectPlace[];
		/// True for a connector whose first end is tied to an object, and the same for its last
		/// end. Absent for anything that is not a connector, and for an end that is free.
		tiedAtStart?: boolean;
		tiedAtEnd?: boolean;
		/// The id of the object a tied end holds on to. Absent for a free end, and for anything
		/// that is not a connector.
		tiedToAtStart?: number;
		tiedToAtEnd?: number;
		/// The numbers a connector's run between its two ends is worked out from. Absent for
		/// anything that is not a connector.
		routing?: ConnectorRouting;
		/** The points someone added to the object for a connector to tie itself to, where they
		 * lie on the object: 0 and 0 is its upper left corner and 1 and 1 its lower right one, so
		 * a point follows the object wherever it goes and however it is turned. Absent for an
		 * object that was given none, and then the four middles of the sides of its box are what
		 * a connector can reach for.
		 *
		 * keepsItsDistance is true for a point the object holds as a distance of its own rather
		 * than as a share of its size: making the object larger leaves such a point where it is,
		 * so its place on the object changes.
		 */
		primitives?: Primitive[];
	}
}
