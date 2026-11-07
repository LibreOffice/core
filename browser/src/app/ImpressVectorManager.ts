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
 * ImpressVectorManager - Draws the slides of a presentation or the pages of a drawing from vector
 * primitives. The client also selects, moves and edits their objects itself.
 *
 * Next to what VectorPartManagerBase holds, it maps slide numbers to parts, draws a slide over its
 * master page and in the states an animation gives its objects, draws the ghosts and aids of
 * objects being edited, keeps the hidden layers of the view, and draws the previews of slides.
 */

class ImpressVectorManager extends VectorPartManagerBase {
	kind(): RenderManagerKind {
		return 'slides';
	}

	isVectorRendering(): boolean {
		return true;
	}

	// Previews waiting for a JSON primitive tree response, keyed by page id.
	private _pendingPreviews: Map<cool.VectorPartGuid, cool.PendingPreview[]> =
		new Map();

	// Size and part of every preview that has been rendered. A redraw
	// triggered by a decoded bitmap reuses these so the re-fired
	// preview keeps the size and part it was rendered at.
	private _renderedPreviews: Map<cool.PreviewId, cool.RenderedPreview> =
		new Map();

	// Ids of the layers this view does not show. Objects on them are
	// kept in the cache and skipped when drawing.
	private _hiddenLayers: Set<number> = new Set();

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

	/// The color the text run is drawn in, with no animation color in its place.
	textColorOf(
		data: cool.VectorPrimitivesData,
		primitive:
			cool.TextSimplePortionPrimitive | cool.TextDecoratedPortionPrimitive,
	): string {
		this._setUpColors(data);
		return this._renderer.ownTextColor(primitive);
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
		this._setUpRenderer(
			data,
			options?.editView === true,
			options?.authoredColors === true,
		);
		const textEdits = this._textEditEntriesToDraw(data);
		// an object active in place covers its upright window, not its own box
		const window = options?.activeInPlace?.area;
		const edited =
			options?.edited !== undefined
				? ImpressVectorManager._editedPart(
						data,
						options.edited.objectId,
						options.activeInPlace?.objectId === options.edited.objectId &&
							window
							? [
									window[0],
									window[1],
									window[0] + window[2],
									window[1] + window[3],
								]
							: undefined,
					)
				: undefined;
		const range = options?.paintRange;
		let index = -1;
		for (const id of data.order) {
			++index;
			if (range && (index < range.start || index >= range.end)) continue;
			const obj = data.objects.get(id);
			if (!obj) continue;
			if (obj.layer !== undefined && this._hiddenLayers.has(obj.layer))
				continue;
			if (obj.kind === 'texteditoverlay' && !textEdits.has(id)) continue;
			if (this._drawnElsewhere.has(id)) continue;
			// The slide's copy of a master placeholder is drawn with the master.
			if (obj.masterContent) continue;
			if (
				options?.withoutFormControls &&
				obj.objectKind === ImpressVectorManager.formControlKind
			)
				continue;
			const state = ImpressVectorManager.stateOf(data, id, options?.states);
			if (state?.hidden) continue;
			// A page entry that draws only the upper part of its master leaves out its own
			// background, which the part below has drawn.
			const isUpperMaster =
				obj.kind === 'page' && (options?.masterRange?.start ?? 0) > 0;
			if (obj.primitives && !isUpperMaster) {
				const outside =
					edited !== undefined &&
					obj.kind === undefined &&
					!ImpressVectorManager._isInside(data, id, edited.objectId);
				const inFront =
					outside &&
					index > edited.lastAt &&
					ImpressVectorManager._overlaps(obj, edited.box);
				const pale = outside && options?.edited?.paleOutside === true;
				if (pale || inFront) this._renderGhosted(context, obj, inFront, pale);
				else if (state) this._renderInState(context, obj, state);
				else
					for (const primitive of obj.primitives) {
						this._renderer.renderPrimitive(context, primitive);
					}
			}
			// The master lies between the page's own background and its objects.
			if (obj.kind === 'page') this._renderMaster(context, data, options);
			options?.drawOver?.get(id)?.(context);
		}
	}

	/// Draw one paragraph of an object's text alone, in the colors and text look given. The
	/// context transform maps the part's twips to the target pixels.
	renderObjectParagraph(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		objectId: number,
		paragraph: number,
		options?: cool.ParagraphRenderOptions,
	): void {
		const object = data.objects.get(objectId);
		if (!object) return;
		this._setUpRenderer(
			data,
			/*editView=*/ false,
			options?.authoredColors === true,
		);
		this._renderUnder(
			context,
			object,
			{
				colorOverride: options?.colorOverride,
				textOverride: options?.textOverride,
			},
			{ only: paragraph },
		);
	}

	// the objectKind of a form control
	private static readonly formControlKind: number = 32;

	// The state of the object, or else of the nearest group above it, since an effect on a
	// group moves every member. Undefined when neither has one.
	static stateOf(
		data: cool.VectorPrimitivesData,
		objectId: number,
		states: Map<number, cool.ObjectRenderState> | undefined,
	): cool.ObjectRenderState | undefined {
		if (!states) return undefined;
		for (
			let at: number | undefined = objectId, guard = 0;
			at !== undefined && guard < 100;
			at = data.objects.get(at)?.parent, ++guard
		) {
			const state = states.get(at);
			if (state) return state;
		}
		return undefined;
	}

	// Draw the object in its state, without the paragraphs that have a state of their own.
	// Those follow right after, each in its own state, so they keep their place in the paint
	// order.
	private _renderInState(
		context: CanvasRenderingContext2D,
		object: cool.SlideObject,
		state: cool.ObjectRenderState,
	): void {
		const paragraphs = state.paragraphs
			? [...state.paragraphs.entries()].sort(
					([first], [second]) => first - second,
				)
			: [];
		this._renderUnder(
			context,
			object,
			state,
			paragraphs.length ? { skip: paragraphs.map(([index]) => index) } : null,
		);
		for (const [index, paragraphState] of paragraphs)
			this._renderUnder(context, object, paragraphState, { only: index });
	}

	private _renderUnder(
		context: CanvasRenderingContext2D,
		object: cool.SlideObject,
		state: cool.DrawState,
		paragraphFilter: cool.ParagraphFilter | null,
	): void {
		if (state.hidden || !object.primitives) return;
		context.save();
		if (state.alpha !== undefined) context.globalAlpha = state.alpha;
		const matrix = cool.Matrix2D.fromArray(state.transform);
		if (matrix) matrix.applyTo(context);
		if (state.clip) ImpressVectorManager.clipTo(context, state.clip);
		this._renderer.setColorOverride(state.colorOverride ?? null);
		this._renderer.setTextOverride(state.textOverride ?? null);
		this._renderer.setParagraphFilter(paragraphFilter);
		for (const primitive of object.primitives) {
			this._renderer.renderPrimitive(context, primitive);
		}
		this._renderer.setParagraphFilter(null);
		this._renderer.setTextOverride(null);
		this._renderer.setColorOverride(null);
		context.restore();
	}

	/*
		Clip what is drawn next to the clip's area. The clip's transform applies only while
		its paths are built.
	*/
	static clipTo(context: CanvasRenderingContext2D, clip: cool.DrawClip): void {
		const saved = context.getTransform();
		cool.Matrix2D.fromArray(clip.transform)?.applyTo(context);

		const [x, y, width, height] = clip.frame;
		const frame = new Path2D();
		frame.rect(x, y, width, height);
		context.clip(frame);

		// Frame and pieces in one even-odd path: the frame minus the pieces, which do not overlap.
		const area = new Path2D();
		if (clip.inverted) area.rect(x, y, width, height);
		for (const piece of clip.pieces)
			ImpressVectorManager._addPiece(area, piece);
		context.clip(area, clip.inverted ? 'evenodd' : 'nonzero');

		context.setTransform(saved);
	}

	// Each piece is a figure of its own.
	private static _addPiece(path: Path2D, piece: cool.AreaPiece): void {
		if ('rectangle' in piece) {
			const [x, y, width, height] = piece.rectangle;
			path.rect(x, y, width, height);
		} else if ('polygon' in piece) {
			const corners = piece.polygon;
			if (corners.length < 6) return;
			path.moveTo(corners[0], corners[1]);
			for (let at = 2; at + 1 < corners.length; at += 2)
				path.lineTo(corners[at], corners[at + 1]);
			path.closePath();
		} else if ('ellipse' in piece) {
			const [centerX, centerY, radiusX, radiusY] = piece.ellipse;
			path.moveTo(centerX + radiusX, centerY);
			path.ellipse(centerX, centerY, radiusX, radiusY, 0, 0, 2 * Math.PI);
			path.closePath();
		} else {
			const [centerX, centerY, radius, start, end] = piece.slice;
			path.moveTo(centerX, centerY);
			path.arc(centerX, centerY, radius, start, end);
			path.closePath();
		}
	}

	// whether the object is the one edited or lies in it, a group, at any depth
	private static _isInside(
		data: cool.VectorPrimitivesData,
		objectId: number,
		editedId: number,
	): boolean {
		for (
			let at: number | undefined = objectId, guard = 0;
			at !== undefined && guard < 100;
			at = data.objects.get(at)?.parent, ++guard
		)
			if (at === editedId) return true;
		return false;
	}

	/*
		What is being edited, a group or one object, as the ghosting needs it: the place of its
		last object in the paint order, and the box round its objects, or the box given. An object
		after that place and over that box lies in front of what is being edited.
	*/
	private static _editedPart(
		data: cool.VectorPrimitivesData,
		objectId: number,
		given: number[] | undefined,
	): { objectId: number; lastAt: number; box: number[] | undefined } {
		let lastAt = -1;
		let box: number[] | undefined = given;
		data.order.forEach((id: number, at: number) => {
			const object = data.objects.get(id);
			if (!object || object.kind !== undefined) return;
			if (!ImpressVectorManager._isInside(data, id, objectId)) return;

			lastAt = at;
			const own = ImpressVectorManager._boxOf(object);
			if (!own || given) return;
			box = box
				? [
						Math.min(box[0], own[0]),
						Math.min(box[1], own[1]),
						Math.max(box[2], own[2]),
						Math.max(box[3], own[3]),
					]
				: own;
		});
		return { objectId, lastAt, box };
	}

	// the box an object paints in, as [minX, minY, maxX, maxY] in twips, where it has one
	private static _boxOf(object: cool.SlideObject): number[] | undefined {
		return object.x !== undefined &&
			object.y !== undefined &&
			object.width !== undefined &&
			object.height !== undefined
			? [object.x, object.y, object.x + object.width, object.y + object.height]
			: undefined;
	}

	private static _overlaps(
		object: cool.SlideObject,
		box: number[] | undefined,
	): boolean {
		const own = ImpressVectorManager._boxOf(object);
		return (
			!!own &&
			!!box &&
			own[0] < box[2] &&
			box[0] < own[2] &&
			own[1] < box[3] &&
			box[1] < own[3]
		);
	}

	/*
		How what lies outside what is being edited is drawn, from the settings: the colour it is
		mixed with (vectorGhostColor, the document background of the view), how far towards it
		from 0 to 1 (vectorGhostAmount, 0.5 as the office has it), and how transparent an object
		in front of it is from 0 to 1 (vectorGhostTransparency, 0.8), so that what is edited
		shows through.
	*/
	private static _ghosting(): {
		color: string;
		amount: number;
		transparency: number;
	} {
		const share = (name: string, byDefault: number) => {
			const value = window.prefs?.getNumber(name, byDefault) ?? byDefault;
			return value >= 0 && value <= 1 ? value : byDefault;
		};
		const background = cool.VectorColors.documentBackground();
		const color = window.prefs?.get('vectorGhostColor', background);
		return {
			color:
				typeof color === 'string' && /^#[0-9a-fA-F]{6}$/.test(color)
					? color
					: background,
			amount: share('vectorGhostAmount', 0.5),
			transparency: share('vectorGhostTransparency', 0.8),
		};
	}

	// an object outside what is being edited, mixed towards the ghost colour where pale, as the
	// office draws it, and made transparent where it lies in front
	private _renderGhosted(
		context: CanvasRenderingContext2D,
		object: cool.SlideObject,
		inFront: boolean,
		pale: boolean,
	): void {
		const ghosting = ImpressVectorManager._ghosting();
		// the box the object paints in bounds the change, without one it covers the view
		const mixed = {
			type: cool.ModifiedColorPrimitive.type,
			modifier: 'interpolate',
			color: ghosting.color,
			// the value is the share of the original colour
			value: 1 - ghosting.amount,
			bounds: ImpressVectorManager._boxOf(object),
			children: object.primitives,
		} as cool.ModifiedColorPrimitive;
		const drawn: cool.Primitive[] = pale ? [mixed] : (object.primitives ?? []);

		if (!inFront) {
			for (const primitive of drawn)
				this._renderer.renderPrimitive(context, primitive);
			return;
		}

		this._renderer.renderPrimitive(context, {
			type: cool.UnifiedTransparencePrimitive.type,
			transparence: ghosting.transparency,
			children: drawn,
		} as cool.UnifiedTransparencePrimitive);
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
		this._setUpRenderer(data, /*editView=*/ true, /*authoredColors=*/ false);
		this._renderer.renderObjectsMoved(context, data.objects, objectIds, matrix);
	}

	/// Draw the master that a page names under it, in the master's order. A
	/// shared object comes from the master, a per-slide one from the page's
	/// own copy, if it has one. An object on a layer the page hides is left
	/// out. The master's page entry is skipped, since the page draws its own
	/// background. A text edit on a master object is drawn from its edit
	/// entry, since the object hides its text during the edit. Only the
	/// positions of the master range of the options are drawn, and what the
	/// options give to draw over an object is drawn right after it.
	private _renderMaster(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		options?: cool.VectorRenderOptions,
	): void {
		if (data.masterPartId === undefined) return;
		const master = this._cache.get(data.masterPartId);
		if (!master) return;

		const textEdits = this._textEditEntriesToDraw(master);
		const start = options?.masterRange?.start ?? 0;
		const end = Math.min(
			options?.masterRange?.end ?? master.order.length,
			master.order.length,
		);
		for (let position = start; position < end; ++position) {
			const id = master.order[position];
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
			for (const primitive of drawn?.primitives ?? []) {
				this._renderer.renderPrimitive(context, primitive);
			}
			options?.drawOver?.get(id)?.(context);
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
		this._renderer.setAuthoredColors(false);
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

	/// A master arrived or changed, so every page drawing under it can be
	/// drawn now, and every preview showing such a page is drawn again. Which
	/// pages those are is read off the cache. A preview that was waiting for
	/// the master is drawn by the drain alone.
	private _onMasterArrived(masterPartId: cool.VectorPartGuid): void {
		for (const [partId, data] of this._cache) {
			if (data.masterPartId !== masterPartId) continue;
			const drained = this._drainDrawable(partId);
			this._redrawRenderedPreviews(partId, drained);
		}
	}

	// The parts whose previews draw again in the layouting task planned for them, if one is.
	private _previewPartsToRedraw = new Set<cool.VectorPartGuid>();
	private _previewRedrawPlanned = false;

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

		// The canvas is the preview's CSS size times the exact device pixel ratio, so at a ratio
		// such as 1.25 the thumbnail maps one to one onto the screen pixels.
		const pxW = Math.max(1, Math.round(maxWidth * app.dpiScale));
		const pxH = Math.max(1, Math.round(maxHeight * app.dpiScale));

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

	/// Redraw what is on screen in the view's current theme, the previews too.
	refreshTheme(): void {
		for (const [id, info] of this._renderedPreviews) {
			const data = this._cache.get(info.partId);
			if (data) this._renderAndFire(id, info.maxWidth, info.maxHeight, data);
		}
		super.refreshTheme();
	}

	/// Drop all cached data for all parts, and the previews drawn from them.
	discardAllCache(): void {
		this._pendingPreviews.clear();
		this._renderedPreviews.clear();
		super.discardAllCache();
	}

	/// The master page of a slide is named by its page entry.
	protected _takePageEntry(
		data: cool.VectorPrimitivesData,
		objects: cool.SlideObject[],
	): void {
		const page = VectorPartManagerBase._pageEntryOf(objects);
		if (page) this._takeMasterReference(data, page);
	}

	/*
		A preview already shown for the page is drawn again from the new content. This is how a
		client that asked for its cached pages again after being away gets fresh thumbnails. A
		preview that was waiting for a whole part is drawn by the drain alone.
	*/
	protected _partArrived(
		data: cool.VectorPrimitivesData,
		whole: boolean,
	): void {
		const partId = data.partId;
		if (whole) {
			const drained = this._drainDrawable(partId);
			if (this._isDrawable(data)) this._redrawRenderedPreviews(partId, drained);
		} else this._redrawRenderedPreviews(partId);
		if (data.mode === cool.VectorMode.MasterPages)
			this._onMasterArrived(partId);
	}

	/// The previews waiting for the part are drawn from what the cache holds.
	protected _partKept(partId: cool.VectorPartGuid): void {
		this._drainDrawable(partId);
	}

	/// The previews queued for a part that is not there are dropped.
	protected _partMissing(partId: cool.VectorPartGuid): void {
		this._pendingPreviews.delete(partId);
	}

	/*
		Every preview is drawn again once for all the pictures that load at the same moment, as
		the pictures of many controls come in together.
	*/
	protected _onPictureReady(): void {
		super._onPictureReady();
		const parts = new Set<cool.VectorPartGuid>();
		for (const info of this._renderedPreviews.values()) parts.add(info.partId);
		this._redrawPartsUsing(parts);
	}

	/*
		Draw the previews of the parts again, with the pages under a master among them, as a page
		draws the objects of its master too. They draw once in a layouting task, however many
		bitmaps and fonts come in at the same moment, as the ones of a master do.
	*/
	protected _redrawPartsUsing(
		parts: Set<cool.VectorPartGuid> | undefined,
	): void {
		if (!parts) return;
		for (const partId of parts) {
			this._previewPartsToRedraw.add(partId);
			for (const [pageId, data] of this._cache) {
				if (data.masterPartId === partId)
					this._previewPartsToRedraw.add(pageId);
			}
		}
		if (this._previewRedrawPlanned || this._previewPartsToRedraw.size === 0)
			return;
		this._previewRedrawPlanned = true;
		app.layoutingService.appendLayoutingTask(() => {
			this._previewRedrawPlanned = false;
			const pending = this._previewPartsToRedraw;
			this._previewPartsToRedraw = new Set();
			for (const partId of pending) this._redrawRenderedPreviews(partId);
		});
	}
}
