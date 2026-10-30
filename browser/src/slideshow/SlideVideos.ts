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

/*
 * The video renderers of each slide, by slide hash. A renderer draws its
 * video over the slide into the render context it was made with.
 */

class SlideVideos {
	private renderers: Map<string, Array<AnimatedContentRenderer>> = new Map();

	/// Make and load a renderer per video of the slide. The Qt and macOS
	/// shells play a video that no command controls at once. A browser
	/// needs a user gesture for sound, so there a video waits for a click.
	public enterSlide(
		slideHash: string,
		slideInfo: SlideInfo,
		context: RenderContext,
	): void {
		// Each entry makes the renderers anew, so the old ones go first.
		const previousRenderers = this.renderers.get(slideHash);
		if (previousRenderers) {
			for (const videoRenderer of previousRenderers)
				videoRenderer.deleteResources();
			this.renderers.delete(slideHash);
		}

		const videosInfo = slideInfo?.videos;
		if (!videosInfo || videosInfo.length === 0) return;

		const slideShowPresenter = app.map.slideShowPresenter;
		const slideRenderer = slideShowPresenter._slideRenderer;
		const metaPresentation = slideShowPresenter._metaPresentation;

		const videoRenderers: Array<AnimatedContentRenderer> = [];
		for (let i = 0; i < videosInfo.length; ++i) {
			const videoRenderer = makeVideoRenderer(
				slideHash + i,
				context,
				slideRenderer,
			);
			videoRenderer.prepare(
				videosInfo[i],
				metaPresentation.docWidth,
				metaPresentation.docHeight,
			);
			videoRenderers.push(videoRenderer);
		}
		this.renderers.set(slideHash, videoRenderers);

		for (const videoRenderer of videoRenderers) videoRenderer.load();
		if (window.ThisIsTheQtApp || window.ThisIsTheMacOSApp) {
			const commandTargets = new Set<string>();
			if (slideInfo.animations)
				collectCommandTargets(slideInfo.animations.root, commandTargets);
			for (let i = 0; i < videosInfo.length; ++i) {
				if (commandTargets.has(videosInfo[i].hash)) continue;
				videoRenderers[i].play();
			}
		}
	}

	public render(slideHash: string): void {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) videoRenderer.render();
	}

	public pause(slideHash: string): void {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) videoRenderer.pause();
	}

	/// The renderer of the video, or null.
	public get(slideHash: string, videoInfo: VideoInfo): AnimatedContentRenderer {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return null;

		for (const videoRenderer of videoRenderers) {
			if (videoRenderer.infoId === videoInfo.id) return videoRenderer;
		}
		return null;
	}

	/// Stop every video and free its renderer.
	public deleteResources(): void {
		this.renderers.forEach((videoRenderers) => {
			for (const videoRenderer of videoRenderers)
				videoRenderer.deleteResources();
		});
	}

	/// Forget every renderer.
	public clear(): void {
		this.renderers.clear();
	}
}
