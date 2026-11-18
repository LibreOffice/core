// @ts-strict-ignore
/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class MetaPresentation {
	public docWidth: number;
	public docHeight: number;
	private _numberOfSlides: number;
	private firstSlideHash: string;
	private lastSlideHash: string;
	private _startSlideIndex: number;
	private metaSlides: Map<string, MetaSlide>;
	private partHashes: Map<number, string>;
	private aSlideShowHandler: SlideShowHandler;
	private slideShowNavigator: SlideShowNavigator;

	constructor(
		info: PresentationInfo,
		aSlideShowHandler: SlideShowHandler,
		aSlideShowNavigator: SlideShowNavigator,
	) {
		this.aSlideShowHandler = aSlideShowHandler;
		this.update(info);
		this.setNavigator(aSlideShowNavigator);
	}

	setNavigator(nav: SlideShowNavigator) {
		this.slideShowNavigator = nav;

		// We set up a low priority for the invocation of canvas handleClick
		// in order to make clicks on shapes, that start interactive animation
		// sequence (on click), have a higher priority.
		this.metaSlides.forEach((metaSlide) => {
			if (metaSlide.animationsHandler) {
				const eventMultiplexer = metaSlide.animationsHandler.eventMultiplexer;
				if (eventMultiplexer)
					eventMultiplexer.registerMouseClickHandler(
						this.slideShowNavigator.canvasClickHandler,
						100,
					);
			}
		});
	}

	public get slideShowHandler(): SlideShowHandler {
		return this.aSlideShowHandler;
	}

	public get startSlideIndex(): number {
		return this._startSlideIndex;
	}

	public update(info: PresentationInfo) {
		this.docWidth = info.docWidth;
		this.docHeight = info.docHeight;

		const aContext = this.aSlideShowHandler.getContext();
		aContext.nSlideWidth = this.docWidth;
		aContext.nSlideHeight = this.docHeight;

		this._numberOfSlides = info.slides.length;
		if (this._numberOfSlides === 0) return;

		this.firstSlideHash = info.slides[0].hash;
		this.lastSlideHash = info.slides[this._numberOfSlides - 1].hash;

		this.metaSlides = new Map();
		this.partHashes = new Map();
		let prevSlideHash = null;
		for (let i = 0; i < this._numberOfSlides; ++i) {
			const slide = info.slides[i];
			slide.indexInSlideShow = i;
			slide.prev = prevSlideHash;
			slide.next =
				i + 1 < this._numberOfSlides ? info.slides[i + 1].hash : null;
			const metaSlide = new MetaSlide(slide, this);
			this.metaSlides.set(slide.hash, metaSlide);
			this.partHashes.set(slide.index, slide.hash);
			prevSlideHash = slide.hash;
		}
	}

	public isEmpty(): boolean {
		return this._numberOfSlides === 0;
	}

	public get numberOfSlides(): number {
		return this._numberOfSlides;
	}

	public getDocWidth(): number {
		return this.docWidth;
	}

	public getDocHeight(): number {
		return this.docHeight;
	}

	public setDocWidth(slideWidth: number) {
		this.docWidth = slideWidth;
	}

	public setDocHeight(slideHeight: number) {
		this.docHeight = slideHeight;
	}

	public getCurrentSlideIndex(): number {
		return this.slideShowNavigator.currentSlideIndex;
	}

	public getCurrentSlideHash(): string {
		return this.getSlideHash(this.getCurrentSlideIndex());
	}

	public getSlideHash(slideIndex: number) {
		return this.partHashes.get(slideIndex);
	}

	public isFirstSlide(hash: string) {
		return hash === this.firstSlideHash;
	}

	public isLastSlide(hash: string) {
		return hash === this.lastSlideHash;
	}

	public getMetaSlide(slideHash: string): MetaSlide {
		return this.metaSlides.get(slideHash);
	}

	public getMetaSlideByIndex(slideIndex: number): MetaSlide {
		return this.getMetaSlide(this.getSlideHash(slideIndex));
	}

	public getSlideInfo(slideHash: string): SlideInfo {
		const metaSlide = this.getMetaSlide(slideHash);
		return metaSlide ? metaSlide.info : null;
	}

	public getSlideInfoByIndex(slideIndex: number): SlideInfo {
		const slideHash = this.getSlideHash(slideIndex);
		return slideHash ? this.getSlideInfo(slideHash) : null;
	}

	public getMetaSlides() {
		return this.metaSlides;
	}

	public setCurrentSlide(nSlideIndex: number) {
		if (nSlideIndex >= 0 && nSlideIndex < this.numberOfSlides) {
			const nCurSlide = this.getCurrentSlideIndex();
			if (nCurSlide !== undefined) this.getMetaSlideByIndex(nCurSlide).hide();
			this.getMetaSlideByIndex(nCurSlide).show();
		} else {
			window.app.console.log(
				'MetaPresentation.setCurrentSlide: slide index out of range: ' +
					nSlideIndex,
			);
		}
	}
}
