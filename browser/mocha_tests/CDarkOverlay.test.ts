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

describe('CDarkOverlay', function () {

class ResizeObserver {
	constructor(f: Function) {}

	disconnect() {}

	observe() {}

	unobserve() {}
}

function initializeJSDOM() {
	addCanvasToDom();
	addMockCanvas(window);
	global.requestAnimationFrame = window.requestAnimationFrame;
	global.cancelAnimationFrame = window.cancelAnimationFrame;
	global.getComputedStyle = window.getComputedStyle.bind(window);
	global.ResizeObserver = ResizeObserver;

	setupCanvasContainer(1024, 768);
}

describe('Dimming around a chart activated for in-place editing', function () {
	this.beforeAll(initializeJSDOM);

	// The frame core reports for a chart that has just been activated for in-place
	// editing, in twips, and the message that carries it.
	const chartFrame = { x: 1500, y: 3000, width: 6000, height: 4500 };
	const inPlaceMessage = 'graphicselection: 1500, 3000, 6000, 4500, "INPLACE"';

	// 15 twips per pixel is the convention at the default zoom level. Halving it
	// stands for a zoom in that doubles every pixel coordinate.
	const defaultScale = 15;
	const zoomedInScale = 7.5;

	function setZoomScale(pixelsToTwips: number): void {
		app.pixelsToTwips = pixelsToTwips;
		app.twipsToPixels = 1 / pixelsToTwips;
	}

	// A CSelections of type 'ole' owns the four dimming rectangles. The stub overlay
	// leaves every path without a renderer, which keeps them pure geometry, and the
	// 'ole' styling reads no CSS properties, so an empty div satisfies the style
	// lookup.
	function createOleSelections(): CSelections {
		const overlay: any = {
			initPathGroup: function () {},
			removePathGroup: function () {},
		};

		return new CSelections(
			undefined,
			overlay,
			document.createElement('div'),
			app.map,
			false /* isView */,
			undefined /* viewId */,
			'ole',
		);
	}

	function setupAppStubs(): void {
		setZoomScale(defaultScale);

		app.map = {
			_docLayer: {
				_docType: 'spreadsheet',
				isCalcRTL: function () { return false; },
				// The frame arrives in print twips. Keeping this conversion a no-op
				// leaves it in the twips the message carried, so the expected pixel
				// values follow from the message alone.
				sheetGeometry: { convertToTileTwips: function () {} },
				_updateCursorAndOverlay: function () {},
				_onMessage: function () {},
				_oleCSelections: createOleSelections(),
			},
		} as any;

		GraphicSelection.rectangle = null;
		GraphicSelection.extraInfo = null;
	}

	this.beforeEach(setupAppStubs);

	function dimmingBands(): CRectangle[] {
		const selections: any = app.map._docLayer._oleCSelections;
		return selections._selection.rectangles;
	}

	function bandCount(): number {
		let count = 0;
		const selections: any = app.map._docLayer._oleCSelections;
		selections._selection.forEach(function () { count++; });
		return count;
	}

	// The four bands cover the sheet above, left of, right of and below the chart, so
	// the area they leave alone runs from the right edge of the left band and the
	// bottom edge of the top band to the left edge of the right band and the top edge
	// of the bottom band.
	function undimmedArea(): cool.Bounds {
		const bands = dimmingBands();
		const top = bands[0].getBounds();
		const left = bands[1].getBounds();
		const right = bands[2].getBounds();
		const bottom = bands[3].getBounds();

		return new cool.Bounds(
			new cool.Point(left.max.x, top.max.y),
			new cool.Point(right.min.x, bottom.min.y),
		);
	}

	function chartFrameInPixels(): cool.Bounds {
		const scale = app.twipsToPixels;
		return new cool.Bounds(
			new cool.Point(chartFrame.x * scale, chartFrame.y * scale),
			new cool.Point(
				(chartFrame.x + chartFrame.width) * scale,
				(chartFrame.y + chartFrame.height) * scale,
			),
		);
	}

	function assertUndimmedAreaMatchesChart(message: string): void {
		const actual = undimmedArea();
		const expected = chartFrameInPixels();

		nodeassert.deepStrictEqual(
			[actual.min.x, actual.min.y, actual.max.x, actual.max.y],
			[expected.min.x, expected.min.y, expected.max.x, expected.max.y],
			message,
		);
	}

	it('the undimmed area matches the chart when in-place editing starts', function () {
		GraphicSelection.onMessage(inPlaceMessage);

		assertUndimmedAreaMatchesChart('undimmed area at the starting zoom');
	});

	it('the undimmed area follows the chart when the zoom changes', function () {
		GraphicSelection.onMessage(inPlaceMessage);

		// Core does not resend coordinates for a zoom change, so the stored in-place
		// message is replayed once the new zoom is in force.
		setZoomScale(zoomedInScale);
		GraphicSelection.onMessage(inPlaceMessage);

		assertUndimmedAreaMatchesChart('undimmed area after zooming in');
	});

	it('the undimmed area follows the chart when the zoom changes repeatedly', function () {
		GraphicSelection.onMessage(inPlaceMessage);

		const scales = [zoomedInScale, defaultScale, 30, zoomedInScale];
		for (let index = 0; index < scales.length; index++) {
			setZoomScale(scales[index]);
			GraphicSelection.onMessage(inPlaceMessage);

			assertUndimmedAreaMatchesChart(
				'undimmed area at ' + scales[index] + ' twips per pixel',
			);
		}
	});

	it('the dimming stays four bands however often it is redrawn', function () {
		GraphicSelection.onMessage(inPlaceMessage);
		nodeassert.strictEqual(bandCount(), 4, 'bands when editing starts');

		const scales = [zoomedInScale, defaultScale, 30];
		for (let index = 0; index < scales.length; index++) {
			setZoomScale(scales[index]);
			GraphicSelection.onMessage(inPlaceMessage);
		}

		nodeassert.strictEqual(bandCount(), 4, 'bands after several zoom changes');
	});

	it('leaving in-place editing removes the dimming', function () {
		GraphicSelection.onMessage(inPlaceMessage);
		nodeassert.ok(
			GraphicSelection.hasDarkOverlay(),
			'the chart is dimmed while it is edited in place',
		);

		GraphicSelection.onMessage('graphicselection: INPLACE EXIT');

		nodeassert.ok(
			!GraphicSelection.hasDarkOverlay(),
			'the dimming is gone once editing ends',
		);
	});
});

});
