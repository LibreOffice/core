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
 * VectorResourceLoader - the images and fonts that vector content names, asked for from the
 * engine once content names them. An image is kept once it decodes, and a font once it loads,
 * registered on document.fonts under the family "vecfont-<id>". Each owner, such as a page or a
 * block of cells, names what its content uses, and an image that no owner uses goes.
 */

class VectorResourceLoader {
	/// The decoded images by their checksum.
	readonly images = new VectorResourceTracker<number, HTMLImageElement>(
		(checksum) =>
			'commandvalues command=.uno:VectorRenderingGraphics?checksum=' +
			String(checksum),
	);

	/// The loaded font faces by font id.
	readonly fonts = new VectorResourceTracker<string, FontFace>(
		(fontId) => 'commandvalues command=.uno:VectorRenderingFont?id=' + fontId,
	);

	/// Called with the owners whose content names an image or a font that has just loaded, or
	/// undefined when no owner names it.
	private readonly _loaded: (owners: Set<string> | undefined) => void;

	/// Called with the checksum of an image that decoded or turned out to be unavailable.
	private readonly _imageSettled?: (checksum: number) => void;

	constructor(
		loaded: (owners: Set<string> | undefined) => void,
		imageSettled?: (checksum: number) => void,
	) {
		this._loaded = loaded;
		this._imageSettled = imageSettled;
	}

	/// Add the images and fonts the walk visits to those the owner uses, and ask for the ones
	/// that are not held.
	add(owner: string, walk: (walker: cool.VectorResourceWalker) => void): void {
		const checksums = new Set<number>();
		const fontIds = new Set<string>();
		walk(new cool.VectorResourceWalker(checksums, fontIds));
		this.images.indexForPart(owner, checksums);
		this.fonts.indexForPart(owner, fontIds);
		this.images.requestMissing(checksums);
		this.fonts.requestMissing(fontIds);
	}

	/// Make the images and fonts the walk visits all that the owner uses, and ask for the ones
	/// that are not held. An image the owner no longer uses goes once no owner uses it. An
	/// image the old and the new content share stays.
	replace(
		owner: string,
		walk: (walker: cool.VectorResourceWalker) => void,
	): void {
		const checksums = new Set<number>();
		const fontIds = new Set<string>();
		walk(new cool.VectorResourceWalker(checksums, fontIds));
		this.images.replacePart(owner, checksums, /*dropUnused=*/ true);
		this.fonts.replacePart(owner, fontIds, /*dropUnused=*/ false);
		this.images.requestMissing(checksums);
		this.fonts.requestMissing(fontIds);
	}

	/// Forget what the owner uses. An image that only it used goes, and the fonts stay loaded.
	release(owner: string): void {
		this.images.releasePart(owner);
		this.fonts.forgetPart(owner);
	}

	/// Forget what every owner uses, keeping the images and fonts loaded.
	forgetOwners(): void {
		this.images.forgetAllParts();
		this.fonts.forgetAllParts();
	}

	/// The decoded image with the checksum, or undefined. An image that an owner uses and that
	/// is not held, such as after dropImages, is asked for again.
	image(checksum: number): HTMLImageElement | undefined {
		const image = this.images.get(checksum);
		if (!image && this.images.partsFor(checksum))
			this.images.requestMissing(new Set([checksum]));
		return image;
	}

	/// Drop the decoded images, keeping which owners use them. The fonts stay registered, since
	/// they are few and small.
	dropImages(): void {
		this.images.dropValues();
	}

	/// Drop the images and the fonts, and take the fonts off the document.
	discard(): void {
		for (const face of this.fonts.values())
			(document as unknown as { fonts: Set<FontFace> }).fonts.delete(face);
		this.fonts.clear();
		this.images.clear();
	}

	/// Decode and keep a fetched image. Data that is missing or does not decode marks the
	/// checksum as unavailable, so it is not asked for again.
	handleImage(values: cool.VectorRenderingGraphicsResponse): void {
		window.app.console.log(
			'vectorrenderinggraphics pulled payload: ' +
				(values.data ? values.data.length : 0) +
				' base64 bytes, checksum=' +
				values.checksum,
		);
		if (this.images.has(values.checksum)) {
			this.images.clearInFlight(values.checksum);
			return;
		}
		if (!values.data) {
			this._imageUnavailable(
				values.checksum,
				'the engine reported: ' + (values.error || 'no image data'),
			);
			return;
		}
		const image = new Image();
		image.onload = () => {
			this.images.setLoaded(values.checksum, image);
			this._imageSettled?.(values.checksum);
			this._loaded(this.images.partsFor(values.checksum));
		};
		image.onerror = () => {
			this._imageUnavailable(
				values.checksum,
				'its image data failed to decode',
			);
		};
		image.src = values.data;
	}

	/// Register a fetched font as a FontFace under the family "vecfont-<id>". The face goes on
	/// the document only once it loads, so a font that fails to decode leaves the text on its
	/// family name.
	handleFont(values: cool.VectorRenderingFontResponse): void {
		const fontId = values.fontId;
		if (this.fonts.has(fontId)) {
			this.fonts.clearInFlight(fontId);
			return;
		}
		if (!values.data) {
			// The engine does not hold the id.
			this.fonts.setUnavailable(fontId);
			return;
		}
		const binary = window.atob(values.data);
		const bytes = new Uint8Array(binary.length);
		for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
		const face = new FontFace('vecfont-' + fontId, bytes.buffer);
		face
			.load()
			.then(() => {
				(document as unknown as { fonts: Set<FontFace> }).fonts.add(face);
				this.fonts.setLoaded(fontId, face);
				this._loaded(this.fonts.partsFor(fontId));
			})
			.catch(() => {
				this.fonts.setUnavailable(fontId);
			});
	}

	/// Record a checksum no usable image could be obtained for, and log the reason.
	private _imageUnavailable(checksum: number, reason: string): void {
		window.app.console.warn(
			'VectorRenderingGraphics: bitmap ' +
				String(checksum) +
				' is unavailable: ' +
				reason,
		);
		this.images.setUnavailable(checksum);
		this._imageSettled?.(checksum);
	}
}
