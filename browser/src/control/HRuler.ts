// @ts-strict-ignore
/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Ruler Handler
/*
 * Ruler Handler

 * HRuler.ts
 *
 * Manages the horizontal ruler for displaying measurements and positioning.
 * Handles user interactions like scrolling and dragging, and renders grid lines/ticks.
 */

declare var JSDialog: any;

class HRuler extends Ruler {
	_firstLineMarker: HTMLDivElement;
	_pStartMarker: HTMLDivElement;
	_pEndMarker: HTMLDivElement;
	_rFace: HTMLDivElement;
	_markerVerticalLine: HTMLDivElement;
	_rWrapper: HTMLDivElement;
	_rMarginWrapper: HTMLDivElement;
	_rBPWrapper: HTMLDivElement;
	_rBPContainer: HTMLDivElement;
	_rTSContainer: CustomHTMLDivElement;
	_lMarginMarker: HTMLDivElement;
	_rMarginMarker: HTMLDivElement;
	_lMarginDrag: HTMLDivElement;
	_rMarginDrag: HTMLDivElement;
	_lToolTip: HTMLDivElement;
	_rToolTip: HTMLDivElement;

	// Hammer variable declaration
	_hammer: any;
	_firstLineHammer: any;
	_pEndHammer: any;
	_pStartHammer: any;

	_indentationElementId: string;
	_initialposition: number;
	_lastposition: number;
	currentPositionInTwips: number | null;
	currentTabStopIndex: number | null;
	_map: ReturnType<typeof window.L.map>;

	options: Options;

	constructor(map: ReturnType<typeof window.L.map>, options: Partial<Options>) {
		super(options);
		this._map = map;
		Object.assign(this.options, options);
		this.onAdd(); // VRuler created

		if (app.map._docLayer._docType === 'presentation')
			this.options.tileMargin = 0;

		app.UI.horizontalRuler = this;
	}

	onAdd() {
		this._map.on('rulerupdate', this._updateOptions, this);
		this._map.on('tabstoplistupdate', this._updateTabStops, this);
		this._map.on('scrolllimits', this._updatePaintTimer, this);
		this._map.on('moveend fixruleroffset', this.fixOffset, this);
		this._map.on('updatepermission', this._changeInteractions, this);
		window.L.DomUtil.addClass(this._map.getContainer(), 'hasruler');

		const container: HTMLDivElement = this._initLayout();
		const corner: HTMLElement =
			this._map._controlCorners[this.options.position];

		window.L.DomUtil.addClass(container, 'leaflet-control');

		if (this.options.position.indexOf('bottom') !== -1) {
			corner.insertBefore(container, corner.firstChild);
		} else {
			corner.appendChild(container);
		}
	}

	onRemove() {
		this._map.off('rulerupdate', this._updateOptions, this);
		this._map.off('tabstoplistupdate', this._updateTabStops, this);
		this._map.off('scrolllimits', this._updatePaintTimer, this);
		this._map.off('moveend fixruleroffset', this.fixOffset, this);
		this._map.off('updatepermission', this._changeInteractions, this);
	}

	_changeInteractions(e: any) {
		if (this._lMarginDrag) {
			if (e.perm === 'edit') {
				this._lMarginDrag.style.cursor = 'e-resize';
				this._rMarginDrag.style.cursor = 'w-resize';

				if (!this.getWindowProperty<boolean>('ThisIsTheiOSApp')) {
					window.L.DomEvent.on(
						this._rMarginDrag,
						'mousedown',
						this._initiateDrag,
						this,
					);
					window.L.DomEvent.on(
						this._lMarginDrag,
						'mousedown',
						this._initiateDrag,
						this,
					);
				}
			} else {
				this._lMarginDrag.style.cursor = 'default';
				this._rMarginDrag.style.cursor = 'default';

				if (!this.getWindowProperty<boolean>('ThisIsTheiOSApp')) {
					window.L.DomEvent.off(
						this._rMarginDrag,
						'mousedown',
						this._initiateDrag,
						this,
					);
					window.L.DomEvent.off(
						this._lMarginDrag,
						'mousedown',
						this._initiateDrag,
						this,
					);
				}
			}
		}
	}

	public show() {
		this._rFace.parentElement.style.display = '';
		this._updateParagraphIndentations();
	}

	public hide() {
		this._rFace.parentElement.style.display = 'none';
	}

	_initiateIndentationMarkers() {
		// First line indentation..
		this._firstLineMarker = document.createElement('div');
		this._firstLineMarker.id = 'lo-fline-marker';
		this._firstLineMarker.classList.add('cool-ruler-indentation-marker-down');
		this._rFace.appendChild(this._firstLineMarker);

		// Paragraph indentation..
		this._pStartMarker = document.createElement('div');
		this._pStartMarker.id = 'lo-pstart-marker';
		this._pStartMarker.classList.add('cool-ruler-indentation-marker-up');
		this._rFace.appendChild(this._pStartMarker);

		// Paragraph end..
		this._pEndMarker = document.createElement('div');
		this._pEndMarker.id = 'lo-pend-marker';
		this._pEndMarker.classList.add('cool-ruler-indentation-marker-up');
		this._rFace.appendChild(this._pEndMarker);

		// While one of the markers is being dragged, a vertical line should be visible in order to indicate the new position of the marker..
		this._markerVerticalLine = window.L.DomUtil.create(
			'div',
			'cool-ruler-indentation-marker-center',
		);
		this._rFace.appendChild(this._markerVerticalLine);

		// Now we have indentation markers. Next we should bind drag initializers to them..
		// We will use 3 hammers. 1 hammer is not usable for this case.
		// Hammer for first line indentation..
		this._firstLineHammer = new Hammer(this._firstLineMarker);
		this._firstLineHammer.add(new Hammer.Pan({ threshold: 0, pointers: 0 }));
		this._firstLineHammer.get('press').set({
			time: 500,
		});
		this._firstLineHammer.on(
			'panstart',
			window.touch.touchOnly((event) => {
				this._initiateIndentationDrag(event);
			}),
		);
		this._firstLineHammer.on(
			'panmove',
			window.touch.touchOnly((event) => {
				this._moveIndentation(event);
			}),
		);
		this._firstLineHammer.on(
			'panend',
			window.touch.touchOnly((event) => {
				this._moveIndentationEnd(event as Event);
			}),
		);

		// Hammer for paragraph start indentation..
		this._pStartHammer = new Hammer(this._pStartMarker);
		this._pStartHammer.add(new Hammer.Pan({ threshold: 0, pointers: 0 }));
		this._pStartHammer.get('press').set({
			time: 500,
		});
		this._pStartHammer.on(
			'panstart',
			window.touch.touchOnly((event) => {
				this._initiateIndentationDrag(event);
			}),
		);
		this._pStartHammer.on(
			'panmove',
			window.touch.touchOnly((event) => {
				this._moveIndentation(event);
			}),
		);
		this._pStartHammer.on(
			'panend',
			window.touch.touchOnly((event) => {
				this._moveIndentationEnd(event as Event);
			}),
		);

		// Hammer for paragraph end indentation..
		this._pEndHammer = new Hammer(this._pEndMarker);
		this._pEndHammer.add(new Hammer.Pan({ threshold: 0, pointers: 0 }));
		this._pEndHammer.get('press').set({
			time: 500,
		});
		this._pEndHammer.on(
			'panstart',
			window.touch.touchOnly((event) => {
				this._initiateIndentationDrag(event);
			}),
		);
		this._pEndHammer.on(
			'panmove',
			window.touch.touchOnly((event) => {
				this._moveIndentation(event);
			}),
		);
		this._pEndHammer.on(
			'panend',
			window.touch.touchOnly((event) => {
				this._moveIndentationEnd(event as Event);
			}),
		);

		window.L.DomEvent.on(
			this._firstLineMarker,
			'mousedown',
			window.touch.mouseOnly(this._initiateIndentationDrag),
			this,
		);
		window.L.DomEvent.on(
			this._pStartMarker,
			'mousedown',
			window.touch.mouseOnly(this._initiateIndentationDrag),
			this,
		);
		window.L.DomEvent.on(
			this._pEndMarker,
			'mousedown',
			window.touch.mouseOnly(this._initiateIndentationDrag),
			this,
		);
	}

	_initLayout() {
		this._rWrapper = window.L.DomUtil.create(
			'div',
			'cool-ruler leaflet-bar leaflet-control leaflet-control-custom',
		);
		this._rWrapper.setAttribute('aria-hidden', 'true');
		this._rWrapper.style.visibility = 'hidden';

		// We start it hidden rather than not initialzing at all.
		// It is due to rulerupdate command that comes from COKit.
		// If we delay its initialization, we can't calculate its margins and have to wait for another rulerupdate message to arrive.
		if (!this.options.showruler) {
			window.L.DomUtil.setStyle(this._rWrapper, 'display', 'none');
		}
		this._rFace = window.L.DomUtil.create(
			'div',
			'cool-ruler-face',
			this._rWrapper,
		);
		this._rMarginWrapper = window.L.DomUtil.create(
			'div',
			'cool-ruler-marginwrapper',
			this._rFace,
		);
		// BP => Break Points
		this._rBPWrapper = window.L.DomUtil.create(
			'div',
			'cool-ruler-breakwrapper',
			this._rFace,
		);
		this._rBPContainer = window.L.DomUtil.create(
			'div',
			'cool-ruler-breakcontainer',
			this._rBPWrapper,
		);

		// Tab stops
		this._rTSContainer = window.L.DomUtil.create(
			'div',
			'cool-ruler-horizontal-tabstopcontainer',
			this._rMarginWrapper,
		);
		window.L.DomEvent.on(
			this._rTSContainer,
			'mousedown',
			this._initiateTabstopDrag,
			this,
		);

		this._rTSContainer.ondblclick = (e) => {
			const offset = this._rTSContainer.getBoundingClientRect().left;

			var position = this._map._docLayer._pixelsToTwips({
				x: e.clientX - offset,
				y: 0,
			}).x;

			this.currentPositionInTwips = position;

			this._insertTabstop();
		};

		this._hammer = new Hammer(this._rTSContainer);
		this._hammer.add(new Hammer.Pan({ threshold: 0, pointers: 0 }));
		this._hammer.get('press').set({
			time: 500,
		});
		this._hammer.on(
			'panstart',
			(window as typeof window & { touch: any }).touch.touchOnly(function (
				event: any,
			) {
				this._initiateTabstopDrag(event);
			}),
		);
		this._hammer.on(
			'panmove',
			(window as typeof window & { touch: any }).touch.touchOnly(function (
				event: any,
			) {
				this._moveTabstop(event);
			}),
		);
		this._hammer.on(
			'panend',
			(window as typeof window & { touch: any }).touch.touchOnly(function (
				event: any,
			) {
				this._endTabstopDrag(event);
			}),
		);
		this._hammer.on(
			'press',
			(window as typeof window & { touch: any }).touch.touchOnly(function (
				event: any,
			) {
				this._onTabstopContainerLongPress(event);
			}),
		);

		this._initiateIndentationMarkers();

		return this._rWrapper;
	}

	_updateOptions(obj: Options) {
		// window.app.console.log('===> _updateOptions');
		// Note that the values for margin1, margin2 and leftOffset are not in any sane
		// units. See the comment in SwCommentRuler::CreateJsonNotification(). The values
		// are pixels for some virtual device in core, not related to the actual device this
		// is running on at all, passed through convertTwipToMm100(), i.e. multiplied by
		// approximately 1.76. Let's call these units "arbitrary pixelish units" in
		// comments here.
		this.options.margin1 = obj['margin1'];
		this.options.margin2 = obj['margin2'];
		this.options.leftOffset = obj['leftOffset'];

		// pageWidth on the other hand *is* really in mm100.
		this.options.pageWidth = obj['pageWidth'];

		// to be enabled only after adding support for other length units as well
		// this.options.unit = obj['unit'].trim();

		this._updateBreakPoints();

		this._rWrapper.style.visibility = '';
	}

	protected _updateParagraphIndentationsImpl() {
		var items = this._map['stateChangeHandler'];
		var state = items.getItemValue('.uno:LeftRightParaMargin');
		// in impress/draw values are not as per Inch factore we should consider this case
		var conversionFactorToInches = app.map.isPresentationOrDrawing() ? 1.76 : 1;

		if (!state) return;

		this.options.firstLineIndent =
			parseFloat(state.firstline.replace(',', '.')) / conversionFactorToInches;
		this.options.leftParagraphIndent =
			parseFloat(state.left.replace(',', '.')) / conversionFactorToInches;
		this.options.rightParagraphIndent =
			parseFloat(state.right.replace(',', '.')) / conversionFactorToInches;
		this.options.unit = state.unit;

		var pxPerMm100 =
			app.map._docLayer._docPixelSize.x /
			((app.activeDocument.fileSize.x * 2540) / 1440);

		// Conversion to mm100.
		if (this.options.unit === 'inch') {
			this.options.firstLineIndent = this.options.firstLineIndent * 2540;
			this.options.leftParagraphIndent =
				this.options.leftParagraphIndent * 2540;
			this.options.rightParagraphIndent =
				this.options.rightParagraphIndent * 2540;
		}

		this.options.firstLineIndent *= pxPerMm100;
		this.options.leftParagraphIndent *= pxPerMm100;
		this.options.rightParagraphIndent *= pxPerMm100;

		// Get navigatiosidebar width only when navigation sidebar is visible
		const navigationsidebarWidth = this._getNavigationSidebarWidth();

		// rTSContainer is the reference element.
		const pStartPosition =
			this._rTSContainer.getBoundingClientRect().left +
			this.options.leftParagraphIndent -
			navigationsidebarWidth;
		const fLinePosition = pStartPosition + this.options.firstLineIndent;
		const pEndPosition =
			this._rTSContainer.getBoundingClientRect().right -
			this.options.rightParagraphIndent -
			navigationsidebarWidth;

		// We calculated the positions. Now we should move them to left in order to make their sharp edge point to the right direction..
		this._firstLineMarker.style.left =
			fLinePosition -
			this._firstLineMarker.getBoundingClientRect().width / 2.0 +
			'px';
		this._pStartMarker.style.left =
			pStartPosition -
			this._pStartMarker.getBoundingClientRect().width / 2.0 +
			'px';
		this._pEndMarker.style.left =
			pEndPosition -
			this._pEndMarker.getBoundingClientRect().width / 2.0 +
			'px';

		this._markerVerticalLine.style.top =
			this._rTSContainer.getBoundingClientRect().bottom + 'px';
	}

	_updateTabStops(obj: {
		tabstops: Array<{ position: string; type: string }> | string;
	}) {
		this.options.tabs = [];
		var jsonTabstops = obj['tabstops'];
		if (jsonTabstops === '') return;
		if (typeof jsonTabstops === 'string' || jsonTabstops.length === 0) return;

		for (const jsonTabstop of jsonTabstops) {
			this.options.tabs.push({
				position: parseInt(jsonTabstop.position),
				style: parseInt(jsonTabstop.type),
			});
			this._updateBreakPoints();
		}
	}

	_updateBreakPointsImpress() {
		if (this.options.margin1 == null || this.options.margin2 == null) return;

		const leftPageMargin = this.options.leftOffset;
		const rightPageMargin =
			this.options.pageWidth - (this.options.leftOffset + this.options.margin2);
		const scale = app.map.getZoomScale(this._map.getZoom(), 10);
		const rulerWidth =
			app.activeDocument.fileSize.cX - this.options.tileMargin * 2 * scale;
		const rulerWidthCM = Math.floor(
			rulerWidth * 0.026458 /* width of 1 CSS px in cm*/,
		);
		const pageWidthCM = this.options.pageWidth / 1000;
		const numbersPerCM = pageWidthCM / rulerWidthCM;
		const increaseBy = Math.round(numbersPerCM + 0.5);
		const markerWidthPx = (increaseBy / numbersPerCM) * (1 / 0.026458);

		this.fixOffset();

		this.options.DraggableConvertRatio = rulerWidth / this.options.pageWidth;
		this._rFace.style.width = rulerWidth + 'px';

		this._rBPContainer.style.marginLeft =
			-1 *
				(this.options.DraggableConvertRatio * (500 - (leftPageMargin % 1000))) +
			1 +
			'px';

		this._rBPContainer.replaceChildren();
		var numCounter = -1 * Math.floor(leftPageMargin / 1000);

		// this.options.pageWidth is in mm100, so the code here makes one ruler division per centimetre.
		for (var num = 0; num <= this.options.pageWidth / 1000 + 1; num++) {
			var marker = window.L.DomUtil.create(
				'div',
				'cool-ruler-maj',
				this._rBPContainer,
			);

			// The - 1 is to compensate for the left and right .5px borders of cool-ruler-maj in leaflet.css.
			marker.style.width =
				(numCounter !== 0 ? markerWidthPx : markerWidthPx * 0.5) - 1 + 'px';
			if (this.options.displayNumber) {
				if (numCounter !== 0) marker.innerText = numCounter;
			}
			numCounter += increaseBy;
		}

		// The tabstops. Only draw user-created ones, with style RULER_TAB_LEFT,
		// RULER_TAB_RIGHT, RULER_TAB_CENTER, and RULER_TAB_DECIMAL. See <svtools/ruler.hxx>.
		this._rTSContainer.replaceChildren();

		var pxPerMm100 =
			app.map._docLayer._docPixelSize.x /
			((app.activeDocument.fileSize.x * 2540) / 1440);
		this._rTSContainer.tabStops = [];
		for (
			var tabstopIndex = 0;
			tabstopIndex < this.options.tabs.length;
			tabstopIndex++
		) {
			var markerClass = null;
			var currentTabstop: any = this.options.tabs[tabstopIndex];
			switch (currentTabstop.style) {
				case 0:
					markerClass = 'cool-ruler-tabstop-left';
					break;
				case 1:
					markerClass = 'cool-ruler-tabstop-right';
					break;
				case 2:
					markerClass = 'cool-ruler-tabstop-center';
					break;
				case 3:
					markerClass = 'cool-ruler-tabstop-decimal';
					break;
			}
			if (markerClass != null) {
				marker = window.L.DomUtil.create(
					'div',
					markerClass,
					this._rTSContainer,
				);
				var positionPixel = currentTabstop.position * pxPerMm100;
				var markerWidth = marker.offsetWidth;
				var markerHalfWidth = markerWidth / 2.0;
				marker.tabStopLocation = {
					left: positionPixel - markerHalfWidth,
					center: positionPixel,
					right: positionPixel + markerHalfWidth,
				};
				marker.style.left = marker.tabStopLocation.left + 'px';
				marker.tabStopNumber = tabstopIndex;
				this._rTSContainer.tabStops[tabstopIndex] = marker;
				marker.style.cursor = 'move';
			}
		}

		if (!this.options.marginSet) {
			this.options.marginSet = true;
			this._lMarginMarker = window.L.DomUtil.create(
				'div',
				'cool-ruler-margin cool-ruler-left',
				this._rFace,
			);
			this._rMarginMarker = window.L.DomUtil.create(
				'div',
				'cool-ruler-margin cool-ruler-right',
				this._rFace,
			);
			this._lMarginDrag = window.L.DomUtil.create(
				'div',
				'cool-ruler-drag cool-ruler-left',
				this._rMarginWrapper,
			);
			this._lToolTip = window.L.DomUtil.create(
				'div',
				'cool-ruler-ltooltip',
				this._lMarginDrag,
			);
			this._rMarginDrag = window.L.DomUtil.create(
				'div',
				'cool-ruler-drag cool-ruler-right',
				this._rMarginWrapper,
			);
			this._rToolTip = window.L.DomUtil.create(
				'div',
				'cool-ruler-rtooltip',
				this._rMarginDrag,
			);
			var lMarginTooltipText = _('Left Margin');
			var rMarginTooltipText = _('Right Margin');

			this._lMarginDrag.dataset.title = lMarginTooltipText;
			this._rMarginDrag.dataset.title = rMarginTooltipText;
		}

		this._lMarginMarker.style.width =
			this.options.DraggableConvertRatio * leftPageMargin + 'px';
		this._rMarginMarker.style.width =
			this.options.DraggableConvertRatio * rightPageMargin + 'px';
		this._lMarginDrag.style.width =
			this.options.DraggableConvertRatio * leftPageMargin + 'px';
		this._rMarginDrag.style.width =
			this.options.DraggableConvertRatio * rightPageMargin + 'px';

		// Put the _rTSContainer in the right place
		this._rTSContainer.style.left =
			this.options.DraggableConvertRatio * leftPageMargin + 'px';
		this._rTSContainer.style.right =
			this.options.DraggableConvertRatio * rightPageMargin + 'px';

		this._updateParagraphIndentations();

		if (this.options.interactive) {
			this._changeInteractions({ perm: 'edit' });
		} else {
			this._changeInteractions({ perm: 'readonly' });
		}
	}

	_updateBreakPoints() {
		if (this.options.margin1 == null || this.options.margin2 == null) return;

		if (app.map._docLayer._docType === 'presentation') {
			this._updateBreakPointsImpress();
			return;
		}

		var lMargin, rMargin, wPixel, scale;

		lMargin = this.options.leftOffset;

		// This is surely bogus. We take pageWidth, which is in mm100, and subtract a value
		// that is in "arbitrary pixelish units". But the only thing rMargin is used for is
		// to calculate the width of the part of the ruler that goes out over the right side
		// of the window anyway (see the assignments to this._rMarginMarker.style.width and
		// this._rMarginDrag.style.width near the end of this function), so presumably it
		// doesn't matter that much what rMargin is.
		rMargin =
			this.options.pageWidth - (this.options.leftOffset + this.options.margin2);

		scale = this._map.getZoomScale(this._map.getZoom(), 10);
		wPixel =
			this._map._docLayer._docPixelSize.x - this.options.tileMargin * 2 * scale;

		this.fixOffset();

		this.options.DraggableConvertRatio = wPixel / this.options.pageWidth;
		this._rFace.style.width = wPixel + 'px';
		this._rBPContainer.style.marginLeft =
			-1 * (this.options.DraggableConvertRatio * (500 - (lMargin % 1000))) +
			1 +
			'px';

		var numCounter = -1 * Math.floor(lMargin / 1000);

		window.L.DomUtil.removeChildNodes(this._rBPContainer);

		// this.options.pageWidth is in mm100, so the code here makes one ruler division per
		// centimetre.
		//
		// FIXME: Surely this should be locale-specific, we would want to use inches at
		// least in the US. (The ruler unit to use doesn't seem to be stored in the document
		// at least for .odt?)
		for (var num = 0; num <= this.options.pageWidth / 1000 + 1; num++) {
			var marker = window.L.DomUtil.create(
				'div',
				'cool-ruler-maj',
				this._rBPContainer,
			);
			// The - 1 is to compensate for the left and right .5px borders of
			// cool-ruler-maj in leaflet.css.
			marker.style.width = this.options.DraggableConvertRatio * 1000 - 1 + 'px';
			if (this.options.displayNumber) {
				if (numCounter !== 0) marker.innerText = Math.abs(numCounter++);
				else numCounter++;
			}
		}

		// The tabstops. Only draw user-created ones, with style RULER_TAB_LEFT,
		// RULER_TAB_RIGHT, RULER_TAB_CENTER, and RULER_TAB_DECIMAL. See <svtools/ruler.hxx>.
		window.L.DomUtil.removeChildNodes(this._rTSContainer);

		var pxPerMm100 =
			this._map._docLayer._docPixelSize.x /
			((app.activeDocument.fileSize.x * 2540) / 1440);
		this._rTSContainer.tabStops = [];
		for (
			var tabstopIndex = 0;
			tabstopIndex < this.options.tabs.length;
			tabstopIndex++
		) {
			var markerClass = null;
			var currentTabstop: any = this.options.tabs[tabstopIndex];
			switch (currentTabstop.style) {
				case 0:
					markerClass = 'cool-ruler-tabstop-left';
					break;
				case 1:
					markerClass = 'cool-ruler-tabstop-right';
					break;
				case 2:
					markerClass = 'cool-ruler-tabstop-center';
					break;
				case 3:
					markerClass = 'cool-ruler-tabstop-decimal';
					break;
			}
			if (markerClass != null) {
				marker = window.L.DomUtil.create(
					'div',
					markerClass,
					this._rTSContainer,
				);
				var positionPixel = currentTabstop.position * pxPerMm100;
				var markerWidth = marker.offsetWidth;
				var markerHalfWidth = markerWidth / 2.0;
				marker.tabStopLocation = {
					left: positionPixel - markerHalfWidth,
					center: positionPixel,
					right: positionPixel + markerHalfWidth,
				};
				marker.style.left = marker.tabStopLocation.left + 'px';
				marker.tabStopNumber = tabstopIndex;
				this._rTSContainer.tabStops[tabstopIndex] = marker;
				marker.style.cursor = 'move';
			}
		}

		if (!this.options.marginSet) {
			this.options.marginSet = true;
			this._lMarginMarker = window.L.DomUtil.create(
				'div',
				'cool-ruler-margin cool-ruler-left',
				this._rFace,
			);
			this._rMarginMarker = window.L.DomUtil.create(
				'div',
				'cool-ruler-margin cool-ruler-right',
				this._rFace,
			);
			this._lMarginDrag = window.L.DomUtil.create(
				'div',
				'cool-ruler-drag cool-ruler-left',
				this._rMarginWrapper,
			);
			this._lToolTip = window.L.DomUtil.create(
				'div',
				'cool-ruler-ltooltip',
				this._lMarginDrag,
			);
			this._rMarginDrag = window.L.DomUtil.create(
				'div',
				'cool-ruler-drag cool-ruler-right',
				this._rMarginWrapper,
			);
			this._rToolTip = window.L.DomUtil.create(
				'div',
				'cool-ruler-rtooltip',
				this._rMarginDrag,
			);
			var lMarginTooltipText = _('Left Margin');
			var rMarginTooltipText = _('Right Margin');

			this._lMarginDrag.dataset.title = lMarginTooltipText;
			this._rMarginDrag.dataset.title = rMarginTooltipText;
		}

		this._lMarginMarker.style.width =
			this.options.DraggableConvertRatio * lMargin + 'px';
		this._rMarginMarker.style.width =
			this.options.DraggableConvertRatio * rMargin + 'px';
		this._lMarginDrag.style.width =
			this.options.DraggableConvertRatio * lMargin + 'px';
		this._rMarginDrag.style.width =
			this.options.DraggableConvertRatio * rMargin + 'px';

		// Put the _rTSContainer in the right place
		this._rTSContainer.style.left =
			this.options.DraggableConvertRatio * lMargin + 'px';
		this._rTSContainer.style.right =
			this.options.DraggableConvertRatio * rMargin + 'px';

		this._updateParagraphIndentations();

		if (this.options.interactive) {
			this._changeInteractions({ perm: 'edit' });
		} else {
			this._changeInteractions({ perm: 'readonly' });
		}
	}

	protected _fixOffsetImpl(): void {
		if (!app.activeDocument || app.activeDocument.fileSize.x === 0) return;

		const layout = app.activeDocument.activeLayout;

		if (layout.type === 'ViewLayoutMultiPage') {
			const multiPageLayout = layout as ViewLayoutMultiPage;
			const pageRectList = app.file.writer.pageRectangleList;
			if (pageRectList.length === 0) return;

			// Find which page the cursor is on.
			let pageIndex = 0;
			const cursorRect = app.file.textCursor.rectangle;
			if (cursorRect) {
				const cursorPoint = new cool.SimplePoint(cursorRect.x1, cursorRect.y1);
				pageIndex = multiPageLayout.getClosestRectangleIndex(cursorPoint);
			}

			// Get the page's top-left corner in document coordinates.
			const pageRect = pageRectList[pageIndex];
			const pageTopLeft = new cool.SimplePoint(pageRect[0], pageRect[1]);

			// Convert to screen position (core pixels -> CSS pixels).
			const screenXCorePixels = layout.documentToViewX(pageTopLeft);
			const rulerOffset = screenXCorePixels / app.dpiScale;

			const newValue = rulerOffset + 'px';
			if (this._rFace.style.marginInlineStart !== newValue)
				this._rFace.style.marginInlineStart = newValue;
		} else if (layout.type === 'ViewLayoutCompareChanges') {
			let rulerOffset =
				-layout.viewedRectangle.cX1 + this.options.tileMargin * app.getScale();
			if (layout.type === 'ViewLayoutCompareChanges')
				rulerOffset += Math.round(
					layout.documentToViewX(new cool.SimplePoint(0, 0)) / app.dpiScale,
				);
			this._rFace.style.marginInlineStart = rulerOffset + 'px';
		} else {
			const rulerOffset =
				-layout.viewedRectangle.cX1 + this.options.tileMargin * app.getScale();

			this._rFace.style.marginInlineStart = rulerOffset + 'px';
		}

		this._updateParagraphIndentations();
	}

	_moveIndentation(e: any) {
		if (e.type === 'panmove') {
			e.clientX = e.center.x;
		}

		var element = document.getElementById(this._indentationElementId);

		// User is moving the cursor / their finger on the screen and we are moving the marker.
		var newLeft =
			parseInt(element.style.left.replace('px', '')) +
			e.clientX -
			this._lastposition;
		element.style.left = String(newLeft) + 'px';
		this._lastposition = e.clientX;
		// halfWidth..
		var halfWidth =
			(element.getBoundingClientRect().right -
				element.getBoundingClientRect().left) *
			0.5;
		this._markerVerticalLine.style.left =
			String(newLeft + halfWidth + this._getNavigationSidebarWidth()) + 'px';
	}

	_moveIndentationEnd(e: Event) {
		this._map.rulerActive = false;

		if (e.type !== 'panend') {
			window.L.DomEvent.off(
				this._rFace,
				'mousemove',
				this._moveIndentation,
				this,
			);
			window.L.DomEvent.off(
				this._map,
				'mouseup',
				this._moveIndentationEnd,
				this,
			);
		}

		var unoObj: any = {};

		// Calculation step..
		// The new coordinate of element subject to indentation is sent as a percentage of the page width..
		// We need to calculate the percentage. Left margin (leftOffset) is not being added to the indentation (on the core part)..
		// We can use TabStopContainer's position as the reference point, as they share the same reference point..
		var element = document.getElementById(this._indentationElementId);

		if (!element) return;

		// The halfWidth of the shape..
		var halfWidth =
			(element.getBoundingClientRect().right -
				element.getBoundingClientRect().left) *
			0.5;

		var firstLineMargin =
			this.options.firstLineIndent / this.options.DraggableConvertRatio;
		var leftValue =
			this.options.leftParagraphIndent / this.options.DraggableConvertRatio;
		var rightValue =
			this.options.rightParagraphIndent / this.options.DraggableConvertRatio;

		// Calculate and update left, right and firstLine margin according to selected marker
		if (element.id === 'lo-fline-marker') {
			firstLineMargin = Math.ceil(
				(this._firstLineMarker.getBoundingClientRect().left -
					this._pStartMarker.getBoundingClientRect().left +
					halfWidth) /
					this.options.DraggableConvertRatio,
			);
			this.options.firstLineIndent =
				firstLineMargin * this.options.DraggableConvertRatio;
		} else if (element.id === 'lo-pend-marker') {
			rightValue = Math.ceil(
				(this._rTSContainer.getBoundingClientRect().right -
					this._pEndMarker.getBoundingClientRect().right +
					halfWidth) /
					this.options.DraggableConvertRatio,
			);
		} else if (element.id === 'lo-pstart-marker') {
			leftValue = Math.ceil(
				(this._pStartMarker.getBoundingClientRect().left -
					this._rTSContainer.getBoundingClientRect().left +
					halfWidth) /
					this.options.DraggableConvertRatio,
			);
		}

		// it is kind of necessary to send all prams details to set values right in CORE otherwise for missing values it will take default as 0
		unoObj['LRSpace.FirstLineIndent'] = {
			type: 'long',
			value: firstLineMargin,
		};
		unoObj['LRSpace.LeftMargin'] = { type: 'long', value: leftValue };
		unoObj['LRSpace.RightMargin'] = { type: 'long', value: rightValue };

		// Send the command
		this._map.sendUnoCommand('.uno:LeftRightParaMargin', unoObj);

		this._indentationElementId = '';
		this._markerVerticalLine.style.display = 'none';
	}

	_initiateIndentationDrag(e: any) {
		if (window.ThisIsTheiOSApp && !this._map.isEditMode()) return;

		this._map.rulerActive = true;

		this._indentationElementId =
			e.target.id.trim() === '' ? e.target.parentNode.id : e.target.id;

		if (e.type !== 'panstart') {
			window.L.DomEvent.on(
				this._rFace,
				'mousemove',
				this._moveIndentation,
				this,
			);
			window.L.DomEvent.on(
				this._rFace,
				'click',
				this._moveIndentationEnd,
				this,
			);
			window.L.DomEvent.on(
				this._rFace,
				'mouseleave',
				this._moveIndentationEnd,
				this,
			);
			window.L.DomEvent.on(
				this._map,
				'mouseup',
				this._moveIndentationEnd,
				this,
			);
		} else {
			e.clientX = e.center.x;
		}

		this._initialposition = this._lastposition = e.clientX;
		this._markerVerticalLine.style.display = 'block';
		this._markerVerticalLine.style.left = this._lastposition + 'px';
	}

	_initiateDrag(e: any) {
		if (e.type === 'touchstart') {
			if (e.touches.length !== 1) return;
			e.clientX = e.touches[0].clientX;
		}

		if (
			this.getWindowProperty<boolean>('ThisIsTheiOSApp') &&
			!this._map.isEditMode()
		)
			return;

		this._map.rulerActive = true;

		var dragableElem = e.srcElement || e.target;
		window.L.DomEvent.on(this._rFace, 'mousemove', this._moveMargin, this);
		window.L.DomEvent.on(this._map, 'mouseup', this._endDrag, this);
		this._initialposition = e.clientX;
		this._lastposition = this._initialposition;

		if (window.L.DomUtil.hasClass(dragableElem, 'cool-ruler-right')) {
			window.L.DomUtil.addClass(this._rMarginDrag, 'leaflet-drag-moving');
			this._rFace.style.cursor = 'w-resize';
		} else {
			window.L.DomUtil.addClass(this._lMarginDrag, 'leaflet-drag-moving');
			this._rFace.style.cursor = 'e-resize';
		}
	}

	_moveMargin(e: any) {
		if (e.type === 'touchmove') e.clientX = e.touches[0].clientX;

		this._lastposition = e.clientX;
		var posChange = e.clientX - this._initialposition;
		var unit = this.options.unit ? this.options.unit : ' cm';
		if (window.L.DomUtil.hasClass(this._rMarginDrag, 'leaflet-drag-moving')) {
			var rMargin =
				this.options.pageWidth -
				(this.options.leftOffset + this.options.margin2);
			var newPos = this.options.DraggableConvertRatio * rMargin - posChange;
			this._rToolTip.style.display = 'block';
			this._rToolTip.style.right = newPos - 25 + 'px';
			this._rToolTip.innerText =
				(
					Math.round(
						this.options.pageWidth / 100 -
							newPos / (this.options.DraggableConvertRatio * 100),
					) / 10
				).toString() + unit;
			this._rMarginDrag.style.width = newPos + 'px';
		} else {
			newPos =
				this.options.DraggableConvertRatio * this.options.leftOffset +
				posChange;
			this._lToolTip.style.display = 'block';
			this._lToolTip.style.left = newPos - 25 + 'px';
			this._lToolTip.innerText =
				(
					Math.round(newPos / (this.options.DraggableConvertRatio * 100)) / 10
				).toString() + unit;
			this._lMarginDrag.style.width = newPos + 'px';
		}
	}

	_endDrag(e: any) {
		this._map.rulerActive = false;

		var posChange;
		if (e.type === 'touchend')
			posChange = this._lastposition - this._initialposition;
		else posChange = e.originalEvent.clientX - this._initialposition;
		var unoObj: any = {},
			marginType,
			fact;

		window.L.DomEvent.off(this._rFace, 'mousemove', this._moveMargin, this);
		window.L.DomEvent.off(this._map, 'mouseup', this._endDrag, this);

		if (window.L.DomUtil.hasClass(this._rMarginDrag, 'leaflet-drag-moving')) {
			marginType = 'Margin2';
			fact = -1;
			window.L.DomUtil.removeClass(this._rMarginDrag, 'leaflet-drag-moving');
			this._rToolTip.style.display = 'none';
		} else {
			marginType = 'Margin1';
			fact = 1;
			window.L.DomUtil.removeClass(this._lMarginDrag, 'leaflet-drag-moving');
			this._lToolTip.style.display = 'none';
		}

		this._rFace.style.cursor = 'default';

		unoObj[marginType] = {};
		unoObj[marginType]['type'] = 'string';
		unoObj[marginType]['value'] =
			(fact * posChange) /
			(this.options.DraggableConvertRatio * this.options.pageWidth);
		app.socket.sendMessage(
			'uno .uno:RulerChangeState ' + JSON.stringify(unoObj),
		);
	}

	_getTabStopHit(tabstopContainer: TabStopContainer, pointX: number) {
		let tabstop: TabStop | null = null;
		const margin = 10;
		let tabstopDiffFromCenter = 100000000; // just a big initial condition

		for (let i = 0; i < tabstopContainer.tabStops.length; i++) {
			const current = tabstopContainer.tabStops[i];
			const location = current.tabStopLocation;
			if (
				pointX >= location.left - margin &&
				pointX <= location.right + margin
			) {
				const diff = Math.abs(pointX - location.center);
				if (diff < tabstopDiffFromCenter) {
					tabstop = current;
					tabstopDiffFromCenter = diff;
				}
			}
		}
		return tabstop;
	}

	_showTabstopContextMenu(
		position: number,
		tabstopNumber: number,
		event: MouseEvent,
	) {
		this.currentPositionInTwips = position;
		this.currentTabStopIndex = tabstopNumber;

		const entries: Array<MenuDefinition> = [];
		if (position != null) {
			entries.push({
				id: 'inserttabstop',
				type: 'comboboxentry',
				text: _('Insert tabstop'),
				pos: 0,
			});
		}
		if (tabstopNumber != null) {
			entries.push({
				id: 'removetabstop',
				type: 'comboboxentry',
				text: _('Delete tabstop'),
				pos: 0,
			});
		}

		const container = this._rTSContainer;
		const childCount = container.children.length;
		const menuPosEl = document.createElement('div');
		menuPosEl.style.position = 'absolute';
		container.appendChild(menuPosEl);

		const rect = container.getBoundingClientRect();
		menuPosEl.style.left = event.clientX - rect.left + 'px';
		menuPosEl.style.top = event.clientY - rect.top + 'px';

		const callback = (
			objectType: string,
			eventType: string,
			object: any,
			data: any,
			entry: MenuDefinition,
		): boolean => {
			if (entry.id === 'inserttabstop') {
				this._insertTabstop();
			} else if (entry.id === 'removetabstop') {
				this._deleteTabstop();
			}
			JSDialog.CloseAllDropdowns();
			menuPosEl.remove();
			console.assert(
				childCount >= container.children.length,
				'HRuler: leak of children detected',
			);
			return true;
		};

		JSDialog.OpenDropdown(
			'hruler-tabstop-menu',
			menuPosEl,
			entries,
			callback,
			'',
			false,
		);
	}

	_initiateTabstopDrag(event: any) {
		// window.app.console.log('===> _initiateTabstopDrag ' + event.type);

		var tabstopContainer = null;
		var pointX = null;

		if (event.type === 'panstart') {
			tabstopContainer = event.target;
			pointX = event.center.x - event.target.getBoundingClientRect().left;
		} else {
			tabstopContainer = event.currentTarget;
			pointX = event.layerX;
		}
		tabstopContainer.tabStopMarkerBeingDragged = null;

		// check if we hit any tabstop
		var tabstop = this._getTabStopHit(tabstopContainer, pointX);

		// Check what to do when a mouse buttons is clicked, ignore touch
		if (event.type !== 'panstart') {
			// right-click inside tabstop container
			if (event.button === 2) {
				if (tabstop == null) {
					var position = this._map._docLayer._pixelsToTwips({
						x: pointX,
						y: 0,
					}).x;
					this._showTabstopContextMenu(position, null, event);
				} else {
					this._showTabstopContextMenu(null, tabstop.tabStopNumber, event);
				}
				event.stopPropagation();
				return;
			} else if (event.button !== 0) {
				event.stopPropagation(); // prevent handling of the mother event elsewhere
				return;
			}
		}

		if (tabstop == null) {
			return;
		}

		tabstopContainer.tabStopMarkerBeingDragged = tabstop;
		tabstopContainer.tabStopInitialPosiiton = pointX;

		if (
			!this.getWindowProperty<boolean>('ThisIsTheiOSApp') &&
			event.pointerType !== 'touch'
		) {
			window.L.DomEvent.on(
				this._rTSContainer,
				'mousemove',
				this._moveTabstop,
				this,
			);
			window.L.DomEvent.on(
				this._rTSContainer,
				'mouseup',
				this._endTabstopDrag,
				this,
			);
			window.L.DomEvent.on(
				this._rTSContainer,
				'mouseout',
				this._endTabstopDrag,
				this,
			);
		}
	}

	_moveTabstop(event: any) {
		var tabstopContainer = null;
		var pointX = null;

		if (event.type === 'panmove') {
			tabstopContainer = event.target;
			pointX = event.center.x - event.target.getBoundingClientRect().left;
		} else {
			tabstopContainer = event.currentTarget;
			pointX = event.layerX;
		}

		if (tabstopContainer === null) return;
		var marker = tabstopContainer.tabStopMarkerBeingDragged;
		if (marker === null) return;

		//window.app.console.log('===> _moveTabstop ' + event.type);

		var pixelDiff = pointX - tabstopContainer.tabStopInitialPosiiton;
		marker.style.left = marker.tabStopLocation.left + pixelDiff + 'px';
	}

	_endTabstopDrag(event: any) {
		//window.app.console.log('===> _endTabstopDrag ' + event.type);

		var tabstopContainer = null;
		var pointX = null;
		if (event.type === 'panend') {
			tabstopContainer = event.target;
			pointX = event.center.x - event.target.getBoundingClientRect().left;
		} else {
			tabstopContainer = event.currentTarget;
			pointX = event.layerX;
		}

		if (tabstopContainer === null) return;
		var marker = tabstopContainer.tabStopMarkerBeingDragged;
		if (marker === null) return;

		if (event.type == 'mouseout') {
			marker.style.left = marker.tabStopLocation.left + 'px';
		} else {
			var positionTwip = this._map._docLayer._pixelsToTwips({
				x: pointX,
				y: 0,
			}).x;
			var params = {
				Index: {
					type: 'int32',
					value: marker.tabStopNumber,
				},
				Position: {
					type: 'int32',
					value: positionTwip,
				},
				Remove: {
					type: 'boolean',
					value: false,
				},
			};
			this._map.sendUnoCommand('.uno:ChangeTabStop', params);
		}
		window.L.DomEvent.off(
			this._rTSContainer,
			'mousemove',
			this._moveTabstop,
			this,
		);
		window.L.DomEvent.off(
			this._rTSContainer,
			'mouseup',
			this._endTabstopDrag,
			this,
		);
		window.L.DomEvent.off(
			this._rTSContainer,
			'mouseout',
			this._endTabstopDrag,
			this,
		);
	}

	_onTabstopContainerLongPress(event: any) {
		var tabstopContainer = event.target;
		var pointX = event.center.x - tabstopContainer.getBoundingClientRect().left;
		var pointXTwip = this._map._docLayer._pixelsToTwips({ x: pointX, y: 0 }).x;
		var tabstop = this._getTabStopHit(tabstopContainer, pointX);

		if (window.mode.isSmallScreenDevice() || window.mode.isTablet()) {
			if (tabstop == null) {
				this.currentPositionInTwips = pointXTwip;
				this.currentTabStopIndex = null;
				this._insertTabstop();
			} else {
				this.currentPositionInTwips = null;
				this.currentTabStopIndex = tabstop.tabStopNumber;
				this._deleteTabstop();
			}
		} else {
			var tabstopNumber = null;
			if (tabstop != null) {
				tabstopNumber = tabstop.tabStopNumber;
				pointXTwip = null;
			}
			this._showTabstopContextMenu(pointXTwip, tabstopNumber, event);
		}
	}

	_deleteTabstop() {
		if (this.currentTabStopIndex != null) {
			var params = {
				Index: {
					type: 'int32',
					value: this.currentTabStopIndex,
				},
				Position: {
					type: 'int32',
					value: 0,
				},
				Remove: {
					type: 'boolean',
					value: true,
				},
			};
			this._map.sendUnoCommand('.uno:ChangeTabStop', params);
			this.currentTabStopIndex = null;
		}
	}

	_insertTabstop() {
		if (this.currentPositionInTwips != null) {
			var params = {
				Index: {
					type: 'int32',
					value: -1,
				},
				Position: {
					type: 'int32',
					value: this.currentPositionInTwips,
				},
				Remove: {
					type: 'boolean',
					value: false,
				},
			};
			this._map.sendUnoCommand('.uno:ChangeTabStop', params);
			this.currentPositionInTwips = null;
		}
	}

	_getNavigationSidebarWidth() {
		return app.map?.navigator ? app.map.navigator.getCurrentWidth() : 0;
	}
}
