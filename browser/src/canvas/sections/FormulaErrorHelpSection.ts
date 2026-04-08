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

interface FormulaErrorInfo {
	error: string; // e.g. "#DIV/0!"
	errorCode: number; // e.g. 532
	errorDescription: string; // e.g. "Error: Division by zero"
}

class FormulaErrorHelpSection extends HTMLObjectSection {
	static sectionName = 'formula error help button';
	private errorInfo: FormulaErrorInfo;
	private menuDiv: HTMLDivElement | null = null;

	constructor(documentPosition: cool.SimplePoint, errorInfo: FormulaErrorInfo) {
		super(
			app.CSections.FormulaErrorHelpButton.name,
			null as unknown as number,
			null as unknown as number,
			documentPosition,
			'formula-error-help-btn',
			true,
		);

		this.errorInfo = errorInfo;

		const objectDiv = this.getHTMLObject();
		objectDiv.style.pointerEvents = '';

		const icon = document.createElement('div');
		icon.className = 'formula-error-icon';
		icon.textContent = '!';
		objectDiv.appendChild(icon);

		objectDiv.onclick = (e: MouseEvent) => {
			e.stopPropagation();
			e.preventDefault();
			if (this.menuDiv) {
				this.closeMenu();
			} else {
				this.openMenu();
			}
		};
	}

	zIndex: number = app.CSections.FormulaErrorHelpButton.zIndex;

	private openMenu(): void {
		this.closeMenu();

		const menu = document.createElement('div');
		menu.className = 'formula-error-menu';

		// Error description header
		const header = document.createElement('div');
		header.className = 'formula-error-menu-header';
		const errorLabel = document.createElement('span');
		errorLabel.className = 'formula-error-menu-error-code';
		errorLabel.textContent = this.errorInfo.error;
		header.appendChild(errorLabel);
		const desc = document.createElement('span');
		desc.textContent =
			' ' + (this.errorInfo.errorDescription || this.errorInfo.error);
		header.appendChild(desc);
		menu.appendChild(header);

		const sep = document.createElement('div');
		sep.className = 'formula-error-menu-separator';
		menu.appendChild(sep);

		// Help on this error
		this.addMenuItem(menu, _('Help on this error'), () => {
			window.open('https://help.collaboraoffice.com/', '_blank');
		});

		// Inspect with Function Wizard
		this.addMenuItem(menu, _('Inspect with Function Wizard'), () => {
			app.map.sendUnoCommand('.uno:FunctionDialog');
		});

		// Edit in Formula Bar
		this.addMenuItem(menu, _('Edit in Formula Bar'), () => {
			app.map.sendUnoCommand('.uno:StartFormula');
			app.map.onFormulaBarFocus();
			app.map.formulabarFocus();
		});

		// AI help (conditional)
		if (app.map.isAIConfigured) {
			this.addMenuItem(menu, _('Help fix this error'), () => {
				app.dispatcher.dispatch('helpfixformulaerror');
			});
		}

		const objectDiv = this.getHTMLObject();
		objectDiv.appendChild(menu);
		this.menuDiv = menu;

		// Close menu when clicking outside
		const closeHandler = (ev: MouseEvent) => {
			if (!objectDiv.contains(ev.target as Node)) {
				this.closeMenu();
				document.removeEventListener('mousedown', closeHandler);
			}
		};
		setTimeout(() => {
			document.addEventListener('mousedown', closeHandler);
		}, 0);
	}

	private addMenuItem(
		container: HTMLElement,
		label: string,
		action: () => void,
	): void {
		const item = document.createElement('div');
		item.className = 'formula-error-menu-item';
		item.textContent = label;
		item.onclick = (e: MouseEvent) => {
			e.stopPropagation();
			e.preventDefault();
			FormulaErrorHelpSection.hide();
			action();
		};
		container.appendChild(item);
	}

	private closeMenu(): void {
		if (this.menuDiv) {
			this.menuDiv.remove();
			this.menuDiv = null;
		}
	}

	public static show(
		documentPosition: cool.SimplePoint,
		errorInfo: FormulaErrorInfo,
	): void {
		FormulaErrorHelpSection.hide();
		const section = new FormulaErrorHelpSection(documentPosition, errorInfo);
		app.sectionContainer.addSection(section);
	}

	public static hide(): void {
		if (FormulaErrorHelpSection.isOpen())
			app.sectionContainer.removeSection(
				app.CSections.FormulaErrorHelpButton.name,
			);
	}

	public static isOpen(): boolean {
		return app.sectionContainer.doesSectionExist(
			app.CSections.FormulaErrorHelpButton.name,
		);
	}
}

app.definitions.formulaErrorHelpSection = FormulaErrorHelpSection;
