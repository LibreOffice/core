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
 * window.L.Control.MobileWizardBuilder used for building the native HTML component
 * variants for mobile/touch devices from the JSON description provided by the server.
 */

/* global $ _UNO _ JSDialog app ColorPicker */

window.L.Control.MobileWizardBuilder = window.L.Control.JSDialogBuilder.extend({
	_customizeOptions: function() {
		this.options.noLabelsForUnoButtons = true;
		this.options.useInLineLabelsForUnoButtons = false;
		this.options.cssClass = 'mobile-wizard';
	},

	_overrideHandlers: function() {
		this._controlHandlers['basespinfield'] = this.baseSpinField;
		this._controlHandlers['checkbox'] = this._checkboxControl;
		this._controlHandlers['combobox'] = JSDialog.mobileCombobox;
		this._controlHandlers['comboboxentry'] = JSDialog.mobileComboboxEntry;
		this._controlHandlers['edit'] = this._editControl;
		this._controlHandlers['frame'] = this._frameHandler;
		this._controlHandlers['grid'] = this._gridHandler;
		this._controlHandlers['listbox'] = this._listboxControl;
		this._controlHandlers['mobile-popup-container'] = this._mobilePopupContainer;
		this._controlHandlers['panel'] = this._panelHandler;
		this._controlHandlers['paneltabs'] = JSDialog.mobilePanelControl;
		this._controlHandlers['radiobutton'] = this._radiobuttonControl;
		this._controlHandlers['scrollwindow'] = undefined;
		this._controlHandlers['submenutabs'] = JSDialog.mobileSubmenuTabControl;
		this._controlHandlers['tabcontrol'] = JSDialog.mobileTabControl;
		this._controlHandlers['borderstyle'] = JSDialog.mobileBorderSelector;

		this._controlHandlers['colorlistbox'] = this._colorControl;
		this._toolitemHandlers['.uno:XLineColor'] = this._colorControl;
		this._toolitemHandlers['.uno:FontColor'] = this._colorControl;
		this._toolitemHandlers['.uno:CharBackColor'] = this._colorControl;
		this._toolitemHandlers['.uno:BackgroundColor'] = this._colorControl;
		this._toolitemHandlers['.uno:TableCellBackgroundColor'] = this._colorControl;
		this._toolitemHandlers['.uno:FrameLineColor'] = this._colorControl;
		this._toolitemHandlers['.uno:Color'] = this._colorControl;
		this._toolitemHandlers['.uno:FillColor'] = this._colorControl;

		this._toolitemHandlers['.uno:FontworkAlignmentFloater'] = function () { return false; };
		this._toolitemHandlers['.uno:FontworkCharacterSpacingFloater'] = function () { return false; };
		this._toolitemHandlers['.uno:ExtrusionToggle'] = function () { return false; };
		this._toolitemHandlers['.uno:Grow'] = function () { return false; };
		this._toolitemHandlers['.uno:Shrink'] = function () { return false; };
		this._toolitemHandlers['.uno:StyleUpdateByExampleimg'] = function () { return false; };
		this._toolitemHandlers['.uno:StyleNewByExampleimg'] = function () { return false; };
		this._toolitemHandlers['.uno:LineEndStyle'] = function () { return false; };
		this._toolitemHandlers['.uno:SetOutline'] = function () { return false; };

		this._toolitemHandlers['.uno:FontworkShapeType'] = this._fontworkShapeControl;
		this._toolitemHandlers['SelectWidth'] = this._lineWidthControl;
		this._toolitemHandlers['.uno:XLineStyle'] = this._explorableToolItemHandler;
	},

	// make a class identifier from parent's id by walking up the tree
	_getParentId : function(it) {
		while (it.parent && !it.id)
			it = it.parent;
		if (it && it.id)
			return '-' + it.id;
		else
			return '';
	},

	// link each node to its parent, should do one recursive descent
	_parentize: function(data, parent) {
		if (data.parent)
			return;
		if (data.children !== undefined) {
			for (var idx in data.children) {
				this._parentize(data.children[idx], data);
			}
		}
		data.parent = parent;
	},

	baseSpinField: function(parentContainer, data, builder, customCallback) {
		var controls = {};
		if (data.label) {
			var fixedTextData = { text: data.label };
			builder._fixedtextControl(parentContainer, fixedTextData, builder);
		}

		var div = window.L.DomUtil.create('div', 'spinfieldcontainer', parentContainer);
		div.id = data.id;
		controls['container'] = div;
		var commandName = data.id  && data.id.startsWith('.uno:') ? data.id.substring('.uno:'.length) : data.id;
		if (commandName && commandName.length && app.LOUtil.existsIconForCommand(commandName, builder.map.getDocType())) {
			var image = window.L.DomUtil.create('img', 'spinfieldimage', div);
			var icon = (data.id === 'Transparency') ? app.LOUtil.getIconNameOfCommand('settransparency') : app.LOUtil.getIconNameOfCommand(data.id);
			app.LOUtil.setImage(image, icon, builder.map);
			icon.alt = '';
		}

		var spinfield = window.L.DomUtil.create('input', 'spinfield', div);
		spinfield.type = 'number';
		spinfield.onkeypress = builder._preventNonNumericalInput;
		spinfield.dir = document.documentElement.dir;
		controls['spinfield'] = spinfield;

		JSDialog.SetupA11yLabelForLabelableElement(parentContainer, spinfield, data, builder);

		if (data.unit) {
			var unit = window.L.DomUtil.create('span', 'spinfieldunit', div);
			unit.textContent = builder._unitToVisibleString(data.unit);
		}

		var controlsContainer = window.L.DomUtil.create('div', 'spinfieldcontrols', div);
		var minus = window.L.DomUtil.create('div', 'minus', controlsContainer);
		minus.textContent = '-';

		var plus = window.L.DomUtil.create('div', 'plus', controlsContainer);
		plus.textContent = '+';

		if (data.min != undefined)
			$(spinfield).attr('min', data.min);

		if (data.max != undefined)
			$(spinfield).attr('max', data.max);

		if (data.step != undefined)
			$(spinfield).attr('step', data.step);

		if (data.enabled === 'false' || data.enabled === false) {
			$(spinfield).attr('disabled', 'disabled');
			$(image).addClass('disabled');
		}

		if (data.readOnly === true)
			$(spinfield).attr('readOnly', 'true');

		if (data.hidden)
			$(spinfield).hide();

		plus.addEventListener('click', function() {
			var attrdisabled = $(spinfield).attr('disabled');
			if (attrdisabled !== 'disabled') {
				if (customCallback)
					customCallback('spinfield', 'plus', div, this.value, builder);
				else
					builder.callback('spinfield', 'plus', div, this.value, builder);
			}
		});

		minus.addEventListener('click', function() {
			var attrdisabled = $(spinfield).attr('disabled');
			if (attrdisabled !== 'disabled') {
				if (customCallback)
					customCallback('spinfield', 'minus', div, this.value, builder);
				else
					builder.callback('spinfield', 'minus', div, this.value, builder);
			}
		});

		return controls;
	},

	_preventNonNumericalInput: function(e) {
		e = e || window.event;
		var charCode = e.which === undefined ? e.keyCode : e.which;
		var charStr = String.fromCharCode(charCode);

		if (!charStr.match(/^[0-9.,]+$/) && charCode !== 13)
			e.preventDefault();
	},

	_swapControls: function(controls, indexA, indexB) {
		var tmp = controls[indexA];
		controls[indexA] = controls[indexB];
		controls[indexB] = tmp;
	},

	/// reorder widgets in case of vertical placement of labels and corresponding controls
	/// current implementation fits for 2 column views
	_gridHandler: function(parentContainer, data, builder) {
		var children = data.children;
		if (children) {
			var count = children.length;
			for (var i = 0; i < count - 2; i++) {
				if (children[i].type == 'fixedtext' && children[i+1].type == 'fixedtext') {
					builder._swapControls(children, i+1, i+2);
				}
			}
		}

		return true;
	},

	_frameHandler: function(parentContainer, data, builder) {
		// display explorable entry only if more than one widget inside
		if (data.children.length > 2) {
			data.text = builder._cleanText(data.children[0].text);
			var contentNode = data.children[1];
			builder._explorableEntry(parentContainer, data, contentNode, builder);
		} else {
			return true;
		}

		return false;
	},

	_listboxControl: function(parentContainer, data, builder) {
		// TODO: event listener in the next level...

		if (!data.entries || data.entries.length === 0)
			return false;

		builder._setIconAndNameForCombobox(data);

		if (data.id === 'FontBox')
			data.text = '';

		var title = data.text;
		var valueNode = null;
		var selectedEntryIsString = false;
		if (data.selectedEntries) {
			selectedEntryIsString = isNaN(parseInt(data.selectedEntries[0]));
			if (title && title.length) {
				var value = data.entries[data.selectedEntries[0]];
				valueNode = window.L.DomUtil.create('div', '', null);
				valueNode.textContent = value;
			} else if (selectedEntryIsString)
				title = builder._cleanText(data.selectedEntries[0]);
			else
				title = data.entries[data.selectedEntries[0]];
		}
		title = builder._cleanText(title);
		data.text = title;

		var entries = [];
		for (var index in data.entries) {
			var style = 'ui-combobox-text';
			if ((data.selectedEntries && index == data.selectedEntries[0])
				|| (data.selectedEntries && selectedEntryIsString && data.entries[index] === data.selectedEntries[0])
				|| data.entries[index] == title) {
				style += ' selected';
			}

			var entry = { type: 'comboboxentry', text: data.entries[index], pos: index, parent: data, style: style };
			entries.push(entry);
		}

		var contentNode = {type: 'container', children: entries};

		var iconPath = null;
		if (data.command)
			iconPath = app.LOUtil.getIconNameOfCommand(data.command);

		builder._explorableEntry(parentContainer, data, contentNode, builder, valueNode, iconPath);

		return false;
	},

	_checkboxControl: function(parentContainer, data, builder) {
		var div = window.L.DomUtil.createWithId('div', data.id, parentContainer);
		window.L.DomUtil.addClass(div, 'checkbutton');

		var checkboxLabel = window.L.DomUtil.create('label', '', div);
		checkboxLabel.textContent = builder._cleanText(data.text);
		checkboxLabel.htmlFor = data.id;
		var checkbox = window.L.DomUtil.createWithId('input', data.id, div);
		checkbox.type = 'checkbox';

		if (data.enabled === 'false' || data.enabled === false) {
			$(checkbox).attr('disabled', 'disabled');
			div.disabled = true;
			div.setAttribute('disabled', 'disabled');
		}

		JSDialog.SynchronizeDisabledState(div, [checkbox, checkboxLabel]);

		checkbox.addEventListener('change', function() {
			builder.callback('checkbox', 'change', div, this.checked, builder);
		});

		if (data.checked === true)
			$(checkbox).prop('checked', true);

		if (data.hidden)
			$(checkbox).hide();

		return false;
	},

	// TODO: use the same handler as desktop one
	_radiobuttonControl: function(parentContainer, data, builder) {
		var container = window.L.DomUtil.createWithId('div', data.id, parentContainer);
		window.L.DomUtil.addClass(container, 'radiobutton');
		window.L.DomUtil.addClass(container, builder.options.cssClass);

		var radiobuttonLabel = window.L.DomUtil.create('label', '', container);
		radiobuttonLabel.textContent = builder._cleanText(data.text);
		radiobuttonLabel.htmlFor = data.id;

		var radiobutton = window.L.DomUtil.create('input', '', container);
		radiobutton.type = 'radio';

		if (data.group)
			radiobutton.name = data.group;

		if (data.enabled === 'false' || data.enabled === false) {
			$(radiobutton).attr('disabled', 'disabled');
			container.disabled = true;
			container.setAttribute('disabled', 'disabled');
		}

		JSDialog.SynchronizeDisabledState(container, [radiobutton, radiobuttonLabel]);

		if (data.checked === 'true' || data.checked === true)
			$(radiobutton).prop('checked', true);

		radiobutton.addEventListener('change', function() {
			builder.callback('radiobutton', 'change', container, this.checked, builder);
		});

		if (data.hidden)
			$(radiobutton).hide();

		return false;
	},

	// TODO: remove
	_editControl: function(parentContainer, data, builder, callback) {
		var container = window.L.DomUtil.create('div', 'ui-edit-container ' + builder.options.cssClass, parentContainer);
		container.id = data.id;

		var edit = window.L.DomUtil.create('input', 'ui-edit ' + builder.options.cssClass, container);
		edit.value = builder._cleanText(data.text);
		edit.id = data.id + '-input';
		edit.dir = 'auto';
		if (data.password)
			edit.type = 'password';

		if (data.enabled === 'false' || data.enabled === false)
			$(edit).prop('disabled', true);

		// TODO: below is not true anymore
		// we still use non welded sidebar where don't have partial updates
		// keyup can be used only in welded dialogs
		edit.addEventListener('change', function() {
			if (callback)
				callback(this.value);
			else
				builder.callback('edit', 'change', container, this.value, builder);
		});

		edit.addEventListener('click', function(e) {
			e.stopPropagation();
		});

		if (data.hidden)
			$(edit).hide();

		if (data.placeholder)
			$(edit).attr('placeholder', data.placeholder);

		return false;
	},

	_fontworkShapeControl: function(parentContainer, data, builder) {
		var json = [
			{
				id: 'fontworkproperties',
				type: 'frame',
				children: [
					{
						text: _UNO('.uno:FontworkShapeType')
					},
					{
						id: 'fontworkshape',
						type: 'toolbox',
						children: [
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-plain-text'),
								command: '.uno:FontworkShapeType.fontwork-plain-text'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-wave'),
								command: '.uno:FontworkShapeType.fontwork-wave'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-inflate'),
								command: '.uno:FontworkShapeType.fontwork-inflate'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-stop'),
								command: '.uno:FontworkShapeType.fontwork-stop'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-curve-up'),
								command: '.uno:FontworkShapeType.fontwork-curve-up'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-curve-down'),
								command: '.uno:FontworkShapeType.fontwork-curve-down'
							},

							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-triangle-up'),
								command: '.uno:FontworkShapeType.fontwork-triangle-up'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-triangle-down'),
								command: '.uno:FontworkShapeType.fontwork-triangle-down'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-right'),
								command: '.uno:FontworkShapeType.fontwork-fade-right'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-left'),
								command: '.uno:FontworkShapeType.fontwork-fade-left'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-up'),
								command: '.uno:FontworkShapeType.fontwork-fade-up'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-down'),
								command: '.uno:FontworkShapeType.fontwork-fade-down'
							},

							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-slant-up'),
								command: '.uno:FontworkShapeType.fontwork-slant-up'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-slant-down'),
								command: '.uno:FontworkShapeType.fontwork-slant-down'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-up-and-right'),
								command: '.uno:FontworkShapeType.fontwork-fade-up-and-right'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-fade-up-and-left'),
								command: '.uno:FontworkShapeType.fontwork-fade-up-and-left'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-chevron-up'),
								command: '.uno:FontworkShapeType.fontwork-chevron-up'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-chevron-down'),
								command: '.uno:FontworkShapeType.fontwork-chevron-down'
							},

							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-up-curve'),
								command: '.uno:FontworkShapeType.fontwork-arch-up-curve'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-down-curve'),
								command: '.uno:FontworkShapeType.fontwork-arch-down-curve'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-left-curve'),
								command: '.uno:FontworkShapeType.fontwork-arch-left-curve'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-right-curve'),
								command: '.uno:FontworkShapeType.fontwork-arch-right-curve'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-circle-curve'),
								command: '.uno:FontworkShapeType.fontwork-circle-curve'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-open-circle-curve'),
								command: '.uno:FontworkShapeType.fontwork-open-circle-curve'
							},

							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-up-pour'),
								command: '.uno:FontworkShapeType.fontwork-arch-up-pour'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-down-pour'),
								command: '.uno:FontworkShapeType.fontwork-arch-down-pour'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-down-pour'),
								command: '.uno:FontworkShapeType.fontwork-arch-down-pour'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-arch-right-pour'),
								command: '.uno:FontworkShapeType.fontwork-arch-right-pour'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-circle-pour'),
								command: '.uno:FontworkShapeType.fontwork-circle-pour'
							},
							{
								type: 'toolitem',
								text: _UNO('.uno:FontworkShapeType.fontwork-open-circle-pour'),
								command: '.uno:FontworkShapeType.fontwork-open-circle-pour'
							},
						]
					}
				]
			}
		];

		builder.build(parentContainer, json);
	},

	_colorControl: function(parentContainer, data, builder) {
		var titleOverride = builder._getTitleForControlWithId(data.id);
		if (titleOverride)
			data.text = titleOverride;

		data.id = data.id ? data.id : (data.command ? data.command.replace('.uno:', '') : undefined);

		data.text = builder._cleanText(data.text);

		var valueNode =  window.L.DomUtil.create('div', 'color-sample-selected', null);
		var selectedColor = null;

		var updateFunction = function (titleSpan) {
			selectedColor = JSDialog.getCurrentColor(data, builder);
			valueNode.style.backgroundColor = selectedColor;
			if (titleSpan) {
				if (data.id === 'fillattr')
					data.text = _('Background Color');
				else if (data.id === 'fillattr2')
					data.text = _('Gradient Start');
				else if (data.id === 'fillattr3')
					data.text = _('Gradient End');
				titleSpan.textContent = data.text;
			}
		}.bind(this);

		updateFunction(null);

		var iconPath = app.LOUtil.getIconNameOfCommand(data.command);
		var noColorControl = (data.command !== '.uno:FontColor' && data.command !== '.uno:Color');
		var autoColorControl = (data.command === '.uno:FontColor' || data.command === '.uno:Color');

		var callback = function(color) {
			JSDialog.sendColorCommand(builder, data, color);
		};

		var colorPickerControl = new ColorPicker(
			valueNode,
			{
				selectedColor: selectedColor,
				noColorControl: noColorControl,
				autoColorControl: autoColorControl,
				selectionCallback: callback
			});
		builder._colorPickers.push(colorPickerControl);

		// color control panel
		var colorsContainer = colorPickerControl.getContainer();

		var contentNode = {type: 'container', children: [colorsContainer], onshow: window.L.bind(colorPickerControl.onShow, colorPickerControl)};

		builder._explorableEntry(parentContainer, data, contentNode, builder, valueNode, iconPath, updateFunction);
		return false;
	},

	_lineWidthControl: function(parentContainer, data, builder) {
		var values = [ 0.0,
			0.5,
			0.8,
			1.0,
			1.5,
			2.3,
			3.0,
			4.5,
			6.0 ];

		var currentWidth = parseInt(builder.map['stateChangeHandler'].getItemValue('.uno:LineWidth'));
		var currentWidthText = currentWidth ? String(parseFloat(currentWidth)/100.0) : '0.5';

		var lineData = { min: 0.5, max: 5, id: 'linewidth', text: currentWidthText, enabled: data.enabled, readOnly: true };

		var callbackFunction = function(objectType, eventType, object) {
			var newValue = 0;
			if (eventType == 'plus') {
				$(object).find('input').val(function(i, oldVal) {
					var index = 1;
					newValue = values[0];
					while (newValue <= oldVal && index < values.length)
						newValue = values[index++];
					return newValue;
				});
			} else if (eventType == 'minus') {
				$(object).find('input').val(function(i, oldVal) {
					if (oldVal > 0.0)
					{
						var index = values.length - 1;
						newValue = values[index];
						while (newValue >= oldVal && index > 0)
							newValue = values[index--];
					}
					else
						newValue = 0.0;
					return newValue;
				});
			}

			var command = '.uno:LineWidth';
			var params = {
				LineWidth: {
					type : 'long',
					value : (newValue * 100).toFixed(0)
				}
			};
			builder.map.sendUnoCommand(command, params);
		};

		JSDialog._spinfieldControl(parentContainer, lineData, builder, callbackFunction);
	},

	_panelHandler: function(parentContainer, data, builder) {
		// we want to show the contents always, hidden property decides if we collapse the panel
		if (data.children && data.children.length)
			data.children[0].visible = true;

		var content = data.children;

		var iconPath = null;
		var entryId = data.name ? data.name : data.id; // name is legacy panel name, id is vcl one
		if (entryId && entryId.length) {
			iconPath = app.LOUtil.getIconNameOfCommand(entryId);
		}

		builder._explorableEntry(parentContainer, data, content, builder, null, iconPath);

		if (data.hidden === true) {
			var control = parentContainer.querySelector('[id=\'' + data.id + '\']');
			if (control)
				window.L.DomUtil.addClass(control, 'hidden');
		}

		return false;
	},

	_mobilePopupContainer: function(parentContainer, data) {
		var container = window.L.DomUtil.create('div', 'mobile-popup-container', parentContainer);
		container.id = 'popup-' + data.id;
		return false;
	},

	_explorableToolItemHandler: function(parentContainer, data, builder) {
		data.text = builder._cleanText(data.text);

		var onShow = function() {
			builder.callback('toolbox', 'togglemenu', {id: data.parent.id}, data.command, builder);
		};

		var nodeId = data.command.indexOf('.uno:') === 0 ? data.command.substr('.uno:'.length) : data.command;
		var contentNode = {id: nodeId, type: 'mobile-popup-container', children: [], onshow: onShow};
		var iconPath = app.LOUtil.getIconNameOfCommand(data.command);

		builder._explorableEntry(parentContainer, data, contentNode, builder, null, iconPath);

		return false;
	},

	// apply needed modifications for mobile
	_modifySidebarNodes: function(data) {
		for (var i = data.length - 1; i >= 0; i--) {
			if (data[i].type === 'menubutton' &&
				(data[i].id === 'fillgrad1'
				|| data[i].id === 'fillgrad2'
				|| data[i].id === 'fillattr'
				|| data[i].id === 'fillattr2'
				|| data[i].id === 'fillattr3'
				|| data[i].id === 'LB_SHADOW_COLOR'
				|| data[i].id === 'LB_GLOW_COLOR')) {

				if (data[i].id === 'LB_GLOW_COLOR')
					data[i].command = '.uno:GlowColor';

				data[i].type = 'colorlistbox';
			} else if ((data[i].command === 'sidebargradient' && data[i].type === 'toolitem')
				|| (data[i].id === 'radiusglow' && data[i].type === 'fixedtext')
				|| (data[i].id === 'radiussoftedge' && data[i].type === 'fixedtext')) {
				data.splice(i, 1);
			}
		}
	},

	_countVisiblePanels: function(panels) {
		var count = 0;

		for (var i in panels)
			if (panels[i].type === 'panel' && (!panels[i].hidden || panels[i].hidden === false))
				count++;

		return count;
	},

	// some widgets we want to modify / change
	isHyperlinkTarget: function (builder, data) {
		return data.type === 'combobox' && (data.id === 'target' || data.id === 'receiver');
	},

	isFindReplaceComboox: function (builder, data) {
		return data.type === 'combobox' && (data.id === 'searchterm' || data.id === 'replaceterm');
	},

	shouldBeListbox: function (builder, data) {
		return data.type === 'combobox' &&
			(data.id === 'applystyle' ||
			data.id === 'fontnamecombobox' ||
			data.id === 'fontsizecombobox' ||
			data.id === 'fontsize' ||
			data.id === 'FontBox');
	},

	requiresOverwriting: function(builder, data) {
		if (builder.isHyperlinkTarget(builder, data) ||
			builder.isFindReplaceComboox(builder, data) ||
			builder.shouldBeListbox(builder, data))
			return true;

		return false;
	},

	overwriteHandler: function(parentContainer, data, builder) {
		if (builder.isHyperlinkTarget(builder, data) ||
			builder.isFindReplaceComboox(builder, data)) {
			// Replace combobox with edit
			var callback = function(value) {
				builder.callback('combobox', 'change', data, value, builder);
			};

			return builder._controlHandlers['edit'](parentContainer, data, builder, callback);
		} else if (builder.shouldBeListbox(builder, data)) {
			return builder._listboxControl(parentContainer, data, builder);
		}

		console.error('It seems widget doesn\'t require overwriting.');
	},

	build: function(parent, data) {
		this._modifySidebarNodes(data);

		for (var childIndex in data) {
			if (!data[childIndex])
				continue;

			var childData = data[childIndex];
			if (!childData)
				continue;

			this._handleResponses(childData, this);

			this._parentize(childData);
			var childType = childData.type;
			var processChildren = true;

			if ((childData.id === undefined || childData.id === '' || childData.id === null)
				&& (childType == 'checkbox' || childType == 'radiobutton')) {
				continue;
			}

			if (childType === 'deck' && childData.visible === false)
				continue;

			var childObject = parent;
			if (childData.dialogid) {
				var dialog = window.L.DomUtil.createWithId('div', childData.dialogid, childObject);
				childObject = dialog;
			}

			if (this.wizard._dialogid === 'ContentControlDialog' && childData.id !== '') {
				var div = window.L.DomUtil.createWithId('div', childData.id, childObject);
				childObject = div;
			}

			var handler = this._controlHandlers[childType];

			if (childData.children && this._countVisiblePanels(childData.children) == 1) {
				handler = this._controlHandlers['singlepanel'];
				processChildren = handler(childObject, childData.children, this);
			} else if (childData.children && this._countVisiblePanels(childData.children) == 2) {
				handler = this._controlHandlers['paneltabs'];
				processChildren = handler(childObject, childData.children, this);
			} else if (childType == 'tabcontrol' && childData.children.length > 3) {
				handler = this._controlHandlers['submenutabs'];
				handler(childObject, childData, this);
				this.postProcess(childObject, childData);
				this.build(childObject, childData.children);
			} else {
				if (handler) {
					if (this.requiresOverwriting(this, childData))
						processChildren = this.overwriteHandler(childObject, childData, this);
					else
						processChildren = handler(childObject, childData, this);
					this.postProcess(childObject, childData);
				} else
					window.app.console.warn('JSDialogBuilder: Unsupported control type: "' + childType + '"');

				if (processChildren && childData.children != undefined)
					this.build(childObject, childData.children);
				else if (childData.visible && (childData.visible === false || childData.visible === 'false')) {
					$('#' + childData.id).addClass('hidden-from-event');
				}
			}

			if ((childType === 'mobilewizard' || childType === 'dialog' || childType === 'messagebox' || childType === 'modelessdialog')
				&& childData.responses) {
				for (var i in childData.responses) {
					var buttonId = childData.responses[i].id;
					var response = childData.responses[i].response;
					var button = parent.querySelector('[id=\'' + buttonId + '\']');
					var isHelp = response === '-11' || response === -11 || buttonId === 'help';
					var isClose = response === '7' || response === 7;
					var isCancel = response === '0' || response === 0;
					if (button && (isHelp || isClose || isCancel))
						button.style.display = 'none';
				}
			}
		}
	}
});

window.L.control.mobileWizardBuilder = function (options) {
	var builder = new window.L.Control.MobileWizardBuilder(options);
	builder._setup(options);
	builder._overrideHandlers();
	builder._customizeOptions();
	return builder;
};
