/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
interface PainterInterface {
	update(): void;
	_addTilePixelGridSection(): void;
	_removeTilePixelGridSection(): void;
	_addPreloadMap(): void;
	_removePreloadMap(): void;
	_addSplitsSection(): void;
	_removeSplitsSection(): void;
	_addDebugOverlaySection(): void;
	_removeDebugOverlaySection(): void;
	_paintContext(): {
		viewBounds: cool.Bounds;
		paneBoundsList: cool.Bounds[];
		paneBoundsActive: boolean;
		splitPos: cool.Point;
	};
}

type MapUpdaterType = (newMapCenterLatLng: LatLngLike) => void;

interface DocLayerInterface {
	_toolbarCommandValues: any;

	_getViewId(): string;
	_painter: PainterInterface;
	_docType: string;
	_coreMousePointer: string;

	isCalc(): boolean;
	isWriter(): boolean;
	isImpress(): boolean;
	isCalcRTL(): boolean;

	_pixelsToTwips(cssPix: cool.PointLike): cool.PointLike;
	_latLngToTwips(latlng: LatLngLike): cool.Point;

	_postMouseEvent(
		typ: string,
		x: number,
		y: number,
		count: number,
		buttons: number,
		modifier: number,
	): void;
	postKeyboardEvent(typ: string, charCode: number, unoKeyCode: number): void;

	filterSlurpedMessage(e: SlurpMessageEvent): boolean;
	_documentInfo?: string;

	removeAllViews(): void;
	_resetClientVisArea(): void;

	_onMessage(textMsg: string, img?: CoolHTMLImageElement): void;
	_resetCanonicalIdStatus(): void;
	_resetViewId(): void;
	_resetDocumentInfo(): void;

	options: {
		tileWidthTwips: number;
	};

	sheetGeometry?: cool.SheetGeometry;
	_cellSelectionArea?: cool.SimpleRectangle;
	scrollToPos(pos: { x: number; y: number } | LatLngLike): void;

	_selectedPart: number;
	_oleCSelections: CSelections;
	_shapeGridOffset: cool.SimplePoint;
	getFiledBasedViewVerticalOffset(): number;
	focus(acceptInput?: boolean): void;
	_updateCursorAndOverlay(): void;
	_twipsToPixels(twips: cool.Point): cool.Point;
	_allowViewJump(): boolean;
	_selectionContentRequest?: TimeoutHdl;
	recalculateZoomOnResize(): void;
	_splitPanesContext?: cool.SplitPanesContext;
	getSplitPanesContext(): cool.SplitPanesContext | undefined;
	_moveInProgress: boolean;
	_moveTileRequests: string[];
	_debug: DebugManager;
	_canonicalIdInitialized: boolean;
	_gotFirstCellCursor?: boolean;
	_sendClientZoom(forceUpdate?: boolean): void;
	_emptyDeltaDiv?: HTMLDivElement;
	_partHeightTwips: number;
	_isZooming: boolean;
	_spaceBetweenParts: number;
	_partWidthTwips: number;
	_parts: number;
	_partDimensions?: cool.SimplePoint[];
	_fitWidthZoom(
		e?: { oldSize: number; newsize: number },
		maxZoom?: number,
		recalcFirstFit?: boolean,
	): void;
	_corePixelsToTwips(corePixels: { x: number; y: number }): cool.Point;
	// Is this still in user?
	_ySplitter: any;
	_xSplitter: any;
	_cursorMarker?: Cursor;
	_syncTileContainerSize(force?: boolean): boolean;
	_postSelectTextEvent(type: string, x: number, y: number): void;
	_viewId: number;
	_openCommentWizard(annotation?: cool.Comment): void;
	_parseCellRange(cellRange: string): cool.Bounds;
	_cellRangeToTwipRect(cellRange: cool.Bounds): cool.Bounds;
	preZoomAnimation(pinchStartCenter: LatLngLike): void;
	zoomStep(zoom: number, newCenter: LatLngLike): void;
	zoomStepEnd(
		zoom: number,
		newCenter: LatLngLike,
		mapUpdater: MapUpdaterType,
		runAtFinish: () => void,
		noGap?: boolean,
	): void;
	postZoomAnimation(): void;
	_checkSelectedPart(): void;
	requestNewFiledBasedViewTiles(): void;
	_docPixelSize: cool.PointLike;
	_closeMobileWizard(): void;
	_openMobileWizard(data: WidgetJSON): void;

	// TODO: window.L.Control.PartsPreview.
	_preview?: any;
	getCommentWizardStructure(
		menuStructure?: WidgetJSON,
		onlyThread?: any,
	): WidgetJSON;

	_resetReferencesMarks(type?: string): void;
	_clearReferences(): void;
	_clearSelections(calledFromSetPartHandler?: boolean): void;
}
