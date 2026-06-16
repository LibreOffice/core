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
 * This file defines the engine logic for the Ruler feature, shared by both
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

// One indent marker paired with the floating tooltip that shows its value.
// The icon and label are shown only while this marker is the focused one.
interface IndentTooltip {
	id: string;
	marker: HTMLElement;
	iconName: string;
	label: string;
	tip: ValueTooltip;
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
	_map: ReturnType<typeof window.L.map>;

	_updateTask: TaskId | null = null;

	// One value tooltip per indent marker, shown together on hover/drag.
	private _indentTooltips: IndentTooltip[] = [];
	// Value tooltip for each margin handle, keyed by the handle element.
	private _marginTooltips: Map<
		HTMLElement,
		{ tip: ValueTooltip; label: string; iconName: string }
	> = new Map();

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

	// Swap an indent marker between its resting icon and its filled icon while
	// it is grabbed, so the dragged marker stands out as a solid darker handle.
	// The base icon name is read from the marker's data-indent-icon attribute.
	// The resting icon and its dark-theme handling are set up once through
	// LOUtil.setImage; this only overrides the image for the duration of a drag,
	// still resolving the path through getImageURL so the dark variant is used.
	protected _setIndentMarkerGrabbed(
		marker: HTMLElement,
		grabbed: boolean,
	): void {
		const base = marker.dataset.indentIcon;
		if (!base) return;
		const name = base + (grabbed ? '_filled' : '') + '.svg';
		marker.style.backgroundImage =
			'url("' + app.LOUtil.getImageURL(name) + '")';
	}

	// ---- Indent value tooltips (shared by both rulers) ----------------------
	//
	// Each indent marker gets a floating value tooltip. Hovering or dragging a
	// marker shows all of them at once, with the active marker focused (icon +
	// label + value) and the others as plain value chips. Subclasses supply the
	// marker set, the value sources and the placement.

	// Create one value tooltip per marker and wire hover to show them. Call this
	// from the subclass once its markers exist.
	protected _initIndentTooltips(
		markers: {
			id: string;
			marker: HTMLElement;
			iconName: string;
			label: string;
		}[],
	): void {
		this._destroyIndentTooltips();
		this._indentTooltips = markers.map((m) => ({
			...m,
			tip: new ValueTooltip(),
		}));
		this._indentTooltips.forEach((spec) => {
			window.L.DomEvent.on(
				spec.marker,
				'mouseenter',
				() => this._showIndentTooltips(spec.id),
				this,
			);
			window.L.DomEvent.on(
				spec.marker,
				'mouseleave',
				() => {
					if (!this._map.rulerActive) this._hideIndentTooltips();
				},
				this,
			);
		});
	}

	protected _destroyIndentTooltips(): void {
		this._indentTooltips.forEach((s) => s.tip.destroy());
		this._indentTooltips = [];
	}

	_showIndentTooltips(focusedId: string): void {
		if (!this._indentTooltips.length || !this._map.isEditMode()) return;
		// Resting values come from engine; the focused marker previews its live
		// value from the moving marker position while a drag is in progress.
		const display = this._indentDisplayValuesMm100();
		if (!display) return;
		const live = this._map.rulerActive ? this._indentLiveValuesMm100() : null;
		this._indentTooltips.forEach((spec) => {
			let mm100 = display[spec.id];
			if (live && spec.id === focusedId) mm100 = live[spec.id];
			spec.tip.render({
				focused: spec.id === focusedId,
				iconName: spec.iconName,
				label: spec.label,
				value: this._formatIndentValue(mm100),
			});
			spec.tip.show();
		});
		// Width and height are known only once shown, so place after showing.
		this._positionIndentTooltips(this._indentTooltips);
	}

	_hideIndentTooltips(): void {
		this._indentTooltips.forEach((s) => s.tip.hide());
	}

	// Indent value in centimetres, no unit suffix (the chips show just a
	// number). The ruler ticks are always centimetres.
	protected _formatIndentValue(mm100: number): string {
		return (mm100 / 1000).toFixed(2);
	}

	// Resting indent values in mm100, keyed by marker id, as reported by engine.
	protected abstract _indentDisplayValuesMm100(): { [id: string]: number };
	// Live indent values in mm100 derived from the current marker positions,
	// used to preview the value of the marker being dragged.
	protected abstract _indentLiveValuesMm100(): { [id: string]: number };
	// Place each marker's tooltip relative to its marker and the ruler. The
	// geometry differs between the horizontal and vertical rulers.
	protected abstract _positionIndentTooltips(tooltips: IndentTooltip[]): void;

	// ---- Margin handle tooltips (shared by both rulers) ---------------------

	// Format an mm100 value as a 2-decimal centimetre string with the unit. The
	// ruler ticks are always drawn in centimetres, so the value is shown in cm
	// regardless of the document's measurement unit.
	_formatRulerValue(mm100: number): string {
		return (mm100 / 1000).toFixed(2) + ' cm';
	}

	// The margin equals the drag handle's own width. This avoids depending on
	// page margin options that can be undefined.
	_currentMarginMm100(elem: HTMLElement): number {
		const widthPx = parseFloat(elem.style.width) || 0;
		return widthPx / this.options.DraggableConvertRatio;
	}

	// Attach a value tooltip to a margin handle. It appears at the pointer on
	// hover and stays put, and remains visible while the handle is dragged.
	protected _setupMarginTooltip(
		elem: HTMLElement,
		label: string,
		iconName: string,
	): void {
		const tip = new ValueTooltip();
		this._marginTooltips.set(elem, { tip, label, iconName });
		window.L.DomEvent.on(
			elem,
			'mouseenter',
			(e: MouseEvent) => {
				this._renderMarginTooltip(elem);
				tip.show();
				tip.placeNearPoint(e.clientX, e.clientY);
				// Moving onto a margin handle hides the indent tooltips so the
				// two do not overlap.
				this._hideIndentTooltips();
			},
			this,
		);
		window.L.DomEvent.on(
			elem,
			'mouseleave',
			() => {
				if (!this._map.rulerActive) tip.hide();
			},
			this,
		);
	}

	private _renderMarginTooltip(elem: HTMLElement): void {
		const m = this._marginTooltips.get(elem);
		if (!m) return;
		m.tip.render({
			focused: true,
			iconName: m.iconName,
			label: m.label,
			value: this._formatRulerValue(this._currentMarginMm100(elem)),
		});
	}

	// Show a margin handle's tooltip as a drag starts. It keeps the position set
	// when the pointer entered the handle.
	_pinMarginTooltip(elem: HTMLElement): void {
		const m = this._marginTooltips.get(elem);
		if (!m) return;
		this._renderMarginTooltip(elem);
		m.tip.show();
	}

	// Refresh the live value on a margin handle's tooltip during a drag.
	_updateMarginTooltip(elem: HTMLElement): void {
		this._renderMarginTooltip(elem);
	}

	_hideMarginTooltips(): void {
		this._marginTooltips.forEach((m) => m.tip.hide());
	}

	protected _destroyMarginTooltips(): void {
		this._marginTooltips.forEach((m) => m.tip.destroy());
		this._marginTooltips.clear();
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
