/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * window.L.CanvasTileLayer is a layer with canvas based rendering.
 */

/* global app JSDialog CanvasSectionContainer GraphicSelection CanvasOverlay CursorHeaderSection $ _ CPolyUtil CPolygon Cursor UNOKey cool OtherViewCellCursorSection TileManager SplitSection TextSelections CellSelectionMarkers URLPopUpSection CalcValidityDropDown DocumentBase CellCursorSection FormFieldButton TextCursorSection CStyleData CSelections CReferences OtherViewGraphicSelectionSection CompareChangesLabelSection */

function clamp(num, min, max)
{
	return Math.min(Math.max(num, min), max);
}

window.L.TileSectionManager = window.L.Class.extend({

	initialize: function (layer) {
		this._layer = layer;
		this._canvas = this._layer._canvas;
		this._map = this._layer._map;
		var mapSize = { x:  app.sectionContainer.getWidth(), y:  app.sectionContainer.getHeight() };
		this._tilesSection = null; // Shortcut.

		if (window.L.Browser.cypressTest) // If cypress is active, create test divs.
			app.sectionContainer.testing = true;

		app.sectionContainer.onResize(mapSize.x, mapSize.y);

		var splitPanesContext = this._layer.getSplitPanesContext();
		this._splitPos = splitPanesContext ?
			splitPanesContext.getSplitPos() : new cool.Point(0, 0);
		this._updatesRunning = false;

		var canvasContainer = document.getElementById('document-container');
		var that = this;
		this.resObserver = new ResizeObserver(function() {
			that._layer._syncTileContainerSize();
		});
		this.resObserver.observe(canvasContainer);

		this._zoomAtDocEdgeX = true;
		this._zoomAtDocEdgeY = true;
	},

	getSplitPos: function () {
		var splitPanesContext = this._layer.getSplitPanesContext();
		return splitPanesContext ?
			splitPanesContext.getSplitPos().multiplyBy(app.dpiScale) :
			new cool.Point(0, 0);
	},

	// Details of tile areas to render
	_paintContext: function() {
		var viewBounds = this._map.getPixelBoundsCore();
		var splitPanesContext = this._layer.getSplitPanesContext();
		var paneBoundsList = splitPanesContext ?
		    splitPanesContext.getPxBoundList(viewBounds) :
		    [viewBounds];

		return {
			 viewBounds: viewBounds,
			 paneBoundsList: paneBoundsList,
			 paneBoundsActive: splitPanesContext ? true: false,
			 splitPos: this.getSplitPos(),
		};
	},

	// Debug tool. Splits are enabled for only Calc for now.
	_addSplitsSection: function () {
		const splitSection = new SplitSection();
		app.sectionContainer.addSection(splitSection);
	},

	_removeSplitsSection: function () {
		var section = app.sectionContainer.getSectionWithName('calc grid');
		if (section) {
			section.setDrawingOrder(app.CSections.CalcGrid.drawingOrder);
			section.sectionProperties.strokeStyle = '#c0c0c0';
		}
		app.sectionContainer.removeSection(app.CSections.Debug.Splits.name);
	},

	// Debug tool
	_addTilePixelGridSection: function () {
		app.sectionContainer.addSection(new app.definitions.pixelGridSection());
	},

	_removeTilePixelGridSection: function () {
		app.sectionContainer.removeSection(app.CSections.Debug.TilePixelGrid.name);
	},

	_addDebugOverlaySection: function () {
		app.sectionContainer.addSection(new app.definitions.debugOverlaySection(this._map._debug));
	},

	_removeDebugOverlaySection: function () {
		app.sectionContainer.removeSection(app.CSections.Debug.DebugOverlay.name);
	},

	_addPreloadMap: function () {
		app.sectionContainer.addSection(new app.definitions.preloadMapSection());
	},

	_removePreloadMap: function () {
		app.sectionContainer.removeSection(app.CSections.Debug.PreloadMap.name);
	},

	update: function () {
		app.sectionContainer.requestReDraw();
	},

	/**
	 * Everything in this doc comment is speculation: I didn't write the code that supplies it and I'm guessing to
	 * have something to work on for this function. That said, given my observations, they seem incredibly likely to be correct
	 *
	 * @param pinchCenter {{x: number, y: number}} The current pinch center in doc core-pixels
	 * Normally expressed as an cool.Point instance
	 *
	 * @param pinchStartCenter {{x: number, y: number}} The pinch center at the start of the pinch in doc core-pixels
	 * Normally expressed as an cool.Point instance
	 *
	 * @param paneBounds {{min: {x: number, y: number}, max: {x: number, y: number}}} The edges of the current pane
	 * Traditionally this is the map border at the start of the pinch
	 *
	 * @param freezePane {{freezeX: boolean, freezeY: boolean}} Whether the pane is frozen in the x or y directions
	 *
	 * @param splitPos {{x: number, y: number}} The inset in core-pixels into the document caused by any splits (e.g. a frozen row at the start of the document)
	 *
	 * @param scale {number} The scale, relative to the initial size, of the document currently
	 * Or rather this is equivalent to: old_width / new_width
	 *
	 * @param findFreePaneCenter {boolean} Wether to return a center point
	 *
	 * @returns {{topLeft: {x: number, y: number}, center?: {x: number, y: number}}} An object with a top left point in core-pixels and optionally a center point
	 * Center is included iff findFreePaneCenter is true
	 * (probably this should be encoded into the type, e.g. with an overload when this is converted to TypeScript)
	 **/
	_getZoomDocPos: function (pinchCenter, pinchStartCenter, paneBounds, freezePane, splitPos, scale, findFreePaneCenter) {
		let xMin = 0;
		const hasXMargin = !this._layer.isCalc();
		if (hasXMargin) {
			xMin = -Infinity;
		} else if (paneBounds.min.x > 0) {
			xMin = splitPos.x;
		}

		let yMin = 0;
		if (paneBounds.min.y < 0) {
			yMin = -Infinity;
		} else if (paneBounds.min.y > 0) {
			yMin = splitPos.y;
		}

		const minTopLeft = new cool.Point(xMin, yMin);

		const paneSize = paneBounds.getSize();

		pinchCenter = pinchCenter.subtract(this._offset);

		let centerOffset = {
			x: pinchCenter.x - pinchStartCenter.x,
			y: pinchCenter.y - pinchStartCenter.y,
		};

		// Portion of the pane away that our pinchStart (which should be where we zoom round) is
		const panePortion = {
			x: (pinchStartCenter.x - this._offset.x - paneBounds.min.x) / paneSize.x,
			y: (pinchStartCenter.y - this._offset.y - paneBounds.min.y) / paneSize.y,
		};

		let docTopLeft = new cool.Point(
			pinchStartCenter.x + (centerOffset.x - paneSize.x * panePortion.x) / scale,
			pinchStartCenter.y + (centerOffset.y - paneSize.y * panePortion.y) / scale
		);

		// Top left in document coordinates.
		const clampedDocTopLeft = new cool.Point(
			Math.max(minTopLeft.x, docTopLeft.x),
			Math.max(minTopLeft.y, docTopLeft.y)
		);

		const offset = clampedDocTopLeft.subtract(docTopLeft);

		if (freezePane.freezeX) {
			docTopLeft.x = paneBounds.min.x;
		} else {
			this._offset.x = Math.round(Math.max(this._offset.x, offset.x));
			docTopLeft.x += this._offset.x;
		}

		if (freezePane.freezeY) {
			docTopLeft.y = paneBounds.min.y;
		} else {
			this._offset.y = Math.round(Math.max(this._offset.y, offset.y));
			docTopLeft.y += this._offset.y;
		}

		if (!findFreePaneCenter) {
			return { offset: this._offset, topLeft: docTopLeft };
		}

		const newPaneCenter = new cool.Point(
			(docTopLeft.x - splitPos.x + (paneSize.x + splitPos.x) * 0.5 / scale),
			(docTopLeft.y - splitPos.y + (paneSize.y + splitPos.y) * 0.5 / scale));

		return {
			offset: this._offset,
			topLeft: docTopLeft.add(this._offset),
			center: this._map.rescale(newPaneCenter, this._map.getZoom(), this._map.getScaleZoom(scale)),
		};
	},

	_getZoomMapCenter: function (zoom) {
		var scale = this._calcZoomFrameScale(zoom);
		var ctx = this._paintContext();
		var splitPos = ctx.splitPos;
		var viewBounds = ctx.viewBounds;
		var freePaneBounds = new cool.Bounds(viewBounds.min.add(splitPos), viewBounds.max);

		return this._getZoomDocPos(
			this._newCenter,
			this._layer._pinchStartCenter,
			freePaneBounds,
			{ freezeX: false, freezeY: false },
			splitPos,
			scale,
			true /* findFreePaneCenter */
		).center;
	},

	_zoomAnimation: function () {
		var painter = this;
		var ctx = this._paintContext();
		var canvasOverlay = this._layer._canvasOverlay;

		var rafFunc = function (timeStamp, final) {
			painter._layer._refreshRowColumnHeaders();

			// Draw zoom frame with grids and directly from the tiles.
			// This will clear the doc area first.
			painter._tilesSection.drawZoomFrame(ctx);
			// Draw the overlay objects.
			canvasOverlay.onDraw();

			if (!final)
				painter._zoomRAF = requestAnimationFrame(rafFunc);
		};
		this.rafFunc = rafFunc;
		rafFunc();
	},

	_calcZoomFrameScale: function (zoom) {
		zoom = this._layer._map._limitZoom(zoom);
		var origZoom = this._layer._map.getZoom();
		// Compute relative-multiplicative scale of this zoom-frame w.r.t the starting zoom(ie the current Map's zoom).
		return this._layer._map.zoomToFactor(zoom - origZoom + this._layer._map.options.zoom);
	},

	_calcZoomFrameParams: function (zoom, newCenter) {
		this._zoomFrameScale = this._calcZoomFrameScale(zoom);
		this._newCenter = this._layer._map.project(newCenter).multiplyBy(app.dpiScale); // in core pixels
	},

	setWaitForTiles: function (wait) {
		this._waitForTiles = wait;
	},

	waitForTiles: function () {
		return this._waitForTiles;
	},

	zoomStep: function (zoom, newCenter) {
		if (this._finishingZoom) // finishing steps of animation still going on.
			return;

		this._calcZoomFrameParams(zoom, newCenter);

		if (!this._inZoomAnim) {
			app.sectionContainer.setInZoomAnimation(true);
			this._inZoomAnim = true;
			// Start RAF loop for zoom-animation
			this._zoomAnimation();
		}
	},

	zoomStepEnd: function (zoom, newCenter, mapUpdater, runAtFinish, noGap) {

		if (!this._inZoomAnim || this._finishingZoom)
			return;

		this._finishingZoom = true;

		this._map.disableTextInput();
		// Do a another animation from current non-integral log-zoom to
		// the final integral zoom, but maintain the same center.
		var steps = 10;
		var stepId = noGap ? steps : 0;

		var startZoom = this._zoomFrameScale;
		var endZoom = this._calcZoomFrameScale(zoom);
		var painter = this;
		var map = this._map;

		// Calculate the final center at final zoom in advance.
		var newMapCenter = this._getZoomMapCenter(zoom).divideBy(app.dpiScale);
		var newMapCenterLatLng = map.unproject(newMapCenter, zoom);
		app.sectionContainer.setZoomChanged(true);

		var stopAnimation = noGap ? true : false;
		var waitForTiles = false;
		var waitTries = 30;
		var finishingRAF = undefined;

		var finishAnimation = function () {

			if (stepId < steps) {
				// continue animating till we reach "close" to 'final zoom'.
				painter._zoomFrameScale = startZoom + (endZoom - startZoom) * stepId / steps;
				stepId += 1;
				if (stepId >= steps)
					stopAnimation = true;
			}

			if (stopAnimation) {
				stopAnimation = false;
				cancelAnimationFrame(painter._zoomRAF);
				painter._calcZoomFrameParams(zoom, newCenter);
				// Draw one last frame at final zoom.
				painter.rafFunc(undefined, true /* final? */);
				painter._zoomFrameScale = undefined;
				app.sectionContainer.setInZoomAnimation(false);
				painter._inZoomAnim = false;

				painter.setWaitForTiles(true);
				// Set view and paint the tiles if all available.
				mapUpdater(newMapCenterLatLng);
				waitForTiles = true;
			}

			if (waitForTiles) {
				// Wait until we get all tiles or wait time exceeded.
				if (waitTries <= 0 || painter._tilesSection.haveAllTilesInView()) {
					// All done.
					waitForTiles = false;
					cancelAnimationFrame(finishingRAF);
					painter.setWaitForTiles(false);
					app.sectionContainer.setZoomChanged(false);
					map.enableTextInput();
					map.focus(map.canAcceptKeyboardInput());
					// Paint everything.
					app.sectionContainer.requestReDraw();
					// Don't let a subsequent pinchZoom start before finishing all steps till this point.
					painter._finishingZoom = false;
					// Run the finish callback.
					runAtFinish();
					return;
				}
				else
					waitTries -= 1;
			}

			finishingRAF = requestAnimationFrame(finishAnimation);
		};

		finishAnimation();
	},

	getTileSectionPos : function () {
		return new cool.Point(this._tilesSection.myTopLeft[0], this._tilesSection.myTopLeft[1]);
	}
});

window.L.CanvasTileLayer = window.L.Layer.extend({

	options: {
		tileSize: window.tileSize,
		opacity: 1,

		updateWhenIdle: (window.mode.isSmallScreenDevice() || window.mode.isTablet()),
		updateInterval: 200,

		attribution: null,
		zIndex: null,
		bounds: null,

		previewInvalidationTimeout: 1000,
	},

	_pngCache: [],

	initialize: function (options) {

		window.L.Layer.prototype.initialize.call(this);

		options = window.L.setOptions(this, options);

		// text, presentation, spreadsheet, etc
		this._docType = options.docType;
		this._documentInfo = '';
		if (!this.isWriter())
			app.setCursorVisibility(false); // Don't change the default for Writer.
		// Last cursor position for invalidation
		this.lastCursorPos = null;
		// Are we zooming currently ? - if so, no cursor.
		this._isZooming = false;

		app.calc.cellCursorVisible = false;
		this._prevCellCursorAddress = null;
		this._shapeGridOffset = new cool.SimplePoint(0, 0);

		// Position and size of the selection start (as if there would be a cursor caret there).

		this._lastValidPart = -1;
		// Cursor marker
		this._cursorMarker = null;

		this._msgQueue = [];
		this._toolbarCommandValues = {};
		this._previewInvalidations = [];

		this._editorId = -1;
		app.setFollowingUser(options.viewId);

		this._selectedTextContent = '';

		this._moveInProgress = false;
		// tile requests issued while _moveInProgress is true,
		// i.e. issued between moveStart and moveEnd
		this._moveTileRequests = [];
		this._canonicalIdInitialized = false;

		TileManager.initialize();
	},

	_initContainer: function () {
		if (this._canvasContainer) {
			window.app.console.error('called _initContainer() when this._canvasContainer is present!');
		}

		if (this._container) { return; }

		this._container = window.L.DomUtil.create('div', 'leaflet-layer');
		this._updateZIndex();

		this.getPane().appendChild(this._container);

		var mapContainer = document.getElementById('document-container');
		var canvasContainerClass = 'leaflet-canvas-container';
		this._canvasContainer = window.L.DomUtil.create('div', canvasContainerClass, mapContainer);
		this._canvasContainer.id = 'canvas-container';
		this._setup();
	},

	_setup: function () {

		if (!this._canvasContainer) {
			window.app.console.error('canvas container not found. _initContainer failed ?');
		}

		this._canvas = window.L.DomUtil.createWithId('canvas', 'document-canvas', this._canvasContainer);
		this._canvas.style.visibility = 'hidden';
		this._canvas.role = 'img';
		this._canvas.ariaLabel = _('Online Editor');

		app.sectionContainer = new CanvasSectionContainer(this._canvas, this.isCalc() /* disableDrawing? */);
		app.activeDocument = new DocumentBase();

		this._container.style.position = 'absolute';
		this._cursorDataDiv = window.L.DomUtil.create('div', 'cell-cursor-data', this._canvasContainer);
		this._selectionsDataDiv = window.L.DomUtil.create('div', 'selections-data', this._canvasContainer);
		this._splittersDataDiv = window.L.DomUtil.create('div', 'splitters-data', this._canvasContainer);
		this._cursorOverlayDiv = window.L.DomUtil.create('div', 'cursor-overlay', this._canvasContainer);
		if (window.L.Browser.cypressTest) {
			this._emptyDeltaDiv = window.L.DomUtil.create('div', 'empty-deltas', this._canvasContainer);
			this._emptyDeltaDiv.innerText = 0;
		}
		this._splittersStyleData = new CStyleData(this._splittersDataDiv);

		this._painter = new window.L.TileSectionManager(this);

		app.sectionContainer.addSection(new cool.TilesSection());
		this._painter._tilesSection = app.sectionContainer.getSectionWithName('tiles');
		app.sectionContainer.setDocumentAnchorSection(app.CSections.Tiles.name);

		if (this._docType === 'text')
			app.sectionContainer.addSection(new CompareChangesLabelSection());

		app.sectionContainer.getSectionWithName('tiles').onResize();

		this._canvasOverlay = new CanvasOverlay(this._map, app.sectionContainer.getContext());
		app.sectionContainer.addSection(this._canvasOverlay);

		app.sectionContainer.addSection(new cool.ScrollSection(() => this.isCalcRTL()));

		// For mobile/tablet the hammerjs swipe handler already uses a requestAnimationFrame to fire move/drag events
		// Using window.L.TileSectionManager's own requestAnimationFrame loop to do the updates in that case does not perform well.
		if (window.mode.isSmallScreenDevice() || window.mode.isTablet()) {
			this._map.on('move', this._painter.update, this._painter);
			this._map.on('moveend', function () {
				setTimeout(this.update.bind(this), 200);
			}, this._painter);
		}
		this._map.on('zoomend', this._painter.update, this._painter);
		this._map.on('splitposchanged', function () {
			TileManager.update();
		}, this);
		this._map.on('sheetgeometrychanged', this._painter.update, this._painter);
		this._map.on('move', this._syncTilePanePos, this);

		this._map.on('viewrowcolumnheaders', this._painter.update, this._painter);
		this._map.on('messagesdone', TileManager.sendProcessedResponse, TileManager);

		if (this._docType === 'spreadsheet') {
			const calcGridSection = new app.definitions.calcGridSection();
			calcGridSection.sectionProperties.tsManager = this._painter;
			this._painter._calcGridSection = calcGridSection;
			app.sectionContainer.addSection(calcGridSection);
		}

		// Add it regardless of the file type.
		app.sectionContainer.addSection(new app.definitions.CommentSection());

		document.addEventListener('blur', this._onDocumentBlur.bind(this));
		document.addEventListener('focus', this._onDocumentFocus.bind(this));

		this._syncTileContainerSize();
	},

	// Returns true if the document type is Writer.
	isWriter: function() {
		return this._docType === 'text';
	},

	// Returns true if the document type is Calc.
	isCalc: function() {
		return this._docType === 'spreadsheet';
	},

	// Returns true if the document type is Impress.
	isImpress: function() {
		return this._docType === 'presentation';
	},

	// Returns true if the document type is Draw.
	isDraw: function() {
		return this._docType === 'drawing';
	},

	getContainer: function () {
		return this._container;
	},

	_updateZIndex: function () {
		if (this._container && this.options.zIndex !== undefined && this.options.zIndex !== null) {
			this._container.style.zIndex = this.options.zIndex;
		}
	},

	_onDocumentBlur: function() {
		// Reclaim the graphics memory of non-visible tiles when the document loses focus
		// (which happens when the browser is minimised, switched away from or when switching
		// tabs).
		// We do this as systems with poor video memory management can show bad side-effects
		// in other applications while we hold onto lots of video memory.
		this.onDocumentBlurTimeout = setTimeout(() => {
			this.onDocumentBlurTimeout = null;
			TileManager.clearPreFetch();
			TileManager.pruneTiles();
			TileManager.reclaimGraphicsMemory();
		}, 500);
	},

	_onDocumentFocus: function() {
		if (this.onDocumentBlurTimeout) {
			clearTimeout(this.onDocumentBlurTimeout);
			this.onDocumentBlurTimeout = null;
			return;
		}

		TileManager.resetPreFetching();

		// We may want to consider touching tiles in an area surrounding the visible area
		// for enhanced interactivity on the first scroll after refocusing.
		// Anecdotally, I don't notice any negative impact right now though.
	},

	_reset: function (hard) {
		var tileZoom = Math.round(this._map.getZoom()),
		    tileZoomChanged = this._tileZoom !== tileZoom;

		if (hard || tileZoomChanged) {
			this._resetClientVisArea();

			this._tileZoom = tileZoom;
			if (tileZoomChanged) {
				this._updateTileTwips();
				this._updateMaxBounds();
			}

			if (app.tile.size.x === 0 || app.tile.size.y === 0) {
				let tileWidthTwips = this.options.tileWidthTwips;
				app.twipsToPixels =  TileManager.tileSize / tileWidthTwips;
				app.pixelsToTwips = 1 / app.twipsToPixels;
				app.tile.size.pX = app.tile.size.pY = TileManager.tileSize;
			}

			if (!window.L.Browser.mobileWebkit)
				TileManager.update(this._map.getCenter(), tileZoom);

			if (tileZoomChanged)
				TileManager.pruneTiles();

			if (this._docType === 'spreadsheet')
				this._syncTileContainerSize();
		}
	},

	// These variables indicates the clientvisiblearea sent to the server and stored by the server
	// We need to reset them when we are reconnecting to the server or reloading a document
	// because the server needs new data even if the client is unmodified.
	_resetClientVisArea: function ()  {
		this._clientZoom = '';
		app.activeDocument.activeLayout.resetClientVisibleArea();
	},

	_resetCanonicalIdStatus: function() {
		this._canonicalIdInitialized = false;
	},

	_resetViewId: function () {
		this._viewId = undefined;
	},

	_resetDocumentInfo: function () {
		this._documentInfo = "";
	},

	_getViewId: function () {
		return this._viewId;
	},

	_updateTileTwips: function () {
		// smaller zoom = zoom in
		const factor = Math.pow(1.2, (this._map.options.zoom - this._tileZoom));
		const tileWidthTwips = Math.round(this.options.tileWidthTwips * factor);

		app.twipsToPixels = TileManager.tileSize / tileWidthTwips;
		app.pixelsToTwips = 1 / app.twipsToPixels;
		app.tile.size.pX = app.tile.size.pY = TileManager.tileSize;

		if (this._docType === 'spreadsheet')
			this._syncTileContainerSize();
	},

	_checkSpreadSheetBounds: function (newZoom) {
		// for spreadsheets, when the document is smaller than the viewing area
		// we want it to be glued to the row/column headers instead of being centered
		// In the future we probably want to remove this and set the bonds only on the
		// left/upper side of the spreadsheet so that we can have an 'infinite' number of
		// cells downwards and to the right, like we have on desktop
		var viewSize = this._map.getSize();
		var scale = this._map.getZoomScale(newZoom);
		var width = app.activeDocument.fileSize.x / app.tile.size.x * TileManager.tileSize * scale;
		var height = app.activeDocument.fileSize.y / app.tile.size.y * TileManager.tileSize * scale;
		if (width < viewSize.x || height < viewSize.y) {
			// if after zoomimg the document becomes smaller than the viewing area
			width = Math.max(width, viewSize.x);
			height = Math.max(height, viewSize.y);
			if (!this._map.options._origMaxBounds) {
				this._map.options._origMaxBounds = this._map.options.maxBounds;
			}
			scale = this._map.options.crs.scale(1);
			this._map.setMaxBounds(new window.L.LatLngBounds(
				this._map.unproject(new cool.Point(0, 0)),
				this._map.unproject(new cool.Point(width * scale, height * scale))));
		}
		else if (this._map.options._origMaxBounds) {
			// if after zoomimg the document becomes larger than the viewing area
			// we need to restore the initial bounds
			this._map.setMaxBounds(this._map.options._origMaxBounds);
			this._map.options._origMaxBounds = null;
		}
	},

	_moveStart: function () {
		TileManager.resetPreFetching();
		this._moveInProgress = true;
		this._moveTileRequests = [];
	},

	_move: function () {
		// We throttle the "move" event, but in moveEnd we always call
		// a _move anyway, so if there are throttled moves still
		// pending by the time moveEnd is called then there is no point
		// processing them after _moveEnd because we are up to date
		// already when they arrive and to do would just duplicate tile
		// requests
		if (!this._moveInProgress)
			return;

		TileManager.update();
		TileManager.resetPreFetching(true);
	},

	_isLatLngInView: function (position) {
		var centerOffset = this._map._getCenterOffset(position);
		var viewHalf = this._map.getSize()._divideBy(2);
		var positionInView =
			centerOffset.x > -viewHalf.x && centerOffset.x < viewHalf.x &&
			centerOffset.y > -viewHalf.y && centerOffset.y < viewHalf.y;
		return positionInView;
	},

	_moveEnd: function () {
		this._move();
		this._moveInProgress = false;
		this._moveTileRequests = [];
		app.updateFollowingUsers();
	},

	_requestNewTiles: function () {
		this.handleInvalidateTilesMsg('invalidatetiles: EMPTY');
		TileManager.update();
	},

	_sendClientZoom: function (forceUpdate) {
		if (!this._map._docLoaded)
			return;

		var newClientZoom = 'tilepixelwidth=' + TileManager.tileSize + ' ' +
		    'tilepixelheight=' + TileManager.tileSize + ' ' +
		    'tiletwipwidth=' + app.tile.size.x + ' ' +
		    'tiletwipheight=' + app.tile.size.y + ' ' +
		    'dpiscale=' + window.devicePixelRatio + ' ' +
		    'zoompercent=' + this._map.getZoomPercent()

		if (this._clientZoom !== newClientZoom || forceUpdate || this.isImpress()) {
			// the zoom level has changed
			app.socket.sendMessage('clientzoom ' + newClientZoom);

			if (!this._map._fatal && app.idleHandler._active && app.socket.connected())
				this._clientZoom = newClientZoom;
		}
	},

	_twipsRectangleToPixelBounds: function (strRectangle) {
		// TODO use this more
		// strRectangle = x, y, width, height
		var strTwips = strRectangle.match(/\d+/g);
		if (!strTwips) {
			return null;
		}
		var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
		var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));
		var bottomRightTwips = topLeftTwips.add(offset);
		return new cool.Bounds(
			this._twipsToPixels(topLeftTwips),
			this._twipsToPixels(bottomRightTwips));
	},

	_twipsRectanglesToPixelBounds: function (strRectangles) {
		// used when we have more rectangles
		strRectangles = strRectangles.split(';');
		var boundsList = [];
		for (var i = 0; i < strRectangles.length; i++) {
			var bounds = this._twipsRectangleToPixelBounds(strRectangles[i]);
			if (bounds) {
				boundsList.push(bounds);
			}
		}
		return boundsList;
	},

	getMaxDocSize: function () {
		return undefined;
	},

	getSnapDocPosX: function (docPosPixX) {
		return docPosPixX;
	},

	getSnapDocPosY: function (docPosPixY) {
		return docPosPixY;
	},

	getSplitPanesContext: function () {
		return undefined;
	},

	_createNewMouseEvent: function (type, inputEvent) {
		var event = inputEvent;
		if (inputEvent.type == 'touchstart' || inputEvent.type == 'touchmove') {
			event = inputEvent.touches[0];
		}
		else if (inputEvent.type == 'touchend') {
			event = inputEvent.changedTouches[0];
		}
		var newEvent = document.createEvent('MouseEvents');
		newEvent.initMouseEvent(
			type, true, true, window, 1,
			event.screenX, event.screenY,
			event.clientX, event.clientY,
			false, false, false, false, 0, null
		);
		return newEvent;
	},

	_getToolbarCommandsValues: function() {
		for (var i = 0; i < this._map.unoToolbarCommands.length; i++) {
			var command = this._map.unoToolbarCommands[i];
			app.socket.sendMessage('commandvalues command=' + command);
		}
	},

	_parseCellRange: function(cellRange) {
		var strTwips = cellRange.match(/\d+/g);
		var startCellAddress = [parseInt(strTwips[0]), parseInt(strTwips[1])];
		var endCellAddress = [parseInt(strTwips[2]), parseInt(strTwips[3])];
		return new cool.Bounds(startCellAddress, endCellAddress);
	},

	_cellRangeToTwipRect: function(cellRange) {
		var startCell = cellRange.getTopLeft();
		var startCellRectPixel = this.sheetGeometry.getCellRect(startCell.x, startCell.y);
		var topLeftTwips = this._corePixelsToTwips(startCellRectPixel.min);
		var endCell = cellRange.getBottomRight();
		var endCellRectPixel = this.sheetGeometry.getCellRect(endCell.x, endCell.y);
		var bottomRightTwips = this._corePixelsToTwips(endCellRectPixel.max);
		return new cool.Bounds(topLeftTwips, bottomRightTwips);
	},

	_onMessage: function (textMsg, img) {
		this._saveMessageForReplay(textMsg);
		// 'tile:' is the most common message type; keep this the first.
		if (textMsg.startsWith('tile:') || textMsg.startsWith('delta:')) {
			TileManager.onTileMsg(textMsg, img);
		}
		else if (textMsg.startsWith('commandvalues:')) {
			this._onCommandValuesMsg(textMsg);
		}
		else if (textMsg.startsWith('cursorvisible:')) {
			this._onCursorVisibleMsg(textMsg);
		}
		else if (textMsg.startsWith('downloadas:')) {
			this._onDownloadAsMsg(textMsg);
		}
		else if (textMsg.startsWith('exportfile:')) {
			this._onExportFileMsg(textMsg);
		}
		else if (textMsg.startsWith('error:')) {
			this._onErrorMsg(textMsg);
		}
		else if (textMsg.startsWith('getchildid:')) {
			this._onGetChildIdMsg(textMsg);
		}
		else if (textMsg.startsWith('shapeselectioncontent:')) {
			GraphicSelection.onShapeSelectionContent(textMsg);
		}
		else if (textMsg.startsWith('graphicselection:')) {
			this._map.fire('resettopbottompagespacing');
			GraphicSelection.onMessage(textMsg);
		}
		else if (textMsg.startsWith('graphicinnertextarea:')) {
			return; // Not used.
		}
		else if (textMsg.startsWith('cellcursor:')) {
			this._onCellCursorMsg(textMsg);
		}
		else if (textMsg.startsWith('celladdress:')) {
			this._onCellAddressMsg(textMsg);
		}
		else if (textMsg.startsWith('cellformula:')) {
			this._onCellFormulaMsg(textMsg);
		}
		else if (textMsg.startsWith('referencemarks:')) {
			this._onReferencesMsg(textMsg);
		}
		else if (textMsg.startsWith('referenceclear:')) {
			this._clearReferences();
		}
		else if (textMsg.startsWith('invalidatecursor:')) {
			this._onInvalidateCursorMsg(textMsg);
		}
		else if (textMsg.startsWith('invalidatetiles:')) {
			console.error("Message should be filterd during slurp");
		}
		else if (textMsg.startsWith('mousepointer:')) {
			this._onMousePointerMsg(textMsg);
		}
		else if (textMsg.startsWith('searchnotfound:')) {
			this._onSearchNotFoundMsg(textMsg);
		}
		else if (textMsg.startsWith('searchresultselection:')) {
			this._onSearchResultSelection(textMsg);
		}
		else if (textMsg.startsWith('setpart:')) {
			this._onSetPartMsg(textMsg);
		}
		else if (textMsg.startsWith('statechanged:')) {
			this._onStateChangedMsg(textMsg);
		}
		else if (textMsg.startsWith('status:') || textMsg.startsWith('statusupdate:')) {
			this._onStatusMsg(textMsg);

			// update tiles and selection because mode could be changed
			TileManager.update();
			app.definitions.otherViewGraphicSelectionSection.updateVisibilities();
			TextCursorSection.updateVisibilities();
		}
		else if (textMsg.startsWith('partstatus:')) {
			this._onStatusMsg(textMsg);
		}
		else if (textMsg.startsWith('textselection:')) {
			this._onTextSelectionMsg(textMsg);
		}
		else if (textMsg.startsWith('textselectioncontent:')) {
			let textMsgContent = textMsg.substr(22);

			if (this._map._clip && this._map._clip.isActionCopy()) {
				// This is the Action_Copy PostMessage API, create the response and
				// don't touch the clipboard.
				this._map._clip.setActionCopy(false);
				const message = {
					msgId: 'Action_Copy_Resp',
					args: {
						content: textMsgContent
					}
				};
				this._map.fire('postMessage', message);
				return;
			}

			let textMsgHtml = '';
			let textMsgPlainText = '';
			if (textMsgContent.startsWith('{')) {
				// Multiple formats: JSON.
				let textMsgJson = JSON.parse(textMsgContent);
				textMsgHtml = textMsgJson['text/html'];
				textMsgPlainText = textMsgJson['text/plain;charset=utf-8'];
			} else {
				// Single format: as-is.
				textMsgHtml = textMsgContent;
			}

			if (this._map._clip) {
				this._map._clip.setTextSelectionHTML(textMsgHtml, textMsgPlainText);
			} else
				// hack for ios and android to get selected text into hyperlink insertion dialog
				this._selectedTextContent = textMsgHtml;

			// Fire map event for external listeners
			if (this._map) {
				this._map.fire('textselectioncontent', {
					msg: textMsg,
					html: textMsgHtml,
					plainText: textMsgPlainText
				});
			}
	}
		else if (textMsg.startsWith('clipboardchanged')) {
			var jMessage = textMsg.substr(17);
			jMessage = JSON.parse(jMessage);

			if (jMessage.mimeType === 'text/plain') {
				this._map._clip.setTextSelectionHTML(jMessage.content);

				// If _navigatorClipboardWrite is available, use it.
				if (window.L.Browser.clipboardApiAvailable || window.ThisIsTheiOSApp)
					this._map.fire('clipboardchanged', { commandName: '.uno:CopyHyperlinkLocation' });
				else // Or use previous method.
					this._map._clip._execCopyCutPaste('copy');
			}
		}
		else if (textMsg.startsWith('textselectionend:')) {
			this._onTextSelectionEndMsg(textMsg);
		}
		else if (textMsg.startsWith('textselectionstart:')) {
			this._onTextSelectionStartMsg(textMsg);
		}
		else if (textMsg.startsWith('cellselectionarea:')) {
			this._onCellSelectionAreaMsg(textMsg);
		}
		else if (textMsg.startsWith('cellautofillarea:')) {
			this._onCellAutoFillAreaMsg(textMsg);
		}
		else if (textMsg.startsWith('complexselection:')) {
			if (this._map._clip)
				this._map._clip.onComplexSelection(textMsg.substr('complexselection:'.length));
			if (this._map)
				this._map.fire('complexselection', { msg: textMsg });
		}
		else if (textMsg.startsWith('windowpaint:')) {
			this._onDialogPaintMsg(textMsg, img);
		}
		else if (textMsg.startsWith('window:')) {
			this._onDialogMsg(textMsg);
		}
		else if (textMsg.startsWith('unocommandresult:')) {
			this._onUnoCommandResultMsg(textMsg);
		}
		else if (textMsg.startsWith('aichatresult:')) {
			try {
				var json = JSON.parse(textMsg.substring('aichatresult:'.length));
				this._map.fire('aichatresult', json);
			} catch (e) {
				window.app.console.error('Failed to parse aichatresult: ' + e);
			}
		}
		else if (textMsg.startsWith('aichatprogress:')) {
			try {
				var json = JSON.parse(textMsg.substring('aichatprogress:'.length));
				this._map.fire('aichatprogress', json);
			} catch (e) {
				window.app.console.error('Failed to parse aichatprogress: ' + e);
			}
		}
		else if (textMsg.startsWith('aichatapproval:')) {
			try {
				var json = JSON.parse(textMsg.substring('aichatapproval:'.length));
				this._map.fire('aichatapproval', json);
			} catch (e) {
				window.app.console.error('Failed to parse aichatapproval: ' + e);
			}
		}
		else if (textMsg.startsWith('hrulerupdate:')) {
			this._onRulerUpdate(textMsg);
		}
		else if (textMsg.startsWith('vrulerupdate:')) {
			this._onRulerUpdate(textMsg);
		}
		else if (textMsg.startsWith('contextmenu:')) {
			this._onContextMenuMsg(textMsg);
		}
		else if (textMsg.startsWith('invalidateviewcursor:')) {
			this._onInvalidateViewCursorMsg(textMsg);
		}
		else if (textMsg.startsWith('viewcursorvisible:')) {
			this._onViewCursorVisibleMsg(textMsg);
		}
		else if (textMsg.startsWith('cellviewcursor:')) {
			this._onCellViewCursorMsg(textMsg);
		}
		else if (textMsg.startsWith('viewlock')) {
			this._onViewLockInfoMsg(textMsg);
		}
		else if (textMsg.startsWith('viewinfo:')) {
			this._onViewInfoMsg(textMsg);
		}
		else if (textMsg.startsWith('textviewselection:')) {
			this._onTextViewSelectionMsg(textMsg);
		}
		else if (textMsg.startsWith('graphicviewselection:')) {
			this._onGraphicViewSelectionMsg(textMsg);
		}
		else if (textMsg.startsWith('tableselected:')) {
			app.activeDocument.tableMiddleware.onTableSelectedMsg(textMsg);
		}
		else if (textMsg.startsWith('editor:')) {
			this._updateEditor(textMsg);
		}
		else if (textMsg.startsWith('validitylistbutton:')) {
			this._onValidityListButtonMsg(textMsg);
		}
		else if (textMsg.startsWith('validityinputhelp:')) {
			this._onValidityInputHelpMsg(textMsg);
		}
		else if (textMsg.startsWith('signaturestatus:')) {
			var signstatus = textMsg.substring('signaturestatus:'.length + 1);
			this._map.onChangeSignStatus(signstatus);
		}
		else if (textMsg.startsWith('removesession')) {
			var viewId = parseInt(textMsg.substring('removesession'.length + 1));
			if (this._map._docLayer._viewId === viewId)
				app.dispatcher.dispatch('closeapp');
		}
		else if (textMsg.startsWith('calcfunctionlist:')) {
			this._onCalcFunctionListMsg(textMsg.substring('calcfunctionlist:'.length + 1));
		}
		else if (textMsg.startsWith('tooltip:')) {
			var tooltipInfo = JSON.parse(textMsg.substring('tooltip:'.length + 1));
			if (tooltipInfo.type === 'formulausage') {
				this._onCalcFunctionUsageMsg(tooltipInfo.text);
			}
			else if (tooltipInfo.type === 'generaltooltip') {
				var tooltipInfo = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
				this._map.uiManager.showDocumentTooltip(tooltipInfo);
			}
			else if (tooltipInfo.type === 'autofillpreviewtooltip') {

				var strTwips = textMsg.match(/\d+/g);
				if (strTwips != null && this._map.isEditMode())
					this._map.fire('openautofillpreviewpopup', { data: tooltipInfo });
			}
			else {
				console.error('unknown tooltip type');
			}
		}
		else if (textMsg.startsWith('tabstoplistupdate:')) {
			this._onTabStopListUpdate(textMsg);
		}
		else if (textMsg.startsWith('context:')) {
			var message = textMsg.substring('context:'.length + 1);
			message = message.split(' ');
			if (message.length > 1) {
				var old = this._map.context || {};
				var newContext = {appId: message[0], context: message[1]};
				if (old.appId !== newContext.appId || old.context !== newContext.context) {
					this._map.context = newContext;
					app.events.fire('contextchange', {
						appId: newContext.appId, context: newContext.context,
						oldAppId: old.appId, oldContext: old.context
					});
				}
			}
		}
		else if (textMsg.startsWith('formfieldbutton:')) {
			this._onFormFieldButtonMsg(textMsg);
		}
		else if (textMsg.startsWith('canonicalidchange:')) {
			var payload = textMsg.substring('canonicalidchange:'.length + 1);
			var viewRenderedState = payload.split('=')[3].split(' ')[0];
			if (this._debug.overlayOn) {
				var viewId = payload.split('=')[1].split(' ')[0];
				var canonicalId = payload.split('=')[2].split(' ')[0];
				this._debug.setOverlayMessage('canonicalViewId',
					'Canonical id changed to: ' + canonicalId + ' for view id: ' + viewId + ' with view renderend state: ' + viewRenderedState
				);
			}
			if (!this._canonicalIdInitialized) {
				this._canonicalIdInitialized = true;
				TileManager.update();
			} else {
				this._requestNewTiles();
				this._invalidateAllPreviews();
				TileManager.redraw();
			}
		}
		else if (textMsg.startsWith('comment:')) {
			var obj = JSON.parse(textMsg.substring('comment:'.length + 1));
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).onACKComment(obj);
		}
		else if (textMsg.startsWith('redlinetablemodified:')) {
			obj = JSON.parse(textMsg.substring('redlinetablemodified:'.length + 1));
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).onACKComment(obj);
		}
		else if (textMsg.startsWith('redlinetablechanged:')) {
			obj = JSON.parse(textMsg.substring('redlinetablechanged:'.length + 1));
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).onACKComment(obj);
		}
		else if (textMsg.startsWith('applicationbackgroundcolor:')) {
			app.sectionContainer.setClearColor('#' + textMsg.substring('applicationbackgroundcolor:'.length + 1).trim());
			app.sectionContainer.requestReDraw();
		}
		else if (textMsg.startsWith('documentbackgroundcolor:')) {
			app.sectionContainer.setDocumentBackgroundColor('#' + textMsg.substring('documentbackgroundcolor:'.length + 1).trim());
		}
		else if (textMsg.startsWith('contentcontrol:')) {
			textMsg = textMsg.substring('contentcontrol:'.length + 1);
			if (!app.sectionContainer.doesSectionExist(app.CSections.ContentControl.name)) {
				app.sectionContainer.addSection(new cool.ContentControlSection());
			}
			var section = app.sectionContainer.getSectionWithName(app.CSections.ContentControl.name);
			section.drawContentControl(JSON.parse(textMsg));
		}
		else if (textMsg.startsWith('versionbar:')) {
			obj = JSON.parse(textMsg.substring('versionbar:'.length + 1));
			this._map.fire('versionbar', obj);
		}
		else if (textMsg.startsWith('lockaccessibilityon')) {
			// a11y forced on by DocumentBroker, from interface settings overrides.
			this._map.lockAccessibilityOn();
		}
		else if (textMsg.startsWith('a11y')) {
			if (!window.prefs.getBoolean('accessibilityState'))
				throw 'A11y events come from the core while it is disabled in the client session.';

			if (textMsg.startsWith('a11yfocuschanged:')) {
				obj = JSON.parse(textMsg.substring('a11yfocuschanged:'.length + 1));
				var listPrefixLength = obj.listPrefixLength !== undefined ? parseInt(obj.listPrefixLength) : 0;
				if (typeof this._map._textInput.onAccessibilityFocusChanged === 'function') {
					this._map._textInput.onAccessibilityFocusChanged(
						obj.content,
						parseInt(obj.position),
						parseInt(obj.start),
						parseInt(obj.end),
						listPrefixLength,
						parseInt(obj.force) > 0);
				}
			}
			else if (textMsg.startsWith('a11ycaretchanged:')) {
				obj = JSON.parse(textMsg.substring('a11yfocuschanged:'.length + 1));
				if (typeof this._map._textInput.onAccessibilityCaretChanged === 'function')
					this._map._textInput.onAccessibilityCaretChanged(parseInt(obj.position));
			}
			else if (textMsg.startsWith('a11ytextselectionchanged:')) {
				obj = JSON.parse(textMsg.substring('a11ytextselectionchanged:'.length + 1));
				this._map._textInput.onAccessibilityTextSelectionChanged(parseInt(obj.start), parseInt(obj.end));
			}
			else if (textMsg.startsWith('a11yfocusedcellchanged:')) {
				obj = JSON.parse(textMsg.substring('a11yfocusedcellchanged:'.length + 1));
				var outCount = obj.outCount !== undefined ? parseInt(obj.outCount) : 0;
				var inList = obj.inList !== undefined ? obj.inList : [];
				var row = parseInt(obj.row);
				var col = parseInt(obj.col);
				var rowSpan = obj.rowSpan !== undefined ? parseInt(obj.rowSpan) : 1;
				var colSpan = obj.colSpan !== undefined ? parseInt(obj.colSpan) : 1;
				this._map._textInput.onAccessibilityFocusedCellChanged(
						outCount, inList, row, col, rowSpan, colSpan, obj.paragraph);
			}
			else if (textMsg.startsWith('a11yeditinginselectionstate:')) {
				obj = JSON.parse(textMsg.substring('a11yeditinginselectionstate:'.length + 1));
				this._map._textInput.onAccessibilityEditingInSelectionState(
					parseInt(obj.cell) > 0, parseInt(obj.enabled) > 0, obj.selection, obj.paragraph);
			}
			else if (textMsg.startsWith('a11yselectionchanged:')) {
				obj = JSON.parse(textMsg.substring('a11yselectionchanged:'.length + 1));
				this._map._textInput.onAccessibilitySelectionChanged(
					parseInt(obj.cell) > 0, obj.action, obj.name, obj.text);
			}
			else if (textMsg.startsWith('a11yfocusedparagraph:')) {
				obj = JSON.parse(textMsg.substring('a11yfocusedparagraph:'.length + 1));
				this._map._textInput.setA11yFocusedParagraph(
					obj.content, parseInt(obj.position), parseInt(obj.start), parseInt(obj.end));
			}
			else if (textMsg.startsWith('a11ycaretposition:')) {
				var pos = textMsg.substring('a11ycaretposition:'.length + 1);
				this._map._textInput.setA11yCaretPosition(parseInt(pos));
			}
		}
		else if (textMsg.startsWith('colorpalettes:')) {
			var json = JSON.parse(textMsg.substring('colorpalettes:'.length + 1));

			for (var key in json) {
				if(key === 'ColorNames') {
					window.app.colorNames = json[key];
					continue;
				}
				if (app.colorPalettes[key]) {
					app.colorPalettes[key].colors = json[key];
				} else {
					window.app.console.warn('Unknown palette: "' + key + '"');
				}
			}

			// Remove empty palettes, eg. Document colors in Impress are empty
			for (var key in app.colorPalettes) {
				if (!app.colorPalettes[key].colors || !app.colorPalettes[key].colors.length) {
					delete app.colorPalettes[key];
				}
			}
		} else if (textMsg.startsWith('serveraudit:')) {
			var serverAudit = textMsg.substr(12).trim();
			if (serverAudit !== 'disabled') {
				// if isAdminUser property is not set by integration - enable audit dialog for all users
				if (app.isAdminUser !== false) {
					this._map.serverAuditDialog = JSDialog.serverAuditDialog(this._map);

					if (this._map.uiManager.notebookbar) {
						this._map.uiManager.notebookbar.showItem('server-audit');
						this._map.uiManager.notebookbar.showItem('help-serveraudit-break');
					}
				}

				var json = JSON.parse(serverAudit);
				app.setServerAuditFromCore(json.serverAudit);
			}
		} else if (textMsg.startsWith('adminuser:')) {
			var value = textMsg.substr(10).trim();
			if (value === 'true')
				app.isAdminUser = true;
			else if (value === 'false')
				app.isAdminUser = false;
			else
				app.isAdminUser = null;

			this._map.fire('adminuser');
		} else if (textMsg.startsWith('presentationinfo:')) {
			var content = JSON.parse(textMsg.substring('presentationinfo:'.length + 1));
			this._map.fire('presentationinfo', content);
		} else if (textMsg.startsWith('slideshowfollow')) {
			const eventInfo = textMsg.substr('slideshowfollow '.length);
			const parameterStartIndex = eventInfo.indexOf('{');
			if (parameterStartIndex === -1) {
				this._map.fire(eventInfo);
			} else {
				const event = eventInfo.substring(0, parameterStartIndex).trim();
				const parameter = JSON.parse(eventInfo.substring(parameterStartIndex));
				this._map.fire(event, parameter);
			}
		} else if (textMsg.startsWith('changepagezoom:')) {
			const payload = textMsg.substring('changepagezoom:'.length).trim();
			const strZoomPercent = payload.match(/\d+/);
			const zoomPercent = strZoomPercent ? parseInt(strZoomPercent[0], 10) : NaN;
			if (zoomPercent) {
				const zoomIndex = this._map.getZoomIndex(zoomPercent);
				this._map.setZoom(zoomIndex, null, false);
			}
		}
	},

	_onInvalidateTilesMsg: function (textMsg) {
		const command = app.socket.parseServerCmd(textMsg);
		if (command.x === undefined || command.y === undefined || command.part === undefined) {
			var strTwips = textMsg.match(/\d+/g);
			command.x = parseInt(strTwips[0]);
			command.y = parseInt(strTwips[1]);
			command.width = parseInt(strTwips[2]);
			command.height = parseInt(strTwips[3]);
			command.part = this._selectedPart;
		}

		if (isNaN(command.mode))
			command.mode = app.activeDocument.activeModes[0];

		const invalidArea = new cool.SimpleRectangle(command.x, command.y, command.width, command.height);
		TileManager.overlapInvalidatedRectangleWithView(command.part, command.mode, command.wireId, invalidArea, textMsg);

		if (this.isImpress() || this.isDraw()) {
			if (command.part === this._selectedPart &&
				app.activeDocument.isModeActive(command.mode) &&
				command.part !== this._lastValidPart) {
				this._map.fire('updatepart', {part: this._lastValidPart, docType: this._docType});
				this._lastValidPart = command.part;
				this._map.fire('updatepart', {part: command.part, docType: this._docType});
			}

			const preview = this._map._docPreviews ? this._map._docPreviews[command.part] : null;
			if (preview) { preview.invalid = true; }

			const topLeftTwips = new cool.Point(command.x, command.y);
			const offset = new cool.Point(command.width, command.height);
			const bottomRightTwips = topLeftTwips.add(offset);
			this._previewInvalidations.push(new cool.Bounds(topLeftTwips, bottomRightTwips));
			// 1s after the last invalidation, update the preview
			clearTimeout(this._previewInvalidator);
			this._previewInvalidator = setTimeout(window.L.bind(this._invalidatePreviews, this), this.options.previewInvalidationTimeout);
		}
	},

	handleInvalidateTilesMsg: function(textMsg) {
		var payload = textMsg.substring('invalidatetiles:'.length + 1);
		if (!payload.startsWith('EMPTY')) {
			this._onInvalidateTilesMsg(textMsg);
		}
		else {
			var msg = 'invalidatetiles: ';

			// see invalidatetiles: in wsd/protocol.txt for structure
			var tmp = payload.substring('EMPTY'.length).replaceAll(',', ' , ');
			var tokens = tmp.split(/[ \n]+/);

			var wireIdToken = undefined;
			var commaargs = [];

			var commaarg = false;
			for (var i = 0; i < tokens.length; i++) {
				if (tokens[i] === ',') {
					commaarg = true;
					continue;
				}
				if (commaarg) {
					commaargs.push(tokens[i]);
					commaarg = false;
				}
				else if (tokens[i].startsWith('wid=')) {
					wireIdToken = tokens[i];
				}
				else if (tokens[i])
					console.error('unsupported invalidatetile token: ' + tokens[i]);
			}

			if (this.isWriter()) {
				msg += 'part=0 ';
			} else {

				var part = parseInt(commaargs.length > 0 ? commaargs[0] : '');
				var mode = parseInt(commaargs.length > 1 ? commaargs[1] : '');

				mode = (isNaN(mode) ? app.activeDocument.activeModes[0] : mode);
				msg += 'part=' + (isNaN(part) ? this._selectedPart : part)
					+ ((mode && mode !== 0) ? (' mode=' + mode) : '')
					+ ' ';
			}
			msg += 'x=0 y=0 ';
			msg += 'width=' + Number.MAX_SAFE_INTEGER + ' ';
			msg += 'height=' + Number.MAX_SAFE_INTEGER;
			if (wireIdToken !== undefined)
				msg += ' ' + wireIdToken;
			this._onInvalidateTilesMsg(msg);
		}
	},

	// Process messages early that won't mess with the DOM
	filterSlurpedMessage: function(evt) {
		var textMsg = evt.textMsg;

		if (textMsg.startsWith('invalidatetiles:')) {
			app.socket._logSocket('INCOMING', textMsg);
			this.handleInvalidateTilesMsg(textMsg);
			return true; // filter
		}

		return false; // continue processing
	},

	_onTabStopListUpdate: function (textMsg) {
		textMsg = textMsg.substring('tabstoplistupdate:'.length + 1);
		var json = JSON.parse(textMsg);
		this._map.fire('tabstoplistupdate', json);
	},

	_onCommandValuesMsg: function (textMsg) {
		var jsonIdx = textMsg.indexOf('{');
		if (jsonIdx === -1) {
			return;
		}
		var obj = JSON.parse(textMsg.substring(jsonIdx));
		if (obj.commandName === '.uno:DocumentRepair') {
			this._onDocumentRepair(obj);
		}
		else if (obj.commandName === '.uno:CellCursor') {
			this._onCellCursorMsg(obj.commandValues);
		}
		else if (this._map.unoToolbarCommands.indexOf(obj.commandName) !== -1) {
			this._toolbarCommandValues[obj.commandName] = obj.commandValues;
			this._map.fire('updatetoolbarcommandvalues', {
				commandName: obj.commandName,
				commandValues: obj.commandValues
			});
		}
		else {
			this._map.fire('commandvalues', {
				commandName: obj.commandName,
				commandValues: obj.commandValues
			});
		}
	},

	_onCellAddressMsg: function (textMsg) {
		// When the user moves the focus to a different cell, a 'cellformula'
		// message is received from coolwsd, *then* a 'celladdress' message.
		var address = textMsg.substring(13);
		if (this._map._clip && !this._map['wopi'].DisableCopy) {
			this._map._clip.setTextSelectionText(this._lastFormula);
		}
		this._map.fire('celladdress', {address: address});
	},

	_onCellFormulaMsg: function (textMsg) {
		// When a 'cellformula' message from coolwsd is received,
		// store the text contents of the cell, but don't push
		// them to the clipboard container (yet).
		// This is done because coolwsd will send several 'cellformula'
		// messages during text composition, and resetting the contents
		// of the clipboard container mid-composition will easily break it.

		let newFormula = textMsg.substring(13);
		if (this._lastFormula) {
			let minLength = Math.min(newFormula.length, this._lastFormula.length);
			let index = -1;
			for (let i = 0; i < minLength; i++) {
				if (newFormula.charAt(i) !== this._lastFormula.charAt(i)) {
					index = i;
					break;
				}
			}

			if (index === -1)
				index = newFormula.length-1;

			// newFormulaDiffIndex have index of last added character in formula
			// It is used during Formula Autocomplete to find partial remaining text
			this._newFormulaDiffIndex = index;
		}
		this._lastFormula = newFormula;
		this._map.fire('cellformula', {formula: newFormula});

		// Clear pending error; statechanged CellFormulaError will set it
		// if the new cell has one.
		this._pendingCellError = null;
	},

	_onCellFormulaError: function (state) {
		if (state && typeof state === 'object' && state.error) {
			this._pendingCellError = state;
		} else {
			this._pendingCellError = null;
			app.definitions.formulaErrorHelpSection.hide();
		}
		this._showPendingCellError();
	},

	_showPendingCellError: function () {
		if (!this._pendingCellError || !app.calc.cellCursorVisible)
			return;
		var rect = app.calc.cellCursorRectangle;
		var pos = new cool.SimplePoint(rect.x1, rect.y2);
		app.definitions.formulaErrorHelpSection.show(pos, this._pendingCellError);
	},

	_onCalcFunctionUsageMsg: function (textMsg) {
		this._map.fire('closepopup');
		this._map.fire('sendformulausagetext', {data: textMsg});
	},

	_onCalcFunctionListMsg: function (textMsg) {
		if (textMsg.startsWith('hidetip')) {
			this._map.fire('closepopup');
		} else {
			var funcData = JSON.parse(textMsg);

			if (window.mode.isSmallScreenDevice()) {
				this._closeMobileWizard();

				var data = {
					id: 'funclist',
					type: '',
					text: _('Functions'),
					enabled: true,
					children: []
				};

				if (funcData.categories)
					this._onCalcFunctionListWithCategories(funcData, data);
				else
					this._onCalcFunctionList(funcData, data);

				if (funcData.wholeList)
					this._map._functionWizardData = data;

				this._openMobileWizard(data);
			}
			else {
				var functionList = this._getFunctionList(textMsg);
				this._map.fire('sendformulatext', {data: functionList});
			}
		}
	},

	_getCalcFunctionListEntry: function(name, category, index, signature, description) {
		return  {
			id: '',
			type: 'calcfuncpanel',
			text: name,
			functionName: name,
			index: index,
			category: category,
			enabled: true,
			children: [
				{
					id: '',
					type: 'fixedtext',
					html: '<div class="func-info-sig">' + signature + '</div>' + '<div class="func-info-desc">' + description + '</div>',
					enabled: true,
					style: 'func-info'
				}
			]
		};
	},

	_onCalcFunctionList: function (funcList, data) {
		var entries = data.children;
		for (var idx = 0; idx < funcList.length; ++idx) {
			var func =  funcList[idx];
			var name = func.signature.split('(')[0];
			entries.push(this._getCalcFunctionListEntry(
				name, undefined, func.index, func.signature, func.description));
		}
	},

	_onCalcFunctionListWithCategories: function (funcData, data) {
		var categoryList = funcData.categories;
		var categoryEntries = data.children;
		for (var idx = 0; idx < categoryList.length; ++idx) {
			var category = categoryList[idx];
			var categoryEntry = {
				id: '',
				type: 'panel',
				text: category.name,
				index: idx,
				enabled: true,
				children: []
			};
			categoryEntries.push(categoryEntry);
		}

		var funcList = funcData.functions;
		for (idx = 0; idx < funcList.length; ++idx) {
			var func =  funcList[idx];
			var name = func.signature.split('(')[0];
			var funcEntries = categoryEntries[func.category].children;
			funcEntries.push(this._getCalcFunctionListEntry(
				name, func.category, func.index, func.signature, func.description));
		}
	},

	_onCursorVisibleMsg: function(textMsg) {
		var command = textMsg.match('cursorvisible: true');
		app.setCursorVisibility(command ? true : false);
		this._onUpdateCursor();
		app.events.fire('TextCursorVisibility', { visible: app.file.textCursor.visible });
	},

	_onDownloadAsMsg: function (textMsg) {
		var command = app.socket.parseServerCmd(textMsg);
		var parser = document.createElement('a');
		parser.href = window.host;

		var url = window.makeHttpUrlWopiSrc('/' + this._map.options.urlPrefix + '/',
			this._map.options.doc, '/download/' + command.downloadid);

		this._map.hideBusy();
		if (this._map['wopi'].DownloadAsPostMessage) {
			this._map.fire('postMessage', {msgId: 'Download_As', args: {Type: command.id, URL: url, filename: command.filename}});
		}
		else if (command.id === 'print') {
			if (this._map.options.print === false || window.L.Browser.cypressTest) {
				// open the pdf in a new tab, it can be printed directly in the browser's pdf viewer
				url = window.makeHttpUrlWopiSrc('/' + this._map.options.urlPrefix + '/',
					this._map.options.doc, '/download/' + command.downloadid,
					'attachment=0');

				if ('processCoolUrl' in window) {
					url = window.processCoolUrl({ url: url, type: 'print' });
				}

				window.open(url, '_blank');
			}
			else {
				if ('processCoolUrl' in window) {
					url = window.processCoolUrl({ url: url, type: 'print' });
				}

				this._map.fire('filedownloadready', {url: url});
			}
		}
		else if (command.id === 'slideshow') {
			this._map.fire('slidedownloadready', {url: url});
		}
		else if (command.id === 'export') {
			if ('processCoolUrl' in window) {
				url = window.processCoolUrl({ url: url, type: 'export' });
			}

			// Don't do a real download during testing
			if (!window.L.Browser.cypressTest)
				this._map._fileDownloader.src = url;
			else
				this._map._fileDownloader.setAttribute('data-src', url);
		}
	},

	_onExportFileMsg: function (textMsg) {
		this._map.hideBusy();
		var command = app.socket.parseServerCmd(textMsg);
		if (command.url) {
			window.postMobileMessage('exportfile url=' + command.url);
		}
	},

	_onErrorMsg: function (textMsg) {
		var command = app.socket.parseServerCmd(textMsg);

		// let's provide some convenience error codes for the UI
		var errorId = 1; // internal error
		if (command.errorCmd === 'load') {
			errorId = 2; // document cannot be loaded
		}
		else if (command.errorCmd === 'save' || command.errorCmd === 'saveas') {
			errorId = 5; // document cannot be saved
		}

		var errorCode = -1;
		if (command.errorCode !== undefined) {
			errorCode = command.errorCode;
		}

		this._map.fire('error', {cmd: command.errorCmd, kind: command.errorKind, id: errorId, code: errorCode});
	},

	_onGetChildIdMsg: function (textMsg) {
		var command = app.socket.parseServerCmd(textMsg);
		this._map.fire('childid', {id: command.id});
	},

	_openMobileWizard: function(data) {
		this._map.fire('mobilewizard', {data: data});
	},

	_closeMobileWizard: function() {
		this._map.fire('closemobilewizard');
	},

	_onGraphicViewSelectionMsg: function (textMsg) {
		var obj = JSON.parse(textMsg.substring('graphicviewselection:'.length + 1));
		var viewId = parseInt(obj.viewId);

		// Ignore if viewid is ours or not in our db
		if (viewId === this._viewId || !this._map._viewInfo[viewId]) {
			return;
		}

		var strTwips = obj.selection.match(/\d+/g);

		app.definitions.otherViewGraphicSelectionSection.addOrUpdateGraphicSelectionIndicator(viewId, strTwips, parseInt(obj.part), obj.mode !== undefined ? parseInt(obj.mode): 0);

		if (app.getFollowedViewId() === viewId && app.isFollowingUser()) {
			if (this.isImpress() || this.isDraw() || this.isWriter()) {
				this.goToOtherUserView(viewId);
			}
		}

		this._saveMessageForReplay(textMsg, viewId);
	},

	_onCellCursorMsg: function (textMsg) {
		var cellfillMarkerSection = app.sectionContainer.getSectionWithName(app.CSections.CellFillMarker.name);

		var oldCursorAddress = app.calc.cellAddress.clone();

		if (textMsg.match('EMPTY')) {
			app.calc.cellCursorVisible = false;
			if (cellfillMarkerSection)
				cellfillMarkerSection.calculatePositionViaCellCursor(null);
			if (this._map._clip)
				this._map._clip.clearSelection();
		}
		else {
			var strTwips = textMsg.match(/\d+/g);
			var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
			var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));
			var bottomRightTwips = topLeftTwips.add(offset);
			let _cellCursorTwips = this._convertToTileTwipsSheetArea(new cool.Bounds(topLeftTwips, bottomRightTwips));

			app.calc.cellAddress = new cool.SimplePoint(parseInt(strTwips[4]), parseInt(strTwips[5]));
			let tempRectangle = _cellCursorTwips.toRectangle();
			app.calc.cellCursorRectangle = new cool.SimpleRectangle(tempRectangle[0], tempRectangle[1], tempRectangle[2], tempRectangle[3]);
			this._cellCursorSection.size[0] = app.calc.cellCursorRectangle.pWidth;
			this._cellCursorSection.size[1] = app.calc.cellCursorRectangle.pHeight;
			this._cellCursorSection.setPosition(app.calc.cellCursorRectangle.pX1, app.calc.cellCursorRectangle.pY1);
			app.calc.cellCursorVisible = true;

			app.sectionContainer.onCellAddressChanged();
			if (cellfillMarkerSection)
				cellfillMarkerSection.calculatePositionViaCellCursor([app.calc.cellCursorRectangle.pX2, app.calc.cellCursorRectangle.pY2]);
		}

		var sameAddress = oldCursorAddress.equals(app.calc.cellAddress.toArray());

		var isFollowingOwnCursor = parseInt(app.getFollowedViewId()) === parseInt(this._viewId);
		var notJump = sameAddress || !isFollowingOwnCursor;
		var scrollToCursor = this._sheetSwitch.tryRestore(notJump, this._selectedPart);

		this._onUpdateCellCursor(scrollToCursor, notJump);

		// Remove input help if there is any:
		app.definitions.validityInputHelpSection.removeValidityInputHelp();

		// Reposition formula error button with the updated cursor rect.
		app.definitions.formulaErrorHelpSection.hide();
		this._showPendingCellError();
	},

	_onDocumentRepair: function (textMsg) {
		if (!this._docRepair) {
			this._docRepair = window.L.control.documentRepair();
		}

		if (!this._docRepair.isVisible()) {
			this._docRepair.addTo(this._map);
			this._docRepair.fillActions(textMsg);
			this._docRepair.show();
		}
	},

	_onMousePointerMsg: function (textMsg) {
		textMsg = textMsg.substring(14); // "mousepointer: "
		textMsg = Cursor.getCustomCursor(textMsg) || textMsg;
		this._coreMousePointer = textMsg;
		const canvas = document.getElementById('document-canvas');
		if (canvas && canvas.style.cursor !== textMsg) {
			canvas.style.cursor = textMsg;
		}
	},

	_getFunctionList: function(textMsg) {
		var resultList = [];
		var suggestionArray = JSON.parse(textMsg);
		for (var i = 0; i < suggestionArray.length; i++) {
			var signature = suggestionArray[i].signature;
			var namedRange = suggestionArray[i].namedRange;
			var name, description;
			if (namedRange) {
				name = signature;
				description = _('Named Range');
			} else {
				name = signature.substring(0,signature.indexOf('('));
				description = suggestionArray[i].description;
			}
			resultList.push({'name': name, 'description': description, 'namedRange': namedRange});
		}
		return resultList;
	},

	_onInvalidateCursorMsg: function (textMsg) {
		textMsg = textMsg.substring('invalidatecursor:'.length + 1);
		var obj = JSON.parse(textMsg);
		var recCursor = this._getEditCursorRectangle(obj);
		if (recCursor === undefined || this.persistCursorPositionInWriter) {
			this.persistCursorPositionInWriter = false;
			return;
		}

		// tells who trigerred cursor invalidation, but recCursors is still "ours"
		var modifierViewId = parseInt(obj.viewId);
		var weAreModifier = (modifierViewId === this._viewId);
		if (weAreModifier && app.isFollowingOff())
			app.setFollowingUser(this._viewId);

		this._cursorAtMispelledWord = obj.mispelledWord ? Boolean(parseInt(obj.mispelledWord)).valueOf() : false;

		if (obj.controlEvent === true)
			this._formID = obj.windowId;
		else
			this._formID = null;

		// Remember the last position of the caret (in core pixels).
		this._cursorPreviousPositionCorePixels = app.file.textCursor.rectangle.clone();

		app.file.textCursor.rectangle = new cool.SimpleRectangle(recCursor.getTopLeft().x, recCursor.getTopLeft().y, recCursor.getSize().x, recCursor.getSize().y);

		if (this.isWriter()) {
			app.sectionContainer.onCursorPositionChanged();
		}

		if (!this._map.editorHasFocus() && app.file.textCursor.visible && weAreModifier) {
			// Regain cursor if we had been out of focus and now have input.
			// Unless the focus is in the Calc Formula-Bar, don't steal the focus.
			if (!this._map.calcInputBarHasFocus())
				this._map.fire('editorgotfocus');
		}

		//first time document open, set last cursor position
		if (!this.lastCursorPos)
			this.lastCursorPos = app.file.textCursor.rectangle.clone();

		var updateCursor = false;
		if (!this.lastCursorPos.equals(app.file.textCursor.rectangle.toArray())) {
			updateCursor = true;
			this.lastCursorPos = app.file.textCursor.rectangle.clone();
		}

		const isHyperlinkChanged = this._isHyperlinkChanged(obj.hyperlink);
		this._map.hyperlinkUnderCursor = obj.hyperlink;
		if (URLPopUpSection.isOpen() && !(obj.hyperlink && obj.hyperlink.link))
			URLPopUpSection.closeURLPopUp();

		if (obj.hyperlink && obj.hyperlink.link &&
			( !URLPopUpSection.isOpen() || updateCursor || isHyperlinkChanged))
			URLPopUpSection.showURLPopUP(obj.hyperlink.link, new cool.SimplePoint(app.file.textCursor.rectangle.x1, app.file.textCursor.rectangle.y1));

		// If modifier view is different than the current view
		// we'll keep the caret position at the same point relative to screen.
		this._onUpdateCursor(
			/* scroll */ updateCursor && weAreModifier,
			/* zoom */ undefined,
			/* keepCaretPositionRelativeToScreen */ !weAreModifier);

		// Only for reference equality comparison.
		this._lastVisibleCursorRef = app.file.textCursor.rectangle.clone();

		// Normally we don't need to refresh the ruler offset.
		// But in multi page view, user may have clicked at the page next to the current one.
		// In that case, we need to fix offset again (if required - it checks values before changing offset).
		const layout = app.activeDocument ? (app.activeDocument.activeLayout ?  app.activeDocument.activeLayout.type : "") : "";
		if (layout === 'ViewLayoutMultiPage' && app.UI.horizontalRuler)
			app.UI.horizontalRuler.fixOffset();
	},

	_isHyperlinkChanged: function(hyperlink)
	{
		// If there is a new hyperlink or existing hyperlink changed or deleted
		if (hyperlink && hyperlink.link)
		{
			if ((this._map.hyperlinkUnderCursor == null || this._map.hyperlinkUnderCursor == undefined) ||
				(this._map.hyperlinkUnderCursor.link != hyperlink.link ||
			     this._map.hyperlinkUnderCursor.text != hyperlink.text))
				return true;
		}
		else if (this._map.hyperlinkUnderCursor != null && this._map.hyperlinkUnderCursor != undefined)
				return true;

		return false;
	},

	_updateEditor: function(textMsg) {
		textMsg = textMsg.substring('editor:'.length + 1);
		var editorId = parseInt(textMsg);
		var docLayer = this._map._docLayer;

		docLayer._editorId = editorId;

		if (app.isFollowingEditor()) {
			app.setFollowingEditor(editorId);
		}

		if (this._map._viewInfo[editorId])
			this._map.fire('updateEditorName', {username: this._map._viewInfo[editorId].username});
	},

	_onInvalidateViewCursorMsg: function (textMsg) {
		var obj = JSON.parse(textMsg.substring('invalidateviewcursor:'.length + 1));
		var viewId = parseInt(obj.viewId);

		// Ignore if viewid is same as ours or not in our db
		if (viewId === this._viewId || !this._map._viewInfo[viewId]) {
			return;
		}

		const username = this._map._viewInfo[viewId].username;
		const mode = obj.mode ? parseInt(obj.mode): 0;

		let rectangle;
		if (obj.refpoint) {
			let refPoint = obj.refpoint.split(',');
			refPoint = new cool.SimplePoint(parseInt(refPoint[0]), parseInt(refPoint[1]));

			if (this.sheetGeometry) {
				this.sheetGeometry.convertToTileTwips(refPoint);

				rectangle = obj.relrect.split(',');
				for (let i = 0; i < rectangle.length; i++) rectangle[i] = parseInt(rectangle[i]);

				rectangle[0] += refPoint.x;
				rectangle[1] += refPoint.y;
			}
		}
		else {
			rectangle = obj.rectangle.split(',');
			for (let i = 0; i < rectangle.length; i++) rectangle[i] = parseInt(rectangle[i]);
		}

		TextCursorSection.addOrUpdateOtherViewCursor(viewId, username, rectangle, parseInt(obj.part), mode);

		if (app.getFollowedViewId() === viewId && (app.isFollowingEditor() || app.isFollowingUser())) {
			if (this.isWriter() || this.isImpress() || this.isDraw()) {
				this.goToViewCursor(viewId);
			}
			else if (this._map.getDocType() === 'spreadsheet') {
				this.goToCellViewCursor(viewId);
			}
		}

		this._saveMessageForReplay(textMsg, viewId);
	},

	_convertRawTwipsToTileTwips: function(strTwips) {
		if (!strTwips)
			return null;

		var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
		var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));
		var bottomRightTwips = topLeftTwips.add(offset);
		strTwips = this._convertToTileTwipsSheetArea(new cool.Bounds(topLeftTwips, bottomRightTwips)).toRectangle();
		return strTwips;
	},

	_onViewLockInfoMsg: function(textMsg) {
		var obj = JSON.parse(textMsg.substring('viewlock:'.length + 1));
		const viewId = parseInt(obj.viewId);

		if (obj.rectangle !== "EMPTY")
			OtherViewGraphicSelectionSection.setViewLockInfo(viewId, obj);
		else
			OtherViewGraphicSelectionSection.setViewLockInfo(viewId, null);
	},

	_onCellViewCursorMsg: function (textMsg) {
		var obj = JSON.parse(textMsg.substring('cellviewcursor:'.length + 1));
		var viewId = parseInt(obj.viewId);

		// Ignore if viewid is same as ours
		if (viewId === this._viewId) {
			return;
		}

		if (obj.rectangle.match('EMPTY'))
			OtherViewCellCursorSection.removeView(viewId);
		else {
			let strTwips = obj.rectangle.match(/\d+/g);
			strTwips = this._convertRawTwipsToTileTwips(strTwips);

			OtherViewCellCursorSection.addOrUpdateOtherViewCellCursor(viewId, this._map.getViewName(viewId), strTwips, parseInt(obj.part));
			CursorHeaderSection.deletePopUpNow(viewId);
		}

		if (this.isCalc()) {
			this._saveMessageForReplay(textMsg, viewId);
		}
	},

	goToCellViewCursor: function(viewId) {
		if (OtherViewCellCursorSection.doesViewCursorExist(viewId)) {
			const viewCursorSection = OtherViewCellCursorSection.getViewCursorSection(viewId);

			if (this._selectedPart !== viewCursorSection.sectionProperties.part)
				this._map.setPart(viewCursorSection.sectionProperties.part);

			if (!viewCursorSection.isVisible) {
				const scrollX = viewCursorSection.position[0];
				const scrollY = viewCursorSection.position[1];
				this.scrollToPos(new cool.SimplePoint(scrollX * app.pixelsToTwips, scrollY * app.pixelsToTwips));
			}

			OtherViewCellCursorSection.showPopUpForView(viewId);
		}
	},

	_onViewCursorVisibleMsg: function(textMsg) {
		textMsg = textMsg.substring('viewcursorvisible:'.length + 1);
		var obj = JSON.parse(textMsg);
		var viewId = parseInt(obj.viewId);

		// Ignore if viewid is same as ours or not in our db
		if (viewId === this._viewId || !this._map._viewInfo[viewId]) {
			return;
		}

		const section = TextCursorSection.getViewCursorSection(viewId);
		if (section) {
			const showCursor = obj.visible === 'true';
			section.sectionProperties.showCursor = showCursor;
			section.setShowSection(section.checkMyVisibility());
			if (!section.showSection)
				CursorHeaderSection.deletePopUpNow(viewId);
		}
	},

	_addView: function(viewInfo) {
		if (viewInfo.color === 0 && this._map.getDocType() !== 'text') {
			viewInfo.color = app.LOUtil.getViewIdColor(viewInfo.id);
		}

		this._map.addView(viewInfo);
	},

	_removeView: function(viewId) {
		// Remove selection, if any.
		app.activeDocument.removeView(viewId);

		TextCursorSection.removeView(viewId);

		OtherViewCellCursorSection.removeView(viewId);
		app.definitions.otherViewGraphicSelectionSection.removeView(viewId);
		this._map.removeView(viewId);
	},

	removeAllViews: function() {
		for (var viewInfoIdx in this._map._viewInfo) {
			this._removeView(parseInt(viewInfoIdx));
		}
	},

	_onViewInfoMsg: function(textMsg) {
		textMsg = textMsg.substring('viewinfo: '.length);
		var viewInfo = JSON.parse(textMsg);
		this._map.fire('viewinfo', viewInfo);

		// A new view
		var viewIds = [];
		for (var viewInfoIdx in viewInfo) {
			if (!(parseInt(viewInfo[viewInfoIdx].id) in this._map._viewInfo)) {
				this._addView(viewInfo[viewInfoIdx]);
			}
			viewIds.push(viewInfo[viewInfoIdx].id);
		}

		// Check if any view is deleted
		for (viewInfoIdx in this._map._viewInfo) {
			if (viewIds.indexOf(parseInt(viewInfoIdx)) === -1) {
				this._removeView(parseInt(viewInfoIdx));
			}
		}

		// Sending postMessage about View_Added / View_Removed is
		// deprecated, going forward we prefer sending the entire information.
		this._map.fire('updateviewslist');
	},

	_onSearchNotFoundMsg: function (textMsg) {
		this._clearSearchResults();
		var originalPhrase = textMsg.substring(16);
		this._map.fire('search', {originalPhrase: originalPhrase, count: 0});
	},

	_getSearchResultRectangles: function (obj, results) {
		for (var i = 0; i < obj.searchResultSelection.length; i++) {
			results.push({
				part: parseInt(obj.searchResultSelection[i].part),
				rectangles: this._twipsRectanglesToPixelBounds(obj.searchResultSelection[i].rectangles),
				twipsRectangles: obj.searchResultSelection[i].rectangles
			});
		}
	},

	_getSearchResultRectanglesFileBasedView: function (obj, results) {
		var additionPerPart = this._partHeightTwips + this._spaceBetweenParts;

		for (var i = 0; i < obj.searchResultSelection.length; i++) {
			var rectangles = obj.searchResultSelection[i].rectangles;
			var part = parseInt(obj.searchResultSelection[i].part);
			rectangles = rectangles.split(',');
			rectangles = rectangles.map(function(element, index) {
				element = parseInt(element);
				if (index < 2)
					element += additionPerPart * part;
				return element;
			});

			rectangles = String(rectangles[0]) + ', ' + String(rectangles[1]) + ', ' + String(rectangles[2]) + ', ' + String(rectangles[3]);

			results.push({
				part: parseInt(obj.searchResultSelection[i].part),
				rectangles: this._twipsRectanglesToPixelBounds(rectangles),
				twipsRectangles: rectangles
			});
		}
	},

	_onSearchResultSelection: function (textMsg) {
		textMsg = textMsg.substring(23);
		var obj = JSON.parse(textMsg);
		var originalPhrase = obj.searchString;
		var count = obj.searchResultSelection.length;
		var highlightAll = obj.highlightAll;
		var results = [];

		if (!app.file.fileBasedView)
			this._getSearchResultRectangles(obj, results);
		else
			this._getSearchResultRectanglesFileBasedView(obj, results);

		// do not cache search results if there is only one result.
		// this way regular searches works fine
		if (count > 1)
		{
			this._clearSearchResults();
			this._searchResults = results;
			if (!app.file.fileBasedView)
				this._map.setPart(results[0].part); // go to first result.
			else
				this._map._docLayer._preview._scrollViewToPartPosition(results[0].part);
		} else if (count === 1) {
			this._lastSearchResult = results[0];
		}
		this._searchTerm = originalPhrase;
		this._map.fire('search', {originalPhrase: originalPhrase, count: count, highlightAll: highlightAll, results: results});

		app.setFollowingUser(this._viewId);

		// always jump to search result - we already received cell / text cursor before so we need
		// to force it in case we had following OFF
		if (app.file.textCursor.visible)
			this._onUpdateCursor(/* scroll */ true);
		else if (app.calc.cellCursorVisible)
			this._onUpdateCellCursor(/* scroll */ true);
	},

	_clearSearchResults: function() {
		if (this._searchTerm)
			app.activeDocument.activeView.clearTextSelection();

		this._lastSearchResult = null;
		this._searchResults = null;
		this._searchTerm = null;
	},

	_onStateChangedMsg: function (textMsg) {
		textMsg = textMsg.substr(14);
		let json = null;

		if (textMsg.trim().startsWith('{') && textMsg.trim().endsWith('}')) {
			try {
				json = JSON.parse(textMsg);
			} catch (e) {
				// Not a valid JSON.
			}
		}

		if (json) {
			// json.state as empty string is fine, for example it means no selection
			// when json.commandName is '.uno:RowColSelCount'.
			if (json.commandName && json.state !== undefined) {
				this._map.fire('commandstatechanged', json);
				if (window.ThisIsTheMacOSApp || window.ThisIsTheQtApp) {
					window.postMobileMessage('COMMANDSTATECHANGED ' + JSON.stringify(json));
				}
			}
		}
		else if (textMsg.startsWith('.uno:Context=') && this.isImpress()) {
			this._selectionContextChanged(textMsg.replace('.uno:Context=', ''));
		}
		else {
			var index = textMsg.indexOf('=');
			var commandName = index !== -1 ? textMsg.substr(0, index) : '';
			var state = index !== -1 ? textMsg.substr(index + 1) : '';
			const json = {commandName : commandName, state : state};
			this._map.fire('commandstatechanged', json);
			if (window.ThisIsTheMacOSApp || window.ThisIsTheQtApp) {
				window.postMobileMessage('COMMANDSTATECHANGED ' + JSON.stringify(json));
			}
		}
	},

	_onUnoCommandResultMsg: function (textMsg) {
		// window.app.console.log('_onUnoCommandResultMsg: "' + textMsg + '"');
		textMsg = textMsg.substring(18);
		var obj = JSON.parse(textMsg);
		var commandName = obj.commandName;
		if (obj.success === 'true' || obj.success === true) {
			var success = true;
		}
		else if (obj.success === 'false' || obj.success === false) {
			success = false;
		}

		this._map.hideBusy();
		this._map.fire('commandresult', {commandName: commandName, success: success, result: obj.result});
		if (window.ThisIsTheMacOSApp || window.ThisIsTheQtApp) {
			window.postMobileMessage('COMMANDRESULT ' + textMsg);
		}

		if (this._map.CallPythonScriptSource != null) {
			this._map.CallPythonScriptSource.postMessage(JSON.stringify({'MessageId': 'CallPythonScript-Result',
										     'SendTime': Date.now(),
										     'Values': obj
										    }),
								     '*');
			this._map.CallPythonScriptSource = null;
		}
	},

	_onRulerUpdate: function (textMsg) {
		var horizontalRuler = true;
		if(textMsg.startsWith('vrulerupdate:')) {
			horizontalRuler = false;
		}
		textMsg = textMsg.substring(13);
		var obj = JSON.parse(textMsg);
		if (!horizontalRuler) {
			this._map.fire('vrulerupdate', obj);
		}
		else {
			this._map.fire('rulerupdate', obj);
		}
	},

	_onContextMenuMsg: function (textMsg) {
		textMsg = textMsg.substring(13);
		var obj = JSON.parse(textMsg);

		this._map.fire('locontextmenu', obj);
	},

	_convertToPointSet(rectangleArray) {
		const result = CPolyUtil.rectanglesToPointSet(rectangleArray,
			function (twipsPoint) {
				var corePxPt = app.map._docLayer._twipsToCorePixels(twipsPoint);
				corePxPt.round();
				return corePxPt;
			});

		return result;
	},

	_getRawRectangles(message) {
		let rawRectangles = message.split('::')[0].split(';');

		let refpoint = new cool.SimplePoint(0, 0);
		if (message.indexOf('::') !== -1) {
			refpoint = message.split('::')[1].split(',');
			refpoint = new cool.SimplePoint(parseInt(refpoint[0]), parseInt(refpoint[1]));
		}

		if (message !== '' && message !== 'EMPTY') {
			rawRectangles = rawRectangles.map((rectangle) => {
				const temp = rectangle.split(',');
				return [parseInt(temp[0]) + refpoint.x, parseInt(temp[1]) + refpoint.y, parseInt(temp[2]), parseInt(temp[3])];
			});
		}
		else rawRectangles = [];

		if (this.isCalc() && this.sheetGeometry) {
			for (let i = 0; i < rawRectangles.length; i++) {
				this.sheetGeometry.convertRawRectangleToTileTwips(rawRectangles[i]);
			}
		}

		return rawRectangles;
	},

	_onTextSelectionMsg: function (textMsg) {
		textMsg = textMsg.replace('textselection:', '').trim();
		const rawRectangles = this._getRawRectangles(textMsg);

		if (rawRectangles.length > 0) {
			TextSelections.activate();

			if (app.file.fileBasedView && this._lastSearchResult) {
				// We rely on that _lastSearchResult has been updated before this function is called.
				const additionPerPart = this._partHeightTwips + this._spaceBetweenParts;

				for (let i = 0; i < rawRectangles.length; i++) {
					rawRectangles[i][1] += additionPerPart * this._lastSearchResult.part;
				}

				this._map._docLayer._preview._scrollViewToPartPosition(this._lastSearchResult.part);
				TileManager.updateFileBasedView();
				setTimeout(function () {app.sectionContainer.requestReDraw();}, 100);
			}

			app.activeDocument.activeView.updateSelectionRawData(app.activeDocument.activeModes[0], this._selectedPart, rawRectangles);

			if (this._map._textInput._cursorHandler)
				this._map._textInput._cursorHandler.setShowSection(false); // User selected text, we remove the carret marker.

			if (window.L.Browser.clipboardApiAvailable) {
				// Just set the selection type, no fetch of the content.
				this._map._clip.setTextSelectionType('text');
			} else {
				// Trigger fetching the selection content, we already need to have
				// it locally by the time 'copy' is executed.
				if (this._selectionContentRequest) {
					clearTimeout(this._selectionContentRequest);
				}
				this._selectionContentRequest = setTimeout(window.L.bind(function () {
					app.socket.sendMessage('gettextselection mimetype=text/html,text/plain;charset=utf-8');}, this), 100);
			}

			if (this._map.contextToolbar)
				this._map.contextToolbar.showContextToolbar();
		}
		else {
			TextSelections.deactivate();
			app.activeDocument.activeView.clearTextSelection();
			this._selectedTextContent = '';
			if (this._map.contextToolbar)
				this._map.contextToolbar.hideContextToolbar();
			if (this._map._clip && this._map._clip._selectionType === 'complex')
				this._map._clip.clearSelection();
		}
	},

	_onTextViewSelectionMsg: function (textMsg) {
		const obj = JSON.parse(textMsg.substring('textviewselection:'.length + 1));
		const viewId = parseInt(obj.viewId);
		const viewMode = (obj.mode !== undefined) ? parseInt(obj.mode) : 0;

		// Ignore if viewid is same as ours or not in our db
		if (viewId === this._viewId || !this._map._viewInfo[viewId])
			return;

		// Get raw rectangles.
		const rawRectangles = this._getRawRectangles(obj.selection.trim());

		app.activeDocument.getView(viewId).updateSelectionRawData(viewMode, parseInt(obj.part), rawRectangles);

		this._saveMessageForReplay(textMsg, viewId);
	},

	_updateReferenceMarks: function() {
		this._clearReferences();

		if (!this._referencesAll)
			return;

		for (var i = 0; i < this._referencesAll.length; i++) {
			// Avoid doubled marks, add only marks for current sheet
			if (!this._references.hasMark(this._referencesAll[i].mark)
				&& this._selectedPart === this._referencesAll[i].part) {
				this._references.addMark(this._referencesAll[i].mark);
			}
		}
	},

	_onReferencesMsg: function (textMsg) {
		textMsg = textMsg.substr(textMsg.indexOf(' ') + 1);
		var marks = JSON.parse(textMsg);
		marks = marks.marks;
		var references = [];
		this._referencesAll = [];

		for (var mark = 0; mark < marks.length; mark++) {
			var strTwips = marks[mark].rectangle.match(/\d+/g);
			var strColor = marks[mark].color;
			var part = parseInt(marks[mark].part);

			if (strTwips != null) {
				var rectangles = [];
				for (var i = 0; i < strTwips.length; i += 4) {
					var topLeftTwips = new cool.Point(parseInt(strTwips[i]), parseInt(strTwips[i + 1]));
					var offset = new cool.Point(parseInt(strTwips[i + 2]), parseInt(strTwips[i + 3]));
					var boundsTwips = this._convertToTileTwipsSheetArea(
						new cool.Bounds(topLeftTwips, topLeftTwips.add(offset)));
					rectangles.push([boundsTwips.getBottomLeft(), boundsTwips.getBottomRight(),
						boundsTwips.getTopLeft(), boundsTwips.getTopRight()]);
				}

				var docLayer = this;
				var pointSet = CPolyUtil.rectanglesToPointSet(rectangles, function (twipsPoint) {
					var corePxPt = docLayer._twipsToCorePixels(twipsPoint);
					corePxPt.round();
					return corePxPt;
				});
				var reference = new CPolygon(pointSet, {
					pointerEvents: 'none',
					fillColor: '#' + strColor,
					fillOpacity: 0.25,
					weight: 2 * app.dpiScale,
					opacity: 0.25});

				references.push({mark: reference, part: part});
			}
		}

		for (i = 0; i < references.length; i++) {
			this._referencesAll.push(references[i]);
		}

		this._updateReferenceMarks();
	},

	_getStringPart: function (string) {
		var code = '';
		var i = 0;
		while (i < string.length) {
			if (string.charCodeAt(i) < 48 || string.charCodeAt(i) > 57) {
				code += string.charAt(i);
			}
			i++;
		}
		return code;
	},

	_getNumberPart: function (string) {
		var number = '';
		var i = 0;
		while (i < string.length) {
			if (string.charCodeAt(i) >= 48 && string.charCodeAt(i) <= 57) {
				number += string.charAt(i);
			}
			i++;
		}
		return parseInt(number);
	},

	_isWholeColumnSelected: function (cellAddress) {
		if (!cellAddress)
			cellAddress = document.querySelector('#addressInput input').value;

		var startEnd = cellAddress.split(':');
		if (startEnd.length === 1)
			return false; // Selection is not a range.

		var rangeStart = this._getNumberPart(startEnd[0]);
		if (rangeStart !== 1)
			return false; // Selection doesn't start at first row.

		var rangeEnd = this._getNumberPart(startEnd[1]);
		if (rangeEnd === 1048576) // Last row's number.
			return true;
		else
			return false;
	},

	_isWholeRowSelected: function (cellAddress) {
		if (!cellAddress)
			cellAddress = document.querySelector('#addressInput input').value;

		var startEnd = cellAddress.split(':');
		if (startEnd.length === 1)
			return false; // Selection is not a range.

		var rangeStart = this._getStringPart(startEnd[0]);
		if (rangeStart !== 'A')
			return false; // Selection doesn't start at first column.

		var rangeEnd = this._getStringPart(startEnd[1]);
		if (rangeEnd === 'XFD') // Last column's code.
			return true;
		else
			return false;
	},

	_updateScrollOnCellSelection: function (oldSelection, newSelection) {
		if (!oldSelection)
			return;

		if (newSelection.equals(oldSelection.toArray()))
			return;

		const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle;
		const directionDownOrRight = (newSelection.pX2 !== oldSelection.pX2) || (newSelection.pY2 !== oldSelection.pY2);

		let needsScroll = false;
		let xVisible;
		let yVisible;

		if (directionDownOrRight) {
			xVisible = app.isXVisibleInTheDisplayedArea(newSelection.x2);
			yVisible = app.isYVisibleInTheDisplayedArea(newSelection.y2);
			needsScroll = !xVisible || !yVisible;
		}
		else {
			xVisible = app.isXVisibleInTheDisplayedArea(newSelection.x1);
			yVisible = app.isYVisibleInTheDisplayedArea(newSelection.y1);
			needsScroll = !xVisible || !yVisible;
		}

		if (needsScroll) {
			const spacingX = Math.abs(app.calc.cellCursorRectangle.pWidth) / 4.0;
			const spacingY = Math.abs(app.calc.cellCursorRectangle.pHeight) / 2.0;
			let scrollX = 0, scrollY = 0;

			if (directionDownOrRight) {
				if (!xVisible)
					scrollX = newSelection.pX2 - viewedRectangle.pX2 + spacingX;

				if (!yVisible)
					scrollY = newSelection.pY2 - viewedRectangle.pY2 + spacingY;
			}
			else {
				if (!xVisible)
					scrollX = newSelection.pX1 - viewedRectangle.pX1 - spacingX;

				if (!yVisible)
					scrollY = newSelection.pY1 - viewedRectangle.pY1 - spacingY;
			}

			if (!this._map.wholeColumnSelected && !this._map.wholeRowSelected) {
				const address = document.querySelector('#addressInput input').value;
				if (!this._isWholeColumnSelected(address) && !this._isWholeRowSelected(address)) {
					app.activeDocument.activeLayout.scroll(scrollX, scrollY);
				}
			}
		}
	},

	_onTextSelectionEndMsg: function (textMsg) {
		var rectangles = this._getTextSelectionRectangles(textMsg);

		if (rectangles.length) {
			var topLeftTwips = rectangles[0].getTopLeft();
			var bottomRightTwips = rectangles[0].getBottomRight();
			TextSelections.setEndRectangle(new cool.SimpleRectangle(topLeftTwips.x, topLeftTwips.y, (bottomRightTwips.x - topLeftTwips.x), (bottomRightTwips.y - topLeftTwips.y)));
		}
		else
			TextSelections.setEndRectangle(null);
	},

	_onTextSelectionStartMsg: function (textMsg) {
		var rectangles = this._getTextSelectionRectangles(textMsg);

		if (rectangles.length) {
			var topLeftTwips = rectangles[0].getTopLeft();
			var bottomRightTwips = rectangles[0].getBottomRight();
			TextSelections.setStartRectangle(new cool.SimpleRectangle(topLeftTwips.x, topLeftTwips.y, (bottomRightTwips.x - topLeftTwips.x), (bottomRightTwips.y - topLeftTwips.y)));
		}
		else
			TextSelections.setStartRectangle(null);
	},

	_refreshRowColumnHeaders: function () {
		if (app.sectionContainer.doesSectionExist(app.CSections.RowHeader.name))
			app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name)._updateCanvas();
		if (app.sectionContainer.doesSectionExist(app.CSections.ColumnHeader.name))
			app.sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name)._updateCanvas();
	},

	_onCellSelectionAreaMsg: function (textMsg) {
		var cellfillMarkerSection = app.sectionContainer.getSectionWithName(app.CSections.CellFillMarker.name);
		var strTwips = textMsg.match(/\d+/g);
		if (strTwips != null) {
			var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
			var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));
			var bottomRightTwips = topLeftTwips.add(offset);
			var boundsTwips = this._convertToTileTwipsSheetArea(new cool.Bounds(topLeftTwips, bottomRightTwips));

			var oldSelection = this._cellSelectionArea ? this._cellSelectionArea.clone(): null;
			const adjustedTwipsWidth = boundsTwips.max.x - boundsTwips.min.x;
			const adjustedTwipsHeight = boundsTwips.max.y - boundsTwips.min.y;
			this._cellSelectionArea = new cool.SimpleRectangle(boundsTwips.min.x, boundsTwips.min.y, adjustedTwipsWidth, adjustedTwipsHeight);

			if (cellfillMarkerSection)
				cellfillMarkerSection.calculatePositionViaCellSelection([this._cellSelectionArea.pX2, this._cellSelectionArea.pY2]);

			this._updateScrollOnCellSelection(oldSelection, this._cellSelectionArea);

			CellSelectionMarkers.update();
		} else {
			this._cellSelectionArea = null;
			if (cellfillMarkerSection)
				cellfillMarkerSection.calculatePositionViaCellSelection(null);
			this._cellSelections = Array(0);
			this._map.wholeColumnSelected = false; // Message related to whole column/row selection should be on the way, we should update the variables now.
			this._map.wholeRowSelected = false;
			if (this._refreshRowColumnHeaders)
				this._refreshRowColumnHeaders();
		}
	},

	_onCellAutoFillAreaMsg: function (textMsg) {
		var strTwips = textMsg.match(/\d+/g);
		if (strTwips != null && this._map.isEditMode()) {
			var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
			var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));

			var topLeftPixels = this._twipsToCorePixels(topLeftTwips);
			var offsetPixels = this._twipsToCorePixels(offset);
			this._cellAutoFillAreaPixels = app.LOUtil.createRectangle(topLeftPixels.x, topLeftPixels.y, offsetPixels.x, offsetPixels.y);
		}
		else {
			this._cellAutoFillAreaPixels = null;
		}
	},

	_onDialogPaintMsg: function(textMsg, img) {
		var command = app.socket.parseServerCmd(textMsg);

		// app.socket.sendMessage('DEBUG _onDialogPaintMsg: hash=' + command.hash + ' img=' + typeof(img) + (typeof(img) == 'string' ? (' (length:' + img.length + ':"' + img.substring(0, 30) + (img.length > 30 ? '...' : '') + '")') : '') + ', cache size ' + this._pngCache.length);
		if (command.nopng) {
			var found = false;
			for (var i = 0; i < this._pngCache.length; i++) {
				if (this._pngCache[i].hash == command.hash) {
					found = true;
					// app.socket.sendMessage('DEBUG - Found in cache');
					img = this._pngCache[i].img;
					// Remove item (and add it below at the start of the array)
					this._pngCache.splice(i, 1);
					break;
				}
			}
			if (!found) {
				var message = 'windowpaint: message assumed PNG for hash ' + command.hash
				    + ' is cached here in the client but not found';
				if (window.L.Browser.cypressTest)
					throw new Error(message);
				app.socket.sendMessage('ERROR ' + message);
				// Not sure what to do. Ask the server to re-send the windowpaint: message but this time including the PNG?
			}
		} else {
			// Sanity check: If we get a PNG it should be for a hash that we don't have cached
			for (i = 0; i < this._pngCache.length; i++) {
				if (this._pngCache[i].hash == command.hash) {
					message = 'windowpaint: message included PNG for hash ' + command.hash
					    + ' even if it was already cached here in the client';
					if (window.L.Browser.cypressTest)
						throw new Error(message);
					app.socket.sendMessage('ERROR ' + message);
					// Remove the extra copy, code below will add it at the start of the array
					this._pngCache.splice(i, 1);
					break;
				}
			}
		}

		// If cache is max size, drop the last element
		if (this._pngCache.length == app.socket.TunnelledDialogImageCacheSize) {
			// app.socket.sendMessage('DEBUG - Dropping last cache element');
			this._pngCache.pop();
		}

		// Add element to cache
		this._pngCache.unshift({hash: command.hash, img:img});

		// app.socket.sendMessage('DEBUG - Cache size now ' + this._pngCache.length);

		this._map.fire('windowpaint', {
			id: command.id,
			img: img,
			width: command.width,
			height: command.height,
			rectangle: command.rectangle,
			hash: command.hash
		});
	},

	_onDialogMsg: function(textMsg) {
		textMsg = textMsg.substring('window: '.length);
		var dialogMsg = JSON.parse(textMsg);
		// e.type refers to signal type
		dialogMsg.winType = dialogMsg.type;
		this._map.fire('window', dialogMsg);
	},

	_mapOnError: function (e) {
		if (e.msg && this._map.isEditMode() && e.critical !== false) {
			this._map.setPermission('view');
		}
	},

	_clearSelections: function (calledFromSetPartHandler) {
		// hide the cursor if not editable
		this._onUpdateCursor(calledFromSetPartHandler);
		// hide the text selection
		app.activeDocument.activeView.clearTextSelection();
		// hide the ole selection
		this._oleCSelections.clear();

		this._onUpdateCellCursor();
		if (this._map._clip)
			this._map._clip.clearSelection();
		else
			this._selectedTextContent = '';
	},

	_clearReferences: function () {
		this._references.clear();
	},

	_resetReferencesMarks: function (type) {
		this._clearReferences();

        if (type === undefined)
		    this._referencesAll = [];
        else if (type === 'focuscell')
            this._referencesAll = this._referencesAll.filter(function(e) { return e.type !== 'focuscell' });

		this._updateReferenceMarks();
	},

	_postMouseEvent: function(type, x, y, count, buttons, modifier) {
		if (!this._map._docLoaded)
			return;

		if (this._map.calcInputBarHasFocus() && type === 'move') {
			// When the Formula-bar has the focus, sending
			// mouse move with the document coordinates
			// hides the cursor (lost focus?). This is clearly
			// a bug in Core, but we need to work around it
			// until fixed. Just don't send mouse move.
			return;
		}

		const verticalOffset = this.getFiledBasedViewVerticalOffset();
		if (verticalOffset) {
			y -= verticalOffset;
		}

		if (this._map.contextToolbar)
			this._map.contextToolbar.setLastInputEventType({input: "mouse", type: type});

		app.socket.sendMessage('mouse type=' + type +
				' x=' + x + ' y=' + y + ' count=' + count +
				' buttons=' + buttons + ' modifier=' + modifier);


		const tempPageLinks = this._map['stateChangeHandler'].getItemValue('PageLinks');
		const thereArePageLinks =  tempPageLinks && tempPageLinks.length > 0;
		if (type === 'buttonup' && thereArePageLinks) {
			URLPopUpSection.closeURLPopUp();
			for (const link of this._map['stateChangeHandler'].getItemValue('PageLinks')) {
				if (link.rectangle.containsPoint([x, y])) {
					URLPopUpSection.showURLPopUP(link.uri, new cool.SimplePoint(x, y + this.getFiledBasedViewVerticalOffset()), undefined, /*linkIsClientSide:*/true);
				}
			}
		}

		if (type === 'move' && thereArePageLinks) {
			let overLink = false;
			for (const link of tempPageLinks) {
				if (link.rectangle.containsPoint([x, y])) {
					overLink = true;
					break;
				}
			}
			const canvas = document.getElementById('document-canvas');
			if (overLink) {
				if (canvas && canvas.style.cursor !== 'pointer')
					canvas.style.cursor = 'pointer';
			} else if (canvas && canvas.style.cursor === 'pointer') {
				canvas.style.cursor = '';
			}
		}

		if (type === 'buttondown')
			this._clearSearchResults();

		if (this._map && this._map._docLayer && (type === 'buttondown' || type === 'buttonup'))
			this._map.userList.followUser(this._map._docLayer._getViewId(), false);
	},

	// If viewing multi-page PDF files, get the twips offset of the current part. This is
	// needed, because core has multiple draw pages in such a case, but we have just one canvas.
	getFiledBasedViewVerticalOffset: function() {
		if (!app.file.fileBasedView) {
			return;
		}

		const additionPerPart = this._partHeightTwips + this._spaceBetweenParts;
		const verticalOffset = additionPerPart * this._selectedPart;

		return verticalOffset;
	},

	// If viewing multi-page PDF files, no precise tracking of invalidations is implemented yet,
	// so this allows requesting new tiles when we know a viewed PDF changes for some special
	// reason.
	requestNewFiledBasedViewTiles: function() {
		if (!app.file.fileBasedView) {
			return;
		}

		this._requestNewTiles();
		TileManager.redraw();
	},

	// Given a character code and a UNO keycode, send a "key" message to coolwsd.
	//
	// "type" is either "input" for key presses (akin to the DOM "keypress"
	// / "beforeinput" events) and "up" for key releases (akin to the DOM
	// "keyup" event).
	//
	// PageUp/PageDown and select column & row are handled as special cases for spreadsheets - in
	// addition of sending messages to coolwsd, they move the cell cursor around.
	postKeyboardEvent: function(type, charCode, unoKeyCode) {
		if (!this._map._docLoaded)
			return;

		if (window.L.Browser.mac) {
			// Map Mac standard shortcuts to the LO shortcuts for the corresponding
			// functions when possible. Note that the Cmd modifier comes here as CTRL.

			// Cmd+UpArrow -> Ctrl+Home
			if (unoKeyCode == UNOKey.UP + app.UNOModifier.CTRL)
				unoKeyCode = UNOKey.HOME + app.UNOModifier.CTRL;
			// Cmd+DownArrow -> Ctrl+End
			else if (unoKeyCode == UNOKey.DOWN + app.UNOModifier.CTRL)
				unoKeyCode = UNOKey.END + app.UNOModifier.CTRL;
			// Cmd+LeftArrow -> Home
			else if (unoKeyCode == UNOKey.LEFT + app.UNOModifier.CTRL)
				unoKeyCode = UNOKey.HOME;
			// Cmd+RightArrow -> End
			else if (unoKeyCode == UNOKey.RIGHT + app.UNOModifier.CTRL)
				unoKeyCode = UNOKey.END;
			// Option+LeftArrow -> Ctrl+LeftArrow
			else if (unoKeyCode == UNOKey.LEFT + app.UNOModifier.ALT)
				unoKeyCode = UNOKey.LEFT + app.UNOModifier.CTRL;
			// Option+RightArrow -> Ctrl+RightArrow (Not entirely equivalent, should go
			// to end of word (or next), LO goes to beginning of next word.)
			else if (unoKeyCode == UNOKey.RIGHT + app.UNOModifier.ALT)
				unoKeyCode = UNOKey.RIGHT + app.UNOModifier.CTRL;
		}

		var completeEvent = app.socket.createCompleteTraceEvent('L.TileSectionManager.postKeyboardEvent', { type: type, charCode: charCode });

		if (this._map.contextToolbar)
			this._map.contextToolbar.setLastInputEventType({input: "key", type: type});
		var winId = this._map.getWinId();
		if (
			this.isCalc() &&
			type === 'input' &&
			winId === 0
		) {
			if (unoKeyCode === UNOKey.SPACE + app.UNOModifier.CTRL) { // Select whole column.
				this._map.wholeColumnSelected = true;
			}
			else if (unoKeyCode === UNOKey.SPACE + app.UNOModifier.SHIFT) { // Select whole row.
				this._map.wholeRowSelected = true;
			}
		}

		if (winId === 0) {
			app.socket.sendMessage(
				'key' +
				' type=' + type +
				' char=' + charCode +
				' key=' + unoKeyCode +
				'\n'
			);
		} else {
			app.socket.sendMessage(
				'windowkey id=' + winId +
				' type=' + type +
				' char=' + charCode +
				' key=' + unoKeyCode +
				'\n'
			);
		}
		if (completeEvent)
			completeEvent.finish();
	},

	_postSelectTextEvent: function(type, x, y) {
		app.socket.sendMessage('selecttext type=' + type +
				' x=' + x + ' y=' + y);
	},

	_onZoomStart: function () {
		this._isZooming = true;
	},


	_onZoomEnd: function () {
		this._isZooming = false;
		app.sectionContainer.setPostZoomReplay(true);
		if (!this.isCalc())
			this._replayPrintTwipsMsgs(false);
		app.sectionContainer.setPostZoomReplay(false);
		this._onUpdateCursor(null, true);
		TextCursorSection.updateVisibilities();
	},

	_updateCursorPos: function () {
		if (!this._cursorMarker) {
			this._cursorMarker = new Cursor(app.file.textCursor.rectangle.clone(), this._map, { blink: true });
		} else {
			this._cursorMarker.setRectangle(app.file.textCursor.rectangle.clone());
		}
	},

	goToTarget: function(target) {
		var command = {
			'Name': {
				type: 'string',
				value: 'URL'
			},
			'URL': {
				type: 'string',
				value: '#' + target
			}
		};

		this._map.sendUnoCommand('.uno:OpenHyperlink', command);
	},

	_allowViewJump: function() {
		return (!this._map._clip || this._map._clip._selectionType !== 'complex');
	},

	// Scrolls the view to selected position
	scrollToPos: function(pos) {
		if (pos instanceof cool.SimplePoint) // Turn into lat/lng if required (pos may also be a simplePoint.).
			pos = this._twipsToLatLng({ x: pos.x, y: pos.y });

		var center = this._map.project(pos);

		let needsXScroll = false;
		let needsYScroll = false;
		const CSSPixelsToTwips = app.dpiScale * app.pixelsToTwips;

		// If x coordinate is already within visible area, we won't scroll to that direction.
		if (app.isXVisibleInTheDisplayedArea(Math.round(center.x * CSSPixelsToTwips)))
			center.x = app.activeDocument.activeLayout.viewedRectangle.cX1;
		else {
			center.x -= this._map.getSize().divideBy(2).x;
			center.x = Math.round(center.x < 0 ? 0 : center.x);
			needsXScroll = true;
		}

		// If y coordinate is already within visible area, we won't scroll to that direction.
		const controlYDown = center.y + (app.file.textCursor.visible ? app.file.textCursor.rectangle.cHeight :
			(app.calc.cellCursorVisible ? app.calc.cellCursorRectangle.cHeight : 0));

		const controlYUp = center.y - (app.file.textCursor.visible ? app.file.textCursor.rectangle.cHeight :
			(app.calc.cellCursorVisible ? app.calc.cellCursorRectangle.cHeight : 0));

		if (app.isYVisibleInTheDisplayedArea(Math.round(controlYDown * CSSPixelsToTwips)) && app.isYVisibleInTheDisplayedArea(Math.round(controlYUp * CSSPixelsToTwips)))
			center.y = app.activeDocument.activeLayout.viewedRectangle.cY1;
		else {
			center.y -= this._map.getSize().divideBy(2).y;
			center.y = Math.round(center.y < 0 ? 0 : center.y);
			needsYScroll = true;
		}

		if (needsXScroll || needsYScroll) {
			const section = app.sectionContainer.getSectionWithName(app.CSections.Scroll.name);
			if (section) {
				section.onScrollTo({x: center.x * app.dpiScale, y: center.y * app.dpiScale});
			}
		}
	},

	// Scroll the view by an amount given by a simplePoint
	scrollByPoint: function(offset) {
		this._map.fire('scrollby', {x: offset.cX, y: offset.cY});
	},

	// Update cursor layer (blinking cursor).
	_onUpdateCursor: function (scroll, zoom, keepCaretPositionRelativeToScreen) {

		if (this._map.ignoreCursorUpdate()) {
			return;
		}

		if (!app.file.textCursor.visible && !GraphicSelection.hasActiveSelection()) {
			this._updateCursorAndOverlay();
			TextCursorSection.updateVisibilities(true);
			return;
		}

		if (!zoom
		&& scroll !== false
		&& (app.file.textCursor.visible || GraphicSelection.hasActiveSelection())
		// Do not center view in Calc if no new cursor coordinates have arrived yet.
		// ie, 'invalidatecursor' has not arrived after 'cursorvisible' yet.
		&& (!this.isCalc() || (this._lastVisibleCursorRef && !this._lastVisibleCursorRef.equals(app.file.textCursor.rectangle.toArray())))
		&& this._allowViewJump()) {

			// Cursor invalidation should take most precedence among all the scrolling to follow the cursor
			// so here we disregard all the pending scrolling
			app.sectionContainer.getSectionWithName(app.CSections.Scroll.name).pendingScrollEvent = null;
			var correctedCursor = app.file.textCursor.rectangle.clone();

			if (this.isWriter()) {
				// For Writer documents, disallow scrolling to cursor outside of the page (horizontally)
				// Use document dimensions to approximate page width
				correctedCursor.x1 = clamp(correctedCursor.x1, 0, app.activeDocument.activeLayout.viewSize.x);
				correctedCursor.x2 = clamp(correctedCursor.x2, 0, app.activeDocument.activeLayout.viewSize.x);
			}

			if (!app.isPointVisibleInTheDisplayedArea(new cool.SimplePoint(correctedCursor.x1, correctedCursor.y1).toArray()) ||
				!app.isPointVisibleInTheDisplayedArea(new cool.SimplePoint(correctedCursor.x2, correctedCursor.y2).toArray())) {
				if (app.isFollowingUser() && app.getFollowedViewId() === this._viewId && !this._map.calcInputBarHasFocus()) {
					this.scrollToPos(new cool.SimplePoint(correctedCursor.x1, correctedCursor.y1));
				}
			}
		}
		else if (keepCaretPositionRelativeToScreen) {
			/* We should be here when:
				Another view updated the text.
				That edit changed our cursor position.
			Now we already set the cursor position to another point.
			We want to keep the cursor position at the same point relative to screen.
			Do that only when we are reaching the end of screen so we don't flicker.
			*/
			var that = this;

			var isCursorVisible = app.isPointVisibleInTheDisplayedArea(app.file.textCursor.rectangle.toArray());

			if (!isCursorVisible) {
				setTimeout(function () {
					var y = app.file.textCursor.rectangle.pY1 - that._cursorPreviousPositionCorePixels.pY1;
					if (y) {
						app.sectionContainer.getSectionWithName(app.CSections.Scroll.name).scrollVerticalWithOffset(y);
					}
				}, 0);
			}
		}

		this._updateCursorAndOverlay();

		TextCursorSection.updateVisibilities();
	},

	activateCursor: function () {
		this._replayPrintTwipsMsg('invalidatecursor');
	},

	// enable or disable blinking cursor and the cursor overlay depending on
	// the state of the document (if the flags are set)
	_updateCursorAndOverlay: function (/*update*/) {
		if (app.file.textCursor.visible   // only when COKit has told us it is ok
			&& this._map.editorHasFocus()   // not when document is not focused
			&& !this._map.isSearching()  	// not when searching within the doc
			&& !this._isZooming             // not when zooming
			&& this._map._permission !== 'readonly' // not when we don't have permission to edit
		) {
			this._updateCursorPos();

			var scrollSection = app.sectionContainer.getSectionWithName(app.CSections.Scroll.name);
			if (!scrollSection.sectionProperties.mouseIsOnVerticalScrollBar && !scrollSection.sectionProperties.mouseIsOnHorizontalScrollBar) {
				this._map._textInput.showCursor();
			}

			var hasMobileWizardOpened = this._map.uiManager.mobileWizard ? this._map.uiManager.mobileWizard.isOpen() : false;
			var hasIframeModalOpened = $('.iframe-dialog-modal').is(':visible');
			// Don't show the keyboard when the Wizard is visible, or when we have just been in a zoom
			if (!window.mobileWizard && !window.pageMobileWizard &&
				!window.insertionMobileWizard && !hasMobileWizardOpened &&
				!JSDialog.IsAnyInputFocused() && !hasIframeModalOpened && !app.sectionContainer.isPostZoomReplay()) {
				// If the user is editing, show the keyboard, but don't change
				// anything if nothing is changed.

				// We will focus map if no comment is being edited (writer only for now).
				if (this.isWriter()) {
					var section = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
					if (!section || !section.sectionProperties.selectedComment || !section.sectionProperties.selectedComment.isEdit())
						this._map.focus(true);
				}
				else
					this._map.focus(true);
			}
		} else {
			this._map._textInput.hideCursor();
			// Maintain input if a dialog or search-box has the focus.
			if (this._map.editorHasFocus() && !this._map.uiManager.isAnyDialogOpen() && !this._map.isSearching()
				&& !JSDialog.IsAnyInputFocused() && (this._map._docLayer._preview && !this._map._docLayer._preview.partsFocused))
				this._map.focus(false);
		}

		if (app.map._textInput && app.activeDocument)
			app.map._textInput.update();

		// when first time we updated the cursor - document is loaded
		// let's move cursor to the target
		if (this._map.options.docTarget !== '') {
			this.goToTarget(this._map.options.docTarget);
			this._map.options.docTarget = '';
		}
	},

	// Jump to view of user with given *viewId*
	goToOtherUserView: function(viewId) {
		const graphicSection = OtherViewGraphicSelectionSection.getViewSection(viewId);

		if (graphicSection) {
			if (this._selectedPart !== graphicSection.sectionProperties.part) {
				this._map.deselectAll();
				this._map.setPart(graphicSection.sectionProperties.part);
			}
			graphicSection.goToSection();
		}
	},

	goToViewCursor: function(viewId) {
		if (viewId === this._viewId) {
			this._onUpdateCursor();
			return;
		}

		const section = TextCursorSection.getViewCursorSection(viewId);

		if (section) {
			if ((this.isImpress() || this.isDraw()) && this._selectedPart !== section.sectionProperties.part) {
				this._map.deselectAll();
				this._map.setPart(section.sectionProperties.part);
			}

			if (section.showSection) {
				section.goToSection();
				app.definitions.cursorHeaderSection.showCursorHeader(viewId);
			}
		}
	},

	eachView: function (views, method, context, item) {
		for (var key in views) {
			method.call(context, item ? views[key] : key);
		}
	},

	// TODO: used only in calc: move to CalcTileLayer
	_onUpdateCellCursor: function (scrollToCursor, sameAddress) {
		CellSelectionMarkers.update();

		if (app.calc.cellCursorVisible) {
			if (scrollToCursor &&
			    !this._map.calcInputBarHasFocus()) {
				const scroll = this._calculateScrollForNewCellCursor();
				if (scroll.x !== 0 || scroll.y !== 0) {
					scroll.x += app.activeDocument.activeLayout.viewedRectangle.x1;
					scroll.y += app.activeDocument.activeLayout.viewedRectangle.y1;
					app.activeDocument.activeLayout.scrollTo(scroll.pX, scroll.pY);
				}

				this._prevCellCursorAddress = app.calc.cellAddress.clone();
			}

			this._addCellDropDownArrow();

			var focusOutOfDocument = document.activeElement === document.body;
			var dontFocusDocument = JSDialog.IsAnyInputFocused() || focusOutOfDocument;
			var dontStealFocus = sameAddress && this._map.calcInputBarHasFocus();
			dontFocusDocument = dontFocusDocument || dontStealFocus;

			// when the cell cursor is moving, the user is in the document,
			// and the focus should leave the cell input bar
			// exception: when dialog opened don't focus the document
			if (!dontFocusDocument)
				this._map.fire('editorgotfocus');
		}

		this._removeCellDropDownArrow();
		URLPopUpSection.closeURLPopUp();
	},

	_onValidityListButtonMsg: function(textMsg) {
		var strXY = textMsg.match(/\d+/g);
		var validatedCellAddress = new cool.SimplePoint(parseInt(strXY[0]), parseInt(strXY[1])); // Cell address of the validity list.
		var show = parseInt(strXY[2]) === 1;
		if (show) {
			if (this._validatedCellAddress && !validatedCellAddress.equals(this._validatedCellAddress.toArray())) {
				this._validatedCellAddress = null;
				this._removeCellDropDownArrow();
			}
			this._validatedCellAddress = validatedCellAddress;
			this._addCellDropDownArrow();
		}
		else if (this._validatedCellAddress && validatedCellAddress.equals(this._validatedCellAddress.toArray())) {
			this._validatedCellAddress = null;
			this._removeCellDropDownArrow();
		}
	},

	_onValidityInputHelpMsg: function(textMsg) {
		app.definitions.validityInputHelpSection.removeValidityInputHelp();
		app.definitions.validityInputHelpSection.showValidityInputHelp(textMsg, new cool.SimplePoint(app.calc.cellCursorRectangle.x2, app.calc.cellCursorRectangle.y1));
	},

	_addCellDropDownArrow: function () {
		if (this._validatedCellAddress && app.calc.cellCursorVisible && this._validatedCellAddress.equals(app.calc.cellAddress.toArray())) {
			let position;
			if (this.sheetGeometry) {
				position = this.sheetGeometry.getCellRect(this._validatedCellAddress.x, this._validatedCellAddress.y);
				position = new cool.SimplePoint(app.calc.cellCursorRectangle.x2, (position.max.y - CalcValidityDropDown.dropDownArrowSize * app.dpiScale) * app.pixelsToTwips);
			}
			else
				position = new cool.SimplePoint(app.calc.cellCursorRectangle.x2, app.calc.cellCursorRectangle.y2 - CalcValidityDropDown.dropDownArrowSize * app.dpiScale * app.pixelsToTwips);

			if (!app.sectionContainer.getSectionWithName(app.CSections.CalcValidityDropDown.name)) {
				let dropDownSection = new CalcValidityDropDown(position);
				app.sectionContainer.addSection(dropDownSection);
			}
			else {
				app.sectionContainer.getSectionWithName(app.CSections.CalcValidityDropDown.name).setPosition(position.pX, position.pY);
			}
		}
	},

	_removeCellDropDownArrow: function () {
		if (!this._validatedCellAddress)
			app.sectionContainer.removeSection(app.CSections.CalcValidityDropDown.name);
	},

	_removeSelection: function() {
		this._selectedTextContent = '';
		app.activeDocument.activeView.clearTextSelection();
	},

	_onDragOver: function (e) {
		e = e.originalEvent;
		e.preventDefault();
	},

	_onDrop: function (e) {
		// Move the cursor, so that the insert position is as close to the drop coordinates as possible.
		var latlng = e.latlng;
		var docLayer = this._map._docLayer;
		var mousePos = docLayer._latLngToTwips(latlng);
		var count = 1;
		var buttons = 1;
		var modifier = this._map.keyboard.modifier;
		this._postMouseEvent('buttondown', mousePos.x, mousePos.y, count, buttons, modifier);
		this._postMouseEvent('buttonup', mousePos.x, mousePos.y, count, buttons, modifier);

		e = e.originalEvent;
		e.preventDefault();

		if (this._map._clip) {
			// Always capture the html content separate as we may lose it when we
			// pass the clipboard data to a different context (async calls, f.e.).
			var htmlText = e.dataTransfer.getData('text/html');
			this._map._clip.dataTransferToDocument(e.dataTransfer, /* preferInternal = */ false, htmlText);
		}
	},

	recalculateZoomOnResize: function() {
		if (this.isWriter())
			this._invalidateZoomFirstFit = true;
	},

	// This is really just called on zoomend
	_fitWidthZoom: function (e, maxZoom, recalcFirstFit=false) {
		if (this.isCalc() || this.isDraw())
			return;

		if (this._map.uiManager.getStartCompareChanges()) {
			// comparechanges view, don't zoom in, to have space for two pages side by
			// side.
			return;
		}

		if (!maxZoom) {
			if (this.isImpress()) maxZoom = 10;
			else if (this.isWriter()) maxZoom = 13;
		}

		if (this._invalidateZoomFirstFit) {
			recalcFirstFit = true;
			this._invalidateZoomFirstFit = false;
		}

		if (app.activeDocument.fileSize.x === 0) { return; }
		var oldSize = e && e.oldSize ? e.oldSize : this._map.getSize();
		var newSize = e && e.newSize ? e.newSize : this._map.getSize();

		newSize.x *= app.dpiScale;
		newSize.y *= app.dpiScale;
		oldSize.x *= app.dpiScale;
		oldSize.y *= app.dpiScale;

		let bringCommentsIntoView = false;
		if (this.isWriter() && app.activeDocument.partHasComments && (recalcFirstFit || !this._includedCommentsInFirstFit)) {
			bringCommentsIntoView = true;
			this._includedCommentsInFirstFit = true;
			this._firstFitDone = false;
		}

		// `recalcFirstFit` is used to recalculate/reset the zoom levels to the
		// maximum possible zoom level based on the window (canvas) size.
		if (recalcFirstFit)
			this._firstFitDone = false;

		// if we are here then that means we have the document size
		// therefore we should continue and do the firstFit zoom resize,
		// or else it keeps waiting for a resize event.
		if (this._firstFitDone && newSize.x - oldSize.x === 0)
			return;

		const commentWidth = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).sectionProperties.commentWidth;
		let documentWidth = app.activeDocument.fileSize.pX;
		if (bringCommentsIntoView) documentWidth += commentWidth;

		var ratio = newSize.x / documentWidth;
		var zoom = this._map.getScaleZoom(ratio);

		if (maxZoom)
			zoom = Math.min(maxZoom, Math.max(0.1, zoom));

		// Not clear why we wanted to zoom in the past.
		// This resets the view & scroll area and does a 'panTo'
		// to keep the cursor in view.
		// But of course, zoom to fit the first time.
		if (this._firstFitDone)
			zoom = this._map._zoom;
		this._firstFitDone = true;

		if (zoom > 1)
			zoom = Math.floor(zoom);

		this._map.setZoom(zoom, {animate: false});
	},

	// Cells can change position during changes of zoom level in calc
	// hence we need to request an updated cell cursor position for this level.
	_onCellCursorShift: function (force) {
		if ((this._cellCursorSection && !this.options.sheetGeometryDataEnabled) || force) {
			this.requestCellCursor();
		}
	},

	requestCellCursor: function() {
		app.socket.sendMessage('commandvalues command=.uno:CellCursor'
			+ '?outputHeight=' + TileManager.tileSize
			+ '&outputWidth=' + TileManager.tileSize
			+ '&tileHeight=' + app.tile.size.x
			+ '&tileWidth=' + app.tile.size.y);
	},

	_invalidateAllPreviews: function () {
		this._previewInvalidations = [];
		for (var key in this._map._docPreviews) {
			var preview = this._map._docPreviews[key];
			preview.invalid = true;
			this._previewInvalidations.push(new cool.Bounds(new cool.Point(0, 0), new cool.Point(preview.maxWidth, preview.maxHeight)));
		}
		this._invalidatePreviews();
	},

	_invalidatePreviews: function () {
		if (this._map && this._map._docPreviews && this._previewInvalidations.length > 0) {
			var toInvalidate = {};
			for (var i = 0; i < this._previewInvalidations.length; i++) {
				var invalidBounds = this._previewInvalidations[i];
				for (var key in this._map._docPreviews) {
					// find preview tiles that need to be updated and add them in a set
					var preview = this._map._docPreviews[key];
					if (preview.index >= 0) {
						// we have a preview for a part
						if (preview.invalid || preview.index === this._selectedPart ||
								(preview.index === this._prevSelectedPart && this._prevSelectedPartNeedsUpdate)) {
							// if the current part needs its preview updated OR
							// the part has been changed and we need to update the previous part preview
							if (preview.index === this._prevSelectedPart) {
								this._prevSelectedPartNeedsUpdate = false;
							}
							toInvalidate[key] = true;
						}
					}
					else {
						// we have a custom preview
						var bounds = new cool.Bounds(
							new cool.Point(preview.tilePosX, preview.tilePosY),
							new cool.Point(preview.tilePosX + preview.tileWidth, preview.tilePosY + preview.tileHeight));
						if (preview.invalid || (preview.part === this._selectedPart ||
								(preview.part === this._prevSelectedPart && this._prevSelectedPartNeedsUpdate)) &&
								invalidBounds.intersects(bounds)) {
							// if the current part needs its preview updated OR
							// the part has been changed and we need to update the previous part preview
							if (preview.index === this._prevSelectedPart) {
								this._prevSelectedPartNeedsUpdate = false;
							}
							toInvalidate[key] = true;
						}

					}
				}

			}

			for (key in toInvalidate) {
				// update invalid preview tiles
				preview = this._map._docPreviews[key];
				if (preview.autoUpdate) {
					if (preview.index >= 0) {
						this._map.getPreview(preview.id, preview.index, preview.maxWidth, preview.maxHeight, {autoUpdate: true});
					}
					else {
						this._map.getCustomPreview(preview.id, preview.part, preview.width, preview.height, preview.tilePosX,
							preview.tilePosY, preview.tileWidth, preview.tileHeight, {autoUpdate: true});
					}
				}
			}
		}
		this._previewInvalidations = [];
	},

	_onFormFieldButtonMsg: function (textMsg) {
		textMsg = textMsg.substring('formfieldbutton:'.length + 1);
		var json = JSON.parse(textMsg);
		if (json.action === 'show') {
			if (this._formFieldButton)
				app.sectionContainer.removeSection(this._formFieldButton.name);

			this._formFieldButton = new FormFieldButton(json);
			app.sectionContainer.addSection(this._formFieldButton);
		} else if (this._formFieldButton)
			app.sectionContainer.removeSection(this._formFieldButton.name);
	},

	// converts rectangle in print-twips to tile-twips rectangle of the smallest cell-range that encloses it.
	_convertToTileTwipsSheetArea: function (rectangle) {
		if (!(rectangle instanceof cool.Bounds) || !this.options.printTwipsMsgsEnabled || !this.sheetGeometry) {
			return rectangle;
		}

		return this.sheetGeometry.getTileTwipsSheetAreaFromPrint(rectangle);
	},

	_convertCalcTileTwips: function (point, offset) {
		if (!this.options.printTwipsMsgsEnabled || !this.sheetGeometry)
			return point;
		var newPoint = new cool.Point(parseInt(point.x), parseInt(point.y));
		var _offset = offset ? new cool.Point(parseInt(offset.x), parseInt(offset.y)) : new cool.Point(this._shapeGridOffset.x, this._shapeGridOffset.y);
		return newPoint.add(_offset);
	},

	_getEditCursorRectangle: function (msgObj) {

		if (typeof msgObj !== 'object' || !Object.prototype.hasOwnProperty.call(msgObj,'rectangle')) {
			window.app.console.error('invalid edit cursor message');
			return undefined;
		}

		return cool.Bounds.parse(msgObj.rectangle);
	},

	_getTextSelectionRectangles: function (textMsg) {

		if (typeof textMsg !== 'string') {
			window.app.console.error('invalid text selection message');
			return [];
		}

		return cool.Bounds.parseArray(textMsg);
	},

	// Needed for the split-panes feature to determine the active split-pane.
	// Needs to be implemented by the app specific TileLayer.
	getCursorPos: function () {
		window.app.console.error('No implementations available for getCursorPos!');
		return new cool.Point(0, 0);
	},

	/// onlyThread - takes annotation indicating which thread will be generated
	getCommentWizardStructure: function(menuStructure, onlyThread) {
		var customTitleBar = window.L.DomUtil.create('div');
		window.L.DomUtil.addClass(customTitleBar, 'mobile-wizard-titlebar-btn-container');
		var title = window.L.DomUtil.create('span', '', customTitleBar);
		title.innerText = _('Comment');
		var button = window.L.DomUtil.createWithId('button', 'insert_comment', customTitleBar);
		window.L.DomUtil.addClass(button, 'mobile-wizard-titlebar-btn');
		button.innerText = '+';
		button.onclick = this._map.insertComment.bind(this._map);

		if (menuStructure === undefined) {
			menuStructure = {
				id : 'comment',
				type : 'mainmenu',
				enabled : true,
				text : _('Comment'),
				executionType : 'menu',
				data : [],
				children : []
			};

			if (app.isCommentEditingAllowed())
				menuStructure['customTitle'] = customTitleBar;
		}

		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).createCommentStructure(menuStructure, onlyThread);

		if (menuStructure.children.length === 0) {
			var noComments = {
				id: 'emptyWizard',
				enable: true,
				type: 'emptyCommentWizard',
				text: _('No Comments'),
				children: []
			};
			menuStructure['children'].push(noComments);
		}
		return menuStructure;
	},

	_openCommentWizard: function(annotation) {
		window.commentWizard = true;
		var menuData = this._map._docLayer.getCommentWizardStructure();
		this._map.fire('mobilewizard', {data: menuData});

		// if annotation is provided we can select particular comment
		if (annotation) {
			$('#comment' + annotation.sectionProperties.data.id).click();
		}
	},

	_saveMessageForReplay: function (textMsg, viewId) {
		// We will not get some messages (with coordinates)
		// from core when zoom changes because print-twips coordinates are zoom-invariant. So we need to
		// remember the last version of them and replay, when zoom is changed.
		// In calc we need to replay the messages when sheet-geometry changes too. This is because it is possible for
		// the updated print-twips messages to arrive before the sheet-geometry update message arrives.

		if (!this._printTwipsMessagesForReplay) {
			var ownViewTypes = this.isCalc() ? [
				'cellcursor',
				'referencemarks',
				'cellselectionarea',
				'textselection',
				'invalidatecursor',
				'textselectionstart',
				'textselectionend',
				'graphicselection',
			] : [
				'invalidatecursor',
				'textselection',
				'graphicselection'
			];

			if (this.isWriter())
				ownViewTypes.push('contentcontrol');

			var otherViewTypes = this.isCalc() ? [
				'cellviewcursor',
				'textviewselection',
				'invalidateviewcursor',
				'graphicviewselection',
			] : [
				'textviewselection',
				'invalidateviewcursor',
				'graphicviewselection'
			];

			this._printTwipsMessagesForReplay = new window.L.MessageStore(ownViewTypes, otherViewTypes);
		}

		var colonIndex = textMsg.indexOf(':');
		if (colonIndex === -1) {
			return;
		}

		var msgType = textMsg.substring(0, colonIndex);
		this._printTwipsMessagesForReplay.save(msgType, textMsg, viewId);
	},

	_clearMsgReplayStore: function (notOtherMsg) {
		if (!this._printTwipsMessagesForReplay) {
			return;
		}

		this._printTwipsMessagesForReplay.clear(notOtherMsg);
	},

	_replayPrintTwipsMsgs: function (differentSheet) {
		if (!this._printTwipsMessagesForReplay) {
			return;
		}

		this._printTwipsMessagesForReplay.forEach(function (msg) {
			// don't try and replace graphic selection if the sheet/page has changed
			var skipMessage = differentSheet && msg.startsWith('graphicselection:');
			if (!skipMessage)
				this._onMessage(msg);
		}.bind(this));
	},

	_replayPrintTwipsMsg: function (msgType) {
		var msg = this._printTwipsMessagesForReplay.get(msgType);
		this._onMessage(msg);
	},

	_replayPrintTwipsMsgAllViews: function (msgType) {
		Object.keys(this._map._viewInfo).forEach(function (viewId) {
			var msg = this._printTwipsMessagesForReplay.get(msgType, parseInt(viewId));
			if (msg)
				this._onMessage(msg);
		}.bind(this));
	},

	_syncTilePanePos: function () {
		if (this._container) {
			var mapPanePos = this._map._getMapPanePos();
			window.L.DomUtil.setPosition(this._container, new cool.Point(-mapPanePos.x , -mapPanePos.y));
		}
		var documentBounds = this._map.getPixelBoundsCore();
		var documentPos = documentBounds.min;
		var documentEndPos = documentBounds.max;

		const size = [documentEndPos.x - documentPos.x, documentEndPos.y - documentPos.y];

		app.activeDocument.activeLayout.viewedRectangle = new cool.SimpleRectangle(
			documentPos.x * app.pixelsToTwips, documentPos.y * app.pixelsToTwips, size[0] * app.pixelsToTwips, size[1] * app.pixelsToTwips
		);
	},

	pauseDrawing: function () {
		if (this._painter && app.sectionContainer)
			app.sectionContainer.pauseDrawing();
	},

	resumeDrawing: function (topLevel) {
		if (this._painter && app.sectionContainer)
			app.sectionContainer.resumeDrawing(topLevel);
	},

	// used in Calc, see CalcTileLayer
	allowDrawing: function() {},

	enableDrawing: function () {
		if (this._painter && app.sectionContainer)
			app.sectionContainer.enableDrawing();
	},

	_getUIWidth: function () {
		var section = app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name);
		if (section) {
			return Math.round(section.size[0] / app.dpiScale);
		}
		else {
			return 0;
		}
	},

	_getUIHeight: function () {
		var section = app.sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name);
		if (section) {
			return Math.round(section.size[1] / app.dpiScale);
		}
		else {
			return 0;
		}
	},

	_getGroupWidth: function () {
		var section = app.sectionContainer.getSectionWithName(app.CSections.RowGroup.name);
		if (section) {
			return Math.round(section.size[0] / app.dpiScale);
		}
		else {
			return 0;
		}
	},

	_getGroupHeight: function () {
		var section = app.sectionContainer.getSectionWithName(app.CSections.ColumnGroup.name);
		if (section) {
			return Math.round(section.size[1] / app.dpiScale);
		}
		else {
			return 0;
		}
	},

	_getTilesSectionRectangle: function () {
		var section = app.sectionContainer.getSectionWithName(app.CSections.Tiles.name);
		if (section) {
			return app.LOUtil.createRectangle(section.myTopLeft[0] / app.dpiScale, section.myTopLeft[1] / app.dpiScale, section.size[0] / app.dpiScale, section.size[1] / app.dpiScale);
		}
		else {
			return app.LOUtil.createRectangle(0, 0, 0, 0);
		}
	},

	_getRealMapSize: function() {
		this._map._sizeChanged = true; // force using real size
		return this._map.getPixelBounds().getSize();
	},

	_getDocumentContainerSize: function() {
		let documentContainerSize = document.getElementById('document-container').getBoundingClientRect();
		documentContainerSize = [documentContainerSize.width, documentContainerSize.height];
		return documentContainerSize;
	},

	_resizeMapElementAndTilesLayer: function(sizeRectangle) {
		const mapElement = document.getElementById('map'); // map's size = tiles section's size.
		mapElement.style.left = sizeRectangle.getPxX1() + 'px';
		mapElement.style.top = sizeRectangle.getPxY1() + 'px';
		mapElement.style.width = sizeRectangle.getPxWidth() + 'px';
		mapElement.style.height = sizeRectangle.getPxHeight() + 'px';

		this._container.style.width = sizeRectangle.getPxWidth() + 'px';
		this._container.style.height = sizeRectangle.getPxHeight() + 'px';
	},

	_mobileChecksAfterResizeEvent: function(heightIncreased) {
		if (!window.mode.isSmallScreenDevice()) return;

		const hasMobileWizardOpened = this._map.uiManager.mobileWizard ? this._map.uiManager.mobileWizard.isOpen() : false;
		const hasIframeModalOpened = $('.iframe-dialog-modal').is(':visible');
		// when integrator has opened dialog in parent frame (eg. save as) we shouldn't steal the focus
		const focusedUI = document.activeElement === document.body;
		if (!hasMobileWizardOpened && !hasIframeModalOpened && !focusedUI) {
			if (heightIncreased) {
				// if the keyboard is hidden - be sure we setup correct state in TextInput
				this._map.setAcceptInput(false);
			} else
				this._onUpdateCursor(true);
		}
	},

	_nonDesktopChecksAfterResizeEvent: function(heightIncreased) {
		// We want to keep cursor visible when we show the keyboard on mobile device or tablet
		if (!window.mode.isSmallScreenDevice() && !window.mode.isTablet()) return;

		const hasVisibleCursor = app.file.textCursor.visible
			&& this._map._docLayer._cursorMarker && this._map._docLayer._cursorMarker.isDomAttached();
		if (!heightIncreased && this._map._docLoaded && hasVisibleCursor) {
			const cursorPos = this._map._docLayer._twipsToLatLng({ x: app.file.textCursor.rectangle.x1, y: app.file.textCursor.rectangle.y2 });
			const cursorPositionInView = this._isLatLngInView(cursorPos);
			if (!cursorPositionInView)
				this._map.panTo(cursorPos);
		}
	},

	_syncTileContainerSize: function () {
		if (!this._map) return;

		if (this.isImpress() || this.isDraw()) this.onResizeImpress();

		if (!this._container) return;

		const documentContainerSize = this._getDocumentContainerSize();

		app.sectionContainer.onResize(documentContainerSize[0], documentContainerSize[1]); // Canvas's size = documentContainer's size.

		const oldSize = this._getRealMapSize();

		this._resizeMapElementAndTilesLayer(this._getTilesSectionRectangle());

		const newSize = this._getRealMapSize();
		const heightIncreased = oldSize.y < newSize.y;
		const widthIncreased = oldSize.x < newSize.x;

		if (oldSize.x !== newSize.x || oldSize.y !== newSize.y)
			this._map.invalidateSize(false, oldSize);

		this._mobileChecksAfterResizeEvent(heightIncreased);

		this._fitWidthZoom();

		// Center the view w.r.t the new map-pane position using the current zoom.
		this._map.setView(this._map.getCenter());

		this._nonDesktopChecksAfterResizeEvent(heightIncreased);

		if (heightIncreased || widthIncreased) {
			app.sectionContainer.requestReDraw();
			this._map.fire('sizeincreased');
		}
	},

	hasSplitPanesSupport: function () {
		// Only enabled for Calc for now
		// It may work without this.options.sheetGeometryDataEnabled but not tested.
		// The overlay-pane with split-panes is still based on svg renderer,
		// and not available for VML or canvas yet.
		if (this.isCalc() &&
			this.options.sheetGeometryDataEnabled) {
			return true;
		}

		return false;
	},

	setZoomChanged: function (zoomChanged) {
		app.sectionContainer.setZoomChanged(zoomChanged);
	},

	onAdd: function (map) {
		this._initContainer();

		/*
			Because of special handling of delete and backspace chars, we need to know which Writer form is focused.
			When sending removeTextContext event to core side, we send the formID instead of the map id.
		*/
		this._formID = null;

		// Initiate selection handles.
		TextSelections.initiate();

		// Initiate cell selection handles.
		CellSelectionMarkers.initiate();

		if (this.isCalc()) {
			var cursorStyle = new CStyleData(this._cursorDataDiv);
			var weight = cursorStyle.getFloatPropWithoutUnit('border-top-width') * app.dpiScale;
			var color = cursorStyle.getPropValue('border-top-color');
			this._cellCursorSection = new CellCursorSection(color, weight);
			app.sectionContainer.addSection(this._cellCursorSection);
		}

		this._getToolbarCommandsValues();
		this._oleCSelections = new CSelections(undefined, this._canvasOverlay,
			this._selectionsDataDiv, this._map, false /* isView */, undefined, 'ole');
		this._references = new CReferences(this._canvasOverlay);

		/*
		 * `recalculateZoomOnResize` sets this flag to `true`. Then in `_fitWidthZoom`
		 * we set the `recalcFirstFit` to `true` & set this flag to `false`. It helps
		 * deal with delayed resizes as the 'resize' events are fired separately from the
		 * UI code which triggers them.
		 */
		this._invalidateZoomFirstFit = false;
		this._includedCommentsInFirstFit = false;

		this._referencesAll = [];

		this._debug = map._debug;

		app.socket.sendMessage('commandvalues command=.uno:AcceptTrackedChanges');

		map._fadeAnimated = false;
		this._viewReset();

		map.on('dragover', this._onDragOver, this);
		map.on('drop', this._onDrop, this);

		map.on('zoomstart', this._onZoomStart, this);
		map.on('zoomend', this._onZoomEnd, this);
		if (this._docType === 'spreadsheet') {
			map.on('zoomend', this._onCellCursorShift, this);
		}
		map.on('error', this._mapOnError, this);
		if (map.options.autoFitWidth !== false) {
			// always true since autoFitWidth is never set
			map.on('resize', this._fitWidthZoom, this);
		}
		this._map.on('resize', this._syncTileContainerSize, this);
		// Retrieve the initial cell cursor position (as COKit only sends us an
		// updated cell cursor when the selected cell is changed and not the initial
		// cell).
		map.on('statusindicator',
			function (e) {
				if (e.statusType === 'alltilesloaded' && this._docType === 'spreadsheet') {
					if (!this._map.uiManager.isAnyDialogOpen())
						this._onCellCursorShift(true);
				}
			},
			this);

		app.events.on('updatepermission', function(e) {
			if (e.detail.perm !== 'edit') {
				this._clearSelections();
			}
			TileManager.update();
		}.bind(this));

		map.setPermission(app.file.permission);

		map.fire('statusindicator', {statusType: 'coolloaded'});

		this._map.sendInitUNOCommands();

		this._resetClientVisArea();
		this._requestNewTiles();

		map.setZoom();

		// This is called when page size is increased
		// the content of the page that become visible may stay empty
		// unless we have the tiles in the cache already
		// This will only fetch the tiles which are invalid or does not exist
		map.on('sizeincreased', function() {
			TileManager.update();
		}.bind(this));
	},

	onRemove: function (map) {
		window.L.DomUtil.remove(this._container);
		map._removeZoomLimit(this);
		this._container = null;
		this._tileZoom = null;
		TileManager.clearPreFetch();
		clearTimeout(this._previewInvalidator);

		app.activeDocument.activeView.clearTextSelection();

		if (!this._oleCSelections.empty()) {
			this._oleCSelections.clear();
		}

		if (this._cursorMarker && this._cursorMarker.isDomAttached()) {
			this._cursorMarker.remove();
		}

		TextSelections.dispose();

		this._removeSplitters();
		window.L.DomUtil.remove(this._canvasContainer);
	},

	getEvents: function () {
		var events = {
			viewreset: this._viewReset,
			movestart: this._moveStart,
			// update tiles on move, but not more often than once per given interval
			move: app.util.throttle(this._move, this.options.updateInterval, this),
			moveend: this._moveEnd,
			splitposchanged: this._move,
		};

		return events;
	},

	// zoom is the new intermediate zoom level (log scale : 1 to 14)
	zoomStep: function (zoom, newCenter) {
		this._painter.zoomStep(zoom, newCenter);
	},

	zoomStepEnd: function (zoom, newCenter, mapUpdater, runAtFinish, noGap) {
		this._painter.zoomStepEnd(zoom, newCenter, mapUpdater, runAtFinish, noGap);
	},

	preZoomAnimation: function (pinchStartCenter) {
		this._pinchStartCenter = this._map.project(pinchStartCenter).multiplyBy(app.dpiScale); // in core pixels
		this._painter._offset = new cool.Point(0, 0);

		if (this._cursorMarker && app.file.textCursor.visible) {
			this._cursorMarker.setOpacity(0);
		}
		if (this._map._textInput._cursorHandler)
			this._map._textInput._cursorHandler.setOpacity(0);

		if (this.isCalc()) {
			this._cellCursorSection.setShowSection(false);
		}

		TextSelections.hideHandles();

		TextCursorSection.updateVisibilities(true);
	},

	postZoomAnimation: function () {
		if (app.file.textCursor.visible) {
			this._cursorMarker.setOpacity(1);
		}

		if (this._map._textInput._cursorHandler)
			this._map._textInput._cursorHandler.setOpacity(1);

		if (this.isCalc()) {
			this._cellCursorSection.setShowSection(true);
		}

		TextSelections.showHandles();

		if (this._annotations) {
			var annotations = this._annotations;
			if (annotations.update)
				setTimeout(function() {
					annotations.update();
				}, 250 /* ms */);
		}
	},

	// Meant for desktop case, where the ending zoom and centers are all known in advance.
	runZoomAnimation: function (zoomEnd, pinchCenter, mapUpdater, runAtFinish) {

		if (this._map.getDocType() === 'spreadsheet')
			OtherViewCellCursorSection.closePopups();

		this.preZoomAnimation(pinchCenter);
		this.zoomStep(this._map.getZoom(), pinchCenter);
		var thisObj = this;
		this.zoomStepEnd(zoomEnd, pinchCenter,
			mapUpdater,
			// runAtFinish
			function () {
				thisObj.postZoomAnimation();
				runAtFinish();
			});
	},

	_viewReset: function (e) {
		this._reset(e && e.hard);
		if (this._docType === 'spreadsheet' && this._annotations !== undefined) {
			app.socket.sendMessage('commandvalues command=.uno:ViewAnnotationsPosition');
		}
	},

	_removeSplitters: function () {
		if (this._xSplitter) {
			this._canvasOverlay.removePath(this._xSplitter);
			this._xSplitter = undefined;
		}

		if (this._ySplitter) {
			this._canvasOverlay.removePath(this._ySplitter);
			this._ySplitter = undefined;
		}
	},

	_cssPixelsToCore: function (cssPixels) {
		return cssPixels.multiplyBy(app.dpiScale);
	},

	_twipsToCorePixels: function (twips) {
		return new cool.Point(
			twips.x * app.twipsToPixels,
			twips.y * app.twipsToPixels);
	},

	_twipsToCorePixelsBounds: function (twips) {
		return new cool.Bounds(
			this._twipsToCorePixels(twips.min),
			this._twipsToCorePixels(twips.max)
		);
	},

	_corePixelsToTwips: function (corePixels) {
		return new cool.Point(
			corePixels.x * app.pixelsToTwips,
			corePixels.y * app.pixelsToTwips);
	},

	_twipsToCssPixels: function (twips) {
		return new cool.Point(
			(twips.x / app.tile.size.x) * (TileManager.tileSize / app.dpiScale),
			(twips.y / app.tile.size.y) * (TileManager.tileSize / app.dpiScale));
	},

	_cssPixelsToTwips: function (pixels) {
		return new cool.Point(
			(pixels.x * app.dpiScale) * app.pixelsToTwips,
			(pixels.y * app.dpiScale) * app.pixelsToTwips);
	},

	_twipsToLatLng: function (twips, zoom) {
		var pixels = this._twipsToCssPixels(twips);
		return this._map.unproject(pixels, zoom);
	},

	_latLngToTwips: function (latLng, zoom) {
		var pixels = this._map.project(latLng, zoom);
		return this._cssPixelsToTwips(pixels);
	},

	_twipsToPixels: function (twips) { // css pixels
		return this._twipsToCssPixels(twips);
	},

	_pixelsToTwips: function (pixels) { // css pixels
		return this._cssPixelsToTwips(pixels);
	},

	_updateMaxBounds: function (sizeChanged, allPages = true) {
		if (app.activeDocument.fileSize.x === 0 || app.activeDocument.fileSize.y === 0) {
			return;
		}

		var docPixelLimits = new cool.Point(app.activeDocument.fileSize.pX / app.dpiScale, app.activeDocument.fileSize.pY / app.dpiScale);
		var scrollPixelLimits = new cool.Point(app.activeDocument.activeLayout.viewSize.pX / app.dpiScale, app.activeDocument.activeLayout.viewSize.pY / app.dpiScale);
		var topLeft = this._map.unproject(new cool.Point(0, 0));

		if (this._documentInfo === '' || sizeChanged) {
			// we just got the first status so we need to center the document
			this._map.setMaxBounds(new window.L.LatLngBounds(topLeft, this._map.unproject(scrollPixelLimits)));
		}

		this._docPixelSize = {x: docPixelLimits.x, y: docPixelLimits.y};
		if (allPages) this._map.fire('scrolllimits', {});
		else this._map.fire('scrolllimit', {})
	},

	// Used with filebasedview.
	_getMostVisiblePart: function (queue) {
		var parts = [];
		var found = false;

		for (var i = 0; i < queue.length; i++) {
			for (var j = 0; j < parts.length; j++) {
				if (parts[j].part === queue[i].part) {
					found = true;
					break;
				}
			}
			if (!found)
				parts.push({part: queue[i].part});
			found = false;
		}

		var partHeightPixels = Math.round((this._partHeightTwips + this._spaceBetweenParts) * app.twipsToPixels);
		var partWidthPixels = Math.round(this._partWidthTwips * app.twipsToPixels);

		var rectangle;
		var maxArea = -1;
		const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle.pToArray();
		const candidates = [];
		for (i = 0; i < parts.length; i++) {
			rectangle = [0, partHeightPixels * parts[i].part, partWidthPixels, Math.round(this._partHeightTwips * app.twipsToPixels)];
			rectangle = app.LOUtil._getIntersectionRectangle(rectangle, viewedRectangle);
			if (rectangle) {
				const currentArea = rectangle[2] * rectangle[3];
				if (currentArea > maxArea) {
					candidates.length = 0;
					maxArea = currentArea;
					candidates.push({part: parts[i].part, area: currentArea});
				}
				else if (currentArea === maxArea)
					candidates.push({part: parts[i].part, area: currentArea});
			}
		}

		// If one of the most visible parts is the selected part, return it.
		for (let i = 0; i < candidates.length; i++) {
			if (candidates[i].area === maxArea && candidates[i].part === this._selectedPart) {
				return this._selectedPart;
			}
		}

		// Otherwise return the first most visible part.
		return candidates[0].part;
	},

	highlightCurrentPart: function (part) {
		var previews = document.getElementsByClassName('preview-frame');
		for (var i = 0; i < previews.length; i++) {
			const img = previews[i].querySelector('img');
			if (parseInt(previews[i].id.replace('preview-frame-part-', '')) === part) {
				window.L.DomUtil.addClass(img, 'preview-img-currentpart');
			}
			else {
				window.L.DomUtil.removeClass(img, 'preview-img-currentpart');
			}
		}
	},

	// Used with file based view. Check the most visible part and set the selected part if needed.
	_checkSelectedPart: function () {
		var queue = TileManager.updateFileBasedView(true);
		if (queue.length > 0) {
			var partToSelect = this._getMostVisiblePart(queue);
			if (this._selectedPart !== partToSelect) {
				this._selectedPart = partToSelect;
				app.socket.sendMessage('setclientpart part=' + this._selectedPart);
			}
			this._preview._scrollToPart();
			this.highlightCurrentPart(partToSelect);
		}
	},

	// Update debug overlay for a tile
	_showDebugForTile: function(key) {
		if (!this._debug.debugOn)
			return;

		const tile = TileManager.get(key);
		tile._debugTime = this._debug.getTimeArray();
	},

	_coordsToPixBounds: function (coords) {
		// coords.x and coords.y are the pixel coordinates of the top-left corner of the tile.
		var topLeft = new cool.Point(coords.x, coords.y);
		var bottomRight = topLeft.add(new cool.Point(TileManager.tileSize, TileManager.tileSize));
		return new cool.Bounds(topLeft, bottomRight);
	},

	hasXSplitter: function () {
		return !!(this._xSplitter);
	},

	hasYSplitter: function () {
		return !!(this._ySplitter);
	},

	getTileSectionPos: function () {
		return this._painter.getTileSectionPos();
	},

	isLayoutRTL: function () {
		return !!this._layoutIsRTL;
	},

	isCalcRTL: function () {
		return this.isCalc() && this.isLayoutRTL();
	}

});

window.L.MessageStore = window.L.Class.extend({

	// ownViewTypes : The types of messages related to own view.
	// otherViewTypes: The types of messages related to other views.
	initialize: function (ownViewTypes, otherViewTypes) {

		if (!Array.isArray(ownViewTypes) || !Array.isArray(otherViewTypes)) {
			window.app.console.error('Unexpected argument types');
			return;
		}

		var ownMessages = {};
		ownViewTypes.forEach(function (msgType) {
			ownMessages[msgType] = '';
		});
		this._ownMessages = ownMessages;

		var othersMessages = {};
		otherViewTypes.forEach(function (msgType) {
			othersMessages[msgType] = [];
		});
		this._othersMessages = othersMessages;
	},

	clear: function (notOtherMsg) {
		var msgs = this._ownMessages;
		Object.keys(msgs).forEach(function (msgType) {
			msgs[msgType] = '';
		});

		if (!notOtherMsg) {
			msgs = this._othersMessages;
			Object.keys(msgs).forEach(function (msgType) {
				msgs[msgType] = [];
			});
		}
	},

	save: function (msgType, textMsg, viewId) {

		var othersMessage = (typeof viewId === 'number');

		if (!othersMessage && Object.prototype.hasOwnProperty.call(this._ownMessages, msgType)) {
			this._ownMessages[msgType] = textMsg;
			return;
		}

		if (othersMessage && Object.prototype.hasOwnProperty.call(this._othersMessages, msgType)) {
			this._othersMessages[msgType][viewId] = textMsg;
		}
	},

	get: function (msgType, viewId) {

		var othersMessage = (typeof viewId === 'number');

		if (!othersMessage && Object.prototype.hasOwnProperty.call(this._ownMessages, msgType)) {
			return this._ownMessages[msgType];
		}

		if (othersMessage && Object.prototype.hasOwnProperty.call(this._othersMessages, msgType)) {
			return this._othersMessages[msgType][viewId];
		}
	},

	forEach: function (callback) {
		if (typeof callback !== 'function') {
			window.app.console.error('Invalid callback type');
			return;
		}

		this._cleanUpSelectionMessages(this._ownMessages);

		var ownMessages = this._ownMessages;
		Object.keys(this._ownMessages).forEach(function (msgType) {
			callback(ownMessages[msgType]);
		});

		var othersMessages = this._othersMessages;
		Object.keys(othersMessages).forEach(function (msgType) {
			othersMessages[msgType].forEach(callback);
		});
	},

	_cleanUpSelectionMessages: function(messages) {
		// must be called only from _replayPrintTwipsMsg !!
		// check if textselection is empty
		// if it is, we need to handle textselectionstart and textselectionend
		// otherwise we get handles without selection and they also may appear in the wrong cell
		// but it is also reproducible on the same cell too. e.g. selection handles without selection
		if (!messages && !messages['textselection'] && messages['textselection'] !== 'textselection: ')
			return;
		messages['textselectionstart'] = 'textselectionstart: ';
		messages['textselectionend'] = 'textselectionend: ';
	}
});
