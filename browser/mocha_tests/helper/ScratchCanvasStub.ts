/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// Stands in for the scratch canvases. jsdom has no 2D context, so each canvas
/// created holds a CanvasRecorder.
class ScratchCanvasStub {
	/// Every scratch canvas handed out, in order.
	public static readonly created: CanvasRecorder[] = [];

	/// The RGBA pixel each scratch canvas reads back, in creation order. None
	/// is transparent black.
	public static readonly imagePixels: (number[] | undefined)[] = [];

	private static _originalCreateElement: any;

	/// For one test. Other elements still come from the real document.
	static install(): void {
		ScratchCanvasStub._redirect(() => ScratchCanvasStub._makeCanvas());
	}

	/// Canvases without a context, as when a browser runs out of them.
	static installUnavailable(): void {
		ScratchCanvasStub._redirect(() => ({
			width: 0,
			height: 0,
			getContext: () => null,
		}));
	}

	/// Forget the canvases and pixels so far, so the next render starts at the
	/// first canvas. The stub stays installed.
	static reset(): void {
		ScratchCanvasStub.created.length = 0;
		ScratchCanvasStub.imagePixels.length = 0;
	}

	private static _redirect(make: () => any): void {
		ScratchCanvasStub.reset();
		const document = (globalThis as any).document;
		ScratchCanvasStub._originalCreateElement = document.createElement;
		document.createElement = function (tag: string, ...rest: any[]): any {
			if (String(tag).toLowerCase() !== 'canvas')
				return ScratchCanvasStub._originalCreateElement.call(
					document,
					tag,
					...rest,
				);
			return make();
		};
	}

	static uninstall(): void {
		if (!ScratchCanvasStub._originalCreateElement) return;
		(globalThis as any).document.createElement =
			ScratchCanvasStub._originalCreateElement;
		ScratchCanvasStub._originalCreateElement = undefined;
	}

	/// Zero is the first canvas created.
	static recorder(index: number): CanvasRecorder {
		return ScratchCanvasStub.created[index];
	}

	private static _makeCanvas(): any {
		const recorder = new CanvasRecorder(0, 0);
		recorder.imagePixel =
			ScratchCanvasStub.imagePixels[ScratchCanvasStub.created.length];
		ScratchCanvasStub.created.push(recorder);
		// A size set on the canvas reads back from the recorder.
		const canvas: any = {
			width: 0,
			height: 0,
			getContext: () => recorder,
		};
		(recorder as any).canvas = canvas;
		return canvas;
	}
}
