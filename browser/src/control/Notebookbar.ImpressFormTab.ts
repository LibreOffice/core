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
 * Notebookbar.ImpressFormTab.ts
 *
 * The tab of the form controls of a presentation: the design mode, the controls to insert and
 * the properties of the controls and of their form.
 */

// A small button of the tab. The text is the label of the command. The icon is the file name of
// the icon, when it is not the one named after the command.
interface FormTabItem {
	id: string;
	text: string;
	command: string;
	combination: string;
	icon?: string;
}

// A group of the buttons that insert form controls, with the icon shown when the group is
// folded.
interface FormTabGroup {
	id: string;
	name: string;
	icon: string;
	combination: string;
	items: FormTabItem[];
}

class ImpressFormTab implements NotebookbarTab {
	public getName(): string {
		return 'Form';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: 'Form-tab-label',
			text: _('Form'),
			name: this.getName(),
			keepSelected: true,
			accessibility: {
				focusBack: false,
				combination: 'CO',
				de: null,
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	public getContent(): NotebookbarTabContent {
		const content = [
			this.getModeGroup(),
			{
				id: 'form-mode-break',
				type: 'separator',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			...this.getInsertGroups().map((group: FormTabGroup) =>
				this.getGroup(group),
			),
			{
				id: 'form-insert-break',
				type: 'separator',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			this.getPropertiesGroup(),
		];

		return content as NotebookbarTabContent;
	}

	// The switch of the design mode, and the two settings of the document about the mode it
	// opens in and the control that takes the focus when it opens.
	private getModeGroup(): any {
		return {
			type: 'overflowgroup',
			id: 'form-mode',
			name: _('Design'),
			icon: 'lc_switchcontroldesignmode.svg',
			accessibility: { focusBack: true, combination: 'FD', de: null },
			children: [
				{
					id: 'form-design-mode',
					type: 'bigtoolitem',
					text: _UNO('.uno:SwitchControlDesignMode'),
					command: '.uno:SwitchControlDesignMode',
					accessibility: { focusBack: true, combination: 'DM', de: null },
				},
				this.getColumn([
					{
						id: 'form-open-in-design-mode',
						text: _UNO('.uno:OpenReadOnly'),
						command: '.uno:OpenReadOnly',
						combination: 'OD',
					},
					{
						id: 'form-automatic-focus',
						text: _UNO('.uno:AutoControlFocus'),
						command: '.uno:AutoControlFocus',
						combination: 'AF',
					},
				]),
			],
		};
	}

	private getPropertiesGroup(): any {
		return {
			type: 'overflowgroup',
			id: 'form-properties',
			name: _('Properties'),
			icon: 'lc_controlproperties.svg',
			accessibility: { focusBack: true, combination: 'FP', de: null },
			children: [
				{
					id: 'form-control-properties',
					type: 'bigtoolitem',
					text: _UNO('.uno:ControlProperties'),
					command: '.uno:ControlProperties',
					accessibility: { focusBack: true, combination: 'CP', de: null },
				},
				this.getColumn([
					{
						id: 'form-form-properties',
						text: _UNO('.uno:FormProperties'),
						command: '.uno:FormProperties',
						combination: 'MP',
					},
					{
						id: 'form-activation-order',
						text: _UNO('.uno:TabDialog'),
						command: '.uno:TabDialog',
						combination: 'AO',
					},
				]),
			],
		};
	}

	// The controls the tab inserts: the fields a value is typed into, and the other controls.
	// The table control and the navigation bar work on the records of a database the form is
	// bound to, so the tab leaves them out.
	private getInsertGroups(): FormTabGroup[] {
		return [
			{
				id: 'form-controls',
				name: _('Controls'),
				icon: 'lc_pushbutton.svg',
				combination: 'FC',
				items: [
					{
						id: 'form-push-button',
						text: _UNO('.uno:Pushbutton'),
						command: '.uno:Pushbutton',
						combination: 'PB',
					},
					{
						id: 'form-image-button',
						text: _UNO('.uno:Imagebutton'),
						command: '.uno:Imagebutton',
						combination: 'IB',
					},
					{
						id: 'form-check-box',
						text: _UNO('.uno:CheckBox'),
						command: '.uno:CheckBox',
						combination: 'CB',
					},
					{
						id: 'form-option-button',
						text: _UNO('.uno:RadioButton'),
						command: '.uno:RadioButton',
						combination: 'OB',
					},
					{
						id: 'form-label',
						text: _UNO('.uno:Label'),
						command: '.uno:Label',
						combination: 'LA',
					},
					{
						id: 'form-group-box',
						text: _UNO('.uno:GroupBox'),
						command: '.uno:GroupBox',
						combination: 'GB',
					},
					{
						id: 'form-list-box',
						text: _UNO('.uno:ListBox'),
						command: '.uno:ListBox',
						combination: 'LB',
					},
					{
						id: 'form-combo-box',
						text: _UNO('.uno:ComboBox'),
						command: '.uno:ComboBox',
						combination: 'CX',
					},
					{
						id: 'form-spin-button',
						text: _UNO('.uno:SpinButton'),
						command: '.uno:SpinButton',
						combination: 'SB',
					},
					{
						id: 'form-scroll-bar',
						text: _UNO('.uno:ScrollBar'),
						command: '.uno:ScrollBar',
						combination: 'SC',
					},
					{
						id: 'form-image-control',
						text: _UNO('.uno:ImageControl'),
						command: '.uno:ImageControl',
						combination: 'IC',
					},
				],
			},
			{
				id: 'form-fields',
				name: _('Fields'),
				icon: 'lc_formtextbox.svg',
				combination: 'FI',
				items: [
					// the text box shows the icon of a text box of the forms
					{
						id: 'form-text-box',
						text: _UNO('.uno:Edit'),
						command: '.uno:Edit',
						combination: 'TX',
						icon: 'lc_formtextbox.svg',
					},
					{
						id: 'form-formatted-field',
						text: _UNO('.uno:FormattedField'),
						command: '.uno:FormattedField',
						combination: 'FF',
					},
					// the icon theme of the engine shows the numeric field with the icon of the
					// standard number format
					{
						id: 'form-numeric-field',
						text: _UNO('.uno:NumericField'),
						command: '.uno:NumericField',
						combination: 'NF',
						icon: 'lc_numberformatstandard.svg',
					},
					{
						id: 'form-currency-field',
						text: _UNO('.uno:CurrencyField'),
						command: '.uno:CurrencyField',
						combination: 'CF',
					},
					{
						id: 'form-date-field',
						text: _UNO('.uno:DateField'),
						command: '.uno:DateField',
						combination: 'DF',
					},
					{
						id: 'form-time-field',
						text: _UNO('.uno:TimeField'),
						command: '.uno:TimeField',
						combination: 'TF',
					},
					{
						id: 'form-pattern-field',
						text: _UNO('.uno:PatternField'),
						command: '.uno:PatternField',
						combination: 'PF',
					},
					{
						id: 'form-file-selection',
						text: _UNO('.uno:FileControl'),
						command: '.uno:FileControl',
						combination: 'FS',
					},
				],
			},
		];
	}

	// The buttons of one group, two above each other in each column.
	private getGroup(group: FormTabGroup): any {
		const columns: any[] = [];
		for (let index = 0; index < group.items.length; index += 2)
			columns.push(this.getColumn(group.items.slice(index, index + 2)));

		return {
			type: 'overflowgroup',
			id: group.id,
			name: group.name,
			icon: group.icon,
			accessibility: {
				focusBack: true,
				combination: group.combination,
				de: null,
			},
			children: columns,
		};
	}

	// Small buttons above each other.
	private getColumn(items: FormTabItem[]): any {
		return {
			type: 'container',
			children: items.map((item: FormTabItem) => {
				return {
					type: 'toolbox',
					children: [this.getItem(item)],
				};
			}),
			vertical: true,
		};
	}

	private getItem(item: FormTabItem): any {
		const widget: any = {
			id: item.id,
			type: 'toolitem',
			text: item.text,
			command: item.command,
			accessibility: {
				focusBack: true,
				combination: item.combination,
				de: null,
			},
		};
		if (item.icon) widget.icon = item.icon;
		return widget;
	}
}

JSDialog.ImpressFormTab = new ImpressFormTab();
