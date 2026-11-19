/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Notebookbar.ImpressAnimationTab.ts
 */

class ImpressAnimationTab implements NotebookbarTab {
	public getName(): string {
		return 'Animation';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: 'Animation-tab-label',
			text: _('Animation'),
			name: this.getName(),
			accessibility: {
				focusBack: false,
				combination: 'Z',
				de: null,
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	/* ids have to match animation pane ids from the .ui in the core */
	public getContent(): NotebookbarTabContent {
		const content = [
			{
				id: 'animations-group',
				type: 'overflowgroup',
				nofold: true,
				name: _('Applied Animations'),
				children: [
					{
						id: 'custom_animation_list',
						type: 'treelistbox',
						entries: [] as any,
					} as TreeWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				id: 'animation-addremove-group',
				type: 'grid',
				rows: 2,
				cols: 2,
				children: [
					{
						id: 'move_up',
						type: 'pushbutton',
						text: _('Move Up'),
						image: 'images/lc_moveup.svg',
						top: '0',
						left: '0',
					} as PushButtonWidget,
					{
						id: 'add_effect',
						type: 'pushbutton',
						text: _('Add'),
						image: 'images/lc_insertsheet.svg',
						top: '0',
						left: '1',
					} as PushButtonWidget,
					{
						id: 'move_down',
						type: 'pushbutton',
						text: _('Move Down'),
						image: 'images/lc_movedown.svg',
						top: '1',
						left: '0',
					} as PushButtonWidget,
					{
						id: 'remove_effect',
						type: 'pushbutton',
						text: _('Remove'),
						image: 'images/lc_delete.svg',
						top: '1',
						left: '1',
					} as PushButtonWidget,
				],
			} as GridWidgetJSON,
			{
				id: 'animations-group',
				type: 'overflowgroup',
				nofold: true,
				name: '',
				children: [
					{
						id: 'animation-new-group',
						type: 'grid',
						rows: 2,
						cols: 2,
						children: [
							{
								id: 'categorylabel',
								type: 'fixedtext',
								text: _('Category'),
								top: '0',
								left: '0',
							} as TextWidget,
							{
								id: 'categorylb',
								type: 'listbox',
								text: '',
								entries: [],
								top: '0',
								left: '1',
							} as ListBoxWidget,
							{
								id: 'start_effect',
								type: 'fixedtext',
								text: _('Start'),
								top: '1',
								left: '0',
							} as TextWidget,
							{
								id: 'start_effect_list',
								type: 'listbox',
								text: '',
								entries: [],
								top: '1',
								left: '1',
							} as ListBoxWidget,
						],
					} as GridWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				id: 'animations-group',
				type: 'overflowgroup',
				nofold: true,
				name: _('Effect'),
				children: [
					{
						id: 'effect_list',
						type: 'listbox',
						text: '',
						entries: [],
					} as ListBoxWidget,
				],
			} as OverflowGroupWidgetJSON,
		];

		return content as NotebookbarTabContent;
	}
}

JSDialog.ImpressAnimationTab = new ImpressAnimationTab();
