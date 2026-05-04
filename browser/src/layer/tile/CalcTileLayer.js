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
/*
 * Calc tile layer is used to display a spreadsheet document
 */

/* global app TileManager cool FocusCellSection SplitterLinesSection */

window.L.CalcTileLayer = window.L.CanvasTileLayer.extend({
	options: {
		// TODO: sync these automatically from SAL_LOK_OPTIONS
		sheetGeometryDataEnabled: true,
		printTwipsMsgsEnabled: true,
		syncSplits: true, // if false, the splits/freezes are not synced with other users viewing the same sheet.
	},

	twipsToHMM: function (twips) {
		return (twips * 127 + 36) / 72;
	},

	newAnnotation: function (commentData) {
		var commentList = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).sectionProperties.commentList;
		var comment = null;

		for (var i = 0; i < commentList.length; i++) {
			if (commentList[i].sectionProperties.data.tab == this._selectedPart) {
				if (commentList[i].sectionProperties.data.cellRange.contains(app.calc.cellAddress.toArray())) {
					comment = commentList[i];
					break;
				}
			}
		}

		if (!comment) {
			var pixelStart = new cool.Point(app.calc.cellCursorRectangle.pX1, app.calc.cellCursorRectangle.pY1);
			var rangeStart = this.sheetGeometry.getCellFromPos(pixelStart, 'corepixels');
			var pixelEnd = new cool.Point(app.calc.cellCursorRectangle.pX2 - 1, app.calc.cellCursorRectangle.pY2 - 1);
			var rangeEnd = this.sheetGeometry.getCellFromPos(pixelEnd, 'corepixels');

			var newComment = {
				cellRange: new cool.Bounds(rangeStart, rangeEnd),
				anchorPos: app.calc.cellCursorRectangle.toArray(),
				id: 'new',
				tab: this._selectedPart,
				dateTime: new Date().toISOString(),
				author: this._map.getViewName(this._viewId),
				threaded: commentData ? commentData.threaded : undefined,
			};

			if (app.sectionContainer.doesSectionExist('new comment')) // If adding a new comment has failed, we need to remove the leftover.
				app.sectionContainer.removeSection('new comment');

			comment = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).add(newComment);
			comment.positionCalcComment();
		}
		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).modify(comment);
		comment.focus();
	},

	beforeAdd: function (map) {
		map._addZoomLimit(this);
		map.on('zoomend', this._onZoomRowColumns, this);
		map.on('updateparts', this._onUpdateParts, this);
		map.on('splitposchanged', this.setSplitCellFromPos, this);
		map.on('commandstatechanged', this._onCommandStateChanged, this);
		map.uiManager.initializeSpecializedUI('spreadsheet');
		window.keyboard.hintOnscreenKeyboard(window.keyboard.guessOnscreenKeyboard());
	},

	onAdd: function (map) {
		map.addControl(window.L.control.tabs());
		window.L.CanvasTileLayer.prototype.onAdd.call(this, map);

		map.on('resize', function () {
			if (app.file.textCursor.visible) {
				this._onUpdateCursor(true /* scroll */);
			}
		}.bind(this));

		app.sectionContainer.addSection(new app.definitions.CellFillMarkerSection());
		app.sectionContainer.addSection(new SplitterLinesSection());
		app.sectionContainer.addSection(new app.definitions.TableFillMarkerSection());

		this.insertMode = false;
		this._resetInternalState();
		this._sheetSwitch = new cool.SheetSwitchViewRestore(map);
		this._sheetGrid = true;

		if (window.prefs.getBoolean('ColumnRowHighlightEnabled', false))
			FocusCellSection.addFocusCellSection();
	},

	_resetInternalState: function() {
		this._cellSelections = Array(0);
		app.calc.cellCursorVisible = false;
		this._gotFirstCellCursor = false;
		this._lastColumn = 0; // with data
		this._lastRow = 0; // with data
		this.requestCellCursor();
	},

	_onUpdateParts: function (e) {
		if (typeof this._prevSelectedPart === 'number' && !e.source) {
			this.refreshViewData(undefined, false /* compatDataSrcOnly */, true /* sheetGeometryChanged */);
			this._switchSplitPanesContext();
		}
	},

	_onMessage: function (textMsg, img) {
		if (textMsg.startsWith('invalidateheader: column')) {
			this.refreshViewData({x: this._map._getTopLeftPoint().x, y: 0,
				offset: {x: undefined, y: 0}}, true /* compatDataSrcOnly */);
		} else if (textMsg.startsWith('invalidateheader: row')) {
			this.refreshViewData({x: 0, y: this._map._getTopLeftPoint().y,
				offset: {x: 0, y: undefined}}, true /* compatDataSrcOnly */);
		} else if (textMsg.startsWith('invalidateheader: all')) {
			this.refreshViewData({x: this._map._getTopLeftPoint().x, y: this._map._getTopLeftPoint().y,
				offset: {x: undefined, y: undefined}}, true /* compatDataSrcOnly */);
		} else if (this.options.sheetGeometryDataEnabled &&
				textMsg.startsWith('invalidatesheetgeometry:')) {
			var params = textMsg.substring('invalidatesheetgeometry:'.length).trim().split(' ');
			var flags = {};
			params.forEach(function (param) {
				flags[param] = true;
			});
			this.requestSheetGeometryData(flags);
		} else if (textMsg.startsWith('printranges:')) {
			this._onPrintRangesMsg(textMsg);
		} else {
			window.L.CanvasTileLayer.prototype._onMessage.call(this, textMsg, img);
		}
	},

	// This is used to read and parse printranges so that the next
	// canvas grid paint will show the visual indication of the print range
	// in the current sheet if any.
	_onPrintRangesMsg: function (textMsg) {
		textMsg = textMsg.substr('printranges:'.length);
		var msgData = JSON.parse(textMsg);
		if (!msgData['printranges'] || !Array.isArray(msgData['printranges']))
			return;

		if (!this._printRanges) {
			this._printRanges = [];
		}

		msgData['printranges'].forEach(function (sheetPrintRange) {
			if (typeof sheetPrintRange['sheet'] !== 'number' || !Array.isArray(sheetPrintRange['ranges'])) {
				return;
			}

			this._printRanges[sheetPrintRange['sheet']] = sheetPrintRange['ranges'];
		}, this);
	},

	_onSetPartMsg: function (textMsg) {
		var part = parseInt(textMsg.match(/\d+/g)[0]);
		if (!app.calc.isPartHidden(part)) {
			this.refreshViewData(undefined, true /* compatDataSrcOnly */, false /* sheetGeometryChanged */);
			this._replayPrintTwipsMsgAllViews('cellviewcursor');
			this._replayPrintTwipsMsgAllViews('textviewselection');
			// Hide previous tab's shown comment (if any).
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).hideAllComments();
			this._sheetSwitch.gotSetPart(part);
			this._syncTileContainerSize();
		}
	},

	_onZoomRowColumns: function () {
		this._sendClientZoom();
		if (this.sheetGeometry) {
			this.sheetGeometry.setTileGeometryData(app.tile.size.x, app.tile.size.y,
				TileManager.tileSize);
		}
		this._restrictDocumentSize();
		this.dontSendSplitPosToCore = true;
		this.setSplitPosFromCell();
		this.dontSendSplitPosToCore = false;
		this._map.fire('zoomchanged');
		this.refreshViewData();
		this._replayPrintTwipsMsgs(false);
	},

	_restrictDocumentSize: function () {
		if (!this.sheetGeometry || !this._lastColumn || !this._lastRow) {
			return;
		}

		var maxDocSize = this.sheetGeometry.getSize('tiletwips');
		var newDocWidth = Math.min(maxDocSize.x, app.activeDocument.fileSize.x);
		var newDocHeight = Math.min(maxDocSize.y, app.activeDocument.fileSize.y);

		var lastCellPixel = this.sheetGeometry.getCellRect(this._lastColumn, this._lastRow);
		var isCalcRTL = this._map._docLayer.isCalcRTL();
		lastCellPixel = isCalcRTL ? lastCellPixel.getBottomRight() : lastCellPixel.getBottomLeft();
		var lastCellTwips = this._corePixelsToTwips(lastCellPixel);
		var mapSizeTwips = this._pixelsToTwips(this._map.getSize());
		var mapPosTwips = this._pixelsToTwips(this._map._getTopLeftPoint());

		// margin outside data area we allow to scroll
		// has to be bigger on mobile to allow scroll
		// to the next place where we extend that area
		// (allow few mobile screens down and right)
		var limitMargin = mapSizeTwips;
		if (!window.mode.isDesktop()) {
			limitMargin.x *= 8;
			limitMargin.y *= 8;
		}

		var limitWidth = mapPosTwips.x + mapSizeTwips.x < lastCellTwips.x && !this.widthShrinked;
		var limitHeight = mapPosTwips.y + mapSizeTwips.y < lastCellTwips.y && !this.heightShrinked;

		// limit to data area only (and map size for margin)
		if (limitWidth)
			newDocWidth = lastCellTwips.x + limitMargin.x;

		if (limitHeight)
			newDocHeight = lastCellTwips.y + limitMargin.y;

		var extendedLimit = false;

		if (!limitWidth && maxDocSize.x > app.activeDocument.fileSize.x) {
			newDocWidth = Math.min(app.activeDocument.fileSize.x + mapSizeTwips.x, maxDocSize.x);
			extendedLimit = true;
		}

		if (!limitHeight && maxDocSize.y > app.activeDocument.fileSize.y) {
			newDocHeight = Math.min(app.activeDocument.fileSize.y + mapSizeTwips.y, maxDocSize.y);
			extendedLimit = true;
		}

		var shouldRestrict = (newDocWidth !== app.activeDocument.fileSize.x ||
				newDocHeight !== app.activeDocument.fileSize.y);

		if (!shouldRestrict) {
			return;
		}

		// When there will be a latlng conversion, we should use CSS pixels.
		var newSizePx = this._twipsToPixels(new cool.Point(newDocWidth, newDocHeight));

		var topLeft = this._map.unproject(new cool.Point(0, 0));
		var bottomRight = this._map.unproject(newSizePx);

		this._docPixelSize = newSizePx.clone();
		app.activeDocument.fileSize = new cool.SimplePoint(newDocWidth, newDocHeight);
		app.activeDocument.activeLayout.viewSize = app.activeDocument.fileSize.clone();

		this._map.setMaxBounds(new window.L.LatLngBounds(topLeft, bottomRight));

		this._map.fire('scrolllimits', newSizePx.clone());

		if (!this._syncTileContainerSize() && (limitWidth || limitHeight || extendedLimit))
			app.sectionContainer.requestReDraw();
	},

	_getCursorPosSize: function () {
		var x = -1, y = -1;
		var size = new cool.Point(0, 0);

		if (app.calc.cellCursorVisible) {
			x = app.calc.cellAddress.x + 1;
			y = app.calc.cellAddress.y + 1;

			size = { x: app.calc.cellCursorRectangle.width, y: app.calc.cellCursorRectangle.height };
		}

		return { curX: x, curY: y, width: size.x, height: size.y };
	},

	_hasPartsCountOrNamesChanged(lastStatusJSON, statusJSON) {
		if (!lastStatusJSON)
			return true;

		if (lastStatusJSON.parts.length !== statusJSON.parts.length)
			return true;
		else {
			for (let i = 0; i < statusJSON.parts.length; i++) {
				if (statusJSON.parts[i].name !== lastStatusJSON.parts[i].name)
					return true;
			}
			return false;
		}
	},

	_refreshPartNames(statusJSON) {
		this._partNames = [];

		for (let i = 0; i < statusJSON.parts.length; i++) {
			this._partNames.push(statusJSON.parts[i].name);
		}
	},

	_refreshPartHashes(statusJSON) {
		app.calc.partHashes = [];

		for (let i = 0; i < statusJSON.parts.length; i++) {
			app.calc.partHashes.push(statusJSON.parts[i].hash);
		}
	},

	_getMarginPropertiesForTheMap: function() {
		const rowHeaderSection = app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name);
		const columnHeaderSection = app.sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name);
		const rowGroupSection = app.sectionContainer.getSectionWithName(app.CSections.RowGroup.name);
		const columnGroupSection = app.sectionContainer.getSectionWithName(app.CSections.ColumnGroup.name);
		const scrollSection = app.sectionContainer.getSectionWithName(app.CSections.Scroll.name);
		const scrollBarThickness = scrollSection ? scrollSection.sectionProperties.scrollBarThickness : 0;

		const marginLeft = (rowHeaderSection ? rowHeaderSection.size[0] : 0) + (rowGroupSection ? rowGroupSection.size[0] : 0);
		const marginTop = (columnHeaderSection ? columnHeaderSection.size[1] : 0) + (columnGroupSection ? columnGroupSection.size[1] : 0);

		return { marginLeft, marginTop, scrollBarThickness };
	},

	_calculateNewCanvasAndMapSizes: function(documentContainerSize, availableSpace, marginLeft, marginTop, scrollBarThickness) {
		let newMapSize = availableSpace.slice();
		let newCanvasSize = documentContainerSize.slice();

		const fileSizePixels = app.activeDocument.fileSize.pToArray();

		// If we don't need that much space.
		if (fileSizePixels[0] < availableSpace[0]) {
			newMapSize[0] = fileSizePixels[0];
			newCanvasSize[0] = fileSizePixels[0] + marginLeft + scrollBarThickness;
			this.widthShrinked = true;
		}
		else this.widthShrinked = false;

		if (fileSizePixels[1] < availableSpace[1]) {
			newMapSize[1] = fileSizePixels[1];
			newCanvasSize[1] = fileSizePixels[1] + marginTop + scrollBarThickness;
			this.heightShrinked = true;
		}
		else this.heightShrinked = false;

		newMapSize = [Math.round(newMapSize[0] / app.dpiScale), Math.round(newMapSize[1] / app.dpiScale)];
		newCanvasSize = [Math.round(newCanvasSize[0] / app.dpiScale), Math.round(newCanvasSize[1] / app.dpiScale)];

		return { newMapSize, newCanvasSize };
	},

	_resizeMapElementAndTilesLayer: function(mapElement, marginLeft, marginTop, newMapSize) {
		mapElement.style.left = Math.round(marginLeft / app.dpiScale) + 'px';
		mapElement.style.top = Math.round(marginTop / app.dpiScale) + 'px';
		mapElement.style.width = newMapSize[0] + 'px';
		mapElement.style.height = newMapSize[1] + 'px';

		this._container.style.width = newMapSize[0] + 'px';
		this._container.style.height = newMapSize[1] + 'px';
	},

	_updateHeaderSections: function() {
		if (app.sectionContainer.doesSectionExist(app.CSections.RowHeader.name)) {
			app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name)._updateCanvas();
			app.sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name)._updateCanvas();
		}
	},

	_syncTileContainerSize: function(force = false) {
		if (!this._map) return false;

		if (!this._container) return false;

		// Document container size is up to date as of now.
		const documentContainerSize = this._getDocumentContainerSize();
		documentContainerSize[0] *= app.dpiScale;
		documentContainerSize[1] *= app.dpiScale;

		// Size has changed. Our map and canvas are not resized yet.
		// But the row header, row group, column header and column group sections don't need to be resized.
		// We can get their width and height from the sections' properties.
		const { marginLeft, marginTop, scrollBarThickness } = this._getMarginPropertiesForTheMap();

		// Available for tiles section.
		const availableSpace = [documentContainerSize[0] - marginLeft - scrollBarThickness, documentContainerSize[1] - marginTop - scrollBarThickness];
		const { newMapSize, newCanvasSize } = this._calculateNewCanvasAndMapSizes(documentContainerSize, availableSpace, marginLeft, marginTop, scrollBarThickness);

		const mapElement = document.getElementById('map'); // map's size = tiles section's size.
		const oldMapSize = [mapElement.clientWidth, mapElement.clientHeight];
		const widthIncreased = oldMapSize[0] < newMapSize[0];
		const heightIncreased = oldMapSize[1] < newMapSize[1];
		const sizeChanged = oldMapSize[0] !== newMapSize[0] || oldMapSize[1] !== newMapSize[1];

		// Early exit. If there is no need to update the size, return here.
		if (sizeChanged || force) {
			this._resizeMapElementAndTilesLayer(mapElement, marginLeft, marginTop, newMapSize);

			this._map.invalidateSize(false, new cool.Point(oldMapSize[0], oldMapSize[1]));
			app.sectionContainer.onResize(newCanvasSize[0], newCanvasSize[1]); // Canvas's size = documentContainer's size.
			this._updateHeaderSections();

			this._mobileChecksAfterResizeEvent(heightIncreased);
		}

		// Center the view w.r.t the new map-pane position using the current zoom.
		this._map.setView(this._map.getCenter());

		if (sizeChanged || force) {
			// We want to keep cursor visible when we show the keyboard on mobile device or tablet
			this._nonDesktopChecksAfterResizeEvent(heightIncreased);

			if (heightIncreased || widthIncreased) {
				app.sectionContainer.requestReDraw();
				this._map.fire('sizeincreased');
				return true;
			}
		}

		return false;
	},

	_onStatusMsg: function (textMsg) {
		console.log('DEBUG: onStatusMsg: ' + textMsg);

		const statusJSON = JSON.parse(textMsg.replace('status:', '').replace('statusupdate:', ''));

		if (statusJSON.width && statusJSON.height && this._documentInfo !== textMsg) {
			const previousStatusJSON = this._lastStatusJSON ? Object.assign({}, this._lastStatusJSON): null;
			this._lastStatusJSON = statusJSON;

			if (statusJSON.readonly && !this._documentInfo)
				this._map.setPermission('readonly');

			this._documentInfo = textMsg;

			var firstSelectedPart = (typeof this._selectedPart !== 'number');

			app.activeDocument.fileSize = new cool.SimplePoint(statusJSON.width, statusJSON.height);
			app.activeDocument.activeLayout.viewSize = app.activeDocument.fileSize.clone();

			if (app.map._docLoaded)
				this._syncTileContainerSize();

			this._docType = statusJSON.type;
			this._parts = statusJSON.partscount;

			if (app.socket._reconnecting) {
				app.socket.sendMessage('setclientpart part=' + this._selectedPart);
				this._resetInternalState();
				window.keyboard.hintOnscreenKeyboard(window.keyboard.guessOnscreenKeyboard());
			} else {
				this._selectedPart = statusJSON.selectedpart;
			}

			this._lastColumn = statusJSON.lastcolumn;
			this._lastRow = statusJSON.lastrow;

			const mode = (statusJSON.mode !== undefined) ? statusJSON.mode : 0;
			app.activeDocument.activeModes = [mode];

			if (this.sheetGeometry && this._selectedPart != this.sheetGeometry.getPart()) {
				// Core initiated sheet switch, need to get full sheetGeometry data for the selected sheet.
				this.requestSheetGeometryData();
			}

			this._viewId = statusJSON.viewid;
			app.activeDocument.setActiveViewID(this._viewId);

			console.assert(this._viewId >= 0, 'Incorrect viewId received: ' + this._viewId);

			var mapSize = this._map.getSize();
			var sizePx = this._twipsToPixels(new cool.Point(app.activeDocument.fileSize.x, app.activeDocument.fileSize.y));
			var width = sizePx.x;
			var height = sizePx.y;

			if (width < mapSize.x || height < mapSize.y) {
				width = Math.max(width, mapSize.x);
				height = Math.max(height, mapSize.y);
				var topLeft = this._map.unproject(new cool.Point(0, 0));
				var bottomRight = this._map.unproject(new cool.Point(width, height));
				this._map.setMaxBounds(new window.L.LatLngBounds(topLeft, bottomRight));
				this._docPixelSize = {x: width, y: height};
				this._map.fire('scrolllimits', {x: width, y: height});
			}
			else {
				this._updateMaxBounds(true);
			}

			this._adjustCanvasSectionsForLayoutChange();

			this._refreshPartNames(statusJSON);
			this._refreshPartHashes(statusJSON);

			// if the number of parts, or order has changed then refresh comment positions
			if (this._hasPartsCountOrNamesChanged(previousStatusJSON, statusJSON))
				app.socket.sendMessage('commandvalues command=.uno:ViewAnnotationsPosition');


			this._map.fire('updateparts', {
				selectedPart: this._selectedPart,
				parts: this._parts,
				docType: this._docType,
				source: 'status',
				partNames: this._partNames
			});

			TileManager.resetPreFetching(true);

			/*
				Side note: There is a getPrintRanges function on the core side that sends the JSON inside a printranges object.
				When we get the printranges object, we put it into another printranges object on the server side. So we have a longer path here.
			*/
			if (statusJSON.printranges && statusJSON.printranges.printranges) {
				this._printRanges = [];
				const info = statusJSON.printranges.printranges;
				for (let i = 0; i < info.length; i++)
					this._printRanges[info[i]['sheet']] = info[i]['ranges'];
			}

			if (firstSelectedPart || (previousStatusJSON && previousStatusJSON.selectedpart !== statusJSON.selectedpart))
				this._switchSplitPanesContext();

			this._map.fire('statusupdated');
		} else {
			this._adjustCanvasSectionsForLayoutChange();
		}

		var scrollSection = app.sectionContainer.getSectionWithName(app.CSections.Scroll.name);
		scrollSection.stepByStepScrolling = true;
	},

	// This initiates a selective repainting of row/col headers and
	// gridlines based on the settings of coordinatesData.offset. This
	// should be called whenever the view area changes (scrolling, panning,
	// zooming, cursor moving out of view-area etc.).  Depending on the
	// active sheet geometry data-source, it may ask core to send current
	// view area's data or the global data on geometry changes.
	refreshViewData: function (coordinatesData, compatDataSrcOnly, sheetGeometryChanged) {

		if (this.options.sheetGeometryDataEnabled && compatDataSrcOnly) {
			return;
		}
		// There are places that call this function with no arguments to indicate that the
		// command arguments should be the current map area coordinates.
		if (typeof coordinatesData != 'object') {
			coordinatesData = {};
		}

		var offset = coordinatesData.offset || {};

		var topLeftPoint = new cool.Point(coordinatesData.x, coordinatesData.y);
		var sizePx = this._map.getSize();

		if (topLeftPoint.x === undefined) {
			topLeftPoint.x = this._map._getTopLeftPoint().x;
		}
		if (topLeftPoint.y === undefined) {
			topLeftPoint.y = this._map._getTopLeftPoint().y;
		}

		var updateRows = true;
		var updateCols = true;

		if (offset.x === 0) {
			updateCols = false;
			if (!this.options.sheetGeometryDataEnabled) {
				topLeftPoint.x = -1;
				sizePx.x = 0;
			}
		}
		if (offset.y === 0) {
			updateRows = false;
			if (!this.options.sheetGeometryDataEnabled) {
				topLeftPoint.y = -1;
				sizePx.y = 0;
			}
		}

		var pos = this._pixelsToTwips(topLeftPoint);
		var size = this._pixelsToTwips(sizePx);

		if (!this.options.sheetGeometryDataEnabled) {
			this.requestViewRowColumnData(pos, size);
			return;
		}

		if (sheetGeometryChanged || !this.sheetGeometry) {
			this.requestSheetGeometryData(
				{columns: updateCols, rows: updateRows});
			return;
		}

		if (this.sheetGeometry) {
			this.sheetGeometry.setViewArea(pos, size);
			this._updateHeadersGridLines(undefined, updateCols, updateRows);
		}
	},

	// This send .uno:ViewRowColumnHeaders command to core with the new view coordinates (tile-twips).
	requestViewRowColumnData: function (pos, size) {

		var payload = 'commandvalues command=.uno:ViewRowColumnHeaders?x=' + Math.round(pos.x) + '&y=' + Math.round(pos.y) +
			'&width=' + Math.round(size.x) + '&height=' + Math.round(size.y);

		app.socket.sendMessage(payload);
	},

	// sends the .uno:SheetGeometryData command optionally with arguments.
	requestSheetGeometryData: function (flags) {
		if (!this.sheetGeometry) {
			// Suppress multiple requests at document load, till we get a response.
			if (this._sheetGeomFirstWait === true) {
				return;
			}
			this._sheetGeomFirstWait = true;
		}
		var unoCmd = '.uno:SheetGeometryData';
		var haveArgs = (typeof flags == 'object' &&
			(flags.columns === true || flags.rows === true || flags.all === true));
		var payload = 'commandvalues command=' + unoCmd;

		if (haveArgs) {
			var argList = [];
			var both = (flags.all === true);
			if (both || flags.columns === true) {
				argList.push('columns=1');
			}
			if (both || flags.rows === true) {
				argList.push('rows=1');
			}

			var dataTypeFlagNames = ['sizes', 'hidden', 'filtered', 'groups'];
			var dataTypesPresent = false;
			dataTypeFlagNames.forEach(function (name) {
				if (flags[name] === true) {
					argList.push(name + '=1');
					dataTypesPresent = true;
				}
			});

			if (!dataTypesPresent) {
				dataTypeFlagNames.forEach(function (name) {
					argList.push(name + '=1');
				});
			}

			payload += '?' + argList.join('&');
		}

		app.socket.sendMessage(payload);
	},

	// Sends a notification to the row/col header and gridline controls that
	// they need repainting.
	// viewAreaData is the parsed .uno:ViewRowColumnHeaders JSON if that source is used.
	// else it should be undefined.
	_updateHeadersGridLines: function (viewAreaData, updateCols, updateRows) {
		this._map.fire('viewrowcolumnheaders', {
			data: viewAreaData,
			updaterows: updateRows,
			updatecolumns: updateCols,
			cursor: this._getCursorPosSize(),
			context: this
		});
	},

	_addRemoveGroupSections: function () {
		// If there are row and column groups at the same time, add CornerGroup section.
		if (this.sheetGeometry._rows._outlines._outlines.length > 0 && this.sheetGeometry._columns._outlines._outlines.length > 0) {
			if (!app.sectionContainer.doesSectionExist(app.CSections.CornerGroup.name))
				app.sectionContainer.addSection(new cool.CornerGroup());
		}
		else { // If not, remove CornerGroup section.
			app.sectionContainer.removeSection(app.CSections.CornerGroup.name);
		}

		// If there are row groups, add RowGroup section.
		if (this.sheetGeometry._rows._outlines._outlines.length > 0) {
			if (!app.sectionContainer.doesSectionExist(app.CSections.RowGroup.name))
				app.sectionContainer.addSection(new cool.RowGroup());
		}
		else { // If not, remove RowGroup section.
			app.sectionContainer.removeSection(app.CSections.RowGroup.name);
		}

		// If there are column groups, add ColumnGroup section.
		if (this.sheetGeometry._columns._outlines._outlines.length > 0) {
			if (!app.sectionContainer.doesSectionExist(app.CSections.ColumnGroup.name)) {
				app.sectionContainer.addSection(new cool.ColumnGroup());
				app.sectionContainer.canvas.style.border = '1px solid darkgrey';
			}
		}
		else { // If not, remove ColumnGroup section.
			app.sectionContainer.removeSection(app.CSections.ColumnGroup.name);
			app.sectionContainer.canvas.style.border = '0px solid darkgrey';
		}
	},

	_setAnchor: function(name, section, value) {
		if (!section) {
			console.debug('_setAnchor: no section found: "' + name + '"');
			return;
		}

		section.anchor = value;
	},

	_adjustCanvasSectionsForLayoutChange: function () {
		var sheetIsRTL = app.calc.isRTL();
		if (sheetIsRTL && this._layoutIsRTL !== true) {
			console.log('debug: in LTR -> RTL canvas section adjustments');
			var sectionContainer = app.sectionContainer;

			var tilesSection = sectionContainer.getSectionWithName(app.CSections.Tiles.name);
			var rowHeaderSection = sectionContainer.getSectionWithName(app.CSections.RowHeader.name);
			var columnHeaderSection = sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name);
			var cornerHeaderSection = sectionContainer.getSectionWithName(app.CSections.CornerHeader.name);
			var columnGroupSection = sectionContainer.getSectionWithName(app.CSections.ColumnGroup.name);
			var rowGroupSection = sectionContainer.getSectionWithName(app.CSections.RowGroup.name);
			var cornerGroupSection = sectionContainer.getSectionWithName(app.CSections.CornerGroup.name);
			// Scroll section covers the entire document area, and needs RTL adjustments internally.

			this._setAnchor('cornerGroupSection', cornerGroupSection, ['top', 'right']);

			this._setAnchor('rowGroupSection', rowGroupSection,
				[[app.CSections.CornerGroup.name, 'bottom', 'top'], 'right']);

			this._setAnchor('columnGroupSection', columnGroupSection,
				['top', [app.CSections.CornerGroup.name, '-left', 'right']]);

			this._setAnchor('cornerHeaderSection', cornerHeaderSection,
				[[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.RowGroup.name, '-left', 'right']]);

			this._setAnchor('rowHeaderSection', rowHeaderSection,
				[[app.CSections.CornerHeader.name, 'bottom', 'top'], [app.CSections.RowGroup.name, '-left', 'right']]);

			this._setAnchor('columnHeaderSection', columnHeaderSection,
				[[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.CornerHeader.name, '-left', 'right']]);
			if (columnHeaderSection) columnHeaderSection.expand = ['left'];

			this._setAnchor('tilesSection', tilesSection,
				[[app.CSections.ColumnHeader.name, 'bottom', 'top'], [app.CSections.RowHeader.name, '-left', 'right']]);

			// Do not set layoutIsRtl to true prematurely. If set before all sections are defined
			// (e.g., during load with onStatusMsg), some sections may not update to their correct positions.
			// Ensure all sections are adjusted first, then set layoutIsRtl to true to show that they have moved.
			if (rowHeaderSection)
				this._layoutIsRTL = true;

			sectionContainer.reNewAllSections(true);
			this._syncTileContainerSize();

		} else if (!sheetIsRTL && this._layoutIsRTL === true) {

			console.log('debug: in RTL -> LTR canvas section adjustments');
			var sectionContainer = app.sectionContainer;

			var tilesSection = sectionContainer.getSectionWithName(app.CSections.Tiles.name);
			var rowHeaderSection = sectionContainer.getSectionWithName(app.CSections.RowHeader.name);
			var columnHeaderSection = sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name);
			var cornerHeaderSection = sectionContainer.getSectionWithName(app.CSections.CornerHeader.name);
			var columnGroupSection = sectionContainer.getSectionWithName(app.CSections.ColumnGroup.name);
			var rowGroupSection = sectionContainer.getSectionWithName(app.CSections.RowGroup.name);
			var cornerGroupSection = sectionContainer.getSectionWithName(app.CSections.CornerGroup.name);

			if (cornerGroupSection) {
				cornerGroupSection.anchor = ['top', 'left'];
			}

			if (rowGroupSection) {
				rowGroupSection.anchor = [[app.CSections.CornerGroup.name, 'bottom', 'top'], 'left'];
			}

			if (columnGroupSection) {
				columnGroupSection.anchor = ['top', [app.CSections.CornerGroup.name, 'right', 'left']];
			}

			cornerHeaderSection.anchor = [[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.RowGroup.name, 'right', 'left']];

			rowHeaderSection.anchor = [[app.CSections.CornerHeader.name, 'bottom', 'top'], [app.CSections.RowGroup.name, 'right', 'left']];

			columnHeaderSection.anchor = [[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.CornerHeader.name, 'right', 'left']];
			columnHeaderSection.expand = ['right'];

			if (rowHeaderSection)
				this._layoutIsRTL = false;

			tilesSection.anchor = [[app.CSections.ColumnHeader.name, 'bottom', 'top'], [app.CSections.RowHeader.name, 'right', 'left']];

			sectionContainer.reNewAllSections(true);
			this._syncTileContainerSize();
		}
	},

	_handleSheetGeometryDataMsg: function (jsonMsgObj, differentSheet) {
		if (!this.sheetGeometry) {
			this._sheetGeomFirstWait = false;
			this.sheetGeometry = new cool.SheetGeometry(jsonMsgObj,
				app.tile.size.x, app.tile.size.y,
				TileManager.tileSize, this._selectedPart);

			app.sectionContainer.addSection(new cool.CornerHeader());
			app.sectionContainer.addSection(new app.definitions.rowHeader());
			app.sectionContainer.addSection(new app.definitions.columnHeader());
		}
		else {
			this.sheetGeometry.update(jsonMsgObj, /* checkCompleteness */ false, this._selectedPart);
		}

		this._replayPrintTwipsMsgs(differentSheet);

		if (this.sheetGeometry.autoFilterChanged) {
			this.sheetGeometry.autoFilterChanged = false;
			let firstVisibleRow = this.sheetGeometry.getFirstNewVisibleRow();
			app.activeDocument.activeLayout.scrollTo(
				this._map._getTopLeftPoint().x,
				this.sheetGeometry.getRowsGeometry().getElementData(firstVisibleRow).startpos);
		} else {
			this.sheetGeometry.setViewArea(
				this._pixelsToTwips(this._map._getTopLeftPoint()),
				this._pixelsToTwips(this._map.getSize()));
		}

		this._addRemoveGroupSections();

		console.log('debug: got sheetGeometry: this._rtlParts = ' + this._rtlParts + ' this._selectedPart = ' + this._selectedPart);

		this._adjustCanvasSectionsForLayoutChange();

		this._updateHeadersGridLines(undefined, true /* updateCols */,
			true /* updateRows */);

		this.dontSendSplitPosToCore = true;
		this.setSplitPosFromCell();
		this.dontSendSplitPosToCore = false;

		app.sectionContainer.reNewAllSections(true);
		this._syncTileContainerSize();

		this._map.fire('sheetgeometrychanged');
	},

	// Calculates the split position in (core-pixels) from the split-cell.
	setSplitPosFromCell: function (forceSplittersUpdate) {
		if (!this.sheetGeometry || !this._splitPanesContext) {
			return;
		}

		this._splitPanesContext.setSplitPosFromCell(forceSplittersUpdate);
	},

	// Calculates the split-cell from the split position in (core-pixels).
	setSplitCellFromPos: function () {

		if (!this.sheetGeometry || !this._splitPanesContext) {
			return;
		}

		this._splitPanesContext.setSplitCellFromPos();
	},

	_switchSplitPanesContext: function () {

		if (!this.hasSplitPanesSupport()) {
			return;
		}

		if (!this._splitPaneCache) {
			this._splitPaneCache = {};
		}

		window.app.console.assert(typeof this._selectedPart === 'number', 'invalid selectedPart');

		var spContext = this._splitPaneCache[this._selectedPart];
		if (!spContext) {
			spContext = new cool.CalcSplitPanesContext(this);
			this._splitPaneCache[this._selectedPart] = spContext;
		}

		this._splitPanesContext = spContext;
		if (this.sheetGeometry) {
			// Force update of the splitter lines.
			this.setSplitPosFromCell(true);
		}
	},

	_onRowColSelCount: function (state) {
		if (state.trim() !== '') {
			var rowCount = parseInt(state.split(', ')[0].trim().split(' ')[0].replace(',', '').replace(',', ''));
			var columnCount = parseInt(state.split(', ')[1].trim().split(' ')[0].replace(',', '').replace(',', ''));
			if (rowCount > 1000000)
				this._map.wholeColumnSelected = true;
			else
				this._map.wholeColumnSelected = false;

			if (columnCount === 1024)
				this._map.wholeRowSelected = true;
			else
				this._map.wholeRowSelected = false;
		}
		else {
			this._map.wholeColumnSelected = false;
			this._map.wholeRowSelected = false;
		}
	},

	_onCommandStateChanged: function (e) {

		if (e.commandName === '.uno:FreezePanesColumn') {
			this._onSplitStateChanged(e, true /* isSplitCol */);
		}
		else if (e.commandName === '.uno:FreezePanesRow') {
			this._onSplitStateChanged(e, false /* isSplitCol */);
		}
		else if (e.commandName === '.uno:RowColSelCount') {
			// We also call the function when state is empty, because row/column variables should be set.
			if (e.state.trim() === '' || e.state.startsWith('Selected'))
				this._onRowColSelCount(e.state.replace('Selected:', '').replace('row', '').replace('column', '').replace('s', ''));
		}
		else if (e.commandName === '.uno:InsertMode') {
			/* If we get textselection message from core:
				When insertMode is active:  User is selecting some text.
				When insertMode is passive: User is selecting cells.
			*/
			this.insertMode = e.state.trim() === '' ? false: true;
			if (!this.insertMode) {
				app.setCursorVisibility(false);
				if (this._map._docLayer._cursorMarker)
					this._map._docLayer._cursorMarker.remove();

				var grid = document.getElementById('document-canvas');
				grid.classList.add('spreadsheet-cursor');
				grid.style.cursor = '';
			}
		}
		else if (e.commandName === '.uno:ToggleSheetGrid') {
			let trimmedState = e.state.trim();
			// Disabled mean we don't change the sheet grid state.
			if (trimmedState != 'disabled') {
				let newState = trimmedState === 'true';
				if (this._sheetGrid != newState) {
					this._sheetGrid = newState;
					app.sectionContainer.requestReDraw();
				}
			}
		}
		else if (e.commandName === 'AutoFilterInfo') {
			app.calc.autoFilterCell = { 'row': e.state.row, 'column': e.state.column };
		}
		else if (e.commandName === 'AutoFilterChange')
		{
			this.sheetGeometry.autoFilterChanged = true;
		}
		else if (e.commandName === 'PivotTableFilterInfo') {
			app.calc.pivotTableFilterCell = { 'row': e.state.row, 'column': e.state.column };
		}
		else if (e.commandName === 'TableAutoFillInfo') {
			this._onTableAutoFillStateChanged(e.state.rectangle);
		}
		else if (e.commandName === 'CellFormulaError') {
			this._onCellFormulaError(e.state);
		}
	},

	_onTableAutoFillStateChanged: function (textMsg) {
		var tablefillMarkerSection = app.sectionContainer.getSectionWithName(app.CSections.TableFillMarker.name);
		if (textMsg.match('EMPTY')) {
			if (tablefillMarkerSection)
				tablefillMarkerSection.calculatePositionViaCellCursor(null);
			this._tableAutoFillAreaPixels = null;
		}
		else
		{
			var strTwips = textMsg.match(/\d+/g);
			if (strTwips != null && this._map.isEditMode()) {
				var topLeftTwips = new cool.Point(parseInt(strTwips[0]), parseInt(strTwips[1]));
				var offset = new cool.Point(parseInt(strTwips[2]), parseInt(strTwips[3]));

				var topLeftPixels = this._twipsToCorePixels(topLeftTwips);
				var offsetPixels = this._twipsToCorePixels(offset);
				this._tableAutoFillAreaPixels = app.LOUtil.createRectangle(topLeftPixels.x, topLeftPixels.y, offsetPixels.x, offsetPixels.y);

				if (tablefillMarkerSection)
					tablefillMarkerSection.calculatePositionViaCellCursor([this._tableAutoFillAreaPixels.x1, this._tableAutoFillAreaPixels.y1]);
			}
			else {
				this._tableAutoFillAreaPixels = null;
			}
		}
	},

	_onSplitStateChanged: function (e, isSplitCol) {
		if (!this._splitPanesContext) {
			return;
		}

		if (!this._splitCellState) {
			this._splitCellState = new cool.Point(-1, -1);
		}

		if (!e.state || e.state.length === 0) {
			window.app.console.warn('Empty argument for ' + e.commandName);
			return;
		}

		var values = e.state.split('/');
		var newSplitIndex = Math.floor(parseInt(values[0]));
		window.app.console.assert(!isNaN(newSplitIndex) && newSplitIndex >= 0, 'invalid argument for ' + e.commandName);

		// This stores the current split-cell state of core, so this should not be modified.
		this._splitCellState[isSplitCol ? 'x' : 'y'] = newSplitIndex;

		if (!this.options.syncSplits) {
			return;
		}

		var changed = isSplitCol ? this._splitPanesContext.setSplitCol(newSplitIndex) :
			this._splitPanesContext.setSplitRow(newSplitIndex);

		if (changed) {
			this.setSplitPosFromCell();
		}
	},

	sendSplitIndex: function (newSplitIndex, isSplitCol) {

		if (!this._map.isEditMode() || !this._splitCellState || !this.options.syncSplits) {
			return false;
		}

		var splitColState = this._splitCellState.x;
		var splitRowState = this._splitCellState.y;
		if (splitColState === -1 || splitRowState === -1) {
			// Did not get the 'first' FreezePanesColumn/FreezePanesRow messages from core yet.
			return false;
		}

		var currentState = isSplitCol ? splitColState : splitRowState;
		if (currentState === newSplitIndex) {
			return false;
		}

		var unoName = isSplitCol ? 'FreezePanesColumn' : 'FreezePanesRow';
		var command = {};
		command['Index'] = {
			type: 'int32',
			value: newSplitIndex
		};

		this._map.sendUnoCommand('.uno:' + unoName, command);
		return true;
	},

	_onCommandValuesMsg: function (textMsg) {
		var jsonIdx = textMsg.indexOf('{');
		if (jsonIdx === -1)
			return;

		var values = JSON.parse(textMsg.substring(jsonIdx));
		if (!values) {
			return;
		}

		var comment;
		if (values.commandName === '.uno:ViewRowColumnHeaders') {
			this._updateHeadersGridLines(values);

		} else if (values.commandName === '.uno:SheetGeometryData') {
			var differentSheet = this.sheetGeometry === undefined || this._selectedPart !== this.sheetGeometry.getPart();
			// duplicate sheet-geometry for same sheet triggers replay of other messages that
			// disrupt the view restore during sheet switch.
			if (this._oldSheetGeomMsg === textMsg && !differentSheet)
				return;

			this._oldSheetGeomMsg = textMsg;
			this._handleSheetGeometryDataMsg(values, differentSheet);
			this._syncTileContainerSize();
		} else if (values.comments) {
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).importComments(values.comments);
		} else if (values.commentsPos) {
			var section = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			// invalidate all comments
			section.sectionProperties.commentList.forEach(function (comment) {
				comment.valid = false;
			});
			for (var index in values.commentsPos) {
				comment = values.commentsPos[index];
				var commentObject = section.getComment(comment.id);
				if (commentObject) {
					if (commentObject.sectionProperties.data.tab !== comment.tab) {
						// tabs can be moved around and we need to update the tab because the id is still valid.
						commentObject.sectionProperties.data.tab = comment.tab;
					}
					commentObject.valid = true;

					// turn cell range string into Bounds
					commentObject.sectionProperties.data.cellRange = this._parseCellRange(comment.cellRange);

				}
			}

			section.onCommentsDataUpdate();

		} else {
			window.L.CanvasTileLayer.prototype._onCommandValuesMsg.call(this, textMsg);
		}
	},

	_onTextSelectionMsg: function (textMsg) {
		window.L.CanvasTileLayer.prototype._onTextSelectionMsg.call(this, textMsg);
		// If this is a cellSelection message, user shouldn't be editing a cell. Below check is for ensuring that.
		if ((this.insertMode === false || app.file.textCursor.visible === false) && app.calc.cellCursorVisible) {
			// When insertMode is false, this is a cell selection message.
			textMsg = textMsg.replace('textselection:', '').trim();
			if (textMsg !== 'EMPTY' && textMsg !== '') {
				this._cellSelections = this._getRawRectangles(textMsg);

				this._cellSelections = this._cellSelections.map(function(element) {
					return new cool.SimpleRectangle(element[0], element[1], element[2], element[3]);
				});
			}
			else {
				this._cellSelections = Array(0);
			}
			this._refreshRowColumnHeaders();
		}
	},

	allowDrawing: function () {
		// Drawing is disabled from CalcTileLayer construction, enable it now.
		this._gotFirstCellCursor = true;
	},

	_onCellCursorMsg: function (textMsg) {
		window.L.CanvasTileLayer.prototype._onCellCursorMsg.call(this, textMsg);
		this._refreshRowColumnHeaders();
		if (!this._gotFirstCellCursor) {
			this.allowDrawing();
			TileManager.update();
			this.enableDrawing();
		}
	},

	_getEditCursorRectangle: function (msgObj) {

		if (!this.options.printTwipsMsgsEnabled || !this.sheetGeometry ||
			!Object.prototype.hasOwnProperty.call(msgObj, 'relrect') || !Object.prototype.hasOwnProperty.call(msgObj, 'refpoint')) {
			// 1) non-print-twips messaging mode OR
			// 2) the edit-cursor belongs to draw/chart objects.
			return window.L.CanvasTileLayer.prototype._getEditCursorRectangle.call(this, msgObj);
		}

		if (typeof msgObj !== 'object') {
			window.app.console.error('invalid edit cursor message');
			return undefined;
		}

		var relrect = cool.Bounds.parse(msgObj.relrect);
		var refpoint = cool.Point.parse(msgObj.refpoint);
		refpoint = this.sheetGeometry.getTileTwipsPointFromPrint(refpoint);
		return relrect.add(refpoint);
	},

	_getTextSelectionRectangles: function (textMsg) {

		if (!this.options.printTwipsMsgsEnabled || !this.sheetGeometry) {
			return window.L.CanvasTileLayer.prototype._getTextSelectionRectangles.call(this, textMsg);
		}

		if (typeof textMsg !== 'string') {
			window.app.console.error('invalid text selection message');
			return [];
		}

		var refpointDelim = '::';
		var delimIndex = textMsg.indexOf(refpointDelim);
		if (delimIndex === -1) {
			// No refpoint information available, treat it as cell-range selection rectangle.
			var rangeRectArray = cool.Bounds.parseArray(textMsg);
			rangeRectArray = rangeRectArray.map(function (rect) {
				return this._convertToTileTwipsSheetArea(rect);
			}, this);
			return rangeRectArray;
		}

		var refpoint = cool.Point.parse(textMsg.substring(delimIndex + refpointDelim.length));
		refpoint = this.sheetGeometry.getTileTwipsPointFromPrint(refpoint);

		var rectArray = cool.Bounds.parseArray(textMsg.substring(0, delimIndex));
		rectArray.forEach(function (rect) {
			rect._add(refpoint); // compute absolute coordinates and update in-place.
		});

		return rectArray;
	},

	getSnapDocPosX: function (docPosX, unit) {
		if (!this.options.sheetGeometryDataEnabled) {
			return docPosX;
		}

		unit = unit || 'corepixels';

		return this.sheetGeometry.getSnapDocPosX(docPosX, unit);
	},

	getSnapDocPosY: function (docPosY, unit) {
		if (!this.options.sheetGeometryDataEnabled) {
			return docPosY;
		}

		unit = unit || 'corepixels';

		return this.sheetGeometry.getSnapDocPosY(docPosY, unit);
	},

	getSplitPanesContext: function () {
		if (!this.hasSplitPanesSupport()) {
			return undefined;
		}

		return this._splitPanesContext;
	},

	getMaxDocSize: function () {

		if (this.sheetGeometry) {
			return this.sheetGeometry.getSize('corepixels');
		}

		return this._twipsToPixels(new cool.Point(app.activeDocument.fileSize.x, app.activeDocument.fileSize.y));
	},

	_calculateScrollForNewCellCursor: function () {
		var scroll = new cool.SimplePoint(0, 0);

		if (!app.calc.cellCursorVisible) {
			return scroll;
		}

		let paneRectangles = app.getViewRectangles(); // SimpleRectangle array.
		let contained = false;
		for (let i = 0; i < paneRectangles.length; i++) {
			if (paneRectangles[i].containsRectangle(app.calc.cellCursorRectangle.toArray()))
				contained = true;
		}

		if (contained)
			return scroll; // No scroll needed.

		var noSplit = !this._splitPanesContext || this._splitPanesContext.getSplitPos().equals(new cool.Point(0, 0));

		// No split panes. Check if target cell is bigger than screen but partially visible.
		if (noSplit && app.calc.cellCursorRectangle.intersectsRectangle(paneRectangles[0].toArray())) {
			if (app.calc.cellCursorRectangle.width > paneRectangles[0].width || app.calc.cellCursorRectangle.height > paneRectangles[0].height)
				return scroll; // no scroll needed.
		}

		let freePane = paneRectangles[paneRectangles.length - 1]; // Last pane, this should be the scrollable - not frozen one.

		// Horizontal split
		if (app.calc.cellCursorRectangle.x2 > app.calc.splitCoordinate.x) {
			if (app.calc.cellCursorRectangle.width > freePane.width)
				return scroll; // no scroll needed.

			if (app.calc.cellCursorRectangle.x1 < freePane.x1) {
				scroll.x = app.calc.cellCursorRectangle.x1 - freePane.x1;
			}
			else if (app.calc.cellCursorRectangle.x2 > freePane.x2) {
				scroll.x = app.calc.cellCursorRectangle.x2 - freePane.x2;
			}
		}

		// Vertical split
		if (app.calc.cellCursorRectangle.y2 > app.calc.splitCoordinate.y) {
			if (app.calc.cellCursorRectangle.height > freePane.height)
				return scroll; // no scroll needed.

			// try to center in free pane the top of a cell
			if (app.calc.cellCursorRectangle.y1 < freePane.y1)
				scroll.y = app.calc.cellCursorRectangle.y1 - freePane.y1;

			// then check if end of a cell is visible
			if (app.calc.cellCursorRectangle.y2 > freePane.y2 + scroll.y)
				scroll.y = scroll.y + (app.calc.cellCursorRectangle.y2 - freePane.y2);
		}

		return scroll;
	},

	getSelectedPart: function () {
		return this._selectedPart;
	},
});
