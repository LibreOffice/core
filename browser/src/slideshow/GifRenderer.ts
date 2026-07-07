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
 * GifRenderer plays an animated Gif inside the slide, through the same
 * texture-upload path the slideshow uses for video.
 *
 * The animated frames are produced by a GifFrameSource, which decodes the image
 * with the WebCodecs ImageDecoder and advances the current frame on each frame's
 * own duration.
 */

type GifInfo = AnimatedContentInfo;

class GifRenderInfo extends AnimatedContentRenderInfo {
	public decoderUrl!: string;

	private source: GifFrameSource | null = null;

	private sId!: string;
	private slideRenderer!: SlideRenderer;
	private renderLoopStarted: boolean = false;

	public setup(sId: string, slideRenderer: SlideRenderer, url: string): void {
		this.sId = sId;
		this.slideRenderer = slideRenderer;
		this.decoderUrl = url;
	}

	public load(): void {
		this.source = new GifFrameSource(this.decoderUrl, () => {
			if (this.renderLoopStarted) return;
			this.renderLoopStarted = true;
			this.slideRenderer.notifyGifStarted(this.sId);
		});
		this.source.load();
	}

	public getCurrentFrame(): any {
		return this.source ? this.source.getCurrentFrame() : null;
	}

	public deleteResources(context: RenderContext) {
		this.slideRenderer.notifyGifEnded(this.sId);
		if (this.source) {
			this.source.dispose();
			this.source = null;
		}
		super.deleteResources(context);
	}
}

class GifRenderer2d extends AnimatedContentRenderer2d {
	public prepare(gifInfo: GifInfo, docWidth: number, docHeight: number): void {
		const gif = new GifRenderInfo();
		gif.setup(this.sId, this._slideRenderer, gifInfo.url);
		gif.pos2d = this.getDocumentPositions(
			gifInfo.x,
			gifInfo.y,
			gifInfo.width,
			gifInfo.height,
			docWidth,
			docHeight,
		);
		this.info = gif;
		this.infoId = gifInfo.id;
	}

	protected getDrawable(): CanvasImageSource | null {
		return (this.info as GifRenderInfo).getCurrentFrame();
	}
}

class GifRendererGl extends AnimatedContentRendererGl {
	public prepare(gifInfo: GifInfo, docWidth: number, docHeight: number) {
		const gif = new GifRenderInfo();
		gif.setup(this.sId, this._slideRenderer, gifInfo.url);
		gif.replaceTexture(this._context, this.initTexture());
		gif.replaceVao(
			this._context,
			this.setupRectangleInDocumentPositions(
				gifInfo.x,
				gifInfo.y,
				gifInfo.width,
				gifInfo.height,
				docWidth,
				docHeight,
			),
		);
		this.info = gif;
		this.infoId = gifInfo.id;
	}

	protected getDrawable(): TexImageSource | null {
		return (this.info as GifRenderInfo).getCurrentFrame();
	}

	protected placeholderPixel(): Uint8Array {
		return new Uint8Array([0, 0, 0, 0]); // transparent
	}
}

function makeGifRenderer(
	sId: string,
	context: RenderContext,
	slideRenderer: SlideRenderer,
): AnimatedContentRenderer {
	return context.is2dGl()
		? new GifRenderer2d(sId, context, slideRenderer)
		: new GifRendererGl(sId, context, slideRenderer);
}
