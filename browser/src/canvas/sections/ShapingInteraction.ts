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
	Dragging a handle that shapes the object itself: the corner radius of a rectangle, the points
	a custom shape is shaped by, the handles of a measurement. What such a handle does to the
	object is the object's own business - a radius is not a transformation of it - so the object
	is not drawn through the drag; a measurement has its lines drawn thin from what the engine sent
	of it. The handle follows the mouse, and the engine is told where it was let go.
*/
class ShapingInteraction extends SelectionInteraction {
	/// The handle that was taken hold of.
	private handle: SelectionHandle;

	/// Where it has been taken, in twips.
	private at: cool.SimplePoint;

	/// Whether Shift is held, which moves the handle as the office moves it then.
	private ortho: boolean = false;

	/// The steps a handle of a measurement snaps to while Ctrl or Alt is held, or nothing.
	private snap: 'whole' | 'tenth' | null = null;

	constructor(selection: SelectionSection, handle: SelectionHandle) {
		super(selection);

		this.handle = handle;

		// Before the mouse has moved the handle stands where it stood, whatever part of it the
		// press landed on.
		this.at = new cool.SimplePoint(handle.point.x, handle.point.y);
	}

	/*
		Where the handle is drawn while it is dragged, which is where the mouse has taken it, held
		to whatever the object allows it. What that is, the object said when it sent the handle.
	*/
	private shownAt(): cool.Point {
		const held = this.selection.holdTheDrag(
			this.handle,
			new cool.Point(this.at.x, this.at.y),
		);
		const kept = this.ortho
			? (MeasurementGhost.heldWithShift(this.selection, this.handle, held) ??
				LineGhost.heldWithShift(this.selection, this.handle, held) ??
				held)
			: held;
		if (!this.snap) return kept;

		return (
			MeasurementGhost.snapped(this.selection, this.handle, kept, this.snap) ??
			LineGhost.snapped(this.selection, this.handle, kept, this.snap) ??
			kept
		);
	}

	public move(to: cool.SimplePoint, event?: MouseEvent): void {
		this.at = this.led(to);
		if (event) {
			this.ortho = event.shiftKey;
			this.snap = event.ctrlKey ? 'whole' : event.altKey ? 'tenth' : null;
		}
		this.selection.redraw();
	}

	/*
		The drag ends where the mouse let go, and what the engine is told about it is the business
		of the group the handle belongs to: a handle the engine knows is named to it, a point of a
		path is named as a point of that path, a point to tie a connector to is moved.
	*/
	public finish(to: cool.SimplePoint, event?: MouseEvent): void {
		this.move(to, event);

		this.selection.handOverTheDrag(this.handle, this.shownAt(), this.ortho);
	}

	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const shown = this.shownAt();

		// The handles of a measurement all go where the drag would leave them.
		const measured = MeasurementGhost.handlesDuring(
			this.selection,
			this.handle,
			shown,
			known,
			this.ortho,
		);
		if (measured) return measured;

		// A point of a path carries its weights, and one weight can swing the other, so the
		// editor says where all three of them stand.
		const path = this.selection
			.pathEditor()
			?.handlesDuring(this.handle.name, shown, known);
		if (path) return path;

		return known.map((one: SelectionHandle) =>
			one.name === this.handle.name ? { ...one, point: shown } : one,
		);
	}

	public handleHeld(): SelectionHandle | null {
		return this.handle;
	}

	public leadingPoint(): cool.SimplePoint | null {
		const shown = this.shownAt();
		return new cool.SimplePoint(shown.x, shown.y);
	}

	/*
		The path the object would be drawn along while one of its points is dragged, which the
		editor of that path works out.
	*/
	public outline(): cool.ObjectPathPolygon[] | null {
		const path = this.selection
			.pathEditor()
			?.outlineFor(this.handle.name, this.shownAt());

		return (
			path ??
			this.selection.runWhileAnEndIsDragged(this.handle, this.shownAt()) ??
			this.selection.runWhileALineIsDragged(this.handle, this.shownAt()) ??
			MeasurementGhost.outline(
				this.selection,
				this.handle,
				this.shownAt(),
				this.ortho,
			) ??
			LineGhost.outline(this.selection, this.handle, this.shownAt())
		);
	}

	/// The value a measurement would show, while an end of its measured line is dragged.
	public reading(): string | null {
		return MeasurementGhost.reading(
			this.selection,
			this.handle,
			this.shownAt(),
			this.ortho,
		);
	}
}

/*
	A measurement as one of its handles would leave it, drawn from what the engine sent of it: the
	dimension line and the two help lines, worked out on the measurement's square the way the
	office lays them out, and the value an end of the measured line would make it show. The
	handles are its own six: 0 and 1 begin the help lines, 2 and 3 are the ends of the measured
	line, 4 and 5 are the far ends of the help lines.
*/
class MeasurementGhost {
	/// The kind the engine gives the handles of a measurement.
	private static readonly handleKind: string = '20';

	/// The measurement the handle belongs to and the square it stands on, or nothing where the
	/// handle belongs to no measurement.
	private static of(
		selection: SelectionSection,
		handle: SelectionHandle,
	): { measurement: cool.ObjectMeasurement; square: cool.Matrix2D } | null {
		if (handle.kind !== MeasurementGhost.handleKind) return null;

		const objectId = selection.selectedObjects()[0];
		if (objectId === undefined) return null;

		const measurement = RenderGeometrySection.objectOf(objectId)?.measurement;
		const square = RenderGeometrySection.transformOf(objectId);
		return measurement && square ? { measurement, square } : null;
	}

	/// The value a measurement writes, read back from the way it is written with its separator
	/// taken for a point. Nothing where it cannot be read.
	private static valueOf(measurement: cool.ObjectMeasurement): number | null {
		const separator = measurement.decimalSeparator || '.';
		const digits = Array.from(measurement.value)
			.filter(
				(character: string) =>
					(character >= '0' && character <= '9') ||
					character === '-' ||
					character === separator,
			)
			.join('')
			.replace(separator, '.');
		const value = parseFloat(digits);
		return isNaN(value) ? null : value;
	}

	/*
		Where a handle of a measurement goes while Ctrl or Alt is held: to the next whole unit of
		what the measurement writes, or with Alt to the next tenth of it. The
		ends of the measured line snap its length, measured from the other end along the way the
		handle goes; the other handles snap the length they change, the length of a help line or
		how far the dimension line stands. The unit is the one of the value, worked out from the
		value written now and the length it is written for. Nothing where the handle belongs to no
		measurement or the value cannot be read.
	*/
	public static snapped(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
		steps: 'whole' | 'tenth',
	): cool.Point | null {
		const found = MeasurementGhost.of(selection, handle);
		if (!found) return null;

		const square = found.square;
		const origin = square.apply(0, 0);
		const unit = square.apply(0, 1).subtract(origin).length();
		const length = unit
			? square.apply(1, 0).subtract(origin).length() / unit
			: 0;
		const value = MeasurementGhost.valueOf(found.measurement);
		if (!length || !value) return null;

		// A tenth of the unit with Alt, where the value is written with places at all.
		const written =
			steps === 'whole' || found.measurement.decimals < 1 ? 1 : 0.1;
		// One step in the units of the model.
		const step = (written * length) / value;
		const onAStep = (model: number): number => Math.round(model / step) * step;

		const which = handle.at ?? 0;
		if (which === 2 || which === 3) {
			// The way goes in twips, the step in the units of the model.
			return EndOnAWay.snapped(
				square.apply(which === 2 ? 1 : 0, 0),
				at,
				step * unit,
			);
		}

		const back = square.invert();
		if (!back) return null;
		const across = back.apply(at.x, at.y).y;
		const x = which === 0 || which === 4 ? 0 : 1;

		if (which === 0 || which === 1) return square.apply(x, onAStep(across));

		// The dimension line stands as far as the handle less the overhang; that is what snaps.
		const overhang = found.measurement.overhang;
		const side = across < 0 ? -1 : 1;
		return square.apply(
			x,
			side * (onAStep(Math.abs(across) - overhang) + overhang),
		);
	}

	/*
		Where a handle of a measurement goes while Shift is held, as the office has it: an end of
		the measured line stays on the line's way, the far end of a help line stays where it is,
		and the beginning of a help line goes where its rail takes it. Nothing for a handle of
		anything else.
	*/
	public static heldWithShift(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
	): cool.Point | null {
		const found = MeasurementGhost.of(selection, handle);
		if (!found) return null;

		const which = handle.at ?? 0;
		const square = found.square;
		if (which === 2 || which === 3) {
			// a measured line without a length has no way to hold the end to
			if (ObjectHandles.spotWithoutLength(selection.selectedObjects()[0]))
				return at;
			return EndOnAWay.heldTo(
				square.apply(which === 2 ? 1 : 0, 0),
				square.apply(1, 0).subtract(square.apply(0, 0)),
				at,
			);
		}
		if (which === 4 || which === 5) return handle.point;
		return at;
	}

	/*
		The measurement and its square as the handle at that point would leave them. An end of the
		measured line makes a square of its own, along the line from the first end to the second,
		as high as before. Every other handle changes one length across the square, measured from
		the measured line: the length of a help line, or how far the dimension line stands, which
		moves it to the other side where the handle crosses the measured line.
	*/
	private static shaped(
		measurement: cool.ObjectMeasurement,
		square: cool.Matrix2D,
		handle: SelectionHandle,
		at: cool.Point,
		ortho: boolean,
	): { measurement: cool.ObjectMeasurement; square: cool.Matrix2D } | null {
		const which = handle.at ?? 0;

		if (which === 2 || which === 3) {
			const start = which === 2 ? at : square.apply(0, 0);
			const end = which === 3 ? at : square.apply(1, 0);
			const along = end.subtract(start);
			const length = along.length();
			if (!length) return null;

			const high = square.apply(0, 1).subtract(square.apply(0, 0)).length();
			return {
				measurement: measurement,
				square: new cool.Matrix2D(
					along.x,
					along.y,
					(-along.y / length) * high,
					(along.x / length) * high,
					start.x,
					start.y,
				),
			};
		}

		const back = square.invert();
		if (!back) return null;
		const across = back.apply(at.x, at.y).y;
		const side = measurement.below ? 1 : -1;
		const changed = { ...measurement };

		// With Shift, as the office has it, the two help lines keep one length, and the dimension
		// line keeps its distance.
		if (which === 0 || which === 1) {
			const length = -side * across;
			if (which === 0 || ortho) changed.helpLineLength1 = length;
			if (which === 1 || ortho) changed.helpLineLength2 = length;
		} else if ((which === 4 || which === 5) && !ortho) {
			if (across) changed.below = across > 0;
			changed.lineDistance = Math.abs(across) - measurement.overhang;
		}

		return { measurement: changed, square: square };
	}

	/// The measurement as the handle at that point would leave it, in twips: its lines and the
	/// box of its text.
	public static outline(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
		ortho: boolean = false,
	): cool.ObjectPathPolygon[] | null {
		const found = MeasurementGhost.of(selection, handle);
		if (!found) return null;

		const shaped = MeasurementGhost.shaped(
			found.measurement,
			found.square,
			handle,
			at,
			ortho,
		);
		if (!shaped) return null;

		// The text grows or shrinks with the value it writes, where an end is dragged.
		const value = MeasurementGhost.reading(selection, handle, at, ortho);
		const written = found.measurement.value + ' ' + found.measurement.unit;
		const textWidth =
			value !== null && written.trim().length
				? (found.measurement.textWidth * value.length) / written.trim().length
				: found.measurement.textWidth;

		return MeasurementGhost.laidOut(
			shaped.measurement,
			shaped.square,
			textWidth,
		);
	}

	/*
		The lines and the box of the text of a measurement on that square, laid out as the office
		lays out its drawing (SdrMeasurePrimitive2D): along the line from 0 at the first point to
		its length at the second, across it from the measured line, all in the units of the
		model. The arrows are not drawn, but they take their room as in the drawing: they go
		outside the help lines where they do not fit between them, the text goes beside the line
		where it does not fit either, the dimension line is split round a text that stands on it,
		and the text changes sides where it stands below or turns itself upside down.
	*/
	private static laidOut(
		measurement: cool.ObjectMeasurement,
		square: cool.Matrix2D,
		textWidth: number,
	): cool.ObjectPathPolygon[] | null {
		const origin = square.apply(0, 0);
		const unit = square.apply(0, 1).subtract(origin).length();
		const along = square.apply(1, 0).subtract(origin);
		const length = unit ? along.length() / unit : 0;
		if (!length) return null;

		const startWidth = measurement.startArrowWidth;
		const startLength = measurement.startArrowLength;
		const endWidth = measurement.endArrowWidth;
		const endLength = measurement.endArrowLength;
		const textHeight = measurement.textHeight;

		const needed = startLength + endLength + (startWidth + endWidth) * 0.5;
		const outsideLength =
			(startLength + endLength + startWidth + endWidth) * 0.5;
		const halfLine = measurement.lineWidth * 0.5;

		let arrowsOutside = needed > length;
		let textAlong = measurement.textAlong;
		let textAcross = measurement.textAcross === 2 ? -1 : measurement.textAcross;
		const split = textAcross === 0;

		if (textAlong === 2) {
			textAlong = textWidth > length ? -1 : 0;
			const smallNeed =
				startLength + endLength + (startWidth + endWidth) * 0.125;
			if (textWidth + (split ? needed : smallNeed) > length)
				arrowsOutside = true;
		}
		if (textAlong !== 0) arrowsOutside = true;

		// The text turns itself upside down by the angle of the line on the page.
		let angle = Math.atan2(along.y, along.x);
		if (measurement.textTurnedRightAngle) {
			angle -= Math.PI / 2;
			if (measurement.textTurnsItself && angle < -Math.PI) angle += 2 * Math.PI;
		}
		const upsideDown =
			measurement.textTurnsItself === true &&
			(angle > Math.PI / 4 || angle < (-Math.PI * 3) / 4);
		if (measurement.below || (upsideDown && !measurement.textTurnedRightAngle))
			textAcross = -textAcross;

		const side = measurement.below ? 1 : -1;
		const line = side * measurement.lineDistance;

		const through = (x: number, y: number): cool.ObjectPathPoint => {
			const point = square.apply(x / length, y);
			return { x: point.x, y: point.y };
		};
		const segment = (from: number, to: number): cool.ObjectPathPolygon => ({
			points: [through(from, line), through(to, line)],
		});
		const drawn: cool.ObjectPathPolygon[] = [];

		if (arrowsOutside) {
			let leftLength = outsideLength;
			let rightLength = outsideLength;
			if (!split) {
				if (textAlong === -1) leftLength = startLength + textWidth;
				else if (textAlong === 1) rightLength = endLength + textWidth;
			}

			drawn.push(
				segment(-leftLength, 0),
				segment(length, length + rightLength),
			);
			if (!split || textAlong !== 0) drawn.push(segment(0, length));
		} else if (split) {
			const half =
				(length - (textWidth + (startLength + endLength) * 0.25)) * 0.5;
			drawn.push(segment(0, half), segment(length - half, length));
		} else {
			drawn.push(segment(0, length));
		}

		// The help lines, from where they begin near the measured line to beyond the dimension
		// line.
		const far = side * (measurement.lineDistance + measurement.overhang);
		const near1 =
			side * (measurement.helpLineDistance - measurement.helpLineLength1);
		const near2 =
			side * (measurement.helpLineDistance - measurement.helpLineLength2);
		drawn.push(
			{ points: [through(0, near1), through(0, far)] },
			{ points: [through(length, near2), through(length, far)] },
		);

		// The box of the text, placed along and across the line.
		const small = outsideLength;
		let textX: number;
		if (textAlong === -1) {
			textX =
				-(startLength + textWidth + small * 0.18 + halfLine) -
				measurement.textRightDistance;
			if (split) textX -= outsideLength - startLength;
		} else if (textAlong === 1) {
			textX =
				length +
				endLength +
				small * 0.18 +
				halfLine +
				measurement.textLeftDistance;
			if (split) textX += outsideLength - endLength;
		} else {
			textX =
				(length - textWidth) * 0.5 +
				(measurement.textLeftDistance - measurement.textRightDistance) / 2;
		}

		let textY: number;
		if (textAcross === -1)
			textY =
				line -
				(textHeight + small * 0.1 + halfLine) -
				measurement.textLowerDistance;
		else if (textAcross === 1)
			textY = line + small * 0.1 + halfLine + measurement.textUpperDistance;
		else
			textY =
				line -
				textHeight * 0.5 +
				(measurement.textUpperDistance - measurement.textLowerDistance) / 2;

		drawn.push({
			closed: true,
			points: [
				through(textX, textY),
				through(textX + textWidth, textY),
				through(textX + textWidth, textY + textHeight),
				through(textX, textY + textHeight),
			],
		});

		return drawn;
	}

	/*
		The handles of the measurement as the handle at that point would leave them, each where
		the office puts it: 0 and 1 where the help lines begin, as far from the measured line as
		they are longer, 2 and 3 on the ends of the measured line, 4 and 5 at the far ends of the
		help lines. Handles of anything else stay as they are. Nothing where the handle belongs
		to no measurement.
	*/
	public static handlesDuring(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
		known: SelectionHandle[],
		ortho: boolean = false,
	): SelectionHandle[] | null {
		const found = MeasurementGhost.of(selection, handle);
		if (!found) return null;

		const shaped = MeasurementGhost.shaped(
			found.measurement,
			found.square,
			handle,
			at,
			ortho,
		);
		if (!shaped) return null;

		const measurement = shaped.measurement;
		const side = measurement.below ? 1 : -1;
		const far = side * (measurement.lineDistance + measurement.overhang);
		const places: { [which: number]: number[] } = {
			0: [0, -side * measurement.helpLineLength1],
			1: [1, -side * measurement.helpLineLength2],
			2: [0, 0],
			3: [1, 0],
			4: [0, far],
			5: [1, far],
		};

		return known.map((one: SelectionHandle) => {
			if (one.kind !== MeasurementGhost.handleKind) return one;
			const place = places[one.at ?? 0];
			if (!place) return one;

			return { ...one, point: shaped.square.apply(place[0], place[1]).round() };
		});
	}

	/*
		The value the measurement would show while an end of its measured line is dragged: the
		value it shows now, grown or shrunk as the line is, written with as many places as it
		writes, without the noughts at the end, and with its unit. Nothing for the other handles,
		which do not change it.
	*/
	public static reading(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
		ortho: boolean = false,
	): string | null {
		const which = handle.at ?? 0;
		if (which !== 2 && which !== 3) return null;

		const found = MeasurementGhost.of(selection, handle);
		if (!found) return null;

		const shaped = MeasurementGhost.shaped(
			found.measurement,
			found.square,
			handle,
			at,
			ortho,
		);
		if (!shaped) return null;

		const lengthOf = (square: cool.Matrix2D): number =>
			square.apply(1, 0).subtract(square.apply(0, 0)).length();
		const was = lengthOf(found.square);
		if (!was) return null;

		const separator = found.measurement.decimalSeparator || '.';
		const value = MeasurementGhost.valueOf(found.measurement);
		if (value === null) return null;

		let written = ((value * lengthOf(shaped.square)) / was).toFixed(
			Math.max(0, found.measurement.decimals),
		);
		if (written.includes('.'))
			written = written.replace(/0+$/, '').replace(/\.$/, '');
		written = written.replace('.', separator);

		return found.measurement.unit
			? written + ' ' + found.measurement.unit
			: written;
	}
}

/*
	a line while one of its two ends is dragged: drawn thin from the end that stays to the one
	that moves. with Shift the end keeps to the way the line runs, as the office has it
*/
class LineGhost {
	/// the kind the engine gives the two ends of a line
	private static readonly handleKind: string = '9';

	/// the end that stays where it is, while the other one is dragged
	private static otherEnd(
		selection: SelectionSection,
		handle: SelectionHandle,
	): cool.Point | null {
		if (handle.kind !== LineGhost.handleKind) return null;

		const objectIds = selection.selectedObjects();
		if (objectIds.length !== 1) return null;

		// a line, and not a connector, which has these handles too
		if (!ObjectHandles.isALine(objectIds[0])) return null;

		const other = ObjectHandles.shapingOf(objectIds).find(
			(one: SelectionHandle) =>
				one.kind === LineGhost.handleKind && one.name !== handle.name,
		);
		return other ? other.point : null;
	}

	public static heldWithShift(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
	): cool.Point | null {
		const fixed = LineGhost.otherEnd(selection, handle);
		if (!fixed) return null;

		return EndOnAWay.heldTo(fixed, handle.point.subtract(fixed), at);
	}

	/// the end at that point with the length of the line snapped to the step of the modifier,
	/// on the way it goes from the other end
	public static snapped(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
		steps: 'whole' | 'tenth',
	): cool.Point | null {
		const fixed = LineGhost.otherEnd(selection, handle);
		if (!fixed) return null;

		return EndOnAWay.snapped(fixed, at, EditSteps.lengthSnap(steps));
	}

	public static outline(
		selection: SelectionSection,
		handle: SelectionHandle,
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const fixed = LineGhost.otherEnd(selection, handle);
		if (!fixed) return null;

		return [
			{
				points: [
					{ x: fixed.x, y: fixed.y },
					{ x: at.x, y: at.y },
				],
			},
		];
	}
}

/*
	an end that moves away from one that stays: held to a way through the one that stays, or with
	its distance from it snapped to a step
*/
class EndOnAWay {
	/// the place on the way through fixed that lies nearest to at
	public static heldTo(
		fixed: cool.Point,
		way: cool.Point,
		at: cool.Point,
	): cool.Point {
		const length = way.length();
		if (!length) return at;

		const unit = way.divideBy(length);
		return fixed.add(unit.multiplyBy(at.subtract(fixed).dot(unit)));
	}

	/// at, with its distance from fixed snapped to steps of that size, and never less than one
	public static snapped(
		fixed: cool.Point,
		at: cool.Point,
		step: number,
	): cool.Point | null {
		const way = at.subtract(fixed);
		const reach = way.length();
		if (!reach || !step) return null;

		const snappedReach = Math.max(step, Math.round(reach / step) * step);
		return fixed.add(way.multiplyBy(snappedReach / reach));
	}
}
