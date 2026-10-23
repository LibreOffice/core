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

// A GIF-only WebCodecs ImageDecoder built on omggif, installed where the browser has none.
// Each decoded frame is a canvas that also carries a duration in microseconds and a close().

declare var GifReader: any;

interface GifDecoderState {
	index: number;
	pixels: Uint8ClampedArray;
	savedPixels: Uint8ClampedArray | null;
}

class GifImageDecoderPolyfill {
	public readonly type: string;
	public readonly complete: boolean = true;
	public readonly completed: Promise<void>;
	public readonly tracks: {
		ready: Promise<void>;
		length: number;
		selectedIndex: number;
		selectedTrack: {
			animated: boolean;
			frameCount: number;
			repetitionCount: number;
			selected: boolean;
		} | null;
	};

	private _reader: any = null;
	private _error: Error | null = null;
	// The whole image after the frames drawn so far, in RGBA.
	private _pixels: Uint8ClampedArray = new Uint8ClampedArray(0);
	// The image from before the last frame with disposal method 3.
	private _savedPixels: Uint8ClampedArray | null = null;
	private _nextIndex: number = 0;
	// The state before the frame that the last jump landed on, to resume from on the next jump.
	private _checkpoint: GifDecoderState | null = null;

	public static isTypeSupported(type: string): Promise<boolean> {
		return Promise.resolve(type === 'image/gif');
	}

	constructor(init: { data: ArrayBuffer | ArrayBufferView; type: string }) {
		if (!init || !init.type) throw new TypeError('ImageDecoder needs a type');
		this.type = init.type;

		if (init.type !== 'image/gif')
			this._error = new DOMException(init.type, 'NotSupportedError');
		else {
			try {
				this._reader = new GifReader(
					GifImageDecoderPolyfill.toBytes(init.data),
				);
				this._pixels = new Uint8ClampedArray(
					this._reader.width * this._reader.height * 4,
				);
			} catch (e) {
				this._error = new DOMException(String(e), 'EncodingError');
			}
		}

		const ready = this._error ? Promise.reject(this._error) : Promise.resolve();
		// A caller that never waits for these is not told about an unhandled rejection.
		ready.catch(() => undefined);
		this.completed = ready;

		const frameCount = this._reader ? this._reader.numFrames() : 0;
		this.tracks = {
			ready: ready,
			length: this._reader ? 1 : 0,
			selectedIndex: this._reader ? 0 : -1,
			selectedTrack: this._reader
				? {
						animated: frameCount > 1,
						frameCount: frameCount,
						repetitionCount: GifImageDecoderPolyfill.repetitions(
							this._reader.loopCount(),
						),
						selected: true,
					}
				: null,
		};
	}

	public decode(options?: {
		frameIndex?: number;
	}): Promise<{ image: any; complete: boolean }> {
		if (this._error) return Promise.reject(this._error);
		if (!this._reader)
			return Promise.reject(
				new DOMException('decoder is closed', 'InvalidStateError'),
			);

		const index = options?.frameIndex ?? 0;
		if (index < 0 || index >= this._reader.numFrames())
			return Promise.reject(new RangeError('frame index out of range'));

		try {
			this.drawUpTo(index);
			return Promise.resolve({
				image: this.createFrame(index),
				complete: true,
			});
		} catch (e) {
			return Promise.reject(new DOMException(String(e), 'EncodingError'));
		}
	}

	public close(): void {
		this._reader = null;
		this._pixels = new Uint8ClampedArray(0);
		this._savedPixels = null;
		this._checkpoint = null;
		this._nextIndex = 0;
	}

	private static toBytes(data: ArrayBuffer | ArrayBufferView): Uint8Array {
		if (data instanceof ArrayBuffer) return new Uint8Array(data);
		return new Uint8Array(data.buffer, data.byteOffset, data.byteLength);
	}

	// The NETSCAPE loop count is 0 for a GIF that loops forever, and missing if it plays once.
	private static repetitions(loopCount: number | null): number {
		if (loopCount === null) return 0;
		return loopCount === 0 ? Infinity : loopCount;
	}

	// Each frame draws over the ones before it, so going back resumes from the checkpoint when
	// it lies before the target frame, or else from the first frame.
	private drawUpTo(index: number): void {
		const jumped = index !== this._nextIndex;
		if (index < this._nextIndex) this.restore(index);
		while (this._nextIndex < index) this.drawNextFrame();
		if (jumped && index > 0 && this._checkpoint?.index !== index)
			this._checkpoint = this.copyState();
		this.drawNextFrame();
	}

	private restore(index: number): void {
		const checkpoint = this._checkpoint;
		if (checkpoint && checkpoint.index <= index) {
			this._pixels.set(checkpoint.pixels);
			this._savedPixels = checkpoint.savedPixels
				? checkpoint.savedPixels.slice()
				: null;
			this._nextIndex = checkpoint.index;
		} else {
			this._pixels.fill(0);
			this._nextIndex = 0;
		}
	}

	private copyState(): GifDecoderState {
		return {
			index: this._nextIndex,
			pixels: this._pixels.slice(),
			savedPixels: this._savedPixels ? this._savedPixels.slice() : null,
		};
	}

	private drawNextFrame(): void {
		if (this._nextIndex > 0) this.disposeFrame(this._nextIndex - 1);

		const info = this._reader.frameInfo(this._nextIndex);
		if (info.disposal === 3) {
			if (!this._savedPixels)
				this._savedPixels = new Uint8ClampedArray(this._pixels.length);
			this._savedPixels.set(this._pixels);
		}
		this._reader.decodeAndBlitFrameRGBA(this._nextIndex, this._pixels);
		this._nextIndex++;
	}

	// Disposal method 2 clears the frame's rectangle and 3 restores the image from before the
	// frame. Methods 0 and 1 leave the frame in place.
	private disposeFrame(index: number): void {
		const info = this._reader.frameInfo(index);
		if (info.disposal === 2) this.clearRectangle(info);
		else if (info.disposal === 3 && this._savedPixels)
			this._pixels.set(this._savedPixels);
	}

	private clearRectangle(info: any): void {
		const width = this._reader.width;
		const right = Math.min(info.x + info.width, width);
		const bottom = Math.min(info.y + info.height, this._reader.height);
		for (let y = info.y; y < bottom; y++)
			this._pixels.fill(0, (y * width + info.x) * 4, (y * width + right) * 4);
	}

	// A canvas, like a VideoFrame, follows the WebGL UNPACK_FLIP_Y_WEBGL setting on upload.
	private createFrame(index: number): any {
		const width = this._reader.width;
		const height = this._reader.height;
		const image: any = document.createElement('canvas');
		image.width = width;
		image.height = height;
		image
			.getContext('2d')
			.putImageData(new ImageData(this._pixels, width, height), 0, 0);
		// The GIF delay is in hundredths of a second.
		image.duration = this._reader.frameInfo(index).delay * 10000;
		image.close = () => {
			image.width = 0;
			image.height = 0;
		};
		return image;
	}
}

if (
	typeof window !== 'undefined' &&
	typeof (window as any).ImageDecoder === 'undefined' &&
	typeof GifReader !== 'undefined'
)
	(window as any).ImageDecoder = GifImageDecoderPolyfill;
