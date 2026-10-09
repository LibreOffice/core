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
    console.assert(body.getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(String(body.getType()) === 'BODY_SECTION');
    console.assert(JSON.stringify(body.getType()) === '"BODY_SECTION"');
    console.assert(body.getText().length > 0);
    console.assert(body.getNumChildren() === 6);

    // Paragraph 0 is a bold "Bold", italic "Italic" and plain "Plain" concatenated, followed by an
    // image anchored to a character with text wrap, which is a positioned image and no child:
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
    console.assert(row0.getText() === 'A1\nB1');
    console.assert(table.getText() === 'A1\nB1\nA2\nB2');
    // As in GAS, a container has a text view, whose offsets run across its paragraphs and the
    // separators between them, and a separator has the attributes of the preceding character:
    const rowText = row0.editAsText();
    console.assert(rowText.getType() === DocumentApp.ElementType.TABLE_ROW);
    console.assert(rowText.getText() === 'A1\nB1');
    console.assert(row0.asText().getText() === 'A1\nB1');
    console.assert(rowText.getParent().getType() === DocumentApp.ElementType.TABLE);
    console.assert(table.editAsText().getText() === table.getText());
    console.assert(body.editAsText().getText() === body.getText());
    rowText.setBold(false).setBold(3, 4, true);
    console.assert(rowText.isBold(2) === false);
    console.assert(rowText.isBold(3) === true);
    console.assert(row0.getCell(1).editAsText().isBold(0) === true);
    console.assert(cell00.editAsText().isBold(1) === false);
    let indexMessage = null;
    try {
        rowText.isBold(5);
    } catch (e) {
        indexMessage = e.message;
    }
    console.assert(indexMessage === 'Index (5) must be less than the content length (5).');

    console.assert(body.getChildIndex(p0) === 0);
    console.assert(body.getChildIndex(body.getChild(1)) === 1);
    console.assert(body.getChildIndex(table) === 3);
    console.assert(p0.getChildIndex(p0.getChild(0)) === 0);
    console.assert(table.getChildIndex(table.getRow(1)) === 1);
    console.assert(row0.getChildIndex(row0.getCell(1)) === 1);
    console.assert(cell00.getChildIndex(cell00.getChild(0)) === 0);
    // The Text child of p0 is not a child of the body, even though it stands for the same document
    // paragraph as p0:
    let threw = false;
    try {
        body.getChildIndex(p0.getChild(0));
    } catch (e) {
        threw = true;
    }
    console.assert(threw);

    // Cell B2 and paragraph 4 each anchor a footnote, whose only paragraph is "Cellnote" and
    // "Note", and the references to them are no part of the text:
    console.assert(body.getText().indexOf('A2\nB2\nTrailing') !== -1);
    console.assert(body.getChild(4).getText() === 'Trailing');
    let pastFootnote = false;
    try {
        DocumentApp.getActiveDocument().newPosition(body.getChild(4).getChild(0), 9);
    } catch (e) {
        pastFootnote = true;
    }
    console.assert(pastFootnote);
    const trailingIndices = body.getChild(4).getChild(0).asText().getTextAttributeIndices();
    console.assert(trailingIndices.length === 1 && trailingIndices[0] === 0);
    const fns = DocumentApp.getActiveDocument().getFootnotes();
    console.assert(fns.length === 2);
    console.assert(fns[0].getFootnoteContents().getText() === 'Cellnote');
    console.assert(fns[1].getType() === DocumentApp.ElementType.FOOTNOTE);
    const noteContents = fns[1].getFootnoteContents();
    console.assert(noteContents.getType() === DocumentApp.ElementType.FOOTNOTE_SECTION);
    console.assert(noteContents.getNumChildren() >= 1);
    console.assert(noteContents.getChild(0).getText() === 'Note');
    console.assert(noteContents.getChildIndex(noteContents.getChild(0)) === 0);

    // As in GAS, an element casts to its own type, a paragraph also to a text that reports the
    // paragraph's type, and any other cast throws:
    console.assert(p0.asParagraph().getText() === 'BoldItalicPlain');
    console.assert(p0.asText().getText() === 'BoldItalicPlain');
    console.assert(p0.asText().asParagraph().getText() === 'BoldItalicPlain');
    console.assert(t0.asText().getText() === 'BoldItalicPlain');
    console.assert(table.asTable().getNumRows() === 2);
    console.assert(row0.asTableRow().getNumCells() === 2);
    console.assert(cell00.asTableCell().getText() === 'A1');
    console.assert(body.asBody().getNumChildren() === body.getNumChildren());
    console.assert(fns[1].asFootnote().getFootnoteContents().getText() === 'Note');
    console.assert(noteContents.asFootnoteSection().getText() === 'Note');
    for (const [element, cast, message] of [
        [p0, 'asTable', "PARAGRAPH can't be cast to TABLE."],
        [table, 'asParagraph', "TABLE can't be cast to PARAGRAPH."],
        [p0, 'asInlineImage', "PARAGRAPH can't be cast to INLINE_IMAGE."],
        [fns[1], 'asText', "FOOTNOTE can't be cast to TEXT."],
    ]) {
        let caught = null;
        try {
            element[cast]();
        } catch (e) {
            caught = e.message;
        }
        console.assert(caught === message);
    }

    // No user selection on a freshly opened document, so getSelection returns null:
    console.assert(DocumentApp.getActiveDocument().getSelection() === null);

    // Build a selection spanning all of paragraph 0 plus the first three characters of paragraph 1,
    // set it on the document, and verify the per-paragraph range split:
    const doc = DocumentApp.getActiveDocument();
    const range = doc.newRange()
        .addElement(body.getChild(0))
        .addElement(body.getChild(1).editAsText(), 0, 2)
        .build();
    const ranges = doc.setSelection(range).getSelection().getRangeElements();
    console.assert(ranges.length === 2);
    console.assert(ranges[0].isPartial() === false);
    console.assert(ranges[0].getStartOffset() === -1);
    console.assert(ranges[0].getEndOffsetInclusive() === -1);
    console.assert(ranges[0].getElement().getText() === 'BoldItalicPlain');
    console.assert(ranges[1].isPartial() === true);
    console.assert(ranges[1].getStartOffset() === 0);
    console.assert(ranges[1].getEndOffsetInclusive() === 2);
    console.assert(ranges[1].getElement().getType() === DocumentApp.ElementType.TEXT);
    console.assert(ranges[1].getElement().getText() === 'UnderStrikeSuperPlain');

    // Append a paragraph and a list item at the end, which a Text of the body taken before
    // already shows:
    const bodyText = body.editAsText();
    const appended = body.appendParagraph('Appended');
    console.assert(appended.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(appended.getText() === 'Appended');
    console.assert(bodyText.getText() === body.getText());
    console.assert(bodyText.getText().endsWith('\nAppended'));
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

    // Offsets beyond what fits in 16 bits reach the right character:
    appended.editAsText().setText('a'.repeat(70000)).insertText(66000, 'X');
    console.assert(appended.getText().indexOf('X') === 66000);
    const far = doc.newPosition(appended.getChild(0), 66001);
    console.assert(far.getSurroundingTextOffset() === 66001);
    appended.editAsText().setText('');

    // A negative offset, or a range that ends before it starts, is rejected:
    const throws = (f) => { try { f(); } catch (e) { return true; } return false; };
    appended.editAsText().setText('foobar');
    console.assert(throws(() => appended.editAsText().insertText(-1, 'x')));
    console.assert(throws(() => appended.editAsText().setBold(5, 3, true)));
    console.assert(throws(() => appended.editAsText().deleteText(-1, 0)));
    console.assert(appended.getText() === 'foobar');
    appended.editAsText().setText('');

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
    console.assert(cleared.getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(cleared.clear().getText() === '');
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
    console.assert(cell.clear().getText() === '');

    // Removing a table row shrinks the surrounding table:
    console.assert(tab.getNumChildren() === 2);
    console.assert(
        tab.getChild(0).removeFromParent().getType() === DocumentApp.ElementType.TABLE_ROW);
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

    // An inline image in the middle of the text takes up no offset of its own:
    const withImage = body.appendParagraph('foobar');
    console.assert(doc.newPosition(withImage.getChild(0), 3).insertInlineImage(blob) !== null);
    withImage.editAsText().insertText(4, 'X');
    console.assert(withImage.editAsText().getText() === 'foobXar');

    // Each appended paragraph's object stays on its own paragraph:
    const one = body.appendParagraph('One');
    const two = body.appendParagraph('Two');
    console.assert(one.getText() === 'One');
    console.assert(two.getText() === 'Two');

    // The last paragraph of a section cannot be removed:
    const childCount = body.getNumChildren();
    console.assert(throws(() => two.removeFromParent()));
    console.assert(throws(() => two.editAsText().removeFromParent()));
    console.assert(body.getNumChildren() === childCount);

    // A paragraph takes a heading, and one appended after a heading is a normal one:
    console.assert(
        two.setHeading(DocumentApp.ParagraphHeading.HEADING1).getHeading()
            === DocumentApp.ParagraphHeading.HEADING1);
    const three = body.appendParagraph('Three');
    console.assert(three.getHeading() === DocumentApp.ParagraphHeading.NORMAL);
    two.setHeading(DocumentApp.ParagraphHeading.NORMAL);
    console.assert(two.getHeading() === DocumentApp.ParagraphHeading.NORMAL);

    // newPosition takes a character offset into a Text element:
    const text1 = body.getChild(1).getChild(0);
    console.assert(text1.getType() === DocumentApp.ElementType.TEXT);
    const inText = doc.newPosition(text1, 5);
    console.assert(inText.getOffset() === 5);
    console.assert(inText.getElement().getType() === DocumentApp.ElementType.TEXT);
    console.assert(inText.getElement().getText() === 'UnderStrikeSuperPlain');
    let pastText = false;
    try {
        doc.newPosition(text1, text1.getText().length + 1);
    } catch (e) {
        pastText = true;
    }
    console.assert(pastText);

    // For any other element, newPosition takes a child index, where the index equal to the number
    // of children is the element's end, and the position keeps that element and index (paragraph 5
    // is "Before" followed by its inline image):
    const para5 = body.getChild(5);
    console.assert(para5.getNumChildren() === 2);
    for (let i = 0; i <= 2; ++i) {
        const pos = doc.newPosition(para5, i);
        console.assert(pos.getOffset() === i);
        console.assert(pos.getElement().getType() === DocumentApp.ElementType.PARAGRAPH);
        console.assert(pos.getElement().getText() === 'Before');
    }
    let pastChildren = false;
    try {
        doc.newPosition(para5, 3);
    } catch (e) {
        pastChildren = true;
    }
    console.assert(pastChildren);

    // After setCursor, the cursor is the given position, with the element and child index it was
    // made from (GAS reports every getCursor after the first one that is not null in a script
    // execution against that first cursor's element, with an offset that is not relative to that
    // element, so this is the only getCursor here that is not null):
    const cursor = doc.setCursor(doc.newPosition(para5, 1)).getCursor();
    console.assert(cursor.getElement().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(cursor.getElement().getText() === 'Before');
    console.assert(cursor.getOffset() === 1);
    console.assert(cursor.getSurroundingText().getText() === 'Before');
    console.assert(cursor.getSurroundingTextOffset() === 6);

    // An inline image can be selected on its own, as one whole range element:
    const imageRanges = doc.setSelection(doc.newRange().addElement(image).build()).getSelection()
        .getRangeElements();
    console.assert(imageRanges.length === 1);
    console.assert(imageRanges[0].isPartial() === false);
    console.assert(imageRanges[0].getStartOffset() === -1);
    console.assert(imageRanges[0].getEndOffsetInclusive() === -1);
    const selected = imageRanges[0].getElement();
    console.assert(selected.getType() === DocumentApp.ElementType.INLINE_IMAGE);
    console.assert(selected.asInlineImage().getWidth() === 100);
    console.assert(selected.getParent().getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(selected.getParent().asText().getText() === 'Before');
    console.assert(selected.getParent().getChildIndex(selected) === 1);
    // While there is a selection, there is no cursor:
    console.assert(doc.getCursor() === null);
    // Setting the cursor ends the selection of the image (the cursor itself is not read again here,
    // see above):
    console.assert(doc.setCursor(doc.newPosition(para5, 1)).getSelection() === null);

    // Clearing the body, which still holds a table and a list item here, keeps one empty
    // paragraph:
    console.assert(body.clear().getNumChildren() === 1);
    console.assert(body.getChild(0).getType() === DocumentApp.ElementType.PARAGRAPH);
    console.assert(
        body.getChild(0).asParagraph().getHeading() === DocumentApp.ParagraphHeading.NORMAL);
    console.assert(body.getText() === '');

    // An image goes into the document at its natural size, here 300 by 200 pixels:
    const png = Utilities.base64Decode(
        'iVBORw0KGgoAAAANSUhEUgAAASwAAADICAIAAADdvUsCAAABrElEQVR42u3TQQkAAAjAwPUvrTF8eHAJBmsKOCQB'
        + 'mBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQ'
        + 'TAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAgmlABM'
        + 'CCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgm'
        + 'BEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBEwIJgRMCCYETAgmBBMCJgQT'
        + 'AiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwIm'
        + 'BBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAiYEEwImBBMCJgQTAhKACcGEgAnBhIAJ'
        + '4akFT+KDSJWA/YAAAAAASUVORK5CYII=');
    const appendedImage = body.appendImage(Utilities.newBlob(png, 'image/png'));
    console.assert(appendedImage.getWidth() === 300);
    console.assert(appendedImage.getHeight() === 200);
    const insertedImage = doc.newPosition(body.getChild(0), 0)
        .insertInlineImage(Utilities.newBlob(png, 'image/png'));
    console.assert(insertedImage.getWidth() === 300);
    console.assert(insertedImage.getHeight() === 200);

    // Document properties are strings that a missing key reads as null:
    const docProps = PropertiesService.getDocumentProperties();
    docProps.deleteAllProperties();
    console.assert(docProps.getProperty('k') === null);
    docProps.setProperty('k', 'v').setProperties({ a: '1', b: 2 });
    console.assert(docProps.getProperty('k') === 'v');
    console.assert(docProps.getProperty('b') === '2.0');
    console.assert(docProps.setProperty('n', 2.5).getProperty('n') === '2.5');
    docProps.deleteProperty('n');
    console.assert(JSON.stringify(docProps.getKeys().sort()) === '["a","b","k"]');
    if (globalThis.cool) {
        console.assert(
            cool.getActiveUnoModel().getDocumentProperties().getUserDefinedProperties()
                .getPropertyValue('scriptinterop_test') === '{"k":"v","a":"1","b":"2.0"}');
    }
    docProps.deleteProperty('a');
    docProps.setProperties({ c: '3' }, true);
    console.assert(JSON.stringify(docProps.getProperties()) === '{"c":"3"}');

    // A number is stored the way GAS writes it:
    console.assert(docProps.setProperty('number', -3).getProperty('number') === '-3.0');
    console.assert(docProps.setProperty('number', 9999999).getProperty('number') === '9999999.0');
    console.assert(docProps.setProperty('number', 10000000).getProperty('number') === '1.0E7');
    console.assert(
        docProps.setProperty('number', 12345678).getProperty('number') === '1.2345678E7');
    console.assert(
        docProps.setProperty('number', 123456789012).getProperty('number') === '1.23456789012E11');
    console.assert(docProps.setProperty('number', 1e21).getProperty('number') === '1.0E21');
    console.assert(docProps.setProperty('number', 0.001).getProperty('number') === '0.001');
    console.assert(docProps.setProperty('number', 0.0001).getProperty('number') === '1.0E-4');
    console.assert(docProps.setProperty('number', 1e-7).getProperty('number') === '1.0E-7');
    console.assert(
        docProps.setProperty('number', 1 / 3).getProperty('number') === '0.3333333333333333');
    console.assert(docProps.setProperty('number', -0).getProperty('number') === '-0.0');
    console.assert(docProps.setProperty('number', NaN).getProperty('number') === 'NaN');
    console.assert(docProps.setProperty('number', Infinity).getProperty('number') === 'Infinity');
    console.assert(docProps.setProperty('number', true).getProperty('number') === 'true');
    {
        let message = null;
        try {
            docProps.setProperty('number', null);
        } catch (e) {
            message = e.message;
        }
        console.assert(message === 'Invalid argument: value');
    }

    // A property whose type a user changed to a number in the properties dialog holds no
    // properties, and setting one replaces it:
    if (globalThis.cool) {
        const custom = cool.getActiveUnoModel().getDocumentProperties().getUserDefinedProperties();
        custom.removeProperty('scriptinterop_test');
        custom.addProperty('scriptinterop_test', 128, 42);
        console.assert(docProps.getKeys().length === 0);
        console.assert(docProps.setProperty('k', 'v').getProperty('k') === 'v');
    }

    docProps.deleteAllProperties();
    console.assert(docProps.getKeys().length === 0);
}
