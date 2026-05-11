// @ts-strict-ignore
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
 * Control.FormulaUsagePopup
 */

/* global app */

class FormulaUsagePopup extends AutoCompletePopup {
	usageText: string;
	declare newPopupData: PopupData;

	constructor(map: any) {
		super(AutoCompleteDialogId.FormulaUsagePopup, map);
		this.newPopupData = {
			children: [
				{
					id: 'container',
					type: 'container',
					enabled: true,
					children: new Array<WidgetJSON>(),
					vertical: true,
				} as any as WidgetJSON,
			] as Array<WidgetJSON>,
			jsontype: 'dialog',
			type: 'dialog', // exception: dialog but it behaves like a popup
			cancellable: true,
			hasClose: false, // do not show titlebar
			isAutoCompletePopup: true, // don't steal focus
			popupParent: undefined,
			clickToClose: undefined,
			id: 'formulausagePopup',
			title: '', // no titlebar
		} as PopupData;
	}

	onAdd() {
		this.newPopupData.isAutoCompletePopup = true;
		this.map.on('openformulausagepopup', this.openFormulaUsagePopup, this);
		this.map.on('sendformulausagetext', this.sendFormulaUsageText, this);
	}

	openFormulaUsagePopup(ev: FireEvent) {
		this.openPopup({ data: ev });
		this.map.focus();
	}

	sendFormulaUsageText(ev: FireEvent) {
		this.openFormulaUsagePopup(ev);
	}

	getPopupEntries(ev: FireEvent): any[] {
		this.usageText = ev.data.data;
		var chIndex = this.usageText.indexOf(':');
		var functionUsage = this.usageText.substring(0, chIndex);
		var usageDescription = this.usageText.substring(chIndex + 1);
		const entries = [
			{
				row: 0,
				columns: [{ text: functionUsage }],
				collapsed: false,
				children: [{ row: 1, columns: [{ text: usageDescription }] }],
			},
		];

		return entries;
	}

	callback(objectType: any, eventType: any, object: any, index: number) {
		if (eventType === 'close') {
			this.closePopup();
		}
		return false;
	}
}
