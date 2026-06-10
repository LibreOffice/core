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

// This is used for other views' cell cursors.

class OtherViewCellCursorSection extends CanvasSectionObject {
    documentObject: boolean = true;
    interactable: boolean = false; // We don't bother with events.
    zIndex: number = app.CSections.ColumnHeader.zIndex;
    drawingOrder: number = app.CSections.OtherViewCellCursor.drawingOrder;
    processingOrder: number = app.CSections.OtherViewCellCursor.processingOrder;

    static sectionNamePrefix = 'OtherViewCellCursorSection ';
    static sectionPointers: Array<OtherViewCellCursorSection> = [];

    constructor(viewId: number, rectangle: cool.SimpleRectangle, part: number) {
        super(OtherViewCellCursorSection.sectionNamePrefix + viewId);

        this.size = [rectangle.pWidth, rectangle.pHeight];
        this.position = [rectangle.pX1, rectangle.pY1];
        this.sectionProperties.color = app.LOUtil.rgbToHex(app.LOUtil.getViewIdColor(viewId));

        this.sectionProperties.viewId = viewId;
        this.sectionProperties.part = part;

        this.sectionProperties.popUpContainer = null;
        this.sectionProperties.popUpShown = false;

        this.sectionProperties.username = null;
        this.sectionProperties.popUpTimer = null;
    }

    onDraw(frameCount?: number, elapsedTime?: number): void {
        this.adjustPopUpPosition();

        this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
        this.context.strokeStyle = this.sectionProperties.color;
        this.context.lineWidth = 2;

        this.drawViewRectangle(this.boundingRectangle);

        this.context.restore();
    }

    checkMyVisibility() {
        if (app.map._docLayer._selectedPart !== this.sectionProperties.part)
            return false;
        else
            return true;
    }

    adjustPopUpPosition() {
        const width = this.sectionProperties.popUpContainer.getBoundingClientRect().width;
        const height = this.sectionProperties.popUpContainer.getBoundingClientRect().height;

        const pos = [this.myTopLeft[0] / app.dpiScale + this.size[0] * 0.5 / app.dpiScale - (width * 0.5), (this.myTopLeft[1] / app.dpiScale) - (height + 15)];
        this.sectionProperties.popUpContainer.style.left = pos[0] + 'px';
        this.sectionProperties.popUpContainer.style.top = pos[1] + 'px';

        if (!this.showSection)
            this.hideUsernamePopUp();
    }

    onNewDocumentTopLeft(): void {
        this.adjustPopUpPosition();
    }

    prepareUsernamePopUp() {
        if (this.sectionProperties.popUpContainer === null) {
            const popUpContainer = document.createElement('div');

            popUpContainer.className = 'username-pop-up';

            const nameContainer = document.createElement('div');
            popUpContainer.appendChild(nameContainer);

            const nameParagraph = document.createElement('p');
            nameContainer.appendChild(nameParagraph);
            nameParagraph.textContent = this.sectionProperties.username;

            const arrowDiv = document.createElement('div');
            arrowDiv.className = 'arrow-div';
            popUpContainer.appendChild(arrowDiv);

            popUpContainer.style.backgroundColor = nameContainer.style.backgroundColor = this.sectionProperties.color;
            arrowDiv.style.backgroundColor = nameParagraph.style.backgroundColor = this.sectionProperties.color;

            document.getElementById('document-container').appendChild(popUpContainer);

            this.sectionProperties.popUpContainer = popUpContainer;

            this.hideUsernamePopUp();
        }
    }

    clearPopUpTimer() {
        if (this.sectionProperties.popUpTimer) {
            clearTimeout(this.sectionProperties.popUpTimer);
            this.sectionProperties.popUpTimer = null;
        }
    }

    showUsernamePopUp() {
        const textCursorSectionName = CursorHeaderSection.namePrefix + this.sectionProperties.viewId;

        if (app.sectionContainer.doesSectionExist(textCursorSectionName))
            return; // Don't show the popup if the cursor header is shown.

        if (this.sectionProperties.popUpContainer && this.isVisible) {
            this.adjustPopUpPosition();

            this.sectionProperties.popUpShown = true;
            this.sectionProperties.popUpContainer.style.display = '';

            this.clearPopUpTimer();

            this.sectionProperties.popUpTimer = setTimeout(() => {
                this.hideUsernamePopUp();
            }, 3000);
        }
    }

    hideUsernamePopUp() {
        if (this.sectionProperties.popUpContainer) {
            this.sectionProperties.popUpShown = false;
            if (this.sectionProperties.popUpContainer.style.display !== 'none')
                this.sectionProperties.popUpContainer.style.display = 'none';
        }
        this.clearPopUpTimer();
    }

    onDocumentObjectVisibilityChange(): void {
        if (this.sectionProperties.popUpShown && !this.isVisible)
            this.hideUsernamePopUp();
    }

    public static addOrUpdateOtherViewCellCursor(viewId: number, username: string, rectangleData: Array<string>, part: number) {
        let rectangle = new cool.SimpleRectangle(0, 0, 0, 0);
        if (rectangleData)
            rectangle = new cool.SimpleRectangle(parseInt(rectangleData[0]), parseInt(rectangleData[1]), parseInt(rectangleData[2]), parseInt(rectangleData[3]));

        const sectionName = OtherViewCellCursorSection.sectionNamePrefix + viewId;
        let section: OtherViewCellCursorSection;
        let newSection = false;
        if (app.sectionContainer.doesSectionExist(sectionName)) {
            section = app.sectionContainer.getSectionWithName(sectionName) as OtherViewCellCursorSection;
            section.sectionProperties.part = part;
            section.size[0] = rectangle.pWidth;
            section.size[1] = rectangle.pHeight;
            section.setPosition(rectangle.pX1, rectangle.pY1);
        }
        else {
            section = new OtherViewCellCursorSection(viewId, rectangle, part);
            app.sectionContainer.addSection(section);
            OtherViewCellCursorSection.sectionPointers.push(section);
            newSection = true;
        }

        section.sectionProperties.username = username;
        section.prepareUsernamePopUp();

        section.setShowSection(section.checkMyVisibility());

        if (section.showSection && !newSection)
            section.showUsernamePopUp();

        if (!section.showSection)
            section.hideUsernamePopUp();

        app.sectionContainer.requestReDraw();
    }

    public static removeView(viewId: number) {
        const sectionName = OtherViewCellCursorSection.sectionNamePrefix + viewId;
        if (app.sectionContainer.doesSectionExist(sectionName)) {
            const section = app.sectionContainer.getSectionWithName(sectionName) as OtherViewCellCursorSection;
            OtherViewCellCursorSection.sectionPointers.splice(OtherViewCellCursorSection.sectionPointers.indexOf(section), 1);
            app.sectionContainer.removeSection(sectionName);
            app.sectionContainer.requestReDraw();
        }
    }

    public static updateVisibilities() {
        for (let i = 0; i < OtherViewCellCursorSection.sectionPointers.length; i++) {
            const section = OtherViewCellCursorSection.sectionPointers[i];
            const newState = section.checkMyVisibility();

            if (newState !== section.showSection) {
                section.setShowSection(newState);
                if (newState === false)
                    section.hideUsernamePopUp();
            }
        }
        app.sectionContainer.requestReDraw();
    }

    public static closePopups() {
        for (let i = 0; i < OtherViewCellCursorSection.sectionPointers.length; i++)
            OtherViewCellCursorSection.sectionPointers[i].hideUsernamePopUp();
    }

    public static getViewCursorSection(viewId: number) {
        const name = OtherViewCellCursorSection.sectionNamePrefix + viewId;
        return app.sectionContainer.getSectionWithName(name);
    }

    public static doesViewCursorExist(viewId: number) {
        const name = OtherViewCellCursorSection.sectionNamePrefix + viewId;
        return app.sectionContainer.doesSectionExist(name);
    }

    public static showPopUpForView(viewId: number) {
        if (OtherViewCellCursorSection.doesViewCursorExist(viewId)) {
            const section = OtherViewCellCursorSection.getViewCursorSection(viewId);
            section.showUsernamePopUp();
        }
    }
}

app.definitions.otherViewCellCursorSection = OtherViewCellCursorSection;
