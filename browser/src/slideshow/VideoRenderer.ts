// @ts-strict-ignore
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

declare var SlideShow: any;

class VideoRenderInfo extends AnimatedContentRenderInfo {
	public videoElement: HTMLVideoElement;
	public playing: boolean;
	public ended: boolean;

	private sId: string;
	private slideRenderer: SlideRenderer;

	public setup(sId: string, slideRenderer: SlideRenderer, url: string): void {
		this.sId = sId;
		this.slideRenderer = slideRenderer;

		const video = document.createElement('video');
		video.playsInline = true;
		video.crossOrigin = 'anonymous';

		video.addEventListener(
			'playing',
			() => {
				this.playing = true;
				this.ended = false;
				this.slideRenderer.notifyVideoStarted(this.sId);
			},
			true,
		);

		video.addEventListener(
			'pause',
			() => {
				this.playing = false;
				this.slideRenderer.notifyVideoEnded(this.sId);
			},
			true,
		);

		video.addEventListener(
			'ended',
			() => {
				this.playing = false;
				this.ended = true;
				this.slideRenderer.notifyVideoEnded(this.sId);
			},
			true,
		);

		video.src = url;
		this.videoElement = video;
	}

	public load(): void {
		this.videoElement.load();
	}

	public play(reset: boolean = true): void {
		if (reset) this.videoElement.currentTime = 0;
		this.videoElement.play();
	}

	public pause(): void {
		app.console.debug('VideoRenderInfo.pause');
		this.videoElement.pause();
	}

	public handleClick(): void {
		if (this.playing) {
			this.pause();
		} else if (this.ended) {
			this.play(true);
		} else {
			this.play(false);
		}
	}

	public deleteResources(context: RenderContext) {
		this.pause();
		super.deleteResources(context);
	}
}

class VideoRenderer2d extends AnimatedContentRenderer2d {
	public prepare(
		videoInfo: VideoInfo,
		docWidth: number,
		docHeight: number,
	): void {
		const video = new VideoRenderInfo();
		video.setup(this.sId, this._slideRenderer, videoInfo.url);
		video.pos2d = this.getDocumentPositions(
			videoInfo.x,
			videoInfo.y,
			videoInfo.width,
			videoInfo.height,
			docWidth,
			docHeight,
		);
		this.info = video;
		this.infoId = videoInfo.id;
	}

	protected getDrawable(): CanvasImageSource | null {
		return (this.info as VideoRenderInfo).videoElement;
	}
}

class VideoRendererGl extends AnimatedContentRendererGl {
	// WebGL reads the raw decoded video frame and ignores any rotation
	// recorded in the container (for example the display matrix that
	// phone-recorded videos carry), so a video whose frames are stored
	// sideways would end up sideways in the texture too. A 2D canvas
	// context applies that rotation when it draws a video frame, so the
	// frame is normalized through one before it is uploaded as a texture.
	private frameCanvas: HTMLCanvasElement;
	private frameContext: CanvasRenderingContext2D;

	public prepare(videoInfo: VideoInfo, docWidth: number, docHeight: number) {
		const video = new VideoRenderInfo();
		video.setup(this.sId, this._slideRenderer, videoInfo.url);
		video.replaceTexture(this._context, this.initTexture());
		video.replaceVao(
			this._context,
			this.setupRectangleInDocumentPositions(
				videoInfo.x,
				videoInfo.y,
				videoInfo.width,
				videoInfo.height,
				docWidth,
				docHeight,
			),
		);
		this.info = video;
		this.infoId = videoInfo.id;
	}

	protected placeholderPixel(): Uint8Array {
		return new Uint8Array([0, 0, 255, 255]); // opaque blue
	}

	protected getDrawable(): TexImageSource | null {
		const video = this.info as VideoRenderInfo;
		// Keep returning the current decoded frame once playback has started,
		// including after the video ends, so the final frame stays on screen
		// instead of the slide's own still image showing through underneath.
		if (video.videoElement.currentTime <= 0) return null;
		return this.getOrientedFrame(video.videoElement);
	}

	private getOrientedFrame(video: HTMLVideoElement): HTMLCanvasElement {
		if (!this.frameCanvas) {
			this.frameCanvas = document.createElement('canvas');
			this.frameContext = this.frameCanvas.getContext('2d');
		}

		if (
			this.frameCanvas.width !== video.videoWidth ||
			this.frameCanvas.height !== video.videoHeight
		) {
			this.frameCanvas.width = video.videoWidth;
			this.frameCanvas.height = video.videoHeight;
		}

		this.frameContext.drawImage(video, 0, 0);
		return this.frameCanvas;
	}
}

function makeVideoRenderer(
	sId: string,
	context: RenderContext,
	slideRenderer: SlideRenderer,
): AnimatedContentRenderer {
	return context.is2dGl()
		? new VideoRenderer2d(sId, context, slideRenderer)
		: new VideoRendererGl(sId, context, slideRenderer);
}
