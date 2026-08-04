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
 * PreviewEditTransform mirrors the engine mapping between canvas-page space and the space of
 * the slide shown by the entered preview: a linear scale plus offset derived from the
 * preview rectangle and the slide size. All values are twips.
 */
class PreviewEditTransform {
	private static mode() {
		return app.impress.previewEditMode;
	}

	public static isActive(): boolean {
		const mode = PreviewEditTransform.mode();
		return !!(mode && mode.active && mode.rectangle);
	}

	public static isInsideEnteredPreview(x: number, y: number): boolean {
		if (!PreviewEditTransform.isActive()) return false;
		const rect = PreviewEditTransform.mode().rectangle;
		return x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2;
	}

	private static hasPositiveSize(value: number): boolean {
		return typeof value === 'number' && isFinite(value) && value > 0;
	}

	public static canvasRectangleToSlide(rectangle: any): any {
		// Returns an empty rectangle while the preview edit mode is inactive or its geometry
		// is degenerate, mirroring the empty-result contract of the engine helpers this
		// method mirrors.
		if (!PreviewEditTransform.isActive())
			return new cool.SimpleRectangle(0, 0, 0, 0);
		const mode = PreviewEditTransform.mode();
		const rect = mode.rectangle;
		if (
			!PreviewEditTransform.hasPositiveSize(rect.width) ||
			!PreviewEditTransform.hasPositiveSize(rect.height) ||
			!PreviewEditTransform.hasPositiveSize(mode.slideWidth) ||
			!PreviewEditTransform.hasPositiveSize(mode.slideHeight)
		)
			return new cool.SimpleRectangle(0, 0, 0, 0);
		const scaleX = mode.slideWidth / rect.width;
		const scaleY = mode.slideHeight / rect.height;
		return new cool.SimpleRectangle(
			Math.round((rectangle.x1 - rect.x1) * scaleX),
			Math.round((rectangle.y1 - rect.y1) * scaleY),
			Math.round(rectangle.width * scaleX),
			Math.round(rectangle.height * scaleY),
		);
	}

	public static slideRectangleToCanvas(rectangle: any): any {
		// Returns an empty rectangle while the preview edit mode is inactive or its geometry
		// is degenerate, mirroring the empty-result contract of the engine helpers this
		// method mirrors.
		if (!PreviewEditTransform.isActive())
			return new cool.SimpleRectangle(0, 0, 0, 0);
		const mode = PreviewEditTransform.mode();
		const rect = mode.rectangle;
		if (
			!PreviewEditTransform.hasPositiveSize(rect.width) ||
			!PreviewEditTransform.hasPositiveSize(rect.height) ||
			!PreviewEditTransform.hasPositiveSize(mode.slideWidth) ||
			!PreviewEditTransform.hasPositiveSize(mode.slideHeight)
		)
			return new cool.SimpleRectangle(0, 0, 0, 0);
		const scaleX = rect.width / mode.slideWidth;
		const scaleY = rect.height / mode.slideHeight;
		return new cool.SimpleRectangle(
			Math.round(rect.x1 + rectangle.x1 * scaleX),
			Math.round(rect.y1 + rectangle.y1 * scaleY),
			Math.round(rectangle.width * scaleX),
			Math.round(rectangle.height * scaleY),
		);
	}
}
