/* -*- js-indent-level: 8 -*- */
/*
 * window.L.Map.Keyboard is handling keyboard interaction with the map, enabled by default.
 *
 * It handles keyboard interactions which are NOT text input, including those which
 * don't require edit permissions (e.g. page scroll). Text input is handled
 * at TextInput.
 */

/* global app UNOKey TileManager */

window.L.Map.mergeOptions({
	keyboard: true,
	keyboardPanOffset: 20,
	keyboardZoomOffset: 1
});

window.L.Map.Keyboard = window.L.Handler.extend({

	keymap: {
		8   : UNOKey.BACKSPACE,
		9   : UNOKey.TAB,
		13  : UNOKey.RETURN,
		16  : null, // shift		: UNKOWN
		17  : null, // ctrl		: UNKOWN
		18  : null, // alt		: UNKOWN
		19  : null, // pause/break	: UNKOWN
		20  : null, // caps lock	: UNKOWN
		27  : UNOKey.ESCAPE,
		32  : UNOKey.SPACE,
		33  : UNOKey.PAGEUP,
		34  : UNOKey.PAGEDOWN,
		35  : UNOKey.END,
		36  : UNOKey.HOME,
		37  : UNOKey.LEFT,
		38  : UNOKey.UP,
		39  : UNOKey.RIGHT,
		40  : UNOKey.DOWN,
		45  : UNOKey.INSERT,
		46  : UNOKey.DELETE,
		48  : UNOKey.NUM0,
		49  : UNOKey.NUM1,
		50  : UNOKey.NUM2,
		51  : UNOKey.NUM3,
		52  : UNOKey.NUM4,
		53  : UNOKey.NUM5,
		54  : UNOKey.NUM6,
		55  : UNOKey.NUM7,
		56  : UNOKey.NUM8,
		57  : UNOKey.NUM9,
		65  : UNOKey.A,
		66  : UNOKey.B,
		67  : UNOKey.C,
		68  : UNOKey.D,
		69  : UNOKey.E,
		70  : UNOKey.F,
		71  : UNOKey.G,
		72  : UNOKey.H,
		73  : UNOKey.I,
		74  : UNOKey.J,
		75  : UNOKey.K,
		76  : UNOKey.L,
		77  : UNOKey.M,
		78  : UNOKey.N,
		79  : UNOKey.O,
		80  : UNOKey.P,
		81  : UNOKey.Q,
		82  : UNOKey.R,
		83  : UNOKey.S,
		84  : UNOKey.T,
		85  : UNOKey.U,
		86  : UNOKey.V,
		87  : UNOKey.W,
		88  : UNOKey.X,
		89  : UNOKey.Y,
		90  : UNOKey.Z,
		91  : null, // left window key	: UNKOWN
		92  : null, // right window key	: UNKOWN
		93	: UNOKey.CONTEXTMENU,
		96  : UNOKey.NUM0,
		97  : UNOKey.NUM1,
		98  : UNOKey.NUM2,
		99  : UNOKey.NUM3,
		100 : UNOKey.NUM4,
		101 : UNOKey.NUM5,
		102 : UNOKey.NUM6,
		103 : UNOKey.NUM7,
		104 : UNOKey.NUM8,
		105 : UNOKey.NUM9,
		106 : UNOKey.MULTIPLY,
		107 : UNOKey.ADD,
		109 : UNOKey.SUBTRACT,
		110 : UNOKey.DECIMAL,
		111 : UNOKey.DIVIDE,
		112 : UNOKey.F1,
		113 : UNOKey.F2,
		114 : UNOKey.F3,
		115 : UNOKey.F4,
		116 : UNOKey.F5,
		117 : UNOKey.F6,
		118 : UNOKey.F7,
		119 : UNOKey.F8,
		120 : UNOKey.F9,
		121 : UNOKey.F10,
		122 : UNOKey.F11,
		123 : UNOKey.F12,
		144 : UNOKey.NUMLOCK,
		145 : UNOKey.SCROLLLOCK,
		173 : UNOKey.SUBTRACT,
		186 : UNOKey.SEMICOLON,
		187 : UNOKey.EQUAL,
		188 : UNOKey.COMMA,
		189 : UNOKey.SUBTRACT,
		190 : null, // period		: UNKOWN
		191 : null, // forward slash	: UNKOWN
		192 : null, // grave accent	: UNKOWN
		219 : null, // open bracket	: UNKOWN
		220 : null, // back slash	: UNKOWN
		221 : null, // close bracket	: UNKOWN
		222 : null  // single quote	: UNKOWN
	},

	handleOnKeyDownKeys: {
		// these keys need to be handled on keydown in order for them
		// to work on chrome
		// Backspace and Delete are handled at TextInput's 'beforeinput' handler.
		9   : true, // tab
		19  : true, // pause/break
		20  : true, // caps lock
		27  : true, // escape
		33  : true, // page up
		34  : true, // page down
		35  : true, // end
		36  : true, // home
		37  : true, // left arrow
		38  : true, // up arrow
		39  : true, // right arrow
		40  : true, // down arrow
		45  : true, // insert
		93  : true, // context menu
		113 : true  // f2
	},

	// See https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode for list of keycodes
	keyCodes: {

		pageUp:   33,
		pageDown: 34,
		enter:    13,
		BACKSPACE:8,
		TAB:      9,
		SPACE :   32,
		SHIFT:    16, // shift		: UNKOWN
		CTRL:     17, // ctrl		: UNKOWN
		ALT:      18, // alt		: UNKOWN
		PAUSE:    19, // pause/break	: UNKOWN
		CAPSLOCK: 20, // caps lock	: UNKOWN,
		PAGEUP:   33,
		PAGEDOWN: 34,
		END:      35,
		HOME:     36,
		LEFT:     37,
		UP:       38,
		RIGHT:    39,
		DOWN:     40,
		INSERT:   45,
		DELETE:   46,
		NUM0:     [48,96], // two values because of different mapping in mac and windows for same keys
		NUM1:     [49,97],
		NUM2:     [50,98],
		NUM3:     [51,99],
		NUM4:     [52,100],
		NUM5:     [53,101],
		NUM6:     [54,102],
		NUM7:     [55,103],
		NUM8:     [56,103],
		NUM9:     [57,104],
		A:        65,
		B:        66,
		C:        [67,99],
		//c:        99,
		D:        68,
		E:        69,
		//f:        70, no need for separate as for Windows it will remain the same
		F:        70,
		G:        71,
		H:        72,
		I:        73,
		J:        74,
		K:        75,
		L:        76,
		M:        77,
		N:        78,
		O:        79,
		P:        80,
		Q:        81,
		R:        82,
		S:        83,
		T:        84,
		U:        85,
		V:        [86,118],
		//v:        118, // this is for MAC as the value of v changes when we press keydown
		W:        87,
		X:        [88,120],
		//x:        120, // this is for MAC as the value of x changes when we press keydown
		Y:        89,
		Z:        90,
		LEFTWINDOWKEY :    [91,91], // left window key	: UNKOWN  and also for MAC
		RIGHTWINDOWKEY:    [92,93], // right window key	: UNKOWN  and also for MAC
		CONTEXTMENU:       93, // context menu
		// NUM0:     96,
		// NUM1:     97,
		// NUM2:     98,
		// NUM3:     99,
		// NUM4:     100,
		// NUM5:     101,
		// NUM6:     102,
		// NUM7:     103,
		// NUM8:     104,
		MULTIPLY:    106,
		ADD:         107,
		//SUBTRACT:    109,
		DECIMAL:     110,
		DIVIDE:      111,
		F1:       112,
		F2:       113,
		F3:       114,
		F4:       115,
		F5:       116,
		F6:       117,
		F7:       118,
		F8:       119,
		F9:       120,
		F10:      121,
		F11:      122,
		F12 :     123,
		NUMLOCK:  144,
		SCROLLLOCK:   145,
		SUBTRACT:     [109,173,189],
		SEMICOLON:    186,
		EQUAL:        187,
		COMMA:        188,
		//SUBTRACT:     189,
		PERIOD:       190, // period		: UNKOWN
		FORWARDSLASH: 191, // forward slash	: UNKOWN
		GRAVEACCENT:  192, // grave accent	: UNKOWN
		OPENBRACKET:  219, // open bracket	: UNKOWN
		BACKSLASH:    220, // back slash	: UNKOWN
		CLOSEBRACKET: 221, // close bracket	: UNKOWN
		SINGLEQUOTE : 222  // single quote	: UNKOWN

	},

	navigationKeyCodes: {
		left:    [37],
		right:   [39],
		down:    [40],
		up:      [38],
		zoomIn:  [187, 107, 61, 171],
		zoomOut: [189, 109, 173]
	},

	allowedKeyCodeWhenNotEditing: {
		27:    true,  // ESC
		33:    true,  // pageUp
		34:    true,  // pageDown
		13:    true,  // enter
		 8:    true,  // BACKSPACE
		16:    true,  // SHIFT
		17:    true,  // CTRL
		18:    true,  // ALT
		19:    true,  // PAUSE
		20:    true,  // CAPSLOCK
		35:    true,  // END
		36:    true,  // HOME
		45:    true,  // INSERT
		46:    true,  // DELETE
		91:    true,  // LEFTWINDOWKEY
		92:    true,  // RIGHTWINDOWKEY
		93:    true,  // CONTEXTMENU
		112:    true,  //F1
		113:    true,  //F2
		114:    true,  //F3
		115:    true,  //F4
		116:    true,  //F5
		117:    true,  //F6
		118:    true,  //F7
		119:    true,  //F8
		120:    true,  //F9
		121:    true,  //F10
		122:    true,  //F11
		123:    true,  //F12
		144:    true,  //NUMLOCK
		145:    true,  //SCROLLLOCK
	},

	initialize: function (map) {
		this._map = map;
		this._setPanOffset(map.options.keyboardPanOffset);
		this._setZoomOffset(map.options.keyboardZoomOffset);
		this.modifier = 0;
		window.KeyboardShortcuts.initialize(map);
	},


	addHooks: function () {
		var container = this._map._container;

		// make the container focusable by tabbing
		if (container.tabIndex === -1) {
			container.tabIndex = '0';
		}

		this._keyEventContainer = document.getElementById('document-container');
		window.L.DomEvent.on(this._keyEventContainer, 'keydown keyup keypress', this._onKeyDown, this);
		window.L.DomEvent.on(window.document, 'keydown', this._globalKeyEvent, this);
		window.document.addEventListener('keyup', this._globalKeyUp.bind(this), true);
	},

	removeHooks: function () {
		window.L.DomEvent.off(this._keyEventContainer, 'keydown keyup keypress', this._onKeyDown, this);
		window.L.DomEvent.off(window.document, 'keydown', this._globalKeyEvent, this);
		window.document.removeEventListener('keyup', this._globalKeyUp.bind(this));
	},

	_ignoreKeyEvent: function(ev) {
		var shift = ev.shiftKey ? app.UNOModifier.SHIFT : 0;
		if (shift && (ev.keyCode === this.keyCodes.INSERT || ev.keyCode === this.keyCodes.DELETE)) {
			// don't handle shift+insert, shift+delete
			// These are converted to 'cut', 'paste' events which are
			// automatically handled by us, so avoid double-handling
			return true;
		}
	},

	_setPanOffset: function (pan) {
		var keys = {},
		    codes = this.navigationKeyCodes,
		    i, len;

		for (i = 0, len = codes.left.length; i < len; i++) {
			keys[codes.left[i]] = [-1 * pan, 0];
		}
		for (i = 0, len = codes.right.length; i < len; i++) {
			keys[codes.right[i]] = [pan, 0];
		}
		for (i = 0, len = codes.down.length; i < len; i++) {
			keys[codes.down[i]] = [0, pan];
		}
		for (i = 0, len = codes.up.length; i < len; i++) {
			keys[codes.up[i]] = [0, -1 * pan];
		}
	},

	_setZoomOffset: function (zoom) {
		var keys = this._zoomKeys = {},
		    codes = this.navigationKeyCodes,
		    i, len;

		for (i = 0, len = codes.zoomIn.length; i < len; i++) {
			keys[codes.zoomIn[i]] = zoom;
		}
		for (i = 0, len = codes.zoomOut.length; i < len; i++) {
			keys[codes.zoomOut[i]] = -zoom;
		}
	},

	// Convert javascript key codes to UNO key codes.
	_toUNOKeyCode: function (keyCode) {
		return this.keymap[keyCode] || keyCode;
	},

	// _onKeyDown - called only as a DOM event handler
	// Calls _handleKeyEvent(), but only if the event doesn't have
	// a charCode property (set to something different than 0) - that ignores
	// any 'beforeinput', 'keypress' and 'input' events that would add
	// printable characters. Those are handled by TextInput.js.
	_onKeyDown: function (ev) {
		if (this._map.uiManager.isUIBlocked()
			|| (this._map._docLayer && (this._map._docLayer._docType === 'presentation' || this._map._docLayer._docType === 'drawing') && this._map._docLayer._preview.partsFocused === true)
		)
			return;

		if (this._map._debug.logKeyboardEvents) {
			window.app.console.log('keyboard handler:', ev.type, ev.key, ev.charCode, this._expectingInput, ev);
		}

		var completeEvent = app.socket.createCompleteTraceEvent('L.Map.Keyboard._onKeyDown', { type: ev.type, charCode: ev.charCode });

		if (ev.charCode == 0) {
			this._handleKeyEvent(ev);
		}
		if (this._map._docLayer)
			if (ev.shiftKey && ev.type === 'keydown')
				this._map._docLayer.shiftKeyPressed = true;
			else if (ev.keyCode === this.keyCodes.SHIFT && ev.type === 'keyup')
				this._map._docLayer.shiftKeyPressed = false;
		if (completeEvent)
			completeEvent.finish();
	},

	_globalKeyEvent: function(ev) {
		if (this._map.uiManager.isUIBlocked())
			return;

		if (app.UI.notebookbarAccessibility) {
			app.UI.notebookbarAccessibility.onDocumentKeyDown(ev);
		}

		if (ev.shortCutActivated === true) {
			window.app.console.log('Shortcut for: ' + ev.code + ' already handled');
			return;
		}

		if (window.KeyboardShortcuts.processEvent(app.UI.language.fromURL, ev)) {
			return;
		}
		if (this._map.jsdialog
			&& (this._map.jsdialog.hasDialogOpened() || this._map.jsdialog.hasSnackbarOpened() || this._map.jsdialog.hasDropdownOpened())
			&& this._map.jsdialog.handleKeyEvent(ev)) {
			ev.preventDefault();
			return;
		}
		else if (this._map._docLayer && (this._map._docLayer._docType === 'presentation' || this._map._docLayer._docType === 'drawing') && this._map._docLayer._preview.partsFocused === true) {
			if (ev.shiftKey && !ev.ctrlKey && !ev.altKey
				&& (ev.keyCode === this.keyCodes.DOWN || ev.keyCode === this.keyCodes.UP || ev.keyCode === this.keyCodes.HOME || ev.keyCode === this.keyCodes.END)
				&& ev.type === 'keydown') {

				if (ev.keyCode === this.keyCodes.UP)
					this._map._docLayer._preview._modifySelectedPartRange("UP");
				else if (ev.keyCode === this.keyCodes.DOWN)
					this._map._docLayer._preview._modifySelectedPartRange("DOWN");
				else if (ev.keyCode === this.keyCodes.HOME)
					this._map._docLayer._preview._selectPartRange(undefined, 0);
				else if (ev.keyCode === this.keyCodes.END)
					this._map._docLayer._preview._selectPartRange(undefined, this._map._docLayer._parts - 1);

				ev.preventDefault();
			}
			else if (!this.modifier && (ev.keyCode === this.keyCodes.DOWN || ev.keyCode === this.keyCodes.UP ||
				               ev.keyCode === this.keyCodes.RIGHT || ev.keyCode === this.keyCodes.LEFT ||
				               ev.keyCode === this.keyCodes.PAGEDOWN || ev.keyCode === this.keyCodes.PAGEUP ||
				               ev.keyCode === this.keyCodes.DELETE || ev.keyCode === this.keyCodes.BACKSPACE ||
				               ev.keyCode === this.keyCodes.HOME || ev.keyCode === this.keyCodes.END)
				           && ev.type === 'keydown') {

				var deletePart = (ev.keyCode === this.keyCodes.DELETE || ev.keyCode === this.keyCodes.BACKSPACE) ? true : false;

				if (!deletePart) {
					if (ev.keyCode === this.keyCodes.HOME) {
						this._map.deselectAll();
						this._map.setPart(0);
					} else if (ev.keyCode === this.keyCodes.END) {
						this._map.deselectAll();
						this._map.setPart(this._map._docLayer._parts - 1);
					} else {
						var partToSelect = (ev.keyCode === this.keyCodes.UP || ev.keyCode === this.keyCodes.LEFT ||
										ev.keyCode === this.keyCodes.PAGEUP) ? 'prev' : 'next';

						this._map.deselectAll();
						this._map.setPart(partToSelect);
					}
					if (app.file.fileBasedView)
						this._map._docLayer._checkSelectedPart();
				}
				else if (this._map.isEditMode() && !app.file.fileBasedView &&
						this._map.jsdialog &&
						!this._map.jsdialog.hasDialogOpened()
				) {
					this._map.deletePage(this._map._docLayer._selectedPart);
				}
				ev.preventDefault();
			}
			else if (ev.ctrlKey && !ev.altKey && ev.keyCode === this.keyCodes.HOME)
				app.map.setPart(0);
			else if (ev.ctrlKey && !ev.altKey && ev.keyCode === this.keyCodes.END)
				app.map.setPart(app.map._docLayer._parts - 1);
			else if (ev.ctrlKey && !ev.altKey && this.keyCodes.C.includes(ev.keyCode)) {
				app.map._clip.clearSelection();
				app.map._clip.setTextSelectionType('slide');
			}
			else if (!ev.ctrlKey && !ev.shiftKey) {
				this._map._docLayer._preview.partsFocused = false;
				app.map._clip.clearSelection();
				app.map.focus();
			}
		}
	},

	_globalKeyUp: function (ev) {
		if (this._map.uiManager.isUIBlocked()) {
			return;
		}

		if (app.UI.notebookbarAccessibility &&
		    app.UI.notebookbarAccessibility.accessibilityInputElement !== document.activeElement) {
			app.UI.notebookbarAccessibility.onDocumentKeyUp(ev);
		}
	},

	// _handleKeyEvent - checks if the given keyboard event shall trigger
	// a message to coolwsd, and calls the given keyEventFn(type, charcode, keycode)
	// callback if so.
	// Called from _onKeyDown
	_handleKeyEvent: function (ev, keyEventFn) {
		if (this._map.uiManager.isUIBlocked())
			return;

		app.idleHandler.notifyActive();
		if ((this._map.slideShow && this._map.slideShow.fullscreen) ||
			(this._map.slideShowPresenter && this._map.slideShowPresenter.isFullscreen())) {
			return;
		}

		if (ev.code === 'NumpadDecimal') this._map.numPadDecimalPressed = true;
		else this._map.numPadDecimalPressed = false;

		var docLayer = this._map._docLayer;

		// if any key is pressed, we stop the following other users
		if (docLayer) this._map.userList.followUser(docLayer._viewId, false);

		if (window.KeyboardShortcuts.processEvent(app.UI.language.fromURL, ev)) {
			ev.shortCutActivated = true;
			return;
		}
		if (!keyEventFn && docLayer && docLayer.postKeyboardEvent) {
			// default is to post keyboard events on the document
			keyEventFn = window.L.bind(docLayer.postKeyboardEvent, docLayer);
		}

		this.modifier = 0;
		var shift = ev.shiftKey ? app.UNOModifier.SHIFT : 0;
		var ctrl = (ev.ctrlKey || ev.metaKey) ? app.UNOModifier.CTRL : 0;
		var alt = ev.altKey ? app.UNOModifier.ALT : 0;
		var location = ev.location;
		this.modifier = shift | ctrl | alt;

		// On Windows, pressing AltGr = Alt + Ctrl
		// Presence of AltGr is detected if previous Ctrl + Alt 'location' === 2 (i.e right)
		// because Ctrl + Alt + <some char> won't give any 'location' information.
		if (ctrl && alt) {
			if (ev.type === 'keydown' && location === 2) {
				this._prevCtrlAltLocation = location;
				return;
			}
			else if (location === 1) {
				this._prevCtrlAltLocation = undefined;
			}

			if (this._prevCtrlAltLocation === 2 && location === 0) {
				// and we got the final character
				if (ev.type === 'keypress') {
					ctrl = alt = this.modifier = 0;
				}
				else {
					// Don't handle remnant 'keyup'
					return;
				}
			}
		}

		if (ctrl) {
			if (this._handleCtrlCommand(ev)) {
				return;
			}
		}

		var charCode = ev.charCode;
		var keyCode = ev.keyCode;

		var DEFAULT =0;
		//var MAC=1; use this when you encounter a MAC value

		if ((this.modifier == app.UNOModifier.ALT || this.modifier == app.UNOModifier.SHIFT + app.UNOModifier.ALT) &&
		    keyCode >= this.keyCodes.NUM0[DEFAULT]) {
			// Presumably a Mac or iOS client accessing a "special character". Just ignore the alt modifier.
			// But don't ignore it for Alt + non-printing keys.
			this.modifier -= alt;
			alt = 0;
		}

		var unoKeyCode = this._toUNOKeyCode(keyCode);

		if (this.modifier) {
			unoKeyCode |= this.modifier;
			if (ev.type !== 'keyup' && (this.modifier !== shift || (keyCode === this.keyCodes.SPACE && !app.file.textCursor.visible))) {
				if (!this._map.isEditMode() && !this._isModifierNavigationKey(keyCode)) {
					ev.preventDefault();
					return;
				}
				if (keyEventFn) {
					keyEventFn('input', charCode, unoKeyCode);
					ev.preventDefault();
				}

				return;
			}
		}

		if (this._map.stateChangeHandler._items['.uno:SlideMasterPage'] === 'true') {
			ev.preventDefault();
			return;
		}

		if (this._map.isEditMode()) {
			TileManager.resetPreFetching();

			if (this._ignoreKeyEvent(ev)) {
				// key ignored
			}
			else if (ev.type === 'keydown') {
				if (this.handleOnKeyDownKeys[keyCode] && charCode === 0) {
					if (keyEventFn) {
						keyEventFn('input', charCode, unoKeyCode);
						if (!(this._map._textInput.hasAccessibilitySupport() && this._map._textInput.hasFocus() &&
							(keyCode === this.keyCodes.LEFT || keyCode === this.keyCodes.RIGHT))) {
							ev.preventDefault();
						} else {
							window.app.console.log('Map.Keyboard._handleKeyEvent: default not prevented for left or right arrow.');
						}
					}
				}
				if (this._map._debug.tileInvalidationsOn) {
					this._map._debug.addTileInvalidationKeypress();
				}
			}
			else if ((ev.type === 'keypress') && (!this.handleOnKeyDownKeys[keyCode] || charCode !== 0)) {
				if (keyCode === this.keyCodes.BACKSPACE || keyCode === this.keyCodes.DELETE || keyCode === this.keyCodes.enter)
				{
					// handled generically in TextInput.js
					window.app.console.log('Ignore backspace/delete/enter keypress');
					return;
				}
				if (charCode === keyCode && charCode !== this.keyCodes.enter) {
					// Chrome sets keyCode = charCode for printable keys
					// while LO requires it to be 0
					keyCode = 0;
					unoKeyCode = this._toUNOKeyCode(keyCode);
				}
				if (this._map._debug.tileInvalidationsOn) {
					this._map._debug.addTileInvalidationKeypress();
				}

				if (keyEventFn) {
					keyEventFn('input', charCode, unoKeyCode);
				}
			}
			else if (ev.type === 'keyup') {
				if ((this.handleOnKeyDownKeys[keyCode] && charCode === 0) ||
				    (this.modifier) ||
				    unoKeyCode === UNOKey.RETURN) {
					if (keyEventFn) {
						keyEventFn('up', charCode, unoKeyCode);
					}
				} else {
					// was handled as textinput
				}
			}
			if (keyCode === this.keyCodes.TAB) {
				// tab would change focus to other DOM elements
				ev.preventDefault();
			}
		}
		else if (ev.type === 'keydown') {
			var key = ev.keyCode;
			var map = this._map;
			if (['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Home', 'End'].includes(ev.code)) {
				keyEventFn('input', charCode, unoKeyCode);
			}
			else if (key in this._zoomKeys) {
				map.setZoom(map.getZoom() + (ev.shiftKey ? 3 : 1) * this._zoomKeys[key], null, true /* animate? */);
			}
			else if (ev.key && ev.key.length === 1 && !ev.ctrlKey && !ev.altKey && !map.isEditMode()) {
				map.uiManager.showViewModeAttention();
			}
		}

		window.L.DomEvent.stopPropagation(ev);
	},

	_isCtrlKey: function (e) {
		if (window.ThisIsTheiOSApp || window.L.Browser.mac)
			return e.metaKey;
		else
			return e.ctrlKey;
	},

	// Keys that should still be sent to core with modifiers in read-only mode.
	_isModifierNavigationKey: function (keyCode) {
		return (keyCode >= this.keyCodes.PAGEUP && keyCode <= this.keyCodes.DOWN) ||
			keyCode === this.keyCodes.TAB ||
			keyCode === this.keyCodes.A ||
			(keyCode >= this.keyCodes.F1 && keyCode <= this.keyCodes.F12);
	},

	// Given a DOM keyboard event that happened while the Control key was depressed,
	// triggers the appropriate action or coolwsd message.
	_handleCtrlCommand: function (e) {

		var DEFAULT =0;
		var MAC=1;
		if (this._map.uiManager.isUIBlocked())
			return;

		// Control
		if (e.keyCode == this.keyCodes.CTRL)
			return true;

		if (e.type !== 'keydown' && e.keyCode !== this.keyCodes.C[DEFAULT] && e.keyCode !== this.keyCodes.V[DEFAULT] && e.keyCode !== this.keyCodes.X[DEFAULT] &&
		/* Safari */ e.keyCode !== this.keyCodes.C[MAC] && e.keyCode !== this.keyCodes.V[MAC] && e.keyCode !== this.keyCodes.X[MAC]) {
			e.preventDefault();
			return true;
		}

		// Control + INSERT
		if (this._isCtrlKey(e) && e.keyCode === this.keyCodes.INSERT) {
			return true;
		}

		if (e.keyCode !== this.keyCodes.C[DEFAULT] && e.keyCode !== this.keyCodes.V[DEFAULT] && e.keyCode !== this.keyCodes.X[DEFAULT] &&
		/* Safari */ e.keyCode !== this.keyCodes.C[MAC] && e.keyCode !== this.keyCodes.V[MAC] && e.keyCode !== this.keyCodes.X[MAC]) {
			// not copy or paste
			e.preventDefault();
		}

		// CTRL + ALT + N for new document. This needs to be handled by the integrator.
		if (this._isCtrlKey(e) && e.altKey && e.keyCode === this.keyCodes.N) {
			this._map.fire('postMessage', {msgId: 'UI_CreateFile', args: { DocumentType: this._map.getDocType() }});
			e.preventDefault();
			return true;
		}

		// CTRL + SHIFT + L is added to the core side for writer. Others can also be checked.
		if (this._isCtrlKey(e) && e.shiftKey && e.keyCode === this.keyCodes.L && this._map.getDocType() !== 'text' && this._map.getDocType() !== 'spreadsheet') {
			app.socket.sendMessage('uno .uno:DefaultBullet');
			e.preventDefault();
			return true;
		}

		// CTRL + ALT + O to open a document. This needs to be handled by the integrator.
		if (this._isCtrlKey(e) && e.altKey && e.keyCode === this.keyCodes.O) {
			this._map.fire('postMessage', { msgId: 'UI_OpenDocument' });
			e.preventDefault();
			return true;
		}

		if (this._isCtrlKey(e) && e.shiftKey && e.key === '?') {
			this._map.showHelp('keyboard-shortcuts-content');
			e.preventDefault();
			return true;
		}

		// Handles unformatted paste
		if (this._isCtrlKey(e) && e.shiftKey && this.keyCodes.V.includes(e.keyCode)) {
			return true;
		}

		// Collapse / expand notebookbar.
		if (this._isCtrlKey(e) && e.keyCode === this.keyCodes.F1) {
			if (this._map.uiManager.getCurrentMode() === 'notebookbar') {
				if (this._map.uiManager.isNotebookbarCollapsed())
					this._map.uiManager.extendNotebookbar();
				else
					this._map.uiManager.collapseNotebookbar();
			}
			return true;
		}

		if (this._isCtrlKey(e) && !e.shiftKey && e.keyCode === this.keyCodes.K) {
			this._map.sendUnoCommand('.uno:HyperlinkDialog');
			e.preventDefault();
			return true;
		}

		if (
			(this._isCtrlKey(e) && e.keyCode === this.keyCodes.Y) ||
			(this._isCtrlKey(e) && e.shiftKey && e.keyCode === this.keyCodes.Z)
		) {
			app.socket.sendMessage('uno .uno:Redo');
			e.preventDefault();
			return true;
		}

		if (this._isCtrlKey(e) && e.keyCode === this.keyCodes.Z) {
			app.socket.sendMessage('uno .uno:Undo');
			e.preventDefault();
			return true;
		}

		if (e.altKey || e.shiftKey) {

			// need to handle Ctrl + Alt + C separately for Firefox
			// Adding also CTRL + Alt + R combination for Safari users. R_MOD1_MOD2 combination is empty on the core side. So this should be safe.
			if ((this.keyCodes.C.includes(e.keyCode) || e.keyCode === this.keyCodes.R) && e.altKey) {
				this._map.insertComment();
				return true;
			}

			// Ctrl + Alt
			if (!e.shiftKey) {
				switch (e.keyCode) {
				case this.keyCodes.NUM5[DEFAULT]: // 5
					app.socket.sendMessage('uno .uno:Strikeout');
					return true;
				case this.keyCodes.F: // f but according to value it is F so I guess it is not MAC so safe to Give F
					app.socket.sendMessage('uno .uno:InsertFootnote');
					return true;
				case this.keyCodes.C[DEFAULT]: // c
				case this.keyCodes.M: // m
					this._map.insertComment();
					return true;
				case this.keyCodes.D: // d
					app.socket.sendMessage('uno .uno:InsertEndnote');
					return true;
				case this.keyCodes.pageUp:
				case this.keyCodes.pageDown :
					if (this._map.getDocType() === 'spreadsheet') {
						var currentSelectedPart = this._map._docLayer._selectedPart;
						var parts = this._map._docLayer._parts;
						var partToSelect = 0;
						this._map._docLayer._clearReferences();

						if (e.keyCode === this.keyCodes.pageDown) {
							partToSelect = currentSelectedPart != parts - 1 ? currentSelectedPart + 1 : 0;
						}
						else {
							partToSelect = currentSelectedPart != 0 ? currentSelectedPart - 1 : parts - 1;
						}
						this._map.setPart(parseInt(partToSelect), /*external:*/ false, /*calledFromSetPartHandler:*/ true);
						return;
					}

				}
			} else if (e.altKey) {
				switch (e.keyCode) {
					case this.keyCodes.D: // Ctrl + Shift + Alt + d for tile debugging mode
						this._map._debug.toggle();
				}
			}

			return false;
		}
		/* Without specifying the key type, the messages are sent twice (both keydown/up) */

		// Don't do this in CODA-W, there it is the sending of
		// the PASTE message in document,onpaste() in
		// Clipboard.js that does the paste.
		if (e.type === 'keydown' && window.ThisIsAMobileApp && !window.ThisIsTheWindowsApp && !window.ThisIsTheQtApp) {
			if (this.keyCodes.C.includes(e.keyCode)) {
				app.socket.sendMessage('uno .uno:Copy');
				return true;
			}
			else if (this.keyCodes.V.includes(e.keyCode)) {
				app.socket.sendMessage('uno .uno:Paste');
				return true;
			}
			else if (this.keyCodes.X.includes(e.keyCode)) {
				app.socket.sendMessage('uno .uno:Cut');
				return true;
			}
			if (window.ThisIsTheAndroidApp)
				e.preventDefault();
		}

		switch (e.keyCode) {
		case this.keyCodes.NUM3[DEFAULT]: // 3
			if (this._map.getDocType() === 'spreadsheet') {
				app.socket.sendMessage('uno .uno:SetOptimalColumnWidthDirect');
				app.socket.sendMessage('commandvalues command=.uno:ViewRowColumnHeaders');
				return true;
			}
			return false;
		case this.keyCodes.NUM5[DEFAULT]: // 5
			if (this._map.getDocType() === 'spreadsheet') {
				app.socket.sendMessage('uno .uno:Strikeout');
				return true;
			}
			return false;
		case this.keyCodes.C[DEFAULT]: // 'C'
		case this.keyCodes.X[DEFAULT]: // 'X'
		case this.keyCodes.C[MAC]: // 'c' Since keydown+c as different value in mac so i had to update the mapping in keyCodes
		case this.keyCodes.X[MAC]: // 'x' same reason as above
		case this.keyCodes.LEFTWINDOWKEY[MAC]: // Left Cmd (Safari)
		case this.keyCodes.RIGHTWINDOWKEY[MAC]: // Right Cmd (Safari)
			// we prepare for a copy or cut event
			// slide operations are handled differently avoid changing focus
			var slidePreviewFocused = this._map._docLayer._preview && this._map._docLayer._preview.partsFocused;
			if (!slidePreviewFocused)
				this._map.focus();
			// Not sure if the commented code is still used, so I didn't remove it.
			// Anyhow, by when editable area is populated with the focused paragraph
			// we can't select its content or on next editing the content is overwritten.
			// this._map._textInput.select();
			return true;
		case this.keyCodes.V[DEFAULT]: // v
		case this.keyCodes.V[MAC]: // v (Safari) needs a separate mapping in keyCodes
			return true;
		case this.keyCodes.F1: // f1
			app.socket.sendMessage('uno .uno:NoteVisible');
			return true;
		case this.keyCodes.COMMA: // ,
			app.socket.sendMessage('uno .uno:SubScript');
			return true;
		case this.keyCodes.PERIOD: // .
			app.socket.sendMessage('uno .uno:SuperScript');
			return true;
		}
		if (e.type === 'keypress' && (e.ctrlKey || e.metaKey) &&
			(this.keyCodes.C.includes(e.keyCode) ||this.keyCodes.V.includes(e.keyCode) || this.keyCodes.X.includes(e.keyCode))) {
			// need to handle this separately for Firefox
			return true;
		}
		return false;
	},
});
