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
				if (commandTargets.has(String(videosInfo[i].id))) continue;
				videoRenderers[i].play();
			}
		}
	}

	/// Draw the videos of the slide, except the ids in drawnIds. lookOf gives
	/// how an animation draws the video of a shape id, undefined for the video
	/// as it comes.
	public render(
		slideHash: string,
		lookOf?: (shapeId: number) => AnimatedContentLook | undefined,
		drawnIds?: Set<number>,
	): void {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) {
			if (drawnIds?.has(videoRenderer.infoId)) continue;
			videoRenderer.render(lookOf?.(videoRenderer.infoId));
		}
	}

	/// Draw the one video of the slide with this id. Returns false when the
	/// slide has no such video.
	public renderVideo(
		slideHash: string,
		videoId: number,
		look?: AnimatedContentLook,
	): boolean {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return false;

		for (const videoRenderer of videoRenderers) {
			if (videoRenderer.infoId !== videoId) continue;
			videoRenderer.render(look);
			return true;
		}
		return false;
	}

	/// The ids of the shapes of the slide that hold a video.
	public shapeIds(slideHash: string): number[] {
		const videoRenderers = this.renderers.get(slideHash);
		if (!videoRenderers) return [];
		return videoRenderers.map((videoRenderer) => videoRenderer.infoId);
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
