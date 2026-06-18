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

/* global app $ cool TileManager ViewLayoutBase ViewLayoutFileBased */

// Single source of truth for Impress view modes. Keyed by the core
// 'contextchange' context. 'mode' is the persisted string, 'uno' enters the
// mode, 'activeMode' is the app.activeDocument.activeModes number. Note: the
// Master activeMode (1) is applied from the .uno:SlideMasterPage state change
// in Map.StateChanges.js, not here.
const IMPRESS_VIEW_MODES = {
	DrawPage:   { mode: 'normal', uno: '.uno:NormalMultiPaneGUI', activeMode: 0 },
	NotesPage:  { mode: 'notes',  uno: '.uno:NotesMode',          activeMode: 2 },
	MasterPage: { mode: 'master', uno: '.uno:SlideMasterPage',    activeMode: 1 },
};

window.L.ImpressTileLayer = window.L.CanvasTileLayer.extend({

	initialize: function (options) {
		window.L.CanvasTileLayer.prototype.initialize.call(this, options);
		// If this is mobile view, we we'll change the layout position of 'presentation-controls-wrapper'.
		if (window.mode.isSmallScreenDevice()) {
			this._putPCWOutsideFlex();
		}

		this._preview = window.L.control.partsPreview();

		// Vector rendered slide thumbnails.
		this._vectorThumbnails = cool.VectorRenderingConfig.isEnabled()
			? new cool.VectorThumbnailHandler(this)
			: null;

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
		// On small-screen devices and tablets, Permission.js sets the UI in read-only mode
		// until the user taps #mobile-edit-button. Start in fileBasedView so the user gets
		// endless slide scrolling while viewing.
		var mobileViewing =
			window.mode.isSmallScreenDevice() || window.mode.isTablet();
		if (app.file.readOnly || mobileViewing) app.file.fileBasedView = true;

		this._partHeightTwips = 0; // Single part's height.
		this._partWidthTwips = 0; // Single part's width. These values are equal to app.activeDocument.fileSize.x & app.activeDocument.fileSize.y when app.file.partBasedView is true.

		this._partDimensions = []; // Width & Height of all the parts
		this._fbCachedFileSize = null; // Cached filebased fileSize from the last status that carried partdimensions; used to prevent shrinking when a later message omits the field.

		// View-mode persistence state machine (see _handleViewModeState).
		this._viewModeRestored = false;
		this._restoringViewMode = false;
		this._lastReportedViewMode = null;

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
		const viewMode = IMPRESS_VIEW_MODES[newContext];
		const isDrawOrNotesPage = newContext === 'DrawPage' || newContext === 'NotesPage';

		if (isDrawOrNotesPage)
			app.impress.notesMode = newContext === 'NotesPage';

		if (app.map.uiManager.getCurrentMode() === 'notebookbar' && isDrawOrNotesPage) {
			const targetElement = document.getElementById('notesmode');
			// Guard rather than return: a missing button must not skip the
			// activeModes update, master handling or view-mode persistence below.
			if (targetElement)
				targetElement.classList.toggle('selected', newContext === 'NotesPage');
		}

		if (isDrawOrNotesPage) {
			app.activeDocument.activeModes = [viewMode.activeMode];
			TileManager.refreshTilesInBackground();
			TileManager.update();
		}

		if (newContext === 'MasterPage' || oldContext === 'MasterPage') {
			app.socket.sendMessage('status');
			this.invalidatePreviewsUponContextChange = true;
		}

		// Persist/restore the view mode for all three view contexts. Master
		// view's activeModes=[1] is set from the .uno:SlideMasterPage state
		// change (Map.StateChanges.js), so we don't touch activeModes here.
		if (viewMode)
			this._handleViewModeState(viewMode.mode);
	},

	// Restore the remembered view mode on first load, then report later user
	// switches so they are persisted per user per document. We key off the
	// first authoritative view context change ('normal'|'notes'|'master'),
	// which means the view is live and a UNO mode command will take effect.
	_handleViewModeState: function(mode) {
		if (!this._viewModeRestored) {
			this._viewModeRestored = true;
			this._lastReportedViewMode = mode;

			const saved = app.impress.savedViewMode;
			// Documents always open in Normal, so restoring only ever enters a
			// mode. Master view is an editing context, so skip restoring it on
			// read-only documents - the UNO command would no-op, no echo would
			// arrive, and the restore guard would stay stuck.
			const canRestore = saved !== 'master' || !app.file.readOnly;
			const target = Object.values(IMPRESS_VIEW_MODES).find((v) => v.mode === saved);
			if (target && saved !== mode && canRestore) {
				// Apply once; the resulting context change is our own echo and
				// must not be reported back as a user action.
				this._restoringViewMode = true;
				this._lastReportedViewMode = saved;
				app.map.sendUnoCommand(target.uno);
			}
			return;
		}

		if (this._restoringViewMode) {
			this._restoringViewMode = false;
			return;
		}

		if (mode !== this._lastReportedViewMode) {
			this._lastReportedViewMode = mode;
			// updateviewmode persists the choice through the WOPI host. The
			// native app builds have no WOPI host and no handler for it, so
			// the bridge rejects it as an unknown command.
			if (!window.ThisIsAMobileApp)
				app.socket.sendMessage('updateviewmode mode=' + mode);
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
		// commentData.position (twips, top-left) lets the caller pin the new
		// comment to a specific document point; used by the PDF click-to-place
		// flow. Without it the marker lands at the current viewport top-left.
		// commentData.size (twips, [w,h]) further pins the marker rectangle
		// to a user-dragged area; without it the marker stays at the default
		// 566x566 twips placeholder.
		const anchorX = commentData.position
			? commentData.position[0]
			: app.activeDocument.activeLayout.viewedRectangle.x1;
		const anchorY = commentData.position
			? commentData.position[1]
			: app.activeDocument.activeLayout.viewedRectangle.y1;
		const w = commentData.size ? commentData.size[0] : 566;
		const h = commentData.size ? commentData.size[1] : 566;
		commentData.anchorPos = [anchorX, anchorY];
		commentData.rectangle = [anchorX, anchorY, w, h];

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
		map.on('commandstatechanged', this._onSlideSectionsCommandState, this);
		app.events.on('updatepermission', this.onUpdatePermission.bind(this));

		if (!map._docPreviews)
			map._docPreviews = {};

		map.uiManager.initializeSpecializedUI(this._docType);
	},

	// Sections arrive as a .uno:SlideSections state-change pushed by core, so
	// the panel stays consistent with the async UNO dispatch.
	_onSlideSectionsCommandState: function (e) {
		if (!e || e.commandName !== '.uno:SlideSections')
			return;

		var sections = e.state;
		if (typeof sections === 'string') {
			try {
				sections = JSON.parse(sections);
			} catch (ex) {
				console.warn('Failed to parse .uno:SlideSections state: ' + ex);
				sections = [];
			}
		}
		if (!Array.isArray(sections))
			sections = [];

		app.impress.sections = sections;
		this._map.fire('updatesections', { sections: sections });
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
		this._map.off('commandstatechanged', this._onSlideSectionsCommandState, this);
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

		// Mobile Impress starts in fileBasedView for endless slide scrolling
		// during the read-only UI phase. Flip back to partBasedView when the user enters
		// edit mode
		if (app.file.readOnly) return;
		var mobile = window.mode.isSmallScreenDevice() || window.mode.isTablet();
		if (!mobile) return;

		if (e.detail.perm === 'edit' && app.file.fileBasedView)
			this._switchToPartBasedView();
		else if (
			(e.detail.perm === 'readonly' || e.detail.perm === 'view') &&
			!app.file.fileBasedView
		)
			this._switchToFileBasedView();
	},

	_switchToPartBasedView: function () {
		app.file.fileBasedView = false;
		this._fbCachedFileSize = null;

		// Collapse the stacked canvas back to a single slide
		app.activeDocument.fileSize = new cool.SimplePoint(
			this._partWidthTwips,
			this._partHeightTwips,
		);
		app.activeDocument.swapLayout(new ViewLayoutBase());
		app.activeDocument.activeLayout.viewSize =
			app.activeDocument.fileSize.clone();
		this._updateMaxBounds(true, true);
	},

	// Total fileSize for filebased view: width = max width across parts (so
	// landscape pages aren't clipped by Leaflet's maxBounds), height = sum of
	// per-part heights plus _spaceBetweenParts between each pair.
	_computeFileBasedFileSize: function () {
		// Prefer per-part dimensions when available. _partWidthTwips and
		// _partHeightTwips reflect the current selected page (the caller in
		// _onStatusMsg overwrites them from statusJSON.width/height) and must
		// not seed maxWidth here - otherwise switching to a narrower page
		// would shrink fileSize.x and clip wider pages.
		if (this._partDimensions.length === this._parts && this._parts > 0) {
			var maxWidth = 0;
			var totalHeight = 0;
			for (var i = 0; i < this._parts; i++) {
				maxWidth = Math.max(maxWidth, this.getPartWidth(i));
				totalHeight += this.getPartHeight(i);
			}
			totalHeight += this._parts * this._spaceBetweenParts;
			this._fbCachedFileSize = new cool.SimplePoint(maxWidth, totalHeight);
			return this._fbCachedFileSize.clone();
		}
		if (this._fbCachedFileSize)
			return this._fbCachedFileSize.clone();
		// First message before partdimensions from statusMsg ever arrives.
		return new cool.SimplePoint(
			this._partWidthTwips,
			this._parts * (this._partHeightTwips + this._spaceBetweenParts),
		);
	},

	_switchToFileBasedView: function () {
		// fileSize stays as the canonical "total document extent" in twips for
		// the consumers that still read it directly. The new ViewLayoutFileBased
		// owns its own viewSize, computed from the per-part rectangles.
		app.activeDocument.fileSize = this._computeFileBasedFileSize();
		app.activeDocument.swapLayout(new ViewLayoutFileBased());
		// Flip the flag after swapLayout so paint code never sees the flag true
		// while activeLayout still points at the previous ViewLayoutBase.
		app.file.fileBasedView = true;
		this._updateMaxBounds(true, true);
		TileManager.updateFileBasedView();
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

		if (values.type === 'vectortile') {
			if (this._vectorThumbnails) {
				this._vectorThumbnails.handleVectorTileResponse(values);
			}
			return;
		}

		if (values.type === 'vectorrenderinggraphics') {
			if (this._vectorThumbnails) {
				this._vectorThumbnails.handleVectorRenderingGraphicsResponse(values);
			}
			return;
		}

		if (values.comments) {
			var comments = Array.isArray(values.comments)
				? values.comments
				: Object.values(values.comments);
			comments.forEach(function(comment) {
				comment.id = String(comment.id);
				if (comment.parentId !== undefined)
					comment.parentId = String(comment.parentId);
			});
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).importComments(comments);
		} else {
			window.L.CanvasTileLayer.prototype._onCommandValuesMsg.call(this, textMsg);
		}
	},

	_onSetPartMsg: function (textMsg) {
		var part = parseInt(textMsg.match(/\d+/g)[0]);
		if (part !== this._selectedPart) {
			this._map.deselectAll(); // Deselect all first. This is a single selection.
			this._map.setPart(part, true);
		}
		// Fire 'setpart' even when the local _selectedPart was already updated
		// synchronously by Parts.js setPart (fileBasedView path), so listeners
		// waiting on server confirmation are not stuck.
		this._map.fire('setpart', {
			selectedPart: this._selectedPart,
			parts: this._parts,
			docType: this._docType
		});
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
				// Rebuild the full filebased fileSize: max width across parts
				// and the stacked total height. Both must be set before
				// _updateMaxBounds below so Leaflet's max bounds cover the
				// widest page.
				app.activeDocument.fileSize = this._computeFileBasedFileSize();
			}

			app.activeDocument.activeLayout.viewSize = app.activeDocument.fileSize.clone();

			// Rebuild the per-part rectangles in the filebased layout whenever
			// part dimensions arrive (or any status update reshapes the parts).
			if (
				app.file.fileBasedView &&
				app.activeDocument.activeLayout.type === 'ViewLayoutFileBased'
			) {
				app.activeDocument.activeLayout.reset();
			}

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

				this._map.fire('updateparts', {
					selectedPart: this._selectedPart,
					parts: this._parts,
					docType: this._docType
				});

				if (refreshAnnotation)
					app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');

				// Fetch slide sections data
				app.socket.sendMessage('getslidesections');
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
