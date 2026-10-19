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
 * Notebookbar.ImpressTransitionTab.ts
 */

class ImpressTransitionTab implements NotebookbarTab {
	public getName(): string {
		return 'Transition';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: 'Transition-tab-label',
			text: _('Transition'),
			name: this.getName(),
			accessibility: {
				focusBack: false,
				combination: 'A',
				de: null,
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	/* ids have to match transition pane ids from the .ui in the core */
	public getContent(): NotebookbarTabContent {
		const content = [
			{
				id: 'transitions-group',
				type: 'overflowgroup',
				name: _('Transitions'),
				icon: 'lc_slidechangewindow.svg',
				children: [
					{
						id: 'transitions_icons-iconview-list',
						type: 'iconviewlist',
						accessibility: { focusBack: false, combination: 'TI', de: null },
						children: [
							{
								id: 'transitions_icons',
								type: 'iconview',
								entries: [...Array(29).keys()].map((n: number) => {
									return { ondemand: true, selected: false, row: n };
								}),
							} as IconViewJSON,
						],
					},
				],
			} as OverflowGroupWidgetJSON,
			{
				id: 'transitions-icons-separator',
				type: 'separator',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				id: 'transition-modify-group',
				type: 'grid',
				rows: 2,
				cols: 2,
				children: [
					{
						id: 'variant_label',
						type: 'fixedtext',
						text: _('Variant'),
						top: '0',
						left: '0',
					} as TextWidget,
					{
						id: 'variant_list',
						type: 'listbox',
						text: '',
						entries: [],
						top: '0',
						left: '1',
						labelledBy: 'variant_label',
					} as ListBoxWidget,
					{
						id: 'duration_label',
						type: 'fixedtext',
						text: _('Duration'),
						top: '1',
						left: '0',
					} as TextWidget,
					{
						id: 'transition_duration',
						type: 'formattedfield',
						text: '',
						top: '1',
						left: '1',
						labelledBy: 'duration_label',
					},
				],
			} as GridWidgetJSON,
			{
				id: 'transition-modify-separator',
				type: 'separator',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				id: 'Transition-advance-group',
				type: 'grid',
				rows: 2,
				cols: 2,
				children: [
					{
						id: 'rb_mouse_click',
						type: 'radiobutton',
						text: _('On mouse click'),
						top: '0',
						left: '0',
						width: '2',
					} as RadioButtonWidget,
					{
						id: 'rb_auto_after',
						type: 'radiobutton',
						text: _('After'),
						top: '1',
						left: '0',
					} as RadioButtonWidget,
					{
						id: 'auto_after_value',
						type: 'formattedfield',
						text: '',
						top: '1',
						left: '1',
					},
				],
			} as GridWidgetJSON,
			{
				id: 'transition-advance-separator',
				type: 'separator',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				id: 'apply_to_all',
				type: 'pushbutton',
				text: _('Apply to All Slides'),
				image: app.LOUtil.getImageURL('lc_applytoallslides.svg'),
			},
		];

		return content as NotebookbarTabContent;
	}
}

JSDialog.ImpressTransitionTab = new ImpressTransitionTab();
