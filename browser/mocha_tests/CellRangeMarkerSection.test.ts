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

describe('CellRangeMarkerSection', function () {
	before(function () {
		// Values mirror browser/src/canvas/CanvasSectionProps.js, which the
		// mocha bundle does not carry.
		app.CSections.CellRangeMarker = {
			name: 'CellRangeMarker',
			zIndex: 9,
			processingOrder: 10,
			drawingOrder: 10,
		};
	});

	it('finds and covers the merged cells of a sheet', function () {
		const section = new CellRangeMarkerSection();

		// Check that nothing is known before the sheet's cell ranges arrive.
		nodeassert.strictEqual(section.hasSheetCellRanges(0), false);
		nodeassert.strictEqual(section.getMergedCellAt(0, 1, 1), null);

		// Store B2:C3 as the only merged cell of the first sheet.
		const mergedCell = new cool.Bounds(
			new cool.Point(1, 1),
			new cool.Point(2, 2),
		);
		section.setSheetCellRanges(0, {
			tables: [],
			sparklineGroups: [],
			mergedCells: [mergedCell],
		});

		// Check the cell it starts in, a cell it covers and a cell outside.
		nodeassert.strictEqual(section.getMergedCellAt(0, 1, 1), mergedCell);
		nodeassert.strictEqual(section.getMergedCellAt(0, 2, 2), mergedCell);
		nodeassert.strictEqual(section.getMergedCellAt(0, 3, 2), null);

		// Check that another sheet is still unknown.
		nodeassert.strictEqual(section.hasSheetCellRanges(1), false);

		// Add E3:F4 and C4:D5 as merged cells
		section.setSheetCellRanges(0, {
			tables: [],
			sparklineGroups: [],
			mergedCells: [
				mergedCell,
				new cool.Bounds(new cool.Point(4, 2), new cool.Point(5, 3)), // E3:F4
				new cool.Bounds(new cool.Point(2, 3), new cool.Point(3, 4)), // C4:D5
			],
		});

		// A7:B8 does not overlap with any merged cells - it should stay unchanged
		nodeassert.deepStrictEqual(
			section.coverMergedCells(0, [0, 6, 1, 7]), // A7:B8
			[0, 6, 1, 7],
		);

		// A1:B2 should grow over B2:C3 to become A1:C3
		nodeassert.deepStrictEqual(
			section.coverMergedCells(0, [0, 0, 1, 1]), // A1:B2
			[0, 0, 2, 2], // A1:C3
		);

		// A1:B4 should grow over B2:C3 and C4:D5 to become A1:D5
		nodeassert.deepStrictEqual(
			section.coverMergedCells(0, [0, 0, 1, 3]), // A1:B4
			[0, 0, 3, 4], // A1:D5
		);

		// A range on another sheet should stay unchanged regardless
		nodeassert.deepStrictEqual(
			section.coverMergedCells(1, [0, 0, 1, 1]),
			[0, 0, 1, 1],
		);
	});
});
