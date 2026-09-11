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
    const presentation = SlidesApp.getActivePresentation();
    console.assert(presentation.getSlides().length === 1);
    if (globalThis.cool) {
        console.assert(presentation.getName() === 'Untitled presentation');
    }

    // One argument picks the layout overload; the enum object goes through as a UNO any.
    const slide = presentation.appendSlide(SlidesApp.PredefinedLayout.TITLE_AND_BODY);
    console.assert(presentation.getSlides().length === 2);
    console.assert(slide.getPageType() === SlidesApp.PageType.SLIDE);
    const title = slide.getPlaceholder(SlidesApp.PlaceholderType.TITLE);
    console.assert(title !== null);
    console.assert(title.getPageElementType() === SlidesApp.PageElementType.SHAPE);
    console.assert(slide.getPlaceholder(SlidesApp.PlaceholderType.SUBTITLE) === null);

    // Five arguments pick the geometry overload.
    const box = slide.insertTextBox('Hello from SlidesApp', 36, 36, 288, 72);
    console.assert(box.getText().asString() === 'Hello from SlidesApp');
    console.assert(slide.replaceAllText('Hello', 'Bye') === 1);
    console.assert(box.getText().asString() === 'Bye from SlidesApp');

    console.assert(slide.getLayout().getMaster() !== null);
    console.assert(presentation.getMasters().length === 1);
    console.assert(slide.getNotesPage().getSpeakerNotesShape() !== null);
    console.assert(slide.isSkipped() === false);
    slide.setSkipped(true);
    console.assert(slide.isSkipped() === true);

    const transform = SlidesApp.newAffineTransformBuilder().setScaleX(2).setTranslateX(10).build();
    console.assert(transform.getScaleX() === 2);
    console.assert(transform.getTranslateX() === 10);

    slide.selectAsCurrentPage();
    const selection = presentation.getSelection();
    console.assert(selection.getSelectionType() === SlidesApp.SelectionType.CURRENT_PAGE);
    console.assert(selection.getCurrentPage().asSlide().getPageType() === SlidesApp.PageType.SLIDE);
}
