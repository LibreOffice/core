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

function documentTest() {
    const body = DocumentApp.getActiveDocument().getBody();
    console.assert(body.getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(body.getText().length > 0);
    console.assert(body.getNumChildren() === 6);

    // Paragraph 0 is a bold "Bold", italic "Italic" and plain "Plain" concatenated:
    const p0 = body.getChild(0);
    console.assert(p0.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(p0.getText() === 'BoldItalicPlain');
    console.assert(p0.isLeftToRight() === true);
    console.assert(p0.getParent().getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(p0.copy().getText() === 'BoldItalicPlain');
    console.assert(p0.getNumChildren() === 1);
    console.assert(p0.getChild(0).getType() === DocumentApp.ElementType.TEXT);
    console.assert(p0.getChild(0).getText() === 'BoldItalicPlain');
    console.assert(p0.getChild(0).getParent().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(p0.getHeading() === DocumentApp.ParagraphHeading.NORMAL);
    console.assert(p0.getAlignment() === null);
    console.assert(p0.getIndentStart() === null);
    console.assert(p0.getPreviousSibling() === null);
    console.assert(p0.getNextSibling().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(p0.getNextSibling().getText() === 'UnderStrikeSuperPlain');
    const t0 = p0.editAsText();
    console.assert(t0.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(t0.getText() === 'BoldItalicPlain');
    console.assert(t0.getParent().getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(t0.isBold(0) === true);
    console.assert(t0.isBold(4) === null);
    console.assert(t0.isItalic(0) === null);
    console.assert(t0.isItalic(4) === true);
    console.assert(t0.isUnderline(0) === null);
    console.assert(t0.isStrikethrough(0) === null);
    console.assert(t0.getTextAlignment(0) === null);
    console.assert(t0.getLinkUrl(0) === null);
    console.assert(t0.getFontFamily(0).length > 0);
    const idx = t0.getTextAttributeIndices();
    console.assert(idx.length >= 1);
    console.assert(idx[0] === 0);

    // Paragraph 1 is an underlined, struck, superscript and plain run:
    const t1 = body.getChild(1).editAsText();
    console.assert(t1.getText() === 'UnderStrikeSuperPlain');
    console.assert(t1.isUnderline(0) === true);
    console.assert(t1.isUnderline(5) === null);
    console.assert(t1.isStrikethrough(5) === true);
    console.assert(t1.isStrikethrough(11) === null);
    console.assert(t1.getTextAlignment(11) === DocumentApp.TextAlignment.SUPERSCRIPT);
    console.assert(t1.getTextAlignment(16) === null);

    // Paragraph 2 concatenates a plain "Third" run and a hyperlinked "Link" run:
    const t2 = body.getChild(2).editAsText();
    console.assert(t2.getText() === 'ThirdLink');
    console.assert(t2.getLinkUrl(0) === null);
    console.assert(t2.getLinkUrl(5) === 'https://example.com');

    // Child 3 is a 2x2 table with cells A1, B1, A2, B2:
    const table = body.getChild(3);
    console.assert(table.getType() === DocumentApp.ElementType.TABLE);
    console.assert(table.getNumRows() === 2);
    console.assert(table.getNumChildren() === 2);
    console.assert(table.getChild(0).getType() === DocumentApp.ElementType.TABLE_ROW);
    console.assert(table.getParent().getType() === DocumentApp.ElementType.BODY_SECTION);
    const row0 = table.getRow(0);
    console.assert(row0.getNumCells() === 2);
    console.assert(row0.getNumChildren() === 2);
    console.assert(row0.getParent().getType() === DocumentApp.ElementType.TABLE);
    const cell00 = row0.getCell(0);
    console.assert(cell00.getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(cell00.getText() === 'A1');
    console.assert(cell00.getNumChildren() >= 1);
    console.assert(cell00.getChild(0).getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(cell00.getParent().getType() === DocumentApp.ElementType.TABLE_ROW);
    console.assert(cell00.getRowSpan() === 1);
    console.assert(cell00.getColSpan() === 1);
    console.assert(cell00.getChild(0).getParent().getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(body.copy().getNumChildren() === body.getNumChildren());
    console.assert(row0.getCell(1).getText() === 'B1');
    console.assert(table.getRow(1).getCell(0).getText() === 'A2');
    console.assert(table.getRow(1).getCell(1).getText() === 'B2');

    // Paragraph 4 anchors a footnote whose only paragraph is "Note":
    console.assert(body.getChild(4).getText().substring(0, 8) === 'Trailing');
    const fns = DocumentApp.getActiveDocument().getFootnotes();
    console.assert(fns.length === 1);
    console.assert(fns[0].getType() === DocumentApp.ElementType.FOOTNOTE);
    const noteContents = fns[0].getFootnoteContents();
    console.assert(noteContents.getType() === DocumentApp.ElementType.FOOTNOTE_SECTION);
    console.assert(noteContents.getNumChildren() >= 1);
    console.assert(noteContents.getChild(0).getText() === 'Note');

    // No user selection on a freshly opened document, so getSelection returns null:
    console.assert(DocumentApp.getActiveDocument().getSelection() === null);

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
    console.assert(body.getNumChildren() === 7);

    // Exercise the setBold overload group:
    appended.editAsText().setBold(true).setBold(0, 3, false);
    console.assert(appended.editAsText().isBold(0) === false);
    console.assert(appended.editAsText().isBold(4) === true);

    // Exercise the Text mutation methods; each returns Text so the calls chain:
    appended.editAsText().setText('foo').appendText('bar').insertText(3, '-');
    console.assert(appended.getText() === 'foo-bar');
    appended.editAsText().deleteText(3, 3);
    console.assert(appended.getText() === 'foobar');
    appended.editAsText().setText('');
    console.assert(appended.getText() === '');

    const listItem = body.appendListItem('Item');
    console.assert(listItem.getType() === DocumentApp.ElementType.LIST_ITEM);
    console.assert(listItem.getText() === 'Item');
    console.assert(listItem.getNestingLevel() === 0);
    console.assert(listItem.getGlyphType() === DocumentApp.GlyphType.NUMBER);
    console.assert(typeof listItem.getListId() === 'string');
    console.assert(body.getNumChildren() === 8);

    // clear empties the paragraph text without removing the paragraph from the body:
    const cleared = body.appendParagraph('Doomed');
    console.assert(body.getNumChildren() === 9);
    console.assert(cleared.getText() === 'Doomed');
    cleared.clear();
    console.assert(cleared.getText() === '');
    console.assert(body.getNumChildren() === 9);

    // GAS refuses to remove the section's last paragraph, so append a guard first, then remove
    // cleared (which is no longer the last):
    body.appendParagraph('Guard');
    console.assert(body.getNumChildren() === 10);
    cleared.removeFromParent();
    console.assert(body.getNumChildren() === 9);

    // Removing a whole added paragraph via its Text view works too, again with a guard so what
    // we remove is not the last paragraph:
    const viaText = body.appendParagraph('AlsoDoomed');
    body.appendParagraph('Guard');
    console.assert(body.getNumChildren() === 11);
    viaText.editAsText().removeFromParent();
    console.assert(body.getNumChildren() === 10);

    // A table cell's clear empties the cell's text:
    const tab = body.getChild(3);
    console.assert(tab.getType() === DocumentApp.ElementType.TABLE);
    const cell = tab.getChild(0).getChild(0);
    console.assert(cell.getType() === DocumentApp.ElementType.TABLE_CELL);
    console.assert(cell.getText().length > 0);
    cell.clear();
    console.assert(cell.getText() === '');

    // Removing a table row shrinks the surrounding table:
    console.assert(tab.getNumChildren() === 2);
    tab.getChild(0).removeFromParent();
    console.assert(tab.getNumChildren() === 1);

    // Paragraph 5 anchors a 100x60-pixel inline image at child index 1 (child index 0 is the
    // whole-paragraph Text):
    const image = body.getChild(5).getChild(1).asInlineImage();
    console.assert(image !== null);
    console.assert(image.getWidth() === 100);
    console.assert(image.getHeight() === 60);
    console.assert(image.getAltTitle() === null);
    console.assert(image.getAltDescription() === null);

    // Insert a 20x10 red PNG at the cursor, then verify the setters chain and round-trip:
    const pngHex
        = '89504e470d0a1a0a0000000d49484452000000140000000a08020000003b37e9b100'
        + '00001549444154789c63f8cfc04036225fe7a8e611a319003144c73974da8b110000'
        + '000049454e44ae426082';
    const pngBytes = [];
    for (let i = 0; i < pngHex.length; i += 2) {
        pngBytes.push(parseInt(pngHex.substr(i, 2), 16));
    }
    const blob = Utilities.newBlob(pngBytes, 'image/png');
    // appendImage places the image at the end of the body, unlike cursor.insertInlineImage
    // which needs a visible cursor and returns null when there is none:
    const inserted = body.appendImage(blob);
    console.assert(inserted !== null);
    console.assert(inserted.getType() === DocumentApp.ElementType.INLINE_IMAGE);
    // GAS stores image geometry in points at 72 DPI, so pixel values that are multiples of 4
    // round-trip exactly through the 96/72 conversion:
    inserted.setAltTitle('title').setAltDescription('desc').setWidth(80).setHeight(40);
    console.assert(inserted.getAltTitle() === 'title');
    console.assert(inserted.getAltDescription() === 'desc');
    console.assert(inserted.getWidth() === 80);
    console.assert(inserted.getHeight() === 40);
}
