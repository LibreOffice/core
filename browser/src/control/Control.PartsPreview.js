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
 * window.L.Control.PartsPreview
 */

/* global _ app $ Hammer _UNO cool JSDialog buildSlideDragGhost */
window.L.Control.PartsPreview = window.L.Control.extend({
	options: {
		fetchThumbnail: true,
		autoUpdate: true,
		imageClass: '',
		frameClass: '',
		axis: '',
		allowOrientation: true,
		maxWidth: window.mode.isDesktop() ? 180: (window.mode.isTablet() ? 120: 60),
		maxHeight: window.mode.isDesktop() ? 180: (window.mode.isTablet() ? 120: 60)
	},
	partsFocused: false,

	initialize: function (container, preview, options) {
		window.L.setOptions(this, options);

		if (!container) {
			container = window.L.DomUtil.get('presentation-controls-wrapper');
		}

		if (!preview) {
			preview = window.L.DomUtil.get('slide-sorter');
		}

		this._container = container;
		this._partsPreviewCont = preview;
		this._partsPreviewCont.setAttribute('role', 'listbox');
		this._partsPreviewCont.setAttribute('aria-multiselectable', 'true');
		this._partsPreviewCont.onscroll = this._onScroll.bind(this);
		this._idNum = 0;
		this._width = 0;
		this._height = 0;
		this.scrollTimer = null;
		this._menuPosEl = null;
		this.partsFocusedApplied = false;
		this._dragState = null;

		// A click clears the slide sorter focus mode, unless that same click
		// re-focused a preview.
		document.addEventListener('click', () => {
			this.partsFocusedApplied = false;
		}, true);
		document.body.addEventListener('click', () => {
			if (!this.partsFocusedApplied && this.partsFocused)
				this.partsFocused = false;
		});
	},

	onAdd: function (map) {
		this._previewInitialized = false;
		this._gridMode = false;
		this._previewTiles = [];
		this._sectionHeaders = []; // Section header DOM elements
		this._collapsedSections = new Set(); // Names of sections collapsed by the user
		this._direction = this.options.allowOrientation ?
			(!window.mode.isDesktop() && window.L.DomUtil.isPortrait() ? 'x' : 'y') :
			this.options.axis;

		map.on('updateparts', this._updateDisabled, this);
		map.on('updatepart', this._updatePart, this);
		map.on('invalidateparts', this._invalidateParts, this);
		map.on('tilepreview', this._updatePreview, this);
		map.on('insertpage', this._insertPreview, this);
		map.on('deletepage', this._deletePreview, this);
		map.on('scrolllimit', this._invalidateCurrentPart, this);
		map.on('scrolllimits', this._invalidateParts, this);
		map.on('scrolltopart', this._scrollToPart, this);
		map.on('beforerequestpreview', this._beforeRequestPreview, this);
		map.on('updatesections', this._updateSections, this);
		map.on('docloaded', this._focusCurrentSlideOnLoad, this);

		window.addEventListener('resize', window.L.bind(this._resize, this));
	},

	createScrollbar: function () {
		this._partsPreviewCont.style.whiteSpace = 'nowrap';
	},

	_updateDisabled: function () {
		const selectedPart = app.map._docLayer._selectedPart;

		const docType = app.map._docLayer._docType;

		if (docType === 'presentation' || docType === 'drawing') {
			if (!this._previewInitialized)
			{
				// Draw documents have pages, presentations have slides.
				this._partsPreviewCont.setAttribute('aria-label',
					docType === 'drawing' ? _('Pages') : _('Slides'));

				// make room for the preview
				var docContainer = this._map.options.documentContainer;

				if (!window.L.DomUtil.hasClass(docContainer, 'parts-preview-document'))
					window.L.DomUtil.addClass(docContainer, 'parts-preview-document');

				this._addContainerDnDHandlers();

				// Add a special frame just as a drop-site for reordering.
				var frameClass = 'preview-frame ' + this.options.frameClass;
				var frame = window.L.DomUtil.create('div', frameClass, this._partsPreviewCont);
				this._addDnDHandlers(frame);
				frame.setAttribute('draggable', false);
				frame.setAttribute('id', 'first-drop-site');
				// reorder-only drop target, not a slide - keep it out of the listbox
				frame.setAttribute('aria-hidden', 'true');

				if (window.mode.isDesktop()) {
					window.L.DomUtil.setStyle(frame, 'height', '20px');
					window.L.DomUtil.setStyle(frame, 'margin', '0em');
				}

				// Create the preview parts
				for (var i = 0; i < app.impress.partList.length; i++) {
					this._previewTiles.push(this._createPreview(i, app.impress.partList[i].hash));
				}
				if (!app.file.fileBasedView)
					window.L.DomUtil.addClass(this._previewTiles[selectedPart], 'preview-img-currentpart');
				this._ensureVisiblePreviews(); // Load previews.
				this._previewInitialized = true;
				this._updateSelectedSection();
			}
			else
			{
				this._syncPreviews();

				if (!app.file.fileBasedView) {
					// change the border style of the selected preview.
					for (let j = 0; j < app.impress.partList.length; j++) {
						window.L.DomUtil.removeClass(this._previewTiles[j], 'preview-img-currentpart');
						window.L.DomUtil.removeClass(this._previewTiles[j], 'preview-img-selectedpart');
						if (j === selectedPart)
							window.L.DomUtil.addClass(this._previewTiles[j], 'preview-img-currentpart');
						else if (app.impress.partList[j].selected)
							window.L.DomUtil.addClass(this._previewTiles[j], 'preview-img-selectedpart');
					}
				}
				this._updateSelectedSection();
			}

			this._updateA11ySelection();

			if (!this.options.allowOrientation) {
				return;
			}

			// update portrait / landscape
			var removePreviewImg = 'preview-img-portrait';
			var addPreviewImg = 'preview-img-landscape';
			var removePreviewFrame = 'preview-frame-portrait';
			var addPreviewFrame = 'preview-frame-landscape';
			if (window.L.DomUtil.isPortrait()) {
				removePreviewImg = 'preview-img-landscape';
				addPreviewImg = 'preview-img-portrait';
				removePreviewFrame = 'preview-frame-landscape';
				addPreviewFrame = 'preview-frame-portrait';
			}

			for (i = 0; i < app.impress.partList.length; i++) {
				window.L.DomUtil.removeClass(this._previewTiles[i], removePreviewImg);
				window.L.DomUtil.addClass(this._previewTiles[i], addPreviewImg);
				if (app.impress.isSlideHidden(i))
					window.L.DomUtil.addClass(this._previewTiles[i], 'hidden-slide');
				else
					window.L.DomUtil.removeClass(this._previewTiles[i], 'hidden-slide');
			}

			var previewFrame = $(this._partsPreviewCont).find('.preview-frame');
			previewFrame.removeClass(removePreviewFrame);
			previewFrame.addClass(addPreviewFrame);

			// re-create scrollbar with new direction
			this._direction = !window.mode.isDesktop() && !window.mode.isTablet() && window.L.DomUtil.isPortrait() ? 'x' : 'y';
		}
	},

	// Keep the listbox selection state current: aria-selected on every
	// selected slide, and the roving tab stop (tabindex 0) on the current one.
	_updateA11ySelection: function () {
		var selectedPart = this._map._docLayer._selectedPart;
		var partList = (app.impress && app.impress.partList) || [];
		var activeTab = (selectedPart >= 0 && selectedPart < this._previewTiles.length)
			? selectedPart : 0;
		for (var i = 0; i < this._previewTiles.length; i++) {
			var img = this._previewTiles[i];
			if (!img)
				continue;
			var selected = app.file.fileBasedView
				? i === activeTab
				: (i === selectedPart || (partList[i] && partList[i].selected));
			img.setAttribute('aria-selected', selected ? 'true' : 'false');
			img.setAttribute('tabindex', i === activeTab ? '0' : '-1');
		}
	},

	isPaddingClick: function (element, e, part) {
		var style = window.getComputedStyle(element, null);
		var nTop = parseInt(style.getPropertyValue('padding-top'));
		var nRight = parseFloat(style.getPropertyValue('padding-right'));
		var nLeft = parseFloat(style.getPropertyValue('padding-left'));
		var nBottom = parseFloat(style.getPropertyValue('padding-bottom'));
		var width = element.offsetWidth;
		var height = element.offsetHeight;
		var x = parseFloat(e.offsetX);
		var y = parseFloat(e.offsetY);

		if (part === 'top')         // Clicked on top padding?
			return !(y > nTop);
		else if (part === 'bottom') // Clicked on bottom padding?
			return !(y < height - nBottom);
		else                        // Clicked on any padding?
			return !((x > nLeft && x < width - nRight) && (y > nTop && y < height - nBottom));
	},

	_getMenuPosEl: function () {
		if (!this._menuPosEl) {
			this._menuPosEl = document.createElement('div');
			this._menuPosEl.id = 'slide-context-menu-pos';
			this._menuPosEl.style.position = 'absolute';
			this._container.appendChild(this._menuPosEl);
		}
		return this._menuPosEl;
	},

	_createPreview: function (i, hashCode) {
		var frameClass = 'preview-frame ' + this.options.frameClass;
		var frame = window.L.DomUtil.create('div', frameClass, this._partsPreviewCont);
		frame.id = 'preview-frame-part-' + this._idNum;
		this._addDnDHandlers(frame);
		window.L.DomUtil.create('span', 'preview-helper', frame);

		// The visible digit comes from a CSS counter keyed off DOM order (see
		// partsPreviewControl.css), so it always matches the frame's current
		// position without any renumbering when frames are inserted or removed.
		const slideNumber = window.L.DomUtil.create('span', 'preview-slide-number', frame);
		slideNumber.setAttribute('aria-hidden', 'true');

		var imgClassName = 'preview-img ' + this.options.imageClass;
		var img = window.L.DomUtil.create('img', imgClassName, frame);
		this._setPreviewPositionLabels(img, i);
		// A drag grabbed on the picture starts from the frame, the nearest
		// draggable ancestor, so it carries the slide marker data and shows
		// the framed preview as its ghost.
		img.draggable = false;
		img.setAttribute('role', 'option');
		img.setAttribute('aria-selected', 'false');
		// roving tabindex - only the current slide is a tab stop so Tab
		// enters the list once and the arrow keys move within it
		img.setAttribute('tabindex', '-1');
		window.L.control.attachTooltipEventListener(img, this._map);
		img.id = 'preview-img-part-' + this._idNum;
		// The unique id of the slide this preview shows. The slide list
		// carries that id under its legacy name, hash, and this property
		// keeps the name it is seeded from.
		img.hash = hashCode;
		img.src = document.querySelector('meta[name="previewSmile"]').content;
		img.fetched = false;
		if (!window.mode.isDesktop()) {
			(new Hammer(img, {recognizers: [[Hammer.Press]]}))
				.on('press', function (e) {
					if (this._map.isEditMode()) {
						this._addDnDTouchHandlers(e);
					}
				}.bind(this));
		}
		window.L.DomEvent.on(img, 'click', function (e) {
			window.L.DomEvent.stopPropagation(e);
			window.L.DomEvent.stop(e);
			var part = this._findClickedPart(e.target.parentNode);
			if (part !== null)
				var partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.
			if (!window.mode.isDesktop() && partId === this._map._docLayer._selectedPart && !app.file.fileBasedView) {
				// if mobile or tab then second tap will open the mobile wizard
				if (this._map._permission === 'edit') {
					// Remove selection to get the slide properties in mobile wizard.
					app.socket.sendMessage('resetselection');
					setTimeout(function () {
						app.dispatcher.dispatch('mobile_wizard');
					}, 0);
				}
			} else {
				this._setPart(e);
				if (!window.mode.isDesktop()) {
					// needed so on-screen keyboard doesn't pop up when switching slides,
					// but would cause PgUp/Down to not work on desktop in slide sorter
					document.activeElement.blur();
				}
			}
			img.focus();
		}, this);

		var that = this;
		img.onfocus = function () {
			that._map._clip.clearSelection();
			that._map._clip.setTextSelectionType('slide');
			that.partsFocused = true;
			that.partsFocusedApplied = true;
		};

		var that = this;
		window.L.DomEvent.on(frame, 'contextmenu', function(e) {
			var isMasterView = this._map['stateChangeHandler'].getItemValue('.uno:SlideMasterPage');
			if (isMasterView === 'true')
				return;
			e.preventDefault();
			// The frame area only offers Paste and Insert, both of which need
			// edit access, so in read-only mode just suppress the system menu.
			if (app.map.isReadOnlyMode())
				return;

			var nPos = undefined;
			if (this.isPaddingClick(frame, e, 'top'))
				nPos = that._findClickedPart(frame) - 1;
			else if (this.isPaddingClick(frame, e, 'bottom'))
				nPos = that._findClickedPart(frame);
			else if (this.isPaddingClick(frame, e, 'right') || this.isPaddingClick(frame, e, 'left'))
				nPos = that._findClickedPart(frame);

			if (!that._isSelected(e))
				that._setPart(e);
			img.focus();

			var entries = [];
			// Offer Paste whenever we can read the clipboard: the browser Clipboard
			// API, or the Mac app's native clipboard bridge. (The other apps drive
			// slide copy/paste through their own native clipboard, which is not
			// wired up for slides yet, so leave them on the Clipboard-API check.)
			if (window.L.Browser.clipboardApiAvailable || window.ThisIsTheMacOSApp) {
				entries.push({
					id: 'paste',
					type: 'comboboxentry',
					text: _('Paste'),
					img: 'Paste',
					class: 'unoPaste',
					pos: 0,
				});
			}
			entries.push({
				id: 'newslide',
				type: 'comboboxentry',
				text: _UNO(that._map._docLayer._docType == 'presentation' ? '.uno:InsertSlide' : '.uno:InsertPage', 'presentation'),
				img: 'InsertPage',
				class: 'unoInsertPage',
				pos: 0,
			});

			var menuPosEl = that._getMenuPosEl();
			var rect = that._container.getBoundingClientRect();
			menuPosEl.style.left = (e.clientX - rect.left) + 'px';
			menuPosEl.style.top = (e.clientY - rect.top) + 'px';

			var callback = function(objectType, eventType, object, data, entry) {
				if (eventType !== 'selected')
					return false;
				if (entry.id === 'paste') {
					if (nPos === undefined)
						nPos = that._findClickedPart(frame);
					that._pasteSlide(nPos);
				} else if (entry.id === 'newslide') {
					that._map.insertPage(nPos);
				}
				JSDialog.CloseAllDropdowns();
				return true;
			};

			JSDialog.OpenDropdown(
				'slide-frame-menu',
				menuPosEl,
				entries,
				callback,
				'',
				false,
				false,
				true,
			);
		}, this);

		window.L.DomEvent.on(img, 'contextmenu', function(e) {
			e.stopPropagation();
			var isMasterView = this._map['stateChangeHandler'].getItemValue('.uno:SlideMasterPage');
			if (isMasterView === 'true')
				return;
			e.preventDefault();

			if (!that._isSelected(e))
				that._setPart(e);
			img.focus();

			var part = that._findClickedPart(img.parentNode);
			var partIndex = parseInt(part) - 1;
			var isPresentation = that._map._docLayer._docType === 'presentation';

			var entries = [];
			if (!(app.impress.hasOverviewPage && that._map._docLayer._selectedPart === 0)) {
				entries.push({
					id: 'copy',
					type: 'comboboxentry',
					text: _('Copy'),
					img: 'Copy',
					class: 'unoCopy',
					pos: 0,
				});
			}
			entries.push({
				id: 'paste',
				type: 'comboboxentry',
				text: _('Paste'),
				img: 'Paste',
				class: 'unoPaste',
				pos: 0,
			});
			entries.push({
				id: 'newslide',
				type: 'comboboxentry',
				text: _UNO(isPresentation ? '.uno:InsertSlide' : '.uno:InsertPage', 'presentation'),
				img: 'InsertPage',
				class: 'unoInsertPage',
				pos: 0,
			});
			entries.push({
				id: 'duplicateslide',
				type: 'comboboxentry',
				text: _UNO(isPresentation ? '.uno:DuplicateSlide' : '.uno:DuplicatePage', 'presentation'),
				img: 'DuplicatePage',
				class: 'unoDuplicatePage',
				pos: 0,
			});
			if (that._map._docLayer._parts > 1) {
				entries.push({
					id: 'delete',
					type: 'comboboxentry',
					text: _UNO(isPresentation ? '.uno:DeleteSlide' : '.uno:DeletePage', 'presentation'),
					img: 'DeletePage',
					class: 'unoDeletePage',
					pos: 0,
				});
			}
			entries.push({
				id: 'slideproperties',
				type: 'comboboxentry',
				text: _UNO(isPresentation ? '.uno:SlideSetup' : '.uno:PageSetup', 'presentation'),
				img: 'PageSetup',
				class: 'slideproperties',
				pos: 0,
			});
			if (isPresentation && app.impress.isSlideHidden(partIndex)) {
				entries.push({
					id: 'showslide',
					type: 'comboboxentry',
					text: _UNO('.uno:ShowSlide', 'presentation'),
					img: 'ShowSlide',
					class: 'showslide',
					pos: 0,
				});
			}
			if (isPresentation && !app.impress.isSlideHidden(partIndex)) {
				entries.push({
					id: 'hideslide',
					type: 'comboboxentry',
					text: _UNO('.uno:HideSlide', 'presentation'),
					img: 'Hideslide',
					class: 'hideslide',
					pos: 0,
				});
			}

			// if not the first section slide then add entry for section
			var isFirstSectionSlide = false;
			const sections = app.impress.sections;
			if (sections) {
				for (let i = 0; i < sections.length; i++) {
					if (sections[i].startIndex === partIndex) {
						isFirstSectionSlide = true;
						break;
					}
				}
			}
			if (!isFirstSectionSlide) {
				entries.push({
					id: 'addsection',
					type: 'comboboxentry',
					text: _('Add Section'),
					img: 'addslidesection',
					class: 'addsection',
					pos: 0,
				});
			}

			// In read-only mode the slide cannot be changed, but copying it is
			// still allowed, so keep only Copy and drop the editing entries. The
			// system menu is already suppressed, so when nothing is left (for
			// example the overview page, which has no Copy entry) just close out.
			if (app.map.isReadOnlyMode())
				entries = entries.filter(function(entry) { return entry.id === 'copy'; });
			if (entries.length === 0)
				return;

			var menuPosEl = that._getMenuPosEl();
			var rect = that._container.getBoundingClientRect();
			menuPosEl.style.left = (e.clientX - rect.left) + 'px';
			menuPosEl.style.top = (e.clientY - rect.top) + 'px';

			var callback = function(objectType, eventType, object, data, entry) {
				if (eventType !== 'selected')
					return false;
				switch (entry.id) {
				case 'copy':
					that._map._clip.clearSelection();
					that._map._clip.setTextSelectionType('slide');
					that._map._clip._execCopyCutPaste('copy', '.uno:CopySlide');
					break;
				case 'paste':
					that._pasteSlide(part);
					break;
				case 'newslide':
					that._map.insertPage();
					break;
				case 'duplicateslide':
					that._map.duplicatePage();
					break;
				case 'delete':
					app.dispatcher.dispatch('deletepage');
					break;
				case 'slideproperties':
					app.socket.sendMessage('uno .uno:PageSetup');
					break;
				case 'showslide':
					that._map.showSlide();
					break;
				case 'hideslide':
					that._map.hideSlide();
					break;
				case 'addsection':
					app.socket.sendMessage('uno .uno:AddSlideSection');
					break;
				}
				JSDialog.CloseAllDropdowns();
				return true;
			};

			JSDialog.OpenDropdown(
				'slide-img-menu',
				menuPosEl,
				entries,
				callback,
				'',
				false,
				false,
				true,
			);
		}, this);

		var imgSize = this._map.getPreview(i, i,
						   this.options.maxWidth,
						   this.options.maxHeight,
						   {autoUpdate: this.options.autoUpdate,
						    fetchThumbnail: false});

		window.L.DomUtil.setStyle(img, 'width', imgSize.width + 'px');
		window.L.DomUtil.setStyle(img, 'height', imgSize.height + 'px');

		this._idNum++;

		return img;
	},

	// Unlike the visible digit, the alt text and tooltip are plain
	// attribute strings, so they only ever hold the position they were
	// given here. Callers that move a preview to a different position
	// must call this again with the new position.
	_setPreviewPositionLabels: function (img, i) {
		img.setAttribute('alt', _('preview of page %1').replace('%1', String(i + 1)));
		img.setAttribute('data-cooltip', _('Slide %1').replace('%1', String(i + 1)));
	},

	// Relabels every preview from startIndex onward to match its current
	// position in _previewTiles, for callers that insert or remove a
	// preview and thereby shift the position of every one that follows it.
	_updatePreviewPositionLabelsFrom: function (startIndex) {
		for (var i = startIndex; i < this._previewTiles.length; i++) {
			this._setPreviewPositionLabels(this._previewTiles[i], i);
		}
	},

	_updateSections: function (e) {
		if (!this._previewInitialized)
			return;

		var sections = e.sections || [];

		// Remove existing section headers
		for (var i = 0; i < this._sectionHeaders.length; i++) {
			window.L.DomUtil.remove(this._sectionHeaders[i]);
		}
		this._sectionHeaders = [];

		if (!sections || sections.length === 0) {
			this._collapsedSections.clear();
			this._clearAllSectionCollapse();
			// Removing the last section un-hides its slides; fetch the
			// thumbnails for any that were collapsed before they were loaded.
			this._ensureVisiblePreviews();
			return;
		}

		// Drop any remembered names that no longer correspond to a section
		// (e.g. after a rename or removal).
		var liveNames = new Set();
		for (var ln = 0; ln < sections.length; ln++)
			liveNames.add(sections[ln].name);
		this._collapsedSections.forEach(function (name) {
			if (!liveNames.has(name))
				this._collapsedSections.delete(name);
		}, this);

		// Insert section headers before the frame of each section's first slide.
		// The container children are: #first-drop-site, frame0, frame1, ...
		// So slide index N corresponds to child index N+1.
		for (var s = 0; s < sections.length; s++) {
			var section = sections[s];
			var slideIndex = section.startIndex;

			if (slideIndex < 0 || slideIndex >= this._previewTiles.length)
				continue;

			var header = this._createSectionHeader(section, s);
			this._sectionHeaders.push(header);

			// Insert before the frame of this section's first slide
			var slideFrame = this._previewTiles[slideIndex].parentNode;
			slideFrame.parentNode.insertBefore(header, slideFrame);
		}

		this._applyAllSectionsCollapse();
		// Removing a section can turn its slides back into ungrouped, visible
		// ones; fetch their thumbnails now that they show again.
		this._ensureVisiblePreviews();

		this._updateSelectedSection();
	},

	_updateSelectedSection: function () {
		if (!this._sectionHeaders || this._sectionHeaders.length === 0)
			return;

		var sections = (app.impress && app.impress.sections) || [];
		var partList = (app.impress && app.impress.partList) || [];
		var fullySelected = new Set();
		for (var s = 0; s < sections.length; s++) {
			var start = sections[s].startIndex;
			var end = (s + 1 < sections.length)
				? sections[s + 1].startIndex - 1
				: this._previewTiles.length - 1;
			if (start < 0 || end < start)
				continue;

			var allSelected = true;
			for (var p = start; p <= end; p++) {
				if (!partList[p] || !partList[p].selected) {
					allSelected = false;
					break;
				}
			}
			if (allSelected)
				fullySelected.add(s);
		}

		for (var h = 0; h < this._sectionHeaders.length; h++) {
			var header = this._sectionHeaders[h];
			var nameSpan = header.querySelector('.slide-section-name');
			if (!nameSpan)
				continue;
			var headerSectionIndex = parseInt(header.getAttribute('data-section-index'), 10);
			if (fullySelected.has(headerSectionIndex))
				window.L.DomUtil.addClass(nameSpan, 'selected');
			else
				window.L.DomUtil.removeClass(nameSpan, 'selected');
		}
	},

	_createSectionHeader: function (section, sectionIndex) {
		var that = this;

		var header = window.L.DomUtil.create('div', 'slide-section-header');
		header.setAttribute('data-section-index', sectionIndex);
		header.setAttribute('data-start-index', section.startIndex);
		header.setAttribute('draggable', 'false');

		var toggleBtn = window.L.DomUtil.create('button', 'slide-section-toggle ui-expander-btn', header);
		toggleBtn.type = 'button';
		toggleBtn.setAttribute('aria-label',
			_('Toggle section %1').replace('%1', section.name));

		var nameSpan = window.L.DomUtil.create('span', 'slide-section-name', header);
		nameSpan.textContent = section.name;
		nameSpan.setAttribute('title', section.name);

		window.L.DomEvent.on(toggleBtn, 'click', function (e) {
			window.L.DomEvent.stopPropagation(e);
			window.L.DomEvent.preventDefault(e);
			that._toggleSectionCollapse(sectionIndex);
		}, this);

		// Click on the header (but not the toggle) selects all slides in the section.
		window.L.DomEvent.on(header, 'click', function (e) {
			if (toggleBtn.contains(e.target))
				return;
			window.L.DomEvent.stopPropagation(e);
			window.L.DomEvent.preventDefault(e);
			that._selectSection(sectionIndex);
		}, this);

		// Section context menu
		if (this._map.isEditMode()) {
			window.L.DomEvent.on(header, 'contextmenu', function(e) {
				window.L.DomEvent.stopPropagation(e);
				window.L.DomEvent.preventDefault(e);

				if (app.map.isReadOnlyMode())
					return;

				that._openSectionContextMenu(section, sectionIndex, e);
			}, this);
		}

		return header;
	},

	_selectSection: function (sectionIndex) {
		var sections = app.impress && app.impress.sections;
		if (!sections || !sections[sectionIndex])
			return;

		var start = sections[sectionIndex].startIndex;
		var end = (sectionIndex + 1 < sections.length)
			? sections[sectionIndex + 1].startIndex - 1
			: this._previewTiles.length - 1;

		if (start < 0 || end < start)
			return;

		var applyRange = () => {
			this._map.selectPart(start, 1, false);
			for (var id = start + 1; id <= end; ++id)
				this._map.selectPart(id, 1, false);
			this._selectedPartRange = [start, end];
		};

		this._map.deselectAll();

		if (this._map._docLayer._selectedPart === start) {
			applyRange();
			return;
		}

		// setPart(start) triggers SwitchPage in core, which posts itself
		// for async execution when a paint is in progress (common under
		// CI load). SwitchPage deselects every page before selecting the
		// new current one, so issuing the multi-select before it runs
		// loses slides start+1..end. Wait for the kit's setpart:
		// confirmation - it only fires after SwitchPage actually
		// completes - before extending the selection.
		var done = false;
		var onSetPart = function () {
			if (done) return;
			done = true;
			applyRange();
		};
		this._map.once('setpart', onSetPart);
		// Safety net in case setpart: never arrives.
		setTimeout(onSetPart, 2000);

		this._map.setPart(start);
	},

	_toggleSectionCollapse: function (sectionIndex) {
		var sections = app.impress.sections || [];
		var section = sections[sectionIndex];
		if (!section)
			return;

		if (this._collapsedSections.has(section.name))
			this._collapsedSections.delete(section.name);
		else
			this._collapsedSections.add(section.name);

		this._applySectionCollapse(sectionIndex);
		// Expanding may reveal thumbnails whose images were never fetched.
		this._ensureVisiblePreviews();
	},

	// Apply the collapsed class to one section's header and its slide frames.
	_applySectionCollapse: function (sectionIndex) {
		var section = app.impress.sections && app.impress.sections[sectionIndex];
		if (!section)
			return;

		var collapsed = this._collapsedSections.has(section.name);
		var end = section.startIndex + section.slideCount;

		var header = this._sectionHeaders[sectionIndex];
		if (header) {
			var toggleBtn = header.querySelector('.slide-section-toggle');
			header.classList.toggle('collapsed', collapsed);
			if (toggleBtn)
				toggleBtn.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
		}

		for (var i = section.startIndex; i < end; i++) {
			var frame = this._previewTiles[i] && this._previewTiles[i].parentNode;
			if (frame)
				frame.classList.toggle('section-collapsed', collapsed);
		}
	},

	// Remove the collapsed marker from every slide frame. Section membership is
	// recomputed from the current section list, so a slide that dropped out of a
	// section, for example when a collapsed section was removed, must not keep
	// the hidden state of the section it used to belong to.
	_clearAllSectionCollapse: function () {
		for (let i = 0; i < this._previewTiles.length; i++) {
			const frame = this._previewTiles[i] && this._previewTiles[i].parentNode;
			if (frame)
				frame.classList.remove('section-collapsed');
		}
	},

	// Apply collapsed state to every section, starting from a clean slate so
	// only slides in a section that is currently collapsed stay hidden.
	_applyAllSectionsCollapse: function () {
		this._clearAllSectionCollapse();
		var sections = app.impress.sections || [];
		for (var s = 0; s < sections.length; s++)
			this._applySectionCollapse(s);
	},

	// Collapse every section.
	_collapseAllSections: function () {
		const sections = app.impress.sections || [];
		if (sections.length === 0)
			return;
		for (let i = 0; i < sections.length; i++)
			this._collapsedSections.add(sections[i].name);
		this._applyAllSectionsCollapse();
	},

	// Expand every section.
	_expandAllSections: function () {
		if (this._collapsedSections.size === 0)
			return;
		this._collapsedSections.clear();
		this._applyAllSectionsCollapse();
		// Expanding may reveal thumbnails whose images were never fetched.
		this._ensureVisiblePreviews();
	},

	_openSectionContextMenu: function (section, sectionIndex, e) {
		var that = this;
		var sections = app.impress.sections || [];

		const hasCollapsed = this._collapsedSections.size > 0;
		const hasExpanded = this._collapsedSections.size < sections.length;

		var entries = [{
			id: 'renameSection',
			type: 'comboboxentry',
			text: _('Rename Section'),
			img: 'renameslidesection',
			pos: 0,
		}];
		entries.push({
			id: 'removeSection',
			type: 'comboboxentry',
			text: _('Remove Section'),
			img: 'removeslidesection',
			pos: 0,
		});
		if (sections.length > 1) {
			entries.push({
				id: 'removeSectionAndSlides',
				type: 'comboboxentry',
				text: _('Remove Section & Slides'),
				img: 'removesectionandslides',
				pos: 0,
			});
		}
		if (sectionIndex > 0) {
			entries.push({
				id: 'moveSectionUp',
				type: 'comboboxentry',
				text: _('Move Section Up'),
				img: 'arrowup',
				pos: 0,
			});
		}
		if (sectionIndex < sections.length - 1) {
			entries.push({
				id: 'moveSectionDown',
				type: 'comboboxentry',
				text: _('Move Section Down'),
				img: 'arrowdown',
				pos: 0,
			});
		}
		if (sections.length > 1 && hasExpanded) {
			entries.push({
				id: 'collapseAllSections',
				type: 'comboboxentry',
				text: _('Collapse All Sections'),
				pos: 0,
			});
		}
		if (sections.length > 1 && hasCollapsed) {
			entries.push({
				id: 'expandAllSections',
				type: 'comboboxentry',
				text: _('Expand All Sections'),
				pos: 0,
			});
		}

		var menuPosEl = this._getMenuPosEl();
		var rect = this._container.getBoundingClientRect();
		menuPosEl.style.left = (e.clientX - rect.left) + 'px';
		menuPosEl.style.top = (e.clientY - rect.top) + 'px';

		var callback = function (objectType, eventType, object, data, entry) {
			if (eventType !== 'selected')
				return false;
			switch (entry.id) {
			case 'renameSection':
				that._renameSection(section, sectionIndex);
				break;
			case 'moveSectionUp':
				that._map.setPart(section.startIndex);
				that._map.selectPart(section.startIndex, 1, false);
				app.socket.sendMessage('uno .uno:MoveSlideSectionUp');
				break;
			case 'moveSectionDown':
				that._map.setPart(section.startIndex);
				that._map.selectPart(section.startIndex, 1, false);
				app.socket.sendMessage('uno .uno:MoveSlideSectionDown');
				break;
			case 'removeSection':
				that._map.deselectAll();
				that._map.setPart(section.startIndex);
				that._map.selectPart(section.startIndex, 1, false);
				app.socket.sendMessage('uno .uno:RemoveSlideSection');
				break;
			case 'removeSectionAndSlides': {
				var n = section.slideCount;
				var msg = _('Delete section "%1" and its %2 slide(s)?')
					.replace('%1', section.name)
					.replace('%2', String(n));
				app.map.uiManager.showInfoModal(
					'remove-section-slides-modal',
					_('Delete'),
					msg,
					'',
					_('OK'),
					function () {
						that._map.setPart(section.startIndex);
						that._map.selectPart(section.startIndex, 1, false);
						app.socket.sendMessage('uno .uno:RemoveSlideSectionAndSlides');
					},
					true,
					'remove-section-slides-modal-response',
				);
				break;
			}
			case 'collapseAllSections':
				that._collapseAllSections();
				break;
			case 'expandAllSections':
				that._expandAllSections();
				break;
			}
			JSDialog.CloseAllDropdowns();
			return true;
		};

		JSDialog.OpenDropdown(
			'slide-section-menu',
			menuPosEl,
			entries,
			callback,
			'',
			false,
			false,
			true,
		);
	},

	_renameSection: function (section, sectionIndex) {
		var currentName = section.name;

		app.map.uiManager.showInputModal(
			'rename-section',
			_('Rename Section'),
			_('Enter new section name:'),
			currentName,
			_('OK'),
			function (newName) {
				if (newName && newName !== currentName) {
					var command = {
						'SectionIndex': {
							'type': 'long',
							'value': sectionIndex
						},
						'Name': {
							'type': 'string',
							'value': newName
						}
					};
					app.socket.sendMessage('uno .uno:RenameSlideSection ' + JSON.stringify(command));
				}
			}
		);
	},

	_scrollToPart: function(part) {
		var partNo = part !== undefined ? part : this._map.getCurrentPartNumber();
		// Use the preview tile's parent frame directly instead of child index
		var node = this._previewTiles[partNo] ? this._previewTiles[partNo].parentNode : null;

		if (node && (!this._previewTiles[partNo] || !this._isPreviewVisible(partNo))) {
			if (this.scrollTimer) clearTimeout(this.scrollTimer);

			 this.scrollTimer = setTimeout(() => {
				node.scrollIntoView();
				this.scrollTimer = null;
			}, 50);
		}
	},

	// Returns the logical child index (counting only frames, not section headers).
	_findClickedPart: function (element) {
		var frameIndex = 0;
		for (var i = 0; i < this._partsPreviewCont.children.length; i++) {
			var child = this._partsPreviewCont.children[i];
			if (child === element || child === element.parentNode) {
				return frameIndex;
			}
			// Only count slide frames: not section headers, and not the
			// drop-gap cell a grid drag keeps in the flow.
			if (!child.classList.contains('slide-section-header') &&
			    !child.classList.contains('drop-gap-cell'))
				frameIndex++;
		}
		return -1;
	},

	// This is used with fileBasedView.
	_scrollViewToPartPosition: function (partNumber, fromBottom) {
		if (this._map._docLayer && this._map._docLayer._isZooming)
			return;

		if (partNumber < 0) partNumber = 0;
		if (partNumber >= this._map._docLayer._parts) partNumber = this._map._docLayer._parts - 1;

		var viewHeight = app.sectionContainer.getViewSize()[1];
		var currentScrollX = app.activeDocument.activeLayout.viewedRectangle.pX1;

		var layout = app.activeDocument.activeLayout;
		var scrollTop;
		var partHeightPixels;
		if (layout.viewRectangles && layout.viewRectangles[partNumber]) {
			scrollTop = layout.viewRectangles[partNumber].pY1;
			partHeightPixels = layout.viewRectangles[partNumber].pHeight + Math.round(this._map._docLayer._spaceBetweenParts * app.twipsToPixels);
		} else {
			partHeightPixels = Math.round((this._map._docLayer._partHeightTwips + this._map._docLayer._spaceBetweenParts) * app.twipsToPixels);
			scrollTop = partHeightPixels * partNumber;
		}

		if (viewHeight > partHeightPixels && partNumber > 0)
			scrollTop -= Math.round((viewHeight - partHeightPixels) * 0.5);

		// scroll to the bottom of the selected part/page instead of its top px
		if (fromBottom)
			scrollTop += partHeightPixels - viewHeight;

		app.activeDocument.activeLayout.scrollTo(currentScrollX, scrollTop);
	},

	_scrollViewByDirection: function(buttonType) {
		if (this._map._docLayer && this._map._docLayer._isZooming)
			return;
		var viewHeight = Math.floor(app.sectionContainer.getViewSize()[1]);
		var viewHeightScaled = Math.round(Math.floor(viewHeight) / app.dpiScale);
		var scrollBySize = Math.floor(viewHeightScaled * 0.75);
		var currentScrollX = app.activeDocument.activeLayout.viewedRectangle.cX1;

		app.sectionContainer.getSectionWithName(app.CSections.Scroll.name).onScrollBy({x: currentScrollX, y: buttonType === 'prev' ? -scrollBySize : scrollBySize});
	},

	// Paste a slide. Always read from the system clipboard and force-upload
	// to the local Kit (skipping the same-tab pTransferClip shortcut),
	// so that a copy from another tab/document wins over any stale local copy.
	// nPos: insertion position for the frame context menu (may be undefined for img context menu).
	_pasteSlide: async function(nPos) {
		if (this._pastePending)
			return;
		this._pastePending = true;
		try {
			if (nPos !== undefined)
				this._map.setPart(Math.max(0, nPos - 1));

			const clip = this._map._clip;

			// Read the system clipboard so a copy from another tab/document wins
			// over a stale local copy. _readClipboardItems handles the CODA/mobile
			// app native bridges (where navigator.clipboard.read() is unavailable
			// or would pop up the WebView's system "Paste" confirmation).
			const canReadClipboard = window.L.Browser.clipboardApiAvailable
				|| window.ThisIsTheiOSApp || window.ThisIsTheMacOSApp || window.ThisIsTheWindowsApp;
			if (canReadClipboard) {
				let html = '';
				try {
					const items = await clip._readClipboardItems();
					let foundItem = null;
					for (const item of (items || [])) {
						if (item.types.includes('text/html')) {
							foundItem = item;
							break;
						}
					}

					if (foundItem) {
						const blob = await foundItem.getType('text/html');
						html = await blob.text();
					}
				} catch (e) {
					html = '';
				}
				if (html) {
					// preferInternal=false skips the pTransferClip shortcut,
					// so the most recent system-clipboard content wins.
					clip.dataTransferToDocument(null, false, html, false);
					return;
				}
			}

			// No usable HTML on the clipboard (or the native app reported an
			// internal copy): let the paste event / native clipboard bridge
			// drive things.
			clip.filterExecCopyPaste('.uno:Paste');
		} finally {
			this._pastePending = false;
		}
	},

	_isSelected: function (e) {
		var part = this._findClickedPart(e.target.parentNode);
		var partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.
		if (partId < 0)
			return false;
		else
			return app.impress.isSlideSelected(partId);
	},

	_setPart: function (e) {
		const editingComment = cool.Comment.isAnyEdit();
		if (editingComment) {
			const commentSection = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			if (commentSection) {
				commentSection.navigateAndFocusComment(editingComment);
			}
			return;
		}

		var part = this._findClickedPart(e.target.parentNode);
		if (part !== -1) {
			var partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.

			if (app.file.fileBasedView) {
				this._map.setPart(partId);
				this._scrollViewToPartPosition(partId);
				return;
			}

			if (e.ctrlKey) {
				this._map.selectPart(partId, 2, false); // Toggle selection on ctrl+click.
			} else if (e.altKey) {
				window.app.console.log('alt');
			} else if (e.shiftKey) {
				this._selectPartRange(this._map._docLayer._selectedPart, partId);
			} else {
				this._map.deselectAll();
				this._map.setPart(partId);
				this._map.selectPart(partId, 1, false); // And select.
			}
		}
	},

	_selectPartRange: function (start, end, scrollToEnd = true) {
		if (start === undefined || start === null)
			start = this._map._docLayer._selectedPart;

		var maxIndex = this._partsPreviewCont.children.length - 1;
		start = Math.max(0, Math.min(start, maxIndex));
		end = Math.max(0, Math.min(end, maxIndex));

		//deselect all slides
		this._map.deselectAll();

		//reselect the first original selection
		this._map.setPart(start);
		this._map.selectPart(start, 1, false);

		if (start < end) {
			for (var id = start + 1; id <= end; ++id) {
				this._map.selectPart(id, 1, false);
			}
		} else if (start > end) {
			for (id = start - 1; id >= end; --id) {
				this._map.selectPart(id, 1, false);
			}
		}
		this._selectedPartRange = [start, end];
		if (scrollToEnd)
			this._scrollToPart(end);
	},

	_modifySelectedPartRange: function (direction) {
		var start, end;
		if (this._selectedPartRange) {
			start = this._selectedPartRange[0];
			end = this._selectedPartRange[1];
		} else {
			start = end = this._map._docLayer._selectedPart;
		}

		if (direction === "UP")
			this._selectPartRange(start, end - 1);
		if (direction === "DOWN")
			this._selectPartRange(start, end + 1);
	},

	_updatePart: function (e) {
		if ((e.docType === 'presentation' || e.docType === 'drawing') && e.part >= 0) {
			this._map.getPreview(e.part, e.part, this.options.maxWidth, this.options.maxHeight, {autoUpdate: this.options.autoUpdate});
		}
	},

	_syncPreviews: function () {
		var it = 0;

		if (app.impress.partList.length !== this._previewTiles.length) {
			if (Math.abs(app.impress.partList.length - this._previewTiles.length) === 1) {
				if (app.impress.partList.length > this._previewTiles.length) {
					for (it = 0; it < app.impress.partList.length; it++) {
						if (it === this._previewTiles.length) {
							this._insertPreview({selectedPart: it - 1, hashCode: app.impress.partList[it].hash});
							break;
						}
						if (this._previewTiles[it].hash !== app.impress.partList[it].hash) {
							// new slide is at it; _insertPreview adds at selectedPart + 1
							this._insertPreview({selectedPart: it - 1, hashCode: app.impress.partList[it].hash});
							break;
						}
					}
				}
				else {
					for (it = 0; it < this._previewTiles.length; it++) {
						if (it === app.impress.partList.length ||
						    this._previewTiles[it].hash !== app.impress.partList[it].hash) {
							this._deletePreview({selectedPart: it});
							break;
						}
					}
				}
			}
			else {
				// sync all, should never happen
				while (this._previewTiles.length < app.impress.partList.length) {
					this._insertPreview({selectedPart: this._previewTiles.length - 1,
							     hashCode: app.impress.partList[this._previewTiles.length].hash});
				}

				while (this._previewTiles.length > app.impress.partList.length) {
					this._deletePreview({selectedPart: this._previewTiles.length - 1});
				}

				for (it = 0; it < app.impress.partList.length; it++) {
					this._previewTiles[it].hash = app.impress.partList[it].hash;
					this._previewTiles[it].src = document.querySelector('meta[name="previewSmile"]').content;
					this._previewTiles[it].fetched = false;
				}
			}
		}
		else {
			// The same number of slides with hashes at new indices means the
			// slides were reordered (or an index holds a new slide). A part's
			// hash stays the same for its whole lifetime, so the preview
			// image that already carries it can move to the part's new index
			// and show up there immediately; only a hash no preview carries
			// needs a fetch.
			const previewByHash = {};
			for (it = 0; it < this._previewTiles.length; it++) {
				if (this._previewTiles[it].hash) {
					previewByHash[this._previewTiles[it].hash] = {
						src: this._previewTiles[it].src,
						fetched: this._previewTiles[it].fetched
					};
				}
			}
			for (it = 0; it < app.impress.partList.length; it++) {
				const newHash = app.impress.partList[it].hash;
				if (this._previewTiles[it].hash !== newHash) {
					this._previewTiles[it].hash = newHash;
					const knownPreview = newHash ? previewByHash[newHash] : undefined;
					if (knownPreview) {
						this._previewTiles[it].src = knownPreview.src;
						this._previewTiles[it].fetched = knownPreview.fetched;
					} else {
						this._map.getPreview(it, it, this.options.maxWidth, this.options.maxHeight, {autoUpdate: this.options.autoUpdate});
					}
				}
			}
			// A moved preview that was never fetched keeps its placeholder;
			// fetch the ones that are in view now.
			this._ensureVisiblePreviews();
		}
	},

	_resize: function () {
		if (this._height == window.innerHeight &&
		    this._width == window.innerWidth)
			return;

		if (this._previewInitialized) {
			clearTimeout(this._resizeTimer);
			this._resizeTimer = setTimeout(window.L.bind(this._onScroll, this), 50);
		}

		this._height = window.innerHeight;
		this._width = window.innerWidth;
	},

	// Switch the slide list between the narrow vertical strip and a wide
	// grid. The grid shows bigger thumbnails, so the previews are re-fetched
	// at a larger size to stay crisp.
	setGridMode: function (enabled) {
		enabled = !!enabled;
		if (this._gridMode === enabled)
			return;
		this._gridMode = enabled;

		var wrapper = window.L.DomUtil.get('presentation-controls-wrapper');
		if (wrapper) {
			if (enabled)
				window.L.DomUtil.addClass(wrapper, 'parts-preview-grid');
			else
				window.L.DomUtil.removeClass(wrapper, 'parts-preview-grid');
			// The wrapper shows and hides through its inline display value, so
			// the layout change writes that same value: 'flex' stacks the
			// slide list and the toolbar as a column for the grid, 'block' is
			// the plain flow of the strip. A hidden wrapper stays hidden.
			if (wrapper.style.display !== 'none')
				wrapper.style.display = enabled ? 'flex' : 'block';
		}

		var listSize = window.mode.isDesktop() ? 180 : (window.mode.isTablet() ? 120 : 60);
		this.options.maxWidth = enabled ? 256 : listSize;
		this.options.maxHeight = this.options.maxWidth;

		if (this._previewInitialized)
			this._invalidateParts();
	},

	_beforeRequestPreview: function (e) {
		if (e.part !== undefined && e.part >= 0 && e.part < this._previewTiles.length) {
			if (this._previewTiles[e.part].src === document.querySelector('meta[name="previewSmile"]').content)
				this._previewTiles[e.part].src = document.querySelector('meta[name="previewImg"]').content;
		}
	},

	_updatePreview: function (e) {
		if (this._map.isPresentationOrDrawing()) {
			this._map._previewRequestsOnFly--;
			if (this._map._previewRequestsOnFly < 0) {
				this._map._previewRequestsOnFly = 0;
				this._map._timeToEmptyQueue = new Date();
			}
			this._map._processPreviewQueue();
			if (!this._previewInitialized)
				return;
			if (e.uniqueId === undefined)
				return;
			// The response names the slide it rendered, so the image lands on
			// that slide wherever it now sits, even if the parts were
			// renumbered while the request was under way. A slide deleted in
			// the meantime has no preview left, so its image is dropped.
			const tile = this._previewTiles.find(function (candidate) {
				return candidate.hash === e.uniqueId;
			});
			if (tile) {
				tile.src = e.tile.src;
				tile.fetched = true;
				window.app.console.debug('PREVIEW: part fetched : ' + parseInt(e.id));
			}
		}
	},

	_insertPreview: function (e) {
		if (this._map.isPresentationOrDrawing()) {
			// The frames are about to change; a drag in progress cannot
			// keep its collapsed frames and gap bookkeeping.
			this._finishDrag(false);
			var newIndex = e.selectedPart + 1;
			var newPreview = this._createPreview(newIndex, (e.hashCode === undefined ? null : e.hashCode));

			// insert newPreview to newIndex position
			this._previewTiles.splice(newIndex, 0, newPreview);
			// newPreview already carries the right label from _createPreview;
			// everything after it moved up by one position.
			this._updatePreviewPositionLabelsFrom(newIndex + 1);

			var newFrame = newPreview.parentNode;
			if (e.selectedPart >= 0) {
				// after selectedPart's frame (null sibling appends at the end)
				var selectedFrame = this._previewTiles[e.selectedPart].parentNode;
				selectedFrame.parentNode.insertBefore(newFrame, selectedFrame.nextSibling);
			} else {
				// no preceding slide: before the old first tile, now at index 1
				var followingFrame = this._previewTiles[1] ? this._previewTiles[1].parentNode : null;
				newFrame.parentNode.insertBefore(newFrame, followingFrame);
			}

			this._ensureVisiblePreviews(); // Load previews
		}
	},

	_deletePreview: function (e) {
		if (this._map.isPresentationOrDrawing()) {
			// The frames are about to change; a drag in progress cannot
			// keep its collapsed frames and gap bookkeeping.
			this._finishDrag(false);
			var selectedFrame = this._previewTiles[e.selectedPart].parentNode;
			window.L.DomUtil.remove(selectedFrame);

			this._previewTiles.splice(e.selectedPart, 1);
			// Everything after the removed preview moved down by one position.
			this._updatePreviewPositionLabelsFrom(e.selectedPart);
			if (this._map._deletePageFromPreview || this.partsFocused)
				this.focusCurrentSlide(); // came from focus + Delete key -> keep focus in preview
			else
				this._map.focus();

			this._map._deletePageFromPreview = false;
		}
	},

	_ensureVisiblePreviews: function () {
		if (this._previewTimer) clearTimeout(this._previewTimer);
		this._previewTimer = setTimeout(window.L.bind(function () {
			for (var i = 0; i < this._previewTiles.length; ++i) {
				if (this._isPreviewVisible(i)) {
					var img = this._previewTiles[i];
					if (img && !img.fetched) {
						this._map.getPreview(i, i, this.options.maxWidth, this.options.maxHeight, {autoUpdate: this.options.autoUpdate});
					}
				}
			}
		}, this), 0);
	},

	_onScroll: function () {
		this._ensureVisiblePreviews();
	},

	_isPreviewVisible: function(part) {
		var el = this._previewTiles[part];
		if (!el)
			return false;

		var elemRect = el.getBoundingClientRect();
		var viewRect = document.getElementById('slide-sorter').getBoundingClientRect();

		return (elemRect.left <= viewRect.right &&
			viewRect.left <= elemRect.right &&
			elemRect.top <= viewRect.bottom &&
			viewRect.top <= elemRect.bottom)
	},

	_addDnDHandlers: function (elem) {
		if (app.file.fileBasedView) // No drag & drop for pdf files and the like.
			return;

		if (elem) {
			elem.setAttribute('draggable', true);
			elem.addEventListener('dragstart', this._handleDragStart, false);
			elem.addEventListener('dragend', this._handleDragEnd, false);
			elem.partsPreview = this;
		}
	},

	// The insertion point moves with the pointer rather than being tied to
	// the frame under it, so the drag position and the drop are handled on
	// the whole sorter.
	_addContainerDnDHandlers: function () {
		if (app.file.fileBasedView || this._containerDnDHandlersAdded)
			return;

		this._partsPreviewCont.addEventListener('dragover', this._handleContainerDragOver.bind(this), false);
		this._partsPreviewCont.addEventListener('dragleave', this._handleContainerDragLeave.bind(this), false);
		this._partsPreviewCont.addEventListener('drop', this._handleContainerDrop.bind(this), false);
		this._containerDnDHandlersAdded = true;
	},

	_addDnDTouchHandlers: function (e) {
		$(e.target).bind('touchmove', this._handleTouchMove.bind(this));
		$(e.target).bind('touchcancel', this._handleTouchCancel.bind(this));
		$(e.target).bind('touchend', this._handleTouchEnd.bind(this));

		// To avoid having to add a new message to move an arbitrary part, let's select the
		// slide that is being dragged.
		var part = this._findClickedPart(e.target.parentNode);
		if (part !== null) {
			var partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.
			this._map.setPart(partId);
			this._map.selectPart(partId, 1, false); // And select.
		}
		this.draggedSlide = window.L.DomUtil.create('img', '', document.body);
		this.draggedSlide.setAttribute('src', e.target.currentSrc);
		$(this.draggedSlide).css('position', 'absolute');
		$(this.draggedSlide).css('height', e.target.height);
		$(this.draggedSlide).css('width', e.target.width);
		$(this.draggedSlide).css('left', e.center.x - (e.target.width/2));
		$(this.draggedSlide).css('top', e.center.y - e.target.height);
		$(this.draggedSlide).css('z-index', '10');
		$(this.draggedSlide).css('opacity', '75%');
		$(this.draggedSlide).css('pointer-events', 'none');
		$('.preview-img').css('pointer-events', 'none');

		this.currentNode = null;
		this.previousNode = null;
	},

	_removeDnDTouchHandlers: function (e) {
		$(e.target).unbind('touchmove');
		$(e.target).unbind('touchcancel');
		$(e.target).unbind('touchend');
		$('.preview-img').css('pointer-events', '');
	},

	_handleTouchMove: function (e) {
		if (e.preventDefault) {
			e.preventDefault();
		}

		this.currentNode = document.elementFromPoint(e.originalEvent.touches[0].clientX, e.originalEvent.touches[0].clientY);

		if (this.currentNode !== this.previousNode && this.previousNode !== null) {
			$('.preview-frame').removeClass('preview-img-dropsite');
		}

		if (this.currentNode.draggable || this.currentNode.id === 'first-drop-site') {
			this.currentNode.classList.add('preview-img-dropsite');
		}

		this.previousNode = this.currentNode;

		$(this.draggedSlide).css('left', e.originalEvent.touches[0].clientX - (e.target.width/2));
		$(this.draggedSlide).css('top', e.originalEvent.touches[0].clientY - e.target.height);
		return false;
	},

	_handleTouchCancel: function(e) {
		$('.preview-frame').removeClass('preview-img-dropsite');
		$(this.draggedSlide).remove();
		this._removeDnDTouchHandlers(e);
	},

	_handleTouchEnd: function (e) {
		if (e.stopPropagation) {
			e.stopPropagation();
		}
		if (this.currentNode) {
			var part = this._findClickedPart(this.currentNode);
			if (part !== null) {
				var partId = parseInt(part) - 1; // First frame is a drop-site for reordering.
				if (partId < 0)
					partId = -1; // First item is -1.
				app.socket.sendMessage('moveselectedclientparts position=' + partId);
			}
		}
		$('.preview-frame').removeClass('preview-img-dropsite');
		$(this.draggedSlide).remove();
		this._removeDnDTouchHandlers(e);
		return false;
	},

	_handleDragStart: function (e) {
		const partsPreview = this.partsPreview;
		if (!partsPreview._map.isEditMode()) {
			e.preventDefault();
			return;
		}
		// To avoid having to add a new message to move an arbitrary part, let's select the
		// slide that is being dragged.
		const targetNode = (e.target.id.startsWith('preview') ? e.target : e.target.parentNode);
		const part = partsPreview._findClickedPart(targetNode);
		const partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.
		if (part === -1 || partId < 0) {
			e.preventDefault();
			return;
		}
		const alreadySelected = app.impress.isSlideSelected(partId);
		if (partsPreview._map._docLayer && !alreadySelected)
		{
			// Same selection change as a plain click on the slide: the
			// previous selection is dropped on the client and the server
			// alike, so both agree on which slides the move applies to.
			partsPreview._map.deselectAll();
			partsPreview._map.setPart(partId);
			partsPreview._map.selectPart(partId, 1, false); // And select.
		}
		// By default we move when dragging, but can
		// support duplication with ctrl in the future.
		e.dataTransfer.effectAllowed = 'move';
		// The drag carries only the custom type. A drag that starts on the
		// preview picture is a native image drag, which the browser preloads
		// with the picture's URL as text and HTML; clearing that keeps a drop
		// on a text input or another application from pasting the preview
		// data there.
		e.dataTransfer.clearData();
		e.dataTransfer.setData('application/x-cool-slide', String(partId));

		partsPreview._beginDrag(partId, alreadySelected);

		// The drag ghost is the grabbed slide's picture at its on-screen
		// size with the standard preview border, held at the grab point;
		// the other dragged slides stack behind it and a badge counts them.
		const img = partsPreview._previewTiles[partId];
		if (img && e.dataTransfer.setDragImage) {
			const rect = img.getBoundingClientRect();
			const draggedParts = partsPreview._dragState.draggedParts;
			const sources = [partId]
				.concat(draggedParts.filter(function (part) { return part !== partId; }))
				.map(function (part) { return partsPreview._previewTiles[part].src; });
			const ghost = buildSlideDragGhost(sources, rect.width, rect.height,
				draggedParts.length);
			document.body.appendChild(ghost);
			e.dataTransfer.setDragImage(ghost, e.clientX - rect.left, e.clientY - rect.top);
			// The snapshot is taken when the dragstart handler returns;
			// the ghost element itself is no longer needed after that.
			setTimeout(function () { ghost.remove(); }, 0);
		}
	},

	// Collect the slides taking part in the drag and start the drag
	// visuals: the dragged frames close smoothly, so the remaining slides
	// move in to take their place.
	_beginDrag: function (partId, includeSelection) {
		const draggedParts = [];
		if (includeSelection) {
			for (let i = 0; i < this._previewTiles.length; i++) {
				if (i === partId || app.impress.isSlideSelected(i))
					draggedParts.push(i);
			}
		} else {
			draggedParts.push(partId);
		}

		// In the horizontal sorter layout the frames close and the gap
		// opens along the x axis.
		const sizeProperty = this._direction === 'x' ? 'width' : 'height';
		const frames = [];
		const sizes = [];
		let gapSize = 0;
		for (let i = 0; i < draggedParts.length; i++) {
			const frame = this._previewTiles[draggedParts[i]].parentNode;
			const size = frame.getBoundingClientRect()[sizeProperty];
			frames.push(frame);
			sizes.push(size);
			gapSize += size;
		}
		// In the grid layout the gap is one empty cell, so it takes a
		// single slide's height rather than the dragged frames' sum.
		if (this._gridMode && sizes.length)
			gapSize = sizes[0];

		this._dragState = {
			draggedParts: draggedParts,
			frames: frames,
			sizes: sizes,
			sizeProperty: sizeProperty,
			grid: this._gridMode,
			gapSize: gapSize,
			gapFrame: null,
			gapSide: null,
			gapPlaceholder: null,
			insertIndex: null,
			intoSection: null,
			pointer: null,
			autoScrollId: null
		};

		this._partsPreviewCont.classList.add('dragging-slide');

		// The browser takes its snapshot for the drag ghost when the
		// dragstart handler returns, so the collapse runs afterwards and
		// the ghost shows the frame at full size.
		const state = this._dragState;
		setTimeout(() => {
			if (this._dragState !== state)
				return;
			// A grid frame keeps occupying its cell at zero size, so in the
			// grid the dragged frames leave the flow entirely and the
			// remaining slides slide into the freed cells.
			if (state.grid) {
				this._animateGridChange(function () {
					for (let i = 0; i < frames.length; i++)
						frames[i].style.display = 'none';
				});
				this._schedulePreviewRefresh();
				return;
			}
			for (let i = 0; i < frames.length; i++) {
				frames[i].style.boxSizing = 'border-box';
				frames[i].style[sizeProperty] = sizes[i] + 'px';
				frames[i].style.overflow = 'hidden';
			}
			// Give the frames their measured size first, so the change
			// animates from the real value.
			void this._partsPreviewCont.offsetHeight;
			for (let i = 0; i < frames.length; i++) {
				frames[i].style[sizeProperty] = '0px';
				frames[i].style.padding = '0px';
				frames[i].style.opacity = '0';
			}
			// Closing the dragged frames pulls slides from below the view
			// into it; load their previews once the frames have closed.
			this._schedulePreviewRefresh();
		}, 0);
	},

	// Load the previews that a layout change brought into view, once the
	// size and margin transitions have settled. Debounced, and fetching
	// stays limited to slides that are visible and have no preview yet.
	_schedulePreviewRefresh: function () {
		clearTimeout(this._layoutPreviewTimer);
		this._layoutPreviewTimer = setTimeout(() => {
			this._layoutPreviewTimer = null;
			this._ensureVisiblePreviews();
		}, 200);
	},

	_handleContainerDragOver: function (e) {
		const state = this._dragState;
		// A drag that did not start on a slide (a file from the desktop,
		// for example) has no drop target here; refusing it keeps the
		// browser from opening the file in place of the editor.
		if (!state) {
			e.preventDefault();
			if (e.dataTransfer)
				e.dataTransfer.dropEffect = 'none';
			return;
		}

		e.preventDefault();
		if (e.dataTransfer)
			e.dataTransfer.dropEffect = 'move';

		// dragover keeps firing while the pointer rests; the gap only
		// needs recomputing when the pointer moved.
		if (state.pointer && state.pointer.x === e.clientX && state.pointer.y === e.clientY)
			return;

		state.pointer = { x: e.clientX, y: e.clientY };
		this._updateDropGap();
		this._ensureAutoScroll();
	},

	_handleContainerDragLeave: function (e) {
		const state = this._dragState;
		if (!state)
			return;
		// Moving between children of the sorter also fires dragleave, on
		// some browsers with a null relatedTarget; the gap only closes
		// when the pointer really left the sorter area.
		if (e.relatedTarget && this._partsPreviewCont.contains(e.relatedTarget))
			return;
		const rect = this._partsPreviewCont.getBoundingClientRect();
		if (e.clientX >= rect.left && e.clientX < rect.right &&
		    e.clientY >= rect.top && e.clientY < rect.bottom)
			return;
		this._closeDropGap(state);
		state.insertIndex = null;
		state.pointer = null;
	},

	_handleContainerDrop: function (e) {
		const state = this._dragState;
		if (!state) {
			e.preventDefault();
			return;
		}
		e.preventDefault();
		e.stopPropagation();

		const insertIndex = state.insertIndex;
		if (insertIndex === null) {
			this._finishDrag(true);
			return;
		}

		// Insert after the slide preceding the gap; -1 inserts before the
		// first slide. On a section boundary intoSection names the section
		// whose first slides the dropped slides become; without it they
		// land above the section, outside it.
		let message = 'moveselectedclientparts position=' + (insertIndex - 1);
		if (state.intoSection !== null)
			message += ' intoSection=' + state.intoSection;
		app.socket.sendMessage(message);

		this._applyDropLocally(insertIndex, state.draggedParts,
			state.intoSection !== null);
		this._finishDrag(false);
	},

	// Slide the sorter's children from where they are into the places a
	// layout change gives them. The grid re-places whole cells in one
	// step, so each moved child briefly keeps its old position as a
	// transform and then slides from there into its new cell. mutate()
	// applies the layout change.
	_animateGridChange: function (mutate) {
		if (window.matchMedia &&
		    window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
			mutate();
			return;
		}

		const container = this._partsPreviewCont;
		const children = Array.prototype.slice.call(container.children);
		const before = new Map();
		for (let i = 0; i < children.length; i++) {
			const el = children[i];
			// A change arriving mid-slide starts from the spot the child
			// has reached: the rect includes the running transform, which
			// then resets so the new layout measures clean.
			before.set(el, el.getBoundingClientRect());
			if (el._gridSlideTimer) {
				clearTimeout(el._gridSlideTimer);
				el._gridSlideTimer = null;
			}
			el.style.transition = '';
			el.style.transform = '';
		}

		mutate();

		const moved = [];
		for (let i = 0; i < container.children.length; i++) {
			const el = container.children[i];
			const from = before.get(el);
			if (!from || (!from.width && !from.height))
				continue;
			const to = el.getBoundingClientRect();
			if (!to.width && !to.height)
				continue;
			const deltaX = from.left - to.left;
			const deltaY = from.top - to.top;
			if (!deltaX && !deltaY)
				continue;
			el.style.transition = 'none';
			el.style.transform =
				'translate(' + deltaX + 'px, ' + deltaY + 'px)';
			moved.push(el);
		}
		if (!moved.length)
			return;

		// With the old positions frozen in as transforms, releasing them
		// after a reflow slides each child into its new cell.
		void container.offsetHeight;
		for (let i = 0; i < moved.length; i++) {
			const el = moved[i];
			el.style.transition = 'transform 0.15s ease-out';
			el.style.transform = '';
			el._gridSlideTimer = setTimeout(() => {
				el.style.transition = '';
				el._gridSlideTimer = null;
			}, 200);
		}
	},

	// Close the insertion gap; in the grid the surrounding slides move
	// back into the freed cells.
	_closeDropGap: function (state) {
		if (!state.gapFrame)
			return;
		if (state.grid)
			this._animateGridChange(() => this._removeDropGap(state));
		else
			this._removeDropGap(state);
	},

	// The header of the section whose first slide sits at the given slide
	// index, or null when no section starts there.
	_sectionHeaderAt: function (slideIndex) {
		const sections = (app.impress && app.impress.sections) || [];
		for (let h = 0; h < this._sectionHeaders.length; h++) {
			const header = this._sectionHeaders[h];
			const s = parseInt(header.getAttribute('data-section-index'), 10);
			if (sections[s] && sections[s].startIndex === slideIndex)
				return header;
		}
		return null;
	},

	// Choose the insertion point from the pointer position and open a
	// slide-sized gap there. The gap is a margin on the frame the dragged
	// slides would be inserted before (or after the last frame, when the
	// pointer is past it), and it moves along with the pointer.
	_updateDropGap: function () {
		const state = this._dragState;
		if (!state || !state.pointer)
			return;

		const horizontal = this._direction === 'x';
		const rtl = document.documentElement.dir === 'rtl';
		const pointerPosition = horizontal ? state.pointer.x : state.pointer.y;

		let insertIndex = this._previewTiles.length;
		let gapFrame = null;
		let gapSide = 'before';
		let lastFrame = null;
		for (let i = 0; i < this._previewTiles.length; i++) {
			if (state.draggedParts.indexOf(i) !== -1)
				continue;
			const frame = this._previewTiles[i].parentNode;
			if (frame.classList.contains('section-collapsed'))
				continue;
			const rect = frame.getBoundingClientRect();
			let pointerBefore;
			if (state.grid) {
				// The grid flows in reading order: the pointer comes before
				// a frame when it is above the frame's row, or in the same
				// row on the near side of the frame's middle.
				const middleX = rect.left + rect.width / 2;
				const beforeInRow = rtl ?
					state.pointer.x > middleX : state.pointer.x < middleX;
				pointerBefore = state.pointer.y < rect.top ||
					(state.pointer.y < rect.bottom && beforeInRow);
			} else {
				const middle = horizontal ?
					rect.left + rect.width / 2 : rect.top + rect.height / 2;
				pointerBefore = pointerPosition < middle;
			}
			if (pointerBefore) {
				insertIndex = i;
				gapFrame = frame;
				break;
			}
			lastFrame = frame;
		}
		if (!gapFrame && lastFrame) {
			gapFrame = lastFrame;
			gapSide = 'after';
		}

		// The same insertion index means two spots when a section starts
		// there: the last spot outside the section, above its header, and
		// the spot of the section's first slide, below it. The side of the
		// header's middle the pointer is on picks between them: above it
		// the slides land outside the section and the gap opens above the
		// header; below it they become the section's first slides and the
		// gap opens between the header and the slide.
		let intoSection = null;
		const boundaryHeader = (gapFrame && gapSide === 'before') ?
			this._sectionHeaderAt(insertIndex) : null;
		if (boundaryHeader) {
			const rect = boundaryHeader.getBoundingClientRect();
			// A grid header spans its own full row, so the grid boundary
			// reads vertically even though the slides flow in rows.
			const useX = horizontal && !state.grid;
			const middle = useX ?
				rect.left + rect.width / 2 : rect.top + rect.height / 2;
			const pointer = useX ? state.pointer.x : state.pointer.y;
			if (pointer < middle)
				gapFrame = boundaryHeader;
			else
				intoSection = parseInt(
					boundaryHeader.getAttribute('data-section-index'), 10);
		}

		state.insertIndex = insertIndex;
		state.intoSection = intoSection;
		if (gapFrame === state.gapFrame && gapSide === state.gapSide)
			return;

		const applyGap = () => {
			this._removeDropGap(state);
			state.gapFrame = gapFrame;
			state.gapSide = gapSide;
			if (!gapFrame)
				return;
			if (state.grid) {
				// A margin on a grid item stays inside its own cell and
				// moves no neighbour, so the grid gap is an empty cell
				// inserted into the flow where the slides would land.
				const cell = document.createElement('div');
				cell.className = 'drop-gap-cell';
				cell.style.height = state.gapSize + 'px';
				this._partsPreviewCont.insertBefore(cell,
					gapSide === 'before' ? gapFrame : gapFrame.nextSibling);
				state.gapPlaceholder = cell;
			} else {
				gapFrame.style[this._gapMarginProperty(gapSide)] = state.gapSize + 'px';
			}
		};
		// A gap move re-places the slides around it in one step; the
		// animation slides them from their old cells into the new ones.
		if (state.grid)
			this._animateGridChange(applyGap);
		else
			applyGap();

		// A gap move shifts the slides around it; load the previews of the
		// ones it brought into view.
		this._schedulePreviewRefresh();
	},

	_gapMarginProperty: function (side) {
		if (this._direction === 'x')
			return side === 'before' ? 'marginLeft' : 'marginRight';
		return side === 'before' ? 'marginTop' : 'marginBottom';
	},

	_removeDropGap: function (state) {
		if (!state || !state.gapFrame)
			return;
		if (state.gapPlaceholder) {
			state.gapPlaceholder.remove();
			state.gapPlaceholder = null;
		} else {
			state.gapFrame.style[this._gapMarginProperty(state.gapSide)] = '';
		}
		state.gapFrame = null;
		state.gapSide = null;
	},

	// Scroll the sorter when the pointer is near one of its edges during a
	// drag, so slides outside the visible area can be reached. Runs as an
	// animation frame loop; the speed grows the closer the pointer is to
	// the edge, and the loop ends when the pointer moves away from it.
	_autoScrollStep: function () {
		const state = this._dragState;
		if (!state)
			return;
		state.autoScrollId = null;
		if (!state.pointer)
			return;

		const horizontal = this._direction === 'x';
		const rect = this._partsPreviewCont.getBoundingClientRect();
		const zone = 48;
		const maxSpeed = 14;
		const pointerPosition = horizontal ? state.pointer.x : state.pointer.y;
		const start = horizontal ? rect.left : rect.top;
		const end = horizontal ? rect.right : rect.bottom;

		let speed = 0;
		if (pointerPosition < start + zone)
			speed = -maxSpeed * Math.min(1, (start + zone - pointerPosition) / zone);
		else if (pointerPosition > end - zone)
			speed = maxSpeed * Math.min(1, (pointerPosition - (end - zone)) / zone);
		if (!speed)
			return;

		if (horizontal)
			this._partsPreviewCont.scrollLeft += speed;
		else
			this._partsPreviewCont.scrollTop += speed;

		// Scrolling moved the frames under the pointer.
		this._updateDropGap();

		state.autoScrollId = requestAnimationFrame(this._autoScrollStep.bind(this));
	},

	_ensureAutoScroll: function () {
		const state = this._dragState;
		if (state && state.autoScrollId === null)
			state.autoScrollId = requestAnimationFrame(this._autoScrollStep.bind(this));
	},

	// Rearrange the local previews into the order the server will confirm.
	// The sorter settles instantly, each preview image travels with its
	// slide, and the status update that follows finds every preview
	// already at its part's index. intoSection is true when the slides
	// become the first slides of the section starting at the insertion
	// point, and false when they stay outside it.
	_applyDropLocally: function (insertIndex, draggedParts, intoSection) {
		const draggedTiles = [];
		for (let i = 0; i < draggedParts.length; i++)
			draggedTiles.push(this._previewTiles[draggedParts[i]]);
		// The frames land in front of the first slide at or after the
		// insertion point that is not itself being dragged.
		let anchorIndex = insertIndex;
		while (anchorIndex < this._previewTiles.length &&
		       draggedParts.indexOf(anchorIndex) !== -1)
			anchorIndex++;
		const anchorTile = anchorIndex < this._previewTiles.length ?
			this._previewTiles[anchorIndex] : null;

		// The client's part bookkeeping moves along, so selection changes
		// made before the server confirms the move already address the
		// parts at their new indices.
		const parts = app.impress.partList;
		if (parts && parts.length === this._previewTiles.length) {
			const draggedEntries = [];
			for (let i = 0; i < draggedParts.length; i++)
				draggedEntries.push(parts[draggedParts[i]]);
			const anchorEntry = anchorIndex < parts.length ? parts[anchorIndex] : null;
			const remainingEntries = parts.filter(function (entry) {
				return draggedEntries.indexOf(entry) === -1;
			});
			const entriesInsertAt = anchorEntry ?
				remainingEntries.indexOf(anchorEntry) : remainingEntries.length;
			Array.prototype.splice.apply(remainingEntries,
				[entriesInsertAt, 0].concat(draggedEntries));
			app.impress.partList = remainingEntries;
		}

		this._previewTiles = this._previewTiles.filter(function (tile) {
			return draggedTiles.indexOf(tile) === -1;
		});
		let anchorNode = anchorTile ? anchorTile.parentNode : null;
		// When the slides stay outside the section that starts at the
		// anchor, they also go in front of its header.
		if (!intoSection && anchorNode) {
			const nodeBefore = anchorNode.previousElementSibling;
			if (nodeBefore && nodeBefore.classList.contains('slide-section-header'))
				anchorNode = nodeBefore;
		}
		for (let i = 0; i < draggedTiles.length; i++) {
			const frame = draggedTiles[i].parentNode;
			// Moving takes a slide out of its section; when it lands in a
			// collapsed one, the next sections update hides it again.
			frame.classList.remove('section-collapsed');
			this._partsPreviewCont.insertBefore(frame, anchorNode);
		}
		const insertAt = anchorTile ?
			this._previewTiles.indexOf(anchorTile) : this._previewTiles.length;
		Array.prototype.splice.apply(this._previewTiles, [insertAt, 0].concat(draggedTiles));

		// The slide numbers in the alt text and tooltip are positional,
		// so renumber every preview after the previews change order.
		this._updatePreviewPositionLabelsFrom(0);
	},

	// End the drag visuals. After a completed drop (animate = false) the
	// dragged frames have just been moved into the gap, so restoring their
	// size while the gap margin goes away in the same step is not a visible
	// change. After a cancelled drag (animate = true) the gap closes and
	// the dragged frames grow back at their original position.
	_finishDrag: function (animate) {
		const state = this._dragState;
		if (!state)
			return;
		this._dragState = null;

		if (state.autoScrollId !== null)
			cancelAnimationFrame(state.autoScrollId);

		const container = this._partsPreviewCont;
		const restoreFrame = function (frame) {
			frame.style.display = '';
			frame.style.width = '';
			frame.style.height = '';
			frame.style.padding = '';
			frame.style.opacity = '';
			frame.style.overflow = '';
			frame.style.boxSizing = '';
		};

		// A grid drag settles by sliding the slides from their current
		// cells into the ones the closed gap and the returning frames
		// give them. The frames come back whole, so there is no size to
		// grow back.
		if (state.grid) {
			this._animateGridChange(() => {
				this._removeDropGap(state);
				state.frames.forEach(restoreFrame);
			});
			container.classList.remove('dragging-slide');
			this._schedulePreviewRefresh();
			return;
		}

		if (!animate)
			container.classList.add('drag-no-transition');

		this._removeDropGap(state);

		if (!animate) {
			state.frames.forEach(restoreFrame);
			void container.offsetHeight;
			container.classList.remove('drag-no-transition');
			container.classList.remove('dragging-slide');
			this._schedulePreviewRefresh();
			return;
		}

		// Grow the frames back to their measured size so the change
		// animates, then drop the inline styles once the transition is over.
		for (let i = 0; i < state.frames.length; i++) {
			state.frames[i].style[state.sizeProperty] = state.sizes[i] + 'px';
			state.frames[i].style.padding = '';
			state.frames[i].style.opacity = '';
		}
		const that = this;
		setTimeout(function () {
			// A drag that started during the restore owns the drag visuals:
			// its frames keep their styles and the container keeps its class.
			const activeDrag = that._dragState;
			for (let i = 0; i < state.frames.length; i++) {
				if (!activeDrag || activeDrag.frames.indexOf(state.frames[i]) === -1)
					restoreFrame(state.frames[i]);
			}
			if (!activeDrag)
				container.classList.remove('dragging-slide');
			that._schedulePreviewRefresh();
		}, 200);
	},

	_handleDragEnd: function () {
		// Reached without a drop when the drag was cancelled or released
		// outside a drop target: put the dragged slides back where they were.
		this.partsPreview._finishDrag(true);
	},

	_invalidateParts: function () {
		if (!this._container ||
		    !this._partsPreviewCont ||
		    !this._previewInitialized ||
		    !this._previewTiles)
			return;

		for (var part = 0; part < this._previewTiles.length; part++) {
			this._previewTiles[part].fetched = false;
			var imgSize = this._map.getPreview(part, part,
					     this.options.maxWidth,
					     this.options.maxHeight,
					     {autoUpdate: this.options.autoUpdate,
					      fetchThumbnail: this.options.fetchThumbnail});
			window.L.DomUtil.setStyle(this._previewTiles[part], 'width', imgSize.width + 'px');
			window.L.DomUtil.setStyle(this._previewTiles[part], 'height', imgSize.height + 'px');
		}

	},

	_invalidateCurrentPart: function () {
		if (!this._container ||
		    !this._partsPreviewCont ||
		    !this._previewInitialized ||
		    !this._previewTiles)
			return;

		// When a new slide is inserted
		if (this._previewTiles[this._map._docLayer._selectedPart] === undefined) {
			this._invalidateParts();
			return;
		}
		this._previewTiles[this._map._docLayer._selectedPart].fetched = false;
		this._map.getPreview(this._map._docLayer._selectedPart, this._map._docLayer._selectedPart,
				     this.options.maxWidth,
				     this.options.maxHeight,
				     {autoUpdate: this.options.autoUpdate,
				      fetchThumbnail: this.options.fetchThumbnail});
	},

	focusCurrentSlide: function () {
		if (this._previewTiles[this._map._docLayer._selectedPart])
			this._previewTiles[this._map._docLayer._selectedPart].focus();
	},

	// On load, move focus to the current slide preview so arrow keys
	// navigate the slides right away. Only take focus while it still rests
	// on the document itself; if the user has already moved it to another
	// control, leave it where they put it.
	_focusCurrentSlideOnLoad: function (e) {
		if (!e || !e.status)
			return;
		if (!this._previewInitialized)
			return;

		var active = document.activeElement;
		if (active && active !== document.body &&
		    active !== this._map.getContainer() && !this._map.hasFocus())
			return;

		this.focusCurrentSlide();
		this.partsFocused = true;
	},
});

window.L.control.partsPreview = function (container, preview, options) {
	return new window.L.Control.PartsPreview(container, preview, options);
};
