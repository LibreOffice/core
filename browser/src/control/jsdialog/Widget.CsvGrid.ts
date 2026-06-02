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
 * JSDialog.CsvGrid - unified ruler + grid widget for the Text Import dialog.
 *
 * Replaces the legacy core-rendered ScCsvRuler + ScCsvGrid pair. The C++ side
 * (ScCsvGrid::DumpWidgetData) ships character-position data; this widget
 * renders both the ruler (canvas) and the data grid (HTML table) together.
 * Splits are only editable in fixed-width mode.
 *
 * Expected JSON payload (in data.data):
 *   {
 *     isFixedMode: bool,
 *     firstImportedLine: int,
 *     posCount: int,
 *     columns: [{ index, type, typeName, selected, pos, width }],
 *     rows:    [{ lineIndex, cells: [{ text }] }],
 *     typeNames: [string],
 *     splits:  [int]
 *   }
 */

declare var JSDialog: any;
declare function _(s: string): string;

const CHAR_WIDTH = 8;
const HDR_WIDTH = 40;
const RULER_HEIGHT = 24;
const ROW_HEIGHT = 18;

const csvRulerCursors = new Map<string, number>();

function getRulerCursor(id: string): number {
	const pos = csvRulerCursors.get(id);
	return pos === undefined ? 1 : pos;
}

function setRulerCursor(id: string, pos: number): void {
	csvRulerCursors.set(id, pos);
}

function clampCursor(pos: number, posCount: number): number {
	const maxPos = Math.max(posCount - 1, 1);
	return Math.max(1, Math.min(pos, maxPos));
}

function updateRulerAria(
	canvas: HTMLCanvasElement,
	cursorPos: number,
	splits: number[],
): void {
	canvas.setAttribute('aria-valuenow', String(cursorPos));
	const text =
		splits.indexOf(cursorPos) >= 0
			? _('Position {n}, split set').replace('{n}', String(cursorPos))
			: _('Position {n}, no split').replace('{n}', String(cursorPos));
	canvas.setAttribute('aria-valuetext', text);
}

interface CsvColumn {
	index: number;
	type: number;
	typeName: string;
	selected: boolean;
	pos: number;
	width: number;
}

interface CsvCell {
	text: string;
}

interface CsvRow {
	lineIndex: number;
	cells: CsvCell[];
}

interface CsvGridData {
	isFixedMode: boolean;
	firstImportedLine: number;
	posCount: number;
	columns: CsvColumn[];
	rows: CsvRow[];
	typeNames: string[];
	splits: number[];
}

interface CsvGridContainer extends HTMLElement {
	builder: JSBuilder;
	gridData: CsvGridData;
	_csvOpenMenu: HTMLElement | null;
	_csvOpenMenuCloseFn: (() => void) | null;
}

interface Palette {
	bg: string;
	surface: string;
	surfaceSelected: string;
	text: string;
	textDim: string;
	border: string;
	gridLine: string;
	accent: string;
	alert: string;
}

function readPalette(): Palette {
	const cs = window.getComputedStyle(document.documentElement);
	const pick = (varName: string, fallback: string): string => {
		const v = cs.getPropertyValue(varName);
		return (v && v.trim()) || fallback;
	};
	return {
		bg: pick('--color-main-background', '#ffffff'),
		surface: pick('--color-calc-header', '#f0f0f0'),
		surfaceSelected: pick('--color-calc-header-selected', '#cce4ff'),
		text: pick('--color-main-text', '#333333'),
		textDim: pick('--color-text-lighter', '#696969'),
		border: pick('--color-border', '#b6b6b6'),
		gridLine: pick('--color-calc-grid', '#c0c0c0'),
		accent: pick('--color-primary', '#0b87e7'),
		alert: pick('--color-error', '#e9322d'),
	};
}

function _csvGridControl(
	parentContainer: Element,
	data: any,
	builder: JSBuilder,
): boolean {
	const widgetData: CsvGridData = data.data || data;

	const container = window.L.DomUtil.create(
		'div',
		'ui-csv-grid-container ' + (builder.options.cssClass || ''),
		parentContainer,
	) as CsvGridContainer;
	container.id = data.id;
	container.builder = builder;
	container.gridData = widgetData;
	container._csvOpenMenu = null;
	container._csvOpenMenuCloseFn = null;

	container.style.setProperty('--csv-ruler-height', RULER_HEIGHT + 'px');
	container.style.setProperty('--csv-row-height', ROW_HEIGHT + 'px');
	if (widgetData.isFixedMode) container.classList.add('fixed-mode');

	container.addEventListener('keydown', (e: KeyboardEvent) => {
		handleCsvKeydown(container, e);
	});

	if (container.parentNode) {
		const observer = new MutationObserver(() => {
			if (!document.body.contains(container)) {
				closeOpenMenu(container);
				observer.disconnect();
			}
		});
		observer.observe(container.parentNode, { childList: true });
	}

	renderInto(container);

	return false;
}

function renderInto(container: CsvGridContainer): void {
	while (container.firstChild) {
		container.removeChild(container.firstChild);
	}

	const data = container.gridData;
	const charWidth = CHAR_WIDTH;
	const hdrWidth = HDR_WIDTH;
	const posCount = data.posCount || 1;
	const totalWidth = hdrWidth + Math.max(posCount, 1) * charWidth;

	if (data.isFixedMode) {
		const ruler = createRuler(container, charWidth, hdrWidth, totalWidth);
		container.appendChild(ruler);
	}

	const table = createGridTable(container, charWidth, hdrWidth, totalWidth);
	container.appendChild(table);
}

function createRuler(
	container: CsvGridContainer,
	charWidth: number,
	hdrWidth: number,
	totalWidth: number,
): HTMLElement {
	const data = container.gridData;
	const posCount = data.posCount || 1;
	const splits = data.splits || [];
	const isFixedMode = !!data.isFixedMode;

	const cursorPos = clampCursor(getRulerCursor(container.id), posCount);
	setRulerCursor(container.id, cursorPos);

	const wrapper = document.createElement('div');
	wrapper.className = 'ui-csv-ruler-wrapper';
	if (isFixedMode) wrapper.classList.add('fixed-mode');
	wrapper.style.width = totalWidth + 'px';

	const canvas = document.createElement('canvas');
	canvas.className = 'ui-csv-ruler-canvas';
	canvas.id = container.id + '-ruler';
	canvas.width = totalWidth;
	canvas.height = RULER_HEIGHT;
	canvas.tabIndex = 0;
	canvas.setAttribute('role', 'slider');
	canvas.setAttribute('aria-orientation', 'horizontal');
	canvas.setAttribute('aria-valuemin', '1');
	canvas.setAttribute('aria-valuemax', String(Math.max(posCount - 1, 1)));
	canvas.setAttribute(
		'aria-label',
		_(
			'Column split ruler. Use the arrow keys to move the cursor and Space to toggle a split.',
		),
	);
	updateRulerAria(canvas, cursorPos, splits);
	wrapper.appendChild(canvas);

	drawRuler(
		canvas,
		charWidth,
		hdrWidth,
		posCount,
		splits,
		isFixedMode,
		cursorPos,
		readPalette(),
	);

	canvas.addEventListener('click', (e: MouseEvent) => {
		if (!isFixedMode) return;
		const rect = canvas.getBoundingClientRect();
		const xPos = e.clientX - rect.left - hdrWidth;
		if (xPos < 0) return;
		const pos = Math.round(xPos / charWidth);
		if (pos <= 0 || pos >= posCount) return;
		setRulerCursor(container.id, pos);
		sendAction(container, 'togglesplit', pos.toString());
	});

	canvas.addEventListener('contextmenu', (e: MouseEvent) => {
		e.preventDefault();
	});

	return wrapper;
}

function drawRuler(
	canvas: HTMLCanvasElement,
	charWidth: number,
	hdrWidth: number,
	posCount: number,
	splits: number[],
	isFixedMode: boolean,
	cursorPos: number,
	palette: Palette,
): void {
	const ctx = canvas.getContext('2d');
	if (!ctx) return;
	const height = canvas.height;

	ctx.fillStyle = palette.surface;
	ctx.fillRect(0, 0, canvas.width, height);

	ctx.fillStyle = isFixedMode ? palette.bg : palette.surface;
	ctx.fillRect(hdrWidth, 0, posCount * charWidth, height - 1);

	ctx.strokeStyle = palette.border;
	ctx.beginPath();
	ctx.moveTo(0, height - 0.5);
	ctx.lineTo(canvas.width, height - 0.5);
	ctx.stroke();

	ctx.fillStyle = palette.text;
	ctx.strokeStyle = palette.text;
	ctx.font = '10px monospace';
	ctx.textAlign = 'center';
	ctx.textBaseline = 'top';

	const midY = Math.floor(height / 2);
	for (let p = 0; p <= posCount; p += 1) {
		const x = hdrWidth + p * charWidth + 0.5;
		if (x < hdrWidth || x > canvas.width) continue;
		if (p % 10 === 0) {
			ctx.beginPath();
			ctx.moveTo(x, midY - 2);
			ctx.lineTo(x, midY + 4);
			ctx.stroke();
			ctx.fillText(p.toString(), x, 1);
		} else if (p % 5 === 0) {
			ctx.beginPath();
			ctx.moveTo(x, midY);
			ctx.lineTo(x, midY + 3);
			ctx.stroke();
		} else {
			ctx.fillRect(x, midY + 1, 1, 1);
		}
	}

	splits.forEach((splitPos: number) => {
		if (splitPos <= 0 || splitPos >= posCount) return;
		const sx = hdrWidth + splitPos * charWidth;
		drawSplitMarker(ctx, sx, height, palette);
	});

	if (isFixedMode && cursorPos > 0 && cursorPos < posCount) {
		drawRulerCursor(ctx, hdrWidth + cursorPos * charWidth, height, palette);
	}
}

function drawSplitMarker(
	ctx: CanvasRenderingContext2D,
	x: number,
	height: number,
	palette: Palette,
): void {
	const size = 5;
	ctx.strokeStyle = palette.alert;
	ctx.lineWidth = 1;
	ctx.beginPath();
	ctx.moveTo(x, 0);
	ctx.lineTo(x, height - size - 1);
	ctx.stroke();

	ctx.fillStyle = palette.alert;
	ctx.beginPath();
	ctx.moveTo(x, height - size * 2);
	ctx.lineTo(x - size, height - size);
	ctx.lineTo(x, height - 1);
	ctx.lineTo(x + size, height - size);
	ctx.closePath();
	ctx.fill();
}

/* The keyboard cursor: a vertical line with a downward caret at the top, drawn
   in the accent colour so it stays distinct from the (red) split markers. */
function drawRulerCursor(
	ctx: CanvasRenderingContext2D,
	x: number,
	height: number,
	palette: Palette,
): void {
	const size = 4;
	ctx.save();
	ctx.strokeStyle = palette.accent;
	ctx.fillStyle = palette.accent;
	ctx.lineWidth = 1;
	ctx.beginPath();
	ctx.moveTo(x + 0.5, 0);
	ctx.lineTo(x + 0.5, height - 1);
	ctx.stroke();
	ctx.beginPath();
	ctx.moveTo(x - size + 0.5, 0);
	ctx.lineTo(x + size + 0.5, 0);
	ctx.lineTo(x + 0.5, size + 1);
	ctx.closePath();
	ctx.fill();
	ctx.restore();
}

function redrawRuler(container: CsvGridContainer): void {
	const canvas = container.querySelector(
		'.ui-csv-ruler-canvas',
	) as HTMLCanvasElement | null;
	if (!canvas) return;
	const data = container.gridData;
	const posCount = data.posCount || 1;
	const splits = data.splits || [];
	const cursorPos = getRulerCursor(container.id);
	drawRuler(
		canvas,
		CHAR_WIDTH,
		HDR_WIDTH,
		posCount,
		splits,
		!!data.isFixedMode,
		cursorPos,
		readPalette(),
	);
	updateRulerAria(canvas, cursorPos, splits);
}

function createGridTable(
	container: CsvGridContainer,
	charWidth: number,
	hdrWidth: number,
	totalWidth: number,
): HTMLElement {
	const data = container.gridData;
	const columns = data.columns || [];
	const rows = data.rows || [];
	const firstImportedLine =
		data.firstImportedLine !== undefined ? data.firstImportedLine : 0;

	const table = document.createElement('table');
	table.className = 'ui-csv-grid';
	table.style.width = totalWidth + 'px';
	table.setAttribute('role', 'grid');
	table.setAttribute('aria-label', _('CSV import preview'));
	table.setAttribute('aria-rowcount', String(rows.length));
	table.setAttribute('aria-colcount', String(columns.length));

	const colgroup = document.createElement('colgroup');
	const rowHeaderCol = document.createElement('col');
	rowHeaderCol.style.width = hdrWidth + 'px';
	colgroup.appendChild(rowHeaderCol);
	columns.forEach((col: CsvColumn) => {
		const c = document.createElement('col');
		c.style.width = col.width * charWidth + 'px';
		colgroup.appendChild(c);
	});
	table.appendChild(colgroup);

	const thead = document.createElement('thead');
	const headerRow = document.createElement('tr');
	headerRow.setAttribute('role', 'row');

	const rowNumberHeader = document.createElement('th');
	rowNumberHeader.className = 'ui-csv-grid-row-header';
	rowNumberHeader.setAttribute('role', 'columnheader');
	rowNumberHeader.setAttribute('aria-label', _('Line number'));
	rowNumberHeader.textContent = '#';
	headerRow.appendChild(rowNumberHeader);

	columns.forEach((col: CsvColumn) => {
		const th = document.createElement('th');
		th.className = 'ui-csv-grid-col-header';
		th.id = container.id + '-colhdr-' + col.index;
		th.tabIndex = 0;
		th.setAttribute('role', 'columnheader');
		th.setAttribute('aria-selected', col.selected ? 'true' : 'false');
		th.setAttribute('aria-keyshortcuts', 'Space');
		if (col.selected) {
			th.classList.add('selected');
		}
		const fallbackLabel = _('Column {index}').replace(
			'{index}',
			String(col.index + 1),
		);
		th.textContent = col.typeName || fallbackLabel;
		th.dataset.colIndex = String(col.index);
		th.title = th.textContent;

		th.addEventListener('mousedown', (e: MouseEvent) => {
			if (e.button !== 0) return;
			e.stopPropagation();
			handleSelect(container, col.index, e);
		});
		th.addEventListener('contextmenu', (e: MouseEvent) => {
			e.preventDefault();
			e.stopPropagation();
			showColumnTypeMenu(container, col.index, e.clientX, e.clientY);
		});
		headerRow.appendChild(th);
	});

	thead.appendChild(headerRow);
	table.appendChild(thead);

	const tbody = document.createElement('tbody');
	rows.forEach((row: CsvRow, rowIdx: number) => {
		const tr = document.createElement('tr');
		tr.setAttribute('role', 'row');
		tr.setAttribute('aria-rowindex', String(rowIdx + 1));
		if (firstImportedLine > 0 && row.lineIndex === firstImportedLine) {
			tr.classList.add('import-start');
		}

		const rowNum = document.createElement('td');
		rowNum.className = 'ui-csv-grid-row-number';
		rowNum.setAttribute('role', 'rowheader');
		rowNum.textContent = (row.lineIndex + 1).toString();
		tr.appendChild(rowNum);

		(row.cells || []).forEach((cell: CsvCell, cellIndex: number) => {
			const col = columns[cellIndex];
			const td = document.createElement('td');
			td.className = 'ui-csv-grid-cell';
			td.setAttribute('role', 'gridcell');
			if (col && col.selected) {
				td.classList.add('selected');
				td.setAttribute('aria-selected', 'true');
			}
			if (row.lineIndex < firstImportedLine) {
				td.classList.add('dimmed');
			}
			td.setAttribute('aria-colindex', String(cellIndex + 1));
			td.dataset.rowIndex = String(row.lineIndex);
			td.dataset.colIndex = String(cellIndex);
			td.textContent = cell.text || '';

			td.addEventListener('mousedown', (e: MouseEvent) => {
				if (e.button !== 0) return;
				e.stopPropagation();
				handleSelect(container, cellIndex, e);
			});
			td.addEventListener('contextmenu', (e: MouseEvent) => {
				e.preventDefault();
				e.stopPropagation();
				showColumnTypeMenu(container, cellIndex, e.clientX, e.clientY);
			});
			tr.appendChild(td);
		});

		tbody.appendChild(tr);
	});
	table.appendChild(tbody);

	return table;
}

function handleSelect(
	container: CsvGridContainer,
	colIndex: number,
	mouseEvent: MouseEvent,
): void {
	sendAction(
		container,
		'selectcolumn',
		JSON.stringify({
			column: colIndex,
			ctrl: !!(mouseEvent.ctrlKey || mouseEvent.metaKey),
			shift: !!mouseEvent.shiftKey,
		}),
	);
}

// keyboard navigation

function handleCsvKeydown(container: CsvGridContainer, e: KeyboardEvent): void {
	const target = e.target as HTMLElement | null;
	if (!target) return;
	if (target.closest('.ui-csv-ruler-canvas')) {
		handleRulerKey(container, e);
		return;
	}
	const header = target.closest(
		'.ui-csv-grid-col-header',
	) as HTMLElement | null;
	if (header) handleHeaderKey(container, header, e);
}

function focusHeader(container: CsvGridContainer, colIndex: number): void {
	const headers = Array.from(
		container.querySelectorAll('.ui-csv-grid-col-header'),
	) as HTMLElement[];
	if (headers.length === 0) return;
	const clamped = Math.max(0, Math.min(colIndex, headers.length - 1));
	const next = headers[clamped];
	next.focus();
	next.scrollIntoView({ block: 'nearest', inline: 'nearest' });
}

function handleHeaderKey(
	container: CsvGridContainer,
	header: HTMLElement,
	e: KeyboardEvent,
): void {
	const col = parseInt(header.dataset.colIndex || '-1', 10);
	if (col < 0) return;
	let handled = true;

	switch (e.key) {
		case ' ':
			sendAction(
				container,
				'selectcolumn',
				JSON.stringify({
					column: col,
					ctrl: !!(e.ctrlKey || e.metaKey),
					shift: !!e.shiftKey,
				}),
			);
			break;
		case 'ArrowRight':
			focusHeader(container, col + 1);
			break;
		case 'ArrowLeft':
			focusHeader(container, col - 1);
			break;
		case 'Home':
			focusHeader(container, 0);
			break;
		case 'End':
			focusHeader(container, Number.MAX_SAFE_INTEGER);
			break;
		default:
			handled = false;
			break;
	}

	if (handled) {
		e.preventDefault();
		e.stopPropagation();
	}
}

function handleRulerKey(container: CsvGridContainer, e: KeyboardEvent): void {
	const posCount = container.gridData.posCount || 1;
	const id = container.id;
	let handled = true;

	switch (e.key) {
		case 'ArrowRight':
			setRulerCursor(id, clampCursor(getRulerCursor(id) + 1, posCount));
			redrawRuler(container);
			break;
		case 'ArrowLeft':
			setRulerCursor(id, clampCursor(getRulerCursor(id) - 1, posCount));
			redrawRuler(container);
			break;
		case 'Home':
			setRulerCursor(id, 1);
			redrawRuler(container);
			break;
		case 'End':
			setRulerCursor(id, clampCursor(posCount - 1, posCount));
			redrawRuler(container);
			break;
		case ' ': {
			const cursorPos = getRulerCursor(id);
			if (cursorPos >= 1 && cursorPos < posCount) {
				sendAction(container, 'togglesplit', String(cursorPos));
			}
			break;
		}
		default:
			handled = false;
			break;
	}

	if (handled) {
		e.preventDefault();
		e.stopPropagation();
	}
}

function showColumnTypeMenu(
	container: CsvGridContainer,
	colIndex: number,
	x: number,
	y: number,
): void {
	const typeNames = (container.gridData && container.gridData.typeNames) || [];
	if (typeNames.length === 0) return;

	closeOpenMenu(container);

	const menu = document.createElement('div');
	menu.className = 'ui-csv-context-menu';
	menu.setAttribute('role', 'menu');
	menu.style.left = x + 'px';
	menu.style.top = y + 'px';

	typeNames.forEach((typeName: string, typeIndex: number) => {
		const item = document.createElement('div');
		item.className = 'ui-csv-menu-item';
		item.setAttribute('role', 'menuitem');
		item.textContent = typeName;

		item.addEventListener('click', (e: MouseEvent) => {
			e.stopPropagation();
			closeOpenMenu(container);
			sendAction(
				container,
				'setcolumntype',
				JSON.stringify({
					column: colIndex,
					type: typeIndex,
				}),
			);
		});

		menu.appendChild(item);
	});

	document.body.appendChild(menu);
	container._csvOpenMenu = menu;

	const close = (): void => {
		closeOpenMenu(container);
	};
	container._csvOpenMenuCloseFn = (): void => {
		container.removeEventListener('mousedown', close, true);
	};
	app.layoutingService.appendLayoutingTask(() => {
		if (!container._csvOpenMenu) return;
		container.addEventListener('mousedown', close, true);
	});
}

function closeOpenMenu(container: CsvGridContainer): void {
	const menu = container._csvOpenMenu;
	if (menu && menu.parentNode) {
		menu.parentNode.removeChild(menu);
	}
	container._csvOpenMenu = null;
	const fn = container._csvOpenMenuCloseFn;
	container._csvOpenMenuCloseFn = null;
	if (fn) fn();
}

function sendAction(
	container: CsvGridContainer,
	action: string,
	data: string,
): void {
	if (!container.builder || !container.builder.callback) return;
	container.builder.callback(
		'customwidget',
		action,
		container,
		data || '',
		container.builder,
	);
}

JSDialog.csvGrid = function (
	parentContainer: Element,
	data: any,
	builder: JSBuilder,
): boolean {
	return _csvGridControl(parentContainer, data, builder);
};

if (JSDialog.registerCustomWidgetHandler) {
	JSDialog.registerCustomWidgetHandler('csvgrid', _csvGridControl);
}
