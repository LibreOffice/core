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

//TODO: GAS appears to return null when text properties are not set explicitly:
function checkEqual(actual, expected) {
    return actual === expected || (!globalThis.cool && actual === null);
}

function documentTest() {
    const body = DocumentApp.getActiveDocument().getBody();
    console.assert(body.getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(body.getText().length > 0);
    console.assert(body.getNumChildren() === 5);

    // Paragraph 0 is a bold "Bold", italic "Italic" and plain "Plain" concatenated:
    const p0 = body.getChild(0);
    console.assert(p0.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(p0.getText() === 'BoldItalicPlain');
    console.assert(p0.isLeftToRight() === true);
    const t0 = p0.editAsText();
    console.assert(t0.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(t0.getText() === 'BoldItalicPlain');
    console.assert(t0.isBold(0) === true);
    console.assert(checkEqual(t0.isBold(4), false));
    console.assert(checkEqual(t0.isItalic(0), false));
    console.assert(t0.isItalic(4) === true);
    console.assert(checkEqual(t0.isUnderline(0), false));
    console.assert(checkEqual(t0.isStrikethrough(0), false));
    console.assert(checkEqual(t0.getTextAlignment(0), DocumentApp.TextAlignment.NORMAL));
    console.assert(checkEqual(t0.getLinkUrl(0), ''));
    console.assert(t0.getFontFamily(0).length > 0);
    const idx = t0.getTextAttributeIndices();
    console.assert(idx.length >= 1);
    console.assert(idx[0] === 0);

    // Paragraph 1 is an underlined, struck, superscript and plain run:
    const t1 = body.getChild(1).editAsText();
    console.assert(t1.getText() === 'UnderStrikeSuperPlain');
    console.assert(t1.isUnderline(0) === true);
    console.assert(checkEqual(t1.isUnderline(5), false));
    console.assert(t1.isStrikethrough(5) === true);
    console.assert(checkEqual(t1.isStrikethrough(11), false));
    console.assert(t1.getTextAlignment(11) === DocumentApp.TextAlignment.SUPERSCRIPT);
    console.assert(checkEqual(t1.getTextAlignment(16), DocumentApp.TextAlignment.NORMAL));

    // Paragraph 2 concatenates a plain "Third" run and a hyperlinked "Link" run:
    const t2 = body.getChild(2).editAsText();
    console.assert(t2.getText() === 'ThirdLink');
    console.assert(checkEqual(t2.getLinkUrl(0), ''));
    console.assert(t2.getLinkUrl(5) === 'https://example.com');

    // Child 3 is a 2x2 table with cells A1, B1, A2, B2:
    const table = body.getChild(3);
    console.assert(table.getType() === DocumentApp.ElementType.TABLE);
    console.assert(table.getNumRows() === 2);
    console.assert(table.getNumChildren() === 2);
    console.assert(table.getChild(0).getType() === DocumentApp.ElementType.TABLE_ROW);
    const row0 = table.getRow(0);
    console.assert(row0.getNumCells() === 2);
    console.assert(row0.getNumChildren() === 2);
    const cell00 = row0.getCell(0);
    console.assert(cell00.getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(cell00.getText() === 'A1');
    console.assert(cell00.getNumChildren() >= 1);
    console.assert(cell00.getChild(0).getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(row0.getCell(1).getText() === 'B1');
    console.assert(table.getRow(1).getCell(0).getText() === 'A2');
    console.assert(table.getRow(1).getCell(1).getText() === 'B2');

    // Paragraph 4 anchors a footnote whose only paragraph is "Note":
    console.assert(body.getChild(4).getText().substring(0, 8) === 'Trailing');
    const fns = DocumentApp.getActiveDocument().getFootnotes();
    console.assert(fns.length === 1);
    const noteContents = fns[0].getFootnoteContents();
    console.assert(noteContents.getNumChildren() >= 1);
    console.assert(noteContents.getChild(0).getText() === 'Note');

    // Build a selection spanning all of paragraph 0 plus the first three characters of paragraph 1,
    // set it on the document, and verify the per-paragraph range split:
    const doc = DocumentApp.getActiveDocument();
    const range = doc.newRange()
        .addElement(body.getChild(0))
        .addElement(body.getChild(1).editAsText(), 0, 2)
        .build();
    doc.setSelection(range);
    const ranges = doc.getSelection().getRangeElements();
    console.assert(ranges.length === 2);
    console.assert(ranges[0].isPartial() === false);
    console.assert(ranges[0].getStartOffset() === -1);
    console.assert(ranges[0].getEndOffsetInclusive() === -1);
    console.assert(ranges[0].getElement().getText() === 'BoldItalicPlain');
    console.assert(ranges[1].isPartial() === true);
    console.assert(ranges[1].getStartOffset() === 0);
    console.assert(ranges[1].getEndOffsetInclusive() === 2);
    console.assert(ranges[1].getElement().getText() === 'UnderStrikeSuperPlain');

    // Append a paragraph and a list item at the end:
    const appended = body.appendParagraph('Appended');
    console.assert(appended.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(appended.getText() === 'Appended');
    console.assert(body.getNumChildren() === 6);

    // Exercise the setBold overload group:
    appended.editAsText().setBold(true).setBold(0, 3, false);
    console.assert(appended.editAsText().isBold(0) === false);
    console.assert(appended.editAsText().isBold(4) === true);

    const listItem = body.appendListItem('Item');
    console.assert(listItem.getType() === DocumentApp.ElementType.LIST_ITEM);
    console.assert(listItem.getText() === 'Item');
    console.assert(body.getNumChildren() === 7);
}
