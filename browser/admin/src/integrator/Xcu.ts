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

var _: any = (window as any)._;

interface XcuObject {
	[app: string]: any;
}

type XcuValue = string | boolean | XcuTree;

interface XcuTree {
	[key: string]: XcuValue;
}

const defaultXcuObj: Record<string, any> = {
	Calc: {
		Grid: {
			ShowGrid: false,
			Option: {
				SnapToGrid: false,
				SizeToGrid: true,
				Synchronize: true,
			},
		},
		Print: {
			Page: {
				EmptyPages: false,
				ForceBreaks: false,
			},
			Other: {
				AllSheets: false,
			},
		},
	},
	Draw: {
		Grid: {
			ShowGrid: false,
			Option: {
				SnapToGrid: true,
				Synchronize: false,
			},
			SnapGrid: {
				Size: true,
			},
		},
		Print: {
			Content: {
				Drawing: true,
			},
			Page: {
				PageSize: false,
				PageTile: false,
				Booklet: false,
				BookletFront: true,
				BookletBack: true,
			},
			Other: {
				PageName: false,
				Date: false,
				Time: false,
				HiddenPage: true,
				FromPrinterSetup: false,
			},
		},
	},
	Impress: {
		Grid: {
			ShowGrid: false,
			Option: {
				SnapToGrid: true,
				Synchronize: false,
			},
			SnapGrid: {
				Size: true,
			},
		},
		Print: {
			Content: {
				Presentation: true,
				Note: false,
				Handout: false,
				Outline: false,
			},
			Page: {
				PageSize: false,
				PageTile: false,
				Booklet: false,
				BookletFront: true,
				BookletBack: true,
			},
			Other: {
				PageName: false,
				Date: false,
				Time: false,
				HiddenPage: true,
				FromPrinterSetup: false,
				HandoutHorizontal: false,
			},
		},
	},
	Writer: {
		Grid: {
			ShowGrid: false,
			Option: {
				SnapToGrid: false,
				Synchronize: false,
			},
		},
		Print: {
			Content: {
				Graphic: true,
				Table: true,
				Drawing: true,
				Control: true,
				Background: true,
				PrintBlack: false,
				PrintHiddenText: false,
				PrintPlaceholders: false,
			},
			Page: {
				LeftPage: true,
				RightPage: true,
				Reversed: false,
				Brochure: false,
				BrochureRightToLeft: false,
			},
			Output: {
				SinglePrintJob: false,
			},
			Papertray: {
				FromPrinterSetup: false,
			},
			EmptyPages: true,
		},
		Content: {
			Display: {
				ShowBoundaries: true,
			},
		},
	},
};

class Xcu {
	private xcuDataObj: XcuObject = {};
	private fileId: string | null = null;

	constructor(fileId: string, XcuFileContent: string | null) {
		this.fileId = fileId;

		try {
			const parsedObj =
				XcuFileContent === null || XcuFileContent.length === 0
					? {}
					: this.parse(XcuFileContent);
			this.xcuDataObj = this.mergeDefaults(defaultXcuObj, parsedObj);
		} catch (error) {
			(window as any).SettingIframe.showErrorModal(
				_('Something went wrong while loading Document settings.'),
			);
			console.error('Error parsing XCU file:', error);
		}
	}

	private parse(content: string): XcuObject {
		const parser = new DOMParser();
		const xmlDoc = parser.parseFromString(content, 'application/xml');

		if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
			(window as any).SettingIframe.showErrorModal(
				_('Something went wrong while loading Document settings.'),
			);
		}

		const result: XcuObject = {};

		const items = xmlDoc.getElementsByTagName('item');

		Array.from(items).forEach((item) => {
			const rawPath = item.getAttribute('oor:path');
			if (!rawPath) {
				return;
			}

			const prefix = '/org.openoffice.Office.';
			const path = rawPath.startsWith(prefix)
				? rawPath.slice(prefix.length)
				: rawPath;

			const keys = path.split('/').filter((key) => key.trim() !== '');

			let currentLevel = result;
			let gridLevel;
			keys.forEach((key) => {
				if (!(key in currentLevel)) {
					currentLevel[key] = {};
				}
				currentLevel = currentLevel[key];

				if (key === 'Grid') {
					gridLevel = currentLevel;
					gridLevel['ShowGrid'] = false;
				}
			});

			const props = item.getElementsByTagName('prop');
			Array.from(props).forEach((prop) => {
				const propName = prop.getAttribute('oor:name');
				if (!propName) {
					return;
				}

				const valueElement = prop.getElementsByTagName('value')[0];
				let value: string | boolean = valueElement
					? valueElement.textContent || ''
					: '';

				const lowerValue = value.toLowerCase();
				if (lowerValue === 'true') {
					value = true;
				} else if (lowerValue === 'false') {
					value = false;
				}

				if (typeof value === 'boolean') {
					if (propName === 'VisibleGrid') {
						gridLevel['ShowGrid'] = value;
					} else {
						currentLevel[propName] = value;
					}
				}
			});
		});

		return result;
	}

	// Returns an XcuTree with overrides merged over defaults. Keys not present
	// in defaults are dropped, so removed/deprecated settings fall away.
	private mergeDefaults(
		defaults: XcuTree,
		overrides: XcuValue | undefined,
	): XcuTree {
		const overrideTree: XcuTree | undefined =
			overrides && typeof overrides === 'object' ? overrides : undefined;

		const result: XcuTree = {};
		for (const key in defaults) {
			if (!Object.prototype.hasOwnProperty.call(defaults, key)) {
				continue;
			}
			const def = defaults[key];
			const parsedValue = overrideTree ? overrideTree[key] : undefined;
			if (typeof def === 'object') {
				result[key] = this.mergeDefaults(def, parsedValue);
			} else {
				result[key] = parsedValue === undefined ? def : parsedValue;
			}
		}

		return result;
	}

	private generate(xcu: XcuObject): string {
		function generateItemNodes(node: any, path: string[]): string[] {
			const items: string[] = [];
			const leafProps: { [key: string]: string | boolean } = {};
			const nestedKeys: string[] = [];

			for (const key in node) {
				if (Object.prototype.hasOwnProperty.call(node, key)) {
					const value = node[key];
					if (
						value !== null &&
						typeof value === 'object' &&
						!Array.isArray(value)
					) {
						nestedKeys.push(key);
					} else {
						leafProps[key] = value;
					}
				}
			}

			let visibleGrid;

			if (Object.keys(leafProps).length > 0) {
				const oorPath = '/org.openoffice.Office.' + path.join('/');
				let propsXml = '';
				for (const propName in leafProps) {
					if (Object.prototype.hasOwnProperty.call(leafProps, propName)) {
						if (propName === 'ShowGrid') {
							visibleGrid = leafProps[propName];
							continue;
						}
						const value = leafProps[propName];
						let valueStr = '';
						if (typeof value === 'boolean') {
							valueStr = value ? 'true' : 'false';
						} else {
							valueStr = String(value);
						}
						propsXml += `\n    <prop oor:name="${propName}" oor:op="fuse"><value>${valueStr}</value></prop>`;
					}
				}
				const itemXml =
					`  <item oor:path="${oorPath}">` + propsXml + `\n  </item>`;
				items.push(itemXml);
			}

			for (const key of nestedKeys) {
				const child = node[key];
				if (typeof visibleGrid === 'boolean') {
					child['VisibleGrid'] = visibleGrid;
				}
				const newPath = path.concat(key);
				items.push(...generateItemNodes(child, newPath));
			}

			return items;
		}

		const itemsXml = generateItemNodes(xcu, []);
		const xcuXml =
			`<?xml version="1.0" encoding="UTF-8"?>\n` +
			`<oor:items\n` +
			`    xmlns:oor="http://openoffice.org/2001/registry"\n` +
			`    xmlns:xs="http://www.w3.org/2001/XMLSchema"\n` +
			`    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">\n\n` +
			itemsXml.join('\n') +
			`\n</oor:items>`;
		return xcuXml;
	}

	public createXcuEditorUI(container: HTMLElement): HTMLElement {
		const heading = document.createElement('h3');
		heading.textContent = _('Document Settings');
		container.appendChild(heading);

		const descEl = document.createElement('p');
		descEl.textContent = _('Adjust how office documents behave.');
		container.appendChild(descEl);

		const editorContainer = document.createElement('div');
		editorContainer.id = 'xcu-editor';

		const navContainer = document.createElement('div');
		navContainer.className = 'xcu-editor-tabs-nav';

		const tabs = [
			{ id: 'calc', label: 'Calc' },
			{ id: 'writer', label: 'Writer' },
			{ id: 'impress', label: 'Impress' },
			{ id: 'draw', label: 'Draw' },
		];

		tabs.forEach((tab) => {
			const btn = document.createElement('button');
			btn.type = 'button';
			btn.className = 'xcu-editor-tab';
			btn.id = `xcu-tab-${tab.id}`;
			btn.textContent = tab.label;
			btn.addEventListener('click', () => {
				navContainer
					.querySelectorAll('.xcu-editor-tab')
					.forEach((b) => b.classList.remove('active'));
				btn.classList.add('active');

				const contentsContainer = editorContainer.querySelector(
					'#xcu-tab-contents',
				) as HTMLElement;
				contentsContainer.innerHTML = '';
				if (this.xcuDataObj && this.xcuDataObj[tab.label]) {
					const renderedTree = (
						window as any
					).settingIframe.renderSettingsOption(this.xcuDataObj[tab.label]);
					renderedTree.classList.add('xcu-settings-grid');
					contentsContainer.appendChild(renderedTree);
				} else {
					contentsContainer.textContent = `No settings available for ${tab.label}`;
				}
			});
			navContainer.appendChild(btn);
		});

		const contentsContainer = document.createElement('div');
		contentsContainer.id = 'xcu-tab-contents';
		contentsContainer.textContent = _('Select a tab to view settings.');

		editorContainer.appendChild(navContainer);
		editorContainer.appendChild(contentsContainer);
		container.appendChild(editorContainer);

		const actionsContainer = document.createElement('div');
		actionsContainer.classList.add('xcu-editor-actions');

		const resetButton = document.createElement('button');
		resetButton.type = 'button';
		resetButton.id = 'document-settings-reset-button';
		resetButton.classList.add('button', 'button--vue-secondary');
		resetButton.title = _('Reset to default Document settings');
		resetButton.innerHTML = `
			<span class="button__wrapper">
				<span class="button__icon xcu-reset-icon">
				<svg fill="currentColor" width="24" height="24" viewBox="0 0 24 24">
					<!-- Replace with your Reset icon SVG path -->
					<path d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 .34-.03.67-.08 1h2.02c.05-.33.06-.66.06-1 0-4.42-3.58-8-8-8zm-6 7c0-.34.03-.67.08-1H4.06c-.05.33-.06.66-.06 1 0 4.42 3.58 8 8 8v3l4-4-4-4v3c-3.31 0-6-2.69-6-6z"></path>
				</svg>
				</span>
			</span>
			`;

		resetButton.addEventListener('click', async () => {
			const confirmed = window.confirm(
				_('Are you sure you want to reset Document settings?'),
			);
			if (!confirmed) {
				return;
			}
			resetButton.disabled = true;
			this.xcuDataObj = defaultXcuObj;
			await this.generateXcuAndUpload();
			resetButton.disabled = false;
		});

		const saveButton = document.createElement('button');
		saveButton.type = 'button';
		saveButton.id = 'document-settings-save-button';
		saveButton.classList.add('button', 'button-primary', 'button--text-only');
		saveButton.title = _('Save Document settings');

		const wrapperSpan = document.createElement('span');
		wrapperSpan.classList.add('button__wrapper');
		saveButton.appendChild(wrapperSpan);

		const textSpan = document.createElement('span');
		textSpan.classList.add('button__text');
		textSpan.textContent = _('Save');
		wrapperSpan.appendChild(textSpan);
		saveButton.addEventListener('click', async () => {
			saveButton.disabled = true;
			await this.generateXcuAndUpload();
			saveButton.disabled = false;
		});

		actionsContainer.appendChild(resetButton);
		actionsContainer.appendChild(saveButton);
		container.appendChild(actionsContainer);

		setTimeout(() => {
			const defaultTab = navContainer.querySelector(
				'#xcu-tab-calc',
			) as HTMLElement;
			if (defaultTab) {
				defaultTab.click();
			}
		}, 0);

		return container;
	}

	public async generateXcuAndUpload(): Promise<void> {
		const xcuContent = this.generate(this.xcuDataObj);
		await (window as any).settingIframe.uploadXcuFile(this.fileId, xcuContent);
	}
}

(window as any).Xcu = Xcu;
