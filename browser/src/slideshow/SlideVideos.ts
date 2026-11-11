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

/*
 * The players of the master page videos, by master page hash. A video on a master page is a
 * background. It starts when the first slide with that master page is shown and keeps playing
 * through the slides after it with the same master page. During a transition the master page of the
 * slide being left keeps its players beside those of the slide coming in.
 */
class MasterVideos {
	private renderers: Map<string, Array<AnimatedContentRenderer>> = new Map();

	/// Make, load and play a player per video of the master page of the slide, unless that
	/// master page has its players already.
	public start(slideInfo: SlideInfo, context: RenderContext): void {
		const masterVideos = slideInfo?.masterVideos || [];
		const masterHash = slideInfo?.masterPage;
		if (masterVideos.length === 0 || this.renderers.has(masterHash)) return;

		const slideShowPresenter = app.map.slideShowPresenter;
		const metaPresentation = slideShowPresenter._metaPresentation;
		const videoRenderers: Array<AnimatedContentRenderer> = [];
		for (let i = 0; i < masterVideos.length; ++i) {
			const videoRenderer = makeVideoRenderer(
				masterHash + i,
				context,
				slideShowPresenter._slideRenderer,
			);
			videoRenderer.prepare(
				masterVideos[i],
				metaPresentation.docWidth,
				metaPresentation.docHeight,
			);
			videoRenderer.load();
			videoRenderer.play();
			videoRenderers.push(videoRenderer);
		}
		this.renderers.set(masterHash, videoRenderers);
	}

	/// Stop and free the players of every master page except masterHash.
	public keepOnly(masterHash: string): void {
		for (const [hash, videoRenderers] of this.renderers) {
			if (hash === masterHash) continue;
			for (const videoRenderer of videoRenderers)
				videoRenderer.deleteResources();
			this.renderers.delete(hash);
		}
	}

	public has(masterHash: string): boolean {
		return this.renderers.has(masterHash);
	}

	/// The players of the master page, empty when it has none.
	public get(masterHash: string): Array<AnimatedContentRenderer> {
		return this.renderers.get(masterHash) ?? [];
	}

	/// The number of frames shown so far by all the videos of the master page.
	public presentedFrames(masterHash: string): number {
		let frames = 0;
		for (const videoRenderer of this.get(masterHash))
			frames += videoRenderer.getPresentedFrames();
		return frames;
	}

	/// Whether sId names a player of a master page video.
	public isPlayer(sId: string): boolean {
		for (const videoRenderers of this.renderers.values())
			if (videoRenderers.some((videoRenderer) => videoRenderer.getId() === sId))
				return true;
		return false;
	}

	/// Stop every video and free its player.
	public deleteResources(): void {
		for (const videoRenderers of this.renderers.values())
			for (const videoRenderer of videoRenderers)
				videoRenderer.deleteResources();
		this.renderers.clear();
	}
}
