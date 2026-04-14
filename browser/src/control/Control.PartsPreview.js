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

/* global _ app $ Hammer _UNO cool */
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
		this._partsPreviewCont.onscroll = this._onScroll.bind(this);
		this._idNum = 0;
		this._width = 0;
		this._height = 0;
		this.scrollTimer = null;

		document.body.addEventListener('click', (e) => {
			if (!e.partsFocusedApplied && this.partsFocused)
				this.partsFocused = false;
		});
	},

	onAdd: function (map) {
		this._previewInitialized = false;
		this._previewTiles = [];
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
				// make room for the preview
				var docContainer = this._map.options.documentContainer;

				if (!window.L.DomUtil.hasClass(docContainer, 'parts-preview-document'))
					window.L.DomUtil.addClass(docContainer, 'parts-preview-document');

				// Add a special frame just as a drop-site for reordering.
				var frameClass = 'preview-frame ' + this.options.frameClass;
				var frame = window.L.DomUtil.create('div', frameClass, this._partsPreviewCont);
				this._addDnDHandlers(frame);
				frame.setAttribute('draggable', false);
				frame.setAttribute('id', 'first-drop-site');

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
			}

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

	_createPreview: function (i, hashCode) {
		var frameClass = 'preview-frame ' + this.options.frameClass;
		var frame = window.L.DomUtil.create('div', frameClass, this._partsPreviewCont);
		frame.id = 'preview-frame-part-' + this._idNum;
		this._addDnDHandlers(frame);
		window.L.DomUtil.create('span', 'preview-helper', frame);

		var imgClassName = 'preview-img ' + this.options.imageClass;
		var img = window.L.DomUtil.create('img', imgClassName, frame);
		img.setAttribute('alt', _('preview of page %1').replace('%1', String(i + 1)));
		img.setAttribute('tabindex', '0');
		img.setAttribute('data-cooltip', _('Slide %1').replace('%1', String(i + 1)));
		window.L.control.attachTooltipEventListener(img, this._map);
		img.id = 'preview-img-part-' + this._idNum;
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
			if (app.file.fileBasedView)
				this._map._docLayer._checkSelectedPart();
			img.focus();
		}, this);

		var that = this;
		img.onfocus = function (e) {
			that._map._clip.clearSelection();
			that._map._clip.setTextSelectionType('slide');
			that.partsFocused = true;
			e.partsFocusedApplied = true;
		};

		var that = this;
		window.L.DomEvent.on(frame, 'contextmenu', function(e) {
			var isMasterView = this._map['stateChangeHandler'].getItemValue('.uno:SlideMasterPage');
			var pcw = document.getElementById('presentation-controls-wrapper');
			var $trigger = $(pcw);
			if (isMasterView === 'true' || app.map.isReadOnlyMode()) {
				$trigger.contextMenu(false);
				return;
			}

			var nPos = undefined;
			if (this.isPaddingClick(frame, e, 'top'))
				nPos = that._findClickedPart(frame) - 1;
			else if (this.isPaddingClick(frame, e, 'bottom'))
				nPos = that._findClickedPart(frame);
			else if (this.isPaddingClick(frame, e, 'right') || this.isPaddingClick(frame, e, 'left'))
				nPos = that._findClickedPart(frame);

			$trigger.contextMenu(true);
			if (!that._isSelected(e))
				that._setPart(e);
			$.contextMenu({
				selector: '#'+frame.id,
				className: 'cool-font',
				items: {
					paste: {
						name: app.IconUtil.createMenuItemLink(_('Paste'), 'Paste'),
						isHtmlName: true,
						callback: function(key, options) {
								if (nPos === undefined)
									nPos = that._findClickedPart(options.$trigger[0]);
								that._pasteSlide(nPos);
						},
						visible: function() {
							// Show paste if we have a local copied slide OR
							// the system clipboard API is available (may have content from another tab)
							return that.copiedSlide || window.L.Browser.clipboardApiAvailable;
						}
					},
					newslide: {
						name: app.IconUtil.createMenuItemLink( _UNO(that._map._docLayer._docType == 'presentation' ? '.uno:InsertSlide' : '.uno:InsertPage', 'presentation'), 'InsertPage'),
						isHtmlName: true,
						callback: function() { that._map.insertPage(nPos); }
					}
				},
				events: {
					hide: function() {
						img.focus();
					}
				}
			});
		}, this);

		window.L.DomEvent.on(img, 'contextmenu', function(e) {
			var isMasterView = this._map['stateChangeHandler'].getItemValue('.uno:SlideMasterPage');
			var $trigger = $('#' + img.id);
			if (isMasterView === 'true' || app.map.isReadOnlyMode()) {
				$trigger.contextMenu(false);
				return;
			}
			$trigger.contextMenu(true);
			if (!that._isSelected(e))
				that._setPart(e);

			$.contextMenu({
				selector: '#' + img.id,
				className: 'cool-font',
				items: {
					copy: {
						name: app.IconUtil.createMenuItemLink(_('Copy'), 'Copy'),
						isHtmlName: true,
						callback: function() {
							that.copiedSlide = e;
							that._map._clip.clearSelection();
							that._map._clip.setTextSelectionType('slide');
							that._map._clip._execCopyCutPaste('copy', '.uno:CopySlide');
						},
						visible: function() {
							return !(app.impress.hasOverviewPage && that._map._docLayer._selectedPart === 0);
						}
					},
					paste: {
						name: app.IconUtil.createMenuItemLink(_('Paste'), 'Paste'),
						isHtmlName: true,
						callback: function() {
							that._pasteSlide();
						},
					},
					newslide: {
						name: app.IconUtil.createMenuItemLink(_UNO(that._map._docLayer._docType == 'presentation' ? '.uno:InsertSlide' : '.uno:InsertPage', 'presentation'), 'InsertPage'),
						isHtmlName: true,
						callback: function() { that._map.insertPage(); }
					},
					duplicateslide: {
						name: app.IconUtil.createMenuItemLink(_UNO(that._map._docLayer._docType == 'presentation' ? '.uno:DuplicateSlide' : '.uno:DuplicatePage', 'presentation'), 'DuplicatePage'),
						isHtmlName: true,
						callback: function() { that._map.duplicatePage(); }
					},
					delete: {
						name: app.IconUtil.createMenuItemLink(_UNO(that._map._docLayer._docType == 'presentation' ? '.uno:DeleteSlide' : '.uno:DeletePage', 'presentation'), 'DeletePage'),
						isHtmlName: true,
						callback: function() { app.dispatcher.dispatch('deletepage'); },
						visible: function() {
							return that._map._docLayer._parts > 1;
						}
					},
					slideproperties: {
						name: app.IconUtil.createMenuItemLink(_UNO(that._map._docLayer._docType == 'presentation' ? '.uno:SlideSetup' : '.uno:PageSetup', 'presentation'), 'PageSetup'),
						isHtmlName: true,
						callback: function() {
							app.socket.sendMessage('uno .uno:PageSetup');
						}
					},
					showslide: {
						name: app.IconUtil.createMenuItemLink(_UNO('.uno:ShowSlide', 'presentation'), 'ShowSlide'),
						isHtmlName: true,
						callback: function(key, options) {
							var part = that._findClickedPart(options.$trigger[0].parentNode);
							if (part !== null) {
								that._map.showSlide();
							}
						},
						visible: function(key, options) {
							var part = that._findClickedPart(options.$trigger[0].parentNode);
							return that._map._docLayer._docType === 'presentation' && app.impress.isSlideHidden(parseInt(part) - 1);
						}
					},
					hideslide: {
						name: app.IconUtil.createMenuItemLink(_UNO('.uno:HideSlide', 'presentation'), 'Hideslide'),
						isHtmlName: true,
						callback: function(key, options) {
							var part = that._findClickedPart(options.$trigger[0].parentNode);
							if (part !== null) {
								that._map.hideSlide();
							}
						},
						visible: function(key, options) {
							var part = that._findClickedPart(options.$trigger[0].parentNode);
							return that._map._docLayer._docType === 'presentation' && !app.impress.isSlideHidden(parseInt(part) - 1);
						}
					}
				},
				events: {
					hide: function() {
						// Restore focus to the element that opened the menu
						img.focus();
					}
				}
			});
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

	_scrollToPart: function(part) {
		var partNo = part !== undefined ? part : this._map.getCurrentPartNumber();
		//var sliderSize, nodePos, nodeOffset, nodeMargin;
		var node = this._partsPreviewCont.children[partNo];

		if (node && (!this._previewTiles[partNo] || !this._isPreviewVisible(partNo))) {
			if (this.scrollTimer) clearTimeout(this.scrollTimer);

			 this.scrollTimer = setTimeout(() => {
				node.scrollIntoView();
				this.scrollTimer = null;
			}, 50);
		}
	},

	// We will use this function because IE doesn't support "Array.from" feature.
	_findClickedPart: function (element) {
		for (var i = 0; i < this._partsPreviewCont.children.length; i++) {
			if (this._partsPreviewCont.children[i] === element || this._partsPreviewCont.children[i] === element.parentNode) {
				return i;
			}
		}
		return -1;
	},

	// This is used with fileBasedView.
	_scrollViewToPartPosition: function (partNumber, fromBottom) {
		if (this._map._docLayer && this._map._docLayer._isZooming)
			return;

		if (partNumber < 0) partNumber = 0;
		if (partNumber >= this._map._docLayer._parts) partNumber = this._map._docLayer._parts - 1;

		var partHeightPixels = Math.round((this._map._docLayer._partHeightTwips + this._map._docLayer._spaceBetweenParts) * app.twipsToPixels);
		var scrollTop = partHeightPixels * partNumber;
		var viewHeight = app.sectionContainer.getViewSize()[1];
		var currentScrollX = app.activeDocument.activeLayout.viewedRectangle.pX1;

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

	// Paste a slide, preferring the system clipboard for cross-tab pastes.
	// nPos: insertion position for the frame context menu (may be undefined for img context menu).
	_pasteSlide: async function(nPos) {
		// Guard against concurrent invocations (e.g. rapid double-click).
		if (this._pastePending)
			return;
		this._pastePending = true;
		try {
			if (this.copiedSlide) {
				// Check if the system clipboard has been updated by a different
				// tab/session since our last copy. If so, prefer the system clipboard.
				let useInternalCopy = true;
				if (window.L.Browser.clipboardApiAvailable) {
					try {
						const items = await navigator.clipboard.read();
						if (items.length > 0 && items[0].types.includes('text/html')) {
							const blob = await items[0].getType('text/html');
							const html = await blob.text();
							const clip = this._map._clip;
							const meta = clip.getMetaOrigin(html);
							const id = clip.getMetaPath(0);
							const idOld = clip.getMetaPath(1);
							// If meta origin does not match this tab's clipboard, use system clipboard
							if (meta !== '' && (id === '' || meta.indexOf(id) < 0) && (idOld === '' || meta.indexOf(idOld) < 0)) {
								useInternalCopy = false;
							}
						}
					} catch (e) {
						// clipboard read failed or permission denied - keep using internal copy
					}
				}
				if (useInternalCopy) {
					// Same-tab paste: use duplicate which allows insertion at a position
					this._setPart(this.copiedSlide);
					this._map.duplicatePage(nPos);
				} else {
					// System clipboard is from a different tab - use it
					this.copiedSlide = null;
					if (nPos !== undefined)
						this._map.setPart(Math.max(0, nPos - 1));
					this._map._clip.filterExecCopyPaste('.uno:Paste');
				}
			} else {
				// Cross-tab/browser paste: use system clipboard
				if (nPos !== undefined)
					this._map.setPart(Math.max(0, nPos - 1)); // new slide is inserted after set slide
				this._map._clip.filterExecCopyPaste('.uno:Paste');
			}
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

	_selectPartRange: function (start, end) {
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
							this._insertPreview({selectedPart: it, hashCode: app.impress.partList[it].hash});
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
			// update hash code when user click insert slide.
			for (it = 0; it < app.impress.partList.length; it++) {
				if (this._previewTiles[it].hash !== app.impress.partList[it].hash) {
					this._previewTiles[it].hash = app.impress.partList[it].hash;
					this._map.getPreview(it, it, this.options.maxWidth, this.options.maxHeight, {autoUpdate: this.options.autoUpdate});
				}
			}
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

	_beforeRequestPreview: function (e) {
		if (e.part !== undefined && e.part >= 0 && e.part < this._previewTiles.length &&
		   this._previewTiles[e.part].src === document.querySelector('meta[name="previewSmile"]').content)
			this._previewTiles[e.part].src = document.querySelector('meta[name="previewImg"]').content;
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
			if (this._previewTiles[e.id]) {
				this._previewTiles[e.id].src = e.tile.src;
				this._previewTiles[e.id].fetched = true;
				window.app.console.debug('PREVIEW: part fetched : ' + e.id);
			}
		}
	},

	_insertPreview: function (e) {
		if (this._map.isPresentationOrDrawing()) {
			var newIndex = e.selectedPart + 1;
			var newPreview = this._createPreview(newIndex, (e.hashCode === undefined ? null : e.hashCode));

			// insert newPreview to newIndex position
			this._previewTiles.splice(newIndex, 0, newPreview);

			var selectedFrame = this._previewTiles[e.selectedPart].parentNode;
			var newFrame = newPreview.parentNode;

			// insert after selectedFrame
			selectedFrame.parentNode.insertBefore(newFrame, selectedFrame.nextSibling);

			this._ensureVisiblePreviews(); // Load previews
		}
	},

	_deletePreview: function (e) {
		if (this._map.isPresentationOrDrawing()) {
			var selectedFrame = this._previewTiles[e.selectedPart].parentNode;
			window.L.DomUtil.remove(selectedFrame);

			this._previewTiles.splice(e.selectedPart, 1);
			this.focusCurrentSlide();
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
			elem.addEventListener('dragenter', this._handleDragEnter, false);
			elem.addEventListener('dragover', this._handleDragOver, false);
			elem.addEventListener('dragleave', this._handleDragLeave, false);
			elem.addEventListener('drop', this._handleDrop, false);
			elem.addEventListener('dragend', this._handleDragEnd, false);
			elem.partsPreview = this;
		}
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
		// To avoid having to add a new message to move an arbitrary part, let's select the
		// slide that is being dragged.
		const targetNode = (e.target.id.startsWith('preview') ? e.target : e.target.parentNode);
		var part = this.partsPreview._findClickedPart(targetNode);
		if (part !== null) {
			var partId = parseInt(part) - 1; // The first part is just a drop-site for reordering.
			if (this.partsPreview._map._docLayer && !app.impress.isSlideSelected(partId))
			{
				this.partsPreview._map.setPart(partId);
				this.partsPreview._map.selectPart(partId, 1, false); // And select.
			}
		}
		// By default we move when dragging, but can
		// support duplication with ctrl in the future.
		e.dataTransfer.effectAllowed = 'move';
	},

	_handleDragOver: function (e) {
		if (e.preventDefault) {
			e.preventDefault();
		}

		// By default we move when dragging, but can
		// support duplication with ctrl in the future.
		e.dataTransfer.dropEffect = 'move';

		this.classList.add('preview-img-dropsite');
		return false;
	},

	_handleDragEnter: function () {
	},

	_handleDragLeave: function () {
		this.classList.remove('preview-img-dropsite');
	},

	_handleDrop: function (e) {
		if (e.stopPropagation) {
			e.stopPropagation();
		}

		// When dropping on a thumbnail we get an `img` tag as a target, so we need to get the
		// parent.
		// Otherwise dropping between slides doesn't work.
		// See https://github.com/CollaboraOnline/online/issues/6941
		var target = e.target.classList.contains('preview-img') ? e.target.parentNode : e.target;

		var part = this.partsPreview._findClickedPart(target);
		if (part !== null) {
			var partId = parseInt(part) - 1; // First frame is a drop-site for reordering.
			if (partId < 0)
				partId = -1; // First item is -1.
			app.socket.sendMessage('moveselectedclientparts position=' + partId);
		}

		this.classList.remove('preview-img-dropsite');
		return false;
	},

	_handleDragEnd: function () {
		this.classList.remove('preview-img-dropsite');
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
});

window.L.control.partsPreview = function (container, preview, options) {
	return new window.L.Control.PartsPreview(container, preview, options);
};
