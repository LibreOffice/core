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
 * window.L.Control.JSDialogBuilder used for building the native HTML components
 * from the JSON description provided by the server.
 */

/* global app $ _ JSDialog ColorPicker */

window.L.Control.JSDialogBuilder = window.L.Control.extend({

	options: {
		// window id
		windowId: null,
		// reference to map
		map: null,
		// reference to the parent container
		mobileWizard: null,
		// css class name added to the html nodes
		cssClass: 'mobile-wizard',
		// custom tabs placement handled by the parent container
		useSetTabs: false,

		// create only icon without label
		noLabelsForUnoButtons: false,

		// create labels next to the icon
		useInLineLabelsForUnoButtons: false
	},

	windowId: null,

	/* Handler is a function which takes three parameters:
	 * parentContainer - place where insert the content
	 * data - data of a control under process
	 * builder - current builder reference
	 *
	 * returns boolean: true if children should be processed
	 * and false otherwise
	 */
	_controlHandlers: null,
	_toolitemHandlers: null,
	_menuItemHandlers: null,
	_menus: null,
	_colorPickers: null,

	// Responses are included in a parent container. While buttons are created, responses need to be checked.
	// So we save the button ids and responses to check them later.
	_responses: {}, // Button id = response

	_currentDepth: 0,
	_expanderDepth: 0,

	rendersCache: {
		fontnamecombobox: { persistent: true, images: [] },
		layoutpanel_icons: { persistent: true, images: [] },
		transitions_icons: { persistent: true, images: [] },
		iconview_theme_colors: { persistent: true, images: [] },
		ctlFavoriteswin: { persistent: true, images: [] },
	}, // eg. custom renders for combobox entries

	setWindowId: function (id) {
		this.windowId = id;
	},

	_setup: function(options) {
		this._clearColorPickers();
		this.wizard = options.mobileWizard;
		this.map = options.map;
		this.windowId = options.windowId;
		this.dialogId = null;
		this.callback = options.callback ? options.callback : this._defaultCallbackHandler;

		this._colorPickers = [];

		// list of types which can have multiple children but are not considered as containers
		this._nonContainerType = ['buttonbox', 'treelistbox', 'iconview', 'iconviewlist', 'combobox', 'listbox',
			'scrollwindow', 'grid', 'tabcontrol', 'multilineedit', 'formulabaredit', 'frame', 'expander'];

		this._controlHandlers = {};
		this._controlHandlers['overflowgroup'] = JSDialog.OverflowGroup;
		this._controlHandlers['overflowmanager'] = JSDialog.OverflowManager;
		this._controlHandlers['radiobutton'] = JSDialog.RadioButton;
		this._controlHandlers['progressbar'] = JSDialog.progressbar;
		this._controlHandlers['pagemarginentry'] = JSDialog.PageMarginEntry;
		this._controlHandlers['newslidelayoutentry'] = JSDialog.slideLayoutEntry;
		this._controlHandlers['pagesizeentry'] = JSDialog.pageSizeEntry;
		this._controlHandlers['checkbox'] = JSDialog.Checkbox;
		this._controlHandlers['basespinfield'] = JSDialog.baseSpinField;
		this._controlHandlers['spinfield'] = JSDialog._spinfieldControl;
		this._controlHandlers['metricfield'] = JSDialog._metricfieldControl;
		this._controlHandlers['time'] = JSDialog.timeField;
		this._controlHandlers['formattedfield'] = JSDialog._formattedfieldControl;
		this._controlHandlers['edit'] = JSDialog.edit;
		this._controlHandlers['searchedit'] = JSDialog.searchEdit;
		this._controlHandlers['formulabaredit'] = JSDialog.formulabarEdit;
		this._controlHandlers['multilineedit'] = JSDialog.multilineEdit;
		this._controlHandlers['pushbutton'] = JSDialog.pushButton;
		this._controlHandlers['okbutton'] = JSDialog.pushButton;
		this._controlHandlers['helpbutton'] = JSDialog.pushButton;
		this._controlHandlers['cancelbutton'] = JSDialog.pushButton;
		this._controlHandlers['combobox'] = JSDialog.combobox;
		this._controlHandlers['comboboxentry'] = JSDialog.comboboxEntry;
		this._controlHandlers['listbox'] = JSDialog.listbox;
		this._controlHandlers['valueset'] = this._valuesetControl;
		this._controlHandlers['fixedtext'] = this._fixedtextControl;
		this._controlHandlers['linkbutton'] = this._linkButtonControl;
		this._controlHandlers['htmlcontrol'] = this._htmlControl;
		this._controlHandlers['expander'] = this._expanderHandler;
		this._controlHandlers['grid'] = JSDialog.grid;
		this._controlHandlers['alignment'] = this._alignmentHandler;
		this._controlHandlers['buttonbox'] = JSDialog.buttonBox;
		this._controlHandlers['frame'] = JSDialog.frame;
		this._controlHandlers['deck'] = JSDialog.deck;
		this._controlHandlers['panel'] = JSDialog.panel;
		this._controlHandlers['calcfuncpanel'] = this._calcFuncListPanelHandler;
		this._controlHandlers['tabcontrol'] = this._tabsControlHandler;
		this._controlHandlers['tabpage'] = this._tabPageHandler;
		this._controlHandlers['singlepanel'] = this._singlePanelHandler;
		this._controlHandlers['container'] = JSDialog.container;
		this._controlHandlers['dialog'] = JSDialog.container;
		this._controlHandlers['messagebox'] = JSDialog.container;
		this._controlHandlers['window'] = JSDialog.container;
		this._controlHandlers['borderwindow'] = this._borderwindowHandler;
		this._controlHandlers['control'] = JSDialog.container;
		this._controlHandlers['scrollbar'] = this._ignoreHandler;
		this._controlHandlers['toolbox'] = JSDialog.toolbox;
		this._controlHandlers['spacer'] = JSDialog.spacer;
		this._controlHandlers['toolitem'] = this._toolitemHandler;
		this._controlHandlers['colorsample'] = this._colorSampleControl;
		this._controlHandlers['divcontainer'] = this._divContainerHandler;
		this._controlHandlers['colorlistbox'] = JSDialog.colorPickerButton;
		this._controlHandlers['treelistbox'] = JSDialog.treeView;
		this._controlHandlers['iconview'] = JSDialog.iconView;
		this._controlHandlers['iconviewlist'] = JSDialog.notebookbarIconViewList;
		this._controlHandlers['drawingarea'] = JSDialog.drawingArea;
		this._controlHandlers['rootcomment'] = JSDialog.rootCommentControl;
		this._controlHandlers['comment'] = JSDialog.commentControl;
		this._controlHandlers['emptyCommentWizard'] = JSDialog.rootCommentControl;
		this._controlHandlers['separator'] = this._separatorControl;
		this._controlHandlers['menubutton'] = JSDialog.menubuttonControl;
		this._controlHandlers['spinner'] = this._spinnerControl;
		this._controlHandlers['spinnerimg'] = this._spinnerImgControl;
		this._controlHandlers['image'] = this._imageHandler;
		this._controlHandlers['scrollwindow'] = JSDialog.scrolledWindow;
		this._controlHandlers['customtoolitem'] = JSDialog.dispatchToolitem;
		this._controlHandlers['bigcustomtoolitem'] = JSDialog.bigDispatchToolitem;
		this._controlHandlers['calendar'] = JSDialog.calendar;
		this._controlHandlers['htmlcontent'] = JSDialog.htmlContent;
		this._controlHandlers['colorpicker'] = JSDialog.colorPicker;
		this._controlHandlers['dropdown'] = JSDialog.Dropdown;
		this._controlHandlers['menu'] = JSDialog.treeView;

		this._controlHandlers['mainmenu'] = JSDialog.container;
		this._controlHandlers['submenu'] = this._subMenuHandler;
		this._controlHandlers['menuitem'] = this._menuItemHandler;

		this._menuItemHandlers = {};
		this._menuItemHandlers['inserttable'] = this._insertTableMenuItem;

		this._toolitemHandlers = {};
		this._toolitemHandlers['.uno:XLineColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FontColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:CharBackColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:BackgroundColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:TableCellBackgroundColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FrameLineColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:Color'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FillColor'] = JSDialog.colorPickerButton;

		this._toolitemHandlers['.uno:InsertFormula'] = function () {};

		this._menus = JSDialog.MenuDefinitions;

		this._currentDepth = 0;
		this._expanderDepth = 0;

		app.localeService.initializeNumberFormatting();
	},

	reportValidity: function() {
		var isValid = true;
		if (!this._container)
			return isValid;

		var inputs = this._container.querySelectorAll('.spinfield');
		for (var item = 0; item < inputs.length; item++) {
			if (!inputs[item].checkVisibility())
				continue;

			var value = JSDialog._parseSpinFieldValue(inputs[item].value);
			if (value !== '' && isNaN(parseFloat(value))) {
				isValid = false;
				inputs[item].focus();
				break;
			}
		}

		return isValid;
	},

	isContainerType: function(type) {
		return this._nonContainerType.indexOf(type) < 0;
	},

	setContainer: function(container) {
		this._container = container;
	},

	_clearColorPickers: function() {
		this._colorPickers = [];
		ColorPicker.ID = 0;
	},

	_preventDocumentLosingFocusOnClick: function(div) {
		$(div).on('mousedown',function (e) {
			e.preventDefault();
			e.stopPropagation();
		});
	},

	_toolitemHandler: function(parentContainer, data, builder) {
		if (data.command || data.postmessage) {
			var handler = builder._toolitemHandlers[data.command];
			if (handler)
				handler(parentContainer, data, builder);
			else if (data.text || data.command || data.postmessage) {
				builder._unoToolButton(parentContainer, data, builder);
			} else
				window.app.console.warn('Unsupported toolitem type: "' + data.command + '"');
		}

		builder.postProcess(parentContainer, data);

		return false;
	},

	_defaultCallbackHandlerSendMessage: function(objectType, eventType, object, data, builder) {
		switch (typeof data) {
		case 'string':
			// escape backspaces, quotes, newlines, and so on; remove added quotes
			data = JSON.stringify(data).slice(1, -1);
			break;
		case 'object':
			data = encodeURIComponent(JSON.stringify(data));
			break;
		}
		var windowId = builder && builder.windowId !== null && builder.windowId !== undefined ? builder.windowId :
			(window.mobileDialogId !== undefined ? window.mobileDialogId :
				(window.sidebarId !== undefined ? window.sidebarId : -1));

		if (typeof windowId !== 'number') {
			window.app.console.error('JSDialog: windowId "' + windowId + '" is not valid. Use a number.');
			return; // core will fail parsing the command, it is a mistake most probably
		}

		var message = 'dialogevent ' + windowId
				+ ' {\"id\":\"' + object.id
			+ '\", \"cmd\": \"' + eventType
			+ '\", \"data\": \"' + data
			+ '\", \"type\": \"' + objectType + '\"}';
		app.socket.sendMessage(message);
		window._firstDialogHandled = true;
	},

	// by default send new state to the core
	_defaultCallbackHandler: function(objectType, eventType, object, data, builder) {
		if (builder.map.uiManager.isUIBlocked())
			return;

		if (objectType === 'responsebutton' && data === 1 && !builder.reportValidity())
			return;

		console.assert(
			typeof object.id === 'number' || (typeof object.id === 'string' && object.id.length > 0),
			'Trying to send command without valid id');

		if (JSDialog.verbose) {
			window.app.console.debug('control: \'' + objectType + '\' id:\'' + object.id + '\' event: \'' + eventType + '\' state: \'' + data + '\'');
		}

		// if user does action - enter following own cursor mode
		var viewId = builder.map && builder.map._docLayer ? builder.map._docLayer._getViewId() : -1;
		if (viewId >= 0)
			app.setFollowingUser(viewId);

		if (builder.wizard.setCurrentScrollPosition)
			builder.wizard.setCurrentScrollPosition();

		if (objectType == 'toolbutton' && eventType == 'click' && data.indexOf('.uno:') >= 0) {
			// encode spaces
			var encodedCommand = data.replace(' ', '%20');
			builder.map.sendUnoCommand(encodedCommand);
		} else if (object) {
			// CSV and Macro Security Warning Dialogs are shown before the document load
			// In that state the document is not really loaded and closing or cancelling it
			// returns docnotloaded error. Instead of this we can return to the integration
			if (!builder.map._docLoaded &&
				 !window._firstDialogHandled &&
				 (eventType === 'close' ||
				 (objectType === 'responsebutton' && data == 7))) {
				let dispatcher = app.dispatcher;
				if (!dispatcher)
					dispatcher = new app.definitions['dispatcher']('global');

				dispatcher.dispatch('closeapp');
			}
			builder._defaultCallbackHandlerSendMessage(objectType, eventType, object, data, builder);
		}
	},

	_setupHandlers: function (controlElement, handlers) {
		if (handlers) {
			for (var i = 0; i < handlers.length; ++i) {
				var event = handlers[i].event;
				var handler = handlers[i].handler;
				if (!app.util.isEmpty(event) && handler) {
					if (event === 'click') {
						var eventData = {
							id: controlElement.id
						};
						$(controlElement).click(
							// avoid to access mutable variable (that is `i` dependent) in closure
							(function (lhandler, leventData) {
								return function() { lhandler(leventData); };
							})(handler, eventData)
						);
					}
				}
			}
		}
	},

	_stressAccessKey: function(element, accessKey) {
		if (!accessKey || window.mode.isSmallScreenDevice() || window.getAccessibilityState())
			return;

		var text = element.textContent;
		var index = text.indexOf(accessKey);
			if (index >= 0) {
					var title = text.replace(accessKey, '<u class="access-key">' + accessKey.replace('~', '') + '</u>');
					element.innerHTML = title;
		}
	},

	_setAccessKey: function(element, key) {
		if (key)
			element.accessKey = key;
	},

	_getAccessKeyFromText: function(text) {
		var nextChar = null;
		if (text && text.includes('~')) {
			var index = text.indexOf('~');
			if (index < text.length - 1) {
				nextChar = text.charAt(index + 1);
			}
		}
		return nextChar;
	},

	_cleanText: function(text) {
		if (!text)
			return '';
		if (text.endsWith('...'))
			text = text.slice(0, -3);
		if (text.endsWith('…'))
			text = text.slice(0, -1);
		return text.replace('~', '');
	},

	_gradientStyleToLabel: function(state) {
		switch (state) {
		case 'NONE':
			return _('None');

		case 'SOLID':
			return _('Solid');

		case 'LINEAR':
			return _('Linear');

		case 'AXIAL':
			return _('Axial');

		case 'RADIAL':
			return _('Radial');

		case 'ELLIPTICAL':
			return _('Ellipsoid');

		// no, not a typo (square - quadratic, rect - square) - same as in the core
		case 'SQUARE':
			return _('Quadratic');

		case 'RECT':
			return _('Square');

		case 'MAKE_FIXED_SIZE':
			return _('Fixed size');
		}

		return '';
	},

	_borderwindowHandler: function(parentContainer, data, builder) {
		if (data.visible === false) {
			for (var i in data.children)
				data.children[i].visible = false;
		}

		return JSDialog.container(parentContainer, data, builder);
	},

	_handleResponses: function(data, builder) {
		// Dialogue is a parent container of a buttonbox, so we will save the responses first, then we will check them while creating the buttons.
		if (data.responses) {
			for (var i in data.responses) {
				// Button id = response
				builder._responses[data.responses[i].id] = data.responses[i].response;
			}
		}
	},

	// used inside tab control and assistant (chart wizard, where it should create own container)
	_tabPageHandler: function(parentContainer, data, builder) {
		var page = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-tabpage', parentContainer);
		page.id = data.id;

		builder.build(page, data.children, false);

		return false;
	},

	_alignmentHandler: function(parentContainer, data, builder) {
		window.L.DomUtil.addClass(parentContainer, 'ui-alignment');
		return JSDialog.container(parentContainer, data, builder);
	},

	_ignoreHandler: function() {
		return false;
	},

	_getGridColumns: function(children) {
		var columns = 0;
		for (var index in children) {
			if (parseInt(children[index].left) > columns)
				columns = parseInt(children[index].left);
		}
		return columns + 1;
	},

	_getGridRows: function(children) {
		var rows = 0;
		for (var index in children) {
			if (parseInt(children[index].top) > rows)
				rows = parseInt(children[index].top);
		}
		return rows + 1;
	},

	_explorableEntry: function(parentContainer, data, content, builder, valueNode, iconURL, updateCallback) {
		var mainContainer = window.L.DomUtil.create('div', 'ui-explorable-entry level-' + builder._currentDepth + ' ' + builder.options.cssClass, parentContainer);
		if (data) {
			if (data.name)
				mainContainer.id = data.name; // use legacy panel id FIXME: convert all CSS and cypress to vcl id
			else if (data.id)
				mainContainer.id = data.id;
		}

		var sectionTitle = window.L.DomUtil.create('div', 'ui-header level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', mainContainer);
		$(sectionTitle).css('justify-content', 'space-between');

		if (data.enabled === 'false' || data.enabled === false) {
			mainContainer.disabled = true;
			$(mainContainer).addClass('disabled');
		}

		var leftDiv = window.L.DomUtil.create('div', 'ui-header-left', sectionTitle);
		var titleClass = '';

		switch (sectionTitle.id)
		{
		case 'paperformat':
		case 'orientation':
		case 'masterslide':
		case 'SdTableDesignPanel':
		case 'ChartTypePanel':
		case 'rotation':
			iconURL = 'lc_'+ sectionTitle.id.toLowerCase() +'.svg';
			break;
		}
		if (iconURL) {
			var icon = window.L.DomUtil.create('img', 'menu-entry-icon', leftDiv);
			app.LOUtil.setImage(icon, iconURL, builder.map);
			icon.alt = '';
			titleClass = 'menu-entry-with-icon';

		}
		var titleSpan = window.L.DomUtil.create('span', titleClass, leftDiv);

		if (!valueNode && data.command) {
			var items = builder.map['stateChangeHandler'];
			var val = items.getItemValue(data.command);
			if (val)
				valueNode = window.L.DomUtil.create('div', '', null);
		}

		var rightDiv = window.L.DomUtil.create('div', 'ui-header-right', sectionTitle);
		if (valueNode) {
			var valueDiv = window.L.DomUtil.create('div', 'entry-value', rightDiv);
			valueDiv.appendChild(valueNode);
		}

		var arrowSpan = window.L.DomUtil.create('span', 'sub-menu-arrow', rightDiv);
		arrowSpan.textContent = '>';

		var updateFunction = function(titleSpan) {
			titleSpan.innerHTML = data.text;
		};

		updateCallback ? updateCallback(titleSpan) : updateFunction(titleSpan);

		var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, mainContainer);
		contentDiv.title = data.text;

		var contentData = content.length ? content : [content];
		var contentNode = contentData.length === 1 ? contentData[0] : null;

		builder._currentDepth++;
		builder.build(contentDiv, contentData);
		builder._currentDepth--;

		if (!data.nosubmenu)
		{
			$(contentDiv).hide();
			if (builder.wizard) {
				$(sectionTitle).click(function(event, data) {
					if (!mainContainer.hasAttribute('disabled')) {
						builder.wizard.goLevelDown(mainContainer, data);
						if (contentNode && contentNode.onshow && !builder.wizard._inBuilding)
							contentNode.onshow();
					}
				});

				if (mainContainer.hasAttribute('disabled')) {
					$(arrowSpan).hide();
				}
			} else {
				window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
			}
		}
		else
			$(sectionTitle).hide();
	},

	_calcFunctionEntry: function(parentContainer, data, contentNode, builder) {
		var mainContainer = window.L.DomUtil.create('div', 'ui-explorable-entry level-' + builder._currentDepth + ' ' + builder.options.cssClass, parentContainer);
		var sectionTitle = window.L.DomUtil.create('div', 'func-entry ui-header level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', mainContainer);
		$(sectionTitle).css('justify-content', 'space-between');
		if (data && data.id)
			sectionTitle.id = data.id;

		var leftDiv = window.L.DomUtil.create('div', 'ui-header-left', sectionTitle);
		var titleClass = 'func-name';
		var titleSpan = window.L.DomUtil.create('span', titleClass, leftDiv);
		titleSpan.textContent = data.text;

		var rightDiv = window.L.DomUtil.create('div', 'ui-header-right', sectionTitle);
		var arrowSpan = window.L.DomUtil.create('div', 'func-info-icon', rightDiv);
		arrowSpan.textContent = '';

		var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, mainContainer);
		contentDiv.title = data.text;

		builder._currentDepth++;
		builder.build(contentDiv, [contentNode]);
		builder._currentDepth--;

		$(contentDiv).hide();
		if (builder.wizard) {
			var that = this;
			var functionName = data.functionName;
			$(rightDiv).click(e => {
				e.stopPropagation();
				builder.wizard.goLevelDown(mainContainer);
				if (contentNode.onshow)
					contentNode.onshow();
			});
			$(sectionTitle).click(() => {
				if (functionName !== '') {
					app.socket.sendMessage('completefunction name=' + functionName);
					that.map.fire('closemobilewizard');
				}
			});
		} else {
			window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
		}
	},

	_expanderHandler: function(parentContainer, data, builder, customCallback) {
		if (data.children.length > 0) {
			var container = window.L.DomUtil.create('div', 'ui-expander-container ' + builder.options.cssClass, parentContainer);
			container.id = data.id;
			container.dataset.expanderDepth = builder._expanderDepth;

			var expanded = data.expanded === true || (data.children[0] && data.children[0].checked === true);
			var expander = window.L.DomUtil.create('div', 'ui-expander ' + builder.options.cssClass, container);
			if (data.children[0].text && data.children[0].text !== '') {
				var prefix = data.children[0].id ? data.children[0].id : data.id;

				// W3C accordion pattern: https://www.w3.org/WAI/ARIA/apg/patterns/accordion/examples/accordion
				// For an initial expander (_expanderDepth of 0) we use h2 instead of h1
				// to leave room for a conceptual parent heading
				var headingLevel = Math.min(builder._expanderDepth + 2, 6);
				var heading = window.L.DomUtil.create('h' + headingLevel, 'ui-expander-heading ' + builder.options.cssClass, expander);
				var expanderBtn = window.L.DomUtil.create('button', 'ui-expander-btn ' + builder.options.cssClass, heading);
				expanderBtn.id = prefix + '-button';
				expanderBtn.tabIndex = '0';
				expanderBtn.setAttribute('aria-controls', prefix + '-children');

				var label = window.L.DomUtil.create('span', 'ui-expander-label ' + builder.options.cssClass, expanderBtn);
				label.innerText = builder._cleanText(data.children[0].text);
				label.id = prefix + '-label';
				if (data.children[0].visible === false)
					window.L.DomUtil.addClass(label, 'hidden');
				builder.postProcess(expanderBtn, data.children[0]);

				var state = data.children.length > 1 && expanded;
				if (state) {
					window.L.DomUtil.addClass(label, 'expanded');
				}
				expanderBtn.setAttribute('aria-expanded', state);

				var toggleFunction = function () {
					if (customCallback)
						customCallback();
					else
						builder.callback('expander', 'toggle', data, null, builder);

					$(label).toggleClass('expanded');
					$(expander).siblings().toggleClass('expanded');

					// Toggle aria-expanded attribute
					const currentState = expanderBtn.getAttribute('aria-expanded') === 'true';
					expanderBtn.setAttribute('aria-expanded', (!currentState).toString());
				};

				$(expanderBtn).click(toggleFunction);
				$(expanderBtn).keypress(function (event) {
					if (event.which === 13) {
						toggleFunction();
						event.preventDefault();
					}
				});
			}

			var expanderChildren = window.L.DomUtil.create('div', 'ui-expander-content ' + builder.options.cssClass, container);
			expanderChildren.id = prefix + '-children';
			expanderChildren.setAttribute('role', 'region');
			expanderChildren.setAttribute('aria-labelledby', prefix + '-label');

			if (expanded) {
				if (data.children.length > 1) {
					label.classList.add('expanded');
					expanderBtn.setAttribute('aria-expanded', 'true');
				}
				expanderChildren.classList.add('expanded');
			}
			else {
				expanderBtn.setAttribute('aria-expanded', 'false');
			}

			var children = [];
			var startPos = 1;

			if (data.children[0].type === 'grid' ||
				data.children[0].type === 'container') {
				startPos = 0;
			}

			for (var i = startPos; i < data.children.length; i++) {
				if (data.children[i].visible === false)
					data.children[i].visible = true;
				children.push(data.children[i]);
			}

			builder._expanderDepth++;
			builder.build(expanderChildren, children);
			builder._expanderDepth--;
		} else {
			return true;
		}

		return false;
	},

	_generateMenuIconName: function(commandName) {
		// command has no parameter
		if (commandName.indexOf('?') === -1) {
			if (commandName.indexOf('InsertDateContentControl') !== -1)
				return 'insertdatefield';
			return commandName.toLowerCase();
		}

		if (commandName.indexOf('SpellCheckIgnoreAll') !== -1)
			return 'spellcheckignoreall';
		if (commandName.indexOf('SpellCheckIgnore') !== -1)
			return 'spellcheckignore';
		if (commandName === 'LanguageStatus?Language:string=Current_LANGUAGE_NONE')
			return 'selectionlanugagenone';
		if (commandName === 'LanguageStatus?Language:string=Current_RESET_LANGUAGES')
			return 'selectionlanugagedefault';
		if (commandName === 'LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE')
			return 'paragraphlanugagenone';
		if (commandName === 'LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES')
			return 'paragraphlanugagedefault';
		if ((this.map.getDocType() === 'spreadsheet' || this.map.getDocType() === 'presentation') &&
						commandName.indexOf('LanguageStatus?Language:string=Paragraph_') !== -1)
			return 'paragraphlanugagesuggestion';
		if ((this.map.getDocType() === 'spreadsheet' || this.map.getDocType() === 'presentation') &&
						commandName.indexOf('LanguageStatus?Language:string=Current_') !== -1)
			return 'selectionlanugagesuggestion';
		return commandName.toLowerCase();
	},

	_explorableMenu: function(parentContainer, title, children, builder, customContent, dataid) {
		dataid = dataid || 0;
		var icon = null;
		var mainContainer = window.L.DomUtil.create('div', 'ui-explorable-entry level-' + builder._currentDepth + ' ' + builder.options.cssClass, parentContainer);
		var sectionTitle = window.L.DomUtil.create('div', 'ui-header level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', mainContainer);
		$(sectionTitle).css('justify-content', 'space-between');

		var commandName = dataid;
		if (commandName && commandName.length && app.LOUtil.existsIconForCommand(commandName, builder.map.getDocType())) {
			var iconName = builder._generateMenuIconName(commandName);
			var iconSpan = window.L.DomUtil.create('span', 'menu-entry-icon ' + iconName, sectionTitle);
			iconName = app.LOUtil.getIconNameOfCommand(iconName, true);
			icon = window.L.DomUtil.create('img', '', iconSpan);
			app.LOUtil.setImage(icon, iconName, builder.map);
			icon.alt = '';
			var titleSpan2 = window.L.DomUtil.create('span', 'menu-entry-with-icon flex-fullwidth', sectionTitle);
			titleSpan2.innerHTML = title;
		}
		else {
			var titleSpan = window.L.DomUtil.create('span', 'sub-menu-title', sectionTitle);
			titleSpan.innerHTML = title;
		}
		var arrowSpan = window.L.DomUtil.create('span', 'sub-menu-arrow', sectionTitle);
		arrowSpan.textContent = '>';

		var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, mainContainer);
		contentDiv.title = title;

		if (customContent) {
			contentDiv.appendChild(customContent);
		} else {
			builder._currentDepth++;
			for (var i = 0; i < children.length; i++) {
				builder.build(contentDiv, [children[i]]);
			}
			builder._currentDepth--;
		}

		$(contentDiv).hide();
		if (builder.wizard) {
			$(sectionTitle).click(() => { builder.wizard.goLevelDown(mainContainer); });
		} else {
			window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
		}
	},

	_calcFuncListPanelHandler: function(parentContainer, data, builder) {
		var contentNode = data.children[0];

		builder._calcFunctionEntry(parentContainer, data, contentNode, builder);

		return false;
	},

	_createTabClick: function(builder, t, tabs, contentDivs, tabIds)
	{
		return function() {
			$(tabs[t]).addClass('selected');
			tabs[t].tabIndex = '0';
			tabs[t].setAttribute('aria-selected', 'true');

			for (var i = 0; i < tabs.length; i++) {
				if (i !== t)
				{
					$(tabs[i]).removeClass('selected');
					$(contentDivs[i]).addClass('hidden');
					tabs[i].setAttribute('aria-selected', 'false');
					tabs[i].tabIndex = -1;
				}
			}
			$(contentDivs[t]).removeClass('hidden');
			builder.wizard.selectedTab(tabIds[t]);
		};
	},

	_tabsControlHandler: function(parentContainer, data, builder, tabTooltip) {
		if (tabTooltip === undefined) {
			tabTooltip = '';
		}

		var contentDivs = [];
		var isMultiTabJSON = false;
		var singleTabId = null;

		if (data.tabs) {
			var tabs = 0;
			for (var tabIdx = 0; data.children && tabIdx < data.children.length; tabIdx++) {
				if (data.children[tabIdx].type === 'tabpage' || data.vertical)
					tabs++;
			}
			isMultiTabJSON = tabs > 1;

			var tabWidgetRootContainer = window.L.DomUtil.create('div', 'ui-tabs-root ' + builder.options.cssClass, parentContainer);
			tabWidgetRootContainer.id = data.id;

			var tabsContainer = window.L.DomUtil.create('div', 'ui-tabs ' + builder.options.cssClass + ' ui-widget', builder.options.useSetTabs ? undefined : tabWidgetRootContainer);
			tabsContainer.setAttribute('role', 'tablist');

			var contentsContainer = window.L.DomUtil.create('div', 'ui-tabs-content ' + builder.options.cssClass, tabWidgetRootContainer);

			var tabs = [];
			var tabIds = [];
			for (var tabIdx = 0; tabIdx < data.tabs.length; tabIdx++) {
				var item = data.tabs[tabIdx];

				var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, contentsContainer);
				contentDiv.id = item.name;
				contentDiv.setAttribute('role', 'tabpanel');

				var title = builder._cleanText(item.text);

				var tab = window.L.DomUtil.create('button', 'ui-tab ' + builder.options.cssClass, tabsContainer);
				// avoid duplicated ids: we receive plain number from core, append prefix
				tab.id = Number.isInteger(parseInt(item.id)) ? data.id + '-' + item.id : item.id;

				contentDiv.setAttribute('aria-labelledby', tab.id);

				tab.textContent = title;
				tab.setAttribute('role', 'tab');
				tab.setAttribute('aria-controls', contentDiv.id);
				JSDialog.AddAriaLabel(tab, item, builder);
				builder._setAccessKey(tab, builder._getAccessKeyFromText(item.text));
				builder._stressAccessKey(tab, tab.accessKey);

				var isSelectedTab = data.selected == item.id;
				if (isSelectedTab) {
					$(tab).addClass('selected');
					tab.setAttribute('aria-selected', 'true');
					tab.tabIndex = '0';
					tab.setAttribute('data-cooltip', tabTooltip);
					singleTabId = tabIdx;
				} else {
					tab.setAttribute('aria-selected', 'false');
					tab.tabIndex = -1;
				}

				var tabContext = item.context;
				if (tabContext) {
					var tabHasCurrentContext = builder.map.context.context !== ''
											&& tabContext.indexOf(builder.map.context.context) !== -1;
					var tabHasDefultContext = tabContext.indexOf('default') !== -1;

					if (!tabHasCurrentContext && !tabHasDefultContext) {
						$(tab).addClass('hidden');
					}
				}

				tabs[tabIdx] = tab;
				tabIds[tabIdx] = item.name;

				if (!isSelectedTab)
					$(contentDiv).addClass('hidden');
				contentDivs[tabIdx] = contentDiv;
			}

			if (builder.wizard) {
				if (builder.options.useSetTabs)
					builder.wizard.setTabs(tabsContainer, builder);

				tabs.forEach(function (tab, index) {
					var eventHandler = builder._createTabClick(builder, index, tabs, contentDivs, tabIds);
					tab.addEventListener('click', function(event) {
						eventHandler(event);
						if (!data.noCoreEvents) {
							builder.callback('tabcontrol', 'selecttab', tabWidgetRootContainer, index, builder);
						}
					});
				});

				// Initialize keyboard navigation for tabs
				JSDialog.KeyboardTabNavigation(tabs, contentDivs);
			} else {
				window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
			}
		}

		if (isMultiTabJSON) {
			var tabId = 0;
			for (var tabIdx = 0; tabIdx < data.children.length; tabIdx++) {
				var tab = data.children[tabIdx];

				if (tab.type !== 'tabpage' && !data.vertical)
					continue;

				builder.build(contentDivs[tabId], [tab], false, false);
				tabId++;
			}
		} else if (singleTabId != null) {
			for (var tabIdx = 0; tabIdx < data.children.length; tabIdx++) {
				var tab = data.children[tabIdx];

				if (tab.type !== 'tabpage' && !data.vertical)
					continue;

				builder.build(contentDivs[singleTabId], [tab], false, false);
				break;
			}
		}

		if (data.tabs && data.isNotebookbar) {
			let that = this;
			contentDivs.forEach(function(tabPage)
			{
				tabPage.addEventListener('keydown', function(e) {
					// Determine key direction
					let key;
					let isTab = e.key === 'Tab';
					if (isTab) {
						key = e.shiftKey ? 'ArrowLeft' : 'ArrowRight'; // Reverse if Shift+Tab
					} else {
						key = e.key;
					}
					if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(key)) {
						var currentElement = e.srcElement;
						if (!(currentElement.tagName === 'INPUT' || currentElement.tagName === 'TEXTAREA') || isTab) {
							if (isTab)
								e.preventDefault();
							let container = document.getElementsByClassName('ui-tabs-content notebookbar');
							let elementToFocus;
							// Use DOM order for left/right (ray-casting misses small
							// vertically-stacked buttons) and also for up/down when
							// inside an iconview widget.
							const useDomOrder = key === 'ArrowLeft' || key === 'ArrowRight'
								|| currentElement.closest('.ui-iconview-root');
							if (useDomOrder) {
								var allFocusables = Array.from(container[0].querySelectorAll('*'))
									.filter(function(el) { return el.checkVisibility() && JSDialog.IsFocusable(el); });
								// Only leaf-level focusable elements are navigation targets.
								var focusables = allFocusables.filter(function(el) {
									return !allFocusables.some(function(other) { return other !== el && el.contains(other); });
								});
								var idx = focusables.indexOf(currentElement);
								if (idx !== -1) {
									var forward = key === 'ArrowRight' || key === 'ArrowDown';
									// When entering an iconview from outside, skip to the selected
									// entry (per WAI-ARIA toolbar radio group pattern). When already
									// inside the iconview, arrows move between entries normally.
									var inIconview = !isTab && currentElement.classList.contains('ui-iconview-entry');
									var shouldSkip = function(el) {
										if (inIconview)
											return false;
										return el.classList.contains('ui-iconview-entry') && !el.classList.contains('selected');
									};
									var advance = function(i) {
										i = forward ? i + 1 : i - 1;
										if (i >= focusables.length) i = 0;
										else if (i < 0) i = focusables.length - 1;
										return i;
									};
									var next = idx;
									do {
										next = advance(next);
									} while (next !== idx && shouldSkip(focusables[next]));
									// If skip logic found nothing (e.g. iconview with no
									// selected entry), just move to the next element.
									if (next === idx)
										next = advance(next);
									if (next !== idx)
										elementToFocus = focusables[next];
								}
							}
							if (!elementToFocus)
								elementToFocus = JSDialog.FindNextElementInContainer(container[0], currentElement, key);
							if (elementToFocus && elementToFocus.tagName !== 'NAV')
								elementToFocus.focus();
							else if (elementToFocus)
								document.querySelector('.ui-tab.notebookbar.selected').focus();
							else {
								// Nothing found — cycle to first focusable
								let visibleContainer = Array.from(container[0].children).find(child =>
									!child.classList.contains('hidden') && child.offsetParent !== null
								);
								
								var allFocusables = visibleContainer ? Array.from(visibleContainer.querySelectorAll('*'))
									.filter(function(el) { return el.checkVisibility() && JSDialog.IsFocusable(el); }) : [];

								// Only leaf-level focusable elements are candidates.
								var focusables = allFocusables.filter(function(el) {
									return !allFocusables.some(function(other) { return other !== el && el.contains(other); });
								});

								if (focusables.length) {
									let first = focusables[0];
									let last = focusables[focusables.length - 1];
									if (e.shiftKey && currentElement === first)
										last.focus();
									else if (!e.shiftKey && currentElement === last)
										first.focus();
									else
										(e.shiftKey ? last : first).focus(); // fallback
								}
							}
						}
					}
				}.bind(that));
			});
		}

		return false;
	},

	_singlePanelHandler: function(parentContainer, data, builder) {
		var item = data[0];
		if (item.children) {
			var child = item.children[0];
			builder.build(parentContainer, [child]);
		}
		return false;
	},

	_unitToVisibleString: function(unit) {
		if (unit == 'inch') {
			return '"';
		} else if (unit == 'percent') {
			return '%';
		} else if (unit == 'degree') {
			return '°';
		}
		return unit;
	},

	_getUnoStateForItemId: function(id, builder) {
		var items = builder.map['stateChangeHandler'];
		var state = null;

		switch (id) {
		case 'fillattr':
			state = items.getItemValue('.uno:FillPageColor');
			if (state) {
				return state;
			}
			break;

		case 'fillattr2':
			state = items.getItemValue('.uno:FillPageGradient');
			if (state) {
				return state.startcolor;
			}
			break;

		case 'fillattr3':
			state = items.getItemValue('.uno:FillPageGradient');
			if (state) {
				return state.endcolor;
			}
			break;

		case 'gradientstyle':
			state = items.getItemValue('.uno:FillGradient');
			if (state) {
				return builder._gradientStyleToLabel(state.style);
			}
			break;

		case 'gradangle':
			state = items.getItemValue('.uno:FillGradient');
			if (state) {
				return state.angle;
			}
			break;

		case 'fillgrad1':
			state = items.getItemValue('.uno:FillGradient');
			if (state) {
				return state.startcolor;
			}
			break;

		case 'fillgrad2':
			state = items.getItemValue('.uno:FillGradient');
			if (state) {
				return state.endcolor;
			}
			break;

		case 'LB_SHADOW_COLOR':
			state = items.getItemValue('.uno:FillShadowColor');
			if (state) {
				return state;
			}
			break;
		}

		return null;
	},

	_getTitleForControlWithId: function(id) {
		switch (id) {

		case 'fillgrad1':
			return _('From');

		case 'fillgrad2':
			return _('To');

		case 'LB_SHADOW_COLOR':
			return _('Color');
		}

		return null;
	},

	_linkButtonControl: function(parentContainer, data, builder) {
		var buttonLink = window.L.DomUtil.create('button', builder.options.cssClass + " ui-linkbutton", parentContainer);

		if (data.text)
			buttonLink.textContent = builder._cleanText(data.text);
		else if (data.html)
			buttonLink.innerHTML = data.html;

		var accKey = builder._getAccessKeyFromText(data.text);
		builder._stressAccessKey(buttonLink, accKey);

		buttonLink.id = data.id;
		if (data.style && data.style.length) {
			window.L.DomUtil.addClass(buttonLink, data.style);
		} else {
			window.L.DomUtil.addClass(buttonLink, 'ui-text');
		}
		if (data.hidden)
			$(buttonLink).hide();

		var clickFunction = function () {
				builder.callback('linkbutton', 'click', data, null, builder);
		};
		$(buttonLink).click(clickFunction);
		return false;
	},

	_setIconAndNameForCombobox: function(data) {
		if (data.command == '.uno:CharFontName') {
			data.text = _('Font Name');
		} else if (data.command == '.uno:FontHeight') {
			data.text = _('Font Size');
		} else if (data.command == '.uno:StyleApply') {
			data.text = _('Style');
		}
	},

	_valuesetControl: function (parentContainer, data, builder) {
		var elem;
		var image;
		var image64;

		if (!data.entries || data.entries.length === 0) {
			return false;
		}

		for (var index in data.entries) {
			image = data.entries[index].image;
			image64 = data.entries[index].image64;
			if (image) {
				image = image.substr(0, image.lastIndexOf('.'));
				image = image.substr(image.lastIndexOf('/') + 1);
				image = 'url("' + app.LOUtil.getImageURL(image + '.svg') + '")';
			}

			if (image64) {
				image = 'url("' + image64 + '")';
			}
			app.LOUtil.checkIfImageExists(image);
			elem = window.L.DomUtil.create('div', 'layout ' +
				(data.entries[index].selected ? ' cool-context-down' : ''), parentContainer);
			$(elem).data('id', data.entries[index].id);
			$(elem).click(function () {
				builder.callback('valueset', 'selected', { id: data.id }, $(this).data('id'), builder);
			});

			elem.style.setProperty('background', image + ' no-repeat center', 'important');
		}

		return false;
	},

	_fixedtextControl: function(parentContainer, data, builder) {
		// Check if this label should render as static content(i.e. span) instead of interactive label
		if (!data.labelFor || !JSDialog.GetFormControlTypesInLO().has(data.labelForType))
			return JSDialog.StaticText(parentContainer, data, builder);

		var fixedtext = window.L.DomUtil.create('label', builder.options.cssClass, parentContainer);

		fixedtext.htmlFor = data.labelFor + '-input';

		if (data.text)
			fixedtext.textContent = builder._cleanText(data.text);
		else if (data.html)
			fixedtext.innerHTML = data.html;

		var accKey = builder._getAccessKeyFromText(data.text);
		builder._stressAccessKey(fixedtext, accKey);

		const updateLabelForAttribute = function(label, labelledControl) {
			const isLabelable = JSDialog.GetFormControlTypesInCO().has(labelledControl.nodeName);
			const isHiddenInput = labelledControl.nodeName === 'INPUT' && labelledControl.type === 'hidden';

			// For labelable element always use htmlFor
			if (isLabelable && !isHiddenInput) {
				labelledControl.removeAttribute('aria-labelledby');
				labelledControl.removeAttribute('aria-label');
				label.htmlFor = labelledControl.id;
				return;
			}

			// For non-labelable element or hidden input always use aria-labelledby
			labelledControl.setAttribute('aria-labelledby', label.id);
			label.removeAttribute('for');
		};

		app.layoutingService.appendLayoutingTask(function () {
			if (!data.labelFor)
				return;

			var labelledControl = document.getElementById(data.labelFor);
			if (labelledControl) {
				var target = labelledControl;
				var input = labelledControl.querySelector('input');
				if (input)
					target = input;
				var select = labelledControl.querySelector('select');
				if (select)
					target = select;

				builder._setAccessKey(target, accKey);
			}

			// we need to schedule it again as some elements are not yet available
			// i.e. pop-ups: Double click on Chart->Sidebar->Colors
			app.layoutingService.appendLayoutingTask(function () {
				var targetElement = document.getElementById(data.labelFor + '-input-' + builder.options.suffix)
					|| document.getElementById(data.labelFor + '-input')
					|| document.getElementById(data.labelFor);

				// Reference label to target element correctly
				if (targetElement)
					updateLabelForAttribute(fixedtext, targetElement);
			});
		});

		fixedtext.id = data.id;
		if (data.style && data.style.length) {
			window.L.DomUtil.addClass(fixedtext, data.style);
		} else {
			window.L.DomUtil.addClass(fixedtext, 'ui-text');
		}

		if (data.hidden)
			$(fixedtext).hide();

		return false;
	},

	_separatorControl: function(parentContainer, data, builder) {
		if (data.orientation && data.orientation === 'vertical') {
			var separator = window.L.DomUtil.create('div', builder.options.cssClass + ' ui-separator vertical', parentContainer);
		} else {
			separator = window.L.DomUtil.create('hr', builder.options.cssClass + ' ui-separator horizontal', parentContainer);
		}
		separator.id = data.id;

		return false;
	},

	_spinnerControl: function(parentContainer, data, builder) {
		var spinner = window.L.DomUtil.create('div', builder.options.cssClass + ' spinner', parentContainer);
		spinner.id = data.id;

		return false;
	},

	_spinnerImgControl: function(parentContainer, data, builder) {
		var svgElement = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
		svgElement.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
		svgElement.setAttribute('width', '298');
		svgElement.setAttribute('height', '192');
		// todo: change svg paths color depending on document type
		svgElement.innerHTML = '<defs> <linearGradient id="spinner-shadow-bottom-linearGradient" x1="131.73" x2="184.28" y1="124.94" y2="177.49" gradientUnits="userSpaceOnUse"> <stop stop-color="#e6e6e6" offset="0"/> <stop stop-color="#ccc" stop-opacity="0" offset="1"/> </linearGradient> <linearGradient id="spinner-shadow-top-linearGradient" x1="130.72" x2="203.26" y1="151.3" y2="118.61" gradientUnits="userSpaceOnUse"> <stop stop-color="#e6e6e6" offset="0"/> <stop stop-color="#b3b3b3" stop-opacity="0" offset="1"/> </linearGradient> <linearGradient id="spinner-paper-linearGradient" x1="166.04" x2="106.19" y1="71.233" y2="36.679" gradientUnits="userSpaceOnUse"> <stop stop-color="#b3d3e5" offset="0"/> <stop stop-color="#d9e9f2" offset="1"/> </linearGradient> <linearGradient id="spinner-paper-fold-linearGradient" x1="167.16" x2="175.12" y1="46.783" y2="51.379" gradientUnits="userSpaceOnUse"> <stop stop-color="#68a7ca" offset="0"/> <stop stop-color="#4290bd" offset="1"/> </linearGradient> <linearGradient id="spinner-inner-left-linearGradient" x1="114.56" x2="142.35" y1="103.94" y2="103.94" gradientUnits="userSpaceOnUse"> <stop stop-color="#4290bd" offset="0"/> <stop stop-color="#4290bd" stop-opacity="0" offset="1"/> </linearGradient> </defs> <g id="spinner-box-g" transform="translate(9.8904)"> <g id="spinner-shadow-g"> <path id="spinner-shadow-top" d="m196.39 151.18 16.366-9.4488-81.829-47.244-16.366 9.4488z" fill="url(#spinner-shadow-top-linearGradient)"/> <path id="spinner-shadow-bottom" d="m130.93 188.98 65.463-37.795 16.366 9.4488-65.463 37.795z" fill="url(#spinner-shadow-bottom-linearGradient)"/> </g> <path id="spinner-box-back" d="m130.93 56.693-65.463 37.795 65.463 37.795 65.463-37.795z" fill="#b4d3e4" fill-opacity=".97667"/> <g> <path id="spinner-box-f-right-bg" d="m130.93 132.28v56.693l65.463-37.795v-56.693z" fill="white"/> <path id="spinner-box-f-right" d="m130.93 132.28v56.693l65.463-37.795v-56.693z" fill="#8ebdd7"/> <path id="spinner-box-f-left" d="m130.93 132.28v56.693l-65.463-37.795v-56.693z" fill="#d9e9f2"/> <path id="spinner-inner-right-bg" d="m81.829 103.94 49.098-28.346 49.098 28.346-49.098 28.346z" fill="white"/> <path id="spinner-inner-right" d="m81.829 103.94 49.098-28.346 49.098 28.346-49.098 28.346z" fill="#68a7ca"/> <path id="spinner-inner-left-bg" d="m130.93 75.591v56.693l-49.098-28.346z" fill="white"/> <path id="spinner-inner-left" d="m130.93 75.591v56.693l-49.098-28.346z" fill="url(#spinner-inner-left-linearGradient)"/> </g> <path id="spinner-handle" d="m114.56 153.07-6.5463 3.7795-32.732-18.898 6.5463-3.7795" fill="none" stroke="#4290bd" stroke-linecap="round" stroke-width="3"/> </g> <g id="spinner-paper-g" transform="translate(9.8904)"> <path id="spinner-paper-bg" d="m130.93 18.898s-14.524 12.927-49.098 28.346l49.098 28.346c16.428-6.7073 29.159-14.202 37.537-19.8 7.5473-5.0432 6.4305-9.2141 6.4305-9.2141-0.42453-2.5692-4.7288-5.0245-4.7288-5.0245z" fill="white"/> <path id="spinner-paper" d="m130.93 18.898s-14.524 12.927-49.098 28.346l49.098 28.346c16.428-6.7073 29.159-14.202 37.537-19.8 7.5473-5.0432 6.4305-9.2141 6.4305-9.2141-0.42453-2.5692-4.7288-5.0245-4.7288-5.0245z" fill="url(#spinner-paper-linearGradient)"/> <path id="spinner-paper-fold-bg" d="m168.46 55.791c7.5473-5.0432-1.2989-9.0071-1.2989-9.0071-0.42453-2.5692 3.0006-5.2315 3.0006-5.2315 4.4877 2.646 4.7288 5.0245 4.7288 5.0245 1.0076 4.4201-6.4305 9.2141-6.4305 9.2141z" fill="white"/> <path id="spinner-paper-fold" d="m168.46 55.791c7.5473-5.0432-1.2989-9.0071-1.2989-9.0071-0.42453-2.5692 3.0006-5.2315 3.0006-5.2315 4.4877 2.646 4.7288 5.0245 4.7288 5.0245 1.0076 4.4201-6.4305 9.2141-6.4305 9.2141z" fill="url(#spinner-paper-fold-linearGradient)"/> </g>';
		var spinner = window.L.DomUtil.create('div', builder.options.cssClass + ' spinner-img ', parentContainer);
		spinner.appendChild(svgElement);
		spinner.id = data.id;

		return false;
	},

	_imageHandler: function(parentContainer, data, builder) {
		if (!data.id)
			return false;

		var image = window.L.DomUtil.create('img', builder.options.cssClass + ' ui-image ui-decorative-image', parentContainer);
		image.id = data.id;
		image.src = data.image ? data.image.replace(/\\/g, '') : '';
		image.alt = data.text;

		return false;
	},

	_htmlControl: function(parentContainer, data, builder) {
		var container = window.L.DomUtil.create('div', builder.options.cssClass, parentContainer);
		container.appendChild(data.content);
		container.id = data.id;
		if (data.style && data.style.length) {
			window.L.DomUtil.addClass(container, data.style);
		}

		if (data.hidden)
			$(container).hide();

		return false;
	},

	// Create a DOM node with an identifiable parent class
	_createIdentifiable : function(type, classNames, parentContainer, data) {
		return window.L.DomUtil.create(
			type, classNames + this._getParentId(data),
			parentContainer);
	},

	_isStringCloseToURL : function(str) {
		return str.indexOf('http') !== -1;
	},

	// TODO: move to jsdialog/Widget.Toolitem.ts
	_unoToolButton: function(parentContainer, data, builder, options) {
		var button = null;

		var controls = {};

		let div = window.L.DomUtil.create('div', 'unotoolbutton ' + builder.options.cssClass + ' ui-content unospan', parentContainer, data);

		controls['container'] = div;
		div.tabIndex = data.tabIndex !== undefined ? data.tabIndex : -1;

		if (data.index)
			div.setAttribute('index', data.index);

		if (data.class)
			window.L.DomUtil.addClass(div, data.class);

		const hasDropdownArrow = !!(options && options.hasDropdownArrow);
		const isSplitButton = !!data.applyCallback;
		const isDropdownButton = !!data.dropdown;
		var shouldHaveArrowbackground = hasDropdownArrow || isDropdownButton;

		const hasPopupRole = data.aria && data.aria.role === 'popup';

		var isRealUnoCommand = true;
		var hasImage = true;

		if (data.text && data.text.endsWith('...')) {
			data.text = data.text.replace('...', '');
		}

		if (data && data.image === false) {
			hasImage = false;
		}

		const itemsToSyncWithContainer = [];

		if (data.visible === false)
			div.classList.add('hidden');

		let enabledTooltip = null;
		const setDisabled = (disabled) => {
			if (disabled) {
				div.setAttribute('disabled', 'true');
				if (button) {
					button.setAttribute('aria-disabled', true);
				}
				if (data.disabledTooltip) {
					div.setAttribute('data-cooltip', builder._cleanText(data.disabledTooltip));
				}
			} else {
				div.removeAttribute('disabled');
				if (button) {
					button.removeAttribute('aria-disabled');
				}
				if (data.disabledTooltip && enabledTooltip) {
					div.setAttribute('data-cooltip', enabledTooltip);
				}
			}
		};

		if (data.command || data.postmessage === true) {
			var id = data.id ? data.id : (data.command && data.command !== '') ? data.command.replace('.uno:', '') : data.text;
			var isUnoCommand = data.command && data.command.indexOf('.uno:') >= 0;

			isRealUnoCommand = isUnoCommand;

			if (id)
				id.replace(/\%/g, '').replace(/\./g, '-').replace(' ', '');
			else
				console.warn('_unoToolButton: no id provided');

			if (data.command)
				window.L.DomUtil.addClass(div, data.command.replace(':', '').replace('.', ''));

			if (isRealUnoCommand) {
				// there is a problem with JSON crafted manually on the JS side
				// we do not have warranty the id we passed is the one used in the DOM
				// updates might then fail to find such item
				// we need to remember the original ID
				div.setAttribute('modelId', id);
				id = JSDialog.MakeIdUnique(id);
			}

			div.id = id;

			var buttonId = id + '-button';

			button = window.L.DomUtil.create('button', 'ui-content unobutton', div);
			if (div.tabIndex == 0)
				button.tabIndex = -1; // prevent button from taking focus since container div itself is focusable element
			button.id = buttonId;

			JSDialog.SetupA11yLabelForNonLabelableElement(button, data, builder);

			itemsToSyncWithContainer.push(button);

			if (!data.accessKey)
				builder._setAccessKey(button, builder._getAccessKeyFromText(data.text));
			else
				button.accessKey = data.accessKey;

			if (data.w2icon) {
				// FIXME: DEPRECATED, this is legacy way to setup icon based on CSS class
				var buttonImage = window.L.DomUtil.create('div', 'w2ui-icon ' + data.w2icon, button);
			}
			else if (hasImage !== false){
				if (data.icon) {
					buttonImage = window.L.DomUtil.create('img', '', button);
					if (this._isStringCloseToURL(data.icon)) {
						buttonImage.src = data.icon;
					} else {
						app.LOUtil.setImage(buttonImage, data.icon, builder.map);
						// Fall back to base64 PNG if the SVG is not available.
						// checkIfImageExists sets display:none on error, so
						// restore it when substituting the fallback image.
						if (data.image) {
							buttonImage.onerror = function() {
								buttonImage.onerror = null;
								buttonImage.src = data.image;
								buttonImage.style.display = '';
							};
						}
					}
				}
				else if (data.image) {
					buttonImage = window.L.DomUtil.create('img', '', button);
					buttonImage.src = data.image;
				}
				else {
					var iconName = app.LOUtil.getIconNameOfCommand(data.command);
					if (iconName) {
						buttonImage = window.L.DomUtil.create('img', '', button);
						app.LOUtil.setImage(buttonImage, iconName, builder.map);
					} else {
						buttonImage = false;
					}
				}
			} else {
				buttonImage = false;
			}

			controls['button'] = button;
			var span;
			// NOTE: menubutton handles the noLabel case differently from other types
			// like toolitem, please see function `_menubuttonControl`
			if (data.noLabel && data.type != 'menubutton')
				$(div).addClass('no-label');
			else if (builder.options.noLabelsForUnoButtons !== true && data.text) {
				span = window.L.DomUtil.create('span', 'ui-content unolabel', button);
				span.textContent = builder._cleanText(data.text);
				builder._stressAccessKey(span, button.accessKey);
				controls['label'] = span;
				$(div).addClass('has-label');
			} else if (builder.options.useInLineLabelsForUnoButtons === true && data.text) {
				$(div).addClass('inline');
				span = window.L.DomUtil.create('span', 'ui-content unolabel', div);
				span.textContent = builder._cleanText(data.text);

				controls['label'] = span;
			} else {
				$(div).addClass('no-label');
			}

			let tooltip = builder._cleanText(data.tooltip) || builder._cleanText(data.text);
			if (data.command && (!tooltip || !tooltip.includes('('))) // Add shortcut to tooltip based on command
				tooltip = JSDialog.ShortcutsUtil.getShortcut(tooltip, data.command);
			div.setAttribute('data-cooltip', tooltip);
			enabledTooltip = tooltip;

			// Set aria-pressed only if:
			// 1. A real toggle button
			const setAriaPressedForToggleBtn = (value) => {
				if (JSDialog.IsToggleButton(id, data.command, builder)) {
					button.setAttribute('aria-pressed', value);
				}
			};
			const selectFn = () => {
				window.L.DomUtil.addClass(button, 'selected');
				window.L.DomUtil.addClass(div, 'selected');
				setAriaPressedForToggleBtn(true);
			};

			const unSelectFn = () => {
				window.L.DomUtil.removeClass(button, 'selected');
				window.L.DomUtil.removeClass(div, 'selected');
				setAriaPressedForToggleBtn(false);
			};

			if (data.command) {
				const updateFunction = () => {
					const items = builder.map['stateChangeHandler'];
					const state = items.getItemValue(data.command);
					const isOn = state && state === 'true';

					if (isOn)
						selectFn();
					else
						unSelectFn();

					// Swap icon if stateIcons are provided
					if (data.stateIcons && buttonImage) {
						const iconName = isOn ? data.stateIcons.on : data.stateIcons.off;
						app.LOUtil.setImage(
							buttonImage,
							'lc_' + iconName + '.svg',
							builder.map,
						);
					}

					// Swap tooltip text when activeTooltip is set
					if (data.activeTooltip) {
						if (isOn) {
							div._wasActiveTooltip = true;
							div.setAttribute('data-cooltip', data.activeTooltip);
						} else if (div._wasActiveTooltip) {
							div._wasActiveTooltip = false;
							div.setAttribute('data-cooltip', enabledTooltip);
						}
					}
				};

				updateFunction();
				setDisabled(data.enabled === false);

				builder.map.on('commandstatechanged', function(e) {
					if (e.commandName === data.command)
					{
						updateFunction();
						// in some cases we will get both property like state and disabled
						// to handle it we will set disable var based on INCOMING info (ex: .uno:ParaRightToLft)
						setDisabled(e.disabled || e.state == 'disabled');
					}
				}, this);
			}

			setDisabled(data.enabled === false);
			div.onSelect = selectFn;
			div.onUnSelect = unSelectFn;

			if (data.selected === true)
				selectFn();
		} else {
			var span = window.L.DomUtil.create('span', 'ui-content unolabel', div);
			span.textContent = builder._cleanText(data.text);
			controls['button'] = button;
			controls['label'] = span;
		}

		const hasPopUp = hasPopupRole || JSDialog.IsDialogButton(id, data.command) || JSDialog.IsDropdownButton(id, data.command);

		if (hasPopUp) {
			button.setAttribute('aria-expanded', false);
			if (JSDialog.IsDialogButton(id, data.command))
				button.setAttribute('aria-haspopup', 'dialog');
			else
				button.setAttribute('aria-haspopup', true);
		}

		JSDialog.SetupA11yLabelForNonLabelableElement(button, data, builder);

		// for Accessibility : graphic elements are located within buttons, the img should receive an empty alt
		if (button.getAttribute('aria-label') || button.getAttribute('aria-labelledby')){ // if we already set the aria-label or aria-labelledby then do not go for image alt attr
			buttonImage.alt = '';
		}
		else if (buttonImage !== false) {
			if (data.aria)
				buttonImage.alt = data.aria.label;
			else
				buttonImage.alt = builder._cleanText(data.text);
		}

		const getAriaLabelGroupText = function(command) {
			if (JSDialog.IsToggleButton(id, command, builder))
				return _('Toggle dropdown');
			else if (JSDialog.IsDialogButton(id, command))
				return _('Dialog dropdown');
			else if (JSDialog.IsDropdownButton(id, data.command))
				return _('Dropdown');

			return _('Apply action dropdown');
		};

		var arrowbackground;
		if (shouldHaveArrowbackground) {
			$(div).addClass('has-dropdown');
			div.setAttribute('role', 'group');

			const a11yData = {
				id: id,
				type: data.type,
				aria: {
					label: getAriaLabelGroupText(data.command),
				},
			};
			JSDialog.AddAriaLabel(div, a11yData, builder);

			// shouldHaveArrowbackground is true when we have dropdown arrow or dropdown button
			// if main button and arrowbackground opens same dropdown then only arrowbackground should be div.
			// otherwise, it should be button always.
			const shouldArrowbackgroundButton = isSplitButton || !JSDialog.IsDropdownButton(id, data.command);
			if (shouldArrowbackgroundButton) {
				// Arrow should be a real button (user can interact with it)
				arrowbackground = window.L.DomUtil.create('button', 'arrowbackground', div);

				const buttonText = data.aria && data.aria.label ? data.aria.label : builder._cleanText(data.text);
				const dropdownAriaLabelText = _('Open {name}').replace('{name}', buttonText);
				arrowbackground.setAttribute('aria-label', dropdownAriaLabelText);

				arrowbackground.setAttribute('aria-haspopup', true);
				arrowbackground.setAttribute('aria-expanded', false);
			} else {
				// Arrow is just decoration
				arrowbackground = window.L.DomUtil.create('div', 'arrowbackground', div);
				arrowbackground.setAttribute('aria-hidden', 'true');
				arrowbackground.tabIndex = '-1'; // arrow is just decorative
			}
			var unoarrow = window.L.DomUtil.create('span', 'unoarrow', arrowbackground);
			unoarrow.setAttribute('aria-hidden', 'true');
			controls['arrow'] = arrowbackground;

			if (!hasDropdownArrow && isDropdownButton) {
				// Attach 'click' event listeners for dropdown buttons only
				arrowbackground.addEventListener('click', function (event) {
					openToolBoxMenu(event);

					if (shouldArrowbackgroundButton) {
						arrowbackground.setAttribute('aria-expanded', 'true');
					} else {
						button.setAttribute('aria-expanded', 'true');
					}
				});

				const closemenuParentId = parentContainer.id;
				div.closeDropdown = function() {
					builder.callback('toolbox', 'closemenu',
						closemenuParentId ? {id: closemenuParentId} : parentContainer,
						data.command, builder);

					if (shouldArrowbackgroundButton) {
						arrowbackground.setAttribute('aria-expanded', 'false');
					} else {
						button.setAttribute('aria-expanded', 'false');
					}
				};
			}
			itemsToSyncWithContainer.push(arrowbackground);
		}

		JSDialog.SynchronizeDisabledState(div, itemsToSyncWithContainer);

		// _onDropDown only works for splitbutton or dropdown arrow button
		// for decorative button we need to manage it via click function and closeDropdown
		div._onDropDown = function(open) {

			if (JSDialog.IsDropdownButton(id, data.command))
				button.setAttribute('aria-expanded', open);
			else
				arrowbackground.setAttribute('aria-expanded', open);
		};

		var openToolBoxMenu = function(event) {
			if (!div.hasAttribute('disabled')) {
				builder.callback('toolbox', 'openmenu', parentContainer, data.command, builder);
				event.stopPropagation();
			}
		};

		var clickFunction = function (e) {
			if (!div.hasAttribute('disabled')) {
				builder.refreshSidebar = true;
				if (data.postmessage) {
					let isContextualButton = e.target.offsetParent.id === 'context-toolbar';
					const msgId = isContextualButton ? 'Clicked_ContextualButton' : 'Clicked_Button';
					builder.map.fire('postMessage', {msgId: msgId, args: {Id: data.id} });
				}
				else if (isRealUnoCommand && data.dropdown !== true)
					builder.callback('toolbutton', 'click', button, data.command, builder);
				else {
					builder.callback('toolbox', 'click', parentContainer, data.command, builder);
					button.setAttribute('aria-expanded', 'true');
				}
			}
			e.preventDefault();
			e.stopPropagation();
		};

		const hasLabel = !!controls.label;
		const hasExplicitTooltip = !!data.tooltip;
		const hasShortcut = JSDialog.ShortcutsUtil.hasShortcut(data.command);
		var mouseEnterFunction = window.touch.mouseOnly(function () {
			if (builder.map.tooltip)
				builder.map.tooltip.beginShow(div);
		});

		var mouseLeaveFunction = function () {
			if (builder.map.tooltip)
				builder.map.tooltip.beginHide(div);
		};

		$(controls.button).on('click', clickFunction);
		$(controls.label).on('click', clickFunction);
		if (data.doubleClickCommand) {
			var doubleClickHandler = function(e) {
				e.preventDefault();
				e.stopPropagation();
				if (div.hasAttribute('disabled')) return;
				builder.map.sendUnoCommand(data.doubleClickCommand, data.doubleClickCommandArgs);
			};
			$(controls.button).on('dblclick', doubleClickHandler);
			$(controls.label).on('dblclick', doubleClickHandler);
		}
		// We need a way to also handle the custom tooltip for any tool button like save in shortcut bar
		if (data.isCustomTooltip) {
			this._handleCustomTooltip(div, builder);
		}
		else if (!hasLabel || hasExplicitTooltip || hasShortcut) {
			$(div).on('mouseenter', mouseEnterFunction);
			$(div).on('mouseleave', mouseLeaveFunction);
		} else {
			div.removeAttribute('data-cooltip'); // We don't need a tooltip for this button
		}

		div.addEventListener('keydown', function(e) {
			switch (e.key) {
			case 'Escape':
				builder.map.focus();
				break;
			}
		});

		builder._preventDocumentLosingFocusOnClick(div);

		setDisabled(data.enabled === false);
		builder.map.hideRestrictedItems(data, controls['container'], controls['container']);
		builder.map.disableLockedItem(data, controls['container'], controls['container']);

		return controls;
	},

	_handleCustomTooltip: function(elem, builder) {
		// Prefer modelid, fallback to id if modelid is missing
		const lookupId = elem.getAttribute('modelid') || elem.id;
		switch (lookupId) {
			case 'save':
				$(elem).on('mouseenter', window.touch.mouseOnly(function() {
					if (builder.map.tooltip)
						builder.map.tooltip.show(elem, builder.map.getLastModDateValue()); // Show the tooltip with the correct content
				}));

				$(elem).on('mouseleave', function() {
					if (builder.map.tooltip)
						builder.map.tooltip.hide(elem);
				});
		}
	},

	_divContainerHandler: function (parentContainer, data, builder) {
		if (!(data.children && data.children.length))
			return false;

		var divElem = window.L.DomUtil.create('div', builder.options.cssClass, parentContainer);
		if (data.style && data.style.length)
			window.L.DomUtil.addClass(divElem, data.style);
		for (var i = 0; i < data.children.length; ++i) {
			var entry = data.children[i];
			var handle = builder._controlHandlers[entry.type];
			if (handle) {
				handle(divElem, entry, builder);
			}
		}
		builder._setupHandlers(divElem, data.handlers);
		return false;
	},

	_colorSampleControl: function (parentContainer, data, builder) {
		var sampleSizeClass = 'color-sample-small';
		if (data.size === 'big')
			sampleSizeClass = 'color-sample-big';
		var colorSample = window.L.DomUtil.create('div', builder.options.cssClass + ' ' + sampleSizeClass, parentContainer);
		colorSample.id = data.id;
		colorSample.style.backgroundColor = data.color;
		colorSample.name = data.color.substring(1);

		if (data.size === 'big') {
			var selectionMarker = window.L.DomUtil.create('div', 'colors-container-tint-mark', colorSample);
			if (data.selected) {
				selectionMarker.style.visibility = 'visible';
			} else {
				selectionMarker.style.visibility = 'hidden';
			}
		} else if (data.selected && data.mark) {
			colorSample.appendChild(data.mark);
			window.L.DomUtil.addClass(colorSample, 'colors-container-selected-basic-color');
		}

		 builder._setupHandlers(colorSample, data.handlers);

		 return false;
	},

	_subMenuHandler: function(parentContainer, data, builder) {
		var title = data.text;
		builder._explorableMenu(parentContainer, title, data.children, builder, undefined, data.id);

		return false;
	},

	_menuItemHandler: function(parentContainer, data, builder) {
		var title = data.text;
		var cssClassHeader = 'ui-header';
		// separator
		if (title === '') {
			return false;
		}

		var id = data.id;
		if (id) {
			cssClassHeader += ' ui-header-' + id;
			var handler = builder._menuItemHandlers[id];
			if (handler) {
				handler(parentContainer, data, builder);
				return;
			}
		}

		var menuEntry = window.L.DomUtil.create('div', cssClassHeader + ' level-' + builder._currentDepth + ' ' + builder.options.cssClass + ' ui-widget', parentContainer);

		if (data.hyperlink) {
			menuEntry = window.L.DomUtil.create('a', 'context-menu-link', menuEntry);
			menuEntry.href = '#';
		}

		var icon = null;
		var commandName = data.command && data.command.startsWith('.uno:') ? data.command.substring('.uno:'.length) : data.id;
		if (commandName && commandName.length && app.LOUtil.existsIconForCommand(commandName, builder.map.getDocType())) {
			var iconName = builder._generateMenuIconName(commandName);
			var iconSpan = window.L.DomUtil.create('span', 'menu-entry-icon ' + iconName, menuEntry);
			iconName = app.LOUtil.getIconNameOfCommand(iconName, true);
			icon = window.L.DomUtil.create('img', '', iconSpan);
			app.LOUtil.setImage(icon, iconName, builder.map);
			icon.alt = '';
		}
		if (data.checked && data.checked === true) {
			window.L.DomUtil.addClass(menuEntry, 'menu-entry-checked');
		}

		var titleSpan = window.L.DomUtil.create('span', '', menuEntry);
		titleSpan.innerHTML = title;
		var paddingClass = icon ? 'menu-entry-with-icon flex-fullwidth' : 'menu-entry-no-icon';
		window.L.DomUtil.addClass(titleSpan, paddingClass);

		if (builder.wizard) {
			$(menuEntry).click(() => {
				if (window.insertionMobileWizard)
					app.dispatcher.dispatch('insertion_mobile_wizard');
				else if (window.mobileMenuWizard)
					$('#main-menu-state').click();
				else if (window.contextMenuWizard) {
					window.contextMenuWizard = false;
					builder.map.fire('closemobilewizard');
				}

				// before close the wizard then execute the action
				if (data.executionType === 'action') {
					builder.map.menubar._executeAction(undefined, data);
				} else if (data.executionType === 'callback') {
					data.callback();
				} else if (!builder.map._clip || !builder.map._clip.filterExecCopyPaste(data.command)) {
					// Header / footer is already inserted.
					if ((data.command.startsWith('.uno:InsertPageHeader') ||
							 data.command.startsWith('.uno:InsertPageFooter')) &&
							data.checked && data.checked === true) {
						return;
					}
					builder.map.sendUnoCommand(data.command);
				}
			});
		} else {
			window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
		}

		builder.map.hideRestrictedItems(data, menuEntry, menuEntry);
		builder.map.disableLockedItem(data, menuEntry, menuEntry);

		return false;
	},

	_insertTableMenuItem: function(parentContainer, data, builder) {
		var title = data.text;

		var content = window.L.DomUtil.create('div', 'inserttablecontrols');

		var rowsData = { min: 0, id: 'rows', text: '2', label: _('Rows') };
		var colsData = { min: 0, id: 'cols', text: '2', label: _('Columns') };

		var callbackFunction = function(objectType, eventType, object) {
			if (eventType == 'plus') {
				$(object).find('input').val(function(i, oldval) {
					return parseInt(oldval, 10) + 1;
				});
			} else if (eventType == 'minus') {
				$(object).find('input').val(function(i, oldval) {
					if (oldval > 0)
						return parseInt(oldval, 10) - 1;
					else
						return 0;
				});
			}
		};

		JSDialog._spinfieldControl(content, rowsData, builder, callbackFunction);
		JSDialog._spinfieldControl(content, colsData, builder, callbackFunction);

		var buttonData = { text: _('Insert Table') };
		JSDialog.pushButton(content, buttonData, builder, function() {
			var rowsCount = parseInt($('#rows > input.spinfield').get(0).value);
			var colsCount = parseInt($('#cols > input.spinfield').get(0).value);
			builder.map.sendUnoCommand('.uno:InsertTable?Columns=' + colsCount + '&Rows=' + rowsCount);
			builder.map.fire('closemobilewizard');
		});

		builder._explorableMenu(parentContainer, title, data.children, builder, content, data.id);
	},

	_getItemById: function(container, id) {
		const plainId = this._removeMenuId(id);
		// prefer the modelId first which is unique within single component, DOM id might be changed due to conflict
		let control = container.querySelector('[modelId=\'' + plainId + '\']');
		if (!control)
			control = container.querySelector('[id=\'' + plainId + '\']');
		return control;
	},

	// executes actions like changing the selection without rebuilding the widget
	executeAction: function(container, data) {
		app.layoutingService.appendLayoutingTask(() => { this.executeActionImpl(container, data); });
	},

	executeActionImpl: function(container, data) {
		let control = this._getItemById(container, this._removeMenuId(data.control_id));
		if (!control && data.control)
			control = this._getItemById(container, this._removeMenuId(data.control.id));
		if (!control) {
			window.app.console.warn('executeAction: not found control with id: "' + data.control_id +
				'" to perform action: "' + data.action_type + '"');
			return;
		}

		console.assert(data.action_type);

		switch (data.action_type) {
		case 'grab_focus':
			if (typeof control.onFocus === 'function')
				control.onFocus();
			else
				control.focus();
			break;
		case 'select':
			if (typeof control.onSelect === 'function')
				control.onSelect(parseInt(data.position));
			else
				window.app.console.warn('widget "' + data.control_id + '" doesn\'t support "select" action');
			break;

		case 'unselect':
			if (typeof control.onUnSelect === 'function')
				control.onUnSelect(parseInt(data.position));
			else
				window.app.console.warn('widget "' + data.control_id + '" doesn\'t support "unselect" action');
			break;

		case 'show':
			$(control).removeClass('hidden');
			$(control).show();
			break;

		case 'hide':
			$(control).addClass('hidden');
			break;

		case 'enable':
			control.disabled = false;
			control.removeAttribute('disabled');
			break;

		case 'disable':
			control.setAttribute('disabled', 'true');
			control.disabled = true;
			break;

		case 'setText':
			if (typeof control.onSetText === 'function') {
				control.onSetText(data.text);
				break;
			}

			// eg. in mobile wizard input is inside spin button div
			var innerInput = control.querySelector('input');
			var isSpinField = innerInput && control.classList.contains('spinfieldcontainer');
			if (innerInput)
				control = innerInput;

			var currentText = this._cleanText(data.text);
			if (isSpinField && control.parentElement._unit)
				currentText = JSDialog._formatSpinFieldValue(currentText, control.parentElement._unit);
			if (isSpinField)
				JSDialog._setSpinFieldValue(control, currentText);
			else
				control.value = currentText;
			if (data.selection) {
				var selection = data.selection.split(';');
				if (selection.length === 2) {
					var start = parseInt(selection[0]);
					var end = parseInt(selection[1]);

					if (start > end) {
						var tmp = start;
						start = end;
						end = tmp;
					}

					if (document.activeElement === control) // Safari/Gnome Web compatibility
						control.setSelectionRange(start, end);
				} else if (selection.length === 4) {
					var startPos = parseInt(selection[0]);
					var endPos = parseInt(selection[1]);
					var startPara = parseInt(selection[2]);
					var endPara = parseInt(selection[3]);
					var start = 0;
					var end = 0;

					var row = 0;
					for (;row < startPara; row++) {
						var found = currentText.indexOf('\n', start);
						if (found === -1)
							break;
						start = found + 1;
					}

					start += startPos;

					row = 0;
					for (;row < endPara; row++) {
						found = currentText.indexOf('\n', end);
						if (found === -1)
							break;
						end = found + 1;
					}

					end += endPos;

					if (start > end) {
						var tmp = start;
						start = end;
						end = tmp;
					}

					if (document.activeElement === control) // Safari/Gnome Web compatibility
						control.setSelectionRange(start, end);
				}
			}
			break;

		case 'rename':
			control.setAttribute('modelId', data.new_id);
			control.id = JSDialog.MakeIdUnique(data.new_id);
			break;

		case 'rendered_entry':
		case 'rendered_combobox_entry':
		{
			if (!this.rendersCache[control.id])
				this.rendersCache[control.id] = { persistent: false, images: [] };

			const oldImage = this.rendersCache[control.id].images[data.pos];
			if (oldImage === data.image) {
					app.console.debug('rendered_entry: "' + data.pos + '" for "' + control.id + '" - no change');
					break;
			}

			this.rendersCache[control.id].images[data.pos] = data.image;

			if (typeof control.updateRenders == 'function')
				control.updateRenders(data.pos);
			else
				app.console.error('widget "' + control.id + '" doesn\'t support custom entries');
		}
		break;

		default:
			app.console.error('unknown action: "' + data.action_type + '" for "' + control.id + '"');
			break;
		}
	},

	_removeMenuId: function (rawId) {
		var elementId = typeof rawId === 'string' ? rawId : (rawId ? String(rawId) : rawId);
		var separatorPos = elementId ? elementId.indexOf(':') : 0; // delete menuId
		if (separatorPos > 0)
			elementId = elementId.substr(0, separatorPos);
		return elementId;
	},

	_updateWidgetImpl: function (container, data, buildFunc) {
		const elementId = this._removeMenuId(data.id);
		const control = this._getItemById(container, elementId);
		if (!control) {
			window.app.console.warn('jsdialogupdate: not found control with id: "' + elementId + '"');
			return;
		}

		var parent = control.parentNode;
		if (!parent)
			return;

		// Don't rebuild a tree view while inline cell editing is active;
		// the backend will send an up-to-date state after editend.
		if (control.querySelector('.ui-treeview-inline-edit'))
			return;

		// Restore expander depth so heading levels are correct when
		// rebuilding a nested expander on-demand (see _expanderHandler).
		var savedExpanderDepth = this._expanderDepth;
		if (control.dataset && control.dataset.expanderDepth !== undefined) {
			this._expanderDepth = parseInt(control.dataset.expanderDepth);
		}

		var scrollTop = control.scrollTop;
		var focusedElement = document.activeElement;
		var focusedElementInDialog = focusedElement ? container.querySelector('[id=\'' + focusedElement.id + '\']') : null;
		var focusedId = focusedElementInDialog ? focusedElementInDialog.id : null;

		var temporaryParent = new DocumentFragment();

		// Preserve spinfield unit across rebuilds: if the old element stored
		// a unit and the incoming data does not carry one, inject it so the
		// rebuilt widget keeps showing the unit.
		if (!data.unit && control.dataset && control.dataset.unit)
			data.unit = control.dataset.unit;

		// Remove the id of the to-be-removed control, so JSDialog.MakeIdUnique() won't rename
		// data.id to something we can't find later.
		control.id = '';

		buildFunc.bind(this)(temporaryParent, [data], false);
		this._expanderDepth = savedExpanderDepth;
		var backupGridColSpan = control.style.gridColumn;
		var backupGridRowSpan = control.style.gridRow;

		control.replaceWith(temporaryParent.firstChild)

		var newControl = container.querySelector('[id=\'' + elementId + '\']');
		if (newControl) {
			newControl.scrollTop = scrollTop;
			newControl.style.gridColumn = backupGridColSpan;
			newControl.style.gridRow = backupGridRowSpan;

			// todo: is that needed? should be in widget impl?
			if (data.has_default === true && (data.type === 'pushbutton' || data.type === 'okbutton')) {
				const buttonNode = newControl.querySelector('button');
				if (buttonNode) window.L.DomUtil.addClass(buttonNode, 'button-primary');
			}
		}

		if (focusedId) {
			var found = container.querySelector('[id=\'' + focusedId + '\']');
			if (found)
				found.focus();
		}
	},

	// replaces widget in-place with new instance with updated data
	updateWidget: function (container, data) {
		app.layoutingService.appendLayoutingTask(() => {
			this._updateWidgetImpl(container, data, this.build);
		});
	},

	// Helper to compose the grid style for the row or column with span
	_composeGridStyle: function(pos, size) {
		let span, grid;
		if (size) {
			span = 'span ' + parseInt(size);
		}
		if (pos) {
			grid = parseInt(pos) + 1;
		}
		if (span) {
			if (grid) {
				return grid + ' / ' + span;
			}
			return span;
		}
		return grid;
	},

	// Helper to set the grid styles for the row and column with span
	_setGridStyles: function(control, data) {
		let gridColumn = this._composeGridStyle(data.left, data.width);
		if (gridColumn) {
			control.style.gridColumn = gridColumn;
		}

		let gridRow = this._composeGridStyle(data.top, data.height);
		if (gridRow) {
			control.style.gridRow = gridRow;
		}
	},

	postProcess: function(parent, data) {
		if (!parent || !data || !data.id || data.id === '')
			return;

		const control = this._getItemById(parent, data.id);
		if (data.visible === 'false' || data.visible === false) {
			if (control)
				window.L.DomUtil.addClass(control, 'hidden');
			else if (parent.id === data.id)
				window.L.DomUtil.addClass(parent, 'hidden');
		}

		if (control) {
			this._setGridStyles(control, data);

			if (data.tooltip) {
				control.setAttribute('data-cooltip', this._cleanText(data.tooltip));
				window.L.control.attachTooltipEventListener(control, this.map);
			}
		}

		// natural tab-order when using keyboard navigation
		if (control && !control.hasAttribute('tabIndex')
			&& control.querySelectorAll('[tabindex]').length === 0
			&& data.type !== 'container'
			&& data.type !== 'tabpage'
			&& data.type !== 'tabcontrol'
			&& data.type !== 'drawingarea'
			&& data.type !== 'grid'
			&& data.type !== 'image'
			&& data.type !== 'toolbox'
			&& data.type !== 'listbox'
			&& data.type !== 'combobox'
			&& data.type !== 'radiobutton'
			&& data.type !== 'checkbox'
			&& data.type !== 'spinfield'
			&& data.type !== 'metricfield'
			&& data.type !== 'formattedfield'
			&& data.type !== 'fixedtext'
			&& data.type !== 'frame'
			&& data.type !== 'expander'
			&& data.type !== 'panel'
			&& data.type !== 'buttonbox'
			&& data.type !== 'treelistbox'
			&& data.type !== 'time'
			&& data.type !== 'separator'
			&& data.type !== 'spacer'
			&& data.type !== 'edit'
			&& data.type !== 'deck'
			&& data.type !== 'pushbutton'
			&& data.type !== 'iconview'
			&& data.type !== 'overflowgroup'
			)
			control.setAttribute('tabIndex', '0');
	},

	// some widgets we want to modify / change
	isHyperlinkTarget: function (builder, data) {
		return data.type === 'combobox' && (data.id === 'target' || data.id === 'receiver');
	},

	requiresOverwriting: function(builder, data) {
		if (builder.isHyperlinkTarget(builder, data))
			return true;

		return false;
	},

	overwriteHandler: function(parentContainer, data, builder) {
		if (builder.isHyperlinkTarget(builder, data)) {
			// Replace combobox with edit
			var callback = function(value) {
				builder.callback('combobox', 'change', data, value, builder);
			};

			return builder._controlHandlers['edit'](parentContainer, data, builder, callback);
		}

		console.error('It seems widget doesn\'t require overwriting.');
	},

	build: function(parent, data, hasVerticalParent) {

		// TODO: check and probably remove additional containers
		if (hasVerticalParent === undefined) {
			parent = window.L.DomUtil.create('div', 'root-container ' + this.options.cssClass, parent);
			parent = window.L.DomUtil.create('div', 'vertical ' + this.options.cssClass, parent);
		}

		for (var childIndex in data) {
			var childData = data[childIndex];
			if (!childData)
				continue;

			var childType = childData.type;

			this._handleResponses(childData, this);

			var containerToInsert = parent;

			if (childData.dialogid) {
				containerToInsert.id = childData.dialogid;
				this.dialogId = childData.dialogid;
			}

			var isVertical = childData.vertical === 'true' || childData.vertical === true ? true : false;

			var processChildren = true;

			if ((childData.id === undefined || childData.id === '' || childData.id === null)
				&& (childType == 'checkbox' || childType == 'radiobutton')) {
				continue;
			}

			var hasManyChildren = childData.children && childData.children.length > 1;
			var isContainer = this.isContainerType(childData.type);
			if (hasManyChildren && isContainer) {
				var table = window.L.DomUtil.createWithId('div', childData.id, containerToInsert);

				if (childData.cargo && childData.cargo['custom-label']) {
					var label = window.L.DomUtil.create('label', '', table);
					label.textContent = childData.cargo['custom-label'];
					label.htmlFor = childData.cargo['custom-label-for'] + '-input';
					table.id = '';
					table = window.L.DomUtil.createWithId('div', childData.id, table);
				}

				$(table).addClass(this.options.cssClass);

				if (!isVertical) {
					var rows = this._getGridRows(childData.children);
					var cols = this._getGridColumns(childData.children);

					if (rows > 1 && cols > 1) {
						var gridRowColStyle = 'grid-template-rows: repeat(' + rows  + '); \
							grid-template-columns: repeat(' + cols  + ');';

						table.style = gridRowColStyle;
					} else {
						$(table).css('grid-auto-flow', 'column');
					}

					$(table).css('display', 'grid');
				}

				$(table).addClass('ui-grid-cell');

				// if 'table' has no id - postprocess won't work...
				this._setGridStyles(table, childData);

				var childObject = table;

				this.postProcess(containerToInsert, childData);
			} else {
				childObject = containerToInsert;
			}

			var handler = this._controlHandlers[childType];

			if (handler) {
				if (this.requiresOverwriting(this, childData))
					processChildren = this.overwriteHandler(childObject, childData, this);
				else
					processChildren = handler(childObject, childData, this);
				this.postProcess(childObject, childData);
			} else
				window.app.console.warn('JSDialogBuilder: Unsupported control type: "' + childType + '"');

			if (processChildren && childData.children != undefined)
				this.build(childObject, childData.children, isVertical);
			else if (childData.visible && (childData.visible === false || childData.visible === 'false')) {
				$('#' + childData.id).addClass('hidden-from-event');
			}
		}
	}
});

window.L.Control.JSDialogBuilder.getMenuStructureForMobileWizard = function(menu, mainMenu, itemCommand) {
	if (itemCommand.indexOf('sep') >= 0)
		return null;

	var itemText = '';
	if (menu.name)
		itemText = menu.name;

	var itemType = 'submenu';
	var executionType = 'menu';
	if (mainMenu) {
		itemType = 'mainmenu';
		executionType = 'menu';
	} else if (menu.callback) {
		itemType = 'menuitem';
		executionType = 'callback';
	} else if (!menu.items) {
		itemType = 'menuitem';
		executionType = 'command';
	}

	var menuStructure = {
		type : itemType,
		enabled : true,
		text : itemText,
		executionType : executionType,
		children : []
	};
	if (itemCommand)
		menuStructure['command'] = itemCommand;
	if (menu.icon)
		menuStructure['checked'] = true;
	if (menu.callback)
		menuStructure['callback'] = menu.callback;
	if (menu.isHtmlName)
		menuStructure['hyperlink'] = true;

	if (mainMenu) {
		for (var menuItem in menu) {
			var subItemCommand = menu[menuItem].command ? menu[menuItem].command : menuItem;
			var element = this.getMenuStructureForMobileWizard(menu[menuItem], false, subItemCommand);
			if (element)
				menuStructure['children'].push(element);
		}
	} else if (itemType == 'submenu') {
		for (menuItem in menu.items) {
			element = this.getMenuStructureForMobileWizard(menu.items[menuItem], false, menuItem);
			if (element)
				menuStructure['children'].push(element);
		}
		if (menu.command) {
			menuStructure.id = menu.command.substring(5).toLowerCase();
		}
	}

	return menuStructure;
};

window.L.control.jsDialogBuilder = function (options) {
	var builder = new window.L.Control.JSDialogBuilder(options);
	builder._setup(options);
	return builder;
};
