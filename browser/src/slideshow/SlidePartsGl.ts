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
 * A slide that plays videos on its master page, split at those videos into still pictures and
 * held as textures in the WebGL context of a slideshow canvas. A frame draws the stills over the
 * whole slide with the current frame of each video between them.
 */

class SlidePartsGl {
	public readonly slideHash: string;
	// The version of the slide the parts were taken at.
	public readonly version: number;
	private readonly context: RenderContextGl;
	// A still part holds its texture. A video part holds its video, and the index of the part
	// is the key of its frame texture.
	private parts: Array<{
		texture?: WebGLTexture | null;
		video?: AnimatedContentRenderer;
	}> = [];

	constructor(
		context: RenderContextGl,
		slideHash: string,
		version: number,
		slideParts: SlidePart[],
	) {
		this.context = context;
		this.slideHash = slideHash;
		this.version = version;

		const gl = context.getGl();
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
		for (const slidePart of slideParts) {
			if (slidePart.image) {
				this.parts.push({
					texture: context.loadTexture(slidePart.image) as WebGLTexture,
				});
				slidePart.image.close();
			} else {
				this.parts.push({ video: slidePart.video });
			}
		}
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
	}

	/// Draw the slide into the frame quads has begun. A video with no frame to draw leaves the
	/// still below it showing.
	public draw(quads: SlideQuadsGl): void {
		const whole = [0, 0, quads.frameWidth, quads.frameHeight];
		this.parts.forEach((part, index) => {
			if (!part.video) {
				quads.drawQuad(part.texture ?? null, whole);
				return;
			}
			const frame = part.video.currentFrame();
			if (!frame) return;
			const texture = quads.frameTexture(index, frame.source);
			if (texture) quads.drawQuad(texture, quads.frameRectangle(frame));
		});
	}

	public dispose(): void {
		if (!this.context.isDisposed()) {
			for (const part of this.parts)
				if (part.texture) this.context.deleteTexture(part.texture);
		}
		this.parts = [];
	}
}
