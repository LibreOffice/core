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
 * JSDialog.TreeView - tree view widget with or without header
 *
 * Example JSON:
 * a) without header
 * {
 *     id: 'id',
 *     type: 'treelistbox',
 *     entries: [
 *         { row: 0, text: 'first entry', collapsed: true, children: [ { row: 1, text: 'first subentry' } ] },
 *         { row: 2, text: 'second entry', selected: true, state: false, ondemand: true }
 *     ]
 *     hideIfEmpty: true // hide treelistbox if entries array is empty
 * }
 *
 * b) with headers
 * {
 *     id: 'id',
 *     type: 'treelistbox',
 *     headers: [ { text: 'first column' }, { text: 'second' }],
 *     entries: [
 *         { row: 0, columns [ { text: 'a' }, { collapsed: 'collapsedIcon.svg' }, { collapsedimage: '<BASE64 encoded PNG>' } ] },
 *         { row: 1, columns [ { link: 'http://example.com' }, { expanded: 'expandedIcon.svg' }, selected: true ]}
 *     ]
 * }
 *
 * c) with header and is a tree, not a list
 * {
 *     id: 'id',
 *     type: 'treelistbox',
 *     headers: [ { text: 'first column' }, { text: 'second' }],
 *     entries: [
 *         { row: 0, columns [ { text: 'a' }, { collapsed: 'collapsedIcon.svg' } ] },
 *         { row: 1, columns [ { text: 'a' }, { collapsed: 'collapsedIcon.svg' }, { expandedimage: '<BASE64 encoded PNG>' } ],
 * 			   children: [
 *                 { row: 2, columns [ { text: 'a2' }, { expanded: 'expandedIcon.svg' }, selected: true ]}
 *             ]
 *         },
 *     ]
 * }
 *
 * 'row' property is used in the callback to differentiate entries
 * 'state' property defines if entry has the checkbox (false/true), when is missing - no checkbox
 * 'enabled' property defines if entry checkbox is enabled
 * 'ondemand' property can be set to provide nodes lazy loading
 * 'collapsed' property means, this entry have childrens, but they are not visible, because
 *             this branch is collapsed.
 */

declare var JSDialog: any;

// TODO: remove this hack
var lastClickHelperRow: string | number = -1;
var lastClickHelperId = '';
// TODO: remove this hack

class TreeViewControl {
	_isRealTree: boolean;
	_isListbox: boolean;
	_containerRole: string;
	_container: HTMLElement;
	_tbody: HTMLElement;
	_thead: HTMLElement = null;
	_columns: number;
	_maxColumnsIncludingState: number = 0;
	_gridModeCache: boolean | undefined;
	_hasState: boolean;
	_hasIcon: boolean;
	_isNavigator: boolean;
	_singleClickActivate: boolean;
	_filterTimer: ReturnType<typeof setTimeout>;
	_rows: Map<string, HTMLElement>;
	readonly PAGE_ENTRY_PREFIX = '-$#~';
	readonly PAGE_ENTRY_SUFFIX = '~#$-';
	readonly PAGE_DIVIDER_ROW_CLASS = 'page-divider-row';

	// Lazy-evaluated: at construction time the container isn't in the
	// DOM yet, so closest() would fail.  Caches on first access.
	get _gridMode(): boolean {
		if (this._gridModeCache === undefined)
			this._gridModeCache = !!this._container.closest('#ChartDataDialog');
		return this._gridModeCache;
	}

	constructor(data: TreeWidgetJSON, builder: JSBuilder) {
		this._container = window.L.DomUtil.create(
			'div',
			builder.options.cssClass + ' ui-treeview',
		);
		this._container.id = data.id;
		this._rows = new Map<string, HTMLElement>();
		JSDialog.SetupA11yLabelForNonLabelableElement(
			this._container,
			data,
			builder,
		);
	}

	get Container() {
		return this._container;
	}

	static countColumns(data: TreeWidgetJSON) {
		if (!data.entries || !data.entries.length)
			return data.headers ? data.headers.length : 1;

		var maxColumns = 0;
		for (var e in data.entries) {
			const entry = data.entries[e];
			const count = entry.columns ? entry.columns.length : 0;
			if (count > maxColumns) maxColumns = count;
		}

		return maxColumns;
	}

	static hasState(data: TreeWidgetJSON) {
		for (var e in data.entries) {
			const entry = data.entries[e];
			if (entry.state !== undefined) return true;
		}

		return false;
	}

	static hasIcon(data: TreeWidgetJSON) {
		for (var e in data.entries) {
			const entry = data.entries[e];
			for (var i in entry.columns) {
				if (
					entry.columns[i].collapsed !== undefined ||
					entry.columns[i].expanded !== undefined ||
					entry.columns[i].collapsedimage !== undefined ||
					entry.columns[i].expandedimage !== undefined
				) {
					return true;
				}
			}
		}
		return false;
	}

	findEntryWithRow(
		entries: Array<TreeEntryJSON>,
		row: number | string,
	): TreeEntryJSON {
		for (const i in entries) {
			if (entries[i].row == row) return entries[i];
			else if (entries[i].children) {
				var found = this.findEntryWithRow(entries[i].children, row);
				if (found) return found;
			}
		}

		return null;
	}

	changeCheckboxStateOnClick(
		checkbox: HTMLInputElement,
		treeViewData: TreeWidgetJSON,
		builder: JSBuilder,
		entry: TreeEntryJSON,
	) {
		let foundEntry: TreeEntryJSON;
		if (checkbox.checked) {
			foundEntry = this.findEntryWithRow(treeViewData.entries, entry.row);
			if (foundEntry) checkbox.checked = foundEntry.state = true;
			builder.callback(
				'treeview',
				'change',
				treeViewData,
				{ row: entry.row, value: true },
				builder,
			);
		} else {
			foundEntry = this.findEntryWithRow(treeViewData.entries, entry.row);
			if (foundEntry) checkbox.checked = foundEntry.state = false;
			builder.callback(
				'treeview',
				'change',
				treeViewData,
				{ row: entry.row, value: false },
				builder,
			);
		}
	}

	createCheckbox(
		parent: HTMLElement,
		treeViewData: TreeWidgetJSON,
		builder: any,
		entry: TreeEntryJSON,
	) {
		const checkbox = window.L.DomUtil.create(
			'input',
			builder.options.cssClass + ' ui-treeview-checkbox',
			parent,
		);
		checkbox.id = `${treeViewData.id}-checkbox-${entry.row}`;
		checkbox.type = 'checkbox';
		checkbox.tabIndex = -1;

		if (entry.state === true) checkbox.checked = true;
		else checkbox.checked = false;

		return checkbox;
	}

	createRadioButton(
		parent: HTMLElement,
		treeViewData: TreeWidgetJSON,
		builder: JSBuilder,
		entry: TreeEntryJSON,
	) {
		const radioButton = window.L.DomUtil.create(
			'input',
			builder.options.cssClass + ' ui-treeview-checkbox',
			parent,
		);
		radioButton.id = `${treeViewData.id}-radio-${entry.row}`;
		radioButton.type = 'radio';
		radioButton.tabIndex = -1;

		if (entry.state === true) radioButton.checked = true;
		else radioButton.checked = false;

		return radioButton;
	}

	createSelectionElement(
		parent: HTMLElement,
		treeViewData: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
	) {
		let selectionElement: HTMLInputElement;
		const checkboxtype = treeViewData.checkboxtype;
		if (checkboxtype == 'radio') {
			selectionElement = this.createRadioButton(
				parent,
				treeViewData,
				builder,
				entry,
			);
		} else {
			selectionElement = this.createCheckbox(
				parent,
				treeViewData,
				builder,
				entry,
			);
		}

		if (entry.enabled === false) selectionElement.disabled = true;

		if (treeViewData.enabled !== false) {
			selectionElement.addEventListener('change', () => {
				this.changeCheckboxStateOnClick(
					selectionElement,
					treeViewData,
					builder,
					entry,
				);
			});
		}

		return selectionElement;
	}

	isSeparator(element: TreeColumnJSON) {
		if (!element.text) return false;
		return element.text.toLowerCase() === 'separator';
	}

	static isSingleClickActivate(treeViewData: TreeWidgetJSON) {
		return treeViewData.singleclickactivate === true;
	}

	isNavigator(data: TreeWidgetJSON) {
		return (
			data.id && typeof data.id === 'string' && data.id.startsWith('Navigator')
		);
	}

	getCellIconId(cellData: TreeColumnJSON) {
		let iconId = (
			cellData.collapsed ? cellData.collapsed : cellData.expanded
		) as string;
		const newLength = iconId.lastIndexOf('.');
		if (newLength > 0)
			iconId = (iconId.substr(0, newLength) as any).replaceAll('/', '');
		else iconId = (iconId as any).replaceAll('/', '');
		return iconId;
	}

	createImageColumn(
		parentContainer: HTMLElement,
		builder: JSBuilder,
		imageUrl: string,
	) {
		const image = window.L.DomUtil.create(
			'img',
			builder.options.cssClass +
				' ui-treeview-checkbox ui-treeview-image ui-decorative-image',
			parentContainer,
		);
		image.src = imageUrl;
		image.tabIndex = -1;
		image.alt = ''; //In this case, it is advisable to use an empty alt tag, as the information of the function is available in text form

		return image;
	}

	isExpanded(entry: TreeEntryJSON) {
		for (const i in entry.columns)
			if (entry.columns[i].expanded === true) return true;
		return false;
	}

	fillHeader(
		header: TreeHeaderJSON,
		builder: JSBuilder,
		data?: TreeWidgetJSON,
		columnIndex?: number,
	) {
		if (!header) return;

		const th = window.L.DomUtil.create(
			'div',
			builder.options.cssClass + ' ui-treeview-header',
			this._thead,
		);
		th.setAttribute('role', 'columnheader');

		if (header.headerName !== undefined) {
			const nameInput = window.L.DomUtil.create(
				'input',
				builder.options.cssClass + ' ui-treeview-header-name',
				th,
			) as HTMLInputElement;
			nameInput.type = 'text';
			nameInput.value = header.headerName;

			if (data && columnIndex !== undefined) {
				const commitEdit = () => {
					builder.callback(
						'treeview',
						'headernamechanged',
						data,
						{ column: columnIndex, value: nameInput.value },
						builder,
					);
				};
				nameInput.addEventListener('change', commitEdit);
				nameInput.addEventListener('keydown', (e: KeyboardEvent) => {
					if (e.key === 'Enter') {
						nameInput.blur();
					}
				});
			}
		}

		if (header.color) {
			const colorBar = window.L.DomUtil.create(
				'div',
				'ui-treeview-header-color',
				th,
			);
			colorBar.style.backgroundColor = '#' + header.color;
		}

		if (header.sortable !== false) {
			const button = window.L.DomUtil.create(
				'button',
				builder.options.cssClass + ' ui-treeview-header-button',
				th,
			);
			if (data && columnIndex !== undefined) {
				button.id = data.id + '-header-' + columnIndex + '-button';
			}
			button.textContent = header.text;
			if (header.arrow) {
				th.setAttribute(
					'aria-sort',
					header.arrow === 'up' ? 'ascending' : 'descending',
				);
			}
			const icon = window.L.DomUtil.create(
				'span',
				builder.options.cssClass + ' ui-treeview-header-sort-icon',
				button,
			);
			icon.setAttribute('aria-hidden', 'true');
			if (header.arrow) window.L.DomUtil.addClass(icon, header.arrow);
		} else {
			const span = window.L.DomUtil.create(
				'span',
				builder.options.cssClass + ' ui-treeview-header-text',
				th,
			);
			span.innerText = header.text;
		}
	}

	fillRow(
		data: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
		level: number,
		parent: HTMLElement,
	): HTMLElement {
		var highlight = false;
		if (data.highlightTerm && data.highlightTerm.trim().length > 0) {
			highlight =
				entry.text &&
				entry.text.toLowerCase().indexOf(data.highlightTerm.toLowerCase()) >= 0;
		}
		const tr: HTMLElement = window.L.DomUtil.create(
			'div',
			builder.options.cssClass +
				' ui-treeview-entry' +
				(highlight ? ' highlighted' : ''),
			parent,
		);
		this._rows.set(String(entry.row), tr);

		//id is needed to find the element to regain focus after widget is updated. see updateWidget in Control.JSDialogBuilder.js
		tr.id = data.id + '_' + entry.row;
		tr.setAttribute('level', String(level));
		(tr as any)._row = entry.row;
		const rowRole =
			this._containerRole === 'tree'
				? 'treeitem'
				: this._containerRole === 'listbox'
					? 'option'
					: 'row';
		tr.setAttribute('role', rowRole);

		let dummyColumns = 0;
		if (this._hasState) dummyColumns++;
		tr.style.gridColumn = '1 / ' + (this._columns + dummyColumns + 1);
		if (this._columns + dummyColumns + 1 > this._maxColumnsIncludingState) {
			this._maxColumnsIncludingState = this._columns + dummyColumns + 1;
		}
		if (
			this.isPageDivider(entry, this.PAGE_ENTRY_PREFIX, this.PAGE_ENTRY_SUFFIX)
		) {
			window.L.DomUtil.addClass(tr, this.PAGE_DIVIDER_ROW_CLASS);
		} else {
			tr.setAttribute('tabindex', '-1');
		}

		let selectionElement;
		if (this._hasState) {
			const td = window.L.DomUtil.create('div', '', tr);
			selectionElement = this.createSelectionElement(td, data, entry, builder);
			if (this._isRealTree) td.setAttribute('aria-level', level);
		}

		this.fillCells(entry, builder, data, tr, level, selectionElement);

		this.setupRowProperties(tr, entry, level, selectionElement);
		this.setupRowDragAndDrop(tr, data, entry, builder);

		return tr;
	}

	highlightAllTreeViews(highlight: boolean) {
		if (highlight) {
			document.querySelectorAll('.ui-treeview').forEach((item) => {
				window.L.DomUtil.addClass(item, 'droptarget');
			});
		} else {
			document.querySelectorAll('.ui-treeview').forEach((item) => {
				window.L.DomUtil.removeClass(item, 'droptarget');
			});
		}
	}

	setupDragAndDrop(treeViewData: TreeWidgetJSON, builder: JSBuilder) {
		if (treeViewData.enabled !== false) {
			this._container.ondrop = (ev) => {
				ev.preventDefault();
				var row = ev.dataTransfer.getData('text');
				builder.callback('treeview', 'dragend', treeViewData, row, builder);
				this.highlightAllTreeViews(false);
			};
			this._container.ondragover = (event) => {
				event.preventDefault();
			};
		}
	}

	setupRowDragAndDrop(
		tr: HTMLElement,
		treeViewData: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
	) {
		if (treeViewData.enabled !== false && entry.state == null) {
			tr.draggable = treeViewData.draggable === false ? false : true;

			tr.ondragstart = (ev) => {
				ev.dataTransfer.setData('text', '' + entry.row);
				builder.callback(
					'treeview',
					'dragstart',
					treeViewData,
					entry.row,
					builder,
				);

				this.highlightAllTreeViews(true);
			};

			tr.ondragend = () => {
				this.highlightAllTreeViews(false);
			};
			tr.ondragover = (event) => {
				event.preventDefault();
			};
		}
	}

	setupRowProperties(
		tr: HTMLElement,
		entry: TreeEntryJSON,
		level: number,
		selectionElement: HTMLInputElement,
	) {
		if (entry.children) tr.setAttribute('aria-expanded', 'true');

		if (level !== undefined && this._isRealTree)
			tr.setAttribute('aria-level', '' + level);

		if (entry.selected === true) this.selectEntry(tr, selectionElement, false);

		const disabled = entry.enabled === false;
		if (disabled) window.L.DomUtil.addClass(tr, 'disabled');

		if (entry.ondemand || entry.collapsed) {
			window.L.DomUtil.addClass(tr, 'collapsed');
			tr.setAttribute('aria-expanded', 'false');
		}
	}

	private setupCellTooltip(cell: HTMLElement, map: MapInterface) {
		app.layoutingService.onDrain(() => {
			if (cell.scrollWidth > cell.clientWidth) {
				cell.dataset.cooltip = cell.innerText;
				window.L.control.attachTooltipEventListener(cell, map);
			}
		});
	}

	createExpandableIconCell(
		parent: HTMLElement,
		entry: TreeEntryJSON,
		index: any,
		builder: JSBuilder,
	) {
		const icon = window.L.DomUtil.create(
			'img',
			'ui-treeview-icon ui-decorative-image',
			parent,
		);

		if (this._isNavigator) icon.draggable = false;

		const iconId = this.getCellIconId(entry.columns[index]);
		window.L.DomUtil.addClass(icon, iconId + 'img');
		const iconName = app.LOUtil.getIconNameOfCommand(iconId, true);
		app.LOUtil.setImage(icon, iconName, builder.map);
		icon.tabIndex = -1;
		icon.alt = ''; //In this case, it is advisable to use an empty alt tag for the icons, as the information of the function is available in text form
	}

	createTextCell(
		treeViewData: TreeWidgetJSON,
		parent: HTMLElement,
		entry: TreeEntryJSON,
		index: any,
		selectionElement: HTMLInputElement,
		builder: JSBuilder,
	) {
		const text =
			entry.columns[index].text !== undefined
				? builder._cleanText(entry.columns[index].text)
				: builder._cleanText(entry.text);

		const hasRenderer =
			entry.columns[index].customEntryRenderer ||
			treeViewData.customEntryRenderer;
		const hasCache = hasRenderer && builder.rendersCache[treeViewData.id];
		const hasCachedImage =
			hasCache && builder.rendersCache[treeViewData.id].images[entry.row];
		let cell = null;

		if (hasCachedImage) {
			// if we rendered entry in the past already
			const image = builder.rendersCache[treeViewData.id].images[entry.row];
			cell = window.L.DomUtil.create(
				'span',
				builder.options.cssClass +
					` ui-treeview-cell-text ui-treeview-cell-text-content ui-treeview-${entry.row}-${index}`,
				parent,
			);
			const img = window.L.DomUtil.create(
				'img',
				'ui-treeview-custom-render',
				cell,
			);
			img.src = image;
			img.alt = text;
			cell = img; // use as placeholder only the image
		} else if (
			this.isPageDivider(entry, this.PAGE_ENTRY_PREFIX, this.PAGE_ENTRY_SUFFIX)
		) {
			// first time we want to render the entry
			cell = window.L.DomUtil.create(
				'span',
				builder.options.cssClass +
					` ui-treeview-cell-text-content page-divider`,
				parent,
			);
			cell.innerText = this.getPageEntryText(
				entry.text,
				this.PAGE_ENTRY_PREFIX,
				this.PAGE_ENTRY_SUFFIX,
			);
			this.setupCellTooltip(cell, builder.map);
		} else if (treeViewData.highlightTerm !== undefined) {
			cell = this.createHighlightedCell(
				parent,
				entry,
				index,
				builder,
				treeViewData.highlightTerm,
			);
		} else {
			const elementType = selectionElement ? 'label' : 'span';
			cell = window.L.DomUtil.create(
				elementType,
				builder.options.cssClass +
					` ui-treeview-cell-text-content ui-treeview-${entry.row}-${index}`,
				parent,
			);
			if (selectionElement) {
				cell.setAttribute('for', selectionElement.id);
			}
			cell.innerText = text;
			this.setupCellTooltip(cell, builder.map);
		}

		// in case of non-persistent entries we want to re-render in case of change
		if (hasRenderer) {
			JSDialog.OnDemandRenderer(
				builder,
				treeViewData.id,
				'treeview',
				entry.row,
				cell,
				parent,
				entry.text,
			);
		}
	}

	/**
	 * Creates a partially highlighted cell for the tree view.
	 * Highlighted part of text corresponds to where core has marked a hit
	 *
	 * e.g. searching for 'line', core sends the following:
	 * 		Collabora On[line],
	 * 		https://www.collaboraon[line].com,
	 * 		des graphiques spark[line],
	 *      etc.
	 */
	createHighlightedCell(
		parent: HTMLElement,
		entry: TreeEntryJSON,
		index: any,
		builder: JSBuilder,
		searchTerm: string,
	) {
		const sourceText = entry.text;

		const searchPattern = `[${searchTerm}]`;
		const mainSpan = window.L.DomUtil.create(
			'span',
			builder.options.cssClass + ` ui-treeview-cell-text-content`,
			parent,
		);

		const fragments = this.caseInsensitiveSplit(sourceText, searchPattern);
		if (fragments.length === 1) {
			/// not found
			mainSpan.appendChild(document.createTextNode(sourceText));
		} else {
			// found, can be many times
			for (let i = 0; i < fragments.length - 1; i += 2) {
				mainSpan.appendChild(document.createTextNode(fragments[i])); // pre
				const highlightSpan = window.L.DomUtil.create(
					'span',
					builder.options.cssClass + ' highlighted',
					mainSpan,
				);
				highlightSpan.innerText = fragments[i + 1].substring(
					1,
					fragments[i + 1].length - 1,
				);
				mainSpan.appendChild(highlightSpan);
			}
			mainSpan.appendChild(
				document.createTextNode(fragments[fragments.length - 1]),
			); // post
		}

		this.setupCellTooltip(mainSpan, builder.map);
		return mainSpan;
	}

	isPageDivider(
		entry: TreeEntryJSON,
		pageEntryPrefix: string,
		pageEntrySuffix: string,
	): boolean {
		// Matches page divider prefix and suffix: -$#~ PAGE ~#$- as set in core: QuickFindPanel::FillSearchFindsList() (QuickFindPanel.cxx)
		return (
			entry.text &&
			entry.text.startsWith(pageEntryPrefix) &&
			entry.text.endsWith(pageEntrySuffix)
		);
	}

	getPageEntryText(
		text: string,
		pageEntryPrefix: string,
		pageEntrySuffix: string,
	): string {
		return text.substring(
			pageEntryPrefix.length,
			text.length - pageEntrySuffix.length,
		);
	}

	caseInsensitiveSplit(text: string, delimeter: string) {
		// escape regex special chars
		const escapedPattern = delimeter.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
		// '()' indicate keeping the delimeter, g:global, i:insensitive
		const regex = new RegExp(`(${escapedPattern})`, 'gi');

		return text.split(regex);
	}

	createLinkCell(
		parent: HTMLElement,
		entry: TreeEntryJSON,
		index: any,
		builder: JSBuilder,
	) {
		const cell = window.L.DomUtil.create(
			'span',
			builder.options.cssClass + ' ui-treeview-cell-text',
			parent,
		);
		const link = window.L.DomUtil.create('a', '', cell);
		link.href = entry.columns[index].link || entry.columns[index].text;
		link.innerText = entry.columns[index].text || entry.text;
		link.target = '_blank';
		link.rel = 'noopener';
		// WCAG 3.2.5: without altering what sighted users see.
		const srOnly = window.L.DomUtil.create('span', 'visuallyhidden', link);
		srOnly.innerText = ' ' + _('(opens in new tab)');
	}

	fillCells(
		entry: TreeEntryJSON,
		builder: JSBuilder,
		treeViewData: TreeWidgetJSON,
		tr: HTMLElement,
		level: number,
		selectionElement: HTMLInputElement,
	) {
		let td, expander, span, text, img;

		const rowElements = [];

		// row is a separator
		if (this.isSeparator(entry))
			window.L.DomUtil.addClass(tr, 'context-menu-separator');

		// column for expander
		if (this._isRealTree) {
			td = window.L.DomUtil.create('div', 'ui-treeview-expander-column', tr);
			rowElements.push(td);

			if (entry.children && entry.children.length)
				expander = window.L.DomUtil.create(
					'div',
					builder.options.cssClass + ' ui-treeview-expander',
					td,
				);
		}

		// regular columns
		for (const index in entry.columns) {
			td = window.L.DomUtil.create('div', '', tr);
			rowElements.push(td);
			if (
				parseInt(index) === 0 &&
				(this._containerRole === 'grid' || this._containerRole === 'treegrid')
			) {
				window.L.DomUtil.addClass(td, 'ui-treeview-rowheader');
				td.setAttribute('role', 'rowheader');
			}

			span = window.L.DomUtil.create(
				'span',
				builder.options.cssClass + ' ui-treeview-cell',
				td,
			);
			text = window.L.DomUtil.create(
				'span',
				builder.options.cssClass + ' ui-treeview-cell-text',
				span,
			);

			if (entry.text == '<dummy>') continue;

			img = entry.columns[index].collapsedimage
				? entry.columns[index].collapsedimage
				: entry.columns[index].expandedimage;
			if (img) {
				window.L.DomUtil.addClass(td, 'ui-treeview-icon-column');
				this.createImageColumn(text, builder, img);
			} else if (
				entry.columns[index].collapsed ||
				entry.columns[index].expanded
			) {
				window.L.DomUtil.addClass(td, 'ui-treeview-icon-column');
				window.L.DomUtil.addClass(span, 'ui-treeview-expandable-with-icon');
				this.createExpandableIconCell(text, entry, index, builder);
			} else if (
				entry.columns[index].link &&
				!this.isSeparator(entry.columns[index])
			) {
				this.createLinkCell(text, entry, index, builder);
			} else if (
				entry.columns[index].text !== undefined &&
				!this.isSeparator(entry.columns[index])
			) {
				this.createTextCell(
					treeViewData,
					text,
					entry,
					index,
					selectionElement,
					builder,
				);
			}

			// row sub-elements
			for (const i in rowElements) {
				const element = rowElements[i];

				// setup properties
				if (
					this._containerRole === 'grid' ||
					this._containerRole === 'treegrid'
				) {
					if (!element.getAttribute('role'))
						element.setAttribute('role', 'gridcell');
				}
			}
		}

		if (
			!this.isPageDivider(entry, this.PAGE_ENTRY_PREFIX, this.PAGE_ENTRY_SUFFIX)
		) {
			// setup callbacks
			var clickFunction = this.createClickFunction(
				tr,
				selectionElement,
				true,
				this._singleClickActivate,
				builder,
				treeViewData,
				entry,
			);
			var doubleClickFunction = this.createClickFunction(
				tr,
				selectionElement,
				false,
				true,
				builder,
				treeViewData,
				entry,
			);

			this.setupEntryMouseEvents(
				tr,
				entry,
				treeViewData,
				builder,
				selectionElement,
				expander,
				clickFunction,
				doubleClickFunction,
			);

			this.setupEntryKeyboardEvents(
				tr,
				entry,
				selectionElement,
				expander,
				clickFunction,
				builder,
				treeViewData,
			);

			this.setupEntryContextMenuEvent(tr, entry, treeViewData, builder);
		}
	}

	setupEntryContextMenuEvent(
		tr: HTMLElement,
		entry: TreeEntryJSON,
		treeViewData: TreeWidgetJSON,
		builder: JSBuilder,
	) {
		tr.addEventListener('contextmenu', (e: Event) => {
			builder.callback(
				'treeview',
				'contextmenu',
				treeViewData,
				entry.row,
				builder,
			);
			e.preventDefault();
		});
	}

	setupEntryMouseEvents(
		tr: HTMLElement,
		entry: TreeEntryJSON,
		treeViewData: TreeWidgetJSON,
		builder: JSBuilder,
		selectionElement: HTMLInputElement,
		expander: HTMLElement,
		clickFunction: any,
		doubleClickFunction: any,
	) {
		tr.addEventListener('click', clickFunction as any);

		if (!this._singleClickActivate) {
			if (window.ThisIsTheiOSApp) {
				// TODO: remove this hack
				tr.addEventListener('click', (event) => {
					if (window.L.DomUtil.hasClass(tr, 'disabled')) return;

					if (
						entry.row == lastClickHelperRow &&
						treeViewData.id == lastClickHelperId
					)
						doubleClickFunction(event);
					else {
						lastClickHelperRow = entry.row;
						lastClickHelperId = treeViewData.id;
						setTimeout(() => {
							lastClickHelperRow = -1;
						}, 300);
					}
				});
				// TODO: remove this hack
			} else {
				$(tr).dblclick(doubleClickFunction as any);
			}
		}

		const toggleFunction = (e: MouseEvent) => {
			this.toggleEntry(tr, treeViewData, entry, builder);
			e.preventDefault();
		};
		const expandFunction = (e: MouseEvent) => {
			this.expandEntry(tr, treeViewData, entry, builder);
			e.preventDefault();
		};

		if (expander && entry.children && entry.children.length) {
			if (entry.ondemand) {
				window.L.DomEvent.on(expander, 'click', expandFunction);
			} else {
				$(expander).click((e) => {
					if (entry.state && e.target === selectionElement) e.preventDefault(); // do not toggle on checkbox
					toggleFunction(e.originalEvent);
				});
			}
		}
	}

	selectParentEntry(tr: HTMLElement, builder: JSBuilder, data: TreeWidgetJSON) {
		const expandedContent = tr.parentElement;
		if (
			!expandedContent ||
			!expandedContent.classList.contains('ui-treeview-expanded-content')
		)
			return;

		const parentEntry = expandedContent.previousElementSibling as HTMLElement;
		if (parentEntry && parentEntry.classList.contains('ui-treeview-entry')) {
			const listElements = Array.from(
				this._container.querySelectorAll(
					`.ui-treeview-entry:not(.${this.PAGE_DIVIDER_ROW_CLASS})`,
				),
			) as Array<HTMLElement>;
			const fromIndex = listElements.indexOf(tr);
			const toIndex = listElements.indexOf(parentEntry);
			if (toIndex >= 0)
				this.changeFocusedRow(listElements, fromIndex, toIndex, builder, data);
		}
	}

	selectFirstChild(tr: HTMLElement, builder: JSBuilder, data: TreeWidgetJSON) {
		const expandedContent = tr.nextElementSibling as HTMLElement;
		if (
			!expandedContent ||
			!expandedContent.classList.contains('ui-treeview-expanded-content')
		)
			return;

		const firstChild = expandedContent.querySelector(
			'.ui-treeview-entry',
		) as HTMLElement;

		if (firstChild) {
			const listElements = Array.from(
				this._container.querySelectorAll(
					`.ui-treeview-entry:not(.${this.PAGE_DIVIDER_ROW_CLASS})`,
				),
			) as Array<HTMLElement>;
			const fromIndex = listElements.indexOf(tr);
			const toIndex = listElements.indexOf(firstChild);
			if (toIndex >= 0)
				this.changeFocusedRow(listElements, fromIndex, toIndex, builder, data);
		}
	}

	setupEntryKeyboardEvents(
		tr: HTMLElement,
		entry: TreeEntryJSON,
		selectionElement: HTMLInputElement,
		expander: HTMLElement,
		clickFunction: any,
		builder: JSBuilder,
		data: TreeWidgetJSON,
	) {
		if (entry.enabled === false) return;

		tr.addEventListener('keydown', (event) => {
			let preventDef = false;

			if (event.key === ' ' && expander) {
				expander.click();
				preventDef = true;
			} else if (event.key === 'Enter' || event.key === ' ') {
				clickFunction(event);
				if (selectionElement) selectionElement.click();
				if (expander) {
					expander.click();
				}
				preventDef = true;
			} else if (event.key === 'ArrowLeft') {
				// Always collapse if expanded
				if (expander && !window.L.DomUtil.hasClass(tr, 'collapsed')) {
					expander.click();
				} else {
					this.selectParentEntry(tr, builder, data);
				}
				preventDef = true;
			} else if (event.key === 'ArrowRight') {
				// Always expand if collapsed
				if (expander && window.L.DomUtil.hasClass(tr, 'collapsed')) {
					expander.click();
				} else if (expander && !window.L.DomUtil.hasClass(tr, 'collapsed')) {
					this.selectFirstChild(tr, builder, data);
				}
				preventDef = true;
			} else if (event.key === 'Tab') {
				if (!window.L.DomUtil.hasClass(tr, 'selected')) this.unselectEntry(tr);
			}

			if (preventDef) {
				event.preventDefault();
				event.stopPropagation();
			}
		});
	}

	toggleEntry(
		span: HTMLElement,
		treeViewData: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
	) {
		if (entry.enabled === false) return;

		if (window.L.DomUtil.hasClass(span, 'collapsed')) {
			builder.callback('treeview', 'expand', treeViewData, entry.row, builder);
			span.setAttribute('aria-expanded', 'true');
		} else {
			builder.callback(
				'treeview',
				'collapse',
				treeViewData,
				entry.row,
				builder,
			);
			span.setAttribute('aria-expanded', 'false');
		}
		$(span).toggleClass('collapsed');
	}

	expandEntry(
		span: HTMLElement,
		treeViewData: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
	) {
		if (entry.enabled === false) return;

		if (entry.ondemand && window.L.DomUtil.hasClass(span, 'collapsed'))
			builder.callback('treeview', 'expand', treeViewData, entry.row, builder);
		$(span).toggleClass('collapsed');
		if (window.L.DomUtil.hasClass(span, 'collapsed'))
			span.setAttribute('aria-expanded', 'false');
		else span.setAttribute('aria-expanded', 'true');
	}

	selectEntry(
		span: HTMLElement,
		checkbox: HTMLInputElement,
		shouldFocus: boolean = false,
	) {
		window.L.DomUtil.addClass(span, 'selected');
		span.setAttribute('aria-selected', 'true');
		span.tabIndex = 0;
		if (shouldFocus) span.focus();

		if (checkbox) checkbox.removeAttribute('tabindex');
	}

	selectEntryByRow(row: number, shouldFocus: boolean = false) {
		// Don't change selection while inline cell editing is active:
		// it would blur the input and discard the edit in progress.
		if (this._container.querySelector('.ui-treeview-inline-edit')) return;

		const rowElement = this._rows.get(String(row));
		if (!rowElement) {
			console.warn('TreeView onSelect: row "' + row + '" not found');
			return;
		}

		// Remember if the focused element is inside this treeview,
		// because clearing selections removes tabindex which drops
		// focus to BODY for non-natively-focusable elements.
		const hadFocus = this._container.contains(document.activeElement);

		// Clear existing selections
		this._container
			.querySelectorAll('.ui-treeview-entry.selected')
			.forEach((item: HTMLElement) => {
				this.unselectEntry(item);
			});

		// Select the target row
		const checkbox = rowElement.querySelector('input') as HTMLInputElement;
		this.selectEntry(rowElement, checkbox, shouldFocus || hadFocus);
	}

	unselectEntry(item: HTMLElement) {
		window.L.DomUtil.removeClass(item, 'selected');
		item.removeAttribute('aria-selected');
		item.tabIndex = -1;
		var itemCheckbox = item.querySelector('input');
		if (itemCheckbox) itemCheckbox.tabIndex = -1;
	}

	createClickFunction(
		parentContainer: HTMLElement,
		checkbox: HTMLInputElement,
		select: boolean,
		activate: boolean,
		builder: JSBuilder,
		treeViewData: TreeWidgetJSON,
		entry: TreeEntryJSON,
	) {
		return (e: MouseEvent | KeyboardEvent) => {
			if (e && e.target === checkbox) return; // allow default handler to trigger change event

			if (e && window.L.DomUtil.hasClass(parentContainer, 'disabled')) {
				e.preventDefault();
				return;
			}

			this._container
				.querySelectorAll('.ui-treeview-entry.selected')
				.forEach((item: HTMLElement) => {
					this.unselectEntry(item);
				});

			this.selectEntry(parentContainer, checkbox, true);
			if (checkbox && (!e || e.target === checkbox))
				this.changeCheckboxStateOnClick(checkbox, treeViewData, builder, entry);

			let cell: Element = this.getTextCellForElement(e.target as Element);

			let column: number | null | undefined;
			let editable: boolean = false;
			if (cell) {
				column = this.getColumnForCell(entry, cell);
				editable = this.canEdit(entry, column);
			}

			// Fallback: determine column from click position when the
			// target is the row itself (empty cells have zero-size spans).
			// Only needed in grid mode where empty cells must be clickable.
			if (column == null && e instanceof MouseEvent && this._gridMode) {
				const row = parentContainer;
				const cells = row.querySelectorAll(
					':scope > [role="gridcell"], :scope > [role="rowheader"]',
				);
				for (let i = 0; i < cells.length; i++) {
					const rect = cells[i].getBoundingClientRect();
					if (e.clientX >= rect.left && e.clientX < rect.right) {
						column = i;
						editable = this.canEdit(entry, column);
						const spans = cells[i].getElementsByClassName(
							'ui-treeview-cell-text-content',
						);
						if (spans.length === 1) cell = spans[0] as Element;
						break;
					}
				}
			}

			if (select) {
				// Only send {row, col} in grid mode;
				// all other consumers expect a plain row number.
				const selectData =
					column != null && this._gridMode
						? { row: entry.row, col: column }
						: entry.row;
				builder.callback(
					'treeview',
					'select',
					treeViewData,
					selectData,
					builder,
				);
			}

			// Highlight selected column header
			if (column != null && this._thead) {
				this._thead
					.querySelectorAll('.ui-treeview-header')
					.forEach((h: Element) => h.classList.remove('selected'));
				const headers = this._thead.querySelectorAll('.ui-treeview-header');
				if (headers[column]) headers[column].classList.add('selected');
			}

			// Inline editing is only supported in grid mode.
			// All other consumers treat cells as read-only.
			const inlineEditable = editable && this._gridMode;

			if (!inlineEditable && activate)
				builder.callback(
					'treeview',
					'activate',
					treeViewData,
					entry.row,
					builder,
				);

			if (inlineEditable && activate && cell)
				this.startEditing(
					builder,
					cell,
					column,
					entry,
					parentContainer,
					treeViewData,
				);
		};
	}

	getTextCellForElement(element: Element): Element {
		const textCells = Array.from(
			element.getElementsByClassName('ui-treeview-cell-text-content'),
		);

		if (element.classList.contains('ui-treeview-cell-text-content')) {
			textCells.push(element);
		}

		if (textCells.length !== 1) {
			return null;
		}

		const cell = textCells[0];

		return cell;
	}

	getColumnForCell(entry: TreeEntryJSON, cell: Element): number | null {
		let column: number | undefined;
		for (const className of Array.from(cell.classList)) {
			const prefix = `ui-treeview-${entry.row}-`;
			if (className.startsWith(prefix)) {
				column = parseInt(className.slice(prefix.length));
			}
		}
		if (column === undefined || Number.isNaN(column)) {
			return null;
		}
		if (column >= entry.columns.length) {
			return null;
		}

		return column;
	}

	canEdit(entry: TreeEntryJSON, column: number | null): boolean {
		if (column === null || entry.columns[column].text === undefined) {
			return false;
		}

		return !!entry.columns[column].editable;
	}

	startEditing(
		builder: JSBuilder,
		cell: Element,
		column: number,
		entry: TreeEntryJSON,
		parentContainer: HTMLElement,
		treeViewData: TreeWidgetJSON,
	): void {
		// Hide existing text but keep it in the layout so column widths don't shift
		for (const child of Array.from(cell.childNodes)) {
			if (child instanceof HTMLElement) child.style.visibility = 'hidden';
			else if (child.nodeType === Node.TEXT_NODE) {
				const wrapper = document.createElement('span');
				wrapper.style.visibility = 'hidden';
				wrapper.textContent = child.textContent;
				child.replaceWith(wrapper);
			}
		}

		const rowShouldBeDraggable = parentContainer.draggable; // TODO: does this work with tree views or only tables?

		// Use the gridcell div as positioning context so the input
		// has consistent size regardless of text content
		const gridCell = (cell as HTMLElement).closest(
			'[role="gridcell"], [role="rowheader"]',
		) as HTMLElement;
		const posParent = gridCell || (cell as HTMLElement);
		posParent.style.position = 'relative';

		const input = document.createElement('input');
		input.className = 'ui-treeview-inline-edit';

		input.value = entry.columns[column].text;

		input.enterKeyHint = 'done';

		let cancelledUpdate = false;

		input.addEventListener(
			'keydown',
			(e) => {
				if (e.code === 'Enter') {
					input.blur();
				} else if (e.code === 'Escape') {
					cancelledUpdate = true;
					input.blur();
				}
				e.stopImmediatePropagation(); // We need events to type and with some keys that doesn't happen (e.g. space which selects a different cell)
			},
			{ capture: true },
		);
		const conflictingEventTypes = ['click', 'dblclick'];
		for (const eventType of conflictingEventTypes) {
			input.addEventListener(eventType, (e) => {
				e.stopPropagation();
			});
		}
		input.addEventListener('blur', () => {
			this.endEditing(
				builder,
				cancelledUpdate,
				cell,
				column,
				entry,
				input,
				parentContainer,
				posParent,
				rowShouldBeDraggable,
				treeViewData,
			);
		});

		parentContainer.draggable = false;
		(
			parentContainer.parentElement as HTMLElement & { onFocus?: () => void }
		).onFocus = () => {
			/* no-op */
		};
		// We need to cancel focus events - which are used when we select - or we will blur our input and stop editing
		// The grab_focus is on the grid we're already in - i.e. we're not changing anything about what is being selected - so there is no need to re-do a selection/etc. once editing is done

		posParent.appendChild(input);
		input.focus();
	}

	endEditing(
		builder: JSBuilder,
		cancelledUpdate: boolean,
		cell: Element,
		column: number,
		entry: TreeEntryJSON,
		input: HTMLInputElement,
		parentContainer: HTMLElement,
		posParent: HTMLElement,
		rowShouldBeDraggable: boolean,
		treeViewData: TreeWidgetJSON,
	) {
		parentContainer.draggable = rowShouldBeDraggable;
		(
			parentContainer.parentElement as HTMLElement & { onFocus?: () => void }
		).onFocus = undefined;

		posParent.style.position = '';
		input.remove();

		const newValue = cancelledUpdate ? entry.columns[column].text : input.value;

		// cell is the ui-treeview-cell-text-content span — replace its
		// contents with the new value, removing the visibility-hidden
		// wrapper spans that startEditing created.
		cell.textContent = newValue;

		if (cancelledUpdate) return;

		builder.callback(
			'treeview',
			'editend',
			treeViewData,
			{ row: entry.row, column, value: input.value },
			builder,
		);
	}

	filterEntries(filter: string) {
		if (this._filterTimer) clearTimeout(this._filterTimer);

		var entriesToHide: Array<HTMLElement> = [];
		var allEntries = this._container.querySelectorAll('.ui-treeview-entry');

		filter = filter.trim();

		allEntries.forEach((entry: HTMLElement) => {
			if (filter === '') return;

			var cells = entry.querySelectorAll('div');
			for (var i in cells) {
				var entryText = cells[i].innerText;
				if (
					entryText &&
					entryText.toLowerCase().indexOf(filter.toLowerCase()) >= 0
				) {
					return;
				}
			}

			entriesToHide.push(entry);
		});

		this._filterTimer = setTimeout(() => {
			allEntries.forEach((entry) => {
				window.L.DomUtil.removeClass(entry, 'hidden');
			});
			entriesToHide.forEach((entry) => {
				window.L.DomUtil.addClass(entry, 'hidden');
			});
		}, 100);
	}

	highlightEntries(searchTerm: string) {
		app.layoutingService.appendLayoutingTask(() => {
			var entriesToHighlight: Array<HTMLElement> = [];
			var allEntries = this._container.querySelectorAll('.ui-treeview-entry');

			searchTerm = searchTerm.trim();

			allEntries.forEach((entry: HTMLElement) => {
				if (searchTerm === '') return;

				var cells = entry.querySelectorAll('div');
				for (var i in cells) {
					var entryText = cells[i].innerText;
					if (
						entryText &&
						entryText.toLowerCase().indexOf(searchTerm.toLowerCase()) >= 0
					) {
						entriesToHighlight.push(entry);
					}
				}

				return;
			});

			allEntries.forEach((entry) => {
				window.L.DomUtil.removeClass(entry, 'highlighted');
			});
			entriesToHighlight.forEach((entry) => {
				window.L.DomUtil.addClass(entry, 'highlighted');
			});
		});
	}

	setupKeyEvents(data: TreeWidgetJSON, builder: JSBuilder) {
		this._container.addEventListener('keydown', (event) => {
			const listElements = this._container.querySelectorAll(
				`.ui-treeview-entry:not(.${this.PAGE_DIVIDER_ROW_CLASS})`,
			);
			this.handleKeyEvent(event, listElements, builder, data);
		});
	}

	setupFocusOutHandler() {
		this._container.addEventListener('focusout', (event) => {
			app.layoutingService.appendLayoutingTask(() => {
				const activeElement = document.activeElement as HTMLElement;
				const isFocusInTreeView =
					activeElement && this._container.contains(activeElement);

				if (!isFocusInTreeView) {
					const listElements = this._container.querySelectorAll(
						`.ui-treeview-entry:not(.${this.PAGE_DIVIDER_ROW_CLASS})`,
					);
					this.restoreInitialTabIndexes(
						Array.from(listElements) as Array<HTMLElement>,
					);
				}
			});
		});
	}

	restoreInitialTabIndexes(listElements: Array<HTMLElement>) {
		var selectedEntry: HTMLElement = null;
		listElements.forEach((entry: HTMLElement) => {
			if (entry.classList.contains('selected')) selectedEntry = entry;
			entry.tabIndex = -1;
		});
		if (selectedEntry) selectedEntry.tabIndex = 0;
		else if (listElements.length > 0) listElements[0].tabIndex = 0;
	}

	changeFocusedRow(
		listElements: Array<HTMLElement>,
		fromIndex: number,
		toIndex: number,
		builder: JSBuilder,
		data: TreeWidgetJSON,
	) {
		var nextElement = listElements.at(toIndex);
		nextElement.tabIndex = 0;
		nextElement.focus();

		if (data.serverSyncSelection !== false) {
			(builder as any).callback(
				'treeview',
				'select',
				data,
				(nextElement as any)._row,
				builder,
			);
		}

		// Update tabindex so the new entry is in the tab order and the
		// old one is removed. Selected entries keep their tabindex so
		// they remain reachable via Tab.
		var nextInput = Array.from(
			listElements
				.at(toIndex)
				.querySelectorAll('.ui-treeview-entry > div > input'),
		) as Array<HTMLElement>;
		if (nextInput && nextInput.length)
			nextInput.at(0).removeAttribute('tabindex');

		if (fromIndex >= 0) {
			var oldElement = listElements.at(fromIndex);
			oldElement.tabIndex = -1;
			var oldInput = Array.from(
				listElements
					.at(fromIndex)
					.querySelectorAll('.ui-treeview-entry > div > input'),
			) as Array<HTMLElement>;
			if (oldInput && oldInput.length) oldInput.at(0).tabIndex = -1;
		}
	}

	getCurrentEntry(listElements: Array<HTMLElement>) {
		var focusedElement = document.activeElement as HTMLElement;
		// tr - row itself
		var currIndex = listElements.indexOf(focusedElement);
		// input - child of a row
		if (currIndex < 0)
			currIndex = listElements.indexOf(
				focusedElement.parentNode.parentNode as HTMLElement,
			);
		// no focused entry - try with selected one
		if (currIndex < 0) {
			var selected = listElements.filter((o) => {
				return o.classList.contains('selected');
			});
			if (selected && selected.length)
				currIndex = listElements.indexOf(selected[0]);
		}
		if (currIndex < 0) {
			for (var i in listElements) {
				var parent = listElements[i].parentNode;

				if (parent) parent = parent.parentNode;
				else break;

				if (parent && window.L.DomUtil.hasClass(parent, 'selected')) {
					currIndex = listElements.indexOf(listElements[i]);
					break;
				}
			}
		}

		return currIndex;
	}

	typeAheadSearch(
		listElements: Array<HTMLElement>,
		currIndex: number,
		char: string,
		builder: JSBuilder,
		data: TreeWidgetJSON,
	) {
		const lowerChar = char.toLowerCase();
		const startIndex = currIndex >= 0 ? currIndex + 1 : 0;
		const total = listElements.length;

		for (let i = 0; i < total; i++) {
			const index = (startIndex + i) % total;
			const el = listElements[index];
			if (el.clientHeight <= 0) continue;
			const text = el.innerText.trim().toLowerCase();
			if (text.startsWith(lowerChar)) {
				this.changeFocusedRow(listElements, currIndex, index, builder, data);
				return;
			}
		}
	}

	expandSiblings(
		listElements: Array<HTMLElement>,
		currIndex: number,
		data: TreeWidgetJSON,
		builder: JSBuilder,
	) {
		if (currIndex < 0) return;
		const currentEntry = listElements[currIndex];
		const level = currentEntry.getAttribute('aria-level');
		if (!level) return;

		const parent = currentEntry.parentElement;
		if (!parent) return;

		const siblings = parent.querySelectorAll(
			'.ui-treeview-entry',
		) as NodeListOf<HTMLElement>;
		siblings.forEach((sibling: HTMLElement) => {
			if (
				sibling.getAttribute('aria-level') === level &&
				sibling.classList.contains('collapsed') &&
				sibling.hasAttribute('aria-expanded')
			) {
				const row = (sibling as any)._row;
				builder.callback('treeview', 'expand', data, row, builder);
				sibling.classList.remove('collapsed');
				sibling.setAttribute('aria-expanded', 'true');
			}
		});
	}

	handleKeyEvent(
		event: KeyboardEvent,
		nodeList: NodeList,
		builder: JSBuilder,
		data: TreeWidgetJSON,
	) {
		var preventDef = false;
		var listElements = Array.from(nodeList) as Array<HTMLElement>; // querySelector returns NodeList not array
		var treeLength = listElements.length;
		var currIndex = this.getCurrentEntry(listElements);

		if (event.key === 'ArrowDown') {
			if (currIndex < 0)
				this.changeFocusedRow(listElements, currIndex, 0, builder, data);
			else {
				var nextIndex = currIndex + 1;
				while (
					nextIndex < treeLength - 1 &&
					listElements[nextIndex].clientHeight <= 0
				)
					nextIndex++;
				if (nextIndex < treeLength)
					this.changeFocusedRow(
						listElements,
						currIndex,
						nextIndex,
						builder,
						data,
					);
			}
			preventDef = true;
		} else if (event.key === 'ArrowUp') {
			if (currIndex < 0)
				this.changeFocusedRow(
					listElements,
					currIndex,
					treeLength - 1,
					builder,
					data,
				);
			else {
				var nextIndex = currIndex - 1;
				while (nextIndex >= 0 && listElements[nextIndex].clientHeight <= 0)
					nextIndex--;
				if (nextIndex >= 0)
					this.changeFocusedRow(
						listElements,
						currIndex,
						nextIndex,
						builder,
						data,
					);
			}

			preventDef = true;
		} else if (event.key === 'Home') {
			var firstIndex = 0;
			while (
				firstIndex < treeLength - 1 &&
				listElements[firstIndex].clientHeight <= 0
			)
				firstIndex++;
			if (firstIndex < treeLength)
				this.changeFocusedRow(
					listElements,
					currIndex,
					firstIndex,
					builder,
					data,
				);
			preventDef = true;
		} else if (event.key === 'End') {
			var lastIndex = treeLength - 1;
			while (lastIndex > 0 && listElements[lastIndex].clientHeight <= 0)
				lastIndex--;
			if (lastIndex >= 0)
				this.changeFocusedRow(
					listElements,
					currIndex,
					lastIndex,
					builder,
					data,
				);
			preventDef = true;
		} else if (event.key === '*') {
			this.expandSiblings(listElements, currIndex, data, builder);
			preventDef = true;
		} else if (
			event.key.length === 1 &&
			event.key.match(/[a-zA-Z]/) &&
			!event.ctrlKey &&
			!event.altKey &&
			!event.metaKey
		) {
			this.typeAheadSearch(listElements, currIndex, event.key, builder, data);
			preventDef = true;
		} else if (
			data.fireKeyEvents &&
			// FIXME: can callback return boolean?
			(builder as any).callback(
				'treeview',
				'keydown',
				{ id: data.id, key: event.key },
				currIndex,
				builder,
			)
		) {
			// used in mentions
			preventDef = true;
		}

		if (preventDef) {
			event.preventDefault();
			event.stopPropagation();
		}
	}

	static isRealTree(data: TreeWidgetJSON) {
		if (data.role) return data.role === 'tree' || data.role === 'treegrid';

		let isRealTreeView = false;
		for (var i in data.entries) {
			if (data.entries[i].children && data.entries[i].children.length) {
				isRealTreeView = true;
				break;
			}
		}
		return isRealTreeView;
	}

	getSortComparator(columnIndex: number, up: boolean) {
		return (a: HTMLElement, b: HTMLElement) => {
			if (!a || !b) return 0;

			var tda = a.querySelectorAll('div').item(columnIndex);
			var tdb = b.querySelectorAll('div').item(columnIndex);

			if (tda.querySelector('input')) {
				if (
					tda.querySelector('input').checked ===
					tdb.querySelector('input').checked
				)
					return 0;
				if (up) {
					if (
						tda.querySelector('input').checked >
						tdb.querySelector('input').checked
					)
						return 1;
					else return -1;
				} else if (
					tdb.querySelector('input').checked >
					tda.querySelector('input').checked
				)
					return 1;
				else return -1;
			}

			if (up)
				return tdb.innerText
					.toLowerCase()
					.localeCompare(tda.innerText.toLowerCase());
			else
				return tda.innerText
					.toLowerCase()
					.localeCompare(tdb.innerText.toLowerCase());
		};
	}

	sortByColumn(icon: HTMLSpanElement, columnIndex: number, up: boolean) {
		this.clearSorting();
		window.L.DomUtil.addClass(icon, up ? 'up' : 'down');
		const headerEl = icon.closest('.ui-treeview-header') as HTMLElement;
		if (headerEl) {
			headerEl.setAttribute('aria-sort', up ? 'ascending' : 'descending');
		}

		var toSort: Array<HTMLDivElement> = [];

		const container = this._container;
		container
			.querySelectorAll(
				':not(.ui-treeview-expanded-content) .ui-treeview-entry',
			)
			.forEach((item: HTMLDivElement) => {
				toSort.push(item);
				container.removeChild(item);
			});

		toSort.sort(this.getSortComparator(columnIndex, up));

		toSort.forEach((item) => {
			container.insertBefore(item, container.lastChild.nextSibling);
		});
	}

	clearSorting() {
		var icons = this._thead.querySelectorAll('.ui-treeview-header-sort-icon');
		icons.forEach((icon) => {
			window.L.DomUtil.removeClass(icon, 'down');
			window.L.DomUtil.removeClass(icon, 'up');
		});
		var headers = this._thead.querySelectorAll('.ui-treeview-header');
		headers.forEach((header: HTMLElement) => {
			header.removeAttribute('aria-sort');
		});
	}

	fillHeaders(
		data: TreeWidgetJSON,
		headers: Array<TreeHeaderJSON>,
		builder: JSBuilder,
	) {
		if (!headers) return;

		this._thead = window.L.DomUtil.create(
			'div',
			'ui-treeview-headers',
			this._container,
		);

		this._thead.setAttribute('role', 'row');

		let dummyCells = this._columns - headers.length;
		if (this._hasState) dummyCells++;
		this._thead.style.gridColumn = '1 / ' + (this._columns + dummyCells + 1);

		for (let index = 0; index < dummyCells; index++) {
			this.fillHeader({ text: '', sortable: false }, builder);
			if (index === 0 && this._hasState)
				window.L.DomUtil.addClass(
					this._thead.lastChild,
					'ui-treeview-state-column',
				);
			else
				window.L.DomUtil.addClass(
					this._thead.lastChild,
					'ui-treeview-icon-column',
				);
		}

		for (const index in headers) {
			this.fillHeader(headers[index], builder, data, parseInt(index));

			if (headers[index].sortable === false) continue;

			var clickFunction = (columnIndex: number, icon: HTMLSpanElement) => {
				return () => {
					if (data.sortLocally) {
						if (window.L.DomUtil.hasClass(icon, 'down'))
							this.sortByColumn(icon, columnIndex + dummyCells, true);
						else this.sortByColumn(icon, columnIndex + dummyCells, false);
					} else
						builder.callback('treeview', 'columnclick', data, index, builder);
				};
			};

			const lastHeader = this._thead.lastChild as HTMLElement;
			const button = lastHeader.querySelector(
				'.ui-treeview-header-button',
			) as HTMLElement;
			button.onclick = clickFunction(
				parseInt(index),
				lastHeader.querySelector(
					'.ui-treeview-header-sort-icon',
				) as HTMLSpanElement,
			);
		}
	}

	makeEmptyList(data: TreeWidgetJSON, builder: JSBuilder) {
		// contentbox and tree can never be empty, 1 page or 1 sheet always exists
		if (data.id === 'contenttree') {
			var tr = window.L.DomUtil.create(
				'div',
				builder.options.cssClass + ' ui-treview-entry ui-treeview-placeholder',
				this._container,
			);
			tr.innerText = _(
				'Headings and objects that you add to the document will appear here',
			);
		} else {
			window.L.DomUtil.addClass(this._container, 'empty');
			if (data.hideIfEmpty)
				window.L.DomUtil.addClass(this._container, 'hidden');
		}
	}

	// when no entry is selected - allow first one to be focusable
	makeTreeViewFocusable(enable: boolean) {
		const firstElement = this._container.querySelector(
			`.ui-treeview-entry:not(.${this.PAGE_DIVIDER_ROW_CLASS})`,
		);
		if (firstElement) {
			if (enable) (firstElement as HTMLElement).tabIndex = 0;
			else firstElement.removeAttribute('tabindex');
		}
	}

	fillEntries(
		data: TreeWidgetJSON,
		entries: Array<TreeEntryJSON>,
		builder: JSBuilder,
		level: number,
		parent: HTMLElement,
	) {
		let hasSelectedEntry = false;
		for (const index in entries) {
			hasSelectedEntry = hasSelectedEntry || entries[index].selected;
			this.fillEntry(data, entries[index], builder, level, parent);
		}

		if (entries && entries.length === 0) this.makeEmptyList(data, builder);

		// we need to provide a way for making the treeview control focusable
		// when no entry is selected
		if (level === 1 && !hasSelectedEntry) this.makeTreeViewFocusable(true);
	}

	showSearchBar(parent: HTMLElement) {
		const searchBox = document.createElement('input');
		searchBox.id = JSDialog.MakeIdUnique('ui-treeview-search-input'); // Form fields should have either a name or an ID - using this instead of a class
		searchBox.type = 'search';
		searchBox.setAttribute(
			'class',
			'jsdialog ui-edit ui-treeview-search-input',
		);
		searchBox.setAttribute('aria-label', _('Search items'));
		searchBox.setAttribute('aria-controls', this._container.id);
		searchBox.placeholder = _('Search...');
		searchBox.addEventListener('input', () =>
			this.filterEntries(searchBox.value),
		);

		const searchContainer = document.createElement('div');
		searchContainer.className = 'ui-treeview-search-container';
		searchContainer.style.gridColumn = '1 / ' + this._maxColumnsIncludingState;
		searchContainer.appendChild(searchBox);

		parent.insertAdjacentElement('afterbegin', searchContainer);
	}

	fillEntry(
		data: TreeWidgetJSON,
		entry: TreeEntryJSON,
		builder: JSBuilder,
		level: number,
		parent: HTMLElement,
	): Array<HTMLElement> {
		const entryElements = new Array<HTMLElement>();
		const row: HTMLElement = this.fillRow(data, entry, builder, level, parent);
		entryElements.push(row);

		if (entry.children && entry.children.length) {
			window.L.DomUtil.addClass(row, 'ui-treeview-expandable');
			const subGrid = window.L.DomUtil.create(
				'div',
				'ui-treeview-expanded-content',
				parent,
			);
			if (this._containerRole === 'tree') subGrid.setAttribute('role', 'group');
			else if (this._containerRole === 'treegrid')
				subGrid.setAttribute('role', 'rowgroup');
			entryElements.push(subGrid);

			let dummyColumns = 0;
			if (this._hasState) dummyColumns++;
			subGrid.style.gridColumn = '1 / ' + (this._columns + dummyColumns + 1);
			if (this._columns + dummyColumns + 1 > this._maxColumnsIncludingState) {
				this._maxColumnsIncludingState = this._columns + dummyColumns + 1;
			}

			this.fillEntries(data, entry.children, builder, level + 1, subGrid);
		}

		return entryElements;
	}

	getColumnType(column: TreeColumnJSON) {
		const isString = column.link || column.text;
		const isIcon =
			column.collapsed ||
			column.collapsedimage ||
			column.expanded ||
			column.expandedimage;

		let columnType = 'unknown';
		if (this.isSeparator(column)) columnType = 'separator';
		else if (isString) columnType = 'string';
		else if (isIcon) columnType = 'icon';

		return columnType;
	}

	preprocessColumnData(entires: Array<TreeEntryJSON>) {
		if (!entires || !entires.length) return;

		// generate array of types for each entry
		const columnTypes = entires
			.map(
				(entry: TreeEntryJSON): Array<string> => {
					const currentTypes = new Array<string>();

					entry.columns.forEach((column: TreeColumnJSON) => {
						currentTypes.push(this.getColumnType(column));
					});

					return currentTypes;
				},
				// use the longest entry - naive approach
			)
			.reduce((prev: Array<string>, next: Array<string>): Array<string> => {
				if (!next || prev.length > next.length) return prev;
				return next;
			});

		// put missing dummy columns where are missing
		entires.forEach((entry: TreeEntryJSON) => {
			const existingColumns = entry.columns;
			const missingColumns = columnTypes.length - existingColumns.length;
			if (missingColumns <= 0) return;

			const newColumns = Array<TreeColumnJSON>();
			let targetIndex = 0;
			let existingIndex = 0;
			while (targetIndex < columnTypes.length) {
				const isExistingColumn = existingIndex < existingColumns.length;
				const currentType = isExistingColumn
					? this.getColumnType(existingColumns[existingIndex])
					: 'unknown';

				if (currentType === 'separator') break; // don't add new columns - full width

				if (!isExistingColumn || currentType !== columnTypes[targetIndex]) {
					newColumns.push({ text: '' });
				} else {
					newColumns.push(existingColumns[existingIndex]);
					existingIndex++;
				}

				targetIndex++;
			}
			entry.columns = newColumns;
		});
	}

	static isMenu(data: TreeWidgetJSON): boolean {
		if (data.type === 'menu') return true;
		return false;
	}

	static hasSearchField(data: TreeWidgetJSON): boolean {
		return (
			!data.noSearchField &&
			!TreeViewControl.isMenu(data) &&
			TreeViewControl.isListbox(data) &&
			data.entries &&
			data.entries.length > 25
		);
	}

	static isListbox(data: TreeWidgetJSON): boolean {
		if (data.role) return data.role === 'listbox';

		if (TreeViewControl.isRealTree(data)) return false;

		const columns = TreeViewControl.countColumns(data);
		if (columns !== 1) return false;

		if (data.headers && data.headers.length > 0) return false;

		if (data.entries) {
			for (const entry of data.entries) {
				if (entry.children && entry.children.length > 0) return false;
			}
		}

		return true;
	}

	build(
		data: TreeWidgetJSON,
		builder: JSBuilder,
		parentContainer: HTMLElement,
	) {
		this._isRealTree = TreeViewControl.isRealTree(data);
		this._isListbox = TreeViewControl.isListbox(data);
		this._columns = TreeViewControl.countColumns(data);
		this._hasState = TreeViewControl.hasState(data);
		this._hasIcon = TreeViewControl.hasIcon(data);
		this._isNavigator = this.isNavigator(data);
		this._singleClickActivate = TreeViewControl.isSingleClickActivate(data);
		this._containerRole =
			data.role ||
			(this._isRealTree
				? this._isNavigator
					? 'tree'
					: 'treegrid'
				: this._isListbox
					? 'listbox'
					: 'grid');

		this._tbody = this._container;
		(this._container as any).onSelect = (position: number) => {
			this.selectEntryByRow(position, false);
		};
		(this._container as any).filterEntries = this.filterEntries.bind(this);
		(this._container as any).highlightEntries =
			this.highlightEntries.bind(this);

		// Prevent grab_focus(in executeActionImpl) from focusing the container
		(this._container as any).onFocus = () => {
			// no-op: focus is already on the correct row
		};

		this.setupDragAndDrop(data, builder);
		this.setupKeyEvents(data, builder);
		this.setupFocusOutHandler();

		this._container.setAttribute('role', this._containerRole);
		if (this._isRealTree && (!data.headers || data.headers.length === 0))
			window.L.DomUtil.addClass(this._container, 'ui-treeview-tree');

		this.preprocessColumnData(data.entries);
		this.fillHeaders(data, data.headers, builder);
		this.fillEntries(data, data.entries, builder, 1, this._tbody);

		if (TreeViewControl.hasSearchField(data)) {
			this.showSearchBar(this._container);
		}

		return true;
	}
}

JSDialog.treeView = function (
	parentContainer: HTMLElement,
	data: TreeWidgetJSON,
	builder: JSBuilder,
) {
	var treeView = new TreeViewControl(data, builder);
	treeView.build(data, builder, parentContainer);
	parentContainer.appendChild(treeView._container);

	const updateRenders: CustomEntryRenderCallback = (pos: number | string) => {
		const row = treeView.findEntryWithRow(data.entries, pos);
		if (!row) {
			console.error('treeview updateRenders: row "' + pos + '" not found');
			return;
		}

		const originalRow = treeView._rows.get(String(pos));
		if (!originalRow) {
			console.error('treeview updateRenders: missing original row');
			return;
		}

		const level = parseInt(originalRow.getAttribute('level'));
		const dummyParent = document.createElement('div');
		const newRow: Array<HTMLElement> = treeView.fillEntry(
			data,
			row,
			builder,
			level,
			dummyParent,
		);

		if (originalRow.classList.contains('ui-treeview-expandable')) {
			// we need to remove also sub nodes
			originalRow.nextSibling.replaceWith(newRow[1]);
			originalRow.replaceWith(newRow[0]);
		} else {
			originalRow.replaceWith(newRow[0]);
		}

		treeView._rows.set(String(pos), newRow[0]);
		app.console.debug('treeview: updated render for pos: ' + pos);
	};

	(treeView._container as any).updateRenders = updateRenders;

	return false;
};

JSDialog.TreeViewHasSearchField = TreeViewControl.hasSearchField;

JSDialog.isDnDActive = function () {
	var dndElements = document.querySelectorAll('.droptarget');
	return dndElements && dndElements.length;
};
