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
 * FormulaAutoCompletePopup
 */

/* global app */

class FormulaAutoCompletePopup extends AutoCompletePopup {
	functionList: Array<any>;

	constructor(map: any) {
		super('formulaautocompletePopup', map);
		this.newPopupData.serverSyncSelection = false;
	}

	onAdd() {
		this.newPopupData.isAutoCompletePopup = true;
		this.map.on(
			'openformulaautocompletepopup',
			this.openFormulaAutoCompletePopup,
			this,
		);
		this.map.on('sendformulatext', this.sendFormulaText, this);
		this.functionList = null;
	}

	openFormulaAutoCompletePopup(ev: FireEvent) {
		this.map.fire('closepopup');
		this.openPopup({ data: ev });
	}

	sendFormulaText(ev: FireEvent) {
		this.openFormulaAutoCompletePopup(ev);
	}

	getPopupEntries(ev: FireEvent): any[] {
		const entries: any[] = [];
		this.functionList = ev.data.data;
		if (this.functionList.length !== 0) {
			for (var i in this.functionList) {
				var entry = {
					text: this.functionList[i].name,
					columns: [
						{ text: this.functionList[i].name },
						{ text: '\n' + this.functionList[i].description },
					],
					row: i.toString(),
				};
				entries.push(entry);
			}
		}
		return entries;
	}

	getAutocompleteText(
		currentCellFormula: string,
		functionName: string,
		endIndex: number,
	): string {
		// Step-1: Find indexes of all the '(', ';', '-', '+', '*', '/'
		const openBracketIndex: number[] = [];
		const semicolonIndex: number[] = [];
		const plusIndex: number[] = [];
		const multiplyIndex: number[] = [];
		const divideIndex: number[] = [];
		const minusIndex: number[] = [];
		const equalIndex: number = 0;

		for (let i = 0; i < currentCellFormula.length; i++) {
			const char = currentCellFormula.charAt(i);
			if (char === '(') openBracketIndex.push(i);
			else if (char === ';') semicolonIndex.push(i);
			else if (char === '+') plusIndex.push(i);
			else if (char === '*') multiplyIndex.push(i);
			else if (char === '/') divideIndex.push(i);
			else if (char === '-') minusIndex.push(i);
		}

		// Step-2: Find smallest difference between endIndex and indexes of all the '(', ';'
		// that will give us the startIndex
		let minDiff: number = Number.MAX_VALUE;
		let startIndex: number;

		const updateMinDiff = (index: number) => {
			const tmp = endIndex - index;
			if (tmp >= 0 && tmp < minDiff) {
				minDiff = tmp;
				startIndex = index + 1;
			}
		};

		updateMinDiff(equalIndex);
		openBracketIndex.forEach(updateMinDiff);
		semicolonIndex.forEach(updateMinDiff);
		plusIndex.forEach(updateMinDiff);
		minusIndex.forEach(updateMinDiff);
		multiplyIndex.forEach(updateMinDiff);
		divideIndex.forEach(updateMinDiff);

		// Step-3: extract the text we want to complete using startIndex and endIndex
		const partialText: string = currentCellFormula
			.substring(startIndex, endIndex + 1)
			.trim();

		// Step-4: compare partialText and functionName to find remaining text need to autocomplete
		let autoCompleteFunctionName: string = '';
		for (
			let i = 0;
			i < Math.max(partialText.length, functionName.length);
			i++
		) {
			if (
				partialText.charAt(i).toLowerCase() !=
				functionName.charAt(i).toLowerCase()
			) {
				autoCompleteFunctionName = functionName.substring(i);
				break;
			}
		}

		return autoCompleteFunctionName;
	}

	callback(objectType: any, eventType: any, object: any, index: number) {
		if (eventType === 'close') {
			this.closePopup();
		} else if (eventType === 'select' || eventType === 'activate') {
			const namedRange: string = this.functionList[index].namedRange;
			const currentText: string = this.map._docLayer._lastFormula;
			const addedCharacterIndex: number =
				this.map._docLayer._newFormulaDiffIndex;

			const functionName: string = this.getAutocompleteText(
				currentText,
				this.functionList[index].name,
				addedCharacterIndex,
			);

			if (namedRange) this.map._textInput._sendText(functionName);
			else this.map._textInput._sendText(functionName + '(');
			this.closePopup();
		} else if (eventType === 'keydown') {
			if (object.key !== 'Tab' && object.key !== 'Shift') {
				this.map.focus();
				return true;
			}
		}
		return false;
	}
}
