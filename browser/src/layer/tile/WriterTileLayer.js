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
 * Writer tile layer is used to display a text document
 */

/* global app GraphicSelection cool TileManager */
window.L.WriterTileLayer = window.L.CanvasTileLayer.extend({

	newAnnotation: function (commentData) {
		const name = cool.Comment.makeName(commentData);
		const comment = new cool.Comment(name, commentData, {}, app.sectionContainer.getSectionWithName(app.CSections.CommentList.name));

		if (app.file.textCursor.visible) {
			comment.sectionProperties.data.anchorPos = [app.file.textCursor.rectangle.x2, app.file.textCursor.rectangle.y1];
		} else if (GraphicSelection.hasActiveSelection()) {
			// An image is selected, then guess the anchor based on the graphic selection.
			comment.sectionProperties.data.anchorPos = [GraphicSelection.rectangle.x1, GraphicSelection.rectangle.y2];
		}

		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).add(comment);
		app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).modify(comment);
	},

	beforeAdd: function (map) {
		map.on('commandstatechanged', this._onCommandStateChanged, this);
		map.uiManager.initializeSpecializedUI('text');
	},

	_onCommandStateChanged: function (e) {
		if (e.commandName === 'CompareDocumentsProperties') {
			if (e.state) {
				app.writer.compareDocumentProperties = e.state;
			}
		}
	},

	_onCommandValuesMsg: function (textMsg) {
		var braceIndex = textMsg.indexOf('{');
		if (braceIndex < 0) {
			return;
		}

		var values = JSON.parse(textMsg.substring(braceIndex));
		if (!values) {
			return;
		}

		if (values.comments) {
			values.comments.forEach(function(comment) {
				comment.id = comment.id.toString();
				comment.parent = comment.parentId.toString();
			});
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).importComments(values.comments);
			app.map.fire('importannotations');
		}
		else if (values.redlines && values.redlines.length > 0) {
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).importChanges(values.redlines);
		}
		else if (this._map.zotero && values.userDefinedProperties) {
			this._map.zotero.handleCustomProperty(values.userDefinedProperties);
		}
		else if (this._map.zotero && values.fields) {
			this._map.zotero.onFieldValue(values.fields);
		} else if (this._map.zotero && values.field) {
			this._map.zotero.handleFieldUnderCursor(values.field);
		} else if (this._map.zotero && values.setRefs) {
			this._map.zotero.onFieldValue(values.setRefs);
		} else if (this._map.zotero && values.setRef) {
			this._map.zotero.handleFieldUnderCursor(values.setRef);
		} else if (this._map.zotero && values.bookmarks) {
			this._map.zotero.handleBookmark(values.bookmarks);
		} else if (this._map.zotero && values.bookmark) {
			this._map.zotero.fetchCustomProperty(values.bookmark.name);
		} else if (this._map.zotero && values.sections) {
			this._map.zotero.onFieldValue(values.sections);
		} else {
			window.L.CanvasTileLayer.prototype._onCommandValuesMsg.call(this, textMsg);
		}
	},

	_shouldIgnoreServerPageSync: function () {
		return !this._map.isEditMode() && !app.file.textCursor.visible;
	},

	_onSetPartMsg: function (textMsg) {
		if (this._shouldIgnoreServerPageSync()) {
			return;
		}

		var part = parseInt(textMsg.match(/\d+/g)[0]);
		if (part !== this._currentPage) {
			this._currentPage = part;
			this._map.fire('pagenumberchanged', {
				currentPage: part,
				pages: this._pages,
				docType: this._docType
			});
		}
	},

	_setNewSize: function (/*cool.SimplePoint*/ size) {
		app.activeDocument.fileSize = size;
		app.activeDocument.activeLayout.viewSize = app.activeDocument.fileSize.clone();
		this._updateMaxBounds(true);
	},

	_releaseReconnectFileSize: function () {
		this._setNewSize(this._reconnectFileSize);
		this._reconnectFileSize = null;
		this._reconnectFileSizeTimer = null;
	},

	_onStatusMsg: function (textMsg) {
		const statusJSON = JSON.parse(textMsg.replace('status:', '').replace('statusupdate:', ''));

		if (app.socket._reconnecting) {
			// persist cursor position on reconnection
			// In writer, core always sends the cursor coordinates
			// of the first paragraph of the document so we want to ignore that
			// to eliminate document jumping while reconnecting
			this.persistCursorPositionInWriter = true;
			if (this.lastCursorPos) {
				// Save position to restore when we have the full layout of the document back
				this._savedCursorPos = this.lastCursorPos.clone();
			}
		}
		if (!statusJSON.width || !statusJSON.height || this._documentInfo === textMsg)
			return;

		if (statusJSON.readonly && !this._documentInfo)
			this._map.setPermission('readonly');

		var sizeChanged = statusJSON.width !== app.activeDocument.fileSize.x || statusJSON.height !== app.activeDocument.fileSize.y;
		if (sizeChanged && (app.socket._reconnecting || this._reconnectFileSize)) {
			if (this._reconnectFileSizeTimer)
				clearTimeout(this._reconnectFileSizeTimer);
			if (statusJSON.width >= app.activeDocument.fileSize.x && statusJSON.height >= app.activeDocument.fileSize.y) {
				this._reconnectFileSize = null;
			} else {
				// Suppress shrinking sizes during reconnection incremental reload
				// The timer avoids this supression being permanent
				sizeChanged = false;
				this._reconnectFileSize = new cool.SimplePoint(statusJSON.width, statusJSON.height);
				this._reconnectFileSizeTimer = setTimeout(this._releaseReconnectFileSize.bind(this), 5000);
			}
		}

		if (statusJSON.viewid !== undefined) {
			this._viewId = statusJSON.viewid;
			app.activeDocument.setActiveViewID(this._viewId);
		}

		if (statusJSON.partHasComments !== undefined &&  statusJSON.partHasComments !== app.activeDocument.partHasComments) {
			const hadValue = app.activeDocument.partHasComments !== undefined;
			app.activeDocument.partHasComments = statusJSON.partHasComments;
			// Only re-fit zoom when comment presence genuinely
			// changes (added or removed), not on the first status
			// message where it goes from undefined to a real value.
			if (hadValue)
				this._fitWidthZoom();
		}

		console.assert(this._viewId >= 0, 'Incorrect viewId received: ' + this._viewId);

		if (sizeChanged) {
			this._docType = statusJSON.type;
			this._setNewSize(new cool.SimplePoint(statusJSON.width, statusJSON.height));
		}

		this._documentInfo = textMsg;
		this._selectedPart = 0;

		const mode = (statusJSON.mode !== undefined) ? statusJSON.mode : 0;

		if (mode === 2)
			app.activeDocument.activeModes = [1, 2];
		else
			app.activeDocument.activeModes = [mode];

		this._parts = 1;
		if (!this._shouldIgnoreServerPageSync()) {
			this._currentPage = statusJSON.selectedpart;
		}
		this._pages = statusJSON.partscount;
		app.file.writer.pageRectangleList = statusJSON.pagerectangles.slice(); // Copy the array.
		// Recalculate view layout so view size reflects the new pages.
		// Needed for ViewLayoutMultiPage where the viewSize setter is a no-op.
		if (app.activeDocument.activeLayout.type === 'ViewLayoutMultiPage')
			app.activeDocument.activeLayout.reset();
		this._map.fire('pagenumberchanged', {
			currentPage: this._currentPage,
			pages: this._pages,
			docType: this._docType
		});
		TileManager.resetPreFetching(true);

		if (this._savedCursorPos && this._savedCursorPos.center[0] <= statusJSON.width && this._savedCursorPos.center[1] <= statusJSON.height) {
			this._postMouseEvent('buttondown', this._savedCursorPos.center[0], this._savedCursorPos.center[1], 1, 1, 0);
			this._postMouseEvent('buttonup', this._savedCursorPos.center[0], this._savedCursorPos.center[1], 1, 1, 0);
			this._savedCursorPos = null;
		}
	},
});
