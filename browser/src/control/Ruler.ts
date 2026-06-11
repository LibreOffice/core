// @ts-strict-ignore
/**
 /*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Ruler.ts - Base Class for Ruler Functionality
 *
 * This file defines the core logic for the Ruler feature, shared by both
 * horizontal (HRuler) and vertical (VRuler) implementations.
 *
 * ### Key Features:
 * - **Configurable Options**: Supports margins, tab stops, interactivity, and display settings.
 * - **Abstract Design**: Subclasses must implement `_updateBreakPoints()` for specific behaviors.
 * - **Shared Utilities**: Includes reusable methods like `_updatePaintTimer` and `getWindowProperty`.
 * - **Easy Initialization**: `initializeRuler` simplifies setup for HRuler and VRuler.
 *
 * ### Guidelines:
 * - Extend the `Options` interface for new features.
 * - Implement subclass-specific logic in `_updateBreakPoints()`.
 * - Ensure event handling (e.g., `rulerchanged`) for updates.
 */

interface CustomHTMLDivElement extends HTMLDivElement {
	tabStops?: any;
}

interface TabStopLocation {
	left: number;
	right: number;
	center: number;
}

interface TabStop {
	tabStopLocation: TabStopLocation;
	tabStopNumber: number;
}

interface TabStopContainer {
	tabStops: TabStop[];
}

interface Params {
	[key: string]: {
		type: string;
		value: any;
	};
}

interface TabstopObject {
	position: number;
	style: number;
}

interface Options {
	interactive: boolean;
	marginSet: boolean;
	displayNumber: boolean;
	tileMargin: number;
	margin1: number | null;
	margin2: number | null;
	leftOffset: number | null;
	pageOffset: number | null;
	pageWidth: number | null;
	pageTopMargin: number | null;
	pageBottomMargin: number | null;
	firstLineIndent: number | null;
	leftParagraphIndent: number | null;
	rightParagraphIndent: number | null;
	tabs: TabstopObject[] | string;
	tabsRelativeToIndent: boolean;
	unit: string | null;
	DraggableConvertRatio: number | null;
	timer: ReturnType<typeof setTimeout>;
	showruler: boolean;
	position: string;
	disableMarker: boolean;
}

abstract class Ruler {
	protected options: Options;

	_updateTask: TaskId | null = null;

	constructor(options: Partial<Options>) {
		// Init default values for ruler options
		this.options = {
			interactive: true,
			marginSet: false,
			displayNumber: true,
			tileMargin: 18,
			margin1: null,
			margin2: null,
			leftOffset: null,
			pageOffset: null,
			pageWidth: null,
			pageTopMargin: null,
			pageBottomMargin: null,
			firstLineIndent: null,
			leftParagraphIndent: null,
			rightParagraphIndent: null,
			tabs: '',
			tabsRelativeToIndent: false,
			unit: null,
			DraggableConvertRatio: null,
			timer: null,
			showruler: true,
			position: 'topleft',
			disableMarker: false,
			...options,
		};
	}

	// Abstract method: Must be implemented by subclasses
	protected abstract _updateBreakPoints(): void;

	_updatePaintTimer() {
		clearTimeout(this.options.timer);
		this.options.timer = setTimeout(
			window.L.bind(this._updateBreakPoints, this),
			300,
		);
	}

	protected abstract _updateParagraphIndentationsImpl(): void;

	public _updateParagraphIndentations() {
		if (this._updateTask)
			app.layoutingService.cancelLayoutingTask(this._updateTask);
		this._updateTask = app.layoutingService.appendLayoutingTask(
			this._updateParagraphIndentationsImpl.bind(this),
		);
	}

	getWindowProperty<T>(propertyName: string): T | undefined {
		return (window as any)[propertyName];
	}

	// Static method to handle the initialization of rulers
	static initializeRuler(
		map: ReturnType<typeof window.L.map>,
		options: Partial<Options>,
	) {
		const isRTL = document.documentElement.dir === 'rtl';
		const interactiveRuler = map.isEditMode();
		const showRuler = options.showruler ?? true;

		// Initialize the horizontal ruler
		new HRuler(map, {
			position: isRTL ? 'topright' : 'topleft',
			interactive: interactiveRuler,
			showruler: showRuler,
		});

		// Initialize the vertical ruler if not in presentation or drawing mode
		if (!map.isPresentationOrDrawing()) {
			new VRuler(map, {
				position: isRTL ? 'topright' : 'topleft',
				interactive: interactiveRuler,
				showruler: showRuler,
			});
		}

		if (showRuler) map.uiManager.showRuler();
	}

	public fixOffset() {
		app.layoutingService.appendLayoutingTask(() => {
			this._fixOffsetImpl();
		});
	}

	protected abstract _fixOffsetImpl(): void;
}

app.definitions.ruler = Ruler;
