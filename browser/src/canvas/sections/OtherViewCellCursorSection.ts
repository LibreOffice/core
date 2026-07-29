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
    private static hoveredViewId: number | null = null;
    private static hoveredCellAddress: string | null = null;
    private static maxPopUpRows = 6; // 5 names + 1 summary row

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

        this.sectionProperties.cellAddress = null;
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
        // Measuring a hidden popup gives 0x0, and getBoundingClientRect forces a layout
        // on every draw. Nothing to place until it is shown.
        if (!this.sectionProperties.popUpShown)
            return;

        const width = this.sectionProperties.popUpContainer.getBoundingClientRect().width;
        const height = this.sectionProperties.popUpContainer.getBoundingClientRect().height;

        // The sheet area: the popup is a child of the document container, so its own
        // coordinates start at the canvas corner, while the grid starts after the headers.
        // Outside of it the popup is covered by the toolbars or the headers.
        const anchor = app.sectionContainer.getDocumentAnchor();
        const viewSize = app.sectionContainer.getViewSize();
        const minLeft = anchor[0] / app.dpiScale;
        const minTop = anchor[1] / app.dpiScale;
        const maxLeft = viewSize[0] / app.dpiScale - width;
        const maxTop = viewSize[1] / app.dpiScale - height;

        // Space for the arrow.
        const arrowGap = 15;

        const above = this.myTopLeft[1] / app.dpiScale - (height + arrowGap);
        const below = (this.myTopLeft[1] + this.size[1]) / app.dpiScale + arrowGap;

        // Above the cell when it fits there, below it when that fits instead.
        const flipped = above < minTop && below <= maxTop;
        if (flipped !== this.sectionProperties.popUpContainer.classList.contains('below-cell')) {
            this.sectionProperties.popUpContainer.classList.toggle('below-cell', flipped);
            this.updateArrowColor();
        }

        // In RTL myTopLeft holds the mirrored left edge.
        const addition = app.map._docLayer.isCalcRTL() ? -this.size[0] : 0;
        const centered = (this.myTopLeft[0] + addition + this.size[0] * 0.5) / app.dpiScale - width * 0.5;

        const pos = [
            Math.max(minLeft, Math.min(centered, maxLeft)),
            Math.max(minTop, Math.min(flipped ? below : above, maxTop)),
        ];
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

            const arrowDiv = document.createElement('div');
            arrowDiv.className = 'arrow-div';
            popUpContainer.appendChild(arrowDiv);

            document.getElementById('document-container').appendChild(popUpContainer);

            this.sectionProperties.popUpContainer = popUpContainer;
            this.sectionProperties.nameContainer = nameContainer;
            this.sectionProperties.arrowDiv = arrowDiv;

            this.hideUsernamePopUp();
        }
    }

    // Every view sitting on this cell, in the order the sections were created, which
    // is also the order they are drawn in.
    getCoLocatedSections(): Array<OtherViewCellCursorSection> {
        const address = this.sectionProperties.cellAddress;

        // Without an address there is nothing to compare, so only this view is listed.
        if (!address)
            return [this];

        const group: Array<OtherViewCellCursorSection> = [];

        for (let i = 0; i < OtherViewCellCursorSection.sectionPointers.length; i++) {
            const section = OtherViewCellCursorSection.sectionPointers[i];

            if (section.showSection && section.isVisible
                && section.sectionProperties.part === this.sectionProperties.part
                && section.sectionProperties.cellAddress === address)
                group.push(section);
        }

        return group;
    }

    fillUsernamePopUp(group: Array<OtherViewCellCursorSection>) {
        const rows = document.createDocumentFragment();
        const maxRows = OtherViewCellCursorSection.maxPopUpRows;

        // Truncate only when that saves a row. The oldest sections go, keeping the ones
        // drawn on top, this one among them: it anchors the bubble and colors the arrow.
        const truncated = group.length > maxRows;
        const shown = truncated ? group.slice(group.length - (maxRows - 1)) : group;

        for (let i = 0; i < shown.length; i++) {
            const row = document.createElement('div');
            row.className = 'name-row';
            row.textContent = shown[i].sectionProperties.username;
            row.style.backgroundColor = shown[i].sectionProperties.color;
            rows.appendChild(row);
        }

        if (truncated) {
            const row = document.createElement('div');
            row.className = 'name-row more-row';
            row.textContent = _('%n more').replace('%n', (group.length - shown.length).toString());
            rows.appendChild(row);
        }

        this.sectionProperties.nameContainer.replaceChildren(rows);
        this.updateArrowColor();
    }

    updateArrowColor() {
        const rows = this.sectionProperties.nameContainer.children;

        if (rows.length === 0)
            return;

        const flipped = this.sectionProperties.popUpContainer.classList.contains('below-cell');
        const row = flipped ? rows[0] : rows[rows.length - 1];

        this.sectionProperties.arrowDiv.style.backgroundColor = getComputedStyle(row).backgroundColor;
    }

    clearPopUpTimer() {
        if (this.sectionProperties.popUpTimer) {
            clearTimeout(this.sectionProperties.popUpTimer);
            this.sectionProperties.popUpTimer = null;
        }
    }

    showUsernamePopUp() {
        if (!this.sectionProperties.popUpContainer || !this.isVisible)
            return;

        const group = this.getCoLocatedSections();
        const owner = group[group.length - 1];

        if (!owner)
            return;

        if (owner !== this) {
            owner.showUsernamePopUp();
            return;
        }

        // Only the owner's bubble is used for the cell.
        OtherViewCellCursorSection.hidePopUpsForCell(this.sectionProperties.part, this.sectionProperties.cellAddress);

        const named: Array<OtherViewCellCursorSection> = [];
        for (let i = 0; i < group.length; i++) {
            const headerName = CursorHeaderSection.namePrefix + group[i].sectionProperties.viewId;
            if (!app.sectionContainer.doesSectionExist(headerName))
                named.push(group[i]);
        }

        if (named.length === 0) {
            this.hideUsernamePopUp();
            return;
        }

        this.fillUsernamePopUp(named);

        this.sectionProperties.popUpShown = true;
        this.sectionProperties.popUpContainer.style.display = '';

        // Position it after it is shown, a hidden element measures 0x0 and would
        // end up right of the cell instead of centered above it.
        this.adjustPopUpPosition();

        this.clearPopUpTimer();

        this.sectionProperties.popUpTimer = setTimeout(() => {
            this.hideUsernamePopUp();
        }, 3000);
    }

    onRemove(): void {
        // The bubble of the cell lists this view, and the container is not a child of
        // the canvas, so it would stay behind.
        OtherViewCellCursorSection.hidePopUpsForCell(this.sectionProperties.part, this.sectionProperties.cellAddress);

        this.clearPopUpTimer();

        if (this.sectionProperties.popUpContainer) {
            this.sectionProperties.popUpContainer.remove();
            this.sectionProperties.popUpContainer = null;
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

    public static addOrUpdateOtherViewCellCursor(viewId: number, username: string, rectangleData: Array<string>, part: number, cellAddress?: string) {
        let rectangle = new cool.SimpleRectangle(0, 0, 0, 0);
        if (rectangleData)
            rectangle = new cool.SimpleRectangle(parseInt(rectangleData[0]), parseInt(rectangleData[1]), parseInt(rectangleData[2]), parseInt(rectangleData[3]));

        const sectionName = OtherViewCellCursorSection.sectionNamePrefix + viewId;
        let section: OtherViewCellCursorSection;
        let newSection = false;
        let moved = true;
        if (app.sectionContainer.doesSectionExist(sectionName)) {
            section = app.sectionContainer.getSectionWithName(sectionName) as OtherViewCellCursorSection;

            // The cursor is re-sent on zoom, on column and row resizes and on sheet
            // switches, partly as a replay of the saved message. The cell is the same
            // then, only its coordinates changed, so don't pop the name up again.
            moved = section.sectionProperties.part !== part
                || !cellAddress
                || section.sectionProperties.cellAddress !== cellAddress;

            // The bubble of the cell being left still lists this view.
            if (moved)
                OtherViewCellCursorSection.hidePopUpsForCell(section.sectionProperties.part, section.sectionProperties.cellAddress);

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
        section.sectionProperties.cellAddress = cellAddress;
        section.prepareUsernamePopUp();

        section.setShowSection(section.checkMyVisibility());

        if (section.showSection && !newSection && moved)
            section.showUsernamePopUp();

        if (!section.showSection)
            section.hideUsernamePopUp();

        app.sectionContainer.requestReDraw();
    }

    public static resetHover() {
        OtherViewCellCursorSection.setHoveredView(null);
    }

    // The bubble is supplementary, so it goes as soon as the pointer moves on, leaving
    // one on screen at a time instead of a trail of them. The hide timer stays as a
    // backstop.
    private static setHoveredView(section: OtherViewCellCursorSection | null) {
        const viewId = section ? section.sectionProperties.viewId : null;
        const previous = OtherViewCellCursorSection.hoveredViewId;
        const entered = viewId !== previous;

        // Only take down what the hover put up. Once that view has moved on, the bubble
        // on screen is the one its move opened, and it lives its own time out.
        if (entered && previous !== null) {
            const previousSection = OtherViewCellCursorSection.getViewCursorSection(previous) as OtherViewCellCursorSection;

            if (previousSection && previousSection.sectionProperties.cellAddress === OtherViewCellCursorSection.hoveredCellAddress)
                previousSection.hideUsernamePopUp();
        }

        // Refreshed even when the view is unchanged: it can have moved under a pointer
        // that did not.
        OtherViewCellCursorSection.hoveredViewId = viewId;
        OtherViewCellCursorSection.hoveredCellAddress = section ? section.sectionProperties.cellAddress : null;

        if (entered && section)
            section.showUsernamePopUp();
    }

    private static hidePopUpsForCell(part: number, cellAddress: string) {
        if (!cellAddress)
            return;

        for (let i = 0; i < OtherViewCellCursorSection.sectionPointers.length; i++) {
            const section = OtherViewCellCursorSection.sectionPointers[i];

            if (section.sectionProperties.part === part && section.sectionProperties.cellAddress === cellAddress)
                section.hideUsernamePopUp();
        }
    }

    public static removeView(viewId: number) {
        if (OtherViewCellCursorSection.hoveredViewId === viewId)
            OtherViewCellCursorSection.resetHover();

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

    public static checkHover(canvasPosition: Array<number>) {
        let hovered: OtherViewCellCursorSection | null = null;

        // Two views on the same cell have the very same rectangle, so all of them are
        // hit. Keep the last one: these sections share their drawing order, so the one
        // added last is drawn on top, and it owns the bubble listing the whole cell.
        for (let i = 0; i < OtherViewCellCursorSection.sectionPointers.length; i++) {
            const section = OtherViewCellCursorSection.sectionPointers[i];

            if (!section.showSection || !section.isVisible)
                continue;

            if (section.isHit(canvasPosition))
                hovered = section;
        }

        OtherViewCellCursorSection.setHoveredView(hovered);
    }
}

app.definitions.otherViewCellCursorSection = OtherViewCellCursorSection;
