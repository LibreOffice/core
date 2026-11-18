// @ts-strict-ignore
/** */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * SlideCache is storing the cached layers
 */

declare var SlideShow: any;

class SlideCache {
	private renderedSlides: Map<string, ImageBitmap> = new Map();
	// TODO: cache other layers here

	public has(slideHash: string) {
		return this.renderedSlides.has(slideHash);
	}

	public get(slideHash: string): ImageBitmap {
		return this.renderedSlides.get(slideHash);
	}

	public set(slideHash: string, image: ImageBitmap): void {
		this.renderedSlides.set(slideHash, image);
	}

	public invalidate(slideHash: string): void {
		this.renderedSlides.delete(slideHash);
	}

	public invalidateAll(): void {
		this.renderedSlides.clear();
	}
}

SlideShow.SlideCache = SlideCache;
