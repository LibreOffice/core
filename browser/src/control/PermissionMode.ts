/* -*- js-indent-level: 8 -*- */
/**
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * PermissionMode.ts - Manages the Permission and View Mode UI
 *
 * This file controls the display and interaction of the "Viewing" vs "Editing" mode
 * dropdown and the "Read-only" indicator in the UI. It handles the logic for
 * switching between edit and readonly permissions based on user interaction and
 * document state.
 *
 * ### Key Features:
 * - **Permission Management**: Switches document permission between 'edit' and 'readonly'.
 * - **UI State Handling**: Dynamically shows a "Read-only" label or a "Thinking/Editing" dropdown.
 * - **Dropdown Integration**: Uses JSDialogBuilder to create and manage the mode selection menu.
 * - **Event Listeners**: Reacts to 'updatepermission' events to refresh the UI state.
 *
 * ### Guidelines:
 * - Use `updateReadonlyIndicator()` to refresh the UI based on current map permissions.
 * - `handleModeSelection(mode)` triggers the actual permission change logic.
 * - Ensure `_setupDropdown()` is called to initialize the JSDialog components.
 */

declare var JSDialog: any;

class PermissionViewMode extends JSDialogComponent {
	viewModeContainer!: HTMLElement;
	readonlyIndicator!: HTMLElement;
	viewModeDropdown!: HTMLElement;
	shareAsContainer!: HTMLElement;
	notebookbarBuilder!: JSBuilder;

	constructor(map: MapInterface) {
		super(map, 'PermissionViewMode', 'permission');
	}

	init(): void {
		this.setupContainer();
		if (this.viewModeContainer) {
			app.events.on(
				'updatepermission',
				this.updateReadonlyIndicator.bind(this),
			);
			this.updateReadonlyIndicator();
		}
	}

	protected createBuilder(): void {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			cssClass: 'jsdialog',
			callback: this.onJSDialogEvent.bind(this),
			noLabelsForUnoButtons: false,
		});

		this.notebookbarBuilder = new window.L.control.notebookbarBuilder({
			mobileWizard: this,
			map: this.map,
			cssClass: 'notebookbar',
			suffix: 'notebookbar',
		});
	}

	protected setupContainer(parentContainer?: HTMLElement): void {
		this.viewModeContainer = window.L.DomUtil.get('viewMode');
		if (!this.viewModeContainer) return;

		// Requested manual DOM structure for readonly mode indicator
		this.readonlyIndicator = window.L.DomUtil.create(
			'div',
			'unotoolbutton notebookbar ui-content unospan readonly inline hidden',
			this.viewModeContainer,
		);
		this.readonlyIndicator.id = 'readonlyMode';
		this.readonlyIndicator.setAttribute('tabindex', '-1');
		this.readonlyIndicator.setAttribute('data-cooltip', _('Permission Mode'));

		const label = window.L.DomUtil.create(
			'span',
			'ui-content unolabel',
			this.readonlyIndicator,
		);
		label.textContent = _('Read-only');

		// Dropdown container for JSDialog-based implementation
		this.viewModeDropdown = window.L.DomUtil.create(
			'div',
			'viewModeDropdown notebookbar',
			this.viewModeContainer,
		);
		this.viewModeDropdown.id = 'viewModeDropdown';
		const separator = window.L.DomUtil.get('closebuttonwrapperseparator');
		this.shareAsContainer = window.L.DomUtil.create(
			'div',
			'shareAsContainer notebookbar',
		);
		this.shareAsContainer.id = 'shareAsContainer';
		separator.parentNode.insertBefore(this.shareAsContainer, separator);

		this.createBuilder();
		this._buildUI();
	}

	private _getMenuButtonJSON(
		text: string,
		visible: boolean,
	): MenuButtonWidgetJSON {
		return {
			type: 'menubutton',
			id: 'viewModeDropdownButton',
			text: text,
			image: false, // Suppress image generation
			visible: visible, // JSDialogBuilder checks visible: false
			accessKey: 'VM',
			menu: [
				{
					id: 'view',
					text: _('Viewing Mode'),
					type: 'action',
					selected: !this.map.isEditMode(),
				},
				{
					id: 'edit',
					text: _('Editing Mode'),
					type: 'action',
					selected: this.map.isEditMode(),
				},
			],
		};
	}

	private _buildUI(): void {
		if (this.viewModeDropdown) {
			const data = [this._getMenuButtonJSON(_('Viewing'), false)];
			this.builder?.build(this.viewModeDropdown, data, false);
		}

		if (this.shareAsContainer) {
			const data = [
				{
					type: 'customtoolitem',
					id: 'shareas',
					text: _('Share'),
					command: 'shareas',
					inlineLabel: true,
					accessibility: {
						focusBack: false,
						combination: 'ZS',
						de: null as any,
					},
					tabIndex: 0,
					visible: false,
				},
			];
			this.notebookbarBuilder?.build(this.shareAsContainer, data, false);
		}
	}

	onJSDialogEvent(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		if (eventType === 'select') {
			this.handleModeSelection(data);
		}
	}

	handleModeSelection(mode: string): void {
		const currentPermission = this.map._permission;
		if (
			(mode === 'edit' && currentPermission === 'edit') ||
			(mode === 'view' && currentPermission === 'readonly')
		) {
			return;
		}

		this.map.uiManager.closeSnackbar();

		if (mode === 'edit') {
			if (typeof this.map._switchToEditMode === 'function')
				this.map._switchToEditMode();
			else this.map.setPermission('edit');
			return;
		}

		this.map.setPermission('readonly');
	}

	updateReadonlyIndicator(): void {
		// Safety check for map methods which might be missing in some contexts
		const isEditMode = this.map.isEditMode ? this.map.isEditMode() : false;
		const shouldStartReadOnly = this.map._shouldStartReadOnly
			? this.map._shouldStartReadOnly()
			: false;

		let showReadonly = false;
		let showDropdown = false;
		let dropdownText = '';

		if (isEditMode) {
			showDropdown = true;
			dropdownText = _('Editing');
		} else if (app.file.permission === 'edit' && shouldStartReadOnly) {
			showDropdown = true;
			dropdownText = _('Viewing');
		} else if (app.file.permission === 'readonly') {
			const userCanWrite = this.map['wopi'] && this.map['wopi'].UserCanWrite;

			if (userCanWrite) {
				showDropdown = true;
				dropdownText = _('Viewing');
			} else {
				showReadonly = true;
			}
		} else {
			showDropdown = true;
			dropdownText = _('Viewing');
		}

		if (this.builder && this.viewModeContainer) {
			this.builder.executeAction(this.viewModeContainer, {
				control_id: 'readonlyMode',
				action_type: showReadonly ? 'show' : 'hide',
			} as ActionData);

			this.builder.executeAction(this.viewModeContainer, {
				control_id: 'viewModeDropdownButton',
				action_type: showDropdown ? 'show' : 'hide',
			} as ActionData);

			if (showDropdown) {
				this.builder.updateWidget(
					this.viewModeContainer,
					this._getMenuButtonJSON(dropdownText, true),
				);
			}
		}

		if (this.map && (this.map.wopi as any).EnableShare) {
			if (this.builder && this.viewModeContainer) {
				this.notebookbarBuilder.executeAction(this.shareAsContainer, {
					control_id: 'shareas',
					action_type: showReadonly ? 'show' : 'hide',
				} as ActionData);
			}
		}
	}
}
