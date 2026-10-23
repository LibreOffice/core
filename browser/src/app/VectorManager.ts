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
 * VectorManager - Renders the document content with vector primitives.
 *
 * It owns the per-part vector data, the bitmap cache and the primitive
 * renderer. It exposes thumbnail rendering (requestThumbnail) and per-part
 * access with change notification (requestPart, renderInto, onVectorChanged)
 * over one shared cache, so each part is fetched and decoded only once. The
 * bitmap-grid behaviour stays as the no-ops inherited from RenderManagerBase:
 * a vector-rendered document has no bitmap tile grid.
 */

/// The code points deleted before and after the caret, and the text then inserted there.
interface PredictedChange {
	text?: string;
	deleteBefore?: number;
	deleteAfter?: number;
}

/// A predicted edit the server has not acknowledged yet.
interface PredictedTextEdit extends PredictedChange {
	/// The sequence number of the edit's input message.
	sequence: number;
	partId: cool.VectorPartGuid;
	/// The id of the edited entry.
	objectId: number;
	/// The caret before and after the edit.
	caretBefore: cool.LogicalCaret;
	caretAfter: cool.LogicalCaret;
}

/// The cached tree and caret that a predicted edit works on.
interface EditTarget {
	tree: cool.PredictionTree;
	caret: cool.SimpleRectangle;
	logical: cool.LogicalCaret | null;
}

class VectorManager extends RenderManagerBase {
	private _renderer: cool.VectorPrimitiveRenderer =
		new cool.VectorPrimitiveRenderer(
			(checksum) => this._bitmaps.get(checksum),
			(fontId) => this._fonts.has(fontId),
		);

	// Cached parsed JSON primitive tree keyed by page id. The id names the
	// page wherever it sits in its list, so a slide inserted or removed
	// before a cached page leaves that page's content valid.
	private _cache: Map<cool.VectorPartGuid, cool.VectorPrimitivesData> =
		new Map();

	// Previews waiting for a JSON primitive tree response, keyed by page id.
	private _pendingPreviews: Map<cool.VectorPartGuid, cool.PendingPreview[]> =
		new Map();

	// Pages a JSON primitive tree request is in flight for, keyed by page id.
	private _inFlightParts: Set<cool.VectorPartGuid> = new Set();

	// Cached pages that may be behind the document, because the client was
	// away while they could change. A stale page is still drawn from the
	// cache, and its next draw asks for what changed since the version held.
	private _staleParts: Set<cool.VectorPartGuid> = new Set();

	// Pages the document said it does not hold. Asking again is pointless
	// until the page list is replaced.
	private _missingParts: Set<cool.VectorPartGuid> = new Set();

	// Decoded bitmap images keyed by their checksum.
	private _bitmaps = new VectorResourceTracker<number, HTMLImageElement>(
		(checksum) =>
			'commandvalues command=.uno:VectorRenderingGraphics?checksum=' +
			String(checksum),
	);

	// Loaded font faces keyed by font id, registered on document.fonts
	// under the family name "vecfont-<id>".
	private _fonts = new VectorResourceTracker<string, FontFace>(
		(fontId) => 'commandvalues command=.uno:VectorRenderingFont?id=' + fontId,
	);

	// Size and part of every preview that has been rendered. A redraw
	// triggered by a decoded bitmap reuses these so the re-fired
	// preview keeps the size and part it was rendered at.
	private _renderedPreviews: Map<cool.PreviewId, cool.RenderedPreview> =
		new Map();

	// Callbacks fired when cached vector data changes: a part's
	// primitive tree arrived, a deferred bitmap decoded, or the cache
	// was cleared.
	private _changeListeners: ((answered?: number[]) => void)[] = [];

	// Ids of the layers this view does not show. Objects on them are
	// kept in the cache and skipped when drawing.
	private _hiddenLayers: Set<number> = new Set();

	/// The objects left out of the drawing of a part because they are drawn elsewhere, by id.
	private _drawnElsewhere: Set<number> = new Set();
	/// Counts up each time the objects left out of the drawing change, so a drawing kept from
	/// before can be told apart from one that matches them.
	private _drawnElsewhereVersion: number = 0;

	// The Impress or Draw doc layer, read lazily so the manager can be
	// created before the layer is registered on the map.
	private get _docLayer(): cool.CanvasTileLayerInterface {
		return app.map._docLayer as unknown as cool.CanvasTileLayerInterface;
	}

	/// The id of the view this client is, or undefined before the document
	/// reported it or before there is a document at all.
	private get _ownViewId(): number | undefined {
		const docLayer = app.map?._docLayer as unknown as
			| cool.CanvasTileLayerInterface
			| undefined;
		return docLayer?._viewId;
	}

	isVectorRendering(): boolean {
		return true;
	}

	/*
		Register a callback fired whenever cached vector data changes. It is handed the ids of
		the objects an answer from the engine replaced, an empty list where the change was made
		here and the engine answered nothing, and nothing at all where everything may have
		changed, as when a whole part arrives.
	*/
	onVectorChanged(callback: (answered?: number[]) => void): void {
		this._changeListeners.push(callback);
	}

	/// The view draws the part that is now selected, from the cache when it holds it, at once
	/// and without an answer from the engine.
	updateOnChangePart(): void {
		this._fireChanged();
	}

	/// Forget a callback registered with onVectorChanged.
	offVectorChanged(callback: () => void): void {
		this._changeListeners = this._changeListeners.filter(
			(listener) => listener !== callback,
		);
	}

	private _fireChanged(answered?: number[]): void {
		for (const callback of this._changeListeners) callback(answered);
	}

	/*
		Says that the objects the client holds have changed here rather than at the engine, which
		is what a drag does when it lays what it did on them. Whoever keeps a picture of a part
		draws it again, as it does when an update arrives.
	*/
	changedHere(): void {
		this._fireChanged([]);
	}

	/// The id of the page at the given index of the page list on screen, or
	/// undefined when the list on screen is of another mode or holds no page
	/// at the index. The status message names every page of that list by id.
	partIdAt(part: number, mode: number): cool.VectorPartGuid | undefined {
		const entry = app.impress?.partList?.[part];
		if (entry && entry.mode === mode && typeof entry.part === 'string')
			return entry.part;
		return undefined;
	}

	/// The index of the page with the given id in the page list on screen, or
	/// -1 when the list on screen is of another mode or does not hold the page.
	private _indexOf(partId: cool.VectorPartGuid, mode: number): number {
		const list = app.impress?.partList;
		if (!list) return -1;
		return list.findIndex(
			(entry: any) => entry.mode === mode && entry.part === partId,
		);
	}

	/// Return the cached primitive tree for the page at the given index of
	/// the page list on screen, or undefined while a request is sent or while
	/// the list does not name the page. Subscribers registered with
	/// onVectorChanged are notified once the data arrives, and again when the
	/// page list is replaced.
	requestPart(
		part: number,
		mode: number,
	): cool.VectorPrimitivesData | undefined {
		const partId = this.partIdAt(part, mode);
		if (partId === undefined) return undefined;
		return this.requestPartById(partId, mode);
	}

	/// Return the cached primitive tree for the page with the given id, or
	/// undefined while a request is sent.
	requestPartById(
		partId: cool.VectorPartGuid,
		mode: number,
	): cool.VectorPrimitivesData | undefined {
		const cached = this._cache.get(partId);
		if (cached) {
			this._revalidateIfStale(cached);
			return cached;
		}

		this._request(partId, mode);
		return undefined;
	}

	/// Ask for what changed on a stale page since the version held. A request
	/// already in flight brings the current content too, so it is not sent
	/// twice.
	private _revalidateIfStale(held: cool.VectorPrimitivesData): void {
		const partId = held.partId;
		if (!this._staleParts.delete(partId) || this._inFlightParts.has(partId))
			return;
		this._inFlightParts.add(partId);
		this._sendVectorPrimitivesRequest(partId, held.mode, held);
	}

	/// Send for a page unless a request for it is already out.
	private _request(partId: cool.VectorPartGuid, mode: number): void {
		if (this._inFlightParts.has(partId) || this._missingParts.has(partId))
			return;
		this._inFlightParts.add(partId);
		this._sendVectorPrimitivesRequest(partId, mode);
	}

	/// Listen for the layers this view hides. The engine reports them as a
	/// state change whenever the view's layer state is rebuilt.
	initialize(): void {
		app.map.on('commandstatechanged', (event: any) => {
			if (event.commandName === '.uno:LayerVisibility')
				this.setHiddenLayers(event.state);
		});
	}

	/// The layers this view hides, given as their ids. Objects on them stay
	/// cached and are skipped when drawing. Anything that is not a list
	/// hides nothing.
	setHiddenLayers(layers: unknown): void {
		const ids = Array.isArray(layers)
			? layers.filter((id): id is number => typeof id === 'number')
			: [];
		const next = new Set<number>(ids);
		const same =
			next.size === this._hiddenLayers.size &&
			ids.every((id) => this._hiddenLayers.has(id));
		if (same) return;
		this._hiddenLayers = next;
		// Hiding a layer changes nothing at the engine, so no object is answered.
		this._fireChanged([]);
	}

	isLayerVisible(layer: number): boolean {
		return !this._hiddenLayers.has(layer);
	}

	/// The objects to leave out of the drawing of a part from now on, since they are drawn
	/// elsewhere. An empty list leaves nothing out.
	drawnElsewhere(objectIds: number[]): void {
		const next = new Set(objectIds);
		const same =
			next.size === this._drawnElsewhere.size &&
			objectIds.every((id) => this._drawnElsewhere.has(id));
		if (same) return;
		this._drawnElsewhere = next;
		this._drawnElsewhereVersion++;
	}

	drawnElsewhereVersion(): number {
		return this._drawnElsewhereVersion;
	}

	/// The page rectangle carried by the entry that stands for the page, or
	/// zeroes when there is no such entry among the given objects.
	static pageBoundsOf(objects: cool.SlideObject[]): [number, number] {
		for (const object of objects) {
			if (object.kind === 'page')
				return [object.width ?? 0, object.height ?? 0];
		}
		return [0, 0];
	}

	/// Tell the renderer what the automatic colors mean in this view and what
	/// the page holds behind its objects. The tree carries both unresolved,
	/// and the renderer keeps what it was last given.
	private _setUpColors(data: cool.VectorPrimitivesData): void {
		this._renderer.setDocumentBackground(
			cool.VectorColors.documentBackground(),
		);
		const page = VectorManager._pageEntryOf(data.objects.values());
		this._renderer.setPageBackdrop(page?.backdrop, page?.backdropTransparency);
	}

	/// The entry that stands for the page among the given objects, if any.
	private static _pageEntryOf(
		objects: Iterable<cool.SlideObject>,
	): cool.SlideObject | undefined {
		for (const object of objects) {
			if (object.kind === 'page') return object;
		}
		return undefined;
	}

	/// Take the master reference the page entry carries into the cached part
	/// and fetch the master, so it is usually cached before the first paint.
	private _takeMasterReference(
		data: cool.VectorPrimitivesData,
		page: cool.SlideObject,
	): void {
		data.masterPartId = page.masterPartId;
		data.masterHiddenLayers = page.masterHiddenLayers
			? new Set(page.masterHiddenLayers)
			: undefined;
		if (data.masterPartId !== undefined)
			this.requestPartById(data.masterPartId, cool.VectorMode.MasterPages);
	}

	/// True when the part and the master it names are both cached. Fetches
	/// whatever is missing.
	isPartDrawable(part: number, mode: number): boolean {
		const data = this.requestPart(part, mode);
		if (!data) return false;
		return this._isDrawable(data);
	}

	/// True when the page with the given id and the master it names are both
	/// cached. Fetches whatever is missing.
	isPartDrawableById(partId: cool.VectorPartGuid, mode: number): boolean {
		const data = this.requestPartById(partId, mode);
		if (!data) return false;
		return this._isDrawable(data);
	}

	private _isDrawable(data: cool.VectorPrimitivesData): boolean {
		if (data.masterPartId === undefined) return true;
		return (
			this.requestPartById(data.masterPartId, cool.VectorMode.MasterPages) !==
			undefined
		);
	}

	/// Render a part's objects in paint order, the page entry first. The
	/// caller sets up the context transform that maps the part's twips to
	/// the target pixels. Objects on a hidden layer are skipped.
	renderInto(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		options?: cool.VectorRenderOptions,
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		this._renderer.setEditViewContentVisible(options?.editView === true);
		this._setUpColors(data);
		const textEdits = this._textEditEntriesToDraw(data);
		for (const id of data.order) {
			const obj = data.objects.get(id);
			if (!obj) continue;
			if (obj.layer !== undefined && this._hiddenLayers.has(obj.layer))
				continue;
			if (obj.kind === 'texteditoverlay' && !textEdits.has(id)) continue;
			if (this._drawnElsewhere.has(id)) continue;
			// The slide's copy of a master placeholder is drawn with the master.
			if (obj.masterContent) continue;
			if (obj.primitives) {
				for (const primitive of obj.primitives) {
					this._renderer.renderPrimitive(context, primitive);
				}
			}
			// The master lies between the page's own background and its objects.
			if (obj.kind === 'page') this._renderMaster(context, data);
		}
	}

	/*
		Draw the objects named, with a transformation laid over what they draw. It is the picture
		a drag shows of itself: the objects as they would look where the drag has taken them. The
		caller sets up the context the same way as for a whole part.
	*/
	renderObjectsWith(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		objectIds: number[],
		matrix: cool.Matrix2D,
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		this._renderer.setEditViewContentVisible(true);
		this._setUpColors(data);

		for (const id of objectIds) {
			const object = data.objects.get(id);
			if (!object?.primitives) continue;

			// the tail of a caption keeps its end, so it is laid out anew rather than carried
			this._renderer.renderPrimitive(context, {
				type: cool.TransformPrimitive.type,
				matrix: matrix.toArray(),
				children:
					cool.CaptionTail.changedBy(object, matrix) ?? object.primitives,
			} as cool.TransformPrimitive);
		}
	}

	/// Draw the master that a page names under it, in the master's order. A
	/// shared object comes from the master, a per-slide one from the page's
	/// own copy, if it has one. An object on a layer the page hides is left
	/// out. The master's page entry is skipped, since the page draws its own
	/// background. A text edit on a master object is drawn from its edit
	/// entry, since the object hides its text during the edit.
	private _renderMaster(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
	): void {
		if (data.masterPartId === undefined) return;
		const master = this._cache.get(data.masterPartId);
		if (!master) return;

		const textEdits = this._textEditEntriesToDraw(master);
		for (const id of master.order) {
			const obj = master.objects.get(id);
			if (!obj || obj.hiddenBehindSlide) continue;
			if (obj.kind !== undefined) {
				if (obj.kind !== 'texteditoverlay' || !textEdits.has(id)) continue;
				const parent = master.objects.get(obj.parent ?? 0);
				if (parent?.hiddenBehindSlide) continue;
			}
			if (obj.layer !== undefined && this._hiddenLayers.has(obj.layer))
				continue;
			if (obj.layer !== undefined && data.masterHiddenLayers?.has(obj.layer))
				continue;
			const drawn = obj.slideDependent ? data.objects.get(id) : obj;
			if (!drawn?.primitives) continue;
			for (const primitive of drawn.primitives) {
				this._renderer.renderPrimitive(context, primitive);
			}
		}
	}

	/// The text edit entries to draw, one per edited object: this view's own
	/// where it has one, otherwise the first in the order. Several views can
	/// edit one object, and drawing every entry would paint the text twice.
	/// One entry is always drawn, whichever view is editing, because an object
	/// under edit carries none of its own text, so the edit entry is the only
	/// source of that text.
	private _textEditEntriesToDraw(data: cool.VectorPrimitivesData): Set<number> {
		const ownViewId = this._ownViewId;
		const chosen = new Map<number, cool.SlideObject>();
		for (const id of data.order) {
			const obj = data.objects.get(id);
			if (!obj || obj.kind !== 'texteditoverlay') continue;
			const parent = obj.parent ?? 0;
			const current = chosen.get(parent);
			const isOwn = ownViewId !== undefined && obj.viewId === ownViewId;
			if (!current || (isOwn && current.viewId !== ownViewId))
				chosen.set(parent, obj);
		}
		const ids = new Set<number>();
		for (const obj of chosen.values()) {
			if (obj.id !== undefined) ids.add(obj.id);
		}
		return ids;
	}

	/// Draw the aids that mark out the placeholders on a page: the dashed
	/// boundary of each and, on a master page, the name of the area. They
	/// are drawn over the page content, so a later object does not cover
	/// them, and objects on a hidden layer are skipped as ever.
	renderPlaceholderAids(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		this._setUpColors(data);
		for (const id of data.order) {
			const obj = data.objects.get(id);
			if (!obj || !obj.aids) continue;
			if (obj.layer !== undefined && this._hiddenLayers.has(obj.layer))
				continue;
			for (const primitive of obj.aids) {
				this._renderer.renderPrimitive(context, primitive);
			}
		}
	}

	/*
		Draw primitives the client built itself, in document twips, the way the objects of the
		part are drawn. The caller sets up the context the same way as for a whole part.
	*/
	renderPrimitives(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		primitives: cool.Primitive[],
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		this._renderer.setEditViewContentVisible(true);
		this._setUpColors(data);

		for (const primitive of primitives)
			this._renderer.renderPrimitive(context, primitive);
	}

	/// Redraw what is on screen in the view's current theme. The cached
	/// trees hold the automatic colors unresolved, so a theme change
	/// costs a repaint and nothing else.
	refreshTheme(): void {
		for (const [id, info] of this._renderedPreviews) {
			const data = this._cache.get(info.partId);
			if (data) this._renderAndFire(id, info.maxWidth, info.maxHeight, data);
		}
		this._fireChanged();
	}

	/// Request a thumbnail for a preview of the page at the given index of
	/// the page list on screen. Nothing is drawn while the list does not name
	/// the page.
	requestThumbnail(
		id: cool.PreviewId,
		part: number,
		mode: number,
		maxWidth: number,
		maxHeight: number,
	): void {
		const partId = this.partIdAt(part, mode);
		if (partId === undefined) return;
		const cached = this._cache.get(partId);
		if (cached) this._revalidateIfStale(cached);
		if (cached && this._isDrawable(cached)) {
			this._renderAndFire(id, maxWidth, maxHeight, cached);
			return;
		}

		// The part or the master it names is still on its way, so the preview
		// waits for whichever arrives last.
		let queue = this._pendingPreviews.get(partId);
		if (!queue) {
			queue = [];
			this._pendingPreviews.set(partId, queue);
		}
		// One preview waits once, at the size it last asked for.
		const pending = { id: id, maxWidth: maxWidth, maxHeight: maxHeight };
		const waiting = queue.findIndex((entry) => entry.id === id);
		if (waiting >= 0) queue[waiting] = pending;
		else queue.push(pending);

		if (!cached) this._request(partId, mode);
	}

	private _sendVectorPrimitivesRequest(
		partId: cool.VectorPartGuid,
		mode: number,
		held?: cool.VectorPrimitivesData,
	): void {
		// A client that holds the page says at which version, and in which
		// version space, so the answer is the step from there rather than
		// the whole page.
		const since =
			held?.version !== undefined && held.epoch !== undefined
				? '&since=' + String(held.version) + '&epoch=' + String(held.epoch)
				: '';
		app.socket.sendMessage(
			'commandvalues command=.uno:VectorPrimitives?partid=' +
				partId +
				'&mode=' +
				String(mode) +
				since,
		);
	}

	/// Collect the bitmap checksums and font ids the walk visits,
	/// remember the part uses them and request the missing ones.
	private _collectResources(
		partId: cool.VectorPartGuid,
		walk: (walker: cool.VectorResourceWalker) => void,
	): void {
		const checksums = new Set<number>();
		const fontIds = new Set<string>();
		walk(new cool.VectorResourceWalker(checksums, fontIds));
		this._bitmaps.indexForPart(partId, checksums);
		this._bitmaps.requestMissing(checksums);
		this._fonts.indexForPart(partId, fontIds);
		this._fonts.requestMissing(fontIds);
	}

	/// Handle a vector primitives response.
	handleVectorPrimitivesResponse(values: cool.VectorPrimitivesResponse): void {
		const mode =
			values.mode !== undefined ? values.mode : cool.VectorMode.Slides;
		// Every request names its page by id, and the answer names it back.
		const partId = values.partId;
		if (partId === undefined) return;
		this._inFlightParts.delete(partId);

		// A response with no objects names a part the document does not
		// hold. It happens while the page list is catching up with a mode
		// switch, or when the page was removed while the client was away. So
		// nothing stays cached for it, the previews queued for a page that
		// is not there are dropped, and the page is not asked for again until
		// the page list is replaced.
		if (values.objects === undefined) {
			this._cache.delete(partId);
			this._staleParts.delete(partId);
			this._pendingPreviews.delete(partId);
			this._missingParts.add(partId);
			return;
		}

		// A full response older than the cache would roll it back, so the
		// cache keeps its newer version and the waiting previews are drawn
		// from that. Versions from two epochs cannot be compared, so a
		// response from another epoch always replaces the cache.
		const held = this._cache.get(partId);
		if (
			held?.epoch === values.epoch &&
			held?.version !== undefined &&
			values.version !== undefined &&
			values.version < held.version
		) {
			this._drainDrawable(partId);
			return;
		}

		const received = values.objects;
		const objects = new Map<number, cool.SlideObject>();
		const arrived: number[] = [];
		for (const object of received) {
			if (object.id === undefined) continue;
			objects.set(object.id, object);
			arrived.push(object.id);
		}

		const [nWidth, nHeight] = VectorManager.pageBoundsOf(received);
		const data: cool.VectorPrimitivesData = {
			partId: partId,
			mode: mode,
			epoch: values.epoch,
			version: values.version,
			slideWidth: nWidth,
			slideHeight: nHeight,
			objects: objects,
			// A full response lists the objects in paint order, so the order
			// they arrive in is the order they are drawn in.
			order: values.order || arrived,
		};
		const page = VectorManager._pageEntryOf(received);
		if (page) this._takeMasterReference(data, page);
		this._cache.set(partId, data);
		this._staleParts.delete(partId);

		this._collectResources(partId, (walker) => {
			walker.walkObjects(received);
		});

		// A preview already shown for the page is drawn again from the new
		// content. This is how a client that asked for its cached pages again
		// after being away gets fresh thumbnails. A preview that was waiting
		// for this response is drawn by the drain alone.
		const drained = this._drainDrawable(partId);
		if (this._isDrawable(data)) this._redrawRenderedPreviews(partId, drained);
		if (mode === cool.VectorMode.MasterPages) this._onMasterArrived(partId);
		this._reconcile(partId, values.inputSequence);
		this._fireChanged();
		this.setVisualsReady();
	}

	/// A master arrived or changed, so every page drawing under it can be
	/// drawn now, and every preview showing such a page is drawn again. Which
	/// pages those are is read off the cache.
	private _onMasterArrived(masterPartId: cool.VectorPartGuid): void {
		for (const [partId, data] of this._cache) {
			if (data.masterPartId !== masterPartId) continue;
			this._drainDrawable(partId);
			this._redrawRenderedPreviews(partId);
		}
	}

	/// Apply a pushed delta and replay the pending edits on the objects it
	/// replaced.
	handleVectorPrimitivesDelta(values: cool.VectorPrimitivesResponse): void {
		const partId = values.partId;
		if (partId === undefined) return;
		const replaced = this._applyDelta(values);
		if (!replaced) return;
		this._reconcile(partId, values.inputSequence, replaced);
	}

	/// Apply a delta to a cached part. The objects it carries replace
	/// theirs and the rest keep what was cached. A delta not newer than
	/// the cache is ignored. A part that is not cached, a delta that starts
	/// above the version held, or an order that names content the client
	/// never had, falls back to a full re-fetch. Returns the ids of the
	/// objects it replaced, or null when the cached tree is unchanged.
	private _applyDelta(values: cool.VectorPrimitivesResponse): number[] | null {
		const partId = values.partId;
		if (partId === undefined) return null;

		const cached = this._cache.get(partId);
		if (!cached) return null;

		// A delta is also what answers a request that said which version the
		// client holds, and only a client that holds the page says that.
		this._inFlightParts.delete(partId);

		// A delta from another epoch counts its versions from another start, so
		// it says nothing about what is cached and the part is fetched whole.
		if (cached.epoch !== values.epoch) {
			this.clearCachedPart(partId);
			return null;
		}

		// Every later delta is checked against the version, so a delta without
		// one cannot be applied.
		if (values.version === undefined) {
			window.app.console.warn(
				'VectorPrimitivesDelta: delta for part ' +
					partId +
					' carries no version and is ignored',
			);
			return null;
		}

		// A delta computed against an older version can arrive after a
		// newer full response. Its order describes that older state, so
		// applying it would roll the cache backwards.
		if (
			cached.version !== undefined &&
			values.version !== undefined &&
			values.version <= cached.version
		)
			return null;

		// A delta that starts at or below the version held carries every object that changed
		// since, and each entry replaces a whole object, so it is applied as it is. A delta that
		// starts above the version held leaves out the objects that changed in between, and no
		// later delta carries them, so the part is dropped and the next draw asks for it whole.
		if (
			cached.version !== undefined &&
			values.from !== undefined &&
			values.from > cached.version
		) {
			this.clearCachedPart(partId);
			return null;
		}

		const carried = values.objects || [];
		// The order is the paint order, so an object it does not name has no
		// place to be drawn. Without a new order the part is fetched whole.
		if (!values.order) {
			const known = new Set(cached.order);
			if (
				carried.some(
					(object) => object.id !== undefined && !known.has(object.id),
				)
			) {
				this.clearCachedPart(partId);
				return null;
			}
		}
		for (const object of carried) {
			if (object.id !== undefined) cached.objects.set(object.id, object);
		}

		// The page rectangle rides on the page entry, so a delta that carries
		// that entry is also how a resized page reaches the client. So does the
		// master the page draws under itself and the page's copies of its objects.
		const [nWidth, nHeight] = VectorManager.pageBoundsOf(carried);
		if (nWidth > 0 && nHeight > 0) {
			cached.slideWidth = nWidth;
			cached.slideHeight = nHeight;
		}
		const page = VectorManager._pageEntryOf(carried);
		if (page) this._takeMasterReference(cached, page);

		// The order travels only when the object set or its order changed.
		// When it does it names the whole live set, so anything missing from
		// it is gone and is dropped from the cache.
		if (values.order) {
			for (const id of values.order) {
				if (!cached.objects.has(id)) {
					this.clearCachedPart(partId);
					return null;
				}
			}
			const live = new Set(values.order);
			const gone: number[] = [];
			for (const id of cached.objects.keys()) {
				if (!live.has(id)) gone.push(id);
			}
			for (const id of gone) cached.objects.delete(id);
			cached.order = values.order;
		}

		cached.version = values.version;

		this._collectResources(partId, (walker) => {
			walker.walkObjects(carried);
		});

		this._redrawRenderedPreviews(partId);
		if (cached.mode === cool.VectorMode.MasterPages)
			this._onMasterArrived(partId);
		const replaced = carried.flatMap((object) =>
			object.id === undefined ? [] : [object.id],
		);
		this._fireChanged(replaced);
		return replaced;
	}

	// Shared off-screen 2d context for measuring text, created on first use.
	private static _measureContext: CanvasRenderingContext2D | null = null;

	private static measureContext(): CanvasRenderingContext2D | null {
		if (!VectorManager._measureContext) {
			const canvas = document.createElement('canvas');
			const context = canvas.getContext('2d');
			// The prediction measures text one code point at a time and sums the widths. With
			// kerning off the sum equals the width of the whole string, where the browser
			// supports the setting.
			if (context && 'fontKerning' in context) context.fontKerning = 'none';
			VectorManager._measureContext = context;
		}
		return VectorManager._measureContext;
	}

	// The width of a string as the renderer draws the run, so predicted widths
	// match the screen. Every width is zero without a 2d context.
	protected _textMeasure(): cool.TextRunMeasure {
		return (run, text) => {
			const context = VectorManager.measureContext();
			if (!context) return 0;
			const family = cool.VectorPrimitiveRenderer.resolveFontFamily(
				run,
				(fontId) => this._fonts.has(fontId),
			);
			context.font = cool.VectorPrimitiveRenderer.cssFont(run, family);
			return context.measureText(text).width;
		};
	}

	// The measurer over that width, created on first use.
	private _textMeasurer: cool.TextMeasurer | null = null;

	private _measurer(): cool.TextMeasurer {
		if (!this._textMeasurer)
			this._textMeasurer = new cool.TextMeasurer(this._textMeasure());
		return this._textMeasurer;
	}

	private _treeOf(data: cool.VectorPrimitivesData): cool.PredictionTree {
		return new cool.PredictionTree(data, this._measurer());
	}

	// The measured text of the last entry pointed at, reused while the tree still holds it.
	private _objectText: { entryId: number; text: cool.ObjectText } | null = null;

	private _textOf(
		tree: cool.PredictionTree,
		entryId: number,
	): cool.ObjectText | null {
		const kept = this._objectText;
		if (kept && kept.entryId === entryId && kept.text.isOf(tree, entryId))
			return kept.text;
		const text = cool.ObjectText.of(tree, entryId);
		this._objectText = text ? { entryId, text } : null;
		return text;
	}

	/// The id of the page in the edit view, or undefined while the page list
	/// does not name it.
	private _selectedPartId(): cool.VectorPartGuid | undefined {
		const entry = app.impress?.partList?.[this._docLayer._selectedPart];
		return typeof entry?.part === 'string' ? entry.part : undefined;
	}

	/// The tree with the entries of other views' running text edits left out of
	/// the order. Every view that edits an object has its own entry for it.
	private _asThisViewEditsIt(
		data: cool.VectorPrimitivesData,
	): cool.VectorPrimitivesData {
		const ownViewId = this._ownViewId;
		if (ownViewId === undefined) return data;
		const order = data.order.filter((id) => {
			const object = data.objects.get(id);
			return object?.kind !== 'texteditoverlay' || object.viewId === ownViewId;
		});
		return order.length === data.order.length ? data : { ...data, order };
	}

	// The cached tree and caret of the edited part, or null when no caret is visible, the part
	// is not cached, or text is selected that the edit does not cover.
	private _editTarget(overSelection = false): EditTarget | null {
		// An edit covers a selection in the caret's paragraph whose ends the client knows.
		// A selection over a paragraph break alone draws no rectangle, so its anchor counts
		// too.
		const selected =
			TextSelections.isActive() || Boolean(app.file.textCursor.anchor);
		if (selected && !(overSelection && this._selectionAnchorInParagraph()))
			return null;
		return this._caretTarget();
	}

	// The other end of a selection in the caret's paragraph, or null when there is none or its
	// start is not known.
	private _selectionAnchorInParagraph(): cool.LogicalCaret | null {
		const cursor = app.file.textCursor;
		const anchor = cursor.anchor;
		return anchor &&
			cursor.logical &&
			anchor.paragraph === cursor.logical.paragraph
			? anchor
			: null;
	}

	// The cached tree and caret of the edited part, with text selected or not, or null when no
	// caret is visible or the part is not cached.
	private _caretTarget(): EditTarget | null {
		const cursor = app.file.textCursor;
		if (!cursor || !cursor.visible || !cursor.rectangle) return null;
		const partId = this._selectedPartId();
		const data = partId === undefined ? undefined : this._cache.get(partId);
		if (!data) return null;
		return {
			tree: this._treeOf(this._asThisViewEditsIt(data)),
			caret: cursor.rectangle,
			logical: cursor.logical ?? null,
		};
	}

	// The caret position in the edit target, or null.
	private _lookUpCaret(target: EditTarget): cool.CaretPosition | null {
		return target.tree.caretAt(
			target.caret.x1,
			target.caret.y1,
			target.caret.y2,
			target.logical ?? undefined,
		);
	}

	/// True when a keystroke at the caret can be predicted. The tree stays
	/// unchanged.
	canPredictAtCaret(): boolean {
		const target = this._editTarget();
		return target !== null && this._lookUpCaret(target) !== null;
	}

	/// Insert typed text into the cached tree so it shows before the server re-renders. It
	/// replaces a selection in the caret's paragraph, as in the engine. True when the tree
	/// changed.
	predictTextInsert(text: string): boolean {
		return this._predictEdit((selected) =>
			selected
				? { text, deleteBefore: selected.before, deleteAfter: selected.after }
				: { text },
		);
	}

	/// Delete before characters left of the caret and after characters right of it in the
	/// cached tree. A selection in the caret's paragraph is the first one deleted, as in the
	/// engine. True when the tree changed.
	predictTextDelete(before: number, after: number): boolean {
		if (before <= 0 && after <= 0) return false;
		return this._predictEdit((selected) => {
			if (selected) {
				// The engine takes one key to delete the selection, whichever kind
				// comes first.
				if (before > 0) before--;
				else after--;
				before += selected.before;
				after += selected.after;
			}
			return { deleteBefore: before, deleteAfter: after };
		});
	}

	/*
		The anchor a move here selects from, or null for no selection. Undefined when the
		move is for the server: it drops a selection, or the anchor is not known here.
	*/
	private _anchorForMove(
		caret: cool.LogicalCaret,
		extend: boolean,
	): cool.LogicalCaret | null | undefined {
		const anchor = app.file.textCursor.anchor ?? null;
		// A selection over a paragraph break alone draws no rectangle and still has its
		// anchor.
		if (extend && anchor) return anchor;
		if (TextSelections.isActive()) return undefined;
		return extend ? { paragraph: caret.paragraph, index: caret.index } : null;
	}

	/// Move the caret one character left (step -1) or right (step 1). With extend, the
	/// selection follows the caret. True when it moved.
	moveCaretByCharacter(step: -1 | 1, extend: boolean = false): boolean {
		return this._stepCaret(extend, (position, logical) =>
			cool.stepByCharacter(position, logical, step),
		);
	}

	/// Move the caret one word left (step -1) or right (step 1). With extend, the selection
	/// follows the caret. True when it moved.
	moveCaretByWord(step: -1 | 1, extend: boolean = false): boolean {
		return this._stepCaret(extend, (position, logical) =>
			cool.stepByWord(position, logical, step),
		);
	}

	// Move the caret to where step takes it, and with extend select from the anchor to it. True
	// when it moved.
	private _stepCaret(
		extend: boolean,
		step: (
			position: cool.CaretPosition,
			logical: cool.LogicalCaret,
		) => cool.LogicalCaret | null,
	): boolean {
		const target = this._caretTarget();
		if (!target || !target.logical) return false;
		const anchor = this._anchorForMove(target.logical, extend);
		if (anchor === undefined) return false;
		const position = this._lookUpCaret(target);
		const entryId = position?.objectId;
		if (!position || entryId === undefined) return false;
		const logical = step(position, target.logical);
		return (
			logical !== null && this._moveCaret(target, entryId, logical, anchor)
		);
	}

	/// Move the caret one line up (step -1) or down (step 1), keeping its starting x over
	/// several moves. With extend, the selection follows the caret. True when it moved.
	moveCaretByLine(step: -1 | 1, extend: boolean = false): boolean {
		const target = this._caretTarget();
		if (!target || !target.logical) return false;
		const anchor = this._anchorForMove(target.logical, extend);
		if (anchor === undefined) return false;
		const position = this._lookUpCaret(target);
		const entryId = position?.objectId;
		if (!position || entryId === undefined) return false;

		const travel = this._travelX;
		const x =
			travel &&
			travel.logical.paragraph === target.logical.paragraph &&
			travel.logical.index === target.logical.index
				? travel.x
				: position.geometry().x;
		const logical =
			this._textOf(target.tree, entryId)?.caretOnNextLine(
				position.run,
				x,
				step,
			) ?? null;
		if (!logical || !this._moveCaret(target, entryId, logical, anchor))
			return false;
		this._travelX = { x, logical: { ...logical } };
		return true;
	}

	// The x kept over moves by line, and the caret the last of them left.
	private _travelX: { x: number; logical: cool.LogicalCaret } | null = null;

	// The entry that holds the caret for a press at (x, y) on the object hitId, or undefined
	// when the press is for the server.
	private _pressedEntry(
		target: EditTarget,
		hitId: number | undefined,
		x: number,
		y: number,
	): number | undefined {
		const entryId = this._lookUpCaret(target)?.objectId;
		if (entryId === undefined) return undefined;
		const partId = target.tree.data.partId;
		// A press off the edited object ends the edit, so it is for the engine.
		if (
			hitId === undefined ||
			(hitId !== entryId && hitId !== this._editedObjectOf(partId, entryId)) ||
			!this._mayHoldTheCaret(partId, entryId, hitId, x, y)
		)
			return undefined;
		return entryId;
	}

	// The objectKind of a table.
	private static readonly tableKind: number = 35;

	// True when a press at (x, y) on the object hitId may land in the entry's text. A table
	// edits one cell at a time, so there the press has to be inside that cell's text.
	private _mayHoldTheCaret(
		partId: cool.VectorPartGuid,
		entryId: number,
		hitId: number,
		x: number,
		y: number,
	): boolean {
		const objects = this._cache.get(partId)?.objects;
		if (objects?.get(hitId)?.objectKind !== VectorManager.tableKind)
			return true;
		const text = objects.get(entryId);
		if (
			text?.x === undefined ||
			text.y === undefined ||
			text.width === undefined ||
			text.height === undefined
		)
			return false;
		return (
			x >= text.x &&
			x <= text.x + text.width &&
			y >= text.y &&
			y <= text.y + text.height
		);
	}

	/// Put the caret where a click at (x, y), in document twips, lands in the edited text, and
	/// drop any selection. hitId is the object at the point, if any. True when the caret moved.
	placeCaretAt(x: number, y: number, hitId: number | undefined): boolean {
		const target = this._caretTarget();
		if (!target) return false;
		const entryId = this._pressedEntry(target, hitId, x, y);
		if (entryId === undefined) return false;
		const logical =
			this._textOf(target.tree, entryId)?.caretAtPoint(x, y) ?? null;
		return logical !== null && this._moveCaret(target, entryId, logical, null);
	}

	/// Select the word under a double click at (x, y), in document twips, as the engine does.
	/// hitId is the object at the point, if any. True when the word is selected.
	selectWordAt(x: number, y: number, hitId: number | undefined): boolean {
		return this._selectAt(x, y, hitId, cool.wordAt);
	}

	/// Select the paragraph under a triple click at (x, y), in document twips. hitId is the
	/// object at the point, if any. True when the paragraph is selected.
	selectParagraphAt(x: number, y: number, hitId: number | undefined): boolean {
		return this._selectAt(x, y, hitId, cool.paragraphAt);
	}

	// Select the span that extent gives around the caret where a click at (x, y) lands.
	private _selectAt(
		x: number,
		y: number,
		hitId: number | undefined,
		extent: (
			position: cool.CaretPosition,
			logical: cool.LogicalCaret,
		) => cool.CaretSpan | null,
	): boolean {
		const target = this._caretTarget();
		if (!target) return false;
		const entryId = this._pressedEntry(target, hitId, x, y);
		if (entryId === undefined) return false;
		const logical = this._textOf(target.tree, entryId)?.caretAtPoint(x, y);
		if (!logical) return false;
		const position = target.tree.caretIn(entryId, logical);
		const span = position ? extent(position, logical) : null;
		return (
			span !== null && this._moveCaret(target, entryId, span.caret, span.anchor)
		);
	}

	// The entry and the caret where a drag that selects text started.
	private _dragStart: { entryId: number; anchor: cool.LogicalCaret } | null =
		null;

	/// Start a drag that selects text from a press at (x, y), in document twips. hitId is the
	/// object at the point, if any. True when the caret moved to the press.
	startTextDrag(x: number, y: number, hitId: number | undefined): boolean {
		this._dragStart = null;
		const target = this._caretTarget();
		if (!target) return false;
		const entryId = this._pressedEntry(target, hitId, x, y);
		if (entryId === undefined) return false;
		const anchor = this._textOf(target.tree, entryId)?.caretAtPoint(x, y);
		if (!anchor || !this._moveCaret(target, entryId, anchor, null))
			return false;
		this._dragStart = { entryId, anchor };
		return true;
	}

	/// Select the text from where the drag started to the mouse at (x, y), in document twips.
	dragTextTo(x: number, y: number): void {
		const start = this._dragStart;
		const target = this._caretTarget();
		if (!start || !target) return;
		const logical = this._textOf(target.tree, start.entryId)?.caretNearPoint(
			x,
			y,
		);
		if (logical) this._moveCaret(target, start.entryId, logical, start.anchor);
	}

	endTextDrag(): void {
		this._dragStart = null;
	}

	// Put the caret at logical in the entry and draw it, with the text from the anchor to it
	// selected. The engine is told later. False when no run shows the position.
	private _moveCaret(
		target: EditTarget,
		entryId: number,
		logical: cool.LogicalCaret,
		anchor: cool.LogicalCaret | null,
	): boolean {
		const geometry = target.tree.caretIn(entryId, logical)?.geometry();
		if (!geometry) return false;
		// An anchor at the caret selects nothing.
		if (anchor && cool.compareCarets(anchor, logical) === 0) anchor = null;

		this._travelX = null;
		const partId = target.tree.data.partId;
		LocalCaret.moved({
			partId,
			entryId,
			objectId: this._editedObjectOf(partId, entryId),
			logical,
			anchor,
		});
		app.file.textCursor.logical = { ...logical };
		app.file.textCursor.anchor = anchor ? { ...anchor } : null;
		this._setCaretRectangle(geometry);
		this._showSelection(target.tree, entryId, anchor, logical);
		return true;
	}

	// Draw the selection from the anchor to the caret, or remove it without an anchor.
	private _showSelection(
		tree: cool.PredictionTree,
		entryId: number,
		anchor: cool.LogicalCaret | null,
		logical: cool.LogicalCaret,
	): void {
		const documentLayer = this._docLayer;
		if (!anchor) {
			if (TextSelections.isActive()) documentLayer.showTextSelection([]);
			return;
		}
		const rectangles =
			this._textOf(tree, entryId)?.selectionRectangles(anchor, logical) ?? [];
		documentLayer.showTextSelection(
			rectangles.map((rectangle) =>
				[rectangle.x, rectangle.y, rectangle.width, rectangle.height].map(
					Math.round,
				),
			),
		);

		// The handles go at the start and the end of the selection, in reading order.
		const [from, to] =
			cool.compareCarets(anchor, logical) <= 0
				? [anchor, logical]
				: [logical, anchor];
		const handle = (caret: cool.LogicalCaret): cool.SimpleRectangle | null => {
			const geometry = tree.caretIn(entryId, caret)?.geometry();
			return geometry
				? new cool.SimpleRectangle(
						Math.round(geometry.x),
						Math.round(geometry.top),
						0,
						Math.round(geometry.height),
					)
				: null;
		};
		TextSelections.setStartRectangle(handle(from));
		TextSelections.setEndRectangle(handle(to));
	}

	// Apply an edit at the caret, record it for replay and move the caret. build gets the code
	// points selected before and after the caret in its paragraph, or null without a selection,
	// and returns the change to apply.
	private _predictEdit(
		build: (
			selected: { before: number; after: number } | null,
		) => PredictedChange,
	): boolean {
		// The engine learns of a caret moved here before the edit, so the edit's number is
		// the one after it.
		LocalCaret.tell();
		const target = this._editTarget(true);
		if (!target) return false;
		const position = this._lookUpCaret(target);
		if (!position) return false;
		const anchor = this._selectionAnchorInParagraph();
		const selected = anchor
			? VectorManager._selectedAt(position, anchor)
			: null;
		const applied = VectorManager._applyChange(
			target.tree,
			position,
			target.logical,
			build(selected),
		);
		if (!applied) return false;
		if (target.logical) {
			const caretAfter = {
				paragraph: target.logical.paragraph,
				index: Math.max(0, target.logical.index + applied.indexShift),
				atEnd: applied.position.atEnd,
			};
			this._recordEdit(
				target.tree.data.partId,
				position,
				target.logical,
				caretAfter,
				applied.change,
			);
			target.logical.index = caretAfter.index;
		}
		if (selected) {
			// The selection is gone with the text it covered.
			app.file.textCursor.anchor = null;
			if (TextSelections.isActive()) this._docLayer.showTextSelection([]);
		}
		this._placeCaret(target, applied.caretShift);
		this.changedHere();
		return true;
	}

	// Apply a change at the position in the tree: the deletion, then the insertion at the caret
	// looked up again, as the deletion leaves the position stale. Returns how far the caret
	// index and x moved, the position after the change, and the change applied, without the
	// text when the insertion could not be predicted. Null when nothing changed.
	private static _applyChange(
		tree: cool.PredictionTree,
		position: cool.CaretPosition,
		logical: cool.LogicalCaret | null,
		change: PredictedChange,
	): {
		indexShift: number;
		caretShift: number;
		position: cool.CaretPosition;
		change: PredictedChange;
	} | null {
		const deleteBefore = change.deleteBefore ?? 0;
		const deleteAfter = change.deleteAfter ?? 0;
		let indexShift = 0;
		let caretShift = 0;
		let deleted = false;
		if (deleteBefore > 0 || deleteAfter > 0) {
			const result = position.delete(deleteBefore, deleteAfter);
			if (result.removed === 0) return null;
			deleted = true;
			indexShift = -result.removedBefore;
			caretShift = result.caretShift;
		}
		const deletion = {
			indexShift,
			caretShift,
			position,
			change: { deleteBefore, deleteAfter },
		};
		const text = change.text;
		if (text === undefined) return deleted ? deletion : null;
		if (deleted) {
			const objectId = position.objectId;
			if (objectId === undefined || !logical) return deletion;
			const after = tree.caretIn(objectId, {
				paragraph: logical.paragraph,
				index: Math.max(0, logical.index + indexShift),
			});
			if (!after) return deletion;
			position = after;
		}
		const advance = position.insert(text);
		if (!(advance > 0)) return deleted ? deletion : null;
		return {
			indexShift: indexShift + text.length,
			caretShift: caretShift + advance,
			position,
			change: deleted ? { text, deleteBefore, deleteAfter } : { text },
		};
	}

	// The code points selected before and after the caret, up to the anchor.
	private static _selectedAt(
		position: cool.CaretPosition,
		anchor: cool.LogicalCaret,
	): { before: number; after: number } {
		const text = position.run.text;
		const caret = position.run.start + position.offset;
		const count = (from: number, to: number) =>
			[...text.slice(from, to)].length;
		return {
			before: anchor.index < caret ? count(anchor.index, caret) : 0,
			after: anchor.index > caret ? count(caret, anchor.index) : 0,
		};
	}

	// Place the caret at its logical position in the edited tree, so it follows
	// text a re-wrap moved to another line, and redraw it. Without a logical
	// position, slide it along its line by the edit's width. target.caret is
	// the caret before the edit, which finds the edited object in the new tree.
	private _placeCaret(target: EditTarget, fallbackShift: number): void {
		if (target.logical) {
			const geometry = this._lookUpCaret(target)?.geometry();
			if (geometry) {
				this._setCaretRectangle(geometry);
				return;
			}
		}
		this._advanceCaret(target.caret, fallbackShift);
	}

	private _advanceCaret(caret: cool.SimpleRectangle, shift: number): void {
		if (shift === 0) return;
		caret.moveBy([shift, 0]);
		(this._docLayer as any)._onUpdateCursor();
	}

	private _setCaretRectangle(geometry: cool.CaretGeometry): void {
		app.file.textCursor.rectangle = new cool.SimpleRectangle(
			geometry.x,
			geometry.top,
			0,
			geometry.height,
		);
		(this._docLayer as any)._onUpdateCursor();
	}

	// The pending edits, oldest first.
	private _pendingEdits: PredictedTextEdit[] = [];

	private _hasPendingEdits(partId: cool.VectorPartGuid): boolean {
		return this._pendingEdits.some((edit) => edit.partId === partId);
	}

	private _dropEdits(partId: cool.VectorPartGuid | null): void {
		this._pendingEdits =
			partId === null
				? []
				: this._pendingEdits.filter((edit) => edit.partId !== partId);
	}

	// Drop the part's edits and ask for the whole part again, as the cached tree
	// still holds the predicted text. A full response replaces every object,
	// where a delta replaces only those the server changed. The cached tree
	// stays drawn until the response arrives.
	private _abandonEdits(partId: cool.VectorPartGuid): void {
		this._dropEdits(partId);
		const data = this._cache.get(partId);
		if (!data) return;
		this._inFlightParts.add(partId);
		this._sendVectorPrimitivesRequest(partId, data.mode);
	}

	// Record an edit for replay under the next input's sequence number. An edit
	// without an object id cannot be replayed and is not recorded.
	private _recordEdit(
		partId: cool.VectorPartGuid,
		position: cool.CaretPosition,
		caretBefore: cool.LogicalCaret,
		caretAfter: cool.LogicalCaret,
		change: PredictedChange,
	): void {
		const objectId = position.objectId;
		if (objectId === undefined) return;
		const sequence = app.file.inputSequence + 1;
		this._pendingEdits.push({
			sequence,
			partId,
			objectId,
			caretBefore: { ...caretBefore, atEnd: position.atEnd },
			caretAfter,
			...change,
		});
	}

	// Drop the edits the content includes and replay the rest, only on the
	// replaced objects when given. Content without a sequence drops all of the
	// part's edits. An edit that no longer fits drops them too and re-fetches
	// the part, as the edits replayed before it stay in the tree.
	private _reconcile(
		partId: cool.VectorPartGuid,
		inputSequence: number | undefined,
		replaced?: number[],
	): void {
		if (!this._hasPendingEdits(partId)) return;
		const acknowledged = inputSequence === undefined ? Infinity : inputSequence;
		this._pendingEdits = this._pendingEdits.filter(
			(edit) => edit.partId !== partId || edit.sequence > acknowledged,
		);
		const remaining = this._pendingEdits.filter(
			(edit) => edit.partId === partId,
		);
		const toReplay = replaced
			? remaining.filter((edit) => replaced.includes(edit.objectId))
			: remaining;
		if (!toReplay.length) return;
		const data = this._cache.get(partId);
		if (!data) {
			this._dropEdits(partId);
			return;
		}
		let treeChanged = false;
		for (const edit of toReplay) {
			if (!this._replayEdit(data, edit)) {
				this._abandonEdits(partId);
				treeChanged = false;
				break;
			}
			treeChanged = true;
		}
		if (treeChanged) {
			// The newest edit set the caret, replayed or not, unless the caret moved
			// here after it.
			const newest = remaining[remaining.length - 1];
			const moved = LocalCaret.ahead();
			const caret =
				moved &&
				moved.caret.partId === partId &&
				moved.sequence > newest.sequence
					? { entryId: moved.caret.entryId, logical: moved.caret.logical }
					: { entryId: newest.objectId, logical: newest.caretAfter };
			const geometry = this._treeOf(data)
				.caretIn(caret.entryId, caret.logical)
				?.geometry();
			if (geometry) this._setCaretRectangle(geometry);
			app.file.textCursor.logical = {
				paragraph: caret.logical.paragraph,
				index: caret.logical.index,
			};
		}
		this.changedHere();
	}

	// Apply a recorded edit. Returns true when the tree changed.
	private _replayEdit(
		data: cool.VectorPrimitivesData,
		edit: PredictedTextEdit,
	): boolean {
		const position = this._treeOf(data).caretIn(
			edit.objectId,
			edit.caretBefore,
		);
		if (!position) return false;
		return (
			VectorManager._applyChange(
				this._treeOf(data),
				position,
				edit.caretBefore,
				edit,
			) !== null
		);
	}

	/// True when the update is older than the newest edit, so the local caret
	/// stays. A current update that puts the caret elsewhere, as after
	/// autocorrect, drops the edits and re-fetches the part.
	keepPredictedCaret(update: cool.CaretUpdate): boolean {
		// The caret moved here, read before isAhead clears it once the engine has caught
		// up.
		const moved = LocalCaret.ahead();
		if (LocalCaret.isAhead(update)) return true;
		const partId = this._selectedPartId();
		if (partId === undefined) return false;
		const pending = this._pendingEdits.filter((edit) => edit.partId === partId);
		if (!pending.length) return false;
		const inputSequence = VectorManager._numberOf(update.inputSequence);
		if (inputSequence === undefined) return false;
		const newest = pending[pending.length - 1];
		if (inputSequence < newest.sequence) return true;
		// A caret moved here after the newest edit is where the engine's caret answers
		// from.
		const expected =
			moved && moved.caret.partId === partId && moved.sequence > newest.sequence
				? moved.caret.logical
				: newest.caretAfter;
		const paragraph = VectorManager._numberOf(update.paragraph);
		const index = VectorManager._numberOf(update.index);
		const objectId = VectorManager._numberOf(update.object);
		// A running edit's entry keeps its id across objects, so compare with
		// the edited object.
		const otherObject =
			objectId !== undefined &&
			objectId !== this._editedObjectOf(partId, newest.objectId);
		const elsewhere =
			paragraph !== undefined &&
			index !== undefined &&
			(paragraph !== expected.paragraph || index !== expected.index);
		if (otherObject || elsewhere) this._abandonEdits(partId);
		return false;
	}

	// The edited object: a running edit entry's parent, or the entry itself.
	private _editedObjectOf(
		partId: cool.VectorPartGuid,
		objectId: number,
	): number {
		const entry = this._cache.get(partId)?.objects.get(objectId);
		return entry?.kind === 'texteditoverlay' && entry.parent !== undefined
			? entry.parent
			: objectId;
	}

	// A message string as a number, or undefined when missing or empty.
	private static _numberOf(value: unknown): number | undefined {
		if (value === undefined || value === null) return undefined;
		if (typeof value === 'string' && value.trim() === '') return undefined;
		const number = Number(value);
		return isNaN(number) ? undefined : number;
	}

	/// Handle a fetched bitmap: cache it and re-render the previews
	/// that use it. Data that is missing or fails to decode marks
	/// the checksum as unavailable.
	handleVectorRenderingGraphicsResponse(
		values: cool.VectorRenderingGraphicsResponse,
	): void {
		window.app.console.log(
			'vectorrenderinggraphics pulled payload: ' +
				(values.data ? values.data.length : 0) +
				' base64 bytes, checksum=' +
				values.checksum,
		);
		if (this._bitmaps.has(values.checksum)) {
			this._bitmaps.clearInFlight(values.checksum);
			return;
		}
		if (!values.data) {
			this._markBitmapUnavailable(
				values.checksum,
				'the engine reported: ' + (values.error || 'no image data'),
			);
			return;
		}
		const image = new Image();
		// Add to the cache only on a successful decode. Clear
		// the in-flight mark on either outcome.
		image.onload = () => {
			this._bitmaps.setLoaded(values.checksum, image);
			this._redrawPartsUsing(this._bitmaps.partsFor(values.checksum));
			// A decoded bitmap changes nothing at the engine, so no object is answered.
			this._fireChanged([]);
		};
		image.onerror = () => {
			this._markBitmapUnavailable(
				values.checksum,
				'its image data failed to decode',
			);
		};
		image.src = values.data;
	}

	/// Record a checksum no usable image could be obtained for, so
	/// it is not requested again, and log the reason.
	private _markBitmapUnavailable(checksum: number, reason: string): void {
		window.app.console.warn(
			'VectorRenderingGraphics: bitmap ' +
				String(checksum) +
				' is unavailable: ' +
				reason,
		);
		this._bitmaps.setUnavailable(checksum);
	}

	private _redrawPartsUsing(parts: Set<cool.VectorPartGuid> | undefined): void {
		if (!parts) return;
		for (const partId of parts) {
			this._redrawRenderedPreviews(partId);
		}
	}

	/// Re-render every preview already shown for a part against its
	/// current cached data, leaving out the ones named.
	private _redrawRenderedPreviews(
		partId: cool.VectorPartGuid,
		except?: Set<cool.PreviewId>,
	): void {
		const data = this._cache.get(partId);
		if (!data) return;
		for (const [id, info] of this._renderedPreviews) {
			if (info.partId !== partId || except?.has(id)) continue;
			this._renderAndFire(id, info.maxWidth, info.maxHeight, data);
		}
	}

	/// Handle a fetched font: register it as a FontFace under a synthetic
	/// family name and re-render the parts that use it. The face is added
	/// to the document only after it loads, so a failed decode leaves the
	/// renderer on the family-name fallback.
	handleVectorRenderingFontResponse(
		values: cool.VectorRenderingFontResponse,
	): void {
		const fontId = values.fontId;
		if (this._fonts.has(fontId)) {
			this._fonts.clearInFlight(fontId);
			return;
		}
		if (!values.data) {
			// The engine does not hold the id.
			this._fonts.setUnavailable(fontId);
			return;
		}
		const face = new FontFace(
			'vecfont-' + fontId,
			this._decodeFontData(values.data),
		);
		face
			.load()
			.then(() => {
				(document as unknown as { fonts: Set<FontFace> }).fonts.add(face);
				this._fonts.setLoaded(fontId, face);
				this._redrawPartsUsing(this._fonts.partsFor(fontId));
				// A loaded font changes nothing at the engine, so no object is answered.
				this._fireChanged([]);
			})
			.catch(() => {
				this._fonts.setUnavailable(fontId);
			});
	}

	private _decodeFontData(base64: string): ArrayBuffer {
		const binary = window.atob(base64);
		const bytes = new Uint8Array(binary.length);
		for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
		return bytes.buffer;
	}

	/// The page list on screen has been replaced, so an index may name
	/// another page than it did. The cache and the requests in flight are
	/// keyed by page id and stay. The change listeners are notified.
	partListChanged(): void {
		this._missingParts.clear();
		// The list brings no object from the engine, so no object is answered.
		this._fireChanged([]);
	}

	/// Mark every cached part as stale. A client that was away while a part changed has
	/// nothing else that tells it so. The views and the previews in view draw again, and each
	/// stale part they draw asks for the delta since the version held, or a header alone
	/// when nothing changed. A part that is not drawn is not asked for until it is.
	revalidateCachedParts(): void {
		for (const partId of this._cache.keys()) this._staleParts.add(partId);
		this._fireChanged([]);
		app.map.fire('vectorcontentstale');
	}

	/// Forget the requests in flight. A request sent before the connection dropped gets no
	/// answer, so the next draw or revalidation asks for the page again.
	forgetRequestsInFlight(): void {
		this._inFlightParts.clear();
	}

	/// Drop cached data for a page and any in-flight state, and the bitmaps
	/// no other cached page uses. The predicted edits stay pending.
	clearCachedPart(partId: cool.VectorPartGuid): void {
		this._cache.delete(partId);
		this._staleParts.delete(partId);
		this._inFlightParts.delete(partId);
		this._bitmaps.releasePart(partId);
		this._fonts.forgetPart(partId);
		this._fireChanged();
	}

	/// Drop the cached primitive trees for every part so the views
	/// re-fetch them. Decoded bitmaps stay cached, since an image that
	/// actually changed comes back under a new checksum anyway. The
	/// predicted edits stay pending.
	clearAllParts(): void {
		this._cache.clear();
		this._staleParts.clear();
		this._inFlightParts.clear();
		this._bitmaps.forgetAllParts();
		this._fonts.forgetAllParts();
		this._fireChanged();
	}

	/// Drop the decoded bitmaps and the primitive trees that reference
	/// them, so the next paint re-fetches what it draws. The loaded
	/// fonts stay registered, since they are few and small. The predicted
	/// edits stay pending.
	reclaimGraphicsMemory(): void {
		this._bitmaps.clear();
		this._cache.clear();
		this._staleParts.clear();
		this._inFlightParts.clear();
		this._renderer.releaseScratchCanvases();
		this._renderer.releaseColorChangedImages();
		this._fireChanged();
	}

	/// Drop all cached data for all parts.
	discardAllCache(): void {
		LocalCaret.forget();
		this._cache.clear();
		this._staleParts.clear();
		this._inFlightParts.clear();
		this._pendingPreviews.clear();
		this._dropEdits(null);
		this._bitmaps.clear();
		this._renderedPreviews.clear();
		this._renderer.releaseScratchCanvases();
		this._renderer.releaseColorChangedImages();
		for (const face of this._fonts.values())
			(document as unknown as { fonts: Set<FontFace> }).fonts.delete(face);
		this._fonts.clear();
		this._fireChanged();
	}

	/// Draw the previews waiting for a part once the part and the master it
	/// names are both cached. Returns the ids of the previews drawn.
	private _drainDrawable(partId: cool.VectorPartGuid): Set<cool.PreviewId> {
		const drawn = new Set<cool.PreviewId>();
		const data = this._cache.get(partId);
		if (!data || !this._isDrawable(data)) return drawn;

		const queue = this._pendingPreviews.get(partId);
		if (!queue) return drawn;
		this._pendingPreviews.delete(partId);

		for (const pending of queue) {
			this._renderAndFire(
				pending.id,
				pending.maxWidth,
				pending.maxHeight,
				data,
			);
			drawn.add(pending.id);
		}
		return drawn;
	}

	/// Render to an offscreen canvas and trigger rendering of the thumbnail.
	private _renderAndFire(
		id: cool.PreviewId,
		maxWidth: number,
		maxHeight: number,
		data: cool.VectorPrimitivesData,
	): void {
		if (data.slideWidth <= 0 || data.slideHeight <= 0) return;

		this._renderedPreviews.set(id, {
			partId: data.partId,
			mode: data.mode,
			maxWidth: maxWidth,
			maxHeight: maxHeight,
		});

		const pxW = Math.max(1, Math.round(maxWidth * app.roundedDpiScale));
		const pxH = Math.max(1, Math.round(maxHeight * app.roundedDpiScale));

		const canvas = document.createElement('canvas');
		canvas.width = pxW;
		canvas.height = pxH;
		const context = canvas.getContext('2d');
		if (!context) return;

		// Twips to canvas pixels.
		context.scale(pxW / data.slideWidth, pxH / data.slideHeight);

		// A master page is only ever looked at while it is being edited, so
		// its thumbnail draws the prompt text of an empty placeholder. A slide
		// stands on its own, and its thumbnail leaves the prompts out.
		this.renderInto(context, data, {
			editView: data.mode === cool.VectorMode.MasterPages,
		});

		const previewImage = new Image();
		previewImage.width = maxWidth;
		previewImage.height = maxHeight;
		previewImage.src = canvas.toDataURL('image/png');

		// The image shows the page with this id, at the index the page list on
		// screen gives it now, or -1 when that list does not hold the page.
		app.map.fire('tilepreview', {
			tile: previewImage,
			id: id,
			width: maxWidth,
			height: maxHeight,
			part: data.partId,
			partIndex: this._indexOf(data.partId, data.mode),
			mode: data.mode,
			docType: this._docLayer._docType,
		});
	}
}
