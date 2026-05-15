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
 * JSDialog.MenuButton - button which can trigger some action or open a menu
 *
 * Example JSON:
 * {
 *     id: 'id:MenuId', (where MenuId is custom menu id, menu is stored in the builder)
 *     type: 'menubutton',
 *     text: 'Label',
 *     image: 'base64 encoded icon',
 *     command: '.uno:Command',
 *     enabled: false
 * }
 */

/* global JSDialog $ app */

function _menubuttonControl (parentContainer, data, builder) {
	var ids;
	var menuId = null;

	if (data.id.includes(':')) {
		ids = data.id.split(':');
		menuId = ids[1];
		data.id = ids[0];
	}
	else if (data.id.includes('-')) {
		ids = data.id.split('-');
		if (ids.length === 2) {
			menuId = ids[1];
			data.id = ids[0];
		} else if (!data.menu){
			console.error('Menubutton without correct menu id: ' + data.id);
		}
	}
	else
		menuId = data.id;

	// import menu
	if (data.menu) {
		// command is needed to generate image
		if (!data.command)
			data.command = data.id;

		menuId = data.id + '-menu';
		var builtMenu = [];
		for (var i in data.menu)
			builtMenu.push(data.menu[i]);
		builder._menus.set(menuId, builtMenu);
	}

	var menuEntries = menuId ? builder._menus.get(menuId) : null;

	if (menuEntries) {
		var noLabels = builder.options.noLabelsForUnoButtons;
		builder.options.noLabelsForUnoButtons = data.noLabel ? data.noLabel : false;

		// command is needed to generate image
		if (!data.command)
			data.command = menuId;

		var options = {hasDropdownArrow: menuEntries.length > 1};
		var control = builder._unoToolButton(parentContainer, data, builder, options);

		if (!window.L.DomUtil.hasClass(control.container, 'selected')) {
			control.button.removeAttribute('aria-pressed');
		}

		var isSplitButton = !!data.applyCallback;
		// can be function or string with command identifier
		const applyCallback =
			(typeof data.applyCallback === 'function') ?
				data.applyCallback
				: () => {
					if (data.applyCallback.indexOf('.uno:') === 0)
						app.map.sendUnoCommand(data.applyCallback);
					else
						app.dispatcher.dispatch(data.applyCallback);
				};

		if (menuEntries.length == 0) {
			control.container.setAttribute('disabled', true);
		}

		$(control.container).addClass('menubutton' + (isSplitButton ? ' splitbutton' : ''));

		$(control.button).unbind('click');
		$(control.label).unbind('click');

		var dropdownId = data.id;
		var clickFunction = function () {
			if (control.container.hasAttribute('disabled'))
				return;

			var callback = function(objectType, eventType, object, data, entry /* : MenuDefinition | JSBuilder */) {
				if ((eventType === 'selected' && entry && entry.items) || eventType === 'showsubmenu') {
					return true;
				} else if (eventType === 'selected' && entry && entry.uno) {
					var uno = (entry.uno.indexOf('.uno:') === 0) ? entry.uno : '.uno:' + entry.uno;
					builder.map.sendUnoCommand(uno);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				} else if (eventType === 'selected' && entry && entry.action) {
					app.dispatcher.dispatch(entry.action, entry);
					const opensExternal = entry.action.startsWith('exportas-') || entry.action.startsWith('saveas-');
					JSDialog.CloseDropdown(dropdownId, opensExternal);
					return true;
				} else if (eventType === 'selected' && entry && entry.id) {
					builder.callback('menubutton', 'select', {id: dropdownId}, entry.id, builder);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				} else /* note: entry can be a builder instance as in regular JSDialog callback */ {
					// custom popup - execute generic action
					builder.callback(objectType, eventType, object, data, builder);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				}
			};

			var freshMenu = builder._menus.get(menuId); // refetch to apply dynamic changes
			if (freshMenu.length && freshMenu[0].type === 'colorpicker') {
				// make copy and fill with information to identify color command
				freshMenu = JSON.parse(JSON.stringify(freshMenu));
				freshMenu[0].command = data.command;
				freshMenu[0].id = data.id;
			}

			if (freshMenu.length === 1) {
				callback(null, 'selected', null, null, freshMenu[0]);
			} else {
				JSDialog.OpenDropdown(dropdownId, control.container, freshMenu, callback);
			}
		};

		// make it possible to setup separate callbacks for split button
		if (isSplitButton) {
			JSDialog.AddOnClick(control.button, applyCallback);
		} else {
			JSDialog.AddOnClick(control.button, clickFunction);
			// Only wire the label separately when it isn't a descendant of
			// the button - otherwise a click on the label fires both
			// handlers (its own and the button's via bubbling), and any
			// toggle-style action ends up firing twice.
			if (control.label && !control.button.contains(control.label))
				JSDialog.AddOnClick(control.label, clickFunction);
		}

		if (control.arrow)
			JSDialog.AddOnClick(control.arrow, clickFunction);

		builder._preventDocumentLosingFocusOnClick(control.container);

		builder.options.noLabelsForUnoButtons = noLabels;

		return control;
	} else if (data.text !== undefined || data.image) {
		var button = window.L.DomUtil.create('button', 'menubutton ' + builder.options.cssClass, parentContainer);
		button.id = data.id;
		button.title = data.text;
		button.setAttribute('aria-haspopup', true);

		JSDialog.SetupA11yLabelForNonLabelableElement(button, data, builder);

		if (data.image) {
			var image = window.L.DomUtil.create('img', '', button);
			image.src = data.image;
			image.setAttribute('alt', '');
		}
		var label = window.L.DomUtil.create('span', 'unolabel', button);
		label.innerText = data.text ? data.text : '';
		window.L.DomUtil.create('i', 'arrow', button);

		$(button).click(function () {
			if (!button.hasAttribute('disabled')) {
				builder.callback('menubutton', 'toggle', button, undefined, builder);
			}
		});

		var enabled = Boolean(data.enabled);
		button.setAttribute('aria-disabled', !enabled);

		if (data.enabled === false)
			button.disabled = true;

		if (data.visible === false)
			button.classList.add('hidden');

	} else {
		window.app.console.warn('Not found menu "' + menuId + '"');
	}

	return false;
}

JSDialog.menubuttonControl = function (parentContainer, data, builder) {
	var buildInnerData = _menubuttonControl(parentContainer, data, builder);
	return buildInnerData;
};
