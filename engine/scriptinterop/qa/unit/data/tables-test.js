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

    // The partly selected text of a table cell has the cell's paragraph, the cell, its row, its
    // table and the body as its ancestors:
    const partial = doc.setSelection(
        doc.newRange().addElement(empty.getRow(0).getCell(1).editAsText(), 0, 0).build())
        .getSelection().getRangeElements();
    console.assert(partial.length === 1);
    const text = partial[0].getElement();
    console.assert(text.getType() === DocumentApp.ElementType.TEXT);
    console.assert(text.getParent().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(
        text.getParent().getParent().getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(
        text.getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.TABLE_ROW);
    console.assert(
        text.getParent().getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.TABLE);
    console.assert(
        text.getParent().getParent().getParent().getParent().getParent().getType()
            === DocumentApp.ElementType.BODY_SECTION);

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

    // A range of the whole table of child 1 between partly selected texts of children 0 and 2, and
    // one of all of child 0 and that table, are selected as such:
    const across = doc.setSelection(
        doc.newRange().addElement(body.getChild(0).editAsText(), 3, 5).addElement(empty)
            .addElement(body.getChild(2).editAsText(), 0, 2).build())
        .getSelection().getRangeElements();
    console.assert(across.length === 3);
    console.assert(across[0].getElement().getType() === DocumentApp.ElementType.TEXT);
    console.assert(across[0].getStartOffset() === 3);
    console.assert(across[0].getEndOffsetInclusive() === 5);
    console.assert(across[1].getElement().getType() === DocumentApp.ElementType.TABLE);
    console.assert(across[1].isPartial() === false);
    console.assert(
        across[1].getElement().getParent().getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(across[2].getElement().getType() === DocumentApp.ElementType.TEXT);
    console.assert(across[2].getStartOffset() === 0);
    console.assert(across[2].getEndOffsetInclusive() === 2);
    console.assert(doc.getCursor() === null);
    const upTo = doc.setSelection(
        doc.newRange().addElement(body.getChild(0)).addElement(empty).build())
        .getSelection().getRangeElements();
    console.assert(upTo.length === 2);
    console.assert(upTo[0].getElement().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(upTo[1].getElement().getType() === DocumentApp.ElementType.TABLE);
    // A selected row is selected as that row (as its cells in Writer, whose selections do not tell
    // a row from its cells):
    const rowSelected = doc.setSelection(doc.newRange().addElement(empty.getRow(1)).build())
        .getSelection().getRangeElements();
    if (globalThis.cool) {
        console.assert(rowSelected.length === 2);
        console.assert(
            rowSelected[0].getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
    } else {
        console.assert(rowSelected.length === 1);
        console.assert(rowSelected[0].getElement().getType() === DocumentApp.ElementType.TABLE_ROW);
    }

    // A range is made up of the elements added to it, in document order, with an added row as a
    // TABLE_ROW, an added table as a TABLE, the whole text of a cell as that TABLE_CELL, and a
    // paragraph that lies in an added cell left out:
    const row = doc.newRange().addElement(empty.getRow(1)).build().getRangeElements();
    console.assert(row.length === 1);
    console.assert(row[0].getElement().getType() === DocumentApp.ElementType.TABLE_ROW);
    const whole = doc.newRange().addElement(empty).build().getRangeElements();
    console.assert(whole.length === 1);
    console.assert(whole[0].getElement().getType() === DocumentApp.ElementType.TABLE);
    const cellText = doc.newRange().addElement(empty.getRow(0).getCell(1).editAsText()).build()
        .getRangeElements();
    console.assert(cellText.length === 1);
    console.assert(cellText[0].getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
    const cellAndParagraph = doc.newRange().addElement(empty.getRow(0).getCell(1).getChild(0))
        .addElement(empty.getRow(0).getCell(1)).build().getRangeElements();
    console.assert(cellAndParagraph.length === 1);
    console.assert(
        cellAndParagraph[0].getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
    // A paragraph added twice, or together with a part of its text, is in a range only once:
    const twice = doc.newRange().addElement(body.getChild(0)).addElement(body.getChild(0)).build()
        .getRangeElements();
    console.assert(twice.length === 1);
    const withPart = doc.newRange().addElement(body.getChild(0))
        .addElement(body.getChild(0).editAsText(), 1, 2).build().getRangeElements();
    console.assert(withPart.length === 1);
    console.assert(withPart[0].getElement().getType() === DocumentApp.ElementType.PARAGRAPH);
    // A text added with offsets stays a part of its paragraph also when it covers all of it, unless
    // that paragraph is added too:
    const allOfText = doc.newRange().addElement(body.getChild(0).editAsText(), 0, 5).build()
        .getRangeElements();
    console.assert(allOfText.length === 1);
    console.assert(allOfText[0].getElement().getType() === DocumentApp.ElementType.TEXT);
    console.assert(allOfText[0].isPartial());
    console.assert(allOfText[0].getEndOffsetInclusive() === 5);
    const textThenParagraph = doc.newRange().addElement(body.getChild(0).editAsText(), 0, 5)
        .addElement(body.getChild(0)).build().getRangeElements();
    console.assert(textThenParagraph.length === 1);
    console.assert(
        textThenParagraph[0].getElement().getType() === DocumentApp.ElementType.PARAGRAPH);
    // The text of a table and of a row stands for that element:
    if (globalThis.cool) {
        const tableText = doc.newRange().addElement(empty.editAsText()).build().getRangeElements();
        console.assert(tableText.length === 1);
        console.assert(tableText[0].getElement().getType() === DocumentApp.ElementType.TABLE);
        const rowText = doc.newRange().addElement(empty.getRow(0).editAsText()).build()
            .getRangeElements();
        console.assert(rowText.length === 1);
        console.assert(rowText[0].getElement().getType() === DocumentApp.ElementType.TABLE_ROW);
    }
    const sorted = doc.newRange().addElement(empty.getRow(0).getCell(0))
        .addElement(body.getChild(2)).addElement(body.getChild(0)).build().getRangeElements();
    console.assert(sorted.length === 3);
    console.assert(sorted[0].getElement().getText() === 'Before');
    console.assert(sorted[1].getElement().getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(sorted[2].getElement().getText() === 'Between');
    // Two paragraphs with the table between them stay two elements, which Writer cannot select
    // together:
    const apart = doc.newRange().addElement(body.getChild(0)).addElement(body.getChild(2)).build();
    console.assert(apart.getRangeElements().length === 2);
    if (globalThis.cool) {
        let threw = false;
        try {
            doc.setSelection(apart);
        } catch (e) {
            threw = true;
        }
        console.assert(threw);
    }

    // A table cell that has already been read sees the changes made since through another object
    // for the same cell, here clearing it, which leaves one empty paragraph:
    console.assert(cell.getNumChildren() > 1);
    nested.getRow(0).getCell(0).clear();
    console.assert(cell.getNumChildren() === 1);

    // With a table added at the end of the body through the UNO model of the document, which puts
    // an empty paragraph after it, the last paragraph before it and that table can be selected:
    if (globalThis.cool) {
        const model = cool.getActiveUnoModel();
        const last = model.createInstance('com.sun.star.text.TextTable');
        last.initialize(1, 1);
        model.getText().insertTextContent(model.getText().getEnd(), last, false);
        const n = body.getNumChildren();
        console.assert(body.getChild(n - 2).getType() === DocumentApp.ElementType.TABLE);
        const withLast = doc.setSelection(
            doc.newRange().addElement(body.getChild(n - 3)).addElement(body.getChild(n - 2))
                .build())
            .getSelection().getRangeElements();
        console.assert(withLast.length === 2);
        console.assert(withLast[1].getElement().getType() === DocumentApp.ElementType.TABLE);
    }
}
