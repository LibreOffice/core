// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class CursorHeaderSection extends HTMLObjectSection {
    static namePrefix: string = 'CursorHeader ';
    static duration = 3000;

    constructor(viewId: number, username: string, documentPosition: cool.SimplePoint, color: string) {
        super(CursorHeaderSection.namePrefix + viewId, null, null, documentPosition, 'cursor-header-section');

        this.sectionProperties.deletionTimeout = null;
        this.sectionProperties.viewId = viewId;
        this.sectionProperties.color = color;
        this.sectionProperties.username = username;

        const div = this.getHTMLObject();

        div.textContent = username;
        div.style.color = 'white';
        div.style.backgroundColor = color;
    }

    deleteThis(force: boolean = false) { // Also resets the timer if it is initiated.
        if (this.sectionProperties.deletionTimeout)
            clearTimeout(this.sectionProperties.deletionTimeout);

        this.sectionProperties.deletionTimeout = setTimeout(() => {
            app.sectionContainer.removeSection(this.name);
        }, (force ? 10: CursorHeaderSection.duration));
    }

    // This section is for text cursor username popups in Calc. When we want to remove the popup before it times out, we use this function.
    public static deletePopUpNow(viewId: number) {
        // If cursor header is also shown, delete it.
        const name = CursorHeaderSection.namePrefix + viewId;
        if (app.sectionContainer.doesSectionExist(name)) {
            const section: CursorHeaderSection = app.sectionContainer.getSectionWithName(name) as CursorHeaderSection;
            section.deleteThis(true);
        }
    }

    public static showCursorHeader(viewId: number, username: string, documentPosition: cool.SimplePoint, color: string) {
        const sectionName = CursorHeaderSection.namePrefix + viewId;
        let section;

        if (viewId && !username) { // This should be an update, section should exist.
            section = app.sectionContainer.getSectionWithName(sectionName);
            if (section)
                section.deleteThis();
        }
        else {
            section = app.sectionContainer.getSectionWithName(sectionName);

            if (!section) {
                section = new CursorHeaderSection(viewId, username, documentPosition, color);
                app.sectionContainer.addSection(section);
            }

            section.setPosition(documentPosition.pX, documentPosition.pY);
            section.deleteThis();
        }

        // If this is calc and cell cursor username popup is shown, hide it.
        if (app.map._docLayer._docType === 'spreadsheet') {
            const cellCursorSection = OtherViewCellCursorSection.getViewCursorSection(viewId) as OtherViewCellCursorSection;
            if (cellCursorSection)
                cellCursorSection.hideUsernamePopUp();
        }
    }
}

app.definitions.cursorHeaderSection = CursorHeaderSection;
