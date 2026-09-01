/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * window.L.Map is the central class of the API - it is used to create a map.
 */

/* global app _ Cursor JSDialog RenderManager cool InternPointUtil */

window.L.Map = window.L.Evented.extend({

	statics: {
		THIS : undefined
	},

	options: {
		center: [0, 0],
		docParams: {},
		// Default zoom level in which the document will be loaded.
		zoom: 10,
		// These zoom values are on a logarithmic scale. Each step away from the default 10
		// (meaning 1 = 100%) is a multiplication by or division with pow(2,1/4). pow(2,1/4)
		// is approximately 1.2. Thus 4 corresponds to six steps of division by pow(2,1/4) =
		// 35%. 18 corresponds to 8 steps of multiplication by pow(2,1/4) = 400%. The
		// percentages available are then rounded to the nearest five percent.
		minZoom: 1,
		maxZoom: 18,
		fadeAnimation: false, // Not useful for typing.
		markerZoomAnimation: true,
		// defaultZoom:
		// The zoom level at which the tile size in twips equals the default size (3840 x 3840).
		// Unless you know what you are doing, this should not be modified.
		defaultZoom: 10,
		// 15 = 1440 twips-per-inch / 96 dpi.
		// Chosen to match previous hardcoded value of 3840 for
		// the current tile pixel size of 256.
		// Default tile width in twips (how much of the document is covered horizontally in a
		// 256x256 pixels tile). Unless you know what you are doing, this should not be modified;
		// this means twips value for 256 pixels at 96dpi.
		tileWidthTwips: window.tileSize * 15,
		// tileHeightTwips :
		// Default tile height in twips (how much of the document is covered vertically in a
		// 256x256 pixels tile).Unless you know what you are doing, this should not be modified;
		// this means twips value for 256 pixels at 96dpi.
		tileHeightTwips: window.tileSize * 15,
		urlPrefix: 'cool',
		wopiSrc: '',
		cursorURL: app.LOUtil.getURL('cursors'),
		// cursorURL
		// The path (local to the server) where custom cursor files are stored.
	},

	// Control.UIManager instance, set in main.js
	uiManager: null,

	// Control.LokDialog instance, is set in Control.UIManager.ts
	dialog: null,

	// Control.JSDialog instance, is set in Control.UIManager.ts
	jsdialog: null,

	context: {context: ''},

	initialize: function (id, options) { // (HTMLElement or String, Object)
		window.L.Evented.prototype.initialize.call(this);
		options = window.L.setOptions(this, options);

		if (this.options.documentContainer) {
			// have it as DOM object
			this.options.documentContainer = window.L.DomUtil.get(this.options.documentContainer);
		}

		this._clip = window.L.clipboard(this);
		this._initContainer(id);
		this._initLayout();

		// Start with readonly toolbars on desktop
		if (window.mode.isDesktop()) {
			window.L.DomUtil.addClass(window.L.DomUtil.get('toolbar-wrapper'), 'readonly');
		}

		this._initEvents();
		this._cacheSVG = [];

		if (options.zoom !== undefined) {
			this._zoom = this._limitZoom(options.zoom);
		}

		if (options.center && options.zoom !== undefined) {
			// Establish the initial map state directly (was setView -> _resetView,
			// which ran before the doc layer/layout existed so its map events had no
			// listeners and its setViewRectangleFromPointAndScale was skipped). The
			// zoom is set just above; the first viewed rectangle and tile kickoff
			// come later from the doc layer's onAdd (applyZoom / _viewReset). Only
			// _loaded needs to be set here.
			this._loaded = true;
		}

		Cursor.imagePath = options.cursorURL;

		/* private members */
		this._handlers = [];
		this._layers = {};
		this._zoomBoundLayers = {};
		this._bDisableKeyboard = false;
		this._fatal = false;
		this._enabled = true;
		this._debugAlwaysActive = false; // disables the dimming / document inactivity when true
		this._disableDefaultAction = {}; // The events for which the default handler is disabled and only issues postMessage.
		this.showSidebar = false;
		this._previewQueue = [];
		this._previewRequestsOnFly = 0;
		this._timeToEmptyQueue = new Date();
		this._partsDirection = 1; // For pre-fetching the slides in the direction of travel.
		this._lockAccessibilityOn = false;
		// Focusing:
		//
		// Cursor is visible or hidden (e.g. for graphic selection).
		app.setCursorVisibility(true);
		// The ID of the window with focus. 0 for the document.
		this._winId = 0;
		// The object of the dialog, if any (must have .focus callable).
		this._activeDialog = null;
		// True only when searching within the doc, as we need to use winId==0.
		this._isSearching = false;

		this.callInitHooks();

		this.addHandler('keyboard', window.L.Map.Keyboard);

		this._addLayers(this.options.layers);
		app.socket = new app.definitions.Socket(this);

		this._progressBar = window.L.progressOverlay(cool.Point.toPoint(150, 25));

		this._debug = new app.DebugManager(this);

		this.on('modificationindicatorinitialized', function() {
			this._modIndicatorInitialized = true;
		});

		// When all these conditions are met, fire statusindicator:initializationcomplete
		this.initConditions = {
			'doclayerinit': false,
			'statusindicatorfinish': false,
			'StyleApply': false,
			'CharFontName': false,
			'updatepermission': false
		};
		this.initComplete = false;

		app.events.on('updatepermission', function(e) {
			if (!this.initComplete) {
				this._fireInitComplete('updatepermission');
			}

			if (e.detail.perm === 'readonly') {
				window.L.DomUtil.addClass(this._container.parentElement, 'readonly');
				if (window.mode.isDesktop() || window.mode.isTablet()) {
					window.L.DomUtil.addClass(window.L.DomUtil.get('toolbar-wrapper'), 'readonly');
				}
				window.L.DomUtil.addClass(window.L.DomUtil.get('main-menu'), 'readonly');
				window.L.DomUtil.addClass(window.L.DomUtil.get('presentation-controls-wrapper'), 'readonly');
			} else {
				window.L.DomUtil.removeClass(this._container.parentElement, 'readonly');
				if (window.mode.isDesktop() || window.mode.isTablet()) {
					window.L.DomUtil.removeClass(window.L.DomUtil.get('toolbar-wrapper'), 'readonly');
				}
				window.L.DomUtil.removeClass(window.L.DomUtil.get('main-menu'), 'readonly');
				window.L.DomUtil.removeClass(window.L.DomUtil.get('presentation-controls-wrapper'), 'readonly');
			}
		}.bind(this));

		this.on('doclayerinit', function() {
			if (!this.initComplete) {
				this._fireInitComplete('doclayerinit');
			}

			if (window.mode.isSmallScreenDevice())
			{
				document.getElementById('document-container').classList.add('mobile');
				this.showCalcInputBar();
			}
		});
		this.on('updatetoolbarcommandvalues', function(e) {
			if (this.initComplete) {
				return;
			}
			if (e.commandName === '.uno:StyleApply') {
				this._fireInitComplete('StyleApply');
			}
			else if (e.commandName === '.uno:CharFontName') {
				this._fireInitComplete('CharFontName');
			}
		});
		if (window.ThisIsTheAndroidApp) {
			this.on('readonlymode', function() {
				this.setPermission('readonly');
			});
			this.on('editmode', function() {
				app.setPermission('edit');
				this._switchToEditMode();
			});
			this.on('hardwarekeyboardattached', function() {
				window.keyboard.setHardwareKeyboardAttached(true);
				if (this.isEditMode())
					this.focus(true);
			});
			this.on('hardwarekeyboarddetached', function() {
				window.keyboard.setHardwareKeyboardAttached(false);
			});
			this.on('darkthemeactivated', function() {
				this.uiManager.applyDarkMode(true, false);
			});
			this.on('lightthemeactivated', function() {
				this.uiManager.applyDarkMode(false, false);
			});
		}

		this.showBusy(_('Initializing...'), false);
		this.on('statusindicator', this._onUpdateProgress, this);

		this.on('editorgotfocus', this._onEditorGotFocus, this);

		// Fired to signal that the input focus is being changed.
		this.on('changefocuswidget', this._onChangeFocusWidget, this);

		this.on('searchstart', this._onSearchStart, this);

		// View info (user names and view ids)
		this._viewInfo = {};
		this._viewInfoByUserName = {};

		// View color map
		this._viewColors = {};

		// This becomes true if document was ever modified by the user
		this._everModified = false;

		// This is the new file name, if the document is renamed, which is used on uno:Save's result.
		this._renameFilename = '';

		// Document is completely loaded or not
		this._docLoaded = false;

		// Unlike _docLoaded, this is flagged only once,
		// after we receive status for the first time.
		this._docLoadedOnce = false;

		//Last modified time of document saved state
		this._lastModDateValue = '';

		// Last part for which Doc_PartChanged was fired
		this._lastPart = -1;
		this._lastPartCount = -1;
		this._lastPartDocType = '';

		var fireDocPartChanged = function(part, partCount, docType) {
			var normalizedPart = Number(part);
			var normalizedPartCount = Number(partCount);

			if (!Number.isInteger(normalizedPart) || !Number.isInteger(normalizedPartCount)) {
				return;
			}

			if (normalizedPart !== this._lastPart || normalizedPartCount !== this._lastPartCount || docType !== this._lastPartDocType) {
				this._lastPart = normalizedPart;
				this._lastPartCount = normalizedPartCount;
				this._lastPartDocType = docType;
				this.fire('postMessage', {msgId: 'Doc_PartChanged', args: { Part: normalizedPart + 1, PartCount: normalizedPartCount, DocType: docType }});
			}
		}.bind(this);

		this.on('pagenumberchanged', function(e) {
			if (!e) {
				return;
			}

			fireDocPartChanged(e.currentPage, e.pages, e.docType);
		}, this);

		this.on('setpart', function(e) {
			if (!e || e.selectedPart === undefined || e.parts === undefined || e.docType === undefined) {
				return;
			}

			fireDocPartChanged(e.selectedPart, e.parts, e.docType);
		}, this);

		this.on('updateparts', function(e) {
			if (!e || e.selectedPart === undefined || e.parts === undefined || e.docType === undefined) {
				return;
			}

			fireDocPartChanged(e.selectedPart, e.parts, e.docType);
		}, this);

		this.on('commandstatechanged', function(e) {
			if (e.commandName === '.uno:ModifiedStatus') {
				app.file.modified = (e.state === 'true');
				this._everModified = this._everModified || app.file.modified;

				// Fire an event to let the client know whether the document needs saving or not.
				this.fire('postMessage', {msgId: 'Doc_ModifiedStatus', args: { Modified: e.state === 'true' }});

				if (this._everModified) {
					this.fire('updatemodificationindicator', { status: e.state === 'true' ? 'MODIFIED' : 'SAVED' });
				}
			}
		}, this);

		this.on('commandresult', function(e) {
			if (e.commandName === '.uno:Save' && this.saveState) {
				if (e.success)
					this.saveState.showSavedStatus();
				else
					this.saveState.showSaveFailedStatus();
			}
		}, this);

		this.on('commandvalues', function(e) {
			if (e.commandName === '.uno:LanguageStatus' && app.util.isArray(e.commandValues)) {
				app.languages = [];
				e.commandValues.forEach(function(language) {
					var split = language.split(';');
					language = split[0];
					var code = '';
					if (split.length > 1)
						code = split[1];
					app.languages.push({translated: _(language), neutral: language, iso: code});
				});
				app.languages.sort(function(a, b) {
					return a.translated < b.translated ? -1 : a.translated > b.translated ? 1 : 0;
				});
				this.fire('languagesupdated');
			}
		});

		this.on('docloaded', function(e) {
			if (this.options.debug && !this._debug.debugOn)
				this._debug.toggle();

			this._docLoaded = e.status;
			if (this._docLoaded) {
				app.idleHandler.notifyActive();
				app.dispatcher = new app.definitions['dispatcher']();
				if (!document.hasFocus()) {
					this.fire('editorgotfocus');
					this.focus();
				}
				app.idleHandler._activate();
				if (window.ThisIsTheAndroidApp) {
					window.postMobileMessage('hideProgressbar');
				}

				app.activeDocument.activeLayout.sendClientVisibleArea(true);
				app.serverConnectionService.onDocumentLoaded();
			} else if (this._docLayer && app.sectionContainer) {
				// remove the comments and changes
				var commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
				if (commentSection)
					commentSection.clearList();
			}

			if (!window.mode.isSmallScreenDevice())
				this.initializeModificationIndicator();

			// We have loaded.
			if (!this._docLoadedOnce) {
				this._docLoadedOnce = this._docLoaded;
			}
		}, this);

		this.fire('postMessage', {
			msgId: 'App_LoadingStatus',
			args: {
				Status: 'Initialized',
			}
		});
	},

	// A11y

	initTextInput: function(docType) {
		var hasAccessibilitySupport =
			window.enableAccessibility && window.prefs.getBoolean('accessibilityState');
		hasAccessibilitySupport = hasAccessibilitySupport &&
			(docType === 'text' || docType === 'presentation'|| docType === 'spreadsheet');

		this.setupCoreAccessibility(hasAccessibilitySupport);
		this.createTextInput(hasAccessibilitySupport);
	},

	createTextInput: function(enableA11y) {
		this._textInput = enableA11y ? window.L.a11yTextInput() : window.L.textInput();
		this.addLayer(this._textInput);
	},

	setupCoreAccessibility: function(enableA11y) {
		app.socket.sendMessage('a11ystate ' + enableA11y);
	},

	setAccessibilityState: function(enable) {
		if (window.prefs.getBoolean('accessibilityState') === enable)
			return;

		window.prefs.set('accessibilityState', enable);
		this.setupCoreAccessibility(enable);
		this.removeLayer(this._textInput);
		this.createTextInput(enable);
		this.fire('a11ystatechanged');

		if (enable)
			this._textInput._requestFocusedParagraph();
		this._textInput.showCursor();
	},

	lockAccessibilityOn: function() {
		this.setAccessibilityState(true);
		this._lockAccessibilityOn = true;
		this.fire('a11ystatechanged');
	},

	// end of A11y

	loadDocument: function(socket) {
		app.socket.connect(socket);
		if (this._clip)
			this._clip.clearSelection();
	},

	sendInitNotebookbarCommands: function() {
		app.socket.sendMessage('commandvalues command=.uno:LanguageStatus');
		this._docLayer._getToolbarCommandsValues();
	},

	sendInitUNOCommands: function() {
		// TODO: remove duplicated init code
		this.sendInitNotebookbarCommands();
		if (this._docLayer._docType === 'spreadsheet') {
			if (this._docLayer.options.sheetGeometryDataEnabled)
				this._docLayer.requestSheetGeometryData();
			this._docLayer.refreshViewData();
			RenderManager.update();
		}
		// For calc parsing this will need SheetGeometry, so send after
		// requesting that
		app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
	},

	// public methods that modify map state

	getViewId: function (username) {
		for (var idx in this._viewInfo) {
			if (this._viewInfo[idx].username === username) {
				return this._viewInfo[idx].id;
			}
		}
		return -1;
	},

	addView: function(viewInfo) {
		this._viewInfo[viewInfo.id] = viewInfo;
		if (viewInfo.userextrainfo !== undefined && viewInfo.userextrainfo.avatar !== undefined) {
			this._viewInfoByUserName[viewInfo.username] = viewInfo;
		}
		this.fire('postMessage', {msgId: 'View_Added', args: {Deprecated: true, ViewId: viewInfo.id, UserId: viewInfo.userid, UserName: viewInfo.username, UserExtraInfo: viewInfo.userextrainfo, Color: app.LOUtil.rgbToHex(viewInfo.color), ReadOnly: viewInfo.readonly}});

		// Fire last, otherwise not all events are handled correctly.
		this.fire('addview', {viewId: viewInfo.id, username: viewInfo.username, extraInfo: viewInfo.userextrainfo, readonly: this.isViewReadOnly(viewInfo.id)});

		this.updateAvatars();
	},

	removeView: function(viewid) {
		var username = this._viewInfo[viewid].username;
		delete this._viewInfoByUserName[this._viewInfo[viewid].username];
		delete this._viewInfo[viewid];
		this.fire('postMessage', {msgId: 'View_Removed', args: {Deprecated: true, ViewId: viewid}});

		// Fire last, otherwise not all events are handled correctly.
		this.fire('removeview', {viewId: viewid, username: username});
	},

	updateAvatars: function() {
		if (this._docLayer && this._docLayer._annotations && this._docLayer._annotations._items) {
			for (var idxAnno in this._docLayer._annotations._items) {
				var annotation = this._docLayer._annotations._items[idxAnno];
				var username = annotation._data.author;
				if (this._viewInfoByUserName[username])
					annotation._data.avatar = this._viewInfoByUserName[username].userextrainfo.avatar;
				annotation._updateContent();
			}
		}
	},

	initializeModificationIndicator: function() {
		this.fire('initmodificationindicator', this._lastmodtime);
		this.updateModificationIndicator(this._lastmodtime);
	},

	updateModificationIndicator: function(newModificationTime) {
		var timeout;

		if (typeof newModificationTime === 'string') {
			this._lastmodtime = newModificationTime;
		}

		clearTimeout(this._modTimeout);

		if (this._modIndicatorInitialized && this._lastmodtime) {
			var dateTime = new Date(this._lastmodtime.replace(/,.*/, 'Z'));
			var dateValue;

			var elapsed = Date.now() - dateTime;
			var rtf1 = new Intl.RelativeTimeFormat(String.locale, { style: 'narrow' });
			if (('minSavedMessageTimeoutSecs' in window) && (elapsed < (window.minSavedMessageTimeoutSecs * 1000))) {
				timeout = window.minSavedMessageTimeoutSecs * 1000;
				dateValue = '';
			} else if (elapsed < 60000) {
				dateValue = _('Last saved:') + ' ' + rtf1.format(-Math.round(elapsed / 1000), 'second');
				timeout = 6000;
			} else if (elapsed < 3600000) {
				dateValue = _('Last saved:') + ' ' + rtf1.format(-Math.round(elapsed / 60000), 'minute');
				timeout = 60000;
			} else if (elapsed < 3600000 * 24) {
				dateValue = _('Last saved:') + ' ' + rtf1.format(-Math.round(elapsed / 3600000), 'hour');
				timeout = 60000;
			} else {
				dateValue = _('Last saved:') + ' ' + dateTime.toLocaleDateString(String.locale,
					{ year: 'numeric', month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit' });
				timeout = 60000;
			}

			this.fire('updatemodificationindicator', {lastSaved: dateValue});

			if (timeout) {
				this._modTimeout = setTimeout(window.L.bind(this.updateModificationIndicator, this, -1), timeout);
			}
		}
		if (this.lastModIndicator !== null && this.lastModIndicator !== undefined)
			this.lastModIndicator.innerHTML = dateValue;
		this.setLastModDateValue(dateValue);
		this._modTimeout = setTimeout(window.L.bind(this.updateModificationIndicator, this, -1), timeout);
	},

	setLastModDateValue: function(dateValue) {
		this._lastModDateValue = dateValue;
	},

	getLastModDateValue: function() {
		return this._lastModDateValue;
	},

	showBusy: function(label, bar) {
		if (window.ThisIsTheAndroidApp)
			return;

		// If document is already loaded, ask the toolbar widget to show busy
		// status on the bottom statusbar
		if (this._docLayer) {
			this.fire('showbusy', {label: label});
			return;
		}
		this._progressBar.delayedStart(this, label, bar);
	},

	hideBusy: function () {
		if (window.ThisIsTheAndroidApp)
			return;

		this.fire('hidebusy');
		this._progressBar.end(this);
	},

	zoomToFactor: function (zoom) {
		return Math.pow(InternPointUtil.SCALE, (zoom - this.options.zoom));
	},

	ignoreCursorUpdate: function () {
		return this._ignoreCursorUpdate;
	},

	enableTextInput: function () {
		this._setTextInputState(true /* enable? */);
	},

	disableTextInput: function () {
		this._setTextInputState(false /* enable? */);
	},

	_setTextInputState: function (enable) {
		var docLayer = this._docLayer;
		if (!docLayer)
			return;
		this._ignoreCursorUpdate = !enable;

		if (!app.file.textCursor.visible)
			return;

		if (!enable) {
			this._textInput.disable();
		} else {
			this._textInput.enable();
			docLayer._updateCursorPos();
		}
	},

	// TODO handler.addTo
	addHandler: function (name, HandlerClass) {
		if (!HandlerClass) { return this; }

		var handler = this[name] = new HandlerClass(this);

		this._handlers.push(handler);

		if (this.options[name]) {
			handler.enable();
		}

		// Check for the special proof-of-concept case where no WOPI is involved but we
		// still run COOL in an iframe of its own and thus need to receive the
		// postMessage things.
		if (name === 'wopi' && this.options['notWopiButIframe']) {
			handler.addHooks();
		}

		return this;
	},

	remove: function () {

		this._initEvents(true);

		try {
			// throws error in IE6-8
			delete this._container._leaflet;
		} catch (e) {
			this._container._leaflet = undefined;
		}

		window.L.DomUtil.remove(this._mapPane);

		if (this._clearControlPos) {
			this._clearControlPos();
		}

		this._clearHandlers();

		if (this._loaded) {
			this.fire('unload');
		}

		if (this._docLayer) {
			this.removeLayer(this._docLayer);
		}
		app.socket.close();
		return this;
	},

	createPane: function (name, container) {
		var className = 'leaflet-pane' + (name ? ' leaflet-' + name.replace('Pane', '') + '-pane' : ''),
		    pane = window.L.DomUtil.create('div', className, container || this._mapPane);

		if (name) {
			this._panes[name] = pane;
		}
		return pane;
	},


	// public methods for getting map state

	hasInfoForView: function(viewid)  {
		return (viewid in this._viewInfo);
	},

	getViewName: function(viewid) {
		if (this._viewInfo[viewid] !== undefined)
			return this._viewInfo[viewid].username;
		else return null;
	},

	getViewColor: function(viewid) {
		return this._viewInfo[viewid].color;
	},

	isViewReadOnly: function(viewid) {
		return this._viewInfo[viewid].readonly !== '0';
	},

	getZoom: function () {
		return this._zoom;
	},

	getMinZoom: function () {
		return this.options.minZoom === undefined ? this._layersMinZoom || 0 : this.options.minZoom;
	},

	getMaxZoom: function () {
		return this.options.maxZoom === undefined ?
			(this._layersMaxZoom === undefined ? Infinity : this._layersMaxZoom) :
			this.options.maxZoom;
	},

	getPane: function (pane) {
		return typeof pane === 'string' ? this._panes[pane] : pane;
	},

	getContainer: function () {
		return this._container;
	},

	// We have one global winId that controls what window (dialog, sidebar, or
	// the main document) has the actual focus.  0 means the document.
	setWinId: function (id) {
		// window.app.console.log('winId set to: ' + id);
		if (typeof id === 'string')
			id = parseInt(id);
		this._winId = id;
	},

	// Getter for the winId, see setWinId() for more.
	getWinId: function () {
		if (this.formulabar && this.formulabar.hasFocus())
			return 0;
		return this._winId;
	},

	// Returns true iff the document has input focus,
	// as opposed to a dialog, sidebar, formula bar, etc.
	editorHasFocus: function () {
		return this.getWinId() === 0 && !this.calcInputBarHasFocus() && !this._iframeDialog;
	},

	// Returns true iff the formula-bar has the focus.
	calcInputBarHasFocus: function () {
		return this.formulabar && this.formulabar.hasFocus();
	},

	// Accepts any pending cell edit, whether it is happening in the formula
	// bar or directly in the cell. app.file.textCursor.visible mirrors the
	// core's cursorvisible: message, which is set for both; the formula-bar
	// checks catch a focused-but-not-yet-typed-into formula bar as well.
	acceptPendingCellEdit: function () {
		if (
			this.getDocType() === 'spreadsheet' &&
			(app.file.textCursor.visible ||
				(this.formulabar &&
					(this.formulabar.hasFocus() ||
						this.formulabar.isInEditMode())))
		)
			app.dispatcher.dispatch('acceptformula');
	},

	// Give the focus to the text input.
	// @acceptInput (only on "mobile" (= mobile phone) or on iOS and Android in general) true if we want to
	// accept key input, and show the virtual keyboard.
	focus: function (acceptInput) {
		if (this._textInput)
			this._textInput.focus(acceptInput);
	},

	// just set the keyboard state for mobile
	// we don't want to change the focus, we know that keyboard is closed
	// and we are just setting the state here
	setAcceptInput: function (acceptInput) {
		this._textInput._setAcceptInput(acceptInput);
	},

	// Lose focus to stop accepting keyboard input.
	// On mobile, it will hide the virtual keyboard.
	blur: function () {
		this._textInput.blur();
	},

	hasFocus: function () {
		return this._textInput && document.activeElement === this._textInput.activeElement();
	},

	// Returns true iff the textarea is enabled and we focused on it.
	// On mobile, this signifies that the keyboard should be visible.
	canAcceptKeyboardInput: function() {
		return this._textInput.canAcceptKeyboardInput();
	},

	isSearching: function() {
		return this._isSearching;
	},

	_fireInitComplete: function (condition) {
		if (this.initComplete) {
			return;
		}

		this.initConditions[condition] = true;
		for (var key in this.initConditions) {
			if (!this.initConditions[key]) {
				return;
			}
		}
		this.fire('statusindicator', {statusType: 'initializationcomplete'});
		this.initComplete = true;

		window.L.DomUtil.addClass(this._container, 'initialized');
	},

	_initContainer: function (id) {
		var container = this._container = window.L.DomUtil.get(id);
		window.L.DomUtil.removeClass(this._container, 'initialized');

		if (!container) {
			throw new Error('Map container not found.');
		} else if (container._leaflet) {
			throw new Error('Map container is already initialized.');
		}

		this._fileDownloader = window.L.DomUtil.create('iframe', '', container);
		window.L.DomUtil.setStyle(this._fileDownloader, 'display', 'none');

		window.L.DomEvent.on(this._fileDownloader.contentWindow, 'contextmenu', window.L.DomEvent.preventDefault);
		window.L.DomEvent.addListener(container, 'scroll', this._onScroll, this);
		container._leaflet = true;
	},

	_onScroll: function() {
		this._container.scrollTop = 0;
		this._container.scrollLeft = 0;
	},

	_initLayout: function () {
		var container = this._container;

		this._fadeAnimated = this.options.fadeAnimation && window.L.Browser.any3d;

		window.L.DomUtil.addClass(container, 'leaflet-container' +
			(window.touch.hasAnyTouchscreen() ? ' leaflet-touch' : '') +
			(window.L.Browser.retina ? ' leaflet-retina' : '') +
			(window.L.Browser.safari ? ' leaflet-safari' : '') +
			(this._fadeAnimated ? ' leaflet-fade-anim' : ''));

		var position = window.L.DomUtil.getStyle(container, 'position');

		if (position !== 'absolute' && position !== 'relative' && position !== 'fixed') {
			container.style.position = 'absolute';
		}

		this._initPanes();

		if (this._initControlPos) {
			this._initControlPos();
		}
	},

	_initPanes: function () {
		this._panes = {};
		this._paneRenderers = {};

		this._mapPane = this.createPane('mapPane', this._container);

		this.createPane('overlayPane');
	},


	// private methods that modify map state

	_getZoomSpan: function () {
		return this.getMaxZoom() - this.getMinZoom();
	},

	// DOM event handling
	_mainEvents: function (onOff) {
		window.L.DomEvent[onOff](this._container, 'click dblclick mousedown mouseup ' +
			'mouseover mouseout mousemove dragover drop ' +
			'trplclick qdrplclick', window.touch.mouseOnly(this._handleDOMEvent), this);
	},

	_initEvents: function (remove) {
		if (!window.L.DomEvent) { return; }

		this._targets = {};

		this._mouseOut = false;

		var onOff = remove ? 'off' : 'on';

		this._mainEvents(onOff);

		window.L.DomEvent[onOff](window, 'blur', this._onLostFocus, this);
		window.L.DomEvent[onOff](window, 'focus', this._onGotFocus, this);
		// A reload, a link followed or a tab closed takes the page away without a
		// blur. pagehide is the signal all of those fire.
		window.L.DomEvent[onOff](window, 'pagehide', this._onPageHide, this);
	},


	showCalcInputBar: function() {
		if (this.formulabar)
			this.formulabar.showFormulabar();
	},

	// Change the focus to a dialog or editor.
	// @dialog is the instance of the dialog class.
	// @winId is the ID of the dialog/sidebar, or 0 for the editor.
	// @acceptInput iff defined, map.focus is called and passed to it.
	_changeFocusWidget: function (dialog, winId, acceptInput) {
		if (!this._loaded) { return; }

		this.setWinId(winId);
		this._activeDialog = dialog;
		this._isSearching = false;

		if (this.editorHasFocus()) {
			// The document has the focus.
			var doclayer = this._docLayer;
			if (doclayer)
				doclayer._updateCursorAndOverlay();
		} else if (acceptInput !== undefined) {
			// A dialog has the focus.
			this.focus(acceptInput);
			this._textInput.hideCursor(); // The cursor is in the dialog.
		}
	},

	// The page is going away. A preference changed moments ago is still sitting in
	// the batch that would have been sent a few seconds from now, so send it while
	// the connection is up.
	_onPageHide: function () {
		window.prefs.sendPendingBrowserSettingsUpdate();
	},

	// Our browser tab lost focus.
	_onLostFocus: function () {
		window.prefs.sendPendingBrowserSettingsUpdate();
		// don't deactivate view while Drag and Drop in Pivot table dialog
		if (!JSDialog.isDnDActive())
			app.idleHandler._deactivate();
	},

	// The editor got focus (probably a dialog closed or user clicked to edit).
	_onEditorGotFocus: function() {
		this._changeFocusWidget(null, 0);
		if (this.formulabar)
			this.onFormulaBarBlur();
	},

	// Our browser tab got focus.
	_onGotFocus: function () {
		if (this.editorHasFocus()) {
			this.fire('editorgotfocus');
		}
		else if (this._activeDialog) {
			this._activeDialog.focus(this.getWinId());
		}

		app.idleHandler._activate();

		if (app.definitions.CommentSection.needFocus)
		{
			app.definitions.CommentSection.needFocus.focus();
			app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).select(app.needFocus)
			app.definitions.CommentSection.needFocus = null;
		}
	},

	// Event to change the focus to dialog or editor.
	_onChangeFocusWidget: function (e) {
		if (e.winId === 0) {
			this._onEditorGotFocus();
		} else {
			this._changeFocusWidget(e.dialog, e.winId, e.acceptInput);
		}
	},

	_onSearchStart: function () {
		this._isSearching = true;
	},

	_onUpdateProgress: function (e) {

		// Minimal UX disruption for background save
		if (e.background)
		{
			switch (e.statusType)
			{
			case 'start':
				this.uiManager.documentNameInput.showProgressBar();
				if (this.saveState)
					this.saveState.showSavingStatus();
				break;
			case 'setvalue':
				this.uiManager.documentNameInput.setProgressBarValue(e.value);
				break;
			case 'finish':
				this.uiManager.documentNameInput.hideProgressBar();
				if (this.saveState)
					this.saveState.showSavedStatus();
				break;
			}
		}
		else
		{
			switch (e.statusType)
			{
			case 'start':
				// e.text translated by Core
				this.showBusy(e.text ? e.text : _('Please wait!'));
				if (e.forceid)
					this._progressBar.forceid = e.forceid;
				break;
			case 'setvalue':
				this._progressBar.setBar(true);
				this._progressBar.setValue(e.value);
				break;
			case 'finish':
			case 'coolloaded':
			case 'reconnected':
				if(this._progressBar.forceid !== e.forceid)
					return;
				this.hideBusy();
				this._progressBar.forceid = undefined;
				break;
			}
		}
	},

	_isMouseEnteringLeaving: function (e) {
		var target = e.target || e.srcElement,
		    related = e.relatedTarget;

		if (!target) { return false; }

		return ((target.id === 'map' || target.classList.contains('leaflet-layer'))
			&& !(related && (window.L.DomUtil.hasClass(related, 'leaflet-tile')
				|| window.L.DomUtil.hasClass(related, 'leaflet-cursor'))));
	},

	_handleDOMEvent: function (e) {
		app.idleHandler.notifyActive();

		if (!this._docLayer || !this._loaded || !this._enabled || window.L.DomEvent._skipped(e)) { return; }

		// find the layer the event is propagating from
		var target = this._targets[app.util.stamp(e.target || e.srcElement)],
		    //type = e.type === 'keypress' && e.keyCode === 13 ? 'click' : e.type;
		    type = e.type;

		// For touch devices, to pop-up the keyboard, it is required to call
		// .focus() method on hidden input within actual 'click' event here
		// Calling from some other place with no real 'click' event doesn't work.

		if (type === 'click' || type === 'dblclick') {
			if (this.isEditMode()) {
				this.fire('editorgotfocus');
				this.focus();
			}

			// unselect if anything is selected already
			if (app.sectionContainer.doesSectionExist(app.CSections.CommentList.name)) {
				app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).unselect();
			}
		}

		// we need to keep track if we have entered/left the map
		this._mouseEnteringLeaving = false;
		// mouse leaving the map ?
		if (!target && !this._mouseOut && type === 'mouseout') {
			this._mouseEnteringLeaving = this._isMouseEnteringLeaving(e);
			this._mouseOut = this._mouseEnteringLeaving; // event type == mouseout
		}
		// mouse entering the map ?
		if (!target && this._mouseOut && type === 'mouseover') {
			this._mouseEnteringLeaving = this._isMouseEnteringLeaving(e);
			this._mouseOut = !this._mouseEnteringLeaving; // event type == mouseover
		}

		// special case for map mouseover/mouseout events so that they're actually mouseenter/mouseleave
		if (!target && !this._mouseEnteringLeaving && (type === 'mouseover' || type === 'mouseout') &&
				!window.L.DomEvent._checkMouse(this._container, e)) { return; }

		// prevents outline when clicking on keyboard-focusable element
		if (type === 'mousedown') {
			window.L.DomUtil.preventOutline(e.target || e.srcElement);
			// Prevents image dragging on Mozilla when map's dragging
			// option is set to false
			e.preventDefault();
		}

		// workaround for drawing shapes, without this shapes cannot be shrunken
		if (target !== undefined && target._path !== undefined && type === 'mousemove') {
			target = undefined;
		}
		this._fireDOMEvent(target || this, e, type);
	},

	_fireDOMEvent: function (target, e, type) {
		if (this.uiManager.isUIBlocked())
			return;

		if (!target.listens(type, true) && (type !== 'click' || !target.listens('preclick', true))) { return; }

		if (type === 'contextmenu') {
			window.L.DomEvent.preventDefault(e);
		}

		// prevents firing click after you just dragged an object
		if (e.type === 'click' && !e._simulated && this._draggableMoved(target)) { return; }

		var data = {
			originalEvent: e
		};
		if (type === 'click') {
			target.fire('preclick', data, true);
		}
		target.fire(type, data, true);
	},

	_draggableMoved: function (obj) {
		obj = obj.options.draggable ? obj : this;
		return obj.dragging && obj.dragging.moved();
	},

	_clearHandlers: function () {
		for (var i = 0, len = this._handlers.length; i < len; i++) {
			this._handlers[i].disable();
		}
	},

	whenReady: function (callback, context) {
		if (this._loaded) {
			callback.call(context || this, {target: this});
		} else {
			this.on('load', callback, context);
		}
		return this;
	},


	// private methods for getting map state


	// adjust center for view to get inside bounds
	_limitZoom: function (zoom) {
		var min = this.getMinZoom(),
		    max = this.getMaxZoom();

		return Math.max(min, Math.min(max, zoom));
	},

	_goToViewId: function(id) {
		if (id === -1)
			return;

		if (this.getDocType() === 'spreadsheet') {
			this._docLayer.goToCellViewCursor(id);
		} else if (this.getDocType() === 'text') {
			this._docLayer.goToViewCursor(id);
		} else if (this.getDocType() === 'presentation' || this.getDocType() === 'drawing') {
			this._docLayer.goToOtherUserView(id);
		}
	},

	/// instantJump = false allows to set the following but waits for a cursor from core to scroll
	_setFollowing: function(followingState, viewId, instantJump) {
		var userDefined = viewId !== null && viewId !== undefined;
		var followDefined = followingState !== null && followingState !== undefined;

		var followEditor = true;
		var followUser = false;

		if (userDefined && viewId !== -1) {
			followUser = true;
			followEditor = false;
		}

		if (followDefined && followingState === false) {
			followEditor = false;
			followUser = false;
		}

		if (followUser) {
			if (instantJump) this._goToViewId(viewId);
			app.setFollowingUser(viewId);
		}
		else if (followEditor) {
			var editorId = this._docLayer._editorId;
			if (editorId !== -1 && editorId !== this._docLayer.viewId) {
				if (instantJump) this._goToViewId(editorId);
				app.setFollowingEditor(editorId);
			}
		}
		else {
			this.fire('deselectuser', {viewId: app.getFollowedViewId()});
			app.setFollowingOff();
		}

		// Notify about changes
		this.fire('postMessage', {msgId: 'FollowUser_Changed',
			args: {FollowedViewId: app.getFollowedViewId(),
				IsFollowUser: followUser,
				IsFollowEditor: followEditor}});
	},

	getSplitPanesContext: function () {
		var docLayer = this._docLayer;
		if (docLayer) {
			return docLayer.getSplitPanesContext();
		}

		return undefined;
	},

	_setPaneOpacity: function(paneClassString, opacity) {
		var panes = document.getElementsByClassName(paneClassString);
		if (panes.length)
			panes[0].style.opacity = opacity;
	},

	setOverlaysOpacity: function(opacity) {
		this._setPaneOpacity('leaflet-pane leaflet-overlay-pane', opacity);
	},

	setMarkersOpacity: function(opacity) {
		this._setPaneOpacity('leaflet-pane leaflet-marker-pane', opacity);
	},

	getTileSectionMgr: function() {
		if (this._docLayer)
			return this._docLayer._painter;
		return undefined;
	},

	getCursorOverlayContainer: function() {
		if (this._docLayer)
			return this._docLayer._cursorOverlayDiv;
		return undefined;
	}
});

window.L.map = function (id, options) {
	return new window.L.Map(id, options);
};
