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
	A drag in the cells of the selected table, which selects cells as in the office: while the
	mouse stays in the cell the press landed in nothing happens, and once it reaches another cell
	the cells from the first to the one under the mouse are selected, through the table context.
*/
class TableCellsDrag extends SelectionInteraction {
	private context: ClassicTableContext;
	// the cell the press landed in, and the one the mouse reached last
	private from: { column: number; row: number } | null;
	private to: { column: number; row: number } | null = null;

	constructor(
		selection: SelectionSection,
		at: cool.SimplePoint,
		context: ClassicTableContext,
	) {
		super(selection);
		this.context = context;
		this.from = TableCellsDrag.cellAt(at);
	}

	private static cellAt(
		at: cool.SimplePoint,
	): { column: number; row: number } | null {
		const data = ClassicTableContext.data();
		return data
			? ClassicTableContext.cellAt(ClassicTableContext.layout(data), at.x, at.y)
			: null;
	}

	public move(to: cool.SimplePoint): void {
		const now = TableCellsDrag.cellAt(to);
		if (!this.from || !now) return;
		const same = (one: { column: number; row: number } | null) =>
			!!one && one.column === now.column && one.row === now.row;
		if (same(this.to) || (!this.to && same(this.from))) return;
		this.to = now;
		this.context.selectCellRange(this.from, now);
	}

	public finish(to: cool.SimplePoint): void {
		this.move(to);
	}
}
