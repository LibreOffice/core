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
 * JSDialog.DrawingArea - drawing area displaying picture sent from the server
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'drawingarea',
 *     image: 'base64 encoded image',
 *     text: 'alternative text',
 *     loading: true, - show additional spinner div
 *     placeholderText: false,  - 'show text next to image'
 * }
 */

/* global JSDialog UNOKey app */

function _drawingAreaControl (parentContainer, data, builder) {
	var container = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-drawing-area-container', parentContainer);
	container.id = data.id;

	container.getCurrent = function () {
		if (container.id == "") {
			return document.getElementById(data.id);
		} else {
			return container;
		}
	};
	if (!data.image)
		return;

	var isTextbox = data.aria && data.aria.role === 'textbox';
	var imageId = data.id + '-img';

	if (isTextbox) {
		// Editable drawing areas (e.g. spell check sentence box) use a
		// div with role="textbox" wrapping the visual image.
		var wrapper = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-drawing-area', container);
		wrapper.id = imageId;
		wrapper.tabIndex = 0;
		wrapper.setAttribute('role', 'textbox');
		wrapper.setAttribute('aria-multiline', 'true');

		if (data.editText) {
			wrapper.setAttribute('aria-label', data.editText);
		}

		if (data.aria.description) {
			var descSpan = window.L.DomUtil.create('span', 'visuallyhidden', wrapper);
			descSpan.id = imageId + '-desc';
			descSpan.textContent = data.aria.description;
			wrapper.setAttribute('aria-describedby', descSpan.id);
		}

		var img = window.L.DomUtil.create('img', '', wrapper);
		img.src = data.image.replace(/\\/g, '');
		img.style.display = 'block';
		img.draggable = false;
		img.ondragstart = function() { return false; };
		img.alt = '';
		img.setAttribute('aria-hidden', 'true');
		img.addEventListener('mousedown', function() { wrapper.focus(); });

		if (data.text) {
			wrapper.setAttribute('data-cooltip', data.text);
			if (builder.map) {
				window.L.control.attachTooltipEventListener(wrapper, builder.map);
			}
		}

		_setupDrawingAreaMouseEvents(img, container, builder);
		_setupDrawingAreaKeyboardEvents(wrapper, container, builder);
	} else {
		var image = window.L.DomUtil.create('img', builder.options.cssClass + ' ui-drawing-area', container);
		image.id = imageId;
		image.src = data.image.replace(/\\/g, '');
		image.draggable = false;
		image.ondragstart = function() { return false; };

		const isFocusableImg = data.enabled && data.canFocus;
		if (isFocusableImg) {
			image.tabIndex = 0;
			JSDialog.AddAltAttrOnFocusableImg(image, data, builder);
			if (data.aria && data.aria.role) {
				image.setAttribute('role', data.aria.role);
			}
		} else {
			image.alt = '';
			image.classList.add('ui-decorative-image');
		}

		if (data.text) {
			image.setAttribute('data-cooltip', data.text);

			if (builder.map) {
				window.L.control.attachTooltipEventListener(image, builder.map);
			}
		}

		// Line width dialog is affected from delay on image render.
		// So If the image render is delayed, use width and height of the data
		if (JSDialog.isWidgetInModalPopup(data) && image.width == 0 && image.height == 0) {
			image.width = data.imagewidth;
			image.height = data.imageheight;
		}

		if (data.loading && data.loading === 'true') {
			var loaderContainer = window.L.DomUtil.create('div', 'ui-drawing-area-loader-container', container);
			window.L.DomUtil.create('div', 'ui-drawing-area-loader', loaderContainer);
		}
		if (data.placeholderText && data.placeholderText === 'true') {
			var spanContainer = window.L.DomUtil.create('div', 'ui-drawing-area-placeholder-container', container);
			var span = window.L.DomUtil.create('span', 'ui-drawing-area-placeholder', spanContainer);
			span.innerText = data.text;
		}

		_setupDrawingAreaMouseEvents(image, container, builder);
		_setupDrawingAreaKeyboardEvents(image, container, builder);
	}

	return false;
}

function _setupDrawingAreaMouseEvents (imageElement, container, builder) {
	var getCoordinatesFromEvent = function (e) {
		var boundingBox = imageElement.getBoundingClientRect();
		var ret = [e.x - boundingBox.left, e.y - boundingBox.top];

		ret[0] = ret[0] / imageElement.offsetWidth;
		ret[1] = ret[1] / imageElement.offsetHeight;

		return ret;
	};

	var moveTimer = null;
	var moveFunc = null;

	window.L.DomEvent.on(imageElement, 'dblclick', function(e) {
		var pos = getCoordinatesFromEvent(e);
		var coordinates = pos[0] + ';' + pos[1];

		clearTimeout(moveTimer);
		moveTimer = null;
		moveFunc = null;
		builder.callback('drawingarea', 'dblclick', container.getCurrent(), coordinates, builder);
	}, this);

	window.L.DomEvent.on(imageElement, 'click touchend', function(e) {
		var pos = getCoordinatesFromEvent(e);
		var coordinates = pos[0] + ';' + pos[1];

		clearTimeout(moveTimer);
		moveTimer = null;
		moveFunc = null;

		builder.callback('drawingarea', 'click', container.getCurrent(), coordinates, builder);
	}, this);

	var onMove = function (e) {
		if (moveTimer && moveFunc) {
			clearTimeout(moveTimer);
			moveTimer = null;
			moveFunc();
			moveFunc = null;
		}

		var pos = getCoordinatesFromEvent(e);
		var coordinates = pos[0] + ';' + pos[1];
		builder.callback('drawingarea', 'mousemove', container.getCurrent(), coordinates, builder);
	};

	var endMove = function (e) {
		clearTimeout(moveTimer);
		moveTimer = null;
		moveFunc = null;

		window.removeEventListener('mousemove', onMove);
		window.removeEventListener('mouseup', endMove);

		var pos = getCoordinatesFromEvent(e);
		var coordinates = pos[0] + ';' + pos[1];
		builder.callback('drawingarea', 'mouseup', container.getCurrent(), coordinates, builder);
	};

	imageElement.addEventListener('mousedown', function (e) {
		moveFunc = function () {
			var pos = getCoordinatesFromEvent(e);
			var coordinates = pos[0] + ';' + pos[1];
			builder.callback('drawingarea', 'mousedown', container.getCurrent(), coordinates, builder);
		};

		moveTimer = setTimeout(function () {
			moveFunc();
			moveFunc = null;
			moveTimer = null;
		}, 200);

		window.addEventListener('mousemove', onMove);
		window.addEventListener('mouseup', endMove);
	});
}

function _setupDrawingAreaKeyboardEvents (focusElement, container, builder) {
	var modifier = 0;

	focusElement.addEventListener('keydown', function(event) {
		if (event.key === 'Enter') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.RETURN | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Escape' || event.key === 'Esc') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.ESCAPE | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Left' || event.key === 'ArrowLeft') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.LEFT | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Right' || event.key === 'ArrowRight') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.RIGHT | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Up' || event.key === 'ArrowUp') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.UP | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Down' || event.key === 'ArrowDown') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.DOWN | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Home') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.HOME | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'End') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.END | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Backspace') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.BACKSPACE | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Delete') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.DELETE | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Space') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.SPACE | modifier, builder);
			event.preventDefault();
		} else if (event.key === 'Tab') {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.TAB | modifier, builder);
		} else if (event.key === 'Shift') {
			modifier = modifier | app.UNOModifier.SHIFT;
			event.preventDefault();
		} else if (event.key === 'Control') {
			modifier = modifier | app.UNOModifier.CTRL;
			event.preventDefault();
		} else if (event.key === 'a' && event.ctrlKey) {
			builder.callback('drawingarea', 'keypress', container.getCurrent(), UNOKey.A | app.UNOModifier.CTRL, builder);
		}
	});

	focusElement.addEventListener('keyup', function(event) {
		if (event.key === 'Shift') {
			modifier = modifier & (~app.UNOModifier.SHIFT);
			event.preventDefault();
		} else if (event.key === 'Control') {
			modifier = modifier & (~app.UNOModifier.CTRL);
			event.preventDefault();
		}
	});

	focusElement.addEventListener('blur', function() {
		modifier = 0;
	});

	focusElement.addEventListener('keypress', function(event) {
		if (event.key === 'Enter' ||
			event.key === 'Escape' ||
			event.key === 'Esc' ||
			event.key === 'Left' ||
			event.key === 'ArrowLeft' ||
			event.key === 'Right' ||
			event.key === 'ArrowRight' ||
			event.key === 'Up' ||
			event.key === 'ArrowUp' ||
			event.key === 'Down' ||
			event.key === 'ArrowDown' ||
			event.key === 'Home' ||
			event.key === 'End' ||
			event.key === 'Backspace' ||
			event.key === 'Delete' ||
			event.key === 'Space' ||
			event.key === 'Tab') {
			// skip
		} else {
			var keyCode = event.keyCode;
			if (event.ctrlKey) {
				keyCode = event.key.toUpperCase().charCodeAt(0);
				keyCode = builder.map.keyboard._toUNOKeyCode(keyCode);
				keyCode |= app.UNOModifier.CTRL;
			}

			builder.callback('drawingarea', 'keypress', container.getCurrent(), keyCode, builder);
		}

		event.preventDefault();
	});
}

JSDialog.drawingArea = function (parentContainer, data, builder) {
	var buildInnerData = _drawingAreaControl(parentContainer, data, builder);
	return buildInnerData;
};
