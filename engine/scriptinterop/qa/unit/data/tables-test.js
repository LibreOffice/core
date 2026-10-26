/* -*- fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// GAS doesn't have console.assert:
if (!globalThis.cool) {
    console.assert = console.assert || (cond => { if (!cond) throw new Error('failed: ' + cond); });
}

function test() {
    const body = DocumentApp.getActiveDocument().getBody();

    // Child 1 is a 2x2 table whose cells are empty, apart from B1, and the text of a row or of the
    // table separates even empty cells and rows with a newline:
    const empty = body.getChild(1);
    console.assert(empty.getRow(0).getText() === '\nB1');
    console.assert(empty.getRow(1).getText() === '\n');
    console.assert(empty.getText() === '\nB1\n\n');

    // Child 3 is a 2x2 table whose first column is one vertically merged cell, holding "Merged" and
    // a footnote reference, and the covered cell below it has no text:
    const merged = body.getChild(3);
    console.assert(merged.getText() === 'Merged\nB1\n\nB2');
    console.assert(merged.getRow(0).getCell(0).getText() === 'Merged');
    console.assert(merged.getRow(1).getCell(0).getText() === '');
    console.assert(merged.getRow(1).getCell(1).getText() === 'B2');

    // Cell A1 of child 5 starts with a nested table (on Google only after an empty paragraph, which
    // the conversion puts there) and ends with an empty paragraph:
    const nested = body.getChild(5);
    const cell = nested.getRow(0).getCell(0);
    const lead = globalThis.cool ? '' : '\n';
    if (globalThis.cool) {
        console.assert(cell.getChild(0).getType() === DocumentApp.ElementType.TABLE);
    }
    console.assert(cell.getText() === lead + 'N1\nN2\n');
    console.assert(nested.getRow(0).getText() === lead + 'N1\nN2\n\nB1');

    console.assert(
        body.getText() === 'Before\n\nB1\n\n\nBetween\nMerged\nB1\n\nB2\nBetween\n' + lead
            + 'N1\nN2\n\nB1\nAfter');

    // With cells A1 to B2 of child 1 selected, in any order, the selection is each of these cells
    // as a whole, row by row, and there is no cursor:
    const doc = DocumentApp.getActiveDocument();
    doc.setSelection(
        doc.newRange().addElement(empty.getRow(1).getCell(1))
            .addElement(empty.getRow(0).getCell(0)).addElement(empty.getRow(1).getCell(0))
            .addElement(empty.getRow(0).getCell(1)).build());
    const cells = doc.getSelection().getRangeElements();
    console.assert(cells.length === 4);
    for (const range of cells) {
        console.assert(range.getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
        console.assert(range.isPartial() === false);
        console.assert(range.getStartOffset() === -1);
        console.assert(range.getEndOffsetInclusive() === -1);
    }
    console.assert(cells.map(range => range.getElement().getText()).join('|') === '|B1||');
    console.assert(
        cells[1].getElement().getParent().getType() === DocumentApp.ElementType.TABLE_ROW);
    console.assert(doc.getCursor() === null);
    // A single cell is selected as that whole cell, too:
    const single = doc.setSelection(doc.newRange().addElement(empty.getRow(0).getCell(1)).build())
        .getSelection().getRangeElements();
    console.assert(single.length === 1);
    console.assert(single[0].getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(single[0].getElement().getText() === 'B1');
    console.assert(single[0].isPartial() === false);
    console.assert(doc.getCursor() === null);
    // The cells of a table whose cell A1 starts with a nested table are selected as such, too:
    const outer = doc.setSelection(
        doc.newRange().addElement(nested.getRow(0).getCell(0))
            .addElement(nested.getRow(0).getCell(1)).build())
        .getSelection().getRangeElements();
    console.assert(outer.length === 2);
    console.assert(outer[0].getElement().getText() === lead + 'N1\nN2\n');
    console.assert(outer[1].getElement().getText() === 'B1');

    // A selected cell of the table nested in cell A1 of child 5 has the nested table, the cell A1
    // and the table of child 5 among its ancestors:
    const innerTable = cell.getChild(globalThis.cool ? 0 : 1);
    const inner = doc.setSelection(
        doc.newRange().addElement(innerTable.getRow(0).getCell(0)).build())
        .getSelection().getRangeElements()[0].getElement();
    console.assert(inner.getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(inner.getText() === 'N1');
    console.assert(inner.getParent().getParent().getType() === DocumentApp.ElementType.TABLE);
    console.assert(
        inner.getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.TABLE_CELL);
    console.assert(
        inner.getParent().getParent().getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.TABLE);
    console.assert(
        inner.getParent().getParent().getParent().getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.BODY_SECTION);

    // A table cell that has already been read sees the changes made since through another object
    // for the same cell, here clearing it, which leaves one empty paragraph:
    console.assert(cell.getNumChildren() > 1);
    nested.getRow(0).getCell(0).clear();
    console.assert(cell.getNumChildren() === 1);
}
