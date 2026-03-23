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
 * window.L.TextInput is the hidden textarea, which handles text input events
 *
 * This is made significantly more difficult than expected by such a
 * mess of browser, and mobile IME quirks that it is not possible to
 * follow events, but we have to re-construct input from a browser
 * text area itself.
 */

/* global app _ CursorHandler cool */

window.L.TextInput = window.L.Layer.extend({
	initialize: function() {

		window.L.Layer.prototype.initialize.call(this);

		this._className = 'TextInput';

		// Flag to denote the composing state, derived from
		// compositionstart/compositionend events; unused
		this._isComposing = false;

		// We need to detect whether delete or backspace was
		// pressed sometimes - consider '  foo' -> ' foo'
		this._deleteHint = ''; // or 'delete' or 'backspace'

		// When <enter> key is hit a <div> element is created and appended to the editable area:
		// inputType: insertParagraph. No new char is added to the editable area textContent property.
		this._newlineHint = false;

		// We need to detect line break in the tunneled formula
		// input window for the multiline case.
		this._linebreakHint = false;

		// Clearing the area can generate input events
		this._ignoreInputCount = 0;

		// If the last focus intended to accept user input.
		// Signifies whether the keyboard is meant to be visible.
		this._setAcceptInput(false);

		// Content
		this._lastContent = []; // unicode characters
		this._hasWorkingSelectionStart = undefined; // does it work ?
		this._ignoreNextBackspace = false;

		this._preSpaceChar = '<img id="pre-space" alt=" " aria-hidden="true">';
		this._postSpaceChar = '<img id="post-space" alt=" " aria-hidden="true">';
		this._initialContent = this._preSpaceChar + this._postSpaceChar;

		// Debug flag, used in fancyLog(). See the debug() method.
		// this._isDebugOn = true;
		this._isDebugOn = false;

		this._initLayout();

		// Under-caret orange marker.
		this._addCursorHandler();

		// Auto-correct characters can trigger auto-correction, but
		// must be sent as key-up/down if we want correction.
		// cf. SvxAutoCorrect::IsAutoCorrectChar
		this._autoCorrectChars = {
			// tab, newline - handled elsewhere
			' ':  [ 32, 0,      0, 1284 ],
			'!':  [ 33, 0,      0, 4353 ],
			'"':  [ 34, 0,      0, 4353 ],
			'%':  [ 37, 0,      0, 4357 ],
			'\'': [ 39, 0,      0,  192 ],
			'*':  [ 42, 0,      0, 4360 ],
			',':  [ 44, 0,      0, 1291 ],
			'-':  [ 45, 0,      0, 1288 ],
			'.':  [ 46, 0,      0,  190 ],
			'/':  [ 47, 0,      0,  191 ],
			':':  [ 58, 0,      0, 5413 ],
			';':  [ 59, 0,      0, 1317 ],
			'?':  [ 63, 0,      0, 4287 ],
			'_':  [ 95, 0,      0, 5384 ]
		};

		// unoKeyCode values of the digits.
		this._unoKeyMap = {
			'48': 96,   // 0
			'49': 97,   // 1
			'50': 98,   // 2
			'51': 99,   // 3
			'52': 100,  // 4
			'53': 101,  // 5
			'54': 102,  // 6
			'55': 103,  // 7
			'56': 104,  // 8
			'57': 105   // 9
		};

	},

	_addCursorHandler() {
		if (document.getElementById('canvas-container')) {
			this._cursorHandler = new CursorHandler();
			app.sectionContainer.addSection(this._cursorHandler);
		}
		else {
			app.layoutingService.appendLayoutingTask(() => {
				this._addCursorHandler();
			});
		}
	},

	hasAccessibilitySupport: function() {
		return false;
	},

	onAdd: function() {
		if (this._container) {
			var docContainer = document.getElementById('document-container');
			if (docContainer)
				docContainer.appendChild(this._container);
			this.update();
		}

		if (!this.hasAccessibilitySupport()) {
			this._emptyArea();
			this._map.on('commandresult', this._onCommandResult, this);
		}

		app.events.on('updatepermission', this._onPermission.bind(this));
		window.L.DomEvent.on(this._textArea, 'focus blur', this._onFocusBlur, this);

		// Do not wait for a 'focus' event to attach events if the
		// textarea/contenteditable is already focused (due to the autofocus
		// HTML attribute, the browser focusing it on DOM creation, or whatever)
		if (document.activeElement === this._textArea) {
			this._onFocusBlur({ type: 'focus' });
		}

		if (window.ThisIsTheiOSApp) {
			var that = this;
			window.MagicToGetHWKeyboardWorking = function() {
				var that2 = that;
				window.MagicKeyDownHandler = function(e) {
					that2._onKeyDown(e);
				};
				window.MagicKeyUpHandler = function(e) {
					that2._onKeyUp(e);
				};
			};
			window.postMobileMessage('FOCUSIFHWKBD');
		}

		window.L.DomEvent.on(this._map.getContainer(), 'mousedown touchstart', this._abortComposition, this);
	},

	onRemove: function() {
		window.MagicToGetHWKeyboardWorking = null;
		window.MagicKeyDownHandler = null;
		window.MagicKeyUpHandler = null;

		if (this._container && this._container.parentNode) {
			this._container.parentNode.removeChild(this._container);
		}

		if (!this.hasAccessibilitySupport()) {
			this._map.off('commandresult', this._onCommandResult, this);
		}
		window.L.DomEvent.off(this._textArea, 'focus blur', this._onFocusBlur, this);
		window.L.DomEvent.off(this._map.getContainer(), 'mousedown touchstart', this._abortComposition, this);

		if (this._cursorHandler)
			app.sectionContainer.removeSection(this._cursorHandler.name);
	},

	disable: function () {
		this._textArea.setAttribute('disabled', true);
	},

	enable: function () {
		this._textArea.removeAttribute('disabled');
	},

	_onPermission: function(e) {
		if (e.detail.perm === 'edit') {
			this._textArea.removeAttribute('disabled');
			this._textArea.setAttribute('contenteditable', 'true');
			this._textArea.removeAttribute('aria-disabled');
		} else {
			this._textArea.setAttribute('disabled', true);
			this._textArea.setAttribute('contenteditable', 'false');
			this._textArea.setAttribute('aria-disabled', 'true');
		}
	},

	_onCommandResult: function(e) {
		if (this.hasAccessibilitySupport())
			return;

		if (e.commandName === '.uno:Undo' || e.commandName === '.uno:Redo') {
			//undoing something does not trigger any input method
			// this causes the editable area content not to be a substring
			// of the document text; moreover this causes problem
			// in mobile working with suggestions:
			//i.e: type "than" and then select "thank" from suggestion
			//now undo and then again select "thanks" from suggestions
			//final output is "thans"
			//this happens because undo doesn't change the editable area content.
			//So better to clean the editable area content.
			this._emptyArea();
		}
	},

	hasFocus: function() {
		return this._textArea && this._textArea === document.activeElement;
	},

	_onFocusBlur: function(ev) {
		this._fancyLog(ev.type, '');
		this._statusLog('_onFocusBlur');
		var onoff = (ev.type === 'focus' ? window.L.DomEvent.on : window.L.DomEvent.off).bind(window.L.DomEvent);

		// Debug - connect first for saner logging.
		onoff(
			this._textArea,
			'copy cut compositionstart compositionupdate compositionend select keydown keypress keyup beforeinput textInput textinput input',
			this._onEvent,
			this
		);

		// we already do the same in _onBeforeInput, anyway for Safari on iOS is too late:
		// the selection is messed up, so we miss the first typed key.
		// Needed also after a Ctrl+C.
		if (ev.type === 'focus') {
			if (!this._isSelectionValid() || this._isCursorAtBeginning()) {
				if (this.hasAccessibilitySupport()) {
					this._setSelectionRange(this._lastSelectionStart, this._lastSelectionEnd);
				} else {
					this._emptyArea();
				}
			} else if (this.hasAccessibilitySupport()) {
				this._updateFocusedParagraph();
			}
		}

		onoff(this._textArea, 'input', this._onInput, this);
		onoff(this._textArea, 'beforeinput', this._onBeforeInput, this);
		onoff(this._textArea, 'compositionstart', this._onCompositionStart, this);
		onoff(this._textArea, 'compositionupdate', this._onCompositionUpdate, this);
		onoff(this._textArea, 'compositionend', this._onCompositionEnd, this);
		onoff(this._textArea, 'keydown', this._onKeyDown, this);
		onoff(this._textArea, 'keyup', this._onKeyUp, this);
		onoff(this._textArea, 'copy cut paste', this._map._handleDOMEvent, this._map);

		app.idleHandler.notifyActive();

		if (ev.type === 'blur' && this._isComposing) {
			this._abortComposition(ev);
		}
	},

	// Focus the textarea/contenteditable
	// @acceptInput (only on "mobile" (= mobile phone) or on iOS and Android in general) true if we want to
	// accept key input, and show the virtual keyboard.
	focus: function(acceptInput) {
		// Note that the acceptInput parameter intentionally
		// is a tri-state boolean: undefined, false, or true.

		// Clicking or otherwise focusing the map should focus on the clipboard
		// container in order for the user to input text (and on-screen keyboards
		// to pop-up), unless the document is read only.
		if (!this._map.isEditMode()) {
			this._setAcceptInput(false);
			// on clicking focus is important
			// specially in chrome once document loses focus it never gets it back
			// which causes shortcuts to stop working (i.e: print, search etc...)
			this._map.getContainer().focus();
			return;
		}

		// Trick to avoid showing the software keyboard: Set the textarea
		// read-only before focus() and reset it again after the blur()
		if (!window.ThisIsTheiOSApp && navigator.platform !== 'iPhone' && !window.mode.isChromebook()) {
			if (window.keyboard.guessOnscreenKeyboard() && acceptInput !== true)
				this._textArea.setAttribute('readonly', true);
		}

		if (!window.ThisIsTheiOSApp && navigator.platform !== 'iPhone' && !window.keyboard.guessOnscreenKeyboard()) {
			this._textArea.focus();
		} else if (acceptInput === true) {
			// On the iPhone, only call the textarea's focus() when we get an explicit
			// true parameter. On the other hand, never call the textarea's blur().

			// Calling blur() leads to so confusing behaviour with the keyboard not
			// showing up when we want. Better to have it show up a bit too long that
			// strictly needed.

			// Probably whether the calls to the textarea's focus() and blur() functions
			// actually do anything or not might depend on whether the call stack
			// originates in a user input event handler or not, for security reasons.

			// To investigate, uncomment the call to console.trace() at the start of
			// this function, and check when the topmost slot in the stack trace is
			// "(anonymous function)" in hammer.js (an event handler), and when it is
			// _onMessage (the WebSocket message handler in Socket.js).

			this._textArea.focus();
		}

		if (!window.ThisIsTheiOSApp && navigator.platform !== 'iPhone' && !window.mode.isChromebook()) {
			if (window.keyboard.guessOnscreenKeyboard() && acceptInput !== true) {
				this._setAcceptInput(false);
				this._textArea.blur();
				this._textArea.removeAttribute('readonly');
			} else {
				this._setAcceptInput(true);
			}
		} else if (acceptInput !== false) {
			this._setAcceptInput(true);
		} else {
			this._setAcceptInput(false);
		}
	},

	blur: function() {
		this._setAcceptInput(false);
		if (!window.ThisIsTheiOSApp && navigator.platform !== 'iPhone' && !window.mode.isChromebook())
			this._textArea.blur();
	},

	// Returns true if the last focus was to accept input.
	// Used to restore the keyboard.
	canAcceptKeyboardInput: function() {
		return this._acceptInput;
	},

	// Marks the content of the textarea/contenteditable as selected,
	// for system clipboard interaction.
	select: function() {
		this._setSelectionRange(0, this.getPlainTextContent().length);
	},

	getValue: function() {
		return this.getPlainTextContent();
	},

	getPlainTextContent: function() {
		return 	this._textArea.textContent;
	},

	getHTML: function() {
		return 	this._textArea.innerHTML;
	},

	_wrapContent: function(content) {
		var wrappedContent = this.hasAccessibilitySupport()
			? '<span id="readable-content" aria-hidden="false">' + content + '</span>'
			: content;
		return content.length === 0
			? this._initialContent
			: this._preSpaceChar + wrappedContent + this._postSpaceChar;
	},

	resetContent: function() {
		this._textArea.innerHTML = this._initialContent;
	},

	// Convert an array of Unicode code points to a string of UTF-16 code units. Workaround
	// for String.fromCodePoint() that is missing in IE.
	codePointsToString: function(codePoints) {
		var result = '';
		for (var i = 0; i < codePoints.length; ++i) {
			if (codePoints[i] <= 0xFFFF)
				result = (result +
					String.fromCharCode(codePoints[i]));
			else
				result = (result +
					String.fromCharCode(((codePoints[i] - 0x10000) >> 10) + 0xD800) +
					String.fromCharCode(((codePoints[i] - 0x10000) % 0x400) + 0xDC00));
		}
		return result;
	},

	// As the name says, this returns this._textArea.value as an array of numbers that are
	// Unicode code points. *Not* UTF-16 code units.
	getValueAsCodePoints: function(value) {
		// this._logCharCodeSequence(value);
		var arr = [];
		var code;
		for (var i = 0; i < value.length; ++i)
		{
			code = value.charCodeAt(i);
			arr.push(code);
		}
		return arr;
	},

	update: function() {
		if (this._container && this._map && app.file.textCursor.rectangle && app.activeDocument) {
			var rect = app.file.textCursor.rectangle;
			var pos = new cool.Point(
				Math.round(rect.v1X / app.dpiScale),
				Math.round(rect.v1Y / app.dpiScale)
			);
			this._setPos(pos);
		}
	},

	_initLayout: function() {
		this._container = window.L.DomUtil.create('div', 'clipboard-container');
		this._container.id = 'doc-clipboard-container';

		this._textAreaLabel = window.L.DomUtil.create('label', 'visuallyhidden', this._container);
		this._textAreaLabel.id = 'clipboard-area-label';
		this._textAreaLabel.innerHTML = _('Clipboard area');

		// The textarea allows the keyboard to pop up and so on.
		// Note that the contents of the textarea are NOT deleted on each composed
		// word, in order to make
		this._textArea = window.L.DomUtil.create('div', 'clipboard', this._container);
		this._textArea.id = 'clipboard-area';
		this._textArea.setAttribute('contenteditable', 'true');
		this._textArea.setAttribute('autocapitalize', 'off');
		this._textArea.setAttribute('autofocus', 'true');
		this._textArea.setAttribute('autocorrect', 'off');
		this._textArea.setAttribute('autocomplete', 'off');
		this._textArea.setAttribute('spellcheck', 'false');
		this._textArea.setAttribute('aria-labelledby', this._textAreaLabel.id);

		if (this.hasAccessibilitySupport()) {
			this._setSelectionFlag(false);
		}

		// Prevent automatic line breaks in the textarea. Without this,
		// chromium/blink will trigger input/insertLineBreak events by
		// just adding whitespace.
		// See https://developer.mozilla.org/en-US/docs/Web/HTML/Element/textarea#attr-wrap
		this._textArea.setAttribute('wrap', 'off');

		// Prevent autofocus
		this._textArea.setAttribute('disabled', true);

		if (window.L.Browser.cypressTest) {
			var that = this;
			this._textArea._hasAccessibilitySupport = function() {
				return that.hasAccessibilitySupport();
			};
			this._textArea._wrapContent = function(content) {
				return that._wrapContent(content);
			};
			this._textArea._getSelectionStart = function() {
				return that._getSelectionStart();
			};
			this._textArea._getSelectionEnd = function() {
				return that._getSelectionEnd();
			};
		}

		this._setupStyles();

		this._emptyArea();

		if (!this.hasAccessibilitySupport()) {
			var warningMessage =
				_('Screen reader support for text content is disabled. ') +
				_('You need to enable it both at server level and in the UI. ') +
				_('Look for the accessibility section in coolwsd.xml for server setting. ') +
				_('Also check the voice over toggle under {parentControl}.').replace(
					'{parentControl}',
					window.userInterfaceMode === 'notebookbar' ? _('the Help tab') : _('the View menu')
				);
			this._textArea.setAttribute('aria-description', warningMessage);
		}
	},

	_setupStyles: function() {
		if (this._isDebugOn) {
			// Style for debugging
			this._container.style.opacity = 0.5;
			this._textArea.style.cssText = 'border:1px solid red !important';
			this._textArea.style.width = window.L.Browser.cypressTest ? '1px' : '120px';
			this._textArea.style.height = window.L.Browser.cypressTest ? '1px' : '50px';
			this._textArea.style.overflow = 'display';

			this._textArea.style.fontSize = '20px';
			this._textArea.style.position = 'relative';
			this._textArea.style.left = '10px';
		} else {
			this._container.style.opacity = 0;
			this._textArea.style.width = '1px';
			this._textArea.style.height = '1px';
			this._textArea.style.caretColor = 'transparent';
			this._textArea.style.resize = 'none';
		}
		this._textArea.style['white-space'] = 'pre';
	},

	debug: function(debugOn) {
		this._isDebugOn = !!debugOn;
		this._setupStyles();
	},

	activeElement: function() {
		return this._textArea;
	},

	// Displays the caret and the under-caret marker.
	// Fetches the coordinates of the caret from the map's doclayer.
	showCursor: function() {
		if (!this._map._docLayer._cursorMarker || app.tile.size.x === 0 || !app.activeDocument) {
			return;
		}

		if (!this._map._docLayer._cursorMarker.isDomAttached()) {
			// Display caret
			this._map._docLayer._cursorMarker.add();
		}
		this._map._docLayer._cursorMarker.setMouseCursorForTextBox();

		// Move and display under-caret marker

		if (window.touch.currentlyUsingTouchscreen() && !app.activeDocument.activeView.hasTextSelection && this._cursorHandler) {
			this._cursorHandler.setPosition(app.file.textCursor.rectangle.pX1, app.file.textCursor.rectangle.pY2 + (0 * app.dpiScale));
			this._cursorHandler.setShowSection(true);
		} else if (this._cursorHandler) {
			this._cursorHandler.setShowSection(false);
		}

		// Move the hidden text area with the cursor
		this.update();
		// shape handlers hidden (if selected)
		this._map.fire('handlerstatus', {hidden: true});
		if (this._map._docLoaded && this._map.getDocType() === 'spreadsheet')
			this._map.onFormulaBarFocus();
	},

	// Hides the caret and the under-caret marker.
	hideCursor: function() {
		if (!this._map._docLayer._cursorMarker) {
			return;
		}
		if (this._map._docLayer._cursorMarker.isDomAttached())
			this._map._docLayer._cursorMarker.remove();

		if (this._cursorHandler)
			this._cursorHandler.setShowSection(false);

		// shape handlers visible again (if selected)
		this._map.fire('handlerstatus', {hidden: false});
	},

	_setPos: function(pos) {
		// the offset is needed since we have to move away from the edited text
		// or double clicks for selecting text doesn't work properly
		if (window.L.Browser.cypressTest) {
			// Some cypress tests require for the editable area to be as near as possible
			// to the caret overlay when editing. In fact a synthetic mouse click on
			// the editable area is performed in order to make it focused and ready for the input.
			// However, since this click is forwarded to core, it could change the current caret
			// position causing a test failure.
			pos.x += 10;
			pos.y += 10;
		}
		else {
			pos.y += this._isDebugOn ? 50 : 200;
		}
		this._container.style.transform = 'translate(' + pos.x + 'px, ' + pos.y + 'px)';
	},

	// Generic handle attached to most text area events, just for debugging purposes.
	_onEvent: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;
		var msg = {
			inputType: ev.inputType,
			data: ev.data,
			key: ev.key,
			isComposing: ev.isComposing
		};

		if ('key' in ev) {
			msg.key = ev.key;
			msg.keyCode = ev.keyCode;
			msg.code = ev.code;
			msg.which = ev.which;
		}
		this._fancyLog(ev.type, msg);
	},

	_fancyLog: function(type, payload) {
		// Avoid unhelpful exceptions
		if (payload === undefined)
			payload = 'undefined';
		else if (payload === null)
			payload = 'null';

		// Pretty-print on console (but only if "tile layer debug mode" is active)
		if (this._isDebugOn) {
			var state = this._isComposing ? 'C' : 'N';
			state += this._hasWorkingSelectionStart ? 'S' : '-';
			state += this._ignoreNextBackspace ? 'I' : '-';
			state += ' ';

			var textSel = this._getSelectionStart() + '!' + this._getSelectionEnd();
			state += textSel + ' ';

			var sel = window.getSelection();
			var content = this.getValue();
			if (sel === null)
				state += '-1';
			else
			{
				state += sel.rangeCount;

				state += ' ';
				var cursorPos = -1;
				for (var i = 0; i < sel.rangeCount; ++i)
				{
					var range = sel.getRangeAt(i);
					state += range.startOffset + '-' + range.endOffset + ' ';
					if (cursorPos < 0)
						cursorPos = range.startOffset;
				}
				if (sel.toString() !== '')
					state += ': "' + sel.toString() + '" ';

				// inject probable cursor
				if (cursorPos >= 0)
					content = content.slice(0, cursorPos) + '|' + content.slice(cursorPos);
			}

			state += '[' + this._deleteHint + '] ';

			window.app.console.log(
				+ new Date() + ' %cINPUT%c: ' + state
				+ '"' + content + '" ' + type + '%c ',
				'background:#bfb;color:black',
				'color:green',
				'color:black',
				JSON.stringify(payload)
			);
		}
	},

	_handleMisplacedCursorAtBeginning: function(ev) {
		// It seems some inputs e.g. GBoard can magically move the cursor from " | " to "|  "
		window.app.console.log('Oh dear, gboard sabotaged our cursor position, fixing');
		// But when we detect the problem only emit a delete when we have one.
		if (ev.inputType && ev.inputType === 'deleteContentBackward')
		{
			this._removeTextContent(1, 0);
			// Having mended it we now get a real backspace on input (sometimes)
			this._ignoreNextBackspace = true;
		}
		else if (this.hasAccessibilitySupport()) {
			this._setCursorPosition(this._getLastCursorPosition());
		}
		if (!this.hasAccessibilitySupport()) {
			this._emptyArea();
		}
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
			this._emptyArea();
		}
		else if (this._isInitialContent() && this._isCursorAtBeginning()) {
			this._handleMisplacedCursorAtBeginning(ev);
		}
		// Firefox is not able to delete the <img> post space. Since no 'input' event is generated,
		// we need to handle a <delete> at the end of the paragraph, here.
		if (window.L.Browser.gecko && this._isCursorAtEnd() && this._deleteHint === 'delete') {
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeTextContent(0, 1);
			this._emptyArea();
		}
		this._statusLog('_onBeforeInput ]');
	},

	_isDigit: function(asciiChar) {
		return asciiChar >= 48 && asciiChar <= 57;
	},

	_hasFormulaBarFocus: function() {
		return 	this._map && this._map.formulabar && this._map.formulabar.hasFocus();
	},

	// Fired when text has been entered, *during* and after composing/spellchecking
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
			if (!ignoreBackspace)
				this._removeTextContent(1, 0);
			this._emptyArea();
			return;
		}
		if (!this._hasPostSpace()) { // missing trailing space.
			if (this._map._debug.logKeyboardEvents) {
				window.app.console.log('Sending delete');
			}
			this._removeTextContent(0, 1);
			this._emptyArea();
			return;
		}

		var value = this.getValue();
		var content = this.getValueAsCodePoints(value);
		// Note that content is an array of Unicode code points

		// In the android keyboard when you try to erase in an empty area
		// and then enter some character,
		// The first character will likely travel with the cursor,
		// And that is caused because after entering the first character
		// cursor position is never updated by keyboard (I know it is strange)
		// so here we manually correct the position
		if (window.mode.isSmallScreenDevice() && content.length === 1 && this._lastContent.length === 0)
			this._setCursorPosition(1);

		var matchTo = 0;
		var sharedLength = Math.min(content.length, this._lastContent.length);
		while (matchTo < sharedLength && content[matchTo] === this._lastContent[matchTo])
			matchTo++;

		if (this._map._debug.logKeyboardEvents) {
			window.app.console.log('Comparison matchAt ' + matchTo + '\n' +
				'\tnew "' + this.codePointsToString(content) + '" (' + content.length + ')' + '\n' +
				'\told "' + this.codePointsToString(this._lastContent) + '" (' + this._lastContent.length + ')');
		}

		var removeBefore = this._lastContent.length - matchTo;
		var removeAfter = 0;

		if (this._lastContent.length > content.length)
		{
			// Pressing '<space><delete>' can delete our terminal space
			// such that subsequent deletes will do nothing; need to
			// detect and reset in this case.
			if (this._deleteHint === 'delete')
			{
				removeBefore--;
				removeAfter++;
			}
		}

		if (removeBefore > 0 || removeAfter > 0)
			this._removeTextContent(removeBefore, removeAfter);

		var newText = content;
		if (matchTo > 0)
			newText = newText.slice(matchTo);

		this._lastContent = content;

		if (newText.length > 0) {
			this._sendNewText(ev, content, newText);
		}

		// was a 'delete' and we need to reset world.
		if (removeAfter > 0)
			this._emptyArea();

		// special handling for formula bar
		this._finishFormulabarEditing(content, matchTo);

		// special handling for mentions
		if (this._map.getDocType() === 'text') {
			const contentStr = this.codePointsToString(content);
			const newPara = contentStr.length === 1;
			this._map.mention.handleMentionInput(ev, newPara);
		}

		this._statusLog('_onInput ]');
	},

	_sendNewText: function (ev, content, newText) {
		// When the cell formatted as percent, to trig percentage sign addition
		// automatically we send the first digit character as KeyEvent.
		if (this._map.getDocType() === 'spreadsheet' &&
			content.length === 1 && ev.inputType === 'insertText' &&
			this._isDigit(newText) && window.mode.isDesktop()) {
			this._sendKeyEvent(newText, this._unoKeyMap[newText], 'input');
		}
		else {
			this._sendText(this.codePointsToString(newText));
		}
	},

	_finishFormulabarEditing: function(content, matchTo) {
		if (this._hasFormulaBarFocus() && content.length) {
			var contentString = this.codePointsToString(content);
			if (contentString[matchTo] === '\n' || contentString.charCodeAt(matchTo) === 13) {
				app.dispatcher.dispatch('acceptformula');
			}
		}
	},

	// Sends the given (UTF-8) string of text to coolwsd, as IME (text composition)
	// messages
	_sendText: function(text) {
		// this._logCharCodeSequence(text);
		this._fancyLog('send-text-to-coolwsd', text);

		// MSIE/Edge cannot compare a string to "\n" for whatever reason,
		// so compare charcode as well
		if (text === '\n' || (text.length === 1 && text.charCodeAt(0) === 13)) {
			this._sendNewlineEvent();
		} else {
			// The composition messages doesn't play well with line breaks inside
			// the composed word (e.g. word and a newline are queued client-side
			// and are sent together), therefore split and send keystrokes accordingly.

			var parts = text.split(/[\n\r]/);
			var l = parts.length;
			for (var i = 0; i < l; i++) {
				if (i !== 0) {
					this._sendKeyEvent(13, 1280);
					if (!this.hasAccessibilitySupport()) {
						this._emptyArea();
					}
				}
				if (parts[i].length > 0) {
					this._sendCompositionEvent(parts[i]);
				}
			}
		}
	},

	// Empties the textarea / contenteditable element.
	// If the browser supports the 'inputType' property on 'input' events, then
	// add empty spaces to the textarea / contenteditable, in order to
	// always catch deleteContentBackward/deleteContentForward input events
	// (some combination of browser + input method don't fire those on an
	// empty contenteditable).
	_emptyArea: function(noSelect) {
		this._statusLog('_emptyArea [');
		this._fancyLog('empty-area');

		this._ignoreInputCount++;
		// Note: 0xA0 is 160, which is the character code for non-breaking space:
		// https://www.fileformat.info/info/unicode/char/00a0/index.htm

		// Using normal spaces would make FFX/Gecko collapse them into an
		// empty string.
		// FIXME: is that true !? ...

		// window.app.console.log('Set old/lastContent to empty');
		this._lastContent = [];
		if (this.hasAccessibilitySupport()) {
			this._setLastCursorPosition(0);
		}

		this.resetContent();

		// avoid setting the focus keyboard
		if (!noSelect && document.getElementById(this._textArea.id)) {
			this._setCursorPosition(0);
			if (this._hasWorkingSelectionStart === undefined)
				this._hasWorkingSelectionStart = (this._getSelectionStart() === 0);
		}

		this._fancyLog('empty-area-end');
		this._statusLog('_emptyArea ]');
		this._ignoreInputCount--;
	},

	_onCompositionStart: function(/*ev*/) {
		this._isComposing = true;
	},

	// Handled only in legacy situations ('input' events with an inputType
	// property are preferred).
	_onCompositionUpdate: function(ev) {
		app.idleHandler.notifyActive();
		this._onInput(ev);
	},

	// Chrome doesn't fire any "input/insertCompositionText" with "isComposing" set to false.
	// Instead , it fires non-standard "textInput" events, but those can be tricky
	// to handle since Chrome also fires "input/insertText" events.
	// The approach here is to use "compositionend" events *only in Chrome* to mark
	// the composing text as committed to the text area.
	_onCompositionEnd: function(ev) {
		app.idleHandler.notifyActive();
		this._isComposing = false;
		if (ev.data && ev.data.charCodeAt(ev.data.length-1) === 10) // 10 === charCode('\n')
			this._newlineHint = true;
		if (this.hasAccessibilitySupport()) {
			if (!this._isWrappedBySpan()) {
				this._restoreSpanWrapper();
			}
		}
		this._onInput(ev);
		if (this.hasAccessibilitySupport()) {
			this._updateFocusedParagraph();
		}
	},

	// Called when the user goes back to a word to spellcheck or replace it,
	// on a timeout.
	// Very difficult to handle right now, so the strategy is to panic and
	// empty the text area.
	_abortComposition: function(ev) {
		this._fancyLog('abort-composition', ev.type);
		if (this._isComposing)
			this._isComposing = false;
		if (this.hasAccessibilitySupport()) {
			this._isLeftRightArrow = 0;
			this._updateFocusedParagraph();
			this._requestFocusedParagraph();
		} else {
			this._emptyArea(document.activeElement !== this._textArea);
		}
	},

	_handleKeyDownForPopup: function (ev, id) {
		var popup = window.L.DomUtil.get(id);
		if (popup) {
			const entries = document.querySelectorAll('#' + id + ' span.ui-treeview-cell');
			if (ev.key === 'ArrowDown') {
				const initialFocusElement = entries[0];
				if (initialFocusElement) {
					initialFocusElement.tabIndex = 0;
					initialFocusElement.focus();
					ev.preventDefault();
					ev.stopPropagation();
				}
			} else if (ev.key === 'Enter' && entries.length === 1) {
					const event = new KeyboardEvent('keydown', {
						key: 'Enter',
						bubbles: true,
						cancelable: true,
					});
					entries[0].dispatchEvent(event);
					ev.preventDefault();
					ev.stopPropagation();
			} else if (ev.key === 'ArrowLeft' || ev.key === 'ArrowRight' ||
				ev.key === 'ArrowUp' || ev.key === 'Home' ||
				ev.key === 'End' || ev.key === 'PageUp' ||
				ev.key === 'PageDown' || ev.key === 'Enter' ||
				ev.key === 'Escape' || ev.key === 'Control' ||
				ev.key === 'Tab') {

				if (id === 'mentionPopup')
					this._map.mention.closeMentionPopup(false);
				else if (id === 'formulaautocompletePopup')
					this._map.fire('closepopup');
			}
		}
		return popup;
	},

	_onKeyDown: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;

		if (ev.keyCode === 8)
			this._deleteHint = 'backspace';
		else if (ev.keyCode === 46)
			this._deleteHint = 'delete';
		else {
			this._deleteHint = '';
		}
		this._newlineHint = ev.keyCode === 13;
		this._linebreakHint = this._newlineHint && ev.shiftKey;

		// In Firefox 117 up to 120 no copy/cut input event is emitted when CTRL+C/X is pressed
		// with no selection in a text element with contenteditable='true'. Since no copy/cut event
		// is emitted, Clipboard.copy/cut is never invoked. So we need to emit it manually.
		// To be honest it seems a Firefox bug. We need to check if they fix it in later version.
		if (!this.hasAccessibilitySupport() && !window.L.Browser.win &&
			window.L.Browser.gecko && window.L.Browser.geckoVersion >= '117.0' && window.L.Browser.geckoVersion <= '120.0' &&
			ev.ctrlKey && window.getSelection().isCollapsed) {
			if (ev.key === 'c') {
				document.execCommand('copy');
			}
			else if (ev.key === 'x') {
				document.execCommand('cut');
			}
		}

		if (this.hasAccessibilitySupport()) {
			if ((this._hasAnySelection && !this._isEditingInSelection && this._map.getDocType() !== 'spreadsheet') ||
				(!this._hasAnySelection && this._map.getDocType() === 'presentation')) {
				if (!window.L.Browser.cypressTest) {
					var allowedKeyEvent =
						this._map.keyboard.allowedKeyCodeWhenNotEditing[ev.keyCode] ||
						ev.ctrlKey ||
						ev.altKey ||
						(this._newlineHint && ev.shiftKey);
					if (!allowedKeyEvent) {
						window.console.log('TextInput._onKeyDown: any input default prevented since no shape editing is active.');
						ev.preventDefault();
					}
				}
			}
			// In order to allow screen reader to track caret navigation properly even if there is some connection delay
			// default behaviour for Left/Right arrow key press is no more prevented in Map.Keyboard._handleKeyEvent,
			// Here we set _isLeftRightArrow flag and handle some special cases.
			else {
				this._isLeftRightArrow = 0;
				if (ev.key === 'ArrowLeft') {
					this._isLeftRightArrow = -1;
				} else if (ev.key === 'ArrowRight') {
					this._isLeftRightArrow = 1;
				}

				// If we are at paragraph begin/end and left/right arrow is pressed, we need to prevent default behaviour
				if (this._isLeftRightArrow) {
					clearTimeout(this._onNavigationEndTimeout);
					if (!this._isSelectionValid() || this._isComposing ||
						(this._isLeftRightArrow > 0 && this._isCursorAtEnd()) ||
						(this._isLeftRightArrow < 0 && this._isCursorAtStart())) {
						this._log('_onKeyDown: preventDefault');
						ev.preventDefault();
					}
				}

				if (this._isLeftRightArrow) {
					if (this._listPrefixLength > 0) {
						var cursorPosition = this._getSelectionEnd();
						if (cursorPosition === this._listPrefixLength && this._isLeftRightArrow < 0) {
							// if caret is at begin of list entry content: "1. |Item 1" and shift+left arrow is pressed,
							// then caret is moved at the end of previous paragraph, if any; or it's not moved at all
							// if we are at the document beginning; so we only need to prevent default behaviour
							ev.preventDefault();
							if (!ev.shiftKey) {
								// when shift is not pressed, caret is moved ahead of list prefix: "|1. Item 1",
								// selection is cleared, if any
								this._updateCursorPosition(0);
							}
						}
						// if caret is ahead of list prefix: "|1. Item 1" and right arrow is pressed, with or without shift,
						// caret is moved at begin of list entry content: "1. |Item 1", nothing is selected
						if (cursorPosition === 0 && this._isLeftRightArrow > 0) {
							ev.preventDefault();
							this._updateCursorPosition(this._listPrefixLength);
						}
					}

					// When left/right arrow is pressed and text is selected, selection is cleared
					// and caret needs to be moved by one char left/right.
					// However, for an editable div the behaviour is different:
					// - when left arrow is pressed caret moves at start of previously selected text
					// - when right arrow is pressed caret moves at end of previously selected text
					// So we need to prevent default behaviour and simulate the same behaviour that occurs in LibreOffice.
					if (!ev.shiftKey) {
						var selection = window.getSelection();
						if (!selection.isCollapsed) {
							// The case where a left arrow is pressed with caret at the beginning of a list entry content
							// is already handled earlier.
							if (!(this._listPrefixLength > 0 &&
								this._lastCursorPosition === this._listPrefixLength && this._isLeftRightArrow < 0)) {
								this._log('_onKeyDown: pressed left/right arrows with selected text');
								ev.preventDefault();
								var pos = this._lastCursorPosition + this._isLeftRightArrow;
								// _updateCursorPosition takes care to normalize pos value
								this._updateCursorPosition(pos);
							}
						}
					}
				}
			}
		}

		// We want to open drowdown menu when cursor is above a dropdown content control.
		if (ev.code === 'Space' || ev.code === 'Enter') {
			if (this._map['stateChangeHandler'].getItemValue('.uno:ContentControlProperties') === 'enabled') {
				if (app.sectionContainer.doesSectionExist(app.CSections.ContentControl.name)) {
					var section = app.sectionContainer.getSectionWithName(app.CSections.ContentControl.name);
					if (section.sectionProperties.dropdownSection)
						section.sectionProperties.dropdownSection.onClick(null, ev);
				}
			}
		}

		if (ev.altKey && ev.code === 'KeyC') {
			// We want to focus on the comment menu if a comment is currently shown in Writer or Calc.
			// This is the key combination (Alt+C or Alt+Shift+C) for focusing on the comment menu.

			// On Calc, first press opens the comment, second press focuses on it.
			section = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
			if (section) {
				if (section.sectionProperties.selectedComment) {
					var id = section.sectionProperties.selectedComment.sectionProperties.menu.id;
					var element = document.getElementById(id);
					if (element)
						element.focus();
				}
				else if (this._map._docLayer._docType === 'spreadsheet') {
					if (section.sectionProperties.calcCurrentComment !== null)
						section.sectionProperties.calcCurrentComment.show();
				}
			}
		}

		this._handleKeyDownForPopup(ev, 'mentionPopup');
		this._handleKeyDownForPopup(ev, 'formulaautocompletePopup');
	},

	// Check arrow keys on 'keyup' event; using 'ArrowLeft' or 'ArrowRight'
	// shall empty the textarea, to prevent FFX/Gecko from ever not having
	// whitespace around the caret.
	// Across browsers, arrow up/down / home / end would move the caret to
	// the beginning/end of the textarea/contenteditable.
	_onKeyUp: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;

		app.idleHandler.notifyActive();
		if (!this.hasAccessibilitySupport() && !this._isComposing &&
			(ev.key === 'ArrowLeft' || ev.key === 'ArrowRight' ||
			ev.key === 'ArrowUp' || ev.key === 'ArrowDown' ||
			ev.key === 'Home' || ev.key === 'End' ||
			ev.key === 'PageUp' || ev.key === 'PageDown' ||
			ev.key === 'Escape')) {
			this._emptyArea();
		}

		if (this.hasAccessibilitySupport()) {
			if (ev.key === 'ArrowLeft' || ev.key === 'ArrowRight') {
				clearTimeout(this._onNavigationEndTimeout);
				this._onNavigationEndTimeout = setTimeout(function () {
					if (this._isLeftRightArrow) {
						this._isLeftRightArrow = 0;
						this._updateFocusedParagraph();
					}
				}.bind(this), 1000);
			}
			else if (ev.key === 'Shift') {
				if (this._isLeftRightArrow/* && this._remotePosition !== undefined*/) {
					this._isLeftRightArrow = 0;
					this._updateFocusedParagraph();
				}
			}
		}
	},

	// Used in the deleteContentBackward for deleting multiple characters with a single
	// message.
	// Will remove characters from the queue first, if there are any.
	_removeTextContent: function(before, after) {
		if (this._map._debug.logKeyboardEvents) {
			window.app.console.log('Remove ' + before + ' before, and ' + after + ' after');
		}

		this._followMyCursor();

		const windowId = app.map._docLayer._formID !== null ? app.map._docLayer._formID : this._map.getWinId();

		/// TODO: rename the event to 'removetextcontent' as soon as coolwsd supports it
		/// TODO: Ask Marco about it
		app.socket.sendMessage(
			'removetextcontext id=' + windowId +
			' before=' + before +
			' after=' + after
		);
	},

	_followMyCursor: function() {
		if (this._map && this._map.userList)
		this._map.userList.followUser(this._map._docLayer._getViewId());
	},

	// Tiny helper - encapsulates sending a 'textinput' websocket message.
	// sends a pair of "input" for a composition update pair with an "end"
	_sendCompositionEvent: function(text) {
		// this._logCharCodeSequence(text);

		// We want to trigger auto-correction, but not if we may
		// have to delete a count of characters in the future,
		// which is specific to crazy mobile keyboard / IMEs:
		if (!window.mode.isSmallScreenDevice() && !window.mode.isTablet() &&
			this._autoCorrectChars[text])
		{
			let codes;

			if (app.calc.decimalSeparator && this._map.numPadDecimalPressed) { // decimalSeparator is set only for Calc.
				this._map.numPadDecimalPressed = false;
				codes = this._autoCorrectChars[app.calc.decimalSeparator];
			}
			else codes = this._autoCorrectChars[text];

			this._sendKeyEvent(codes[0], codes[1], 'input');
			this._sendKeyEvent(codes[2], codes[3], 'up');
		}
		else
		{
			var encodedText = encodeURIComponent(text);
			var winId = this._map.getWinId();
			this._followMyCursor();
			app.socket.sendMessage(
				'textinput id=' + winId + ' text=' + encodedText);
		}
	},

	// Tiny helper - encapsulates sending a 'key' or 'windowkey' websocket message
	// "type" can be "input" (default) or "up"
	_sendKeyEvent: function(charCode, unoKeyCode, type) {
		this._followMyCursor();
		if (!type) {
			type = 'input';
		}
		if (this._map.editorHasFocus() || this._hasFormulaBarFocus()) {
			app.socket.sendMessage(
				'key type=' + type + ' char=' + charCode + ' key=' + unoKeyCode + '\n'
			);
		} else {
			app.socket.sendMessage(
				'windowkey id=' +
				this._map.getWinId() +
				' type=' +
				type +
				' char=' +
				charCode +
				' key=' +
				unoKeyCode +
				'\n'
			);
		}
	},

	_sendNewlineEvent: function() {
		// The composition messages doesn't play well with just a line break,
		// therefore send a keystroke.
		var unoKeyCode = this._linebreakHint ? 5376 : 1280;
		this._sendKeyEvent(13, unoKeyCode);
		this._newlineHint = false;
		if (this.hasAccessibilitySupport()) {
			var pos = this._getLastCursorPosition();
			var empty = this._lastContent.length === 0;
		}
		this._emptyArea();
		if (this.hasAccessibilitySupport()) {
			this._setSelectionFlag(false);
			if (pos === 0 && !empty)
				this._requestFocusedParagraph();
		}
	},

	_onCursorHandlerDragEnd: function(ev) {
		var cursorPos = this._map._docLayer._latLngToTwips(ev.target.getLatLng());
		this._map._docLayer._postMouseEvent('buttondown', cursorPos.x, cursorPos.y, 1, 1, 0);
		this._map._docLayer._postMouseEvent('buttonup', cursorPos.x, cursorPos.y, 1, 1, 0);
	},

	_setCursorPosition: function(pos) {
		try {
			this._setSelectionRange(pos, pos);
		} catch (err) {
			// old firefox throws an exception on start.
		}
	},

	setSwitchedToEditMode: function() {
		if (this.hasAccessibilitySupport()) {
			this._justSwitchedToEditMode = true;
		}
	},

	_setAcceptInput: function(accept) {
		if (window.L.Browser.cypressTest && this._textArea) {
			// This is used to track whether we *intended*
			// the keyboard to be visible or hidden.
			// There is no way track the keyboard state
			// programmatically, so the next best thing
			// is to track what we intended to do.
			this._textArea.setAttribute('data-accept-input', accept);
		}
		if (!this.hasAccessibilitySupport()) {
			this._acceptInput = accept;
		}
		else if (this._acceptInput !== accept) {
			this._acceptInput = accept;
			if (this._justSwitchedToEditMode && accept && this._isInitialContent()) {
				// We need to make the paragraph at the cursor position focused in core
				// so its content is sent to the editable area.
				this._justSwitchedToEditMode = false;
				if (this._map._docLayer && app.file.textCursor.visible) {
					window.app.console.log('A11yTextInput._setAcceptInput: going to emit a synthetic click after switching to edit mode.');
					let center = app.file.textCursor.rectangle.center;
					center[0] = Math.round(center[0]);
					center[1] = Math.round(center[1]);
					this._map._docLayer._postMouseEvent('buttondown', center[0], center[1], 1, 1, 0);
					this._map._docLayer._postMouseEvent('buttonup', center[0], center[1], 1, 1, 0);
				}
			}
		}
	},

	_hasPreSpace: function() {
		var child = this._textArea.firstChild;
		return child && child.id === 'pre-space';
	},

	_hasPostSpace: function() {
		var child = this._textArea.lastChild;
		return child && child.id === 'post-space';
	},

	_isInitialContent: function() {
		var children = this._textArea.childNodes;
		return children.length === 2 && this._hasPreSpace() && this._hasPostSpace();
	},

	_isCursorAtBeginning: function() {
		var selection = window.getSelection();
		return selection.isCollapsed && this._getSelectionStart() === -1;
	},

	_isSelectionValid: function() {
		return typeof this._getSelectionStart() === 'number' && typeof this._getSelectionEnd() === 'number';
	},

	_isCursorAtStart: function() {
		var selection = window.getSelection();
		return selection.isCollapsed && this._getSelectionStart() === 0;
	},

	_isCursorAtEnd: function() {
		var selection = window.getSelection();
		return selection.isCollapsed && this._getSelectionEnd() === this.getPlainTextContent().length;
	},

	// When the cursor is on a text node return the position wrt. the whole plain text content
	// When the cursor is on a pre- / post-space node return -1 / -2
	// Otherwise return undefined
	_getSelection: function(isStart) {
		var selection = window.getSelection();
		var node, offset;
		if (isStart) {
			node = selection.anchorNode;
			offset = selection.anchorOffset;
		} else {
			node = selection.focusNode;
			offset = selection.focusOffset;
		}

		if (!node)
			return;
		if (node.id === 'pre-space')      // cursor position: <div><img>|</img> ... <img></img></div>
			return -1;
		if (node.id === 'post-space')     // cursor position: <div><img></img> ... <img>|</img></div>
			return -2;
		if (node.id === this._textArea.id) {
			if (this._hasPreSpace()) {
				if (offset === 0)         // cursor position: <div>|<img></img> ... <img></img></div>
					return -1;
				else if (offset === 1)    // cursor position: <div><img></img>| ... <img></img></div>
					return 0;
			} else if (this._hasPostSpace()) {
				if (offset === node.childNodes.length)    // cursor position: <div><img></img> ... <img></img>|</div>
					return -2;
			}
		}
		if (node.nodeType !== Node.TEXT_NODE)
			return;

		var walker = document.createTreeWalker(this._textArea, NodeFilter.SHOW_TEXT);
		var currentNode = walker.nextNode();
		var pos = 0;
		while (currentNode) {
			if (currentNode === node) {
				return pos + offset;
			}
			pos += currentNode.textContent.length;
			currentNode = walker.nextNode();
		}
	},

	_getSelectionStart: function() {
		return this._getSelection(true);
	},

	_getSelectionEnd: function() {
		return this._getSelection(false);
	},

	// set the selection range
	// start/end refer to the string represented by the whole plain text content
	// it's not possible to set range start/end position at <img> delimiters
	_setSelectionRange: function(start, end) {
		this._statusLog('_setSelectionRange [');
		var selection = window.getSelection();
		selection.removeAllRanges();
		var range = document.createRange();

		// at the beginning we have no text node, so set cursor between <img> delimiters
		if (this._isInitialContent()) {
			range.setStart(this._textArea, 1);
			range.setEnd(this._textArea, 1);
			selection.addRange(range);
			this._statusLog('_setSelectionRange ]');
			return;
		}

		// Normalize input parameters
		var l = this.getPlainTextContent().length;
		if (start < 0)
			start = 0;
		if (end < 0)
			end = 0;
		if (start > l)
			start = l;
		if (end > l)
			end = l;
		if (start > end) {
			var t = start;
			start = end;
			end = t;
		}
		var msg = '_setSelectionRange:\n' +
			'    start: ' + start + ', end: ' + end +'\n';

		var startContainer = null;
		var walker = document.createTreeWalker(this._textArea, NodeFilter.SHOW_TEXT);
		var currentNode = walker.nextNode(); // first text node in <div>
		var pos = 0;
		// walker iterates over text nodes only
		while (currentNode) {
			msg += '    current node: ' + currentNode + ', text content: "' + currentNode.textContent + '"\n';
			var len = currentNode.textContent.length;
			msg += '    pos: ' + pos + ', len: ' + len +'\n';
			if (!startContainer && start <= pos + len) {
				startContainer = currentNode;
				range.setStart(currentNode, start - pos);
				msg += '    current node set as start: offset: ' + (start - pos) + '\n';
			}
			// The range end is set only after the range start has been set.
			if (startContainer && end <= pos + len) {
				range.setEnd(currentNode, end - pos);
				msg += '    current node set as end:   offset: ' + (end - pos) + '\n';
				break;
			}
			pos += len;
			currentNode = walker.nextNode();
		}

		// Get the selection object and add the range to it
		selection.addRange(range);
		window.console.log(msg);
		this._statusLog('_setSelectionRange ]');
	},

	_logCharCodeSequence: function(text) {
		var s = '[';
		for (var ii = 0; ii < text.length; ++ii) {
			if (ii > 0)
				s = s + ',';
			s = s + '0x' + text.charCodeAt(ii).toString(16);
		}
		s = s + ']';
		window.app.console.log('L.' + this._className + '._sendText: ' + s);
	},

	_log: function(msg) {
		if (!this._isDebugOn)
			return;
		window.app.console.log(msg);
	},

	_statusLog: function(header) {
		if (!this._isDebugOn)
			return;

		var msg = this._className + ' ' + header + '\n';
		msg += '  _lastContent: ' + this._lastContent  +'\n';
		msg += '  _lastContent: >' + this.codePointsToString(this._lastContent) + '<\n';
		if (this.hasAccessibilitySupport()) {
			msg += '  _remotePosition: ' + this._remotePosition + '\n';
			msg += '  _lastCursorPosition: ' + this._getLastCursorPosition() + '\n';
			msg += '  _lastSelectionStart: ' + this._lastSelectionStart + '\n';
			msg += '  _lastSelectionEnd: ' + this._lastSelectionEnd + '\n';
			msg += '  _hasSelection: ' + this._hasSelection + '\n';
		}
		msg += '  SelectionStart: ' + this._getSelectionStart() + '\n';
		msg += '  SelectionEnd: ' + this._getSelectionEnd() + '\n';
		msg += '  active element: ' + document.activeElement +'\n';
		msg += '  _isComposing: ' + this._isComposing + '\n';
		var textArea = this._textArea;
		msg += '  editable element: ' + '\n';
		msg += '    innerHTML: >' +  textArea.innerHTML + '<\n';
		msg += '    innerText: >' +  textArea.innerText + '<\n';
		msg += '    textContent: >' +  textArea.textContent + '<\n';
		msg += '    has focus: ' + (textArea === document.activeElement) + '\n';
		msg += '    has pre space: ' + this._hasPreSpace() + '\n';
		msg += '    has post space: ' + this._hasPostSpace() + '\n';

		var children = textArea.childNodes;
		for (var i = 0; i < children.length; ++i) {
			var child = children[i];
			msg += '    child: ' + child + '\n';
			if (child) {
				msg += '      name: ' + child.nodeName + '\n';
				msg += '      textContent: >' + child.textContent + '<\n';
			}
		}

		var selection = window.getSelection();
		msg += '  selection: \n';
		msg += '    content: >' + selection.toString() + '<\n';
		msg += '    range count: ' + selection.rangeCount + '\n';
		msg += '    anchorNode: ' + selection.anchorNode + '\n';
		if (selection.anchorNode) {
			msg += '      name: ' + selection.anchorNode.nodeName + '\n';
			msg += '      textContent: >' + selection.anchorNode.textContent + '<\n';
			for (i = 0; i < children.length; ++i) {
				if (children[i] === selection.anchorNode) {
					msg += '      equal to child: ' + i + '\n';
				} else if (children[i].contains(selection.anchorNode)) {
					msg += '      contained in child: ' + i + '\n';
				}
			}
		}
		msg += '    focusNode: ' + selection.focusNode + '\n';
		if (selection.focusNode) {
			msg += '      name: ' + selection.focusNode.nodeName + '\n';
			msg += '      textContent: >' + selection.focusNode.textContent + '<\n';
			for (i = 0; i < children.length; ++i) {
				if (children[i] === selection.focusNode) {
					msg += '      equal to child: ' + i + '\n';
				} else if (children[i].contains(selection.focusNode)) {
					msg += '      contained in child: ' + i + '\n';
				}
			}
		}
		msg += '    editable element contains anchorNode: ' + textArea.contains(selection.anchorNode) + '\n';
		msg += '    anchorNode == focusNode ? ' + (selection.anchorNode === selection.focusNode) + '\n';
		msg += '    anchorOffset: ' + selection.anchorOffset + '\n';
		msg += '    focusOffset: ' + selection.focusOffset + '\n';
		msg += '    is collapsed: ' + selection.isCollapsed + '\n';

		window.app.console.log(msg);
	},

	onAccessibilityCaretChange: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	onAccessibilityTextSelectionChanged: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	onAccessibilityFocusedCellChanged: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	onAccessibilityEditingInSelectionState: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	onAccessibilitySelectionChanged: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	setA11yFocusedParagraph: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	},

	setA11yCaretPosition: function () {
		window.app.console.warn('invalid call: ' + (new Error()).stack);
	}
});

window.L.textInput = function() {
	return new window.L.TextInput();
};
