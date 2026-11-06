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
 * VectorManagerBase - What every render manager that draws from vector primitives holds: the
 * images and fonts the content names, the primitive renderer, the listeners that hear of each
 * change of the content, and the objects drawn elsewhere.
 */

abstract class VectorManagerBase extends RenderManagerBase {
	/// The images and fonts the content names, each kept for the owners that name it.
	protected _resources = new VectorResourceLoader(
		(owners) => this._resourcesLoaded(owners),
		(checksum) => this._imageSettled(checksum),
	);

	protected readonly _renderer: cool.VectorPrimitiveRenderer =
		new cool.VectorPrimitiveRenderer(
			(checksum) => this._imageFor(checksum),
			(fontId) => this._resources.fonts.has(fontId),
			// a picture the client made of a form control is drawn once it has loaded
			() => this._onPictureReady(),
		);

	private _changeListeners = new cool.VectorChangeListeners();

	/// The objects left out of the drawing because something else draws them. Its version
	/// tells a drawing kept from before apart from one that matches them.
	protected _drawnElsewhere = new cool.DrawnElsewhere();

	/*
		Register a callback fired whenever the content changes. It is handed the ids of the
		objects an answer from the engine replaced, an empty list where the change was made here
		and the engine answered nothing, and nothing at all where everything may have changed.
		inputSequence is the newest input of this view that an answer reflects.
	*/
	onVectorChanged(callback: VectorChangeListener): void {
		this._changeListeners.add(callback);
	}

	/// Forget a callback registered with onVectorChanged.
	offVectorChanged(callback: VectorChangeListener): void {
		this._changeListeners.remove(callback);
	}

	protected _fireChanged(answered?: number[], inputSequence?: number): void {
		this._changeListeners.fire(answered, inputSequence);
	}

	/// Draw again what the client changed in place, such as the objects after a drag.
	changedHere(): void {
		this._fireChanged([]);
	}

	/// The objects to leave out of the drawing from now on, since they are drawn elsewhere. An
	/// empty list leaves nothing out.
	drawnElsewhere(objectIds: number[]): void {
		this._drawnElsewhere.set(objectIds);
	}

	drawnElsewhereVersion(): number {
		return this._drawnElsewhere.version();
	}

	/// Cache a fetched image, or mark it unavailable when it is missing or fails to decode.
	handleVectorRenderingGraphicsResponse(
		values: cool.VectorRenderingGraphicsResponse,
	): void {
		this._resources.handleImage(values);
	}

	/// Register a fetched font as a FontFace, which is drawn with once it loads.
	handleVectorRenderingFontResponse(
		values: cool.VectorRenderingFontResponse,
	): void {
		this._resources.handleFont(values);
	}

	/// Release the decoded images and what the renderer keeps between draws. What is still
	/// needed is asked for again. The fonts stay registered, since they are few and small.
	reclaimGraphicsMemory(): void {
		this._resources.dropImages();
		this._renderer.releaseScratchCanvases();
		this._renderer.releaseColorChangedImages();
	}

	/// The width of a string as the renderer draws the run, so predicted widths match the
	/// screen.
	protected _textMeasure(): cool.TextRunMeasure {
		return cool.canvasTextMeasure((fontId) =>
			this._resources.fonts.has(fontId),
		);
	}

	/// The image the renderer draws for the checksum, or undefined while it is not decoded.
	protected _imageFor(checksum: number): HTMLImageElement | undefined {
		return this._resources.image(checksum);
	}

	/// An image or a font that the content names has loaded, for the owners that name it, or
	/// undefined when no owner does. It changes nothing at the engine, so no object is answered.
	protected _resourcesLoaded(_owners: Set<string> | undefined): void {
		this._fireChanged([]);
	}

	/// An image decoded or turned out to be unavailable.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected _imageSettled(_checksum: number): void {}

	/// A picture the renderer made of a form control has loaded.
	protected _onPictureReady(): void {
		this._fireChanged([]);
	}
}
