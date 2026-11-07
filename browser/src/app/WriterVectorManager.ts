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
 * WriterVectorManager - Draws the pages of a text document from vector primitives. Once the
 * cache holds more pages than its budget, it drops the pages that are not wanted now, the ones
 * wanted longest ago first.
 */

class WriterVectorManager extends VectorPartManagerBase {
	/// How many pages the cache holds before it drops any. A page wanted now is never dropped, so
	/// the cache holds more while more pages are wanted. A page that is dropped is asked for
	/// whole again when it comes into view.
	private static readonly textPageBudget: number = 20;

	/// For each page, the value of the wanted count when the page was last wanted. The page
	/// with the lowest value was wanted longest ago.
	private _textPageLastWanted = new Map<string, number>();
	/// How many times the wanted pages were named. It goes up by one each time.
	private _textPageWantedCount = 0;

	// The index of each page id in the page list, and the list of ids the indexes were read
	// from. The list is replaced whole with each status, so the indexes are read again when
	// the list is another one.
	private _pageIndexes = new Map<string, number>();
	private _indexedPageIds: string[] | null = null;

	kind(): RenderManagerKind {
		return 'text';
	}

	/// The rectangle of each page in document twips, as [x, y, width, height], in page order.
	/// Empty until every page has both its rectangle and its id.
	pageRectangles(): number[][] {
		const rectangles: number[][] = app.file.writer.pageRectangleList;
		const pageIds: string[] = app.file.writer.pageIds;
		return rectangles && pageIds && pageIds.length === rectangles.length
			? rectangles
			: [];
	}

	/// The id of the page at the index of the page list, or undefined when the list holds no
	/// page there.
	partIdAt(page: number): cool.VectorPartGuid | undefined {
		return app.file.writer.pageIds[page];
	}

	// The index of the page with the id in the page list, or -1 when the list does not hold
	// the page.
	private _pageIndexOf(partId: string): number {
		const pageIds: string[] = app.file.writer.pageIds;
		if (
			this._indexedPageIds !== pageIds ||
			this._pageIndexes.size !== pageIds.length
		) {
			this._pageIndexes.clear();
			pageIds.forEach((pageId, page) => this._pageIndexes.set(pageId, page));
			this._indexedPageIds = pageIds;
		}
		return this._pageIndexes.get(partId) ?? -1;
	}

	// Where the top left of the page with the id lies in document twips, or null when the page
	// list does not hold the page.
	private _pageOriginOf(partId: string): { x: number; y: number } | null {
		const rectangle = this.pageRectangles()[this._pageIndexOf(partId)];
		return rectangle ? { x: rectangle[0], y: rectangle[1] } : null;
	}

	// The index of the page that the point (x, y), in document twips, lies on, or -1 off the
	// pages.
	private _pageIndexAt(x: number, y: number): number {
		return this.pageRectangles().findIndex(
			(rectangle) =>
				x >= rectangle[0] &&
				x <= rectangle[0] + rectangle[2] &&
				y >= rectangle[1] &&
				y <= rectangle[1] + rectangle[3],
		);
	}

	requestTextPage(partId: string): cool.VectorPrimitivesData | undefined {
		return this.requestPartById(
			partId as cool.VectorPartGuid,
			cool.VectorMode.Slides,
		);
	}

	/// Ask for a page that is near the view ahead of its draw. A few such pages are asked for
	/// at a time and the rest wait until earlier requests are answered. A page that is not among
	/// the pages last named as wanted by then is not asked for. A cached page that is stale asks
	/// for what changed.
	requestTextPageAhead(partId: string): void {
		const cached = this._cache.get(partId as cool.VectorPartGuid);
		if (cached) {
			this._revalidateIfStale(cached);
			return;
		}
		this._requestAhead(
			partId as cool.VectorPartGuid,
			cool.VectorMode.Slides,
			() => this._textPageLastWanted.get(partId) === this._textPageWantedCount,
		);
	}

	/// Draw the entries of a text document page in paint order. The primitives of each entry
	/// are relative to the top left of its box, so each is moved onto its box.
	renderTextPage(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
	): void {
		this._setUpRenderer(data, /*editView=*/ false, /*authoredColors=*/ false);
		for (const id of data.order) {
			const entry = data.objects.get(id);
			if (!entry?.primitives) continue;
			// The context comes back as it was, also when a primitive throws.
			context.save();
			try {
				context.translate(entry.x ?? 0, entry.y ?? 0);
				for (const primitive of entry.primitives)
					this._renderer.renderPrimitive(context, primitive);
			} finally {
				context.restore();
			}
		}
	}

	keepTextPages(partIds: Set<string>): void {
		const wantedCount = ++this._textPageWantedCount;
		for (const partId of partIds)
			this._textPageLastWanted.set(partId, wantedCount);
		if (this._cache.size <= WriterVectorManager.textPageBudget) return;

		const lastWanted = (partId: string): number =>
			this._textPageLastWanted.get(partId) ?? 0;
		const droppable = Array.from(this._cache.keys())
			.filter((partId) => !partIds.has(partId))
			.sort((one, other) => lastWanted(one) - lastWanted(other));
		for (const partId of droppable) {
			if (this._cache.size <= WriterVectorManager.textPageBudget) break;
			// The pages kept are drawn already, so dropping the others needs no redraw.
			this._dropCachedPart(partId as cool.VectorPartGuid);
			this._textPageLastWanted.delete(partId);
			// The session stops pushing changes of the page, which would only be dropped here.
			app.socket.sendMessage(
				'commandvalues command=.uno:VectorForgetPart?partid=' + partId,
			);
		}
	}
}
