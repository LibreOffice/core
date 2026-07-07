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
 * GifFrameSource decodes an animated image (for example an animated GIF) into
 * individual frames with the WebCodecs ImageDecoder, and advances the current
 * frame on each frame's own duration.
 */

// The frame duration to use when the decoder reports none, in microseconds.
const GIF_DEFAULT_FRAME_DURATION_MICROS = 100000;

// Upper bound on how many decoded frames are held in memory. Decoded frames are
// heavy image resources, so a gif with more frames than this keeps decoding its
// extra frames one at a time instead of caching them all.
const GIF_MAX_CACHED_FRAMES = 60;

class GifFrameSource {
	private _url: string;
	private _onFrame: () => void;
	private _decoder: any = null;
	private _frameCount: number = 0;
	private _frameIndex: number = 0;
	private _currentFrame: any = null;
	private _currentFrameOwned: boolean = false;
	private _frameCache: Map<number, any> = new Map();
	private _timer: ReturnType<typeof setTimeout> | null = null;
	private _disposed: boolean = false;

	constructor(url: string, onFrame: () => void) {
		this._url = url;
		this._onFrame = onFrame;
	}

	public static isSupported(): boolean {
		return typeof (window as any).ImageDecoder !== 'undefined';
	}

	public getCurrentFrame(): any {
		return this._currentFrame;
	}

	// Returns the raw image bytes for the source, or null when the source is not
	// a base64 data URL. The image arrives inline as a base64 data URL, so the
	// bytes are decoded in place.
	private _readImageBytes(url: string): ArrayBuffer | null {
		const comma = url.indexOf(',');
		if (url.startsWith('data:') && comma !== -1) {
			const header = url.slice('data:'.length, comma);
			if (header.endsWith(';base64')) {
				const payload = url.slice(comma + 1);
				const bytes = (Uint8Array as any).fromBase64(payload);
				return bytes.buffer;
			}
		}
		app.console.error('GifFrameSource: image data is not a base64 data URL');
		return null;
	}

	// Reads the image bytes, sets up the decoder, and starts the clock that
	// advances frames on their own durations.
	public load(): void {
		if (!GifFrameSource.isSupported()) {
			app.console.error(
				'GifFrameSource.load failed: ImageDecoder is not available',
			);
			return;
		}

		const buffer = this._readImageBytes(this._url);
		if (!buffer) return;

		this._initDecoder(buffer);
		this._scheduleNextFrame(GIF_DEFAULT_FRAME_DURATION_MICROS);
	}

	// Creates the decoder from the image bytes and reads the frame count once
	// the decoder's track metadata is ready.
	private _initDecoder(buffer: ArrayBuffer): void {
		if (this._disposed) return;
		this._decoder = new (window as any).ImageDecoder({
			data: buffer,
			type: 'image/gif',
		});
		this._decoder.tracks.ready
			.then(() => {
				if (this._disposed) return;
				const track = this._decoder.tracks.selectedTrack;
				this._frameCount = track ? track.frameCount : 0;
			})
			.catch((e: any) => {
				app.console.error('GifFrameSource.load failed: ' + e);
			});
	}

	// Advances to the next frame and arms the timer that shows it. The document
	// tiles already show the first frame, so the animation can start on a timer
	// while the decoder's track metadata is still loading; until that metadata
	// is ready the frame count is zero, so the index stays on the first frame.
	private _scheduleNextFrame(durationMicros: number): void {
		if (this._frameCount)
			this._frameIndex = (this._frameIndex + 1) % this._frameCount;
		this._timer = app.timerRegistry.setTimeout(
			'gifframe',
			() => this._decodeNextFrame(),
			Math.max(16, durationMicros / 1000),
		);
	}

	// Shows the current frame, either from the cache or freshly decoded, then
	// schedules the next one after this frame's duration.
	private _decodeNextFrame(): void {
		if (this._disposed || !this._decoder || !this._frameCount) return;

		const index = this._frameIndex % this._frameCount;

		const cached = this._frameCache.get(index);
		if (cached) {
			this._showFrame(
				cached,
				cached.duration || GIF_DEFAULT_FRAME_DURATION_MICROS,
				false,
			);
			return;
		}

		this._decoder
			.decode({ frameIndex: index })
			.then((result: { image: any }) => {
				if (this._disposed) {
					result.image.close();
					return;
				}
				const durationMicros =
					result.image.duration || GIF_DEFAULT_FRAME_DURATION_MICROS;

				const cacheable = this._frameCache.size < GIF_MAX_CACHED_FRAMES;
				if (cacheable) this._frameCache.set(index, result.image);
				this._showFrame(result.image, durationMicros, !cacheable);
			})
			.catch((e: any) => {
				app.console.error('GifFrameSource.decode failed: ' + e);
			});
	}

	// Stores the frame for the host to draw, notifies the host, and schedules
	// the next frame.
	private _showFrame(frame: any, durationMicros: number, owned: boolean): void {
		if (this._currentFrame && this._currentFrameOwned)
			this._currentFrame.close();
		this._currentFrame = frame;
		this._currentFrameOwned = owned;
		this._onFrame();
		this._scheduleNextFrame(durationMicros);
	}

	public dispose(): void {
		this._disposed = true;
		if (this._timer) {
			app.timerRegistry.clearTimeout(this._timer);
			this._timer = null;
		}

		if (this._currentFrame && this._currentFrameOwned) {
			this._currentFrame.close();
		}
		this._currentFrame = null;
		for (const frame of this._frameCache.values()) {
			frame.close();
		}
		this._frameCache.clear();
		if (this._decoder) {
			try {
				this._decoder.close();
			} catch (e) {
				// The decoder may already be closed.
			}
			this._decoder = null;
		}
	}
}
