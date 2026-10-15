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

// Review mode is editing with change recording on. One value names the situation the view is in.
enum ReviewModeState {
	// Core has not yet reported whether recording is on. Not reviewing.
	Unknown,
	// Not reviewing: the view is read-only, or it is editing with recording off.
	Off,
	// The user picked Review Mode while the view was read-only. Waiting for the edit permission.
	Requested,
	// The recording command was sent. Waiting for core to confirm that recording is on.
	Starting,
	// Editing with recording on.
	On,
}

// What caused a transition.
enum ReviewModeTrigger {
	// A choice in the mode dropdown.
	User,
	// The view became editable or read-only.
	Permission,
	// Core reported the .uno:TrackChanges state.
	CoreState,
}

// The states each state can move to.
const reviewModeTransitions: Record<ReviewModeState, ReviewModeState[]> = {
	[ReviewModeState.Unknown]: [
		ReviewModeState.Off,
		ReviewModeState.On,
		ReviewModeState.Starting,
		ReviewModeState.Requested,
	],
	[ReviewModeState.Off]: [
		ReviewModeState.Off,
		ReviewModeState.On,
		ReviewModeState.Starting,
		ReviewModeState.Requested,
	],
	[ReviewModeState.Requested]: [
		ReviewModeState.Requested,
		ReviewModeState.Starting,
		ReviewModeState.On,
		ReviewModeState.Off,
	],
	[ReviewModeState.Starting]: [
		ReviewModeState.Starting,
		ReviewModeState.On,
		ReviewModeState.Off,
	],
	[ReviewModeState.On]: [ReviewModeState.On, ReviewModeState.Off],
};

// Holds the review mode state and performs every change to it.
class ReviewModeMachine {
	private current: ReviewModeState = ReviewModeState.Unknown;
	private map: MapInterface;
	private onChanged: () => void;

	constructor(map: MapInterface, onChanged: () => void) {
		this.map = map;
		this.onChanged = onChanged;
	}

	get state(): ReviewModeState {
		return this.current;
	}

	// True while the view is in review mode, whether or not core has confirmed recording yet.
	isReviewing(): boolean {
		return (
			this.current === ReviewModeState.Starting ||
			this.current === ReviewModeState.On
		);
	}

	// The recording state core last reported for this view.
	isRecording(): boolean {
		const state =
			this.map['stateChangeHandler'].getItemValue('.uno:TrackChanges');
		return state === 'true' || state === true;
	}

	// The only place the state changes.
	transition(
		from: ReviewModeState,
		to: ReviewModeState,
		trigger: ReviewModeTrigger,
	): boolean {
		if (this.current !== from) {
			app.console.warn(
				'ReviewModeMachine: refused transition from ' +
					ReviewModeState[from] +
					' to ' +
					ReviewModeState[to] +
					', the current state is ' +
					ReviewModeState[this.current],
			);
			return false;
		}
		if (!reviewModeTransitions[from].includes(to)) {
			app.console.warn(
				'ReviewModeMachine: no transition from ' +
					ReviewModeState[from] +
					' to ' +
					ReviewModeState[to],
			);
			return false;
		}

		this.current = to;

		if (to === ReviewModeState.Starting) {
			// Sent again when the view becomes editable while still waiting, so a reloaded
			// view records too.
			this.map.sendUnoCommand('.uno:TrackChangesInThisView');
			if (from !== to) this.showReviewTab();
		} else if (to === ReviewModeState.On) {
			// A document that loads with recording on keeps its current tab, and so does a
			// view that becomes editable while recording is already on.
			const userTurnedRecordingOn =
				from === ReviewModeState.Off && trigger === ReviewModeTrigger.CoreState;
			if (
				trigger === ReviewModeTrigger.User ||
				from === ReviewModeState.Requested ||
				userTurnedRecordingOn
			)
				this.showReviewTab();
		} else if (
			to === ReviewModeState.Off &&
			trigger === ReviewModeTrigger.User &&
			(from === ReviewModeState.Starting || from === ReviewModeState.On)
		) {
			// Leaving review mode by choice stops this user's recording. The command takes an
			// argument because swriter.sdi gives TrackChangesInThisView none and core treats
			// that as on.
			this.map.sendUnoCommand('.uno:TrackChanges?TrackChanges:bool=false');
		}

		if (from !== to) this.onChanged();
		return true;
	}

	private showReviewTab(): void {
		this.map.uiManager.notebookbar?.selectTab('Review');
	}
}

class PermissionViewMode extends JSDialogComponent {
	viewModeContainer!: HTMLElement;
	readonlyIndicator!: HTMLElement;
	viewModeDropdown!: HTMLElement;
	shareAsContainer!: HTMLElement;
	notebookbarBuilder!: JSBuilder;
	reviewMode: ReviewModeMachine;

	constructor(map: MapInterface) {
		super(map, 'PermissionViewMode', 'permission');
		this.reviewMode = new ReviewModeMachine(
			map,
			this.updateReadonlyIndicator.bind(this),
		);
	}

	init(): void {
		this.setupContainer();
		if (this.viewModeContainer) {
			app.events.on('updatepermission', this.onUpdatePermission.bind(this));
			this.map.on('commandstatechanged', this.onCommandStateChanged, this);

			if (!this.map._docLoaded)
				this.map.on('docloaded', this.updateReadonlyIndicator, this);

			// may be not ready now and will apply update in above callback
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
		const isReview = this.reviewMode.isReviewing();
		const menu: Array<MenuDefinition> = [
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
				selected: this.map.isEditMode() && !isReview,
			},
		];
		if (this.map.getDocType() === 'text')
			menu.push({
				id: 'review',
				text: _('Review Mode'),
				type: 'action',
				selected: this.map.isEditMode() && isReview,
			});

		return {
			type: 'menubutton',
			id: 'viewModeDropdownButton',
			text: text,
			image: false, // Suppress image generation
			visible: visible, // JSDialogBuilder checks visible: false
			accessKey: 'VM',
			menu: menu,
		};
	}

	private _buildUI(): void {
		if (this.viewModeDropdown) {
			const data = [this._getMenuButtonJSON(_('Viewing'), false)];
			this.builder?.build(this.viewModeDropdown, data, false);
		}

		this._buildShareButton();
	}

	private _buildShareButton(): void {
		if (!this.shareAsContainer) return;

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
		const editing = currentPermission === 'edit';
		const state = this.reviewMode.state;
		const isReview = this.reviewMode.isReviewing();
		const requested = state === ReviewModeState.Requested;
		if (
			(mode === 'edit' && editing && !isReview) ||
			(mode === 'review' && editing && isReview) ||
			(mode === 'view' && currentPermission === 'readonly' && !requested)
		) {
			return;
		}

		this.map.uiManager.closeSnackbar();

		if (mode === 'review') {
			if (editing) {
				const next = this.reviewMode.isRecording()
					? ReviewModeState.On
					: ReviewModeState.Starting;
				this.reviewMode.transition(state, next, ReviewModeTrigger.User);
				return;
			}
			// Recording starts in onUpdatePermission once the view is editable.
			this.reviewMode.transition(
				state,
				ReviewModeState.Requested,
				ReviewModeTrigger.User,
			);
			this.switchToEditMode();
			return;
		}

		if (isReview || requested)
			this.reviewMode.transition(
				state,
				ReviewModeState.Off,
				ReviewModeTrigger.User,
			);

		if (mode === 'edit') {
			if (editing) return;
			this.switchToEditMode();
			return;
		}

		if (currentPermission !== 'readonly') this.map.setPermission('readonly');
	}

	private switchToEditMode(): void {
		if (typeof this.map._switchToEditMode === 'function')
			this.map._switchToEditMode();
		else this.map.setPermission('edit');
	}

	private _isStrictReadonly(): boolean {
		const isEditMode = this.map.isEditMode ? this.map.isEditMode() : false;
		const userCanWrite = this.map['wopi'] && this.map['wopi'].UserCanWrite;
		return !isEditMode && app.file.permission === 'readonly' && !userCanWrite;
	}

	onUpdatePermission(): void {
		const state = this.reviewMode.state;
		const trigger = ReviewModeTrigger.Permission;
		if (this.map.isEditMode() && this.map.getDocType() === 'text') {
			const recording = this.reviewMode.isRecording();
			if (state === ReviewModeState.Requested) {
				const next = recording ? ReviewModeState.On : ReviewModeState.Starting;
				this.reviewMode.transition(state, next, trigger);
			} else if (state === ReviewModeState.Starting) {
				this.reviewMode.transition(state, state, trigger);
			} else if (state !== ReviewModeState.On && recording) {
				this.reviewMode.transition(state, ReviewModeState.On, trigger);
			}
		} else if (
			state !== ReviewModeState.Off &&
			state !== ReviewModeState.Unknown
		) {
			this.reviewMode.transition(state, ReviewModeState.Off, trigger);
		}
		this.updateReadonlyIndicator();
	}

	onCommandStateChanged(e: any): void {
		if (e.commandName !== '.uno:TrackChanges') return;
		const recording = e.state === 'true';
		const editingText =
			this.map.isEditMode() && this.map.getDocType() === 'text';
		const state = this.reviewMode.state;
		const trigger = ReviewModeTrigger.CoreState;
		switch (state) {
			case ReviewModeState.Unknown:
				this.reviewMode.transition(
					state,
					recording && editingText ? ReviewModeState.On : ReviewModeState.Off,
					trigger,
				);
				break;
			case ReviewModeState.Off:
				if (recording && editingText)
					this.reviewMode.transition(state, ReviewModeState.On, trigger);
				break;
			case ReviewModeState.Starting:
				// A report of recording off while waiting is the state from before the
				// command and is ignored.
				if (recording)
					this.reviewMode.transition(state, ReviewModeState.On, trigger);
				break;
			case ReviewModeState.On:
				if (!recording)
					this.reviewMode.transition(state, ReviewModeState.Off, trigger);
				break;
			case ReviewModeState.Requested:
				// The view is read-only, so the report is picked up once it is editable.
				break;
		}
	}

	updateReadonlyIndicator(): void {
		if (!this.map._docLoaded) return;

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
			dropdownText = this.reviewMode.isReviewing()
				? _('Reviewing')
				: _('Editing');
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

		this.updateShareButton(showReadonly);
	}

	updateShareButton(showReadonly: boolean = this._isStrictReadonly()): void {
		if (!this.map._docLoaded) return;

		if ((this.map.wopi as any).EnableShare) {
			const isCompact = this.map.uiManager.getCurrentMode() === 'classic';
			if (
				isCompact &&
				this.shareAsContainer &&
				!this.shareAsContainer.querySelector('#shareas')
			)
				this._buildShareButton();

			if (this.builder && this.viewModeContainer) {
				this.notebookbarBuilder.executeAction(this.shareAsContainer, {
					control_id: 'shareas',
					action_type: showReadonly || isCompact ? 'show' : 'hide',
				} as ActionData);
			}
		}
	}
}
