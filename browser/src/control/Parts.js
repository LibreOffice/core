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
 * Document parts switching and selecting handler
 */

/* global app _ cool */
/* global _ JSDialog app OtherViewCellCursorSection TileManager TextCursorSection */

window.L.Map.include({
	/*
		@param {number} part - Target part
		@param {boolean} external - Do we need to inform a core
		@param {boolean} calledFromSetPartHandler - Requests a scroll to the cursor
	*/
	setPart: function (part, external, calledFromSetPartHandler) {
		const editingComment = cool.Comment.isAnyEdit();
		if (editingComment) {
			const commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			if (commentSection) {
				commentSection.navigateAndFocusComment(editingComment);
			}
			return;
		}

		app.idleHandler.notifyActive();

		var docLayer = this._docLayer;
		var docType = docLayer._docType;
		var isTheSamePart = false;

		// check hashes, when we add/delete/move parts they can have the same part number as before
		if (docType === 'spreadsheet') {
			isTheSamePart =
				app.calc.partHashes[docLayer._prevSelectedPart] === app.calc.partHashes[part];
		} else if ((docType === 'presentation' || docType === 'drawing')) {
			if (docLayer._prevSelectedPart !== undefined && part < app.impress.partList.length && app.impress.partList[docLayer._prevSelectedPart])
				isTheSamePart = app.impress.partList[docLayer._prevSelectedPart].hash === app.impress.partList[part].hash;
		} else if (docType === 'text') {
			isTheSamePart = true;
		} else {
			console.error('Unknown docType: ' + docType);
		}

		if (docLayer._selectedPart === part && isTheSamePart) {
			return;
		}

		if (docLayer.isCalc())
			docLayer._sheetSwitch.save(part /* toPart */);

		docLayer._clearMsgReplayStore(true /* notOtherMsg*/);
		docLayer._prevSelectedPart = docLayer._selectedPart;

		if (part === 'prev') {
			if (docLayer._selectedPart > 0) {
				docLayer._selectedPart -= 1;
				this._partsDirection = -1;
			}
		}
		else if (part === 'next') {
			if (docLayer._selectedPart < docLayer._parts - 1) {
				docLayer._selectedPart += 1;
				this._partsDirection = 1;
			}
		}
		else if (typeof (part) === 'number' && part >= 0 && part < docLayer._parts) {
			this._partsDirection = (part >= docLayer._selectedPart) ? 1 : -1;
			docLayer._selectedPart = part;
			docLayer._updateReferenceMarks();
		}
		else {
			return;
		}

		var notifyServer = function (part) {
			// If this wasn't triggered from the server,
			// then notify the server of the change.
			if (!external)
				app.socket.sendMessage('setclientpart part=' + part);
		};

		if (app.file.fileBasedView) {
			docLayer._preview._scrollViewToPartPosition(docLayer._selectedPart);
			// _checkSelectedPart's "most visible part" reconciliation is for
			// free scroll; calling it after an explicit setPart would race
			// notifyServer with a stale guess. Run only its UI side effects.
			docLayer._preview._scrollToPart(docLayer._selectedPart);
			docLayer.highlightCurrentPart(docLayer._selectedPart);
			notifyServer(part);
			return;
		}

		this.fire('scrolltopart');

		if (app.file.textCursor.visible) {
			// a click outside the slide to clear any selection
			app.socket.sendMessage('resetselection');
		}

		notifyServer(docLayer._selectedPart);

		this.fire('updateparts', {
			selectedPart: docLayer._selectedPart,
			parts: docLayer._parts,
			docType: docLayer._docType
		});

		OtherViewCellCursorSection.updateVisibilities();
		TextCursorSection.updateVisibilities();
		app.definitions.otherViewGraphicSelectionSection.updateVisibilities();
		docLayer._clearSelections(calledFromSetPartHandler);
		TileManager.updateOnChangePart();
		TileManager.pruneTiles();
		docLayer._prevSelectedPartNeedsUpdate = true;
		if (docLayer._invalidatePreviews) {
			docLayer._invalidatePreviews();
		}
		if (this._docLayer._docType === 'presentation' || this._docLayer._docType === 'drawing') {
			this._docLayer._preview.focusCurrentSlide();
		} else {
			this.focus();
		}
	},

	// part is the part index/id
	// how is 0 to deselect, 1 to select, and 2 to toggle selection
	// This function is Impress only.
	selectPart: function (part, how, external, fireEvent = true) {
		const currentSelectedCount = app.impress.getSelectedSlidesCount();

		const targetPart = app.impress.partList[part];

		if (how < 2) targetPart.selected = how;
		else targetPart.selected = targetPart.selected === 1 ? 0 : 1;

		if (currentSelectedCount !== app.impress.getSelectedSlidesCount()) {
			if (fireEvent) this.fire('updateparts', {});

			// If this wasn't triggered from the server,
			// then notify the server of the change.
			if (!external) {
				app.socket.sendMessage('selectclientpart part=' + part + ' how=' + how);
			}
		}
	},

	deselectAll: function() {
		this._docLayer._preview._selectedPartRange = undefined;
		for (let i = 0; i < app.impress.partList.length; i++) {
			this.selectPart(i, 0, false, false);
		}
		this.fire('updateparts', {});
	},

	_processPreviewQueue: function() {
		if (!this._docLayer)
			return;

		if (!this._docLayer._canonicalIdInitialized)
			return;

		if (!this._docLayer._preview)
			return;

		if (this._previewRequestsOnFly > 1) {
			// we don't always get a response for each tile requests
			// especially when we have more than one view
			// the server can determine that we have the tile already
			// and does not response to us
			// in that case we cannot decrease previewRequestsOnFly counter
			// we should not wait more than 2 seconds for each 3 requests
			var now = new Date();
			if (now - this._timeToEmptyQueue < 2000)
				// wait until the queue is empty
				return;
			else {
				this._previewRequestsOnFly = 0;
				this._timeToEmptyQueue = now;
			}
		}

		var previewParts = [];
		// take 3 requests from the queue:
		while (this._previewRequestsOnFly < 3) {
			var tile = this._previewQueue.shift();
			if (!tile)
				break;
			var isVisible = this._docLayer._preview._isPreviewVisible(tile[0]);
			if (isVisible != true && tile[1].indexOf('slideshow') < 0)
				// skip this! we can't see it
				continue;
			this._previewRequestsOnFly++;
			this.fire('beforerequestpreview', { part: tile[0] });
			app.socket.sendMessage(tile[1]);
			previewParts.push(tile[0]);
		}

		if (previewParts.length > 0)
			window.app.console.debug('PREVIEW: request preview parts : ' + previewParts.join());
	},

	_addPreviewToQueue: function(part, tileMsg) {
		for (var tile in this._previewQueue)
			if (this._previewQueue[tile][0] === part)
				// we already have this tile in the queue
				// no need to ask for it twice
				return;
		this._previewQueue.push([part, tileMsg]);
	},

	getPreview: function (id, part, maxWidth, maxHeight, options) {

		if (!this._docPreviews) this._docPreviews = {};

		const autoUpdate = options ? !!options.autoUpdate : false;
		const fetchThumbnail = options && options.fetchThumbnail !== undefined ? options.fetchThumbnail : true;
		const isSlideshow = options && options.slideshow !== undefined ? options.slideshow : false;

		this._docPreviews[id] = {id: id, index: part, maxWidth: maxWidth, maxHeight: maxHeight, autoUpdate: autoUpdate, invalid: false};

		let docLayer = this._docLayer;

		if (docLayer._docType === 'text') return;

		// Use part specific dimensions if available, otherwise fall back to document size
		let tileWidth, tileHeight;
		if (docLayer._partDimensions.length === docLayer._parts) {
			tileWidth = docLayer.getPartWidth(part);
			tileHeight = docLayer.getPartHeight(part);
		} else {
			tileWidth = docLayer._partWidthTwips ? docLayer._partWidthTwips: app.activeDocument.fileSize.x;
			tileHeight = docLayer._partHeightTwips ? docLayer._partHeightTwips: app.activeDocument.fileSize.y;
		}

		const docRatio = tileWidth / tileHeight;
		const imgRatio = maxWidth / maxHeight;

		// fit into the given rectangle while maintaining the ratio
		if (imgRatio > docRatio) maxWidth = Math.round(tileWidth * maxHeight / tileHeight);
		else maxHeight = Math.round(tileHeight * maxWidth / tileWidth);

		if (fetchThumbnail) {
			var mode = app.activeDocument.activeModes[0];
			this._addPreviewToQueue(part, 'tile ' +
							'nviewid=0' + ' ' +
							'part=' + String(part) + ' ' +
							'mode=' + String(mode) + ' ' +
							'width=' + String(maxWidth * app.roundedDpiScale) + ' ' +
							'height=' + String(maxHeight * app.roundedDpiScale) + ' ' +
							'tileposx=' + '0 ' +
							'tileposy=' + '0 ' +
							'tilewidth=' + String(tileWidth) + ' ' +
							'tileheight=' + String(tileHeight) + ' ' +
							'id=' + String(id) +
							(isSlideshow ? ' slideshow=1' : ''));
			this._processPreviewQueue();
		}

		return {width: maxWidth, height: maxHeight};
	},

	// getCustomPreview
	// Triggers the creation of a preview with the given id, of width X height size, of the [(tilePosX,tilePosY),
	// (tilePosX + tileWidth, tilePosY + tileHeight)] section of the document.
	getCustomPreview: function (id, part, width, height, tilePosX, tilePosY, tileWidth, tileHeight, options) {
		if (!this._docPreviews) {
			this._docPreviews = {};
		}
		var autoUpdate = options ? options.autoUpdate : false;
		this._docPreviews[id] = {id: id, part: part, width: width, height: height, tilePosX: tilePosX,
			tilePosY: tilePosY, tileWidth: tileWidth, tileHeight: tileHeight, autoUpdate: autoUpdate, invalid: false};

		var mode = app.activeDocument.activeModes[0];
		this._addPreviewToQueue(part, 'tile ' +
							'nviewid=0' + ' ' +
							'part=' + part + ' ' +
							((mode !== 0) ? ('mode=' + mode + ' ') : '') +
							'width=' + width * app.roundedDpiScale + ' ' +
							'height=' + height * app.roundedDpiScale + ' ' +
							'tileposx=' + tilePosX + ' ' +
							'tileposy=' + tilePosY + ' ' +
							'tilewidth=' + tileWidth + ' ' +
							'tileheight=' + tileHeight + ' ' +
							'id=' + id);
		this._processPreviewQueue();
	},

	_resolveCurrentPage: function (page, currentPage, pageRects) {
		if ((page !== 'prev' && page !== 'next') || this.isEditMode() || !pageRects || pageRects.length === 0) {
			return currentPage;
		}

		// In the mode where the cursor is absent, _currentPage may be stale.
		// Determine the currently visible page from the viewport instead.
		for (let i = 0; i < pageRects.length; i++) {
			if (!app.isRectangleVisibleInTheDisplayedArea(pageRects[i]))
				continue;

			return i;
		}

		return currentPage;
	},

	goToPage: function (page) {
		const docLayer = this._docLayer;
		const pageRects = app.file && app.file.writer && app.file.writer.pageRectangleList;
		const sourcePage = this._resolveCurrentPage(page, docLayer._currentPage, pageRects);
		app.idleHandler.notifyActive();

		if (page === 'prev') {
			if (sourcePage > 0) {
				docLayer._currentPage = sourcePage - 1;
			}
		}
		else if (page === 'next') {
			if (sourcePage < docLayer._pages - 1) {
				docLayer._currentPage = sourcePage + 1;
			}
		}
		else if (typeof (page) === 'number' && page >= 0 && page < docLayer._pages) {
			docLayer._currentPage = page;
		}

		if (!this.isEditMode() && pageRects && pageRects.length > docLayer._currentPage) {
			const posY = Math.round(pageRects[docLayer._currentPage][1] / app.dpiScale);

			const section = app.sectionContainer.getSectionWithName(app.CSections.Scroll.name);
			if (section)
				section.onScrollTo({x: 0, y: posY});

			const state = 'Page ' + (docLayer._currentPage + 1) + ' of ' + pageRects.length;
			this.fire('updatestatepagenumber', {
				state: state
			});
		}
		else {
			app.socket.sendMessage('setpage page=' + docLayer._currentPage);
		}
		this.fire('pagenumberchanged', {
			currentPage: docLayer._currentPage,
			pages: docLayer._pages,
			docType: docLayer._docType
		});
	},

	insertPage: function(nPos) {
		const editingComment = cool.Comment.isAnyEdit();
		if (editingComment) {
			const commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			if (commentSection) {
				commentSection.navigateAndFocusComment(editingComment);
			}
			return;
		}

		app.idleHandler.notifyActive();
		if (this.isPresentationOrDrawing()) {
			if (nPos === undefined) {
				app.socket.sendMessage('uno .uno:InsertPage');
			}
			else {
				var argument = {InsertPos: {type: 'int16', value: nPos}};
				app.socket.sendMessage('uno .uno:InsertPage ' + JSON.stringify(argument));
			}
		}
		else if (this.getDocType() === 'spreadsheet') {
			this._docLayer._sheetSwitch.updateOnSheetInsertion(nPos);
			var command = {
				'Name': {
					'type': 'string',
					'value': ''
				},
				'Index': {
					'type': 'long',
					'value': nPos + 1
				}
			};

			app.socket.sendMessage('uno .uno:Insert ' + JSON.stringify(command));
		}
		else {
			return;
		}

		var docLayer = this._docLayer;

		// At least for Impress, we should not fire this. It causes a circular reference.
		if (!this.isPresentationOrDrawing()) {
			this.fire('insertpage', {
				selectedPart: docLayer._selectedPart,
				parts:        docLayer._parts
			});
		}

		docLayer._parts++;

		// user interaction - follow own cursor so it's visible after switch
		if (this.userList)
			this.userList.followUser(docLayer._getViewId());

		// Since we know which part we want to set, use the index (instead of 'next', 'prev')
		if (typeof nPos === 'number') {
			this.setPart(nPos);
		}
		else {
			this.setPart('next');
		}
	},

	duplicatePage: function(pos) {
		if (!this.isPresentationOrDrawing()) {
			return;
		}
		app.idleHandler.notifyActive();

		if (pos === undefined) {
			app.socket.sendMessage('uno .uno:DuplicatePage');
		} else {
			var argument = {InsertPos: {type: 'int16', value: pos}};
			app.socket.sendMessage('uno .uno:DuplicatePage ' + JSON.stringify(argument));
		}
		var docLayer = this._docLayer;

		docLayer._parts++;
		this.setPart('next');
	},

	deletePage: function (nPos) {
		if (this.isPresentationOrDrawing()) {
			app.socket.sendMessage('uno .uno:DeletePage');
		}
		else if (this.getDocType() === 'spreadsheet') {
			this._docLayer._sheetSwitch.updateOnSheetDeleted(nPos);
			var command = {
				'Index': {
					'type': 'long',
					'value': nPos + 1
				}
			};

			app.socket.sendMessage('uno .uno:Remove ' + JSON.stringify(command));
		}
		else {
			return;
		}
		app.idleHandler.notifyActive();

		var docLayer = this._docLayer;
		// TO DO: Deleting all the pages causes problem.
		if (docLayer._parts === 1) {
			return;
		}

		if (this.getDocType() === 'spreadsheet' && docLayer._parts <= app.calc.getHiddenPartCount() + 1) {
			return;
		}

		// At least for Impress, we should not fire this. It causes a circular reference.
		if (!this.isPresentationOrDrawing()) {
			this.fire('deletepage', {
				selectedPart: docLayer._selectedPart,
				parts:        docLayer._parts
			});
		}

		docLayer._parts--;
		if (docLayer._selectedPart >= docLayer._parts) {
			docLayer._selectedPart--;
		}

		if (typeof nPos === 'number') {
			this.setPart(nPos);
		}
		else {
			this.setPart(docLayer._selectedPart);
		}
	},

	renamePage: function (name, nPos) {
		if (this.getDocType() === 'spreadsheet') {
			var command = {
				'Name': {
					'type': 'string',
					'value': name
				},
				'Index': {
					'type': 'long',
					'value': nPos + 1
				}
			};

			app.socket.sendMessage('uno .uno:Name ' + JSON.stringify(command));
			this.setPart(this._docLayer);
		}
	},

	showPage: function () {
		if (this.getDocType() === 'spreadsheet' && app.calc.isAnyPartHidden()) {
			const dialogId = 'show-sheets-modal';

			const buttonCallback = function() {
				var checkboxList = document.querySelectorAll('input[id^="hidden-part-checkbox"]');
				for (var i = 0; i < checkboxList.length; i++) {
					if (checkboxList[i].checked === true) {
						var partName_ = checkboxList[i].id.replace('hidden-part-checkbox-', '');
						var argument = {aTableName: {type: 'string', value: partName_}};
						app.socket.sendMessage('uno .uno:Show ' + JSON.stringify(argument));
					}
				}
			};

			this.uiManager.showInfoModal(dialogId, _('Show sheets'), ' ', ' ', _('OK'), buttonCallback, true, dialogId + '-response');
			this.showPageModalImpl(dialogId);
		}
	},

	showPageModalImpl: function(dialogId) {
		const modal = document.getElementById(dialogId);

		if (!modal) {
			setTimeout(() => { this.showPageModalImpl(dialogId) }, 10);
			return;
		}

		var hiddenParts = app.calc.getHiddenPartNameArray();

		if (app.calc.isAnyPartHidden()) {
			var container = document.createElement('div');
			container.style.maxHeight = '300px';
			container.style.overflowY = 'auto';
			for (var i = 0; i < hiddenParts.length; i++) {
				var checkbox = document.createElement('input');
				checkbox.type = 'checkbox';
				checkbox.id = 'hidden-part-checkbox-' + hiddenParts[i];
				var label = document.createElement('label');
				label.htmlFor = 'hidden-part-checkbox-' + hiddenParts[i];
				label.innerText = hiddenParts[i];
				var newLine = document.createElement('br');
				container.appendChild(checkbox);
				container.appendChild(label);
				container.appendChild(newLine);
			}
		}

		modal.insertBefore(container, modal.children[0]);

		JSDialog.enableButtonInModal(dialogId, dialogId + '-response', false);

		var checkboxes = document.querySelectorAll('#show-sheets-modal input[type="checkbox"]');
		checkboxes.forEach(function(checkbox) {
			checkbox.addEventListener('change', function() {
				var anyChecked = false;
				checkboxes.forEach(function(checkbox) {
					if (checkbox.checked) {
						anyChecked = true;
					}
				});
				JSDialog.enableButtonInModal(dialogId, dialogId + '-response', anyChecked);
			});
		});
	},

	hidePage: function (tabNumber) {
		if (this.getDocType() === 'spreadsheet' && app.calc.getVisiblePartCount() > 1) {
			var argument = {nTabNumber: {type: 'int16', value: tabNumber}};
			app.socket.sendMessage('uno .uno:Hide ' + JSON.stringify(argument));
		}
	},

	hideSlide: function() {
		for (let i = 0; i < app.impress.partList.length; i++) {
			if (app.impress.partList[i].selected) {
				app.impress.partList[i].visible = 0;
				window.L.DomUtil.addClass(this._docLayer._preview._previewTiles[i], 'hidden-slide');
			}
		}

		app.socket.sendMessage('uno .uno:HideSlide');
		this.fire('toggleslidehide');
	},

	showSlide: function() {
		for (let i = 0; i < app.impress.partList.length; i++) {
			if (app.impress.partList[i].selected) {
				app.impress.partList[i].visible = 1;
				window.L.DomUtil.removeClass(this._docLayer._preview._previewTiles[i], 'hidden-slide');
			}
		}

		app.socket.sendMessage('uno .uno:ShowSlide');
		this.fire('toggleslidehide');
	},

	getNumberOfParts: function () {
		return this._docLayer._parts;
	},

	getCurrentPartNumber: function () {
		return this._docLayer._selectedPart;
	},

	getDocSize: function () {
		return this._docLayer._docPixelSize;
	},

	getDocType: function () {
		if (!this._docLayer)
			return null;

		return this._docLayer._docType;
	},

	isPresentationOrDrawing: function () {
		return this.getDocType() === 'presentation' || this.getDocType() === 'drawing';
	},

	isText: function () {
		return this.getDocType() === 'text';
	},
});
