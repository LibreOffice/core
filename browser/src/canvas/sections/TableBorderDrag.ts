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
	The drag of a border marker of a table: the border follows the mouse across, for a column, or
	down, for a row, kept between the borders beside it, and is moved on the engine when the
	mouse is let go, as the tiled view moves it. While it runs the marker goes along and a line
	across the table shows where the border would go.
*/
class TableBorderDrag extends SelectionInteraction {
	/// The border being dragged and where it stands now, in twips; absent while none is.
	public static running: { columns: boolean; place: number } | null = null;

	// how near a border may come to the one beside it, in CSS pixels, as in the tiled view
	private static readonly nearest = 10;

	private handle: SelectionHandle;
	private columns: boolean;
	// the place of the border in the list of borders, the outer ones included
	private at: number;
	private last: boolean;
	private start: number;
	private low: number;
	private high: number;

	constructor(
		selection: SelectionSection,
		handle: SelectionHandle,
		table: TableLayout,
	) {
		super(selection);
		this.handle = handle;
		this.columns = handle.kind === TableHandles.columnBorder;
		this.at = Number(handle.name.split('.')[2]);
		const lines = this.columns ? table.columns : table.rows;
		this.last = this.at === lines.length - 1;
		this.start = lines[this.at];
		const limits = TableBorderDrag.limits(lines, this.at);
		this.low = limits.low;
		this.high = limits.high;
		TableBorderDrag.running = { columns: this.columns, place: this.start };
	}

	public move(to: cool.SimplePoint): void {
		const led = this.led(to);
		const place = Math.min(
			this.high,
			Math.max(this.low, this.columns ? led.x : led.y),
		);
		TableBorderDrag.running = { columns: this.columns, place: place };
		app.sectionContainer?.requestReDraw();
	}

	public finish(to: cool.SimplePoint): void {
		this.move(to);
		const offset = Math.round(
			(TableBorderDrag.running?.place ?? this.start) - this.start,
		);
		TableBorderDrag.running = null;
		TableBorderDrag.moveBorder(this.columns, this.at, this.last, offset);
	}

	/*
		Moves a border of the selected table on the engine by that much, in twips: the border at
		that place in the list of borders between columns, or between rows, the outer ones
		included, where last says it is the outer one at the end.
	*/
	public static moveBorder(
		columns: boolean,
		at: number,
		last: boolean,
		offset: number,
	): void {
		if (!offset) return;
		// the engine names an inner border by the column or row before it, the outer one alone
		const type = (columns ? 'column-' : 'row-') + (last ? 'right' : 'middle');
		app.map.sendUnoCommand('.uno:TableChangeCurrentBorderPosition', {
			BorderType: { type: 'string', value: type },
			Index: { type: 'uint16', value: last ? 0 : at - 1 },
			Offset: { type: 'int32', value: offset },
		});
		app.activeDocument?.tableMiddleware?.moveBorderLocally(columns, at, offset);
	}

	/// The places a border may go between the ones beside it, in twips.
	public static limits(
		lines: number[],
		at: number,
	): { low: number; high: number } {
		const nearest = TableBorderDrag.nearest * app.dpiScale * app.pixelsToTwips;
		return {
			low: lines[at - 1] + nearest,
			high: at === lines.length - 1 ? Infinity : lines[at + 1] - nearest,
		};
	}

	public cancel(): void {
		TableBorderDrag.running = null;
		app.sectionContainer?.requestReDraw();
	}

	/// The marker being dragged stands where the border would go.
	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const running = TableBorderDrag.running;
		if (!running) return known;
		return known.map((handle: SelectionHandle) =>
			handle.name !== this.handle.name
				? handle
				: {
						...handle,
						point: this.columns
							? new cool.Point(running.place, handle.point.y)
							: new cool.Point(handle.point.x, running.place),
					},
		);
	}
}
