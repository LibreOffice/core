/* -*- js-indent-level: 8; fill-column: 100 -*- */

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
 * This file contains service which encapsulates table styles synchronization
 * with the core.
 */

// for uno
interface TableStyleInfo {
	ContainsHeader: boolean;
	TotalsRow: boolean;
	UseFirstColumnFormatting: boolean;
	UseLastColumnFormatting: boolean;
	UseRowStripes: boolean;
	UseColStripes: boolean;
	AutoFilter: boolean;
	TableStyleName: string;
}

// from state change
type TableStyleElementType =
	| 'WholeTable'
	| 'FirstColumnStripe'
	| 'FirstRowStripe'
	| 'LastColumn'
	| 'FirstColumn'
	| 'HeaderRow'
	| 'TotalRow';

interface TableStyleElement {
	Type: TableStyleElementType;
	FillColor: string; // hex value
}

interface TableStyleEntry {
	Name: string;
	UIName: string;
	Elements: Array<TableStyleElement>;
}

interface TableStyleLayoutEntry {
	isSeparator: boolean;
	text?: string;
	style?: TableStyleEntry;
}

function getElementColor(
	style: TableStyleEntry,
	type: string,
): string | undefined {
	const el = style.Elements.find((e) => e.Type === type);
	return el ? el.FillColor : undefined;
}

function toHex(r: number, g: number, b: number): string {
	const clamp = (v: number) => Math.max(0, Math.min(255, Math.round(v)));
	return (
		'#' +
		clamp(r).toString(16).padStart(2, '0') +
		clamp(g).toString(16).padStart(2, '0') +
		clamp(b).toString(16).padStart(2, '0')
	);
}

function darkenColor(hex: string, factor: number): string {
	const [r, g, b] = parseHexToRgb(hex);
	return toHex(r * (1 - factor), g * (1 - factor), b * (1 - factor));
}

class TableStylesService {
	private styles = new Array<TableStyleEntry>();

	private static readonly groupOrder: Record<string, number> = {
		light: 1,
		medium: 2,
		dark: 3,
		other: 4,
	};

	private static getGroup(name: string): string {
		const lower = name.toLowerCase();
		if (lower.includes('light')) return 'light';
		if (lower.includes('medium')) return 'medium';
		if (lower.includes('dark')) return 'dark';
		return 'other';
	}

	private static getGroupLabel(group: string): string {
		switch (group) {
			case 'light':
				return _('Light');
			case 'medium':
				return _('Medium');
			case 'dark':
				return _('Dark');
			default:
				return _('Custom');
		}
	}

	public get(): Array<TableStyleEntry> {
		return this.styles;
	}

	constructor() {
		app.map.on('commandstatechanged', this.onCommandState.bind(this));
	}

	/** Build the .uno:DatabaseSettings args from a TableStyleInfo, translating
	 * state-side property names to the PoolItem MID names declared in
	 * scslots.sdi (HeaderRow, TotalRow, ...) that the SDI parameter list
	 * expects. */
	private buildArgs(state: TableStyleInfo): any {
		return {
			'DatabaseSettings.HeaderRow': {
				type: 'boolean',
				value: state.ContainsHeader,
			},
			'DatabaseSettings.TotalRow': {
				type: 'boolean',
				value: state.TotalsRow,
			},
			'DatabaseSettings.FirstCol': {
				type: 'boolean',
				value: state.UseFirstColumnFormatting,
			},
			'DatabaseSettings.LastCol': {
				type: 'boolean',
				value: state.UseLastColumnFormatting,
			},
			'DatabaseSettings.StripedRows': {
				type: 'boolean',
				value: state.UseRowStripes,
			},
			'DatabaseSettings.StripedCols': {
				type: 'boolean',
				value: state.UseColStripes,
			},
			'DatabaseSettings.ShowFilters': {
				type: 'boolean',
				value: state.AutoFilter,
			},
			'DatabaseSettings.StyleID': {
				type: 'string',
				value: state.TableStyleName || '',
			},
		};
	}

	public onCommandState(e: any) {
		if (e.commandName === '.uno:TableStyles') {
			if (e.state === '') return;

			try {
				this.styles = JSON.parse(e.state).TableStyles;
				this.styles.sort((a, b) => {
					const tableGroupA =
						TableStylesService.groupOrder[
							TableStylesService.getGroup(a.Name)
						] ?? 4;
					const tableGroupB =
						TableStylesService.groupOrder[
							TableStylesService.getGroup(b.Name)
						] ?? 4;
					if (tableGroupA !== tableGroupB) return tableGroupA - tableGroupB;
					return a.Name.localeCompare(b.Name, undefined, {
						numeric: true,
						sensitivity: 'base',
					});
				});
			} catch (e) {
				app.console.error('Failed to parse TableStyles: ' + e);
			}

			app.map.fire('jsdialogupdate', {
				data: {
					id: WindowId.Notebookbar + '',
					type: '',
					jsontype: 'notebookbar',
					action: 'update',
					control: this.generateTableStylesJSON(),
				} as JSDialogJSON,
			});
		} else if (e.commandName === '.uno:DatabaseSettings') {
			const currentStyle = e.state as TableStyleInfo;

			let position = -1;
			if (currentStyle) {
				const layout = this.getEntriesLayout();
				const targetName = currentStyle.TableStyleName || 'None';
				position = layout.findIndex(
					(item: TableStyleLayoutEntry) =>
						!item.isSeparator && item.style?.Name === targetName,
				);
				if (position === -1) {
					position = layout.findIndex(
						(item: TableStyleLayoutEntry) => !item.isSeparator,
					);
					if (position === -1) position = 0;
				}
			}

			app.map.fire('jsdialogaction', {
				data: {
					id: WindowId.Notebookbar + '',
					type: '',
					jsontype: 'notebookbar',
					action: 'action',
					data: {
						control_id: 'tablestyles_design',
						action_type: 'select',
						position: position,
						data: {
							position: position,
						},
					},
				} as JSDialogJSON,
			});
		}
	}

	public generateTableStylesJSON(): OverflowGroupWidgetJSON {
		return {
			id: 'Tablestyles_design--group',
			type: 'overflowgroup',
			name: _('Table Styles'),
			nofold: true,
			icon: 'lc_tablestyle.svg',
			children: [
				{
					id: 'tablestyles_design-iconview-list',
					type: 'iconviewlist',
					accessibility: { focusBack: false, combination: 'TL', de: null },
					children: [
						{
							id: 'tablestyles_design',
							type: 'iconview',
							text: _('Table Styles'),
							command: '.uno:DatabaseSettings',
							aria: { label: _('Table Styles') },
							accessibility: {
								focusBack: true,
								combination: 'TS',
							},
							entries: this.generateJSON(),
							singleclickactivate: true,
							textWithIconEnabled: false, // standard names from core are not translated yet
							selectionmode: 'single',
						} as IconViewJSON,
					],
				},
			],
		};
	}

	public getNoneStyle(): TableStyleEntry {
		return {
			Name: 'None',
			UIName: _('None'),
			Elements: [],
		} as TableStyleEntry;
	}

	private styleHasElement(
		style: TableStyleEntry,
		elementType: TableStyleElementType,
	) {
		for (const element of style.Elements)
			if (element.Type === elementType) return true;

		return false;
	}

	public applyStyle(newStyleNumber: number) {
		const tableStyleEntry =
			newStyleNumber === -1 ? this.getNoneStyle() : this.styles[newStyleNumber];
		if (!tableStyleEntry) {
			app.console.error(
				'TableStylesService: not found style with id: ' + newStyleNumber,
			);
			return;
		}

		let tableStyle = app.map['stateChangeHandler'].getItemValue(
			'.uno:DatabaseSettings',
		) as TableStyleInfo;
		if (!tableStyle) {
			// fallback, generate from defined styles
			tableStyle = {
				ContainsHeader: this.styleHasElement(tableStyleEntry, 'HeaderRow'),
				TotalsRow: this.styleHasElement(tableStyleEntry, 'TotalRow'),
				UseFirstColumnFormatting: this.styleHasElement(
					tableStyleEntry,
					'FirstColumn',
				),
				UseLastColumnFormatting: this.styleHasElement(
					tableStyleEntry,
					'LastColumn',
				),
				UseRowStripes: this.styleHasElement(tableStyleEntry, 'FirstRowStripe'),
				UseColStripes: this.styleHasElement(
					tableStyleEntry,
					'FirstColumnStripe',
				),
				AutoFilter: true,
			} as TableStyleInfo;
		}

		const updated = { ...tableStyle, TableStyleName: tableStyleEntry.Name };
		app.map.sendUnoCommand('.uno:DatabaseSettings', this.buildArgs(updated));
	}

	public generateIcon(style: TableStyleEntry): string {
		const wholeTable = getElementColor(style, 'WholeTable') || 'FFFF';
		const headerRow = getElementColor(style, 'HeaderRow') || wholeTable;
		const firstRowStripe =
			getElementColor(style, 'FirstRowStripe') || wholeTable;
		const secondRowStripe =
			getElementColor(style, 'SecondRowStripe') || wholeTable;

		const wt = '#' + wholeTable;
		const hr = '#' + headerRow;
		const frs = '#' + firstRowStripe;
		const srs = '#' + secondRowStripe;

		const getStyleIndex = (variant: string) => {
			const match = style.Name.match(new RegExp(`${variant}(\\d+)$`));
			return match ? parseInt(match[1], 10) : 1;
		};

		let svg: string;

		if (style.Name.indexOf('Light') >= 0) {
			svg = lightTableStyleSvg(hr, wt, frs, getStyleIndex('Light'));
		} else if (style.Name.indexOf('Medium') >= 0) {
			svg = mediumTableStyleSvg(hr, wt, frs, getStyleIndex('Medium'));
		} else if (style.Name.indexOf('Dark') >= 0) {
			svg = darkTableStyleSvg(hr, wt, frs, getStyleIndex('Dark'));
		} else {
			svg = customTableStyleSvg(hr, frs, srs);
		}

		return 'data:image/svg+xml;charset=UTF-8,' + encodeURIComponent(svg);
	}

	private getEntriesLayout(): TableStyleLayoutEntry[] {
		const layout: TableStyleLayoutEntry[] = [];
		layout.push({
			isSeparator: true,
			text: TableStylesService.getGroupLabel('light'),
		});
		layout.push({ isSeparator: false, style: this.getNoneStyle() });

		let previousGroup = 'light';
		this.styles.forEach((element) => {
			const currentGroup = TableStylesService.getGroup(element.Name);
			if (currentGroup === 'other') return;

			if (currentGroup !== previousGroup) {
				layout.push({
					isSeparator: true,
					text: TableStylesService.getGroupLabel(currentGroup),
				});
			}
			previousGroup = currentGroup;
			layout.push({ isSeparator: false, style: element });
		});

		const customStyles = this.styles.filter(
			(style) => TableStylesService.getGroup(style.Name) === 'other',
		);
		if (customStyles.length > 0) {
			layout.push({
				isSeparator: true,
				text: TableStylesService.getGroupLabel('other'),
			});
			customStyles.forEach((style) => {
				layout.push({ isSeparator: false, style: style });
			});
		}

		return layout;
	}

	public generateJSON(): Array<IconViewEntry> {
		if (!this.styles) return [];

		const currentStyle = app.map['stateChangeHandler'].getItemValue(
			'.uno:DatabaseSettings',
		) as TableStyleInfo | undefined;

		const iconViewEntries = new Array<IconViewEntry>();
		let i = 0;

		const layout = this.getEntriesLayout();
		layout.forEach((item) => {
			if (item.isSeparator) {
				iconViewEntries.push({
					row: 'sep-' + i,
					separator: true,
					text: item.text || '',
					image: '',
				} as IconViewEntry);
			} else {
				if (!item.style) return;
				const element = item.style;
				const isNone = element.Name === 'None';
				const selected = isNone
					? !currentStyle ||
						currentStyle.TableStyleName === '' ||
						currentStyle.TableStyleName === 'None'
					: currentStyle && element.Name === currentStyle.TableStyleName;

				const formattedText = isNone ? _('None') : element.UIName;

				iconViewEntries.push({
					row: isNone ? -1 : i++,
					text: formattedText,
					image: isNone
						? 'images/lc_table_none.svg'
						: this.generateIcon(element),
					width: 50,
					height: 50,
					selected: selected,
				} as IconViewEntry);
			}
		});

		return iconViewEntries;
	}
}
