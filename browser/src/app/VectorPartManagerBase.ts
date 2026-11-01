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
 * VectorPartManagerBase - Draws the parts of a document, slides or text document pages, from
 * vector primitives.
 *
 * It owns the vector data of each part. A part is fetched and decoded only once, kept up to date
 * by deltas, and the change listeners hear of each change. The typing, caret moves and
 * selections done here before the engine answers work on the cached primitives. A
 * vector-rendered document has no bitmap tile grid, so the tile methods stay the no-ops of
 * RenderManagerBase.
 */

/// A predicted edit the server has not acknowledged yet.
interface PredictedTextEdit extends cool.UnansweredEdit {
	partId: cool.VectorPartGuid;
	/// The id of the edited entry.
	objectId: number;
}

/// Where a press puts the caret: the edit target, the entry and the logical caret in it.
interface CaretPress {
	target: EditTarget;
	entryId: number;
	logical: cool.LogicalCaret;
}

/// The cached tree and caret that a predicted edit works on.
interface EditTarget {
	tree: cool.PredictionTree;
	caret: cool.SimpleRectangle;
	logical: cool.LogicalCaret | null;
	/// The text frame on a text document page that holds the caret, which the logical caret
	/// alone finds a place in. Undefined on a slide, where the caret's rectangle picks the
	/// object.
	entryId?: number;
}

abstract class VectorPartManagerBase extends VectorManagerBase {
	// Cached parsed JSON primitive tree keyed by page id. The id names the
	// page wherever it sits in its list, so a slide inserted or removed
	// before a cached page leaves that page's content valid.
	protected _cache: Map<cool.VectorPartGuid, cool.VectorPrimitivesData> =
		new Map();

	// Cached pages that may be behind the document, because the client was
	// away while they could change. A stale page is still drawn from the
	// cache, and its next draw asks for what changed since the version held.
	private _staleParts: Set<cool.VectorPartGuid> = new Set();

	// The most stale pages that ask for what changed at a time. A client back from being
	// away draws every cached page in view at once, and the engine compares every object
	// of a page it answers, so the pages ask a few at a time.
	private static readonly revalidationsAtATime = 2;

	// The pages a JSON primitive tree request is out for, by page id, each with the mode it
	// was asked for in. A stale page that asks for what changed, and a page asked for ahead of
	// its draw, are limited requests, so a few ask at a time and the rest wait in the order
	// they were asked in.
	private _pageRequests = new cool.RequestTracker<cool.VectorPartGuid, number>({
		limit: VectorPartManagerBase.revalidationsAtATime,
		timeout: VectorPartManagerBase.requestTimeout,
		expired: (requests) => this._pageRequestsExpired(requests),
	});

	// Pages the document said it does not hold. Asking again is pointless
	// until the page list is replaced.
	private _missingParts: Set<cool.VectorPartGuid> = new Set();

	// The parts the server was asked for and has not been told to forget since. The server
	// pushes the changes of each of them to this view.
	private _askedParts: Set<cool.VectorPartGuid> = new Set();

	// The parts the server was told to forget while a request for them was out. The answer to
	// that request is the last the server sends of the part, so it is left out of the cache.
	private _answersToLeaveOut: Set<cool.VectorPartGuid> = new Set();

	/// A decoded bitmap, or the one drawn in its place until it is decoded.
	protected _imageFor(checksum: number): HTMLImageElement | undefined {
		return super._imageFor(checksum) ?? this._standInFor(checksum);
	}

	/// The parts that use a loaded image or font draw again.
	protected _resourcesLoaded(partIds: Set<string> | undefined): void {
		this._redrawPartsUsing(partIds);
		super._resourcesLoaded(partIds);
	}

	/// A bitmap that decoded, or turned out unavailable, needs its stand-in no more.
	protected _imageSettled(checksum: number): void {
		this._standIns.delete(checksum);
	}

	// Decoded bitmap images keyed by their checksum.
	private _bitmaps = this._resources.images;

	/*
		Bitmaps drawn in place of one that is not decoded yet, by its checksum: the bitmap the
		object had at the same place before the engine's answer. A picture the engine renders
		anew at every size, a 3D scene for one, so shows the old picture at the new size until
		the new one is there, rather than nothing.
	*/
	private _standIns: Map<number, number> = new Map();

	// Loaded font faces keyed by font id, registered on document.fonts
	// under the family name "vecfont-<id>".
	private _fonts = this._resources.fonts;

	// The document layer, read lazily so the manager can be created before
	// the layer is registered on the map.
	protected get _docLayer(): cool.CanvasTileLayerInterface {
		return app.map._docLayer as unknown as cool.CanvasTileLayerInterface;
	}

	/// The id of the view this client is, or undefined before the document
	/// reported it or before there is a document at all.
	protected get _ownViewId(): number | undefined {
		const docLayer = app.map?._docLayer as unknown as
			cool.CanvasTileLayerInterface | undefined;
		return docLayer?._viewId;
	}

	drawsFromPrimitives(): boolean {
		return true;
	}

	/// The view draws the part that is now selected, from the cache when it holds it, at once
	/// and without an answer from the engine.
	updateOnChangePart(): void {
		this._fireChanged();
	}

	/*
		Says that the objects the client holds have changed here rather than at the engine, which
		is what a drag does when it lays what it did on them. Whoever keeps a picture of a part
		draws it again, as it does when an update arrives.
	*/
	changedHere(): void {
		// The objects changed in place, so text measured from them is out of date.
		this._objectText = null;
		super.changedHere();
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

	/// Ask for what changed on a stale page since the version held. A request already out
	/// brings the current content too, so it is not sent twice. Only a few stale pages ask
	/// at a time, and a page whose turn comes when it is no longer stale or no longer cached
	/// asks for nothing.
	protected _revalidateIfStale(stale: cool.VectorPrimitivesData): void {
		const partId = stale.partId;
		if (!this._staleParts.has(partId)) return;
		this._pageRequests.request(
			partId,
			() => {
				// The page may have changed while it waited its turn.
				const held = this._cache.get(partId);
				if (!held) return;
				this._staleParts.delete(partId);
				this._sendVectorPrimitivesRequest(partId, held.mode, held);
			},
			{
				limited: true,
				needed: () => this._cache.has(partId) && this._staleParts.has(partId),
				data: stale.mode,
			},
		);
	}

	/// Send for a page unless a request for it is already out.
	protected _request(partId: cool.VectorPartGuid, mode: number): void {
		if (this._missingParts.has(partId)) return;
		this._pageRequests.request(
			partId,
			() => this._sendVectorPrimitivesRequest(partId, mode),
			{ data: mode },
		);
	}

	/// Send for a page ahead of its draw, unless a request for it is already out. It is a
	/// limited request, so a few are out at a time and the rest wait in the order they were
	/// asked for. A page whose turn comes when it is cached, or when wanted answers false,
	/// asks for nothing.
	protected _requestAhead(
		partId: cool.VectorPartGuid,
		mode: number,
		wanted: () => boolean,
	): void {
		if (this._cache.has(partId) || this._missingParts.has(partId)) return;
		this._pageRequests.request(
			partId,
			() => this._sendVectorPrimitivesRequest(partId, mode),
			{
				limited: true,
				needed: () => !this._cache.has(partId) && wanted(),
				data: mode,
			},
		);
	}

	// The requests got no answer in time. A page that is held asks again for what changed
	// when it is next drawn. A page that is not held is asked for again now, as a preview may
	// wait for it. A page asked for ahead of its draw is asked for again by a later draw.
	private _pageRequestsExpired(
		requests: cool.RequestOut<cool.VectorPartGuid, number>[],
	): void {
		let stale = false;
		for (const request of requests) {
			for (const partId of request.keys) {
				if (this._cache.has(partId)) {
					this._staleParts.add(partId);
					stale = true;
				} else if (!request.limited && request.data !== undefined) {
					this._request(partId, request.data);
				}
			}
		}
		if (!stale) return;
		this._fireChanged([]);
		app.map.fire('vectorcontentstale');
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

	/// The entry that stands for the page among the given objects, if any.
	protected static _pageEntryOf(
		objects: Iterable<cool.SlideObject>,
	): cool.SlideObject | undefined {
		for (const object of objects) {
			if (object.kind === 'page') return object;
		}
		return undefined;
	}

	/// Tell the renderer what the automatic colors mean in this view and what
	/// the page holds behind its objects. The tree carries both unresolved,
	/// and the renderer keeps what it was last given.
	protected _setUpColors(data: cool.VectorPrimitivesData): void {
		this._renderer.setDocumentBackground(
			cool.VectorColors.documentBackground(),
		);
		const page = VectorPartManagerBase._pageEntryOf(data.objects.values());
		this._renderer.setPageBackdrop(page?.backdrop, page?.backdropTransparency);
	}

	/// Set the renderer up for a draw of the part: the bounds of its page, whether the content
	/// that only an editing view shows is drawn, whether fills and lines keep the colors the
	/// document gives them or follow a dark view, and the colors of the view and of the page.
	/// The renderer keeps each of them as it was last given.
	protected _setUpRenderer(
		data: cool.VectorPrimitivesData,
		editView: boolean,
		authoredColors: boolean,
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		this._renderer.setEditViewContentVisible(editView);
		this._renderer.setAuthoredColors(authoredColors);
		this._setUpColors(data);
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
		this._setUpRenderer(data, /*editView=*/ true, /*authoredColors=*/ false);

		for (const primitive of primitives)
			this._renderer.renderPrimitive(context, primitive);
	}

	/// Redraw what is on screen in the view's current theme. The cached
	/// trees hold the automatic colors unresolved, so a theme change
	/// costs a repaint and nothing else.
	refreshTheme(): void {
		this._fireChanged();
	}

	private _sendVectorPrimitivesRequest(
		partId: cool.VectorPartGuid,
		mode: number,
		held?: cool.VectorPrimitivesData,
	): void {
		// A client that holds the page says at which version, and in which
		// version space, so the answer is the step from there rather than
		// the whole page.
		const holds = held?.version !== undefined && held.epoch !== undefined;
		this._askedParts.add(partId);
		this._answersToLeaveOut.delete(partId);
		app.socket.sendMessage(
			cool.vectorPrimitivesRequest({
				partId: partId,
				mode: mode,
				epoch: holds ? held?.epoch : undefined,
				since: holds ? held?.version : undefined,
			}),
		);
	}

	/// Note what the page entry of a part says, such as the master page of a slide.
	protected _takePageEntry(
		_data: cool.VectorPrimitivesData,
		_objects: cool.SlideObject[],
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// A part arrived whole or changed, and the cache holds its new content.
	protected _partArrived(
		_data: cool.VectorPrimitivesData,
		_whole: boolean,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// An answer older than the cached part arrived, and the cache keeps what it holds.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected _partKept(_partId: cool.VectorPartGuid): void {}

	/// The document said that it holds no such part.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected _partMissing(_partId: cool.VectorPartGuid): void {}

	/// The page list on screen has been replaced, so an index may name
	/// another page than it did. The cache and the requests in flight are
	/// keyed by page id and stay. The change listeners are notified.
	partListChanged(): void {
		this._missingParts.clear();
		// The list brings no object from the engine, so no object is answered.
		this._fireChanged([]);
	}

	/// Handle a vector primitives response.
	handleVectorPrimitivesResponse(values: cool.VectorPrimitivesResponse): void {
		const mode =
			values.mode !== undefined ? values.mode : cool.VectorMode.Slides;
		// Every request names its page by id, and the answer names it back.
		const partId = values.partId;
		if (partId === undefined) return;
		this._pageRequests.answered(partId);
		if (this._answersToLeaveOut.delete(partId)) return;

		// A response with no objects names a part the document does not
		// hold. It happens while the page list is catching up with a mode
		// switch, or when the page was removed while the client was away. So
		// nothing stays cached for it, the previews queued for a page that
		// is not there are dropped, and the page is not asked for again until
		// the page list is replaced.
		if (values.objects === undefined) {
			this._forgetAtServer(partId);
			this._cache.delete(partId);
			this._staleParts.delete(partId);
			this._missingParts.add(partId);
			this._partMissing(partId);
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
			this._partKept(partId);
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

		const [nWidth, nHeight] = VectorPartManagerBase.pageBoundsOf(received);
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
		this._takePageEntry(data, received);
		this._cache.set(partId, data);
		this._staleParts.delete(partId);

		this._resources.add(partId, (walker) => {
			walker.walkObjects(received);
		});

		this._partArrived(data, /*whole=*/ true);
		this._reconcile(partId, values.inputSequence);
		this._fireChanged(undefined, values.inputSequence);
		this.setVisualsReady();
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
	/// objects whose content it replaced, or null when the cached tree is
	/// unchanged.
	private _applyDelta(values: cool.VectorPrimitivesResponse): number[] | null {
		const partId = values.partId;
		if (partId === undefined) return null;

		const cached = this._cache.get(partId);
		if (!cached) return null;

		// A delta is also what answers a request that said which version the
		// client holds, and only a client that holds the page says that.
		this._pageRequests.answered(partId);

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
		// The entries whose content the delta replaced. An entry without primitives only moved,
		// so it keeps what it draws, the edits typed here included.
		const replaced: number[] = [];
		for (const object of carried) {
			if (object.id === undefined) continue;
			const before = cached.objects.get(object.id);
			if (before && object.primitives === undefined)
				object.primitives = before.primitives;
			else replaced.push(object.id);
			if (before) this._noteStandIns(before, object);
			cached.objects.set(object.id, object);
		}

		// The page rectangle rides on the page entry, so a delta that carries
		// that entry is also how a resized page reaches the client. So does the
		// master the page draws under itself and the page's copies of its objects.
		const [nWidth, nHeight] = VectorPartManagerBase.pageBoundsOf(carried);
		if (nWidth > 0 && nHeight > 0) {
			cached.slideWidth = nWidth;
			cached.slideHeight = nHeight;
		}
		this._takePageEntry(cached, carried);

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

		this._resources.add(partId, (walker) => {
			walker.walkObjects(carried);
		});

		this._partArrived(cached, /*whole=*/ false);
		const answered = carried.flatMap((object) =>
			object.id === undefined ? [] : [object.id],
		);
		this._fireChanged(answered, values.inputSequence);
		return replaced;
	}

	/// The decoded bitmap standing in for that checksum, where one does.
	private _standInFor(checksum: number): HTMLImageElement | undefined {
		const standIn = this._standIns.get(checksum);
		return standIn === undefined ? undefined : this._bitmaps.get(standIn);
	}

	// The bitmaps the primitives name, in the order they are drawn.
	private static _bitmapsInOrder(
		primitives: cool.Primitive[] | undefined,
	): number[] {
		const found: number[] = [];
		const walk = (list: cool.Primitive[]) => {
			for (const primitive of list) {
				if (cool.GraphicResource.is(primitive)) found.push(primitive.checksum);
				const fillGraphic = (primitive as cool.FillGraphicPrimitive)
					.fillGraphic;
				if (fillGraphic && typeof fillGraphic.checksum === 'number')
					found.push(fillGraphic.checksum);
				const transparence = (primitive as cool.TransparencePrimitive)
					.transparence;
				if (Array.isArray(transparence)) walk(transparence);
				if (primitive.children) walk(primitive.children);
			}
		};
		walk(primitives ?? []);
		return found;
	}

	/*
		Pairs the bitmaps of an object as it was and as the engine answered, in drawing order,
		and notes the old one as standing in for each new one that is not decoded yet.
	*/
	private _noteStandIns(
		before: cool.SlideObject,
		after: cool.SlideObject,
	): void {
		const was = VectorPartManagerBase._bitmapsInOrder(before.primitives);
		const now = VectorPartManagerBase._bitmapsInOrder(after.primitives);
		const count = Math.min(was.length, now.length);
		for (let at = 0; at < count; ++at) {
			if (now[at] === was[at] || this._bitmaps.get(now[at])) continue;
			// the old one may itself still be stood in for
			const standIn = this._bitmaps.get(was[at])
				? was[at]
				: this._standIns.get(was[at]);
			if (standIn !== undefined) this._standIns.set(now[at], standIn);
		}
	}

	// The measurer over that width, created on first use.
	private _textMeasurer: cool.TextMeasurer | null = null;

	private _measurer(): cool.TextMeasurer {
		if (!this._textMeasurer)
			this._textMeasurer = new cool.TextMeasurer(this._textMeasure());
		return this._textMeasurer;
	}

	protected _treeOf(data: cool.VectorPrimitivesData): cool.PredictionTree {
		return new cool.PredictionTree(
			data,
			this._measurer(),
			this._paragraphsKeepTheirLines(),
		);
	}

	/// True when the text after each paragraph lies where the server placed it, so an edit here
	/// keeps a paragraph to the lines it has. The text of a slide grows inside its object.
	protected _paragraphsKeepTheirLines(): boolean {
		return false;
	}

	// The measured text of the last entry pointed at, reused while the tree still holds it and
	// nothing changed it here.
	private _objectText: { entryId: number; text: cool.ObjectText } | null = null;

	protected _textOf(
		tree: cool.PredictionTree,
		entryId: number,
	): cool.ObjectText | null {
		const kept = this._objectText;
		if (
			kept &&
			kept.entryId === entryId &&
			kept.text.isCollectedFrom(tree, entryId)
		)
			return kept.text;
		const text = cool.ObjectText.collectFrom(tree, entryId);
		this._objectText = text ? { entryId, text } : null;
		return text;
	}

	/// The id of the page in the edit view, or undefined while no page is named as the one in
	/// the edit view.
	protected abstract _selectedPartId(): cool.VectorPartGuid | undefined;

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
	protected _caretTarget(): EditTarget | null {
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

	/// Where the twips of an entry start in document twips, or null when the entry is not known.
	/// The twips of a slide are document twips already.
	protected _entryOrigin(
		_data: cool.VectorPrimitivesData,
		_entryId: number | undefined,
	): { x: number; y: number } | null {
		return { x: 0, y: 0 };
	}

	// A caret in the twips of the entry, moved to document twips, or null.
	private _inDocument(
		data: cool.VectorPrimitivesData,
		entryId: number | undefined,
		geometry: cool.CaretGeometry,
	): cool.CaretGeometry | null {
		const origin = this._entryOrigin(data, entryId);
		if (!origin) return null;
		return {
			x: geometry.x + origin.x,
			top: geometry.top + origin.y,
			height: geometry.height,
		};
	}

	// The caret position in the edit target, or null.
	private _lookUpCaret(target: EditTarget): cool.CaretPosition | null {
		if (target.entryId !== undefined)
			return target.logical
				? target.tree.caretIn(target.entryId, target.logical)
				: null;
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
		return this._predictEdit((selected) => cool.typedChange(text, selected));
	}

	/// Delete before characters left of the caret and after characters right of it in the
	/// cached tree. A selection in the caret's paragraph is the first one deleted, as in the
	/// engine. True when the tree changed.
	predictTextDelete(before: number, after: number): boolean {
		if (before <= 0 && after <= 0) return false;
		return this._predictEdit((selected) =>
			cool.deletedChange(before, after, selected),
		);
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
		const start = this._moveStart(extend);
		if (!start) return false;
		const logical = step(start.position, start.logical);
		return (
			logical !== null &&
			this._moveCaret(start.target, start.entryId, logical, start.anchor)
		);
	}

	// Where a caret key moves the caret from: the edited part, the caret, the anchor the move
	// selects from and the caret's entry. Null when the move is for the server.
	private _moveStart(extend: boolean): {
		target: EditTarget;
		logical: cool.LogicalCaret;
		anchor: cool.LogicalCaret | null;
		position: cool.CaretPosition;
		entryId: number;
	} | null {
		const target = this._caretTarget();
		if (!target || !target.logical) return null;
		const anchor = this._anchorForMove(target.logical, extend);
		if (anchor === undefined) return null;
		const position = this._lookUpCaret(target);
		const entryId = position?.objectId;
		if (!position || entryId === undefined) return null;
		return { target, logical: target.logical, anchor, position, entryId };
	}

	/// Move the caret one line up (step -1) or down (step 1), keeping its starting x over
	/// several moves. With extend, the selection follows the caret. True when it moved.
	moveCaretByLine(step: -1 | 1, extend: boolean = false): boolean {
		const start = this._moveStart(extend);
		if (!start) return false;
		const { target, entryId, position } = start;

		const travel = this._travelX;
		const x =
			travel &&
			travel.logical.paragraph === start.logical.paragraph &&
			travel.logical.index === start.logical.index
				? travel.x
				: position.geometry().x;
		const logical =
			this._textOf(target.tree, entryId)?.caretOnNextLine(
				position.run,
				x,
				step,
			) ?? null;
		if (!logical || !this._moveCaret(target, entryId, logical, start.anchor))
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

	// The edited part, the entry and the caret where a press at (x, y) on the object hitId
	// lands, or null when the press is for the server.
	protected _pressAt(
		x: number,
		y: number,
		hitId: number | undefined,
	): CaretPress | null {
		const target = this._caretTarget();
		if (!target) return null;
		const entryId = this._pressedEntry(target, hitId, x, y);
		if (entryId === undefined) return null;
		const origin = this._entryOrigin(target.tree.data, entryId);
		if (!origin) return null;
		const logical = this._textOf(target.tree, entryId)?.caretAtPoint(
			x - origin.x,
			y - origin.y,
		);
		return logical ? { target, entryId, logical } : null;
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
		if (objects?.get(hitId)?.objectKind !== VectorPartManagerBase.tableKind)
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
		const press = this._pressAt(x, y, hitId);
		return (
			press !== null &&
			this._moveCaret(press.target, press.entryId, press.logical, null)
		);
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
		const press = this._pressAt(x, y, hitId);
		if (!press) return false;
		const { target, entryId, logical } = press;
		const position = target.tree.caretIn(entryId, logical);
		const span = position ? extent(position, logical) : null;
		return (
			span !== null && this._moveCaret(target, entryId, span.caret, span.anchor)
		);
	}

	// The entry and the caret where a drag that selects text started.
	protected _dragStart: { entryId: number; anchor: cool.LogicalCaret } | null =
		null;

	/// Start a drag that selects text from a press at (x, y), in document twips. hitId is the
	/// object at the point, if any. True when the caret moved to the press.
	startTextDrag(x: number, y: number, hitId: number | undefined): boolean {
		this._dragStart = null;
		const press = this._pressAt(x, y, hitId);
		if (
			!press ||
			!this._moveCaret(press.target, press.entryId, press.logical, null)
		)
			return false;
		this._dragStart = { entryId: press.entryId, anchor: press.logical };
		return true;
	}

	/// Select the text from where the drag started to the mouse at (x, y), in document twips.
	dragTextTo(x: number, y: number): void {
		const start = this._dragStart;
		const target = this._caretTarget();
		const origin = target
			? this._entryOrigin(target.tree.data, start?.entryId)
			: null;
		if (!start || !target || !origin) return;
		const logical = this._textOf(target.tree, start.entryId)?.caretNearPoint(
			x - origin.x,
			y - origin.y,
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
		const local = target.tree.caretIn(entryId, logical)?.geometry();
		const geometry = local
			? this._inDocument(target.tree.data, entryId, local)
			: null;
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
		this._caretMovedTo(entryId);
		this._setCaretRectangle(geometry);
		this._showSelection(target.tree, entryId, anchor, logical);
		return true;
	}

	/// The caret moved here into the entry. On a slide the edited object stays the same.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected _caretMovedTo(_entryId: number): void {}

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
		const origin = this._entryOrigin(tree.data, entryId);
		if (!origin) return;
		const rectangles =
			this._textOf(tree, entryId)?.selectionRectangles(anchor, logical) ?? [];
		documentLayer.showTextSelection(
			rectangles.map((rectangle) =>
				[
					rectangle.x + origin.x,
					rectangle.y + origin.y,
					rectangle.width,
					rectangle.height,
				].map(Math.round),
			),
		);

		// The handles go at the start and the end of the selection, in reading order.
		const [from, to] =
			cool.compareCarets(anchor, logical) <= 0
				? [anchor, logical]
				: [logical, anchor];
		const handle = (caret: cool.LogicalCaret): cool.SimpleRectangle | null => {
			const local = tree.caretIn(entryId, caret)?.geometry();
			const geometry = local
				? this._inDocument(tree.data, entryId, local)
				: null;
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
		build: (selected: cool.SelectedAround | null) => cool.PredictedChange,
	): boolean {
		// The engine learns of a caret moved here before the edit, so the edit's number is
		// the one after it.
		LocalCaret.tell();
		const target = this._editTarget(true);
		if (!target) return false;
		const position = this._lookUpCaret(target);
		if (!position) return false;
		const anchor = this._selectionAnchorInParagraph();
		const selected = anchor ? cool.selectedAround(position, anchor) : null;
		const applied = cool.applyPredictedChange(
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

	// Place the caret at its logical position in the edited tree, so it follows
	// text a re-wrap moved to another line, and redraw it. Without a logical
	// position, slide it along its line by the edit's width. target.caret is
	// the caret before the edit, which finds the edited object in the new tree.
	protected _placeCaret(target: EditTarget, fallbackShift: number): void {
		if (target.logical) {
			const local = this._lookUpCaret(target)?.geometry();
			const geometry = local
				? this._inDocument(target.tree.data, target.entryId, local)
				: null;
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

	protected _setCaretRectangle(geometry: cool.CaretGeometry): void {
		cool.showCaret(geometry);
	}

	// The edits typed here that the engine has not answered, of every part.
	protected _typedEdits = new cool.UnansweredEdits<PredictedTextEdit>();

	// The edits of the part.
	private static _ofPart(
		partId: cool.VectorPartGuid,
	): (edit: PredictedTextEdit) => boolean {
		return (edit) => edit.partId === partId;
	}

	// Drop the part's edits and ask for the whole part again, as the cached tree
	// still holds the predicted text. A full response replaces every object,
	// where a delta replaces only those the server changed. The cached tree
	// stays drawn until the response arrives.
	private _abandonEdits(partId: cool.VectorPartGuid): void {
		this._typedEdits.forget(VectorPartManagerBase._ofPart(partId));
		const data = this._cache.get(partId);
		if (!data) return;
		// A request out for the page is answered with a delta, so the full one goes out
		// beside it.
		this._pageRequests.forget(partId);
		this._pageRequests.request(
			partId,
			() => this._sendVectorPrimitivesRequest(partId, data.mode),
			{ data: data.mode },
		);
	}

	// Record an edit for replay until its message is answered. An edit without an object id
	// is not recorded.
	private _recordEdit(
		partId: cool.VectorPartGuid,
		position: cool.CaretPosition,
		caretBefore: cool.LogicalCaret,
		caretAfter: cool.LogicalCaret,
		change: cool.PredictedChange,
	): void {
		const objectId = position.objectId;
		if (objectId === undefined) return;
		this._typedEdits.record({
			ticket: cool.InputSequence.ticketForTheNextMessage(),
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
		const ofPart = VectorPartManagerBase._ofPart(partId);
		if (!this._typedEdits.has(ofPart)) return;
		if (
			replaced &&
			inputSequence !== undefined &&
			this._answerCarriesEditedEntries()
		) {
			// An edit this delta answers whose entry came without new content was not applied,
			// so the part is asked for whole, which takes the predicted text out of the tree.
			const sequence: number = inputSequence;
			const replacedIds: number[] = replaced;
			const notApplied = this._typedEdits.has(
				(edit) =>
					ofPart(edit) &&
					edit.ticket.appliedBy(sequence) &&
					!replacedIds.includes(edit.objectId),
			);
			if (notApplied) {
				this._abandonEdits(partId);
				return;
			}
		}
		const remaining = this._typedEdits.answered(inputSequence, ofPart);
		const toReplay = replaced
			? remaining.filter((edit) => replaced.includes(edit.objectId))
			: remaining;
		if (!toReplay.length) return;
		const data = this._cache.get(partId);
		if (!data) {
			this._typedEdits.forget(ofPart);
			return;
		}
		const treeChanged = cool.UnansweredEdits.applyAgain(toReplay, (edit) =>
			this._replayEdit(data, edit),
		);
		if (!treeChanged) this._abandonEdits(partId);
		if (treeChanged) {
			// The newest edit set the caret, replayed or not, unless the caret moved
			// here after it.
			const newest = remaining[remaining.length - 1];
			const moved = LocalCaret.ahead();
			const caret =
				moved &&
				moved.caret.partId === partId &&
				moved.sequence > cool.UnansweredEdits.sentAs(newest)
					? { entryId: moved.caret.entryId, logical: moved.caret.logical }
					: { entryId: newest.objectId, logical: newest.caretAfter };
			const local = this._treeOf(data)
				.caretIn(caret.entryId, caret.logical)
				?.geometry();
			const geometry = local
				? this._inDocument(data, caret.entryId, local)
				: null;
			if (geometry) this._setCaretRectangle(geometry);
			app.file.textCursor.logical = {
				paragraph: caret.logical.paragraph,
				index: caret.logical.index,
			};
		}
		this.changedHere();
	}

	/// True when the delta that answers an applied edit always carries the new content of the
	/// entry the edit went into. An answered edit whose entry came without new content was then
	/// not applied by the engine.
	protected _answerCarriesEditedEntries(): boolean {
		return false;
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
			cool.applyPredictedChange(
				this._treeOf(data),
				position,
				edit.caretBefore,
				edit,
			) !== null
		);
	}

	/// The part that the predicted edits lie on: the part of a slide that is selected.
	protected _editedPartId(): cool.VectorPartGuid | undefined {
		return this._selectedPartId();
	}

	/// True when the update is older than the newest edit, so the local caret
	/// stays. A current update that puts the caret elsewhere, as after
	/// autocorrect, drops the edits and re-fetches the part.
	keepPredictedCaret(update: cool.CaretUpdate): boolean {
		// The caret moved here, read before isAhead clears it once the engine has caught
		// up.
		const moved = LocalCaret.ahead();
		if (LocalCaret.isAhead(update)) return true;
		const partId = this._editedPartId();
		if (partId === undefined) return false;
		const verdict = this._typedEdits.verdictOn(
			update,
			// A running edit's entry keeps its id across objects, so compare with the
			// edited object.
			(objectId, newest) =>
				objectId === this._editedObjectOf(partId, newest.objectId),
			VectorPartManagerBase._ofPart(partId),
			// A caret moved here after the newest edit is where the engine's caret
			// answers from.
			(newest) =>
				moved &&
				moved.caret.partId === partId &&
				moved.sequence > cool.UnansweredEdits.sentAs(newest)
					? moved.caret.logical
					: newest.caretAfter,
		);
		if (verdict === cool.CaretUpdateVerdict.Elsewhere)
			this._abandonEdits(partId);
		return verdict === cool.CaretUpdateVerdict.Older;
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

	/// Draw again what shows the parts, once a picture or a font that they use has loaded. The
	/// base class does nothing, because the change listeners report the change.
	protected _redrawPartsUsing(
		_parts: Set<cool.VectorPartGuid> | undefined,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// Mark every cached part as stale. A client that was away while a part changed has
	/// nothing else that tells it so. The views and the previews in view draw again, and each
	/// stale part they draw asks for the delta since the version held, or a header alone
	/// when nothing changed. A part that is not drawn is not asked for until it is.
	revalidateCachedParts(): void {
		for (const partId of this._cache.keys()) this._staleParts.add(partId);
		this._fireChanged([]);
		app.map.fire('vectorcontentstale');
	}

	/// Drop cached data for a page and any in-flight state, and the bitmaps
	/// no other cached page uses. The predicted edits stay pending.
	clearCachedPart(partId: cool.VectorPartGuid): void {
		this._dropCachedPart(partId);
		this._fireChanged();
	}

	/// Drop the cached data of a part and any request for it, and tell the server to forget
	/// the part.
	protected _dropCachedPart(partId: cool.VectorPartGuid): void {
		this._forgetAtServer(partId);
		this._cache.delete(partId);
		this._staleParts.delete(partId);
		this._pageRequests.forget(partId);
		this._resources.release(partId);
	}

	/// Drop the cached data of every part and every request, and tell the server to forget
	/// each part.
	protected _dropAllCachedParts(): void {
		for (const partId of Array.from(this._askedParts))
			this._forgetAtServer(partId);
		this._cache.clear();
		this._staleParts.clear();
		this._pageRequests.forgetAll();
	}

	// Tell the server that a part it was asked for is held no more, so it stops pushing the
	// changes of the part. Messages arrive in the order they are sent, so a request that
	// follows makes the server push them again.
	private _forgetAtServer(partId: cool.VectorPartGuid): void {
		if (!this._askedParts.has(partId)) return;
		const requestOut = this._pageRequests.isOut(partId);
		// A part that is not cached has its answer on the way. The answer is taken into the
		// cache when it comes, so the part stays one the server pushes the changes of.
		if (requestOut && !this._cache.has(partId)) return;
		if (requestOut) this._answersToLeaveOut.add(partId);
		this._askedParts.delete(partId);
		app.socket.sendMessage(
			'commandvalues command=.uno:VectorForgetPart?partid=' + partId,
		);
	}

	/// Drop the cached primitive trees for every part so the views
	/// re-fetch them. Decoded bitmaps stay cached, since an image that
	/// actually changed comes back under a new checksum anyway. The
	/// predicted edits stay pending.
	clearAllParts(): void {
		this._dropAllCachedParts();
		this._resources.forgetOwners();
		this._fireChanged();
	}

	/// Drop the decoded bitmaps and the primitive trees that reference
	/// them, so the next paint re-fetches what it draws. The loaded
	/// fonts stay registered, since they are few and small. The predicted
	/// edits stay pending.
	reclaimGraphicsMemory(): void {
		super.reclaimGraphicsMemory();
		this._resources.forgetOwners();
		this._dropAllCachedParts();
		this._fireChanged();
	}

	/// Drop all cached data for all parts.
	discardAllCache(): void {
		LocalCaret.forget();
		this._dropAllCachedParts();
		this._typedEdits.forget();
		this._resources.discard();
		this._renderer.releaseScratchCanvases();
		this._renderer.releaseColorChangedImages();
		this._fireChanged();
	}
}
