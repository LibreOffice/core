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
 * Impress tile layer is used to display a presentation document
 */

/* global app $ cool TileManager */

window.L.ImpressTileLayer = window.L.CanvasTileLayer.extend({

	initialize: function (options) {
		window.L.CanvasTileLayer.prototype.initialize.call(this, options);
		// If this is mobile view, we we'll change the layout position of 'presentation-controls-wrapper'.
		if (window.mode.isSmallScreenDevice()) {
			this._putPCWOutsideFlex();
		}

		this._preview = window.L.control.partsPreview();

		if (window.mode.isSmallScreenDevice()) {
			this._addButton = window.L.control.mobileSlide();
			window.L.DomUtil.addClass(window.L.DomUtil.get('mobile-edit-button'), 'impress');
		}
		this._spaceBetweenParts = 300; // In twips. This is used when all parts of an Impress or Draw document is shown in one view (like a Writer file). This mode is used when document is read only.

		// app.file variable should exist, this is a precaution.
		if (!app.file)
			app.file = {};

		// Before this instance is created, app.file.readOnly and app.file.editComments variables are set.
		// If document is on read only mode, we will draw all parts at once.
		// Let's call default view the "part based view" and new view the "file based view".
		if (app.file.readOnly)
			app.file.fileBasedView = true;
		else
			app.file.partBasedView = true; // For Writer and Calc, this one should always be "true".

		this._partHeightTwips = 0; // Single part's height.
		this._partWidthTwips = 0; // Single part's width. These values are equal to app.activeDocument.fileSize.x & app.activeDocument.fileSize.y when app.file.partBasedView is true.

		this._partDimensions = []; // Width & Height of all the parts

		app.events.on('contextchange', this._onContextChange.bind(this));
	},

	_onContextChange(e) {
		/*
			We need to check the context content for now. Because we are using this property for both context and the page kind.
			When user modifies the content of the notes, the context is changed again. As we use context as a view mode, we shouldn't change our variable in that case.
			We need to check if the context is something related to view mode or not.
			For a better solution, we need to send the page kinds along with status messages. Then we will check the page kind and set the notes view toggle accordingly.
		*/

		const newContext = e.detail.context;
		const oldContext = e.detail.oldContext;
		const isDrawOrNotesPage = ['DrawPage', 'NotesPage'].includes(newContext);

		if (isDrawOrNotesPage)
			app.impress.notesMode = newContext === 'NotesPage';

		if (app.map.uiManager.getCurrentMode() === 'notebookbar' && isDrawOrNotesPage) {
			const targetElement = document.getElementById('notesmode');
			if (!targetElement) return;

			if (newContext === 'NotesPage')
				targetElement.classList.add('selected');
			else
				targetElement.classList.remove('selected');
		}

		if (isDrawOrNotesPage) {
			const mode = newContext === 'NotesPage' ? 2 : 0;
			app.activeDocument.activeModes = [mode];
			TileManager.refreshTilesInBackground();
			TileManager.update();
		}

		if (newContext === 'MasterPage' || oldContext === 'MasterPage') {
			app.socket.sendMessage('status');
			this.invalidatePreviewsUponContextChange = true;
		}
	},

	_selectionContextChanged: function(newContext) {
		if (newContext === 'Mark objects')
			app.activeDocument.selectionMiddleware.activate();
		else if (newContext === '')
			app.activeDocument.selectionMiddleware.deactivate();
	},

	_isPCWInsideFlex: function () {
		var PCW = document.getElementById('main-document-content').querySelector('#presentation-controls-wrapper');
		return PCW ? true: false;
	},

	_putPCWOutsideFlex: function () {
		if (this._isPCWInsideFlex()) {
			var pcw = document.getElementById('presentation-controls-wrapper');
			if (pcw && pcw.parentNode) {
				pcw.parentNode.removeChild(pcw);  // Remove from its actual parent
				var frc = document.getElementById('main-document-content');
				frc.parentNode.insertBefore(pcw, frc.nextSibling);
			}
		}
	},

	_putPCWInsideFlex: function () {
		if (!this._isPCWInsideFlex()) {
			var pcw = document.getElementById('presentation-controls-wrapper');
			if (pcw) {
				var frc = document.getElementById('main-document-content');
				document.body.removeChild(pcw);

				document.getElementById('document-container').parentNode.insertBefore(pcw, frc.children[0]);
			}
		}
	},

	newAnnotation: function (commentData) {
		commentData.anchorPos = [app.activeDocument.activeLayout.viewedRectangle.x1, app.activeDocument.activeLayout.viewedRectangle.y1];
		commentData.rectangle = [app.activeDocument.activeLayout.viewedRectangle.x1, app.activeDocument.activeLayout.viewedRectangle.y1, 566, 566];

		commentData.parthash = app.impress.partList[this._selectedPart].hash;

		const name = cool.Comment.makeName(commentData);
		const comment = new cool.Comment(name, commentData, {}, app.sectionContainer.getSectionWithName(app.CSections.CommentList.name));

		var annotation = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).add(comment);
		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).modify(annotation);
	},

	beforeAdd: function (map) {
		this._map = map;
		map.addControl(this._preview);
		map.on('updateparts', this.onUpdateParts, this);
		app.events.on('updatepermission', this.onUpdatePermission.bind(this));

		if (!map._docPreviews)
			map._docPreviews = {};

		map.uiManager.initializeSpecializedUI(this._docType);
	},

	onResizeImpress: function () {
		window.L.DomUtil.updateElementsOrientation(['presentation-controls-wrapper', 'document-container', 'slide-sorter']);

		var mobileEditButton = document.getElementById('mobile-edit-button');

		if (window.mode.isSmallScreenDevice()) {
			if (window.L.DomUtil.isPortrait()) {
				this._putPCWOutsideFlex();
				if (mobileEditButton)
					mobileEditButton.classList.add('portrait');
			}
			else {
				this._putPCWInsideFlex();
				if (mobileEditButton)
					mobileEditButton.classList.remove('portrait');
			}
		}
		else {
			var container = window.L.DomUtil.get('main-document-content');// consider height of document area to calculate estimated height for slide-sorter
			var slideSorter = window.L.DomUtil.get('slide-sorter');
			var navigationOptions = window.L.DomUtil.get('navigation-options-wrapper');
			if (container && slideSorter && toolbar) {
				$(slideSorter).height($(container).height() - $(navigationOptions).height());
			}
		}
	},

	onRemove: function () {
		clearTimeout(this._previewInvalidator);
	},

	_openMobileWizard: function(data) {
		window.L.CanvasTileLayer.prototype._openMobileWizard.call(this, data);
	},

	onUpdateParts: function () {
		if (this._map.uiManager.isAnyDialogOpen()) // Need this check else dialog loses focus
			return;

		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).onPartChange();
	},

	onUpdatePermission: function (e) {
		if (window.mode.isSmallScreenDevice()) {
			if (e.detail.perm === 'edit') {
				this._addButton.addTo(this._map);
			} else {
				this._addButton.remove();
			}
		}
	},

	_onCommandValuesMsg: function (textMsg) {
		try {
			var values = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
		} catch (e) {
			// One such case is 'commandvalues: ' for draw documents in response to .uno:AcceptTrackedChanges
			values = null;
		}

		if (!values) {
			return;
		}

		if (values.comments) {
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).importComments(values.comments);
		} else {
			window.L.CanvasTileLayer.prototype._onCommandValuesMsg.call(this, textMsg);
		}
	},

	_onSetPartMsg: function (textMsg) {
		var part = parseInt(textMsg.match(/\d+/g)[0]);
		if (part !== this._selectedPart) {
			this._map.deselectAll(); // Deselect all first. This is a single selection.
			this._map.setPart(part, true);
			this._map.fire('setpart', {selectedPart: this._selectedPart});
		}
	},

	_onStatusMsg: function (textMsg) {
		const statusJSON = JSON.parse(textMsg.replace('partstatus:', '').replace('status:', '').replace('statusupdate:', ''));

		// Since we have three status commands, remove them so we store and compare payloads only.
		textMsg = textMsg.replace('partstatus: ', '');
		textMsg = textMsg.replace('status: ', '');
		textMsg = textMsg.replace('statusupdate: ', '');
		if (statusJSON.width && statusJSON.height && this._documentInfo !== textMsg) {
			let dimensionsChanged = false;
			if (statusJSON.partdimensions) {
				const oldDims = this._partDimensions;
				this._partDimensions = [];
				for (let i = 0; i < statusJSON.partdimensions.length; i++) {
					this._partDimensions.push(new cool.SimplePoint(statusJSON.partdimensions[i].width, statusJSON.partdimensions[i].height));
				}
				if (!oldDims || oldDims.length !== this._partDimensions.length) {
					dimensionsChanged = true;
				} else {
					for (let i = 0; i < oldDims.length; i++) {
						if (oldDims[i].x !== this._partDimensions[i].x ||
							oldDims[i].y !== this._partDimensions[i].y) {
							dimensionsChanged = true;
							break;
						}
					}
				}
			}

			if (statusJSON.readonly && !this._documentInfo)
				this._map.setPermission('readonly');

			app.activeDocument.fileSize = new cool.SimplePoint(statusJSON.width, statusJSON.height);

			this._docType = statusJSON.type;
			if (this._docType === 'drawing') {
				window.L.DomUtil.addClass(window.L.DomUtil.get('presentation-controls-wrapper'), 'drawing');
			}
			this._parts = statusJSON.partscount;
			this._partHeightTwips = app.activeDocument.fileSize.y;
			this._partWidthTwips = app.activeDocument.fileSize.x;

			if (app.file.fileBasedView) {
				let totalHeight = 0; // Total height in twips.
				if (this._partDimensions.length === this._parts) {
					for (let i = 0; i < this._parts; i++) {
						totalHeight += this.getPartHeight(i);
					}
				}
				else
					totalHeight = this._parts * app.activeDocument.fileSize.y;
				totalHeight += (this._parts) * this._spaceBetweenParts; // Space between parts.
				app.activeDocument.fileSize.y = totalHeight;
			}

			app.activeDocument.activeLayout.viewSize = app.activeDocument.fileSize.clone();

			let allPagesResized = !statusJSON.currentpageresized;
			this._updateMaxBounds(true, allPagesResized);

			this._viewId = statusJSON.viewid;
			app.activeDocument.setActiveViewID(this._viewId);
			console.assert(this._viewId >= 0, 'Incorrect viewId received: ' + this._viewId);
			if (app.socket._reconnecting) {
				app.socket.sendMessage('setclientpart part=' + this._selectedPart);
			} else {
				this._selectedPart = statusJSON.selectedpart;
			}

			TileManager.resetPreFetching(true);

			// partdimensions is only included in statusJSON in case of current page resize or switching to a page having different size
			// The following statements seems unnecessary in those cases
			if (allPagesResized) {
				if (statusJSON.gridSnapEnabled === true)
					app.map.stateChangeHandler.setItemValue('.uno:GridUse', 'true');
				else if (statusJSON.parts.length > 0 && statusJSON.parts[0].gridSnapEnabled === true)
					app.map.stateChangeHandler.setItemValue('.uno:GridUse', 'true');

				if (statusJSON.gridVisible === true)
					app.map.stateChangeHandler.setItemValue('.uno:GridVisible', 'true');
				else if (statusJSON.parts.length > 0 && statusJSON.parts[0].gridVisible === true)
					app.map.stateChangeHandler.setItemValue('.uno:GridVisible', 'true');

				app.impress.partList = Object.assign([], statusJSON.parts);
				var refreshAnnotation = this._documentInfo !== textMsg;

				this._documentInfo = textMsg;

				const mode = (statusJSON.mode !== undefined) ? statusJSON.mode : (statusJSON.parts.length > 0 && statusJSON.parts[0].mode !== undefined ? statusJSON.parts[0].mode : 0);
				app.activeDocument.activeModes = [mode];
				this._map.fire('impressmodechanged', {mode: mode});

				this._map.fire('updateparts', {});

				if (refreshAnnotation)
					app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
			}

			this._documentInfo = textMsg;

			if (dimensionsChanged) {
				this._invalidateAllPreviews();
			}
		}

		if (app.file.fileBasedView)
			TileManager.updateFileBasedView();

		if (this.invalidatePreviewsUponContextChange === true) {
			this._invalidateAllPreviews();
			this.invalidatePreviewsUponContextChange = false;
		}
	},

	getPartDimensions: function(partIndex) {
		if (this._partDimensions && this._partDimensions[partIndex]) {
			return this._partDimensions[partIndex];
		}
		else {
			return app.activeDocument.fileSize;
		}
	},

	getPartWidth: function(partIndex) {
		const dimensions = this.getPartDimensions(partIndex);
		return dimensions.x;
	},

	getPartHeight: function(partIndex) {
		const dimensions = this.getPartDimensions(partIndex);
		return dimensions.y;
	},

	_invalidateAllPreviews: function () {
		window.L.CanvasTileLayer.prototype._invalidateAllPreviews.call(this);
		this._map.fire('invalidateparts');
	}
});
