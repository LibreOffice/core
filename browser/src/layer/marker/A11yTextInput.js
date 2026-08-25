/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * window.L.A11yTextInput is the hidden textarea, which handles text input events
 *
 * This is made significantly more difficult than expected by such a
 * mess of browser, and mobile IME quirks that it is not possible to
 * follow events, but we have to re-construct input from a browser
 * text area itself.
 */

/* global app _ _n */

window.L.A11yTextInput = window.L.TextInput.extend({
	initialize: function() {
		window.L.TextInput.prototype.initialize.call(this);

		this._className = 'A11yTextInput';

		// Used for signaling when in a mobile device the user tapped the edit button
		this._justSwitchedToEditMode = false;

		// Used when editing a shape text content.
		this._isEditingInSelection = false;
		this._hasAnySelection = false;


		// In core text selection exists even if it's empty and <backspace> deletes the empty selection
		// instead of the previous character.
		this._hasSelection = false;
		this._lastCursorPosition = 0;
		this._lastSelectionStart = 0;
		this._lastSelectionEnd = 0;
		this._listPrefixLength = 0;
		this._isLeftRightArrow = 0;

		// pending macOS live region update
		this._a11yLiveRegionUpdate = 0;

		// what this element stands for while it is not a text box
		this._a11yContext = '';

		this._updateA11yEditableStateBound = this._updateA11yEditableState.bind(this);
	},

	onAdd: function() {
		window.L.TextInput.prototype.onAdd.call(this);
		// the canvas only exists once the doc layer has been built
		this._map.on('doclayerinit', this._bindCanvasFocusGuard, this);
		this._map.on('doclayerinit', this._updateA11yEditableState, this);
		this._map.on('updateparts', this._onA11yPartChanged, this);
		app.events.on('updatepermission', this._updateA11yEditableStateBound);
	},

	onRemove: function() {
		this._map.off('doclayerinit', this._bindCanvasFocusGuard, this);
		this._map.off('doclayerinit', this._updateA11yEditableState, this);
		this._map.off('updateparts', this._onA11yPartChanged, this);
		app.events.off('updatepermission', this._updateA11yEditableStateBound);
		var canvas = document.getElementById('document-canvas');
		if (canvas)
			window.L.DomEvent.off(canvas, 'mousedown', this._keepFocusOnCanvasClick, this);
		window.L.TextInput.prototype.onRemove.call(this);
	},

	// What a screen reader says about this element while it is the idle
	_setA11yContext: function(text) {
		this._a11yContext = text;
		if (this._textArea.isContentEditable)
			return;
		this._textArea.setAttribute('role', 'group');
		this._textArea.setAttribute('aria-roledescription', text);
	},

	_slideContext: function() {
		return _('Slide');
	},

	// A part change drops the selection with it, so the context goes back to
	// naming the slide.
	_onA11yPartChanged: function() {
		this._setA11yContext(this._slideContext());
	},

	// macOS only: this hands the announcing over to the assertive live region,
	// which is itself macOS only, since a screen reader reads aria-description
	// as part of announcing a form field and stops doing so once this is a plain div.
	_updateA11yEditableState: function() {
		if (!window.L.Browser.mac)
			return;
		// a read-only document has its own contenteditable handling, see
		// TextInput._onPermission
		if (!this._map.isEditMode())
			return;

		var idleSurface = this._map.getDocType() === 'presentation' && !this._isEditingInSelection;
		// already in the wanted state?
		if (this._textArea.isContentEditable === !idleSurface)
			return;

		var hadFocus = document.activeElement === this._textArea;
		if (idleSurface) {
			// a div that is not editable needs a tabindex to stay focusable, so
			// give it one before taking the contenteditable away
			this._textArea.setAttribute('tabindex', '0');
			this._textArea.setAttribute('contenteditable', 'false');
			this._setA11yContext(this._a11yContext || this._slideContext());
		} else {
			this._textArea.setAttribute('contenteditable', 'true');
			this._textArea.removeAttribute('tabindex');
			// a text box announces itself as one again
			this._textArea.removeAttribute('role');
			this._textArea.removeAttribute('aria-roledescription');
		}
		// dropping contenteditable can drop the focus with it
		if (hadFocus && document.activeElement !== this._textArea)
			this._textArea.focus({ preventScroll: true });
	},

	_bindCanvasFocusGuard: function() {
		var canvas = document.getElementById('document-canvas');
		if (canvas)
			window.L.DomEvent.on(canvas, 'mousedown', this._keepFocusOnCanvasClick, this);
	},

	_keepFocusOnCanvasClick: function(ev) {
		if (ev.button === 0 && this._map.getDocType() === 'presentation'
			&& document.activeElement === this._textArea)
			ev.preventDefault();
	},

	hasAccessibilitySupport: function() {
		return true;
	},

	setHTML: function(content) {
		this._textArea.innerHTML = this._wrapContent(content);
	},

	_prependSpace: function() {
		this._textArea.innerHTML = this._preSpaceChar + this._textArea.innerHTML;
	},

	_appendSpace: function() {
		this._textArea.innerHTML = this._textArea.innerHTML + this._postSpaceChar;
	},

	_getLastCursorPosition: function() {
		return this._lastCursorPosition;
	},

	_setLastCursorPosition: function(nPos) {
		this._lastCursorPosition = nPos;
		this._lastSelectionStart = this._lastSelectionEnd = nPos;
	},

	_setLastSelection: function(nStart, nEnd) {
		this._lastSelectionStart = nStart;
		this._lastSelectionEnd = this._lastCursorPosition = nEnd;
	},

	_isLastSelectionEmpty: function() {
		return this._lastSelectionStart === this._lastSelectionEnd;
	},

	_isLastSelection: function(start, end) {
		return this._hasSelection &&  this._lastSelectionStart === start && this._lastSelectionEnd === end;
	},

	_updateCursorPosition: function(pos) {
		if (typeof pos !== 'number')
			return;
		// Normalize input parameters
		var l = this.getPlainTextContent().length;
		if (pos < 0)
			pos = 0;
		if (pos > l)
			pos = l;

		this._setLastCursorPosition(pos);
		this._setCursorPosition(pos);
	} ,

	_updateSelection: function(pos, start, end, forced) {
		window.app.console.log('_updateSelection: pos: ' + pos + ', start: ' + start + ', end: ' + end);
		if (typeof pos !== 'number' || typeof start !== 'number' || typeof end !== 'number')
			return;

		var hasSelection= !(start === -1 && end === -1);
		if (!hasSelection) {
			this._updateCursorPosition(pos);
		} else if (forced || !this._isLastSelection(start, end)) {
			if (forced || start !== end || !this._hasSelection || !this._isLastSelectionEmpty()) {
				// When the new selection is empty (start == end). the cursor position is updated
				// only if there was no previous selection, or previous selection was not empty.
				// In fact when both old and new selection are empty, it means that the old selection
				// has been moved to a new position by some typing. Anyway changing cursor position
				// while typing can mess up editable area content.
				this._setLastSelection(start, end);
				this._setSelectionRange(start, end);
			}
		}
		this._setSelectionFlag(hasSelection);
	},

	_setSelectionFlag: function(flag) {
		this._hasSelection = flag;
		if (window.L.Browser.cypressTest)
			this._textArea.isSelectionNull = !flag;
	},

	_setFocusedParagraph: function(content, pos, start, end) {
		window.app.console.log('_setFocusedParagraph:'
			+ '\n    content "' + content + '"'
			+ '\n    pos: ' + pos
			+ '\n    start: ' + start + ', end: ' + end);

		this._isComposing = false;
		this._isLeftRightArrow = 0;
		if (!this._hasFormulaBarFocus()) {
			this.setHTML(content);
			this.updateLastContent();
			this._updateSelection(pos, start, end, true);
		}
	},

	_updateFocusedParagraph: function() {
		this._log('_updateFocusedParagraph');
		if (this._remoteContent !== undefined) {
			this._setFocusedParagraph(this._remoteContent, this._remotePosition,
				this._remoteSelectionStart, this._remoteSelectionEnd);
		} else if (this._remoteSelectionEnd !== undefined) {
			this._updateSelection(this._remotePosition, this._remoteSelectionStart, this._remoteSelectionEnd);
		} else if (this._remotePosition !== undefined) {
			this._updateCursorPosition(this._remotePosition);
		}
		this._remoteContent = undefined;
		this._remotePosition = undefined;
		this._remoteSelectionStart = undefined;
		this._remoteSelectionEnd = undefined;
	},

	onAccessibilityFocusChanged: function(content, pos, start, end, listPrefixLength, force) {
		this._listPrefixLength = listPrefixLength;
		if (!this.hasFocus() || (this._isComposing && !force)) {
			this._log('onAccessibilityFocusChanged: skipped updating: '
				+ '\n  hasFocus: ' + this.hasFocus()
				+ '\n  _isComposing: ' + this._isComposing
				+ '\n  force: ' + force);
			this._remoteContent = content;
			this._remotePosition = pos;
			this._remoteSelectionStart = start;
			this._remoteSelectionEnd = end;
		} else {
			this._setFocusedParagraph(content, pos, start, end);
		}
	},

	setA11yFocusedParagraph: function(content, pos, start, end) {
		this._setFocusedParagraph(content, pos, start, end);
	},

	onAccessibilityCaretChanged: function(nPos) {
		this._log('onAccessibilityCaretChanged: \n' +
			'    position: ' + nPos + '\n' +
			'    _isComposing: ' + this._isComposing);
		if (this._isLeftRightArrow || !this.hasFocus() || this._isComposing) {
			this._log('onAccessibilityCaretChanged: skip updating');
			this._remotePosition = nPos;
		}
		else if (!this._hasFormulaBarFocus()) {
			this._updateCursorPosition(nPos);
		}
	},

	setA11yCaretPosition: function(nPos) {
		if (this._isLastSelectionEmpty()) {
			this.onAccessibilityCaretChanged(nPos);
		}
	},

	onAccessibilityTextSelectionChanged: function(start, end) {
		if (this._isLeftRightArrow || !this.hasFocus() || this._isComposing) {
			this._remoteSelectionStart = start;
			this._remoteSelectionEnd = end;
			var hasSelection = !(start === -1 && end === -1);
			this._setSelectionFlag(hasSelection);
			if (hasSelection) {
				this._remotePosition = end;
			}
			this._statusLog('onAccessibilityTextSelectionChanged: skip updating');
		} else {
			this._updateSelection(this._lastCursorPosition, start, end);
		}
	},

	A11Y_FOCUS_ANNOUNCEMENT_MS: 500,

	_setDescription: function(text) {
		this._log('setDescription: ' + text);
		this._textArea.setAttribute('aria-description', text);

		if (window.L.Browser.mac) {
			// avoid duplicate cell announcement in Chrome
			var sinceFocus = Date.now() - (this._a11yFocusTime || 0);
			if (this._textArea.isContentEditable && text
				&& sinceFocus < this.A11Y_FOCUS_ANNOUNCEMENT_MS
				&& text === this._a11yFocusDescription)
				return;

			// required on macOS as VoiceOver is not triggered by description change only
			var region = this._a11yLiveRegion;
			if (region) {
				if (this._a11yLiveRegionUpdate)
					cancelAnimationFrame(this._a11yLiveRegionUpdate);
				region.textContent = '';
				var t = text;
				var that = this;
				this._a11yLiveRegionUpdate = requestAnimationFrame(function () {
					that._a11yLiveRegionUpdate = 0;
					region.textContent = t;
				});
			}
		}
	},

	_updateTable: function(outCount, inList, row, col, rowSpan, colSpan) {
		this._log('_updateTable: '
			+ '\n outCount: ' + outCount
			+ '\n inList: ' + inList.toString()
			+ '\n row: ' + row + ', rowSpan: ' + rowSpan
			+ '\n col: ' + col + ', colSpan: ' + colSpan
		);

		if (this._timeoutForA11yDescription)
			clearTimeout(this._timeoutForA11yDescription);

		var eventDescription = '';
		if (outCount > 0 || inList.length > 0) {
			this._lastRowIndex = 0;
			this._lastColIndex = 0;
			this._lastRowSpan = 1;
			this._lastColSpan = 1;
		}
		for (var i = 0; i < outCount; i++) {
			eventDescription += _('Out of table') + '. ';
		}
		for (i = 0; i < inList.length; i++) {
			// One msgid can carry only one plural count, so the row and the column
			// count are separate plural entries that the sentence below composes.
			const rows = _n('%n row', '%n rows', inList[i].rowCount);
			const columns = _n('%n column', '%n columns', inList[i].colCount);
			// TRANSLATORS: {0} is a row count, {1} a column count, both already
			// TRANSLATORS: in their plural form, e.g. "3 rows" and "1 column"
			eventDescription += _('Table with {0} and {1}').replace('{0}', rows).replace('{1}', columns) + '. ';
		}
		if (this._lastRowIndex !== row || this._lastRowSpan !== rowSpan) {
			this._lastRowIndex = row;
			if (this._lastRowSpan !== rowSpan && rowSpan > 1) {
				eventDescription += _('Row {0} through {1}').replace('{0}', row).replace('{1}', row + rowSpan - 1);
			}
			else {
				eventDescription += _('Row {0}').replace('{0}', row);
			}
			eventDescription += '. ';
			this._lastRowSpan = rowSpan;
		}
		if (this._lastColIndex !== col || this._lastColSpan !== colSpan) {
			this._lastColIndex = col;
			if (this._lastColSpan !== colSpan && colSpan > 1) {
				eventDescription += _('Column {0} through {1}').replace('{0}', col).replace('{1}', col + colSpan - 1);
			}
			else {
				eventDescription += _('Column {0}').replace('{0}', col);
			}
			eventDescription += '. ';
			this._lastColSpan = colSpan;
		}
		this._setDescription(eventDescription);

		var that = this;
		this._timeoutForA11yDescription = setTimeout(function() {
			that._setDescription('');
		}, 1000);
	},

	onAccessibilityFocusedCellChanged: function(outCount, inList, row, col, rowSpan, colSpan, paragraph) {
		this._setFocusedParagraph(paragraph.content, parseInt(paragraph.position), parseInt(paragraph.start), parseInt(paragraph.end));
		this._updateTable(outCount, inList, row + 1, col + 1, rowSpan, colSpan);
	},

	onAccessibilityEditingInSelectionState: function(cell, enabled, selectionDescr, paragraph) {
		this._log('onAccessibilityEditingInSelectionState: cell: ' + cell + ', enabled: ' + enabled);
		if (!cell) {
			this._isEditingInSelection = enabled;
			// entering shape text editing makes this a real text box again
			this._updateA11yEditableState();
		}
		if (enabled) {
			clearTimeout(this._timeoutForA11yDescription);
			var eventDescription = '';
			if (typeof selectionDescr === 'string' && selectionDescr.length > 0)
				eventDescription += selectionDescr + '. ';
			eventDescription += _('Editing activated. ');
			if (typeof paragraph === 'string' && paragraph.length > 0)
				eventDescription += paragraph;
			this._setDescription(eventDescription);
			this._timeoutForA11yDescription = setTimeout(function () {
				this._setDescription('');
			}.bind(this), 1000);
		}
	},

	onAccessibilitySelectionChanged: function(cell, action, name, textContent) {
		this._log('onAccessibilitySelectionChanged: cell: ' + cell + ', action: ' + action + ', name: ' + name);
		if (this._timeoutForA11yDescription)
			clearTimeout(this._timeoutForA11yDescription);
		if (!this._isFormula())
			this._emptyArea();
		var eventDescription = '';
		if (action === 'create' || action === 'add') {
			this._hasAnySelection = true;
			// the selected object is now what this element stands for, so that is
			// what a screen reader reads when the focus next enters the document
			this._setA11yContext(name);
			eventDescription =  _('{0} selected').replace('{0}', name) + '. ';
			if (typeof textContent === 'string' && textContent.length > 0) {
				eventDescription += (cell ? '' : _('Has text: ')) + textContent;
			}
		}
		else if (action === 'remove') {
			this._hasAnySelection = false;
			this._setA11yContext(this._slideContext());
			eventDescription = _('{0} unselected').replace('{0}', name);
		}
		else if (action === 'delete') {
			this._hasAnySelection = false;
			this._setA11yContext(this._slideContext());
			eventDescription = _('{0} deleted').replace('{0}', name);
		}
		this._setDescription(eventDescription);
		if (action !== 'create' && action !== 'add') {
			this._timeoutForA11yDescription = setTimeout(function () {
				this._setDescription('');
			}.bind(this), 1000);
		}
	},

	// Check if a UTF-16 pair represents a Unicode code point
	_isSurrogatePair: function(hi, lo) {
		return 	hi >= 0xd800 && hi <= 0xdbff && lo >= 0xdc00 && lo <= 0xdfff;
	},

	// Backspaces and deletes at the beginning / end are filtered out, so
	// we get a beforeinput, but no input for them. Sometimes we can end up
	// in a state where we lost our leading / terminal chars and can't recover
	_onBeforeInput: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;
		this._statusLog('_onBeforeInput [');
		this._ignoreNextBackspace = false;
		if (!this._isSelectionValid()) {
			this._setCursorPosition(this._getLastCursorPosition());
		}
		else if (this._isCursorAtBeginning()) {
			this._handleMisplacedCursorAtBeginning(ev);
		}
		else if (!this._isLastSelectionEmpty() && !this._hasFormulaBarFocus() && this._isFormula()) {
			// A cell address is selected in formula input mode,
			// before inserting a new input we need to clear selection
			this._updateCursorPosition(this._lastSelectionEnd);
		}

		if (!this._isComposing && !this._isLeftRightArrow && this._remotePosition !== undefined) {
			this._updateFocusedParagraph();
		}

		// Firefox is not able to delete the <img> post space. Since no 'input' event is generated,
		// we need to handle a <delete> at the end of the paragraph, here.
		if (window.L.Browser.gecko && (!this._hasSelection || this._isLastSelectionEmpty()) &&
			this._getLastCursorPosition() === this.getPlainTextContent().length &&
			this._deleteHint === 'delete') {
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeEmptySelectionIfAny();
			this._removeTextContent(0, 1);
		}
		this._statusLog('_onBeforeInput ]');
	},

	updateLastContent: function() {
		var value = this.getValue();
		this._lastContent = this.getValueAsCodePoints(value);
	},

	_isFormula: function() {
		var content = this.getValue();
		return this._map._docLoaded && this._map.getDocType() === 'spreadsheet'
			&& content.length > 0 && content[0] === '=';
	},

	_requestFocusedParagraph: function() {
		app.socket.sendMessage('geta11yfocusedparagraph');
	},

	_restoreSpanWrapper: function() {
		var children = this._textArea.childNodes;
		if (children.length >= 3 && children[1].nodeName === '#text') {
			if (children.length === 3) {
				// When typing in an empty paragraph, we get <img>H<img>
				var htmlContent = this.getHTML();
				htmlContent = htmlContent.slice(this._preSpaceChar.length, -this._postSpaceChar.length);
				this.setHTML(htmlContent);
			}
			else if (children.length === 4 && children[2].id === 'readable-content') {
				// When typing, let's say 'k', at beginning of a not empty paragraph,
				// we get: <img>k<span>Hello World</span><img>
				var newText = children[1].textContent;
				children[2].innerHTML = newText + children[2].innerHTML;
				this._textArea.removeChild(children[1]);
			}
		}
	},

	// Fired when text has been inputted, *during* and after composing/spellchecking
	_onInput: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;
		this._statusLog('_onInput [');
		app.idleHandler.notifyActive();

		if (this._ignoreInputCount > 0) {
			window.app.console.log('ignoring synthetic input ' + this._ignoreInputCount);
			return;
		}

		if (this._deleteHint === '' && ev.inputType) {
			if (ev.inputType === 'deleteContentForward')
				this._deleteHint = 'delete';
			else if (ev.inputType === 'deleteContentBackward')
				this._deleteHint = 'backspace';
		}

		var ignoreBackspace = this._ignoreNextBackspace;
		this._ignoreNextBackspace = false;

		if (this._newlineHint) {
			this._sendNewlineEvent();
			return;
		}

		// We use a different leading and terminal space character
		// to differentiate backspace from delete, then replace the character.
		if (!this._hasPreSpace()) { // missing initial space
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending backspace');
			}
			if (!ignoreBackspace) {
				this._removeEmptySelectionIfAny();
				this._removeTextContent(1, 0);
			}
			// Lately we receive the new paragraph == above paragraph + current paragraph,
			// except current paragraph is the first one.
			// In this last case we need to restore the pre space.
			this._prependSpace();
			this._updateCursorPosition(0);
			return;
		}
		if (!this._hasPostSpace()) { // missing trailing space.
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeTextContent(0, this._hasSelection && this._isLastSelectionEmpty() ? 2 : 1);
			this._appendSpace();
			var pos = this._getLastCursorPosition();
			this._updateCursorPosition(pos);
			return;
		}

		// We assume that what is on the right of the new cursor position has not been modified
		// We also assume that lastCursorPosition is synchronized with the cursor position in core
		var cursorPosition = this._getSelectionEnd();
		var lastCursorPosition = this._getLastCursorPosition();
		var value = this.getValue();

		// In the android keyboard when you try to erase the pre-space
		// and then enter some character,
		// The first character will likely travel with the cursor,
		// And that is caused because after entering the first character
		// cursor position is never updated by keyboard (I know it is strange)
		// so here we manually correct the position
		if (lastCursorPosition === 0 && cursorPosition < 1 && value.length - this._lastContent.length === 1) {
			cursorPosition = 1;
			if (!this._isComposing)
				this._setCursorPosition(1);
		}

		// We need to take into account the case that lastCursorPosition is beyond the new cursor position.
		// For instance that can happen when after entering a word, several spaces are typed:
		// a '.' is appended automatically.
		var contentTailLength = value.length - cursorPosition;
		var lastContentTailLength = this._lastContent.length - lastCursorPosition;
		var guessedBackMatchTo = Math.min(lastContentTailLength, contentTailLength);
		var contentEnd = value.length - guessedBackMatchTo;
		var content = this.getValueAsCodePoints(value.slice(0, contentEnd));
		// Note that content is an array of Unicode code points
		var lastContentEnd = this._lastContent.length - guessedBackMatchTo;
		var lastContent = this._lastContent.slice(0, lastContentEnd);

		window.app.console.log('_onInput: cursorPosition: ' + cursorPosition + ', lastContentEnd: ' + lastContentEnd);

		var matchTo = 0;
		var compareUpTo = Math.min(content.length, lastContent.length);
		if (!this._isLastSelectionEmpty()) {
			// Selected text has always to be removed, so there is no need for comparing old and new content
			// over selection start. Moreover, if selection content starts with the typed key, it would lead to
			// an empty new content and the input would never be forwarded to core.
			compareUpTo = Math.min(compareUpTo, this._lastSelectionStart);
		}
		while (matchTo < compareUpTo && content[matchTo] === lastContent[matchTo])
			matchTo++;

		if (this._map._debug.logKeyboardEvents) {
			window.app.console.log('Comparison matchAt ' + matchTo + '\n' +
				'\tnew "' + this.codePointsToString(content) + '" (' + content.length + ')' + '\n' +
				'\told "' + this.codePointsToString(lastContent) + '" (' + lastContent.length + ')');
		}

		// no new content
		if (matchTo === content.length && matchTo === lastContent.length)
			return;

		// matchTo <= lastCursorPosition <= lastContent.length
		matchTo = Math.min(matchTo, lastCursorPosition);

		var removeAfter = 0;
		var removeBefore = 0;
		if (!this._hasSelection) {
			removeAfter = lastContent.length - lastCursorPosition;
			removeBefore = (lastContent.length - matchTo) - removeAfter;
		}
		else if (this._deleteHint === 'backspace') {
			this._removeEmptySelectionIfAny();
			this._setSelectionFlag(false);
			removeBefore = 1;
		}
		else if (this._deleteHint === 'delete') {
			// when in core there is an empty selection the first <delete> deletes
			// the selection instead of the next char
			this._setSelectionFlag(false);
			removeAfter = this._isLastSelectionEmpty() ? 2 : 1;
		}

		// A browser selection range counts a surrogate UTF-16 pair as 2 chars.
		// The same occurs in core for the text cursor position reported by the caret changed accessibility event.
		// However, in core a single <backspace> or <delete> is needed for deleting a surrogate pair.
        if (removeBefore > 1) {
			var start = lastCursorPosition - removeBefore;
			for (var i = start; i < lastCursorPosition; i++) {
				if (this._isSurrogatePair(lastContent[i], lastContent[i+1])) {
					removeBefore--;
					i++;
				}
			}
		}
		if (removeAfter > 1) {
			var end = lastCursorPosition + removeAfter;
			for (var j = lastCursorPosition; j < end; j++) {
				if (this._isSurrogatePair(lastContent[j], lastContent[j+1])) {
					removeAfter--;
					j++;
				}
			}
		}

		if (removeBefore > 0 || removeAfter > 0)
			this._removeTextContent(removeBefore, removeAfter);

		var newText = content;
		if (matchTo > 0)
			newText = newText.slice(matchTo);

		var head = this._lastContent.slice(0, matchTo);
		var tail = this._lastContent.slice(lastContentEnd);
		this._lastContent = head.concat(newText, tail);
		window.app.console.log('_onInput: \n'
			+ 'head: "' + this.codePointsToString(head) + '"\n'
			+ 'newText: "' + this.codePointsToString(newText) + '"\n'
			+ 'tail: "' + this.codePointsToString(tail) + '"');

		this._setLastCursorPosition(cursorPosition);

		if (newText.length > 0) {
			if (!this._isComposing && !this._isWrappedBySpan()) {
				this._restoreSpanWrapper();
				this._setCursorPosition(cursorPosition);
			}
			this._sendNewText(ev, content, newText);
		}

		// special handling for formula bar
		this._finishFormulabarEditing(content, matchTo);

		// special handling for mentions
		if (this._map.getDocType() === 'text')
			this._map.mention.handleMentionInput(ev);

		this._statusLog('_onInput ]');
	},

	_removeEmptySelectionIfAny: function() {
		if (this._hasSelection && this._isLastSelectionEmpty()) {
			// when in core there is an empty selection a <backspace> or a <delete> removes
			// the selection instead of the previous or next char, so we send a fake <delete>
			// in order to remove the empty selection.
			this._sendDelete();
		}
	},

	_sendDelete: function() {
		this._sendKeyEvent(46, 1286, 'input');
	},

	_hasPreSpace: function() {
		var child = this._textArea.firstChild;
		while (child && child.tagName !== 'img') {
			if (child.id === 'pre-space')
				return true;
			child = child.firstChild;
		}
		return false;
	},

	_hasPostSpace: function() {
		var child = this._textArea.lastChild;
		while (child) {
			if (child.id === 'post-space')
				return true;
			child = child.lastChild;
		}
		return false;
	},

	_isWrappedBySpan: function() {
		var children = this._textArea.childNodes;
		return children.length === 3 && children[1].nodeName === 'SPAN';
	}
});

L.a11yTextInput = function() {
	return new window.L.A11yTextInput();
};
