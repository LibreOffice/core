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

describe('CanvasSectionContainer', function() {

addCanvasToDom();
addMockCanvas(window);
global.requestAnimationFrame = window.requestAnimationFrame;
global.cancelAnimationFrame = window.cancelAnimationFrame;

const canvasWidth = 1024;
const canvasHeight = 768;
const halfWidth = Math.floor(canvasWidth / 2);
const halfHeight = Math.floor(canvasHeight / 2);
const originX = 0;
const originY = 0;

describe('Singleton section container', function() {

    beforeEach(function () {
        setupCanvasContainer(canvasWidth, canvasHeight);

        app.sectionContainer.enableDrawing();

        const docLayer = {};
        const tsManager = {};

        let onlySection = new CanvasSectionObject('OnlySection');
        onlySection.anchor = ['top', 'left'];
        onlySection.position = [originX, originY];
        onlySection.size = [1, 1];
        onlySection.expand = ['bottom', 'right'];
        onlySection.processingOrder = onlySection.drawingOrder = onlySection.zIndex = 1;
        onlySection.interactable = false;
        onlySection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };

        app.sectionContainer.addSection(onlySection);
    });

    it('Container should have OnlySection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('OnlySection'));
    });

    it('OnlySection PosSize checks', function () {
        const only = app.sectionContainer.getSectionWithName('OnlySection');
        const onlyRect = getSectionRectangle(only);
        assertPosSize(onlyRect,
            {
                x: originX,
                y: originY,
                width: canvasWidth - originX,
                height: canvasHeight - originY
            });
    });
});

describe('Horizontally packed two section container', function() {

    beforeEach(function () {
        setupCanvasContainer(canvasWidth, canvasHeight);

        const docLayer = {};
        const tsManager = {};

        let leftSection = new CanvasSectionObject('LeftSection');
        leftSection.anchor = ['top', 'left'];
        leftSection.position = [originX, originY];
        leftSection.size = [halfWidth, 1];
        leftSection.expand = ['bottom'];
        leftSection.processingOrder = leftSection.drawingOrder = leftSection.zIndex = 1;
        leftSection.interactable = false;
        leftSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(leftSection);

        let rightSection = new CanvasSectionObject('RightSection');
        rightSection.anchor = ['top', ['LeftSection', 'right', 'left']] as (string[] | Array<Array<string>>);
        rightSection.position = [originX, originY];
        rightSection.size = [1, 1];
        rightSection.expand = ['bottom', 'right'];
        rightSection.processingOrder = rightSection.drawingOrder = 2;
        rightSection.zIndex = 1;
        rightSection.interactable = false;
        rightSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(rightSection);

        app.sectionContainer.enableDrawing();
    });

    it('Container should have LeftSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('LeftSection'));
    });

    it('Container should have RightSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('RightSection'));
    });

    it('LeftSection PosSize checks', function () {
        const left = app.sectionContainer.getSectionWithName('LeftSection');
        const leftRect = getSectionRectangle(left);
        assertPosSize(leftRect,
            {
                x: originX,
                y: originY,
                width: halfWidth,
                height: canvasHeight - originY
            });
    });

    it('RightSection PosSize checks', function () {
        const right = app.sectionContainer.getSectionWithName('RightSection');
        const rightRect = getSectionRectangle(right);
        assertPosSize(rightRect,
            {
                x: 1 + halfWidth + originX,
                y: originY,
                width: halfWidth - 1 - originX,
                height: canvasHeight - originY
            });
    });
});

describe('Vertically packed two section container', function() {

    beforeEach(function () {
        setupCanvasContainer(canvasWidth, canvasHeight);

        const docLayer = {};
        const tsManager = {};

        let topSection = new CanvasSectionObject('TopSection');
        topSection.anchor = ['top', 'left'];
        topSection.position = [originX, originY];
        topSection.size = [1, halfHeight];
        topSection.expand = ['right'];
        topSection.processingOrder = topSection.drawingOrder = topSection.zIndex = 1;
        topSection.interactable = false;
        topSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(topSection);

        let bottomSection = new CanvasSectionObject('BottomSection');
        bottomSection.anchor = [['TopSection', 'bottom', 'top'], 'left'] as (string[] | Array<Array<string>>);
        bottomSection.position = [originX, originY];
        bottomSection.size = [1, 1];
        bottomSection.expand = ['bottom', 'right'];
        bottomSection.processingOrder = bottomSection.drawingOrder = 2;
        bottomSection.zIndex = 1;
        bottomSection.interactable = false;
        bottomSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(bottomSection);

        app.sectionContainer.enableDrawing();
    });

    it('Container should have TopSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('TopSection'));
    });

    it('Container should have BottomSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('BottomSection'));
    });

    it('TopSection PosSize checks', function () {
        const top = app.sectionContainer.getSectionWithName('TopSection');
        const topRect = getSectionRectangle(top);
        assertPosSize(topRect,
            {
                x: originX,
                y: originY,
                width: canvasWidth - originX,
                height: halfHeight
            });
    });

    it('BottomSection PosSize checks', function () {
        const bottom = app.sectionContainer.getSectionWithName('BottomSection');
        const bottomRect = getSectionRectangle(bottom);
        assertPosSize(bottomRect,
            {
                x: originX,
                y: 1 + halfHeight + originY,
                width: canvasWidth - originX,
                height: halfHeight - 1 - originY
            });
    });
});

// '-left' layout is usually used for RTL where it is used to attach a section's right to the left of another section.
describe('Horizontally packed two section container with -left layout', function() {

    beforeEach(function () {
        setupCanvasContainer(canvasWidth, canvasHeight);

        const docLayer = {};
        const tsManager = {};

        let rightSection = new CanvasSectionObject('RightSection');
        rightSection.anchor = ['top', 'right'];
        rightSection.position = [originX, originY];
        rightSection.size = [halfWidth, 1];
        rightSection.expand = ['bottom'];
        rightSection.processingOrder = rightSection.drawingOrder = rightSection.zIndex = 1;
        rightSection.interactable = false;
        rightSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(rightSection);

        let leftSection = new CanvasSectionObject('LeftSection');
        leftSection.anchor = ['top', ['RightSection', '-left', 'right']] as (string[] | Array<Array<string>>); // Attach LeftSection's right to left of RightSection.
        leftSection.position = [originX, originY];
        leftSection.size = [1, 1];
        leftSection.expand = ['bottom', 'left'];
        leftSection.processingOrder = leftSection.drawingOrder = 2;
        leftSection.zIndex = 1;
        leftSection.interactable = false;
        leftSection.sectionProperties = {
            docLayer: docLayer,
            tsManager: tsManager,
            strokeStyle: '#c0c0c0'
        };
        app.sectionContainer.addSection(leftSection);

        app.sectionContainer.enableDrawing();
    });


    it('Container should have LeftSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('LeftSection'));
    });

    it('Container should have RightSection', function() {
        nodeassert.ok(app.sectionContainer.doesSectionExist('RightSection'));
    });

    it('LeftSection PosSize checks', function () {
        const left = app.sectionContainer.getSectionWithName('LeftSection');
        const leftRect = getSectionRectangle(left);
        assertPosSize(leftRect,
            {
                x: 0,
                y: originY,
                width: halfWidth - 1,
                height: canvasHeight - originY
            });
    });

    it('RightSection PosSize checks', function () {
        const right = app.sectionContainer.getSectionWithName('RightSection');
        const rightRect = getSectionRectangle(right);
        assertPosSize(rightRect,
            {
                x: canvasWidth - originX - halfWidth,
                y: originY,
                width: halfWidth,
                height: canvasHeight - originY
            });
    });
});

describe('Display density changes', function() {

    afterEach(function () {
        restoreMatchMedia();
        setDevicePixelRatio(1);
    });

    it('re-lays out the container when the display density changes', function () {
        installFakeMatchMedia();
        // Construct at the baseline density. onResize during setup syncs
        // app.dpiScale to it, so the container starts in step with the display.
        setDevicePixelRatio(1);
        setupCanvasContainer(canvasWidth, canvasHeight);

        let resizeCalls = 0;
        app.sectionContainer.onResize = function () { resizeCalls++; };

        // The display density moves; app.dpiScale still holds the old value
        // until a relayout catches up.
        setDevicePixelRatio(2);

        // A listener was registered while the container was constructed.
        nodeassert.ok(fakeMediaQueries.length > 0);
        fakeMediaQueries[fakeMediaQueries.length - 1].fireChange();

        nodeassert.strictEqual(resizeCalls, 1);
    });

    it('does not re-lay out when the density has not actually moved', function () {
        installFakeMatchMedia();
        setDevicePixelRatio(1);
        setupCanvasContainer(canvasWidth, canvasHeight);

        let resizeCalls = 0;
        app.sectionContainer.onResize = function () { resizeCalls++; };

        // A change event fires but the density still matches app.dpiScale.
        fakeMediaQueries[fakeMediaQueries.length - 1].fireChange();

        nodeassert.strictEqual(resizeCalls, 0);
    });
});
});
