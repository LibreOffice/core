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
	The markers of a selected table in a view that draws from objects, as handles among the
	others of the selection: a square above each column and beside each row that selects its
	cells, a marker above each border between columns and beside each border between rows that
	moves it, a bar beyond the right side and one below the table that add a column or a row, and
	the anchor at the upper left that moves the table. They come from the rows and columns the
	server sends with the selection of a table, the same the markers of the tiled view come from,
	and look as those do. They stand outside the frame, beyond the eight, so that every handle is
	free to take hold of.
*/
class TableHandles extends HandleGroup {
	public readonly name = 'table';

	// the objectKind of a table
	private static readonly tableKind = 35;

	// the kinds of its handles
	public static readonly columnSelect = 'tableColumnSelect';
	public static readonly rowSelect = 'tableRowSelect';
	public static readonly columnBorder = 'tableColumnBorder';
	public static readonly rowBorder = 'tableRowBorder';
	public static readonly insertColumn = 'tableInsertColumn';
	public static readonly insertRow = 'tableInsertRow';
	public static readonly anchor = 'tableAnchor';

	// sizes in CSS pixels, as the markers of the tiled view have them; a border marker takes the
	// size its class gives it
	private static readonly selectSize = 12;
	public static readonly borderSizeByDefault = 32;
	private static readonly insertWidth = 16;
	private static readonly plusSize = 16;
	private static readonly anchorSize = 20;
	// the air between the eight and the markers, and between two rows of markers
	private static readonly gap = 4;

	/// The rows and columns of the selected table, where the selection is one table.
	private data(): any {
		const objects = this.selection.selectedObjects();
		if (
			objects.length !== 1 ||
			RenderGeometrySection.objectOf(objects[0])?.objectKind !==
				TableHandles.tableKind
		)
			return undefined;
		return ClassicTableContext.data();
	}

	public isShown(): boolean {
		return RenderManager.isVectorRendering() && !!this.data();
	}

	// a length in CSS pixels, in twips
	private static twips(cssPixels: number): number {
		return cssPixels * app.dpiScale * app.pixelsToTwips;
	}

	// how far out of the table the eight reach, in twips, with the air beyond them
	private static clear(): number {
		return (
			(HandleLook.distanceTheEightMoveOut() +
				HandleLook.widthOf('framing') * 0.5) *
				app.pixelsToTwips +
			TableHandles.twips(TableHandles.gap)
		);
	}

	// how far out of the table the middle of a select square and of a border marker stand
	private static selectOut(): number {
		return (
			TableHandles.clear() + TableHandles.twips(TableHandles.selectSize * 0.5)
		);
	}

	// the size of a border marker, in CSS pixels
	public static borderSize(): number {
		return (
			HandleLook.markerLook('table-column-resize-marker').width ||
			TableHandles.borderSizeByDefault
		);
	}

	private static borderOut(): number {
		return (
			TableHandles.clear() +
			TableHandles.twips(
				TableHandles.selectSize +
					TableHandles.gap +
					TableHandles.borderSize() * 0.5,
			)
		);
	}

	private static insertOut(): number {
		return (
			TableHandles.clear() + TableHandles.twips(TableHandles.insertWidth * 0.5)
		);
	}

	public handles(): SelectionHandle[] {
		const data = this.data();
		if (!data || !RenderManager.isVectorRendering()) return [];

		const handles: SelectionHandle[] = [];
		// the reach is given as half the width and half the height, in twips
		const add = (
			name: string,
			kind: string,
			x: number,
			y: number,
			reach?: cool.Point,
		) =>
			handles.push({
				name: 'table.' + name,
				kind: kind,
				pointer: '0',
				point: new cool.Point(x, y).round(),
				reach: reach
					? new cool.Point(
							reach.x * app.twipsToPixels,
							reach.y * app.twipsToPixels,
						)
					: undefined,
			});

		const table = ClassicTableContext.layout(data);
		const select = TableHandles.selectOut();
		const border = TableHandles.borderOut();

		for (let at = 0; at + 1 < table.columns.length; ++at)
			add(
				'columnSelect.' + at,
				TableHandles.columnSelect,
				(table.columns[at] + table.columns[at + 1]) * 0.5,
				table.top - select,
				// the whole strip above the column, as in the tiled view
				new cool.Point(
					(table.columns[at + 1] - table.columns[at]) * 0.5,
					TableHandles.twips(TableHandles.selectSize),
				),
			);
		for (let at = 0; at + 1 < table.rows.length; ++at)
			add(
				'rowSelect.' + at,
				TableHandles.rowSelect,
				table.left - select,
				(table.rows[at] + table.rows[at + 1]) * 0.5,
				new cool.Point(
					TableHandles.twips(TableHandles.selectSize),
					(table.rows[at + 1] - table.rows[at]) * 0.5,
				),
			);

		// the left border of a table in a presentation moves with the table, as in the tiled
		// view, so it has no marker
		const marker = TableHandles.twips(TableHandles.borderSize() * 0.5);
		const markerReach = new cool.Point(marker, marker);
		table.columns.forEach((x: number, at: number) => {
			if (at > 0)
				add(
					'columnBorder.' + at,
					TableHandles.columnBorder,
					x,
					table.top - border,
					markerReach,
				);
		});
		// the top border of the rows moves with the table, as in the tiled view
		table.rows.forEach((y: number, at: number) => {
			if (at > 0)
				add(
					'rowBorder.' + at,
					TableHandles.rowBorder,
					table.left - border,
					y,
					markerReach,
				);
		});

		const insert = TableHandles.insertOut();
		add(
			'insertColumn',
			TableHandles.insertColumn,
			table.right + insert,
			(table.top + table.bottom) * 0.5,
			// the whole bar
			new cool.Point(
				TableHandles.twips(TableHandles.insertWidth * 0.5),
				(table.bottom - table.top) * 0.5,
			),
		);
		add(
			'insertRow',
			TableHandles.insertRow,
			(table.left + table.right) * 0.5,
			table.bottom + insert,
			new cool.Point(
				(table.right - table.left) * 0.5,
				TableHandles.twips(TableHandles.insertWidth * 0.5),
			),
		);

		const anchor = TableHandles.twips(TableHandles.anchorSize * 0.5);
		add(
			'anchor',
			TableHandles.anchor,
			table.left - border,
			table.top - border,
			new cool.Point(anchor, anchor),
		);
		return handles;
	}

	/// The markers stand outside the table, inside the band of a group the table stands in.
	public extendsTheBand(): boolean {
		return true;
	}

	/// A click on a marker is its own, the engine is not asked what lies under it.
	public passesTheClickOn(): boolean {
		return false;
	}

	/*
		A click on a select square selects the cells of its column or row alone; a click on the
		one selected alone takes the selection away again. With Shift
		only that column or row changes: next to the selection it joins it, at its edge it
		leaves it, as cells are selected as one rectangle. A click on a bar adds a column or a
		row after the one the cursor stands in.
	*/
	public letGoWhereItWasPressed(
		handle: SelectionHandle,
		event?: MouseEvent,
	): void {
		this.act(handle, event?.shiftKey === true);
	}

	/// Return and Space do what a click does.
	public pressByKey(handle: SelectionHandle, event: KeyboardEvent): boolean {
		if (event.key !== 'Enter' && event.key !== ' ') return false;
		this.act(handle, event.shiftKey);
		return true;
	}

	private act(handle: SelectionHandle, extending: boolean): void {
		if (!this.data()) return;
		const at = Number(handle.name.split('.')[2]);

		const isLine =
			handle.kind === TableHandles.columnSelect ||
			handle.kind === TableHandles.rowSelect;
		if (isLine) {
			// the cells are the table context's
			const context = Interaction.context();
			if (context instanceof ClassicTableContext)
				context.selectLine(
					handle.kind === TableHandles.columnSelect,
					at,
					extending,
				);
			return;
		}

		if (handle.kind === TableHandles.insertColumn)
			app.socket.sendMessage('uno .uno:InsertColumnsAfter');
		else if (handle.kind === TableHandles.insertRow)
			app.socket.sendMessage('uno .uno:InsertRowsAfter');
	}

	/*
		The arrow keys along a border marker move its border a step, as a drag does, kept between
		the borders beside it: left and right for a border between columns, up and down for one
		between rows. Answers where the marker lands.
	*/
	private moveBorderByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		const data = this.data();
		if (!data) return null;
		const columns = handle.kind === TableHandles.columnBorder;
		const along = columns ? towards.x : towards.y;
		if (!along) return null;

		const table = ClassicTableContext.layout(data);
		const lines = columns ? table.columns : table.rows;
		const at = Number(handle.name.split('.')[2]);
		const limits = TableBorderDrag.limits(lines, at);
		const place = Math.min(
			limits.high,
			Math.max(limits.low, lines[at] + along * HandleTravel.stepFor(event)),
		);
		TableBorderDrag.moveBorder(
			columns,
			at,
			at === lines.length - 1,
			Math.round(place - lines[at]),
		);
		return columns
			? new cool.Point(place, handle.point.y)
			: new cool.Point(handle.point.x, place);
	}

	/// The arrow keys on the anchor move the table, as they move a selection.
	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		if (
			handle.kind === TableHandles.columnBorder ||
			handle.kind === TableHandles.rowBorder
		)
			return this.moveBorderByKey(handle, towards, event);
		if (handle.kind !== TableHandles.anchor) return null;
		this.selection.moveByKey(towards, event);
		const step = HandleTravel.stepFor(event);
		return new cool.Point(
			handle.point.x + towards.x * step,
			handle.point.y + towards.y * step,
		);
	}

	/// A drag of the anchor moves the table, as a drag of the table itself does; a drag of a
	/// border marker moves that border.
	public interactionFor(
		handle: SelectionHandle,
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		if (
			handle.kind === TableHandles.columnBorder ||
			handle.kind === TableHandles.rowBorder
		) {
			const data = this.data();
			return data
				? new TableBorderDrag(
						this.selection,
						handle,
						ClassicTableContext.layout(data),
					)
				: null;
		}
		if (
			handle.kind !== TableHandles.anchor ||
			!ObjectHandles.canBeMoved(this.selection.selectedObjects())
		)
			return null;
		return new MovingInteraction(this.selection, at);
	}

	/*
		Draws the picture of the marker and lays no shape: the picture is the whole look of the
		marker, as in the tiled view. Under the mouse and under the keyboard it grows as the eight
		do, so the one the keyboard is on blinks as they do; it then shows the picture the tiled
		view shows under the mouse.
	*/
	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		const t = TableHandles;
		// under the mouse and the keyboard a marker grows; a border marker then shows the drag
		// marker, the plus of a bar stays as it is, the others show their picture for the mouse
		const draw = (
			className: string,
			size: number,
			underMouse: 'lit' | 'drag' | 'usual' = 'lit',
		) =>
			HandleLook.drawMarker(context, className, at, size, grown, {
				underMouse: underMouse,
			});

		switch (handle.kind) {
			case t.columnSelect:
			case t.rowSelect:
				draw('table-select-marker', t.selectSize);
				break;
			case t.columnBorder:
				draw('table-column-resize-marker', t.borderSize(), 'drag');
				break;
			case t.rowBorder:
				draw('table-row-resize-marker', t.borderSize(), 'drag');
				break;
			case t.insertColumn:
			case t.insertRow:
				draw('table-add-col-row-marker', t.plusSize, 'usual');
				break;
			case t.anchor:
				draw('table-move-marker', t.anchorSize);
				break;
		}
		return 'transparent';
	}

	/*
		Under the markers: the dashed line along the select squares of the columns and of the
		rows, and the two bars that add a column and a row, as the tiled view draws them.
	*/
	public paint(section: SelectionSection, _shown: SelectionHandle[]): void {
		const data = this.data();
		if (!data || !this.isShown()) return;

		const t = TableHandles;
		const table = ClassicTableContext.layout(data);
		const select = t.selectOut();
		const corner = (x: number, y: number) => new cool.SimplePoint(x, y);
		const context = section.context;

		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);

		// the border being dragged, across the table where it would go, as the tiled view
		// shows it
		const dragged = TableBorderDrag.running;
		if (dragged) {
			const from = dragged.columns
				? corner(dragged.place, table.top)
				: corner(table.left, dragged.place);
			const to = dragged.columns
				? corner(dragged.place, table.bottom)
				: corner(table.right, dragged.place);
			context.strokeStyle = '#3388FF';
			context.lineWidth = 2 * app.dpiScale;
			context.beginPath();
			context.moveTo(from.vX, from.vY);
			context.lineTo(to.vX, to.vY);
			context.stroke();
		}

		context.strokeStyle = 'lightgray';
		context.lineWidth = app.dpiScale;
		context.setLineDash([3 * app.dpiScale, 3 * app.dpiScale]);
		context.beginPath();
		const columnsFrom = corner(table.left, table.top - select);
		const columnsTo = corner(table.right, table.top - select);
		context.moveTo(columnsFrom.vX, columnsFrom.vY);
		context.lineTo(columnsTo.vX, columnsTo.vY);
		const rowsFrom = corner(table.left - select, table.top);
		const rowsTo = corner(table.left - select, table.bottom);
		context.moveTo(rowsFrom.vX, rowsFrom.vY);
		context.lineTo(rowsTo.vX, rowsTo.vY);
		context.stroke();
		context.setLineDash([]);

		// a bar under the mouse takes the darker colour, as in the tiled view
		const look = getComputedStyle(document.body);
		const colour = (name: string) =>
			look.getPropertyValue(name).trim() || 'lightgray';
		const hovered = section.hoveredHandle();
		const bar = t.twips(t.insertWidth);
		const out = t.clear();
		const radius = 4 * app.dpiScale;
		for (const [name, x, y, width, height] of [
			[
				'table.insertColumn',
				table.right + out,
				table.top,
				bar,
				table.bottom - table.top,
			],
			[
				'table.insertRow',
				table.left,
				table.bottom + out,
				table.right - table.left,
				bar,
			],
		] as [string, number, number, number, number][]) {
			context.fillStyle = colour(
				hovered === name
					? '--color-insert-marker-background-dark'
					: '--color-insert-marker-background',
			);
			// the bar grows as a handle does under the mouse and under the keyboard, where it
			// blinks
			const size = ShapeHandlesSection.handleSize();
			const grown = section.travel.showsAsActive(name)
				? size / 3
				: hovered === name
					? size / 8
					: 0;
			const from = corner(x, y);
			const to = corner(x + width, y + height);
			context.beginPath();
			context.roundRect(
				from.vX - grown,
				from.vY - grown,
				to.vX - from.vX + 2 * grown,
				to.vY - from.vY + 2 * grown,
				radius,
			);
			context.fill();
		}

		context.restore();
	}
}
