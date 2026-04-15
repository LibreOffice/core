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
 * JSDialog.FormulabarEdit - text field in the fromulabar
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'formulabaredit',
 *     test: 'text content\nsecond line',
 *     cursor: true,
 *     enabled: false
 * }
 *
 * 'cursor' specifies if user can type into the field or it is readonly
 * 'enabled' editable field can be temporarily disabled
 */

/* global JSDialog FormulaBarSelectionHandle cool */

var scrollToCursorTimeout = null;
var startHandle;
var endHandle;

function _sendSelection(edit, builder, id) {
	if (document.activeElement != edit)
		return;

	var selection = document.getSelection();
	if (!selection)
		return;

	var startPos = 0;
	var anchorOffset = selection.anchorOffset;
	var endPos = 0;
	var focusOffset = selection.focusOffset;
	var startPara = 0;
	var endPara = 0;

	var startElement = selection.anchorNode;
	var endElement = selection.focusNode;

	if (selection.anchorNode == edit) {
		startPos = endPos = 0;
		for (var i in edit.childNodes) {
			if (i == selection.anchorOffset)
				break;
			if (edit.childNodes[i].tagName == 'BR') {
				startPara++;
				endPara++;
			}
		}
	} else {
		for (var i in edit.childNodes) {
			if (edit.childNodes[i] != startElement && edit.childNodes[i].firstChild != startElement) {
				if (edit.childNodes[i].tagName == 'BR') {
					startPara++;
					startPos = 0;
				} else {
					startPos++;
				}
			} else {
				break;
			}
		}

		for (var i in edit.childNodes) {
			if (edit.childNodes[i] != endElement && edit.childNodes[i].firstChild != endElement) {
				if (edit.childNodes[i].tagName == 'BR') {
					endPara++;
					endPos = 0;
				} else {
					endPos++;
				}
			} else {
				break;
			}
		}
	}

	var selection = (startPos + anchorOffset) + ';' + (endPos + focusOffset) + ';' + startPara + ';' + endPara;
	builder.callback('edit', 'textselection', {id: id}, selection, builder);
}

function _appendText(cursorLayer, text, style) {
	var span = window.L.DomUtil.create('span', style);
	span.innerText = text;
	cursorLayer.appendChild(span);
	return span;
}

function _appendNewLine(cursorLayer) {
	cursorLayer.appendChild(window.L.DomUtil.create('br', ''));
}

function _appendCursor(cursorLayer) {
	var cursor = window.L.DomUtil.create('span', 'cursor');
	cursorLayer.appendChild(cursor);
	return cursor;
}

function _onSelectionHandleDragEnd(
	start,
	builder,
	container,
	wrapper,
	cursorLayer,
	handleLayer,
	textLayer,
	text,
	startX,
	endX,
	startY,
	endY
) {
	const resetSelection = () => {
		_setSelection(
			builder,
			container,
			wrapper,
			cursorLayer,
			handleLayer,
			textLayer,
			text,
			startX,
			endX,
			startY,
			endY
		);
		// Reset the selection to jump the handle back to a valid place
		// No need to send this to core - since as we're not actually moving the selection
	};

	return (point) => {
		const cursorLayerPosition = cursorLayer.getBoundingClientRect();
		const newY = cursorLayerPosition.top + point.y - 1 /* or we just hit the selection handle... */;
		let newX;

		if (start) {
			newX = cursorLayerPosition.left + point.x + startHandle.width;
		} else {
			newX = cursorLayerPosition.left + point.x;
		}

		let node;
		if (document.caretPositionFromPoint) {
			const position = document.caretPositionFromPoint(newX, newY);

			if (!position) {
				resetSelection();
				return;
			}

			node = position.offsetNode;
		} else {
			const range = document.caretRangeFromPoint(newX, newY);

			if (!range) {
				resetSelection();
				return;
			}

			node = range.startContainer;
		}

		if (!textLayer.contains(node) || textLayer === node) {
			resetSelection();
			return;
		}

		if (node instanceof Text) {
			node = node.parentElement;
		}

		let newCharX = 0;
		let newCharY = 0;
		for (const child of textLayer.children) {
			if (child === node && start) {
				// The start selection should include the character you're highlighting by being *before* it
				break;
			}

			if (child.tagName === "BR") {
				newCharX = 0;
				newCharY++;
			} else {
				newCharX++;
			}

			if (child === node) {
				// The end selection should include the character you're highlighting by being *after* it
				break;
			}
		}

		let selection;
		if (start) {
			_setSelection(
				builder,
				container,
				wrapper,
				cursorLayer,
				handleLayer,
				textLayer,
				text,
				newCharX,
				endX,
				newCharY,
				endY
			);
			selection = `${newCharX};${endX};${newCharY};${endY}`;
		} else {
			_setSelection(
				builder,
				container,
				wrapper,
				cursorLayer,
				handleLayer,
				textLayer,
				text,
				startX,
				newCharX,
				startY,
				newCharY
			);
			selection = `${startX};${newCharX};${startY};${newCharY}`;
		}

		builder.callback('edit', 'textselection', {id: container.id}, selection, builder);
	};
}

function _setSelection(builder, container, wrapper, cursorLayer, handleLayer, textLayer, text, startX, endX, startY, endY) {
	var newCursorLayer = document.createDocumentFragment();

	var reversedSelection = false;
	if (endY == startY && endX < startX) {
		reversedSelection = true;

		var tmp = startX;
		startX = endX;
		endX = tmp;
	}

	if (endY < startY) {
		reversedSelection = true;

		var tmp = startY;
		startY = endY;
		endY = tmp;

		tmp = startX;
		startX = endX;
		endX = tmp;
	}

	let selectionTexts = [];

	for (var i in text) {
		var line = text[i];

		if (i < startY) {
			_appendText(newCursorLayer, line, '');
			_appendNewLine(newCursorLayer);
		} else if (i == startY) {
			_appendText(newCursorLayer, line.substr(0, startX), '');

			if (reversedSelection)
				var cursor = _appendCursor(newCursorLayer);

			selectionTexts.push(_appendText(newCursorLayer,
				line.substr(startX, startY == endY ? endX - startX : undefined),
				((startX != endX || startY != endY) ? 'selection' : '')));

			if (startY == endY) {
				if (!reversedSelection)
					cursor = _appendCursor(newCursorLayer);

				_appendText(newCursorLayer, line.substr(endX), '');
				_appendNewLine(newCursorLayer);
			} else
				_appendNewLine(newCursorLayer);
		} else if (i > startY && i < endY) {
			selectionTexts.push(_appendText(newCursorLayer, line, 'selection'));
			_appendNewLine(newCursorLayer);
		} else if (i == endY && endY != startY) {
			selectionTexts.push(_appendText(newCursorLayer, line.substr(0, endX), 'selection'));
			if (!reversedSelection)
				cursor = _appendCursor(newCursorLayer);
			_appendText(newCursorLayer, line.substr(endX), '');
			_appendNewLine(newCursorLayer);
		} else if (i > endY) {
			_appendText(newCursorLayer, line, '');
			_appendNewLine(newCursorLayer);
		}
	}

	cursorLayer.textContent = '';
	cursorLayer.appendChild(newCursorLayer);

	// possible after cursor is added to the DOM
	if (cursor) {
		if (scrollToCursorTimeout)
			clearTimeout(scrollToCursorTimeout);

		// put scrol at the end of the task queue so we will not scroll multiple times
		// during one session of processing events where multiple setSelection actions
		// can be found, profiling shows it is heavy operation
		scrollToCursorTimeout = setTimeout(function () {
			var blockOption = JSDialog.ScrollIntoViewBlockOption('nearest');
			cursor.scrollIntoView({behavior: 'smooth', block: blockOption, inline: 'nearest'});
			scrollToCursorTimeout = null;
		}, 0);
	}

	if (!handleLayer) {
		return;
	}

	if (startX === endX && startY === endY) {
		if (startHandle) startHandle.requestVisible(false);
		if (endHandle) endHandle.requestVisible(false);
		return;
	}

	if (!startHandle) {
		startHandle = new FormulaBarSelectionHandle(wrapper, handleLayer, 'start');
	}

	const selectionPositions = selectionTexts.flatMap(selectionText => Array.from(selectionText.getClientRects()));
	if (!selectionPositions.length)
		return;

	const selectionStartPosition = selectionPositions[0];
	const selectionEndPosition = selectionPositions[selectionPositions.length - 1];
	const cursorLayerPosition = cursorLayer.getBoundingClientRect();

	startHandle.setPosition(new cool.SimplePoint(selectionStartPosition.left - cursorLayerPosition.left, selectionStartPosition.bottom - cursorLayerPosition.top));
	startHandle.requestVisible(true);
	startHandle.onDragEnd = _onSelectionHandleDragEnd(
		true,
		builder,
		container,
		wrapper,
		cursorLayer,
		handleLayer,
		textLayer,
		text,
		startX,
		endX,
		startY,
		endY
	);

	if (!endHandle) {
		endHandle = new FormulaBarSelectionHandle(wrapper, handleLayer, 'end');
	}
	endHandle.setPosition(new cool.SimplePoint(selectionEndPosition.right - cursorLayerPosition.left, selectionEndPosition.bottom - cursorLayerPosition.top));
	endHandle.requestVisible(true);
	endHandle.onDragEnd = _onSelectionHandleDragEnd(
		false,
		builder,
		container,
		wrapper,
		cursorLayer,
		handleLayer,
		textLayer,
		text,
		startX,
		endX,
		startY,
		endY
	);
}

function _formulabarEditControl(parentContainer, data, builder) {
	var container = window.L.DomUtil.create('div', 'ui-custom-textarea ' + builder.options.cssClass, parentContainer);
	var wrapper = window.L.DomUtil.create('div', 'ui-custom-textarea-overflow-wrapper ' + builder.options.cssClass, container);
	container.id = data.id;

	var textLayer = window.L.DomUtil.create('div', 'ui-custom-textarea-text-layer ' + builder.options.cssClass, wrapper);

	if (data.enabled !== false)
		textLayer.setAttribute('contenteditable', 'true');

	var cursorLayer = window.L.DomUtil.create('div', 'ui-custom-textarea-cursor-layer ' + builder.options.cssClass, wrapper);
	var handleLayer = window.L.DomUtil.create('div', 'ui-custom-textarea-handle-layer ' + builder.options.cssClass, wrapper);

	wrapper.addEventListener('scroll', () => {
		requestAnimationFrame(() => {
			handleLayer.style.top = `${-wrapper.scrollTop}px`;
		});
	});

	container.setText = function(text, selection) {
		var newTextLayer = document.createDocumentFragment();
		for (var c = 0; c < text.length; c++) {
			if (text[c] == '\n')
				_appendNewLine(newTextLayer);
			else
				_appendText(newTextLayer, text[c], '');
		}

		textLayer.textContent = '';
		textLayer.appendChild(newTextLayer);

		var startX = parseInt(selection[0]);
		var endX = parseInt(selection[1]);
		var startY = parseInt(selection[2]);
		var endY = parseInt(selection[3]);

		text = text.split('\n');

		_setSelection(builder, container, wrapper, cursorLayer, handleLayer, textLayer, text, startX, endX, startY, endY);
	};

	container.enable = function() {
		window.L.DomUtil.removeClass(container, 'disabled');
		textLayer.setAttribute('contenteditable', 'true');
	};
	container.disable = function() {
		window.L.DomUtil.addClass(container, 'disabled');
		textLayer.setAttribute('contenteditable', 'false');
	};

	var textSelectionHandler = function(event) {
		if (window.L.DomUtil.hasClass(container, 'disabled')) {
			event.preventDefault();
			return;
		}

		builder.callback('edit', 'grab_focus', container, null, builder);
		_sendSelection(textLayer, builder, container.id);

		builder.map.setWinId(0);
		builder.map._textInput._emptyArea();
		builder.map._textInput.focus(true);

		event.preventDefault();
	};

	['click', 'dblclick', 'contextmenu'].forEach(function (ev) {
		textLayer.addEventListener(ev, textSelectionHandler);
	});

	// hide old selection when user starts to select something else
	textLayer.addEventListener('mousedown', function() {
		textLayer.addEventListener('mouseup', textSelectionHandler, {once: true});
		builder.callback('edit', 'grab_focus', container, null, builder);

		cursorLayer.querySelectorAll('.selection').forEach(function (element) {
			window.L.DomUtil.addClass(element, 'hidden');
		});

		var cursor = cursorLayer.querySelector('.cursor');
		if (cursor)
			window.L.DomUtil.addClass(cursor, 'hidden');
	});

	var text = builder._cleanText(data.text);
	container.setText(text, [0, 0, 0, 0]);

	if (data.enabled === false)
		window.L.DomUtil.addClass(container, 'disabled');

	if (data.hidden)
		window.L.DomUtil.addClass(container, 'hidden');

	return false;
}

JSDialog.formulabarEdit = function (parentContainer, data, builder) {
	var buildInnerData = _formulabarEditControl(parentContainer, data, builder);
	return buildInnerData;
};
