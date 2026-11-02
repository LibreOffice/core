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

    // The body holds "Before", a table of contents with the one entry "Heading", the heading
    // itself, "After", and an alphabetical index with the one entry "Term". The table of contents
    // is one child, which holds the entry, and so is the index, of the type INDEX that is not in
    // GAS:
    const types = [];
    for (let i = 0; i !== body.getNumChildren(); ++i) {
        types.push(String(body.getChild(i).getType()));
    }
    console.assert(
        JSON.stringify(types)
        === '["PARAGRAPH","TABLE_OF_CONTENTS","PARAGRAPH","PARAGRAPH","INDEX"]');
    const toc = body.getChild(1);
    console.assert(toc.getType() === DocumentApp.ElementType.TABLE_OF_CONTENTS);
    console.assert(toc.getNumChildren() === 1);
    console.assert(toc.getChild(0).getText().startsWith('Heading'));
    console.assert(toc.getText() === toc.getChild(0).getText());
    console.assert(
        toc.getChild(0).getParent().getType() === DocumentApp.ElementType.TABLE_OF_CONTENTS);
    console.assert(toc.getParent().getType() === DocumentApp.ElementType.BODY_SECTION);
    console.assert(toc.getPreviousSibling().getText() === 'Before');
    console.assert(toc.getNextSibling().getText() === 'Heading');
    console.assert(body.getChildIndex(toc) === 1);
    const index = body.getChild(4);
    console.assert(index.getText().startsWith('Term'));
    console.assert(index.asIndex().getIndexKind() === DocumentApp.IndexKind.ALPHABETICAL);
    console.assert(index.getNumChildren() === 1);
    console.assert(index.getChild(0).getParent().getType() === DocumentApp.ElementType.INDEX);
    console.assert(index.getPreviousSibling().getText() === 'After');

    // Each index, and the text for editing it, casts to its own type only:
    console.assert(toc.asTableOfContents().getNumChildren() === 1);
    console.assert(toc.editAsText().asTableOfContents().getNumChildren() === 1);
    for (const [element, message] of [
        [body.getChild(0), "PARAGRAPH can't be cast to TABLE_OF_CONTENTS."],
        [index, "INDEX can't be cast to TABLE_OF_CONTENTS."],
    ]) {
        let caught = null;
        try {
            element.asTableOfContents();
        } catch (e) {
            caught = e.message;
        }
        console.assert(caught === message);
    }
    let caught = null;
    try {
        toc.asIndex();
    } catch (e) {
        caught = e.message;
    }
    console.assert(caught === "TABLE_OF_CONTENTS can't be cast to INDEX.");

    // A range holds a whole table of contents as one element, and an entry that is added on its own
    // as a paragraph:
    const doc = DocumentApp.getActiveDocument();
    const describe = range => range.getRangeElements().map(
        e => String(e.getElement().getType()) + (e.isPartial() ? '-partial' : '')).join(',');
    console.assert(describe(doc.newRange().addElement(toc).build()) === 'TABLE_OF_CONTENTS');
    console.assert(
        describe(doc.newRange().addElement(toc.editAsText()).build()) === 'TABLE_OF_CONTENTS');
    console.assert(describe(doc.newRange().addElement(toc.getChild(0)).build()) === 'PARAGRAPH');
    console.assert(describe(doc.newRange().addElement(index).build()) === 'INDEX');
    console.assert(
        describe(doc.newRange().addElementRange(toc.editAsText(), 0, 3).build())
        === 'TEXT-partial');
    const between = doc.newRange().addElementsBetween(body.getChild(0), body.getChild(3)).build();
    console.assert(describe(between) === 'PARAGRAPH,TABLE_OF_CONTENTS,PARAGRAPH,PARAGRAPH');
    console.assert(between.getRangeElements()[1].getElement().getText().startsWith('Heading'));
    console.assert(
        describe(doc.newRange().addElementsBetween(toc.getChild(0), body.getChild(3)).build())
        === 'PARAGRAPH,PARAGRAPH,PARAGRAPH');

    // The view selects the whole table of contents, and the selection keeps it as one element:
    doc.setSelection(doc.newRange().addElement(toc).build());
    console.assert(describe(doc.getSelection()) === 'TABLE_OF_CONTENTS');
    doc.setSelection(between);
    console.assert(
        describe(doc.getSelection()) === 'PARAGRAPH,TABLE_OF_CONTENTS,PARAGRAPH,PARAGRAPH');

    // A position can be at the end of a table of contents:
    doc.setCursor(doc.newPosition(toc, 1));
    console.assert(
        doc.getCursor().getElement().getType() === DocumentApp.ElementType.TABLE_OF_CONTENTS);
    console.assert(doc.getCursor().getOffset() === 1);

    // The text of the body and its text for editing include the entry, so an offset found in one
    // is the same place in the other:
    console.assert(body.getText() === body.editAsText().getText());
    const i = body.getText().indexOf('After');
    body.editAsText().setBold(i, i + 4, true);
    console.assert(body.getChild(3).editAsText().isBold(0) === true);
}
