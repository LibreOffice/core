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

describe('OtherViewCursorSection', function() {

describe('Text cursor of another view', function () {
	const canvasWidth = 1024;
	const canvasHeight = 768;

	// The default scale: one core pixel is 15 twips.
	const twipsPerPixel = 15;

	const otherViewId = 2;

	// Column A of the cursor's cell sits at 3000 twips (200 core pixels) and its
	// row at 6000 twips (400 core pixels). The cell is 300 twips high.
	const cursorX = 3000;
	const cursorY = 6000;
	const cursorHeight = 300;

	let layout: ViewLayoutBase;

	function initializeJSDOM() {
		addCanvasToDom();
		addMockCanvas(window);
		global.requestAnimationFrame = window.requestAnimationFrame;
		global.cancelAnimationFrame = window.cancelAnimationFrame;
	}

	// Values mirror browser/src/canvas/CanvasSectionProps.js, which the mocha
	// bundle does not carry.
	function setupSectionProps() {
		(app.CSections as any).DefaultForDocumentObjects = {
			zIndex: 9, processingOrder: 10, drawingOrder: 10,
		};
		(app.CSections as any).HTMLObject = {
			zIndex: 9, processingOrder: 1, drawingOrder: 55,
		};
	}

	// A spreadsheet on its first sheet, with a second view the client knows about.
	function setupAppStubs() {
		app.pixelsToTwips = twipsPerPixel;
		app.twipsToPixels = 1 / twipsPerPixel;

		// The reading user edits nothing, so no cursor of their own stands where
		// the other view's cursor is.
		(app as any).file = { textCursor: { visible: false, rectangle: null } };

		app.map = {
			on: function () {},
			off: function () {},
			isViewReadOnly: function () { return false; },
			_debug: null,
			_docLayer: {
				_docType: 'spreadsheet',
				isWriter: function () { return false; },
				_selectedPart: 0,
			},
		} as any;
	}

	// The grid area starts at the canvas origin, so a document position and its
	// view position differ by the scroll offset alone.
	function setupDocumentAnchor() {
		const anchorSection = new CanvasSectionObject('document anchor');
		anchorSection.anchor = ['top', 'left'];
		anchorSection.position = [0, 0];
		anchorSection.size = [canvasWidth, canvasHeight];
		anchorSection.processingOrder = 1;
		anchorSection.drawingOrder = 1;
		anchorSection.zIndex = 1;
		anchorSection.interactable = false;

		app.sectionContainer.addSection(anchorSection);
		app.sectionContainer.setDocumentAnchorSection('document anchor');
		app.sectionContainer.reNewAllSections(false);
	}

	function setupDocument() {
		layout = new ViewLayoutBase();

		app.activeDocument = {
			activeLayout: layout,
			isModeActive: function (mode: number) { return mode === 0; },
		} as any;

		setupDocumentAnchor();

		// The view starts at the top left corner of the sheet.
		layout.viewedRectangle = new cool.SimpleRectangle(
			0, 0, canvasWidth * twipsPerPixel, canvasHeight * twipsPerPixel);
	}

	// The cursor of the other view, as an incoming cursor position creates it.
	function addOtherViewCursor(): TextCursorSection {
		const rectangle = new cool.SimpleRectangle(cursorX, cursorY, 0, cursorHeight);
		rectangle.pWidth = 2 * app.dpiScale;

		const section = new TextCursorSection(
			otherViewId, '#ff0000', rectangle, 0, 0);

		app.sectionContainer.addSection(section);
		return section;
	}

	// The place the cursor was given in the document, in CSS pixels. The write is
	// scheduled, so the pending layouting work is done first.
	function placeInDocument(section: TextCursorSection): number[] {
		while (app.layoutingService.hasTasksPending())
			app.layoutingService.runTheTopTask();

		const style = section.getHTMLObject().style;
		return [parseInt(style.left), parseInt(style.top)];
	}

	this.beforeAll(initializeJSDOM);

	this.beforeEach(function () {
		setupCanvasContainer(canvasWidth, canvasHeight);
		setupSectionProps();
		setupAppStubs();
		setupDocument();
	});

	it('is drawn for the area the view shows when the cursor is shown again', function () {
		const section = addOtherViewCursor();

		nodeassert.deepStrictEqual(section.myTopLeft, [200, 400]);

		// A zoom frame moves the viewed rectangle on its own, without the section
		// container placing the document objects again. The view now starts 3000
		// twips (200 core pixels) further down the sheet.
		layout.setZoomFrameViewedRectangle(new cool.SimpleRectangle(
			0, 3000, canvasWidth * twipsPerPixel, canvasHeight * twipsPerPixel));

		// The other view reports that its cursor is visible, which is where the
		// client decides what to show and where.
		section.sectionProperties.showCursor = true;
		section.applyVisibility();

		nodeassert.ok(section.showSection);
		nodeassert.ok(section.isVisible);
		nodeassert.deepStrictEqual(section.myTopLeft, [200, 200]);
		nodeassert.deepStrictEqual(placeInDocument(section), [200, 200]);
	});
});

});
