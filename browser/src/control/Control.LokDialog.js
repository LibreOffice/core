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
 * window.L.Control.LokDialog used for displaying COKit dialogs
 */

/* global app $ Hammer brandProductName app cool */

window.L.WinUtil = {

};

var firstTouchPositionX = null;
var firstTouchPositionY = null;
var previousTouchType = null;

function updateTransformation(target) {
	if (target !== null && target !== undefined) {
		var value = [
			'translate3d(' + target.transformation.translate.x + 'px, ' + target.transformation.translate.y + 'px, 0)',
			'scale(' + target.transformation.scale + ', ' + target.transformation.scale + ')'
		];

		value = value.join(' ');
		target.value.style.webkitTransform = value;
		target.value.style.mozTransform = value;
		target.value.style.transform = value;

		if (target.transformation.origin) {
			target.value.style[window.L.DomUtil.TRANSFORM_ORIGIN] = target.transformation.origin;
		}
	}
}

var draggedObject = null;

var zoomTargets = [];

function findZoomTarget(id) {
	for (var item in zoomTargets) {
		if (zoomTargets[item].key === id || zoomTargets[item].titlebar.id === id) {
			return zoomTargets[item];
		}
	}
	return null;
}

function removeZoomTarget(id) {
	for (var item in zoomTargets) {
		if (zoomTargets[item].key === id || zoomTargets[item].titlebar.id === id) {
			delete zoomTargets[item];
		}
	}
}

function toZoomTargetId(id) {
	return id.replace('-canvas', '');
}

window.L.Control.LokDialog = window.L.Control.extend({

	dialogIdPrefix: 'lokdialog-',

	hasDialogInMobilePanelOpened: function() {
		return window.mobileDialogId !== undefined;
	},

	onPan: function (ev) {
		if (!draggedObject)
			return;

		var id = toZoomTargetId(draggedObject.id);
		var target = findZoomTarget(id);

		if (target) {
			var newX = target.initialState.startX + ev.deltaX;
			var newY = target.initialState.startY + ev.deltaY;

			// Don't allow to put dialog outside the view
			if (window.mode.isDesktop() &&
				(newX < -target.width/2 || newY < -target.height/2
				|| newX > window.innerWidth - target.width/2
				|| newY > window.innerHeight - target.height/2)) {
				var dialog = $('.lokdialog_container');
				var left = parseFloat(dialog.css('left'));
				var top = parseFloat(dialog.css('top'));
				newX = Math.max(newX, -left);
				newY = Math.max(newY, -top);
			}

			target.transformation.translate = {
				x: newX,
				y: newY
			};

			target.transformation.translate = {
				x: newX,
				y: newY
			};

			updateTransformation(target);
		}
	},

	onPinch: function (ev) {
		var id = toZoomTargetId(ev.target.id);
		var target = findZoomTarget(id);

		if (target) {
			if (ev.type == 'pinchstart') {
				target.initialState.initScale = target.transformation.scale || 1;
			}

			if (target.initialState.initScale * ev.scale > 0.4) {
				target.transformation.scale = target.initialState.initScale * ev.scale;
			}

			updateTransformation(target);
		}
	},

	onAdd: function (map) {
		map.on('window', this._onDialogMsg, this);
		map.on('windowpaint', this._onDialogPaint, this);
		map.on('docloaded', this._docLoaded, this);
		map.on('closepopup', this.onCloseCurrentPopUp, this);
		map.on('closepopups', this._onClosePopups, this);
		map.on('editorgotfocus', this._onEditorGotFocus, this);
		// Fired to signal that the input focus is being changed.
		map.on('changefocuswidget', this._changeFocusWidget, this);
		window.L.DomEvent.on(document, 'mouseup', this.onCloseCurrentPopUp, this);
	},

	_dialogs: {},

	hasOpenedDialog: function() {
		return Object.keys(this._dialogs).length > 0;
	},

	getCurrentDialogContainer: function() {
		if (this._currentId)
			return document.getElementById(this._dialogs[this._currentId].strId);
		else
			return null;
	},

	// method used to warn user about dialog modality
	blinkOpenDialog: function() {
		$('.lokdialog_container').addClass('lokblink');
		setTimeout(function () {
			$('.lokdialog_container').removeClass('lokblink');
		}, 600);
	},

	_docLoaded: function(e) {
		if (!e.status) {
			$('.lokdialog_container:not(.jsdialog-container)').remove();
			$('.lokdialogchild-canvas').remove();
		}
	},

	_getParentId: function(id) {
		id = parseInt(id);
		for (var winId in this._dialogs) {
			if (this._dialogs[winId].childid && this._dialogs[winId].childid === id) {
				return winId;
			}
		}
		return null;
	},

	_isOpen: function(id) {
		return (id in this._dialogs) && this._dialogs[id] &&
			$('#' + this._toStrId(id)).length > 0;
	},

	isCursorVisible: function(id) {
		return (id in this._dialogs) && this._dialogs[id].cursorVisible;
	},

	_isSelectionHandle: function(el) {
		return window.L.DomUtil.hasClass(el, 'text-selection-handle-start')	||
			window.L.DomUtil.hasClass(el, 'text-selection-handle-end');
	},

	// Given a prefixed dialog id like 'lokdialog-323', gives a raw id, 323.
	_toIntId: function(id) {
		if (typeof(id) === 'string')
			return parseInt(id.replace(this.dialogIdPrefix, ''));
		return id;
	},

	// Converts a raw dialog id like 432, to 'lokdialog-432'.
	_toStrId: function(id) {
		return this.dialogIdPrefix + id;
	},

	// Create a rectangle string of form "x,y,width,height"
	// if params are missing, assumes 0,0,dialog width, dialog height
	_createRectStr: function(id, x, y, width, height) {
		if (!width && id !== null)
			width = this._dialogs[parseInt(id)].width;
		if (!width || width <= 0)
			return null;
		if (!height && id !== null)
			height = this._dialogs[parseInt(id)].height;
		if (!height || height <= 0)
			return null;
		if (!x)
			x = 0;
		if (!y)
			y = 0;

		// pre-multiplied by the scale factor
		return [x * app.roundedDpiScale, y * app.roundedDpiScale, width * app.roundedDpiScale, height * app.roundedDpiScale].join(',');
	},

	_sendPaintWindowRect: function(id, x, y, width, height) {
		this._sendPaintWindow(id, this._createRectStr(id, x, y, width, height));
	},

	// TODO: Extract to tool in Debug.js
	_debugPaintWindow: function(id, rectangle) {
		var strId = this._toStrId(id);
		var canvas = document.getElementById(strId + '-canvas');
		if (!canvas)
			return; // no window to paint to
		var ctx = canvas.getContext('2d');
		ctx.beginPath();
		var rect = rectangle.split(',');
		ctx.rect(rect[0], rect[1], rect[2], rect[3]);
		ctx.fillStyle = 'rgba(255, 0, 0, 0.5)';
		ctx.fill();
	},

	_sendPaintWindow: function(id, rectangle) {
		if (!rectangle)
			return; // Don't request rendering an empty area.

		rectangle = rectangle.replace(/ /g, '');
		if (!rectangle)
			return; // Don't request rendering an empty area.

		//window.app.console.log('_sendPaintWindow: rectangle: ' + rectangle + ', dpiscale: ' + dpiscale);
		app.socket.sendMessage('paintwindow ' + id + ' rectangle=' + rectangle + ' dpiscale=' + app.roundedDpiScale);

		if (this._map._debug.debugOn)
			this._debugPaintWindow(id, rectangle);
	},

	_sendCloseWindow: function(id) {
		app.socket.sendMessage('windowcommand ' + id + ' close');
		// CSV and Macro Security Warning Dialogs are shown before the document load
		// In that state the document is not really loaded and closing or cancelling it
		// returns docnotloaded error. Instead of this we can return to the integration
		if (!this._map._docLoaded && !window._firstDialogHandled) {
			app.dispatcher.dispatch('closeapp');
		}
	},

	_isRectangleValid: function(rect) {
		rect = rect.split(',');
		return (!isNaN(parseInt(rect[0])) && !isNaN(parseInt(rect[1])) &&
				parseInt(rect[2]) >= 0 && parseInt(rect[3]) >= 0);
	},

	_onDialogMsg: function(e) {
		// window.app.console.log('onDialogMsg: id: ' + e.id + ', winType: ' + e.winType + ', action: ' + e.action + ', size: ' + e.size + ', rectangle: ' + e.rectangle);
		if (e.winType != undefined &&
		    e.winType !== 'dialog' &&
		    e.winType !== 'calc-input-win' &&
		    e.winType !== 'child' &&
		    e.winType !== 'deck' &&
		    e.winType !== 'tooltip' &&
		    e.winType !== 'dropdown') {
			return;
		}

		e.id = parseInt(e.id);
		var strId = this._toStrId(e.id);

		var width = 0;
		var height = 0;
		if (e.size) {
			width = parseInt(e.size.split(',')[0]);
			height = parseInt(e.size.split(',')[1]);
		}

		var left;
		var top;
		if (e.position) {
			left = parseInt(e.position.split(',')[0]);
			top = parseInt(e.position.split(',')[1]);
		}

		if (e.title && typeof brandProductName !== 'undefined') {
			e.title = e.title.replace('Collabora Office', brandProductName);
		}

		if (e.action === 'created') {
			if ((e.winType === 'dialog' || e.winType === 'dropdown') && !window.mode.isSmallScreenDevice()) {
				// When left/top are invalid, the dialog shows in the center.
				this._launchDialog(e.id, left, top, width, height, e.title, null, e.unique_id);
			} else if (e.winType === 'child' || e.winType === 'tooltip') {
				var parentId = parseInt(e.parentId);
				if (!this._isOpen(parentId))
					return;

				// In case of tooltips, do not remove the previous popup
				// only if that's also a tooltip.
				if (e.winType === 'tooltip' &&
				    this._dialogs[parentId].childid !== undefined &&
				    this._dialogs[parentId].childistooltip !== true)
					return;

				if (!left)
					left = 0;
				if (!top)
					top = 0;
				this._removeDialogChild(parentId);

				this._dialogs[parentId].childid = e.id;
				this._dialogs[parentId].childwidth = width;
				this._dialogs[parentId].childheight = height;
				this._dialogs[parentId].childx = left;
				this._dialogs[parentId].childy = top;

				if (e.winType === 'tooltip')
					this._dialogs[parentId].childistooltip = true;
				else
					this._dialogs[parentId].childistooltip = false;
				this._createDialogChild(e.id, parentId, top, left);
				this._sendPaintWindow(e.id, this._createRectStr(null, 0, 0, width, height));
			}
		}

		// All other callbacks don't make sense without an active dialog.
		if (!(this._isOpen(e.id) || this._getParentId(e.id))) {
			if (e.action == 'close' && window.mobileDialogId == e.id) {
				window.mobileDialogId = undefined;
				//this._map.fire('closemobilewizard');
			}
			return;
		}

		// We don't want dialogs on smartphones, only calc input window is allowed
		if (window.mode.isSmallScreenDevice())
			return;

		if (e.action === 'invalidate') {
			this.wasInvalidated = true;
			var parent = this._getParentId(e.id);
			var rectangle = e.rectangle;
			if (parent) { // this is a floating window
				if (e.rectangle && this._dialogs[parent].childistooltip === true) {
					// resize tooltips on invalidation
					this._removeDialogChild(parent);
					left = this._dialogs[parent].childx;
					top = this._dialogs[parent].childy;
					width = parseInt(e.rectangle.split(',')[2]);
					height = parseInt(e.rectangle.split(',')[3]);
					this._dialogs[parent].childwidth = width;
					this._dialogs[parent].childheight = height;
					this._createDialogChild(e.id, parent, top, left);
				}

				rectangle = this._createRectStr(null, 0, 0, this._dialogs[parent].childwidth, this._dialogs[parent].childheight);
			} else if (rectangle) { // this is the actual dialog
				if (this._isRectangleValid(rectangle)) {
					rectangle = e.rectangle.split(',');
					x = parseInt(rectangle[0]);
					y = parseInt(rectangle[1]);
					width = parseInt(rectangle[2]);
					height = parseInt(rectangle[3]);

					rectangle = this._createRectStr(null, x, y, width, height);
				} else {
					return;
				}
			} else {
				rectangle = this._createRectStr(e.id);
			}
			this._sendPaintWindow(e.id, rectangle);
		} else if (e.action === 'size_changed') {
			// FIXME: we don't really have to destroy and launch the dialog again but do it for
			// now because the size sent to us previously in 'created' cb is not correct
			$('#' + strId).remove();
			this._launchDialog(e.id, null, null, width, height, this._dialogs[parseInt(e.id)].title, null, e.unique_id);
			if (this._map._docLayer && this._map._docLayer._docType === 'spreadsheet') {
				if (app.sectionContainer.doesSectionExist(app.CSections.RowHeader.name)) {
					app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name)._updateCanvas();
					app.sectionContainer.getSectionWithName(app.CSections.ColumnHeader.name)._updateCanvas();
				}
			}
		} else if (e.action === 'cursor_invalidate') {
			if (this._isOpen(e.id) && !!e.rectangle) {
				rectangle = e.rectangle.split(',');
				var x = parseInt(rectangle[0]);
				var y = parseInt(rectangle[1]);
				height = parseInt(rectangle[3]);

				this._updateDialogCursor(e.id, x, y, height);
			}
		} else if (e.action === 'text_selection') {
			if (this._isOpen(e.id)) {
				var rectangles = [];
				var startHandleVisible, endHandleVisible;
				if (e.rectangles) {
					var dataList = e.rectangles.match(/\d+/g);
					if (dataList != null) {
						for (var i = 0; i < dataList.length; i += 4) {
							var rect = {};
							rect.x = parseInt(dataList[i]);
							rect.y = parseInt(dataList[i + 1]);
							rect.width = parseInt(dataList[i + 2]);
							rect.height = parseInt(dataList[i + 3]);
							rectangles.push(rect);
						}
					}
				}

				if (e.startHandleVisible) {
					startHandleVisible = e.startHandleVisible === 'true';
				}
				if (e.endHandleVisible) {
					endHandleVisible = e.endHandleVisible === 'true';
				}
				this._updateTextSelection(e.id, rectangles, startHandleVisible, endHandleVisible);
			}
		} else if (e.action === 'title_changed') {
			if (e.title && this._dialogs[parseInt(e.id)]) {
				this._dialogs[parseInt(e.id)].title = e.title;
				$('#' + strId).dialog('option', 'title', e.title);
			}
		} else if (e.action === 'cursor_visible') {
			// cursor_visible implies focus has changed, but can
			// be misleading when it flips back on forth on typing!
			var visible = (e.visible === 'true');
			this._dialogs[e.id].cursorVisible = visible;
			if (visible) {
				$('#' + strId + '-cursor').css({display: 'block'});
				this._map.fire('changefocuswidget', {winId: e.id, dialog: this, acceptInput: true}); // Us.
			}
			else {
				$('#' + strId + '-cursor').css({display: 'none'});
			}
		} else if (e.action === 'close') {
			parent = this._getParentId(e.id);
			if (parent)
				this._onDialogChildClose(parent);
			else
				this._onDialogClose(e.id, false);
		} else if (e.action === 'hide') {
			$('#' + strId).parent().css({display: 'none'});
		} else if (e.action === 'show') {
			$('#' + strId).parent().css({display: 'block'});
		}
	},

	_updateDialogCursor: function(dlgId, x, y, height) {
		var strId = this._toStrId(dlgId);
		var dialogCursor = window.L.DomUtil.get(strId + '-cursor');
		var cursorVisible = this.isCursorVisible(dlgId);
		window.L.DomUtil.setStyle(dialogCursor, 'height', height + 'px');
		window.L.DomUtil.setStyle(dialogCursor, 'display', cursorVisible ? 'block' : 'none');
		// set the position of the cursor container element
		window.L.DomUtil.setStyle(this._dialogs[dlgId].cursor, 'left', x + 'px');
		window.L.DomUtil.setStyle(this._dialogs[dlgId].cursor, 'top', y + 'px');

		// Make sure the keyboard is visible if there is a cursor.
		// But don't hide the keyboard otherwise.
		// At least the formula-input hides the cursor after each key input.
		if (cursorVisible)
			this._map.focus(true);
	},

	_createDialogCursor: function(dialogId) {
		var id = this._toIntId(dialogId);
		this._dialogs[id].cursor = window.L.DomUtil.create('div', 'leaflet-cursor-container', window.L.DomUtil.get(dialogId));
		var cursor = window.L.DomUtil.create('div', 'leaflet-cursor lokdialog-cursor', this._dialogs[id].cursor);
		cursor.id = dialogId + '-cursor';
		window.L.DomUtil.addClass(cursor, 'blinking-cursor');
	},

	_updateTextSelection: function(dlgId, rectangles, startHandleVisible, endHandleVisible) {
		var strId = this._toIntId(dlgId);
		var selections = this._dialogs[strId].textSelection.rectangles;
		window.L.DomUtil.empty(selections);
		var handles = this._dialogs[strId].textSelection.handles;
		var startHandle, endHandle;
		if (startHandleVisible) {
			startHandle = this._dialogs[strId].textSelection.startHandle;
		} else if (handles.start) {
			window.L.DomUtil.remove(handles.start);
			handles.start = null;
		}
		if (endHandleVisible) {
			endHandle = this._dialogs[strId].textSelection.endHandle;
		}  else if (handles.end) {
			window.L.DomUtil.remove(handles.end);
			handles.end = null;
		}

		if (!handles.start && !handles.end)
			handles.draggingStopped = true;

		if (!rectangles || rectangles.length < 1) {
			return;
		}

		for (var i = 0; i < rectangles.length; ++i) {
			var container = window.L.DomUtil.create('div', 'leaflet-text-selection-container', selections);
			var selection = window.L.DomUtil.create('div', 'leaflet-text-selection', container);
			var rect = rectangles[i];
			window.L.DomUtil.setStyle(selection, 'width', rect.width + 'px');
			window.L.DomUtil.setStyle(selection, 'height', rect.height + 'px');
			window.L.DomUtil.setStyle(container, 'left',  rect.x + 'px');
			window.L.DomUtil.setStyle(container, 'top', rect.y + 'px');
		}

		var startPos;
		if (startHandle) {
			var startRect = rectangles[0];
			if (startRect.width < 1)
				return;
			startRect = {x: startRect.x, y: startRect.y, width: 1, height: startRect.height};
			startPos = cool.Point.toPoint(startRect.x, startRect.y + startRect.height);
			startPos = startPos.subtract(cool.Point.toPoint(0, 2));
			startHandle.lastPos = startPos;
			startHandle.rowHeight = startRect.height;
		}

		var endPos;
		if (endHandle) {
			var endRect = rectangles[rectangles.length - 1];
			if (endRect.width < 1)
				return;
			endRect = {x: endRect.x + endRect.width - 1, y: endRect.y, width: 1, height: endRect.height};
			endPos = cool.Point.toPoint(endRect.x, endRect.y + endRect.height);
			endPos = endPos.subtract(cool.Point.toPoint(0, 2));
			endHandle.lastPos = endPos;
			endHandle.rowHeight = endRect.height;
		}

		if (startHandle && handles.draggingStopped) {
			if (!handles.start)
				handles.start = handles.appendChild(startHandle);
			// window.app.console.log('lokdialog: _updateTextSelection: startPos: x: ' + startPos.x + ', y: ' + startPos.y);
			startHandle.pos = startPos;
			window.L.DomUtil.setStyle(startHandle, 'left',  startPos.x + 'px');
			window.L.DomUtil.setStyle(startHandle, 'top', startPos.y + 'px');
		}

		if (endHandle && handles.draggingStopped) {
			if (!handles.end)
				handles.end = handles.appendChild(endHandle);
			// window.app.console.log('lokdialog: _updateTextSelection: endPos: x: ' + endPos.x + ', y: ' + endPos.y);
			endHandle.pos = endPos;
			window.L.DomUtil.setStyle(endHandle, 'left',  endPos.x + 'px');
			window.L.DomUtil.setStyle(endHandle, 'top', endPos.y + 'px');
		}
	},

	focus: function(dlgId, acceptInput) {
		this._map.setWinId(dlgId);
		if (dlgId in this._dialogs) {
			this._map.focus(acceptInput);
		}
	},

	_setCanvasWidthHeight: function(canvas, width, height) {
		var newWidth = width * app.roundedDpiScale;
		var changed = false;
		if (canvas.width != newWidth) {
			window.L.DomUtil.setStyle(canvas, 'width', width + 'px');
			canvas.width = newWidth;
			changed = true;
		}

		var newHeight = height * app.roundedDpiScale;
		if (canvas.height != newHeight) {
			window.L.DomUtil.setStyle(canvas, 'height', height + 'px');
			canvas.height = newHeight;
			changed = true;
		}
		return changed;
	},

	_launchDialog: function(id, leftTwips, topTwips, width, height, title, type, uniqueId) {
		if (window.ThisIsTheiOSApp && app.map.mobileTopBar)
				app.map.mobileTopBar.enableItem('closemobile', false);
		this.onCloseCurrentPopUp();
		var dialogContainer = window.L.DomUtil.create('div', 'lokdialog', document.body);
		window.L.DomUtil.setStyle(dialogContainer, 'padding', '0px');
		window.L.DomUtil.setStyle(dialogContainer, 'margin', '0px');
		window.L.DomUtil.setStyle(dialogContainer, 'touch-action', 'manipulate');

		var strId = this._toStrId(id);
		dialogContainer.id = strId;
		if (uniqueId)
			dialogContainer.dataset.uniqueId = uniqueId;

		var dialogCanvas = window.L.DomUtil.create('canvas', 'lokdialog_canvas', dialogContainer);
		this._setCanvasWidthHeight(dialogCanvas, width, height);
		dialogCanvas.id = strId + '-canvas';

		var dialogClass = 'lokdialog_container';
		if (!title)
			dialogClass += ' lokdialog_notitle';

		var that = this;
		var size = $(window).width();
		$(dialogContainer).dialog({
			minWidth: Math.min(width, size.x),
			width: Math.min(width, size.x),
			maxHeight: $(window).height(),
			height: 'auto',
			title: title ? title : '',
			modal: false,
			closeOnEscape: true,
			draggable: false,
			resizable: false,
			dialogClass: dialogClass,
			close: function() {
				that._onDialogClose(id, true);
			}
		});

		if (leftTwips != null && topTwips != null) {
			// magic to re-calculate the position in twips to absolute pixel
			// position inside the #document-container
			var pixels = this._map._docLayer._twipsToPixels(new cool.Point(leftTwips, topTwips));

			// The viewed rectangle's top-left (in CSS pixels) is the scroll and
			// centring offset, kept current for every layout.
			var viewed = app.activeDocument.activeLayout.viewedRectangle;
			var left = pixels.x - viewed.cX1;
			var top = pixels.y - viewed.cY1;

			if (left >= 0 && top >= 0) {
				$(dialogContainer).dialog('option', 'position',
							  { my: 'left top',
							    at: 'left+' + left + ' top+' + top,
							    of: '#document-container' });
			}
		}

		// don't show the dialog surround until we have the dialog content
		$(dialogContainer).parent().hide();

		// Override default minHeight, which can be too large for thin dialogs.
		window.L.DomUtil.setStyle(dialogContainer, 'minHeight', height + 'px');

		// Title bar may overflow due to range name. So we should have max width
		var titleBar = dialogContainer.previousSibling;
		var leftPadding = window.getComputedStyle(titleBar).getPropertyValue('padding-left').slice(0, -2);
		var rightPadding = window.getComputedStyle(titleBar).getPropertyValue('padding-right').slice(0, -2);
		window.L.DomUtil.setStyle(titleBar, 'maxWidth', width - (+leftPadding + +rightPadding) + 'px');

		this._dialogs[id] = {
			id: id,
			strId: strId,
			width: width,
			height: height,
			cursor: null,
			title: title
		};

		// don't make 'TAB' focus on this button; we want to cycle focus in the lok dialog with each TAB
		$('.lokdialog_container button.ui-dialog-titlebar-close').attr('tabindex', '-1').blur();

		this._createDialogCursor(strId);
		this._setupWindowEvents(id, dialogCanvas/*, dlgInput*/);
		this._setupGestures(dialogContainer, id, dialogCanvas);

		this._currentId = id;
		this._sendPaintWindow(id, this._createRectStr(id));
	},

	_postLaunch: function(id, panelContainer, panelCanvas) {
		if (window.mode.isDesktop()) {
			this._setupWindowEvents(id, panelCanvas/*, dlgInput*/);
		}

		// Render window.
		this._sendPaintWindowRect(id);
	},

	_setupWindowEvents: function(id, canvas/*, dlgInput*/) {
		window.L.DomEvent.on(canvas, 'contextmenu', window.L.DomEvent.preventDefault);
		window.L.DomEvent.on(canvas, 'mousemove', function(e) {
			var pos = this._isSelectionHandle(e.target) ? window.L.DomEvent.getMousePosition(e, canvas) : {x: e.offsetX, y: e.offsetY};
			this._postWindowMouseEvent('move', id, pos.x, pos.y, 1, 0, 0);
		}, this);

		window.L.DomEvent.on(canvas, 'mousedown mouseup', function(e) {
			window.L.DomEvent.preventDefault(e);

			if (this._map.uiManager.isUIBlocked())
				return;

			if (canvas.lastDraggedHandle)
				canvas.lastDraggedHandle = null;

			var buttons = 0;
			if (this._map['mouse']) {
				buttons |= e.buttons === app.JSButtons.left ? app.LOButtons.left : 0;
				buttons |= e.buttons === app.JSButtons.middle ? app.LOButtons.middle : 0;
				buttons |= e.buttons === app.JSButtons.right ? app.LOButtons.right : 0;
			} else {
				buttons = 1;
			}

			var modifier = 0;
			var shift = e.shiftKey ? app.UNOModifier.SHIFT : 0;
			var ctrl = e.ctrlKey ? app.UNOModifier.CTRL : 0;
			var alt = e.altKey ? app.UNOModifier.ALT : 0;
			var cmd = e.metaKey ? app.UNOModifier.CTRLMAC : 0;
			modifier = shift | ctrl | alt | cmd;

			// 'mousedown' -> 'buttondown'
			var lokEventType = e.type.replace('mouse', 'button');
			var pos = this._isSelectionHandle(e.target) ? window.L.DomEvent.getMousePosition(e, canvas) : {x: e.offsetX, y: e.offsetY};
			this._postWindowMouseEvent(lokEventType, id, pos.x, pos.y, 1, buttons, modifier);
			this._map.setWinId(id);
			//dlgInput.focus();
		}, this);

		window.L.DomEvent.on(canvas, 'click', function(ev) {
			// Clicking on the dialog's canvas shall not trigger any
			// focus change - therefore the event is stopped and preventDefault()ed.
			window.L.DomEvent.stop(ev);
		});
	},

	_setupGestures: function(dialogContainer, id, canvas) {
		var targetId = toZoomTargetId(canvas.id);
		var zoomTarget = $('#' + targetId).parent().get(0);
		var titlebar = $('#' + targetId).prev().children().get(0);

		var ratio = 1.0;
		var width = this._dialogs[id].width;
		var height = this._dialogs[id].height;
		var offsetX = 0;
		var offsetY = 0;

		if ((window.mode.isSmallScreenDevice() || window.mode.isTablet()) && width > window.screen.width) {
			ratio = window.screen.width / (width + 40);
			offsetX = -(width - window.screen.width) / 2;
			offsetY = -(height - window.screen.height) / 2;
		}

		var state = {
			startX: offsetX,
			startY: offsetY,
			initScale: ratio
		};
		var transformation = {
			translate: { x: offsetX, y: offsetY },
			scale: ratio,
			angle: 0,
			rx: 0,
			ry: 0,
			rz: 0
		};

		// on mobile, force the positioning to the top, so that it is not
		// covered by the virtual keyboard
		if (window.mode.isSmallScreenDevice()) {
			$(dialogContainer).dialog('option', 'position', { my: 'center top', at: 'center top', of: '#document-container' });
			transformation.origin = 'center top';
			transformation.translate.y = 0;
		}

		if (findZoomTarget(targetId) != null) {
			removeZoomTarget(targetId);
		}
		zoomTargets.push({key: targetId, value: zoomTarget, titlebar: titlebar, transformation: transformation, initialState: state, width:width, height: height});

		var that = this;
		var hammerTitlebar = new Hammer(titlebar);
		hammerTitlebar.add(new Hammer.Pan({ threshold: 20, pointers: 0 }));
		hammerTitlebar.add(new Hammer.Pinch({ threshold: 0 })).recognizeWith([hammerTitlebar.get('pan')]);

		hammerTitlebar.on('panstart', this.onPan);
		hammerTitlebar.on('panmove', this.onPan);
		hammerTitlebar.on('pinchstart pinchmove', this.onPinch);
		hammerTitlebar.on('hammer.input', function(ev) {
			if (ev.isFirst) {
				draggedObject = ev.target;
			}

			if (ev.isFinal && draggedObject) {
				var id = toZoomTargetId(draggedObject.id);
				var target = findZoomTarget(id);
				if (target) {
					target.initialState.startX = target.transformation.translate.x;
					target.initialState.startY = target.transformation.translate.y;
				}
				draggedObject = null;
			}
		});

		var hammerContent = new Hammer(canvas);
		hammerContent.add(new Hammer.Pan({ threshold: 20, pointers: 0 }));
		hammerContent.add(new Hammer.Pinch({ threshold: 0 })).recognizeWith([hammerContent.get('pan')]);

		hammerContent.on('panstart', this.onPan);
		hammerContent.on('panmove', this.onPan);
		hammerContent.on('pinchstart pinchmove', this.onPinch);
		hammerContent.on('hammer.input', function(ev) {
			if (ev.isFirst) {
				that.wasInvalidated = false;
				draggedObject = ev.target;
			}
			else if (that.wasInvalidated) {
				draggedObject = null;
				that.wasInvalidated = false;
				return;
			}

			if (ev.isFinal && draggedObject) {
				var id = toZoomTargetId(draggedObject.id);
				var target = findZoomTarget(id);
				if (target) {
					target.initialState.startX = target.transformation.translate.x;
					target.initialState.startY = target.transformation.translate.y;
				}
				draggedObject = null;
			}
		});

		updateTransformation(findZoomTarget(targetId));
	},

	_postWindowMouseEvent: function(type, winid, x, y, count, buttons, modifier) {
		app.socket.sendMessage('windowmouse id=' + winid +  ' type=' + type +
		                              ' x=' + x + ' y=' + y + ' count=' + count +
		                              ' buttons=' + buttons + ' modifier=' + modifier);
	},

	_postWindowGestureEvent: function(winid, type, x, y, offset) {
		// window.app.console.log('x ' + x + ' y ' + y + ' o ' + offset);
		app.socket.sendMessage('windowgesture id=' + winid +  ' type=' + type +
		                              ' x=' + x + ' y=' + y + ' offset=' + offset);
	},

	_closeChildWindows: function(dialogId) {
		// child windows - with greater id number
		var that = this;
		var foundCurrent = false;

		Object.keys(this._dialogs).forEach(function(id) {
			if (foundCurrent)
				that._onDialogClose(id, true);

			if (id == dialogId)
				foundCurrent = true;
		});
	},

	_onDialogClose: function(dialogId, notifyBackend) {
		this._closeChildWindows(dialogId);

		if (window.ThisIsTheiOSApp && app.map.mobileTopBar)
			app.map.mobileTopBar.enableItem('closemobile', true);

		if (notifyBackend)
			this._sendCloseWindow(dialogId);
		$('#' + this._toStrId(dialogId)).remove();

		// focus the main document
		this._map.fire('editorgotfocus');
		this._map.focus();

		delete this._dialogs[dialogId];
		this._currentId = null;

		removeZoomTarget(this._toStrId(dialogId));
	},

	_onClosePopups: function() {
		for (var dialogId in this._dialogs) {
			this._onDialogClose(dialogId, true);
		}
		if (this.hasDialogInMobilePanelOpened()) {
			this._onDialogClose(window.mobileDialogId, true);
		}
	},

	onCloseCurrentPopUp: function() {
		// for title-less dialog only (context menu, pop-up)
		if (this._currentId && this._isOpen(this._currentId) &&
			!this._dialogs[this._currentId].title)
			this._onDialogClose(this._currentId, true);
	},

	_onEditorGotFocus: function() {
		// We need to lose focus on any dialogs currently with focus.
		for (var winId in this._dialogs) {
			$('#' + this._dialogs[winId].strId + '-cursor').css({display: 'none'});
		}
	},

	// Focus is being changed, update states.
	_changeFocusWidget: function (e) {
		if (e.winId === 0) {
			// We lost the focus.
			this._onEditorGotFocus();
		} else {
			this.focus(e.winId, e.acceptInput);
			if (this._map.formulabar)
				this._map.onFormulaBarFocus();
		}
	},

	_paintDialog: function(parentId, rectangle, img) {
		var strId = this._toStrId(parentId);
		var canvas = document.getElementById(strId + '-canvas');
		if (!canvas)
			return; // no window to paint to

		this._dialogs[parentId].isPainting = true;
		var ctx = canvas.getContext('2d');

		var that = this;
		var x = 0;
		var y = 0;
		if (rectangle) {
			rectangle = rectangle.split(',');
			x = parseInt(rectangle[0]);
			y = parseInt(rectangle[1]);
		}

		var container = window.L.DomUtil.get(strId);

		ctx.drawImage(img, x, y);

		// if dialog is hidden, show it
		if (container)
			$(container).parent().show();

		if (parentId in that._dialogs) {
			// We might have closed the dialog by the time we render.
			that._dialogs[parentId].isPainting = false;
			that._map.fire('changefocuswidget', {winId: parentId, dialog: that});
		}
	},

	// Binary dialog msg recvd from core
	_onDialogPaint: function (e) {
		var id = parseInt(e.id);
		var parentId = this._getParentId(id);
		if (parentId) {
			this._paintDialogChild(parentId, e.img);
		} else {
			this._paintDialog(id, e.rectangle, e.img);
		}
	},

	// Dialog Child Methods

	_paintDialogChild: function(parentId, img) {
		var strId = this._toStrId(parentId);
		var canvas = window.L.DomUtil.get(strId + '-floating');
		if (!canvas)
			return; // no floating window to paint to

		// Make sure the child is not trimmed on the right.
		var width = this._dialogs[parentId].childwidth;
		var left = parseInt(canvas.style.left);
		var leftPos = left + width;
		if (leftPos > window.innerWidth) {
			var newLeft = window.innerWidth - width - 20;
			window.L.DomUtil.setStyle(canvas, 'left', newLeft + 'px');
		}
		// Also, make sure child is not trimmed on bottom.
		var top = parseInt(canvas.style.top);
		var height = this._dialogs[parentId].childheight;
		var bottomPos = top + height;
		if (bottomPos > window.innerHeight) {
			var newTop = top - height - 20;
			window.L.DomUtil.setStyle(canvas, 'top', newTop + 'px');
		}

		// The image is rendered per the HiDPI scale we used
		// while requesting rendering the image. Here we
		// set the canvas to have the actual size, while
		// the image is rendered with the HiDPI scale.
		this._setCanvasWidthHeight(canvas, this._dialogs[parentId].childwidth,
			this._dialogs[parentId].childheight);

		var ctx = canvas.getContext('2d');
		ctx.drawImage(img, 0, 0);
		$(canvas).show();
	},

	_onDialogChildClose: function(dialogId) {
		$('#' + this._toStrId(dialogId) + '-floating').remove();

		// Remove any extra height allocated for the parent container (only for floating dialogs).
		var canvas = document.getElementById(dialogId + '-canvas');
		if (!canvas) {
			canvas = document.getElementById(this._toStrId(dialogId) + '-canvas');
			if (!canvas)
				return;
		}
		var canvasHeight = canvas.height;
		$('#' + dialogId).height(canvasHeight + 'px');

		this._dialogs[dialogId].childid = undefined;
		this._dialogs[dialogId].childx = undefined;
		this._dialogs[dialogId].childy = undefined;
	},

	_removeDialogChild: function(id) {
		$('#' + this._toStrId(id) + '-floating').remove();
		this._dialogs[id].childid = undefined;
		this._dialogs[id].childx = undefined;
		this._dialogs[id].childy = undefined;
	},

	_createDialogChild: function(childId, parentId, top, left) {
		var strId = this._toStrId(parentId);
		var dialogContainer = window.L.DomUtil.get(strId);
		var floatingCanvas = window.L.DomUtil.create('canvas', 'lokdialogchild-canvas', dialogContainer);
		$(floatingCanvas).hide(); // Hide to avoid flickering while we set the dimensions.

		floatingCanvas.id = strId + '-floating';
		window.L.DomUtil.setStyle(floatingCanvas, 'position', 'fixed');
		window.L.DomUtil.setStyle(floatingCanvas, 'z-index', '11');
		window.L.DomUtil.setStyle(floatingCanvas, 'width', '0px');
		window.L.DomUtil.setStyle(floatingCanvas, 'height', '0px');

		/*
			Some notes:
				* Modal windows' child positions are relative to page borders.
				* So this code adapts to it.
		*/

		// Add header height..
		var addition = 40;
		window.L.DomUtil.setStyle(floatingCanvas, 'margin-inline-start', left + 'px');
		window.L.DomUtil.setStyle(floatingCanvas, 'top', (top + addition) + 'px');

		// attach events
		this._setupChildEvents(childId, floatingCanvas);
	},

	_setupChildEvents: function(childId, canvas) {
		window.L.DomEvent.on(canvas, 'contextmenu', window.L.DomEvent.preventDefault);

		window.L.DomEvent.on(canvas, 'touchstart touchmove touchend', function(e) {
			window.L.DomEvent.preventDefault(e);
			var rect = canvas.getBoundingClientRect();
			var touchX = (e.type === 'touchend') ? e.changedTouches[0].clientX : e.targetTouches[0].clientX;
			var touchY = (e.type === 'touchend') ? e.changedTouches[0].clientY : e.targetTouches[0].clientY;
			touchX = touchX - rect.x;
			touchY = touchY - rect.y;
			if (e.type === 'touchstart')
			{
				firstTouchPositionX = touchX;
				firstTouchPositionY = touchY;
				this._postWindowGestureEvent(childId, 'panBegin', firstTouchPositionX, firstTouchPositionY, 0);
			}
			else if (e.type === 'touchend')
			{
				this._postWindowGestureEvent(childId, 'panEnd', firstTouchPositionX, firstTouchPositionY, firstTouchPositionY - touchY);
				if (previousTouchType === 'touchstart') {
					// Simulate mouse click
					if (this._map['mouse']) {
						this._postWindowMouseEvent('buttondown', childId, firstTouchPositionX, firstTouchPositionY, 1, app.LOButtons.left, 0);
						this._postWindowMouseEvent('buttonup', childId, firstTouchPositionX, firstTouchPositionY, 1, app.LOButtons.left, 0);
					} else {
						this._postWindowMouseEvent('buttondown', childId, firstTouchPositionX, firstTouchPositionY, 1, 1, 0);
						this._postWindowMouseEvent('buttonup', childId, firstTouchPositionX, firstTouchPositionY, 1, 1, 0);
					}
				}
				firstTouchPositionX = null;
				firstTouchPositionY = null;
			}
			else if (e.type === 'touchmove')
			{
				this._postWindowGestureEvent(childId, 'panUpdate', firstTouchPositionX, firstTouchPositionY, firstTouchPositionY - touchY);
			}
			previousTouchType = e.type;
		}, this);

		window.L.DomEvent.on(canvas, 'mousedown mouseup', function(e) {
			var buttons = 0;
			if (this._map['mouse']) {
				buttons |= e.buttons === app.JSButtons.left ? app.LOButtons.left : 0;
				buttons |= e.buttons === app.JSButtons.middle ? app.LOButtons.middle : 0;
				buttons |= e.buttons === app.JSButtons.right ? app.LOButtons.right : 0;
			} else {
				buttons = 1;
			}
			var lokEventType = e.type.replace('mouse', 'button');
			this._postWindowMouseEvent(lokEventType, childId, e.offsetX, e.offsetY, 1, buttons, 0);
		}, this);
		window.L.DomEvent.on(canvas, 'mousemove', function(e) {
			this._postWindowMouseEvent('move', childId, e.offsetX, e.offsetY, 1, 0, 0);
		}, this);
		window.L.DomEvent.on(canvas, 'contextmenu', function() {
			return false;
		});
	}

});

window.L.control.lokDialog = function (options) {
	return new window.L.Control.LokDialog(options);
};
