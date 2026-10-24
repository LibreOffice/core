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
 * CalcVectorContentSection - draws spreadsheet cells from vector primitives in place of bitmap
 * tiles. Blocks sit at their first cell in the client sheet geometry, so they line up with its
 * grid. Each layer is drawn for all blocks before the next, so text overflowing into a neighbor
 * block stays over the fills of that block.
 */

/* global app */

namespace cool {
	/// Where a pane draws. The bounds and the offset are in core pixels.
	interface PaneFrame {
		bounds: cool.Bounds;
		offset: cool.Point;
		geometry: cool.SheetGeometry;
	}

	/// The text of a strip of rows drawn into a canvas. resources counts the loaded fonts and
	/// images. left and top are the pixel offset of the canvas from the block corner. canvas is
	/// null for a strip too large for one.
	interface TextRaster {
		source: cool.CalcBlockEntry;
		resources: number;
		background: string;
		canvas: HTMLCanvasElement | null;
		left: number;
		top: number;
		bytes: number;
		/// The twips in a pixel at the zoom the raster was drawn at.
		twipsPerPixel: number;
	}

	/// A range of cells, its first and last column and row.
	interface CellRange {
		startColumn: number;
		endColumn: number;
		startRow: number;
		endRow: number;
	}

	/// A block placed at the view zoom, or a held block that is placed when its job runs.
	type RasterBlock =
		| { placed: true; block: cool.CalcBlockEntry }
		| { placed: false; block: cool.CalcBlockEntry<cool.CalcCellRect> };

	/// A block to draw into rasters while idle. origin and area are in pixels.
	type RasterJob = RasterBlock & {
		sheet: cool.CalcSheetData;
		origin: cool.Point;
		area: { left: number; top: number; right: number; bottom: number };
	};

	export class CalcVectorContentSection extends CanvasSectionObject {
		processingOrder: number = app.CSections.CalcVectorContent.processingOrder;
		drawingOrder: number = app.CSections.CalcVectorContent.drawingOrder;
		zIndex: number = app.CSections.CalcVectorContent.zIndex;
		boundToSection: string = 'tiles';
		interactable: boolean = false;

		// The shifts in pixels match the tiles. Borders sit one pixel before the cell and fills
		// start one pixel above it.
		private static readonly cellLayers: Array<{
			name: keyof cool.CalcBlockLayers;
			shiftX: number;
			shiftY: number;
		}> = [
			{ name: 'background', shiftX: 0, shiftY: -1 },
			{ name: 'borders', shiftX: -1, shiftY: -1 },
			{ name: 'text', shiftX: 0, shiftY: 0 },
			{ name: 'extras', shiftX: 0, shiftY: 0 },
			{ name: 'marks', shiftX: 0, shiftY: 0 },
		];

		/// The costliest layers to draw, kept in text rasters. They have no shift.
		private static readonly rasterLayers: Array<keyof cool.CalcBlockLayers> = [
			'text',
			'extras',
			'marks',
		];

		/// Gray and translucent, so it shows on light and dark backgrounds.
		private static readonly placeholderColor = 'rgba(128, 128, 128, 0.25)';
		/// The most cells waiting for their blocks that get a placeholder each. Beyond that the
		/// view is zoomed out so far that the bars are too small and too many to draw each frame.
		private static readonly maxPlaceholders = 4096;

		/// The number of rows of a block that one text raster holds.
		private static readonly stripRows = 16;

		/// The byte limit for all text rasters. The least recently drawn go first.
		private static readonly rasterBudget = 160 * 1024 * 1024;

		/// The longest side of a text raster in pixels.
		private static readonly maxRasterSide = 8192;

		// Text rasters by sheet, block and strip, least recently drawn first.
		private _rasters: Map<string, TextRaster> = new Map();
		private _rasterBytes = 0;
		// The sheet the rasters are of.
		private _rasterSheet = '';

		/// The time in milliseconds that one frame may spend drawing rasters.
		private static readonly frameRasterBudget = 8;

		// The time this frame spent on rasters, and whether a strip was left over.
		private _frameRasterTime = 0;
		private _rasterLeftOver = false;
		// Rasters shown this frame. They stay even when they pass the budget.
		private _frameRasters: Set<string> = new Set();

		// Idle raster jobs, the next one to run, and whether a run is scheduled.
		private _rasterJobs: RasterJob[] = [];
		private _nextRasterJob = 0;
		private _idleScheduled = false;

		private _onChanged = (): void => {
			if (app.sectionContainer) app.sectionContainer.requestReDraw();
		};

		private _onReclaimGraphicsMemory = (): void => {
			this._rasters.clear();
			this._rasterBytes = 0;
			// The next draw of the sheet queues the raster jobs again.
			this._rasterJobs = [];
			this._nextRasterJob = 0;
		};

		constructor() {
			super(app.CSections.CalcVectorContent.name);
			CalcVectorManager.instance().onChanged(this._onChanged);
			CalcVectorManager.instance().onReclaimGraphicsMemory(
				this._onReclaimGraphicsMemory,
			);
		}

		onRemove(): void {
			CalcVectorManager.instance().offChanged(this._onChanged);
			CalcVectorManager.instance().offReclaimGraphicsMemory(
				this._onReclaimGraphicsMemory,
			);
			this._onReclaimGraphicsMemory();
		}

		/// Add or remove the section to match whether vector rendering is active.
		static update(): void {
			if (!app.sectionContainer) return;
			const wanted = CalcVectorManager.isActive();
			const present = app.sectionContainer.doesSectionExist(
				app.CSections.CalcVectorContent.name,
			);
			if (wanted === present) return;
			if (wanted) {
				app.sectionContainer.addSection(new cool.CalcVectorContentSection());
				// The tiles held show the sheet as it was, and none is drawn from now on.
				RenderManager.discardTiles();
			} else
				app.sectionContainer.removeSection(
					app.CSections.CalcVectorContent.name,
				);
			// The tiles are asked for again when the view draws from them.
			if (!wanted) RenderManager.update();
		}

		onDraw(): void {
			if (this.containerObject.isInZoomAnimation()) return;
			this._frameRasterTime = 0;
			this._rasterLeftOver = false;
			this._frameRasters.clear();
			this._rasterJobs = [];
			this._nextRasterJob = 0;
			this._drawSheet();
			if (this._rasterLeftOver)
				setTimeout(() => {
					if (app.sectionContainer) app.sectionContainer.requestReDraw();
				}, 0);
			this._scheduleIdleRasters();
		}

		private _drawSheet(): void {
			const docLayer = app.map._docLayer;
			if (!docLayer) return;
			const part: number = docLayer._selectedPart;
			if (!(part >= 0)) return;
			// During a sheet switch the geometry can still be of the previous sheet.
			const geometry: cool.SheetGeometry | undefined = docLayer.sheetGeometry;
			if (!geometry || geometry.getPart() !== part) return;

			const painter = docLayer._painter;
			const paintContext = painter._paintContext();
			// Content is zoom independent. Each block is placed at the view zoom.
			const mode = CalcVectorManager.contentMode;
			if (!app.tile.size) return;
			const twipsPerPixel = app.tile.size.x / RenderManager.tileSize;

			const manager = CalcVectorManager.instance();
			const renderer = manager.renderer;
			const background = this.containerObject.getClearColor();
			renderer.setDocumentBackground(background);
			renderer.setEditViewContentVisible(true);

			const sheet = manager.getSheet(part, mode);
			const blockRows = sheet ? sheet.blockRows : 64;
			const blockColumns = sheet ? sheet.blockColumns : 16;
			const columns = geometry.getColumnsGeometry();
			const rows = geometry.getRowsGeometry();
			// The last column and row that hold content on the sheet.
			const calcLayer = docLayer as unknown as {
				_lastColumn?: number;
				_lastRow?: number;
			};
			const usedArea = {
				lastColumn: calcLayer._lastColumn ?? 0,
				lastRow: calcLayer._lastRow ?? 0,
			};

			// Blocks to hold, with their distance in blocks from the view middle.
			const wanted: Array<{
				block: [number, number];
				inView: boolean;
				distance: number;
			}> = [];
			const keep = new Set<string>();

			for (const paneBounds of paintContext.paneBoundsList) {
				const viewBounds = paintContext.viewBounds;
				paneBounds.round();
				viewBounds.round();
				// The pane offset in canvas pixels, computed as in the tiles section.
				const offset = paneBounds.getTopLeft();
				offset.x = Math.min(offset.x, viewBounds.min.x);
				offset.y = Math.min(offset.y, viewBounds.min.y);
				const pane: PaneFrame = {
					bounds: paneBounds,
					offset: offset,
					geometry: geometry,
				};

				const firstColumn = columns.getIndexFromPos(
					paneBounds.min.x,
					'corepixels',
				);
				const lastColumn = columns.getIndexFromPos(
					paneBounds.max.x,
					'corepixels',
				);
				const firstRow = rows.getIndexFromPos(paneBounds.min.y, 'corepixels');
				const lastRow = rows.getIndexFromPos(paneBounds.max.y, 'corepixels');
				if (
					firstColumn === undefined ||
					lastColumn === undefined ||
					firstRow === undefined ||
					lastRow === undefined
				)
					continue;

				// One more block left, right and above is held, because text overflows sideways and
				// merged areas reach down.
				const firstBlockColumn = Math.max(
					0,
					Math.floor(firstColumn / blockColumns) - 1,
				);
				const lastBlockColumn = Math.floor(lastColumn / blockColumns) + 1;
				const firstBlockRow = Math.max(0, Math.floor(firstRow / blockRows) - 1);
				const lastBlockRow = Math.floor(lastRow / blockRows);
				const middleRow = (firstRow + lastRow + 1) / 2 / blockRows;
				const middleColumn = (firstColumn + lastColumn + 1) / 2 / blockColumns;

				// The block row below the view is held too, ready for scrolling.
				const blocks: cool.CalcBlockEntry[] = [];
				// The held blocks below the view, drawn into rasters while idle.
				const ahead: cool.CalcBlockEntry<cool.CalcCellRect>[] = [];
				// Visible cells of missing blocks, limited to the used area.
				const waiting: CellRange[] = [];
				for (
					let blockRow = firstBlockRow;
					blockRow <= lastBlockRow + 1;
					++blockRow
				) {
					for (
						let blockColumn = firstBlockColumn;
						blockColumn <= lastBlockColumn;
						++blockColumn
					) {
						if (blockColumn * blockColumns > columns.getMaxIndex()) continue;
						if (blockRow * blockRows > rows.getMaxIndex()) continue;
						const key = cool.calcBlockKey(blockRow, blockColumn);
						keep.add(key);
						wanted.push({
							block: [blockRow, blockColumn],
							inView:
								(blockRow + 1) * blockRows > firstRow &&
								blockRow * blockRows <= lastRow &&
								(blockColumn + 1) * blockColumns > firstColumn &&
								blockColumn * blockColumns <= lastColumn,
							distance: Math.hypot(
								blockRow + 0.5 - middleRow,
								blockColumn + 0.5 - middleColumn,
							),
						});
						const block = sheet && sheet.blocks.get(key);
						const placed =
							sheet && block && blockRow <= lastBlockRow
								? manager.placedBlock(sheet, block)
								: undefined;
						if (placed) blocks.push(placed);
						else if (sheet && block && blockRow > lastBlockRow)
							ahead.push(block);
						else if (blockRow <= lastBlockRow) {
							const range: CellRange = {
								startColumn: Math.max(blockColumn * blockColumns, firstColumn),
								endColumn: Math.min(
									(blockColumn + 1) * blockColumns - 1,
									lastColumn,
									usedArea.lastColumn,
								),
								startRow: Math.max(blockRow * blockRows, firstRow),
								endRow: Math.min(
									(blockRow + 1) * blockRows - 1,
									lastRow,
									usedArea.lastRow,
								),
							};
							if (
								range.startColumn <= range.endColumn &&
								range.startRow <= range.endRow
							)
								waiting.push(range);
						}
					}
				}

				// A block whose objects reach into the view is held too, so an object anchored out
				// of view is drawn where it paints.
				if (sheet)
					for (const objectBlock of sheet.objectBlocks) {
						if (
							objectBlock.endColumn < firstColumn ||
							objectBlock.startColumn > lastColumn ||
							objectBlock.endRow < firstRow ||
							objectBlock.startRow > lastRow
						)
							continue;
						const key = cool.calcBlockKey(
							objectBlock.blockRow,
							objectBlock.blockColumn,
						);
						if (keep.has(key)) continue;
						keep.add(key);
						wanted.push({
							block: [objectBlock.blockRow, objectBlock.blockColumn],
							inView: true,
							distance: Math.hypot(
								objectBlock.blockRow + 0.5 - middleRow,
								objectBlock.blockColumn + 0.5 - middleColumn,
							),
						});
					}

				this._drawPane(pane, sheet, blocks, waiting, twipsPerPixel, background);
				if (sheet) this._queueRasterJobs(pane, sheet, blocks, ahead);
			}

			// Visible blocks nearest the middle come first for the engine to build.
			wanted.sort(
				(a, b) =>
					Number(b.inView) - Number(a.inView) || a.distance - b.distance,
			);
			manager.requestBlocks(
				part,
				mode,
				wanted.map((entry) => entry.block),
			);
			manager.evict(part, mode, keep);
		}

		private _drawPane(
			pane: PaneFrame,
			sheet: cool.CalcSheetData | undefined,
			blocks: cool.CalcBlockEntry[],
			waiting: CellRange[],
			twipsPerPixel: number,
			background: string,
		): void {
			const context = this.context;
			const bounds = pane.bounds;
			context.save();
			context.beginPath();
			context.rect(
				bounds.min.x - pane.offset.x,
				bounds.min.y - pane.offset.y,
				bounds.max.x - bounds.min.x,
				bounds.max.y - bounds.min.y,
			);
			context.clip();

			// Blocks are taller than the view, so only parts in the pane are drawn.
			const view = {
				left: bounds.min.x - pane.offset.x,
				top: bounds.min.y - pane.offset.y,
				right: bounds.max.x - pane.offset.x,
				bottom: bounds.max.y - pane.offset.y,
			};
			const drawn: Array<{
				block: cool.CalcBlockEntry;
				origin: cool.Point;
				parts: cool.CalcBlockPart[];
			}> = [];
			for (const block of blocks) {
				const origin = this._cellOrigin(
					pane,
					block.startColumn,
					block.startRow,
				);
				if (!origin) continue;
				const parts: cool.CalcBlockPart[] = [];
				for (const part of [block.borders, ...block.rows.values()]) {
					if (!part || !part.content.layers) continue;
					// The ink leaves out grid covers, and a merged area's cover can reach down from
					// a row above the view.
					const ranges = [part.content.ink].concat(
						CalcVectorContentSection._coverRanges(part),
					);
					if (
						!part.content.ink ||
						ranges.some((range) =>
							CalcVectorContentSection._paintsInto(
								range,
								origin,
								twipsPerPixel,
								view,
							),
						)
					)
						parts.push(part);
				}
				drawn.push({ block, origin, parts });
			}

			for (const { origin, parts } of drawn)
				for (const part of parts)
					this._coverGrid(origin, part, twipsPerPixel, background);

			// Draw the layers under the text, each one across all blocks.
			for (const layer of CalcVectorContentSection.cellLayers) {
				if (CalcVectorContentSection.rasterLayers.indexOf(layer.name) >= 0)
					continue;
				for (const { origin, parts } of drawn) {
					for (const part of parts) {
						const primitives = part.content.layers?.[layer.name];
						if (!primitives || primitives.length === 0) continue;
						this._drawAt(
							origin,
							layer.shiftX,
							layer.shiftY,
							twipsPerPixel,
							primitives,
						);
					}
				}
			}

			const sheetKey = sheet ? String(sheet.part) : '';
			// The rasters hold the text of the sheet shown, so a switch of sheet drops them.
			if (sheetKey !== this._rasterSheet) {
				this._rasters.clear();
				this._rasterBytes = 0;
				this._rasterSheet = sheetKey;
			}
			let waitingCells = 0;
			for (const range of waiting)
				waitingCells +=
					(range.endRow - range.startRow + 1) *
					(range.endColumn - range.startColumn + 1);
			if (waitingCells <= CalcVectorContentSection.maxPlaceholders)
				for (const range of waiting) this._drawPlaceholders(pane, range);

			for (const { block, origin } of drawn)
				this._drawTextRasters(
					pane,
					sheetKey,
					block,
					origin,
					twipsPerPixel,
					view,
					background,
				);
			this._evictRasters();

			if (sheet) {
				this._drawTextEdits(pane, sheet, twipsPerPixel);
			}

			context.restore();
		}

		/// Draw visible text strips from rasters, and make missing or stale ones.
		private _drawTextRasters(
			pane: PaneFrame,
			sheetKey: string,
			block: cool.CalcBlockEntry,
			origin: cool.Point,
			twipsPerPixel: number,
			view: { left: number; top: number; right: number; bottom: number },
			background: string,
		): void {
			const resources = CalcVectorManager.instance().resourceVersion();
			for (const strip of cool.calcTextStrips(
				block,
				CalcVectorContentSection.stripRows,
			)) {
				// The raster adds one pixel on each side for glyph edges.
				const left = Math.floor(strip.ink.x / twipsPerPixel) - 1;
				const top = Math.floor(strip.ink.y / twipsPerPixel) - 1;
				const right =
					Math.ceil((strip.ink.x + strip.ink.width) / twipsPerPixel) + 1;
				const bottom =
					Math.ceil((strip.ink.y + strip.ink.height) / twipsPerPixel) + 1;
				if (
					origin.x + right < view.left ||
					origin.x + left > view.right ||
					origin.y + bottom < view.top ||
					origin.y + top > view.bottom
				)
					continue;

				const key = CalcVectorContentSection._rasterKey(
					sheetKey,
					block,
					strip.strip,
				);
				let raster = this._rasters.get(key);
				const current =
					raster !== undefined &&
					CalcVectorContentSection._isCurrent(
						raster,
						block,
						resources,
						background,
						twipsPerPixel,
					);
				if (!current) {
					if (
						this._frameRasterTime < CalcVectorContentSection.frameRasterBudget
					) {
						const started = performance.now();
						const made = this._makeRaster(
							key,
							block,
							strip.parts,
							left,
							top,
							right - left,
							bottom - top,
							twipsPerPixel,
							resources,
							background,
						);
						this._frameRasterTime += performance.now() - started;
						if (raster) {
							this._rasters.delete(key);
							this._rasterBytes -= raster.bytes;
						}
						if (!made.canvas) {
							// A strip too large for a canvas is drawn from its primitives.
							this._drawStripPrimitives(origin, strip.parts, twipsPerPixel);
							continue;
						}
						this._rasterBytes += made.bytes;
						raster = made;
					} else {
						// Out of time this frame. The old raster or placeholders show until a later
						// frame draws the strip.
						this._rasterLeftOver = true;
						if (!raster || !raster.canvas) {
							this._drawTextPlaceholders(pane, strip.parts);
							continue;
						}
					}
				}
				const shown = raster as TextRaster;
				this._rasters.delete(key);
				this._rasters.set(key, shown);
				this._frameRasters.add(key);

				if (shown.canvas && shown.twipsPerPixel !== twipsPerPixel) {
					// A raster of another zoom is drawn scaled, a little off.
					const scale = shown.twipsPerPixel / twipsPerPixel;
					this.context.drawImage(
						shown.canvas,
						origin.x + shown.left * scale,
						origin.y + shown.top * scale,
						shown.canvas.width * scale,
						shown.canvas.height * scale,
					);
					continue;
				}
				raster = shown;
				if (raster.canvas)
					this.context.drawImage(
						raster.canvas,
						origin.x + raster.left,
						origin.y + raster.top,
					);
				else this._drawStripPrimitives(origin, strip.parts, twipsPerPixel);
			}
		}

		/// Draw the text layers of the parts of a strip from their primitives.
		private _drawStripPrimitives(
			origin: cool.Point,
			parts: cool.CalcBlockPart[],
			twipsPerPixel: number,
		): void {
			for (const name of CalcVectorContentSection.rasterLayers)
				for (const part of parts) {
					const primitives = part.content.layers?.[name];
					if (primitives?.length)
						this._drawAt(origin, 0, 0, twipsPerPixel, primitives);
				}
		}

		/// Draw the text layers of a strip into a canvas, if one fits.
		private _makeRaster(
			key: string,
			block: cool.CalcBlockEntry,
			parts: cool.CalcBlockPart[],
			left: number,
			top: number,
			width: number,
			height: number,
			twipsPerPixel: number,
			resources: number,
			background: string,
		): TextRaster {
			const raster: TextRaster = {
				source: block,
				resources,
				background,
				canvas: null,
				left,
				top,
				bytes: 0,
				twipsPerPixel,
			};
			const side = CalcVectorContentSection.maxRasterSide;
			if (width <= 0 || height <= 0 || width > side || height > side)
				return raster;
			const canvas = document.createElement('canvas');
			canvas.width = width;
			canvas.height = height;
			const context = canvas.getContext('2d');
			if (!context) return raster;

			context.translate(-left, -top);
			context.scale(1 / twipsPerPixel, 1 / twipsPerPixel);
			const renderer = CalcVectorManager.instance().renderer;
			for (const name of CalcVectorContentSection.rasterLayers)
				for (const part of parts)
					for (const primitive of part.content.layers?.[name] ?? [])
						renderer.renderPrimitive(context, primitive);

			raster.canvas = canvas;
			raster.bytes = width * height * 4;

			// A canvas can lose its pixels when memory is low. Drop the raster then.
			canvas.addEventListener('contextlost', () => {
				if (this._rasters.get(key) !== raster) return;
				this._rasters.delete(key);
				this._rasterBytes -= raster.bytes;
				if (app.sectionContainer) app.sectionContainer.requestReDraw();
			});
			return raster;
		}

		/// Mark the cells with text of the parts with placeholders.
		private _drawTextPlaceholders(
			pane: PaneFrame,
			parts: cool.CalcBlockPart[],
		): void {
			for (const part of parts)
				for (const primitive of part.content.layers?.text ?? []) {
					const text = primitive as unknown as {
						type: string;
						column?: number;
						row?: number;
					};
					if (
						text.type === 'calcCellText' &&
						text.column !== undefined &&
						text.row !== undefined
					)
						this._drawPlaceholder(pane, text.column, text.row);
				}
		}

		/// Mark every cell of a range whose content is not drawn yet. Each column and row of the
		/// range is looked up once.
		private _drawPlaceholders(pane: PaneFrame, range: CellRange): void {
			const columnsGeometry = pane.geometry.getColumnsGeometry();
			const rowsGeometry = pane.geometry.getRowsGeometry();
			const columns: DimensionPosSize[] = [];
			for (
				let column = range.startColumn;
				column <= range.endColumn;
				++column
			) {
				const columnData = columnsGeometry.getElementData(column);
				if (columnData && columnData.size) columns.push(columnData);
			}
			for (let row = range.startRow; row <= range.endRow; ++row) {
				const rowData = rowsGeometry.getElementData(row);
				if (!rowData || !rowData.size) continue;
				for (const columnData of columns)
					this._drawPlaceholderBar(pane, columnData, rowData);
			}
		}

		/// Mark a cell whose content is not drawn yet with a short gray bar.
		private _drawPlaceholder(
			pane: PaneFrame,
			column: number,
			row: number,
		): void {
			const columnData = pane.geometry
				.getColumnsGeometry()
				.getElementData(column);
			const rowData = pane.geometry.getRowsGeometry().getElementData(row);
			if (!columnData || !rowData || !columnData.size || !rowData.size) return;
			this._drawPlaceholderBar(pane, columnData, rowData);
		}

		private _drawPlaceholderBar(
			pane: PaneFrame,
			columnData: DimensionPosSize,
			rowData: DimensionPosSize,
		): void {
			const width = Math.min(16 * app.dpiScale, columnData.size * 0.4);
			const height = Math.min(5 * app.dpiScale, rowData.size * 0.3);
			const context = this.context;
			context.fillStyle = CalcVectorContentSection.placeholderColor;
			context.fillRect(
				Math.round(
					columnData.startpos - pane.offset.x + (columnData.size - width) / 2,
				),
				Math.round(
					rowData.startpos - pane.offset.y + (rowData.size - height) / 2,
				),
				Math.round(width),
				Math.round(height),
			);
		}

		private static _rasterKey(
			sheetKey: string,
			block: cool.CalcBlockEntry,
			strip: number,
		): string {
			return (
				sheetKey +
				'/' +
				cool.calcBlockKey(block.blockRow, block.blockColumn) +
				'/' +
				String(strip)
			);
		}

		/// Whether the raster matches the current block, resources, background and zoom.
		private static _isCurrent(
			raster: TextRaster,
			block: cool.CalcBlockEntry,
			resources: number,
			background: string,
			twipsPerPixel: number,
		): boolean {
			return (
				raster.source === block &&
				raster.resources === resources &&
				raster.background === background &&
				raster.twipsPerPixel === twipsPerPixel
			);
		}

		/// Queue idle raster jobs: the drawn blocks, then the held blocks below.
		private _queueRasterJobs(
			pane: PaneFrame,
			sheet: cool.CalcSheetData,
			drawn: cool.CalcBlockEntry[],
			ahead: cool.CalcBlockEntry<cool.CalcCellRect>[],
		): void {
			const bounds = pane.bounds;
			const width = bounds.max.x - bounds.min.x;
			const height = bounds.max.y - bounds.min.y;
			// The area one view around the view, in the pixels of the section.
			const area = {
				left: bounds.min.x - pane.offset.x - width,
				top: bounds.min.y - pane.offset.y - height,
				right: bounds.max.x - pane.offset.x + width,
				bottom: bounds.max.y - pane.offset.y + height,
			};
			const queue = (job: RasterBlock): void => {
				const origin = this._cellOrigin(
					pane,
					job.block.startColumn,
					job.block.startRow,
				);
				if (origin) this._rasterJobs.push({ ...job, sheet, origin, area });
			};
			for (const block of drawn) queue({ placed: true, block });
			for (const block of ahead) queue({ placed: false, block });
		}

		private _scheduleIdleRasters(): void {
			if (this._idleScheduled || this._nextRasterJob >= this._rasterJobs.length)
				return;
			this._idleScheduled = true;
			const run = (deadline?: IdleDeadline): void => {
				this._idleScheduled = false;
				// Without idle callbacks, a slice of the frame budget is taken.
				const ends =
					performance.now() + CalcVectorContentSection.frameRasterBudget;
				const timeLeft = (): number =>
					deadline ? deadline.timeRemaining() : ends - performance.now();
				this._runRasterJobs(timeLeft);
				this._scheduleIdleRasters();
			};
			if (typeof window.requestIdleCallback === 'function')
				window.requestIdleCallback(run);
			else setTimeout(run, 50);
		}

		/// Run raster jobs while time is left, stopping when the budget is full.
		private _runRasterJobs(timeLeft: () => number): void {
			if (!app.tile.size) return;
			const twipsPerPixel = app.tile.size.x / RenderManager.tileSize;
			const manager = CalcVectorManager.instance();
			const resources = manager.resourceVersion();
			const background = this.containerObject.getClearColor();
			while (this._nextRasterJob < this._rasterJobs.length) {
				const job = this._rasterJobs[this._nextRasterJob];
				let block: cool.CalcBlockEntry | undefined;
				if (job.placed === true) block = job.block;
				else block = manager.placedBlock(job.sheet, job.block);
				if (!block) {
					this._nextRasterJob++;
					continue;
				}
				const sheetKey = String(job.sheet.part);
				if (sheetKey !== this._rasterSheet) {
					this._nextRasterJob++;
					continue;
				}
				for (const strip of cool.calcTextStrips(
					block,
					CalcVectorContentSection.stripRows,
				)) {
					if (timeLeft() < 1) return;
					const left = Math.floor(strip.ink.x / twipsPerPixel) - 1;
					const top = Math.floor(strip.ink.y / twipsPerPixel) - 1;
					const right =
						Math.ceil((strip.ink.x + strip.ink.width) / twipsPerPixel) + 1;
					const bottom =
						Math.ceil((strip.ink.y + strip.ink.height) / twipsPerPixel) + 1;
					if (
						job.origin.x + right < job.area.left ||
						job.origin.x + left > job.area.right ||
						job.origin.y + bottom < job.area.top ||
						job.origin.y + top > job.area.bottom
					)
						continue;
					const key = CalcVectorContentSection._rasterKey(
						sheetKey,
						block,
						strip.strip,
					);
					const held = this._rasters.get(key);
					if (
						held &&
						CalcVectorContentSection._isCurrent(
							held,
							block,
							resources,
							background,
							twipsPerPixel,
						)
					)
						continue;
					const bytes = (right - left) * (bottom - top) * 4;
					if (
						this._rasterBytes - (held ? held.bytes : 0) + bytes >
						CalcVectorContentSection.rasterBudget
					) {
						// The budget is full. The next draw queues the jobs left again.
						this._rasterJobs = [];
						this._nextRasterJob = 0;
						return;
					}
					const made = this._makeRaster(
						key,
						block,
						strip.parts,
						left,
						top,
						right - left,
						bottom - top,
						twipsPerPixel,
						resources,
						background,
					);
					if (held) {
						this._rasters.delete(key);
						this._rasterBytes -= held.bytes;
					}
					// A strip too large for a canvas is drawn from its primitives.
					if (!made.canvas) continue;
					this._rasters.set(key, made);
					this._rasterBytes += made.bytes;
				}
				this._nextRasterJob++;
			}
		}

		/// Drop old rasters until under budget. Rasters shown this frame stay.
		private _evictRasters(): void {
			for (const [key, raster] of this._rasters) {
				if (this._rasterBytes <= CalcVectorContentSection.rasterBudget) break;
				if (this._frameRasters.has(key)) continue;
				this._rasters.delete(key);
				this._rasterBytes -= raster.bytes;
			}
		}

		/// The top-left corner of a cell in section pixels, or undefined.
		private _cellOrigin(
			pane: PaneFrame,
			column: number,
			row: number,
		): cool.Point | undefined {
			const columnData = pane.geometry
				.getColumnsGeometry()
				.getElementData(column);
			const rowData = pane.geometry.getRowsGeometry().getElementData(row);
			if (!columnData || !rowData) return undefined;
			return new cool.Point(
				columnData.startpos - pane.offset.x,
				rowData.startpos - pane.offset.y,
			);
		}

		/// Whether an ink range in twips from the block origin reaches the view in pixels. One
		/// pixel of margin covers the layers drawn a pixel off.
		private static _paintsInto(
			ink: cool.CalcVectorRect | undefined,
			origin: cool.Point,
			twipsPerPixel: number,
			view: { left: number; top: number; right: number; bottom: number },
		): boolean {
			if (!ink) return true;
			const left = origin.x + ink.x / twipsPerPixel - 1;
			const top = origin.y + ink.y / twipsPerPixel - 1;
			const right = origin.x + (ink.x + ink.width) / twipsPerPixel + 1;
			const bottom = origin.y + (ink.y + ink.height) / twipsPerPixel + 1;
			return (
				right >= view.left &&
				left <= view.right &&
				bottom >= view.top &&
				top <= view.bottom
			);
		}

		/// The grid cover areas of a part, in twips from its block corner.
		private static _coverRanges(
			part: cool.CalcBlockPart,
		): cool.CalcVectorRect[] {
			const ranges: cool.CalcVectorRect[] = [];
			for (const cover of (part.content.layers?.gridCover ??
				[]) as unknown as Array<{
				areas?: Array<{ rect: cool.CalcVectorRect }>;
			}>)
				for (const area of cover.areas ?? []) ranges.push(area.rect);
			return ranges;
		}

		private _drawInCell(
			pane: PaneFrame,
			column: number,
			row: number,
			shiftX: number,
			shiftY: number,
			twipsPerPixel: number,
			primitives: cool.Primitive[],
		): void {
			const origin = this._cellOrigin(pane, column, row);
			if (!origin) return;
			this._drawAt(origin, shiftX, shiftY, twipsPerPixel, primitives);
		}

		/// Draw primitives in twips from the origin, in pixels.
		private _drawAt(
			origin: cool.Point,
			shiftX: number,
			shiftY: number,
			twipsPerPixel: number,
			primitives: cool.Primitive[],
		): void {
			const context = this.context;
			context.save();
			context.translate(origin.x + shiftX, origin.y + shiftY);
			context.scale(1 / twipsPerPixel, 1 / twipsPerPixel);
			const renderer = CalcVectorManager.instance().renderer;
			for (const primitive of primitives)
				renderer.renderPrimitive(context, primitive);
			context.restore();
		}

		/// Hide the grid lines inside merged cells and under overflowing text. The fill ends a
		/// pixel short at the right and bottom, keeping the edge lines.
		private _coverGrid(
			origin: cool.Point,
			part: cool.CalcBlockPart,
			twipsPerPixel: number,
			background: string,
		): void {
			const covers = part.content.layers?.gridCover;
			if (!covers) return;

			const context = this.context;
			context.save();
			context.fillStyle = background;
			for (const cover of covers as unknown as Array<{
				areas?: Array<{ rect: cool.CalcVectorRect }>;
			}>) {
				for (const area of cover.areas ?? []) {
					const x = Math.round(origin.x + area.rect.x / twipsPerPixel);
					const y = Math.round(origin.y + area.rect.y / twipsPerPixel);
					const width = Math.round(area.rect.width / twipsPerPixel) - 1;
					const height = Math.round(area.rect.height / twipsPerPixel) - 1;
					if (width > 0 && height > 0) context.fillRect(x, y, width, height);
				}
			}
			context.restore();
		}

		/// Draw the text that views edit in place. The text of a cell is drawn
		/// on the background of the edited cells.
		private _drawTextEdits(
			pane: PaneFrame,
			sheet: cool.CalcSheetData,
			twipsPerPixel: number,
		): void {
			const context = this.context;
			for (const edit of sheet.textEdits) {
				if (edit.column === undefined || edit.row === undefined) continue;
				const start = this._cellOrigin(pane, edit.column, edit.row);
				const end = this._cellOrigin(
					pane,
					(edit.endColumn ?? edit.column) + 1,
					(edit.endRow ?? edit.row) + 1,
				);
				if (!start) continue;
				if (end && edit.background) {
					context.save();
					context.fillStyle = edit.background;
					context.fillRect(
						start.x,
						start.y,
						end.x - start.x - 1,
						end.y - start.y - 1,
					);
					context.restore();
				}
				this._drawInCell(
					pane,
					edit.column,
					edit.row,
					0,
					0,
					twipsPerPixel,
					edit.primitives,
				);
			}
		}
	}
}
