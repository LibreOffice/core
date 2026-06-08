// @ts-strict-ignore
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

class ValidityInputHelpSection extends HTMLObjectSection {
    static sectionName = 'Validity Input Help';
    static className = 'input-help';

    public static showValidityInputHelp(textMsg: string, documentPosition: cool.SimplePoint) {
        let message: any = textMsg.replace('validityinputhelp: ', '');
		message = JSON.parse(message);

        const section = new ValidityInputHelpSection(ValidityInputHelpSection.sectionName, null, null, documentPosition, ValidityInputHelpSection.className, true);
        app.sectionContainer.addSection(section);

        const objectDiv = section.getHTMLObject();

        const title = document.createElement('h4');
        title.textContent = message.title;
        objectDiv.appendChild(title);

        const content = document.createElement('p');
        content.textContent = message.content;
        objectDiv.appendChild(content);

        // The section was constructed without explicit dimensions, so its size
        // stayed zero. A zero-size section whose anchor sits on the edge of the
        // viewport is treated as not visible, which is what happens for input
        // help on the top row or the first column (the anchor is at y or x zero).
        // Adopt the rendered size of the content, in core pixels, so the section
        // is a real rectangle that reaches into the viewport and stays shown.
        const renderedSize = objectDiv.getBoundingClientRect();
        section.size = [renderedSize.width * app.dpiScale, renderedSize.height * app.dpiScale];
    }

    public static removeValidityInputHelp() {
        if (app.sectionContainer.doesSectionExist(ValidityInputHelpSection.sectionName))
            app.sectionContainer.removeSection(ValidityInputHelpSection.sectionName);
    }

    constructor (sectionName: string, objectWidth: number, objectHeight: number, documentPosition: cool.SimplePoint, extraClass: string = "", showSection: boolean = true) {
        super(sectionName, objectWidth, objectHeight, documentPosition, extraClass, showSection);
    }
}

app.definitions.validityInputHelpSection = ValidityInputHelpSection;
