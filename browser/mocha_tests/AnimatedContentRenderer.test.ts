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

describe('AnimatedContentRenderer2d', function () {
	// Node has no Path2D, so a stand-in records the path.
	let originalPath2D: any;
	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});
	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	class FrameInfo extends AnimatedContentRenderInfo {
		public load(): void {
			// The frame has nothing to load.
		}
	}

	// Content that covers the middle of the slide with one frame.
	class FrameRenderer extends AnimatedContentRenderer2d {
		public prepare(): void {
			const info = new FrameInfo();
			info.pos2d = [0.25, 0.75, 0.25, 0.75];
			this.info = info;
		}

		protected getDrawable(): CanvasImageSource | null {
			return {} as CanvasImageSource;
		}
	}

	// The canvas is the slide's size, so a slide unit is a pixel.
	function draw(state?: cool.DrawState): CanvasRecorder {
		const recorder = new CanvasRecorder(200, 100);
		const context: any = {
			isDisposed: () => false,
			get2dGl: () => recorder,
			get2dOffscreen: () => null,
		};
		const renderer = new FrameRenderer('frame', context, null as any);
		renderer.prepare();
		renderer.render(
			state ? { state, slideWidth: 200, slideHeight: 100 } : undefined,
		);
		return recorder;
	}

	it('draws the content at the opacity and under the transform an animation gives it', function () {
		const recorder = draw({ alpha: 0.5, transform: [1, 0, 0, 1, 30, 0] });

		const frame = recorder.findCall('drawImage');
		const transform = recorder.findCall('transform');
		nodeassert.ok(frame);
		nodeassert.ok(transform);
		nodeassert.strictEqual(frame.properties.globalAlpha, 0.5);
		nodeassert.deepStrictEqual(transform.args, [1, 0, 0, 1, 30, 0]);
		// The transform applies to the frame, so it comes before the frame.
		nodeassert.ok(
			recorder.calls.indexOf(transform) < recorder.calls.indexOf(frame),
		);
	});

	it('shows only the part of the content that the clip of its shape shows', function () {
		const recorder = draw({
			clip: {
				transform: [100, 0, 0, 50, 0, 0],
				frame: [0, 0, 1, 1],
				pieces: [],
			},
		});

		const clip = recorder.findCall('clip');
		const frame = recorder.findCall('drawImage');
		nodeassert.ok(clip);
		nodeassert.ok(frame);
		nodeassert.ok(recorder.calls.indexOf(clip) < recorder.calls.indexOf(frame));
	});

	it('leaves out the content while its shape is hidden', function () {
		const recorder = draw({ hidden: true });

		nodeassert.strictEqual(recorder.findCall('drawImage'), undefined);
	});

	it('draws the content as it comes without a look', function () {
		const recorder = draw();

		nodeassert.ok(recorder.findCall('drawImage'));
		nodeassert.strictEqual(recorder.findCall('transform'), undefined);
	});
});
