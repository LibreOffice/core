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

function _menubuttonControl(
	parentContainer: Element,
	data: MenuButtonWidgetJSON,
	builder: JSBuilder,
): UnoToolButtonControl | false {
	const jsdialog: JSDialogGlobal = JSDialog;
	let ids: Array<string>;
	let menuId: string | null = null;

	if (data.id.includes(':')) {
		ids = data.id.split(':');
		menuId = ids[1];
		data.id = ids[0];
	} else if (data.id.includes('-')) {
		ids = data.id.split('-');
		if (ids.length === 2) {
			menuId = ids[1];
			data.id = ids[0];
		} else if (!data.menu) {
			console.error('Menubutton without correct menu id: ' + data.id);
		}
	} else menuId = data.id;

	// import menu
	if (data.menu) {
		// command is needed to generate image
		if (!data.command) data.command = data.id;

		menuId = data.id + '-menu';
		const builtMenu: Array<MenuDefinition> = Object.values(data.menu);
		builder._menus.set(menuId, builtMenu);
	}

	const menuEntries = menuId ? builder._menus.get(menuId) : null;

	if (menuEntries) {
		const noLabels = builder.options.noLabelsForUnoButtons;
		builder.options.noLabelsForUnoButtons = data.noLabel ? data.noLabel : false;

		// command is needed to generate image
		if (!data.command) data.command = menuId as string;

		const isSplitButton = !!data.applyCallback;
		const options = {
			hasDropdownArrow: menuEntries.length > 1 || isSplitButton,
		};
		const control = builder._unoToolButton(
			parentContainer,
			data,
			builder,
			options,
		);

		if (!window.L.DomUtil.hasClass(control.container, 'selected')) {
			control.button.removeAttribute('aria-pressed');
		}
		// can be function or string with command identifier
		const applyCallback =
			typeof data.applyCallback === 'function'
				? data.applyCallback
				: () => {
						const command = data.applyCallback as string;
						if (command.indexOf('.uno:') === 0) app.map.sendUnoCommand(command);
						else app.dispatcher.dispatch(command);
					};

		if (menuEntries.length == 0) {
			control.container.setAttribute('disabled', 'true');
		}

		$(control.container).addClass(
			'menubutton' + (isSplitButton ? ' splitbutton' : ''),
		);

		$(control.button).unbind('click');
		$(control.label).unbind('click');

		const dropdownId = data.id;
		const clickFunction = function () {
			if (control.container.hasAttribute('disabled')) return;

			const callback = function (
				objectType: string | null,
				eventType: string,
				object: unknown,
				data: unknown,
				entry: MenuDefinition | JSBuilder | null,
			): boolean {
				const menuEntry = entry as MenuDefinition | null;
				if (
					(eventType === 'selected' && menuEntry && menuEntry.items) ||
					eventType === 'showsubmenu'
				) {
					return true;
				} else if (eventType === 'selected' && menuEntry && menuEntry.uno) {
					const uno =
						menuEntry.uno.indexOf('.uno:') === 0
							? menuEntry.uno
							: '.uno:' + menuEntry.uno;
					builder.map.sendUnoCommand(uno);
					jsdialog.CloseDropdown(dropdownId);
					return true;
				} else if (eventType === 'selected' && menuEntry && menuEntry.action) {
					app.dispatcher.dispatch(menuEntry.action, menuEntry);
					const opensExternal =
						menuEntry.action.startsWith('exportas-') ||
						menuEntry.action.startsWith('saveas-');
					jsdialog.CloseDropdown(dropdownId, opensExternal);
					return true;
				} else if (eventType === 'selected' && menuEntry && menuEntry.id) {
					builder.callback(
						'menubutton',
						'select',
						{ id: dropdownId },
						menuEntry.id,
						builder,
					);
					jsdialog.CloseDropdown(dropdownId);
					return true;
				} else /* note: entry can be a builder instance as in regular JSDialog callback */ {
					// custom popup - execute generic action
					builder.callback(
						objectType as string,
						eventType,
						object,
						data,
						builder,
					);
					jsdialog.CloseDropdown(dropdownId);
					return true;
				}
			};

			// refetch to apply dynamic changes
			let freshMenu = builder._menus.get(
				menuId as string,
			) as Array<MenuDefinition>;
			if (freshMenu.length && freshMenu[0].type === 'colorpicker') {
				// make copy and fill with information to identify color command
				freshMenu = JSON.parse(JSON.stringify(freshMenu));
				freshMenu[0].command = data.command;
				freshMenu[0].id = data.id;
			}

			if (freshMenu.length === 1 && !isSplitButton) {
				callback(null, 'selected', null, null, freshMenu[0]);
			} else {
				jsdialog.OpenDropdown(
					dropdownId,
					control.container,
					freshMenu,
					callback,
					'',
					false,
					false,
					true,
				);
			}
		};

		// make it possible to setup separate callbacks for split button
		if (isSplitButton) {
			jsdialog.AddOnClick(control.button, applyCallback);
		} else {
			jsdialog.AddOnClick(control.button, clickFunction);
			// Only wire the label separately when it isn't a descendant of
			// the button - otherwise a click on the label fires both
			// handlers (its own and the button's via bubbling), and any
			// toggle-style action ends up firing twice.
			if (control.label && !control.button.contains(control.label))
				jsdialog.AddOnClick(control.label, clickFunction);
		}

		if (control.arrow) jsdialog.AddOnClick(control.arrow, clickFunction);

		builder._preventDocumentLosingFocusOnClick(control.container);

		builder.options.noLabelsForUnoButtons = noLabels;

		return control;
	} else if (data.text !== undefined || data.image) {
		const button = window.L.DomUtil.create(
			'button',
			'menubutton ' + builder.options.cssClass,
			parentContainer,
		) as HTMLButtonElement;
		button.id = data.id;
		button.title = data.text as string;
		button.setAttribute('aria-haspopup', 'true');

		jsdialog.SetupA11yLabelForNonLabelableElement(button, data, builder);

		if (data.image) {
			const image = window.L.DomUtil.create(
				'img',
				'',
				button,
			) as HTMLImageElement;
			image.src = data.image as string;
			image.setAttribute('alt', '');
		}
		const label = window.L.DomUtil.create(
			'span',
			'unolabel',
			button,
		) as HTMLElement;
		label.innerText = data.text ? data.text : '';
		window.L.DomUtil.create('i', 'arrow', button);

		$(button).click(function () {
			if (!button.hasAttribute('disabled')) {
				builder.callback('menubutton', 'toggle', button, undefined, builder);
			}
		});

		const enabled = Boolean(data.enabled);
		button.setAttribute('aria-disabled', String(!enabled));

		if (data.enabled === false) button.disabled = true;

		if (data.visible === false) button.classList.add('hidden');
	} else {
		window.app.console.warn('Not found menu "' + menuId + '"');
	}

	return false;
}

JSDialog.menubuttonControl = function (
	parentContainer: Element,
	data: MenuButtonWidgetJSON,
	builder: JSBuilder,
): UnoToolButtonControl | false {
	const buildInnerData = _menubuttonControl(parentContainer, data, builder);
	return buildInnerData;
};
