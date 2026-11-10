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

/// The borders of a table in twips: the outer edges, and the borders between columns and between
/// rows, the outer ones included.
interface TableLayout {
	left: number;
	top: number;
	right: number;
	bottom: number;
	columns: number[];
	rows: number[];
}

/*
	A table one stands at: one table selected, as an object or with some of its cells. For
	everything else it acts as the place the table stands at does, the page or the group holding
	it. It holds what is done with the cells: which of them are selected, and selecting whole
	columns and rows. The client holds the cell selection as it holds the selection of objects: a
	change shows at once and goes to the engine without waiting, and the engine reports its own
	back, which is what counts where the two differ.
*/
class ClassicTableContext extends ClassicContext {
	private table: number;

	constructor(table: number, group: number | undefined) {
		super(group);
		this.table = table;
	}

	/// The rows and columns of the selected table, as the server sends them. Absent while it has
	/// sent none.
	public static data(): any {
		return app.activeDocument?.tableMiddleware?.tableData();
	}

	/// Where the borders of the table lie, from its rows and columns.
	public static layout(data: any): TableLayout {
		const across = data.columns;
		const down = data.rows;
		const columns = [across.left]
			.concat(across.entries.map((entry: any) => entry.position))
			.concat([across.right])
			.map((at: number) => across.tableOffset + at);
		const rows = [down.left]
			.concat(down.entries.map((entry: any) => entry.position))
			.concat([down.right])
			.map((at: number) => down.tableOffset + at);
		return {
			left: columns[0],
			right: columns[columns.length - 1],
			top: rows[0],
			bottom: rows[rows.length - 1],
			columns: columns,
			rows: rows,
		};
	}

	/*
		The columns, or the rows, the cells selected now cover whole, the first and the last of
		them: where the selection reaches across the whole table one way and covers whole columns
		or rows the other way. Nothing where no cell is selected or the selection is other. Cells
		of a table are selected as one rectangle.
	*/
	private static selectedLines(
		columns: boolean,
		table: TableLayout,
	): { from: number; to: number } | null {
		const area = app.map._docLayer._cellSelectionArea;
		if (!area) return null;
		// how far a border may lie from the edge of the selection, in twips
		const near = 20;
		const across = columns
			? area.y1 <= table.top + near && area.y2 >= table.bottom - near
			: area.x1 <= table.left + near && area.x2 >= table.right - near;
		if (!across) return null;

		const lines = columns ? table.columns : table.rows;
		const low = columns ? area.x1 : area.y1;
		const high = columns ? area.x2 : area.y2;
		const covered: number[] = [];
		for (let at = 0; at + 1 < lines.length; ++at)
			if (lines[at] >= low - near && lines[at + 1] <= high + near)
				covered.push(at);
		return covered.length
			? { from: covered[0], to: covered[covered.length - 1] }
			: null;
	}

	/*
		Selects the cells of that column, or of that row, alone; where that one is selected alone
		already, no cell is selected any more. With extending only that column or row changes:
		next to the selection it joins it, at its edge it leaves it, the last one leaves the
		selection empty, and one further away changes nothing, as cells are selected as one
		rectangle.
	*/
	public selectLine(columns: boolean, at: number, extending: boolean): void {
		const data = ClassicTableContext.data();
		if (!data) return;
		const table = ClassicTableContext.layout(data);
		const standing = ClassicTableContext.selectedLines(columns, table);

		let from = at;
		let to = at;
		if (standing && !extending) {
			if (standing.from === at && standing.to === at) {
				this.selectCells(table, null);
				return;
			}
		} else if (standing) {
			from = standing.from;
			to = standing.to;
			if (at === from - 1) from = at;
			else if (at === to + 1) to = at;
			else if (at === from && from < to) ++from;
			else if (at === to && from < to) --to;
			else if (at === from && at === to) {
				this.selectCells(table, null);
				return;
			} else return;
		}

		this.selectCells(
			table,
			columns
				? [from, 0, to, table.rows.length - 2]
				: [0, from, table.columns.length - 2, to],
		);
	}

	/// The cell at that point of the page, in twips, as its column and row; nothing outside the
	/// table.
	public static cellAt(
		table: TableLayout,
		x: number,
		y: number,
	): { column: number; row: number } | null {
		const within = (lines: number[], at: number) => {
			for (let line = 0; line + 1 < lines.length; ++line)
				if (at >= lines[line] && at <= lines[line + 1]) return line;
			return -1;
		};
		const column = within(table.columns, x);
		const row = within(table.rows, y);
		return column < 0 || row < 0 ? null : { column: column, row: row };
	}

	/// The cell the text cursor stands in, where a cell of the table is being edited.
	public static editedCell(
		table: TableLayout,
	): { column: number; row: number } | null {
		const cursor = app.file.textCursor;
		if (!cursor.visible || !cursor.rectangle) return null;
		return ClassicTableContext.cellAt(
			table,
			cursor.rectangle.x1 + cursor.rectangle.width * 0.5,
			cursor.rectangle.y1 + cursor.rectangle.height * 0.5,
		);
	}

	/*
		Extends the selection of cells to the cell at that point, in twips: from the cell being
		edited, or from the cell the selection started at. Answers whether it did, which it does
		not inside the edited cell, where Shift selects text.
	*/
	public extendCellsTo(x: number, y: number): boolean {
		const data = ClassicTableContext.data();
		if (!data) return false;
		const table = ClassicTableContext.layout(data);
		const to = ClassicTableContext.cellAt(table, x, y);
		const edited = ClassicTableContext.editedCell(table);
		const from =
			edited ??
			(app.map._docLayer._cellSelectionArea
				? ClassicTableContext.anchor
				: null);
		if (!from || !to) return false;
		if (edited && edited.column === to.column && edited.row === to.row)
			return false;
		this.selectCellRange(from, to);
		return true;
	}

	/*
		The cell a selection of cells by the mouse started at, which Shift and a click extend it
		from; nothing while no such selection stands.
	*/
	public static anchor: { column: number; row: number } | null = null;

	/// Selects the cells from one cell to another, whichever way round they lie; the first is
	/// where it is extended from with Shift.
	public selectCellRange(
		from: { column: number; row: number },
		to: { column: number; row: number },
	): void {
		const data = ClassicTableContext.data();
		if (!data) return;
		ClassicTableContext.anchor = from;
		this.selectCells(ClassicTableContext.layout(data), [
			Math.min(from.column, to.column),
			Math.min(from.row, to.row),
			Math.max(from.column, to.column),
			Math.max(from.row, to.row),
		]);
	}

	/*
		Selects the cells from the first to the last, as [first column, first row, last column,
		last row], or none: it shows the selection at once, as the engine reports one, and tells
		the engine the range.
	*/
	private selectCells(table: TableLayout, cells: number[] | null): void {
		const docLayer = app.map._docLayer;
		if (!cells) {
			docLayer._cellSelectionArea = null;
			docLayer.showTextSelection([]);
		} else {
			const [firstColumn, firstRow, lastColumn, lastRow] = cells;
			const x = table.columns[firstColumn];
			const y = table.rows[firstRow];
			const width = table.columns[lastColumn + 1] - x;
			const height = table.rows[lastRow + 1] - y;
			docLayer._cellSelectionArea = new cool.SimpleRectangle(
				x,
				y,
				width,
				height,
			);
			docLayer.showTextSelection([[x, y, width, height]]);
		}

		const [firstColumn, firstRow, lastColumn, lastRow] = cells ?? [
			-1, -1, -1, -1,
		];
		app.socket.sendMessage(
			'selectcells id=' +
				this.table +
				' firstcolumn=' +
				firstColumn +
				' firstrow=' +
				firstRow +
				' lastcolumn=' +
				lastColumn +
				' lastrow=' +
				lastRow,
		);
	}
}
