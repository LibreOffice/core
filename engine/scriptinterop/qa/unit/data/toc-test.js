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
    // is one child, which holds the entry, and so is the index, which GAS has no type for:
    const types = [];
    for (let i = 0; i !== body.getNumChildren(); ++i) {
        types.push(String(body.getChild(i).getType()));
    }
    console.assert(
        JSON.stringify(types)
        === '["PARAGRAPH","TABLE_OF_CONTENTS","PARAGRAPH","PARAGRAPH","UNSUPPORTED"]');
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
    console.assert(body.getChild(4).getText().startsWith('Term'));

    // Only the table of contents, and the text for editing it, cast to a table of contents:
    console.assert(toc.asTableOfContents().getNumChildren() === 1);
    console.assert(toc.editAsText().asTableOfContents().getNumChildren() === 1);
    for (const [element, message] of [
        [body.getChild(0), "PARAGRAPH can't be cast to TABLE_OF_CONTENTS."],
        [body.getChild(4), "UNSUPPORTED can't be cast to TABLE_OF_CONTENTS."],
    ]) {
        let caught = null;
        try {
            element.asTableOfContents();
        } catch (e) {
            caught = e.message;
        }
        console.assert(caught === message);
    }

    // The text of the body and its text for editing include the entry, so an offset found in one
    // is the same place in the other:
    console.assert(body.getText() === body.editAsText().getText());
    const i = body.getText().indexOf('After');
    body.editAsText().setBold(i, i + 4, true);
    console.assert(body.getChild(3).editAsText().isBold(0) === true);
}
