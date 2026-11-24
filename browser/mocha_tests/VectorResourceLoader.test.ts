/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('VectorResourceLoader', function () {
	// The first bytes of every PNG file.
	const pngSignature = [0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a];

	// Reads a blob into a data: URL some time later, as FileReader does in the browser.
	class FileReaderStandIn {
		result: string | null = null;
		onload: (() => void) | null = null;
		onerror: (() => void) | null = null;

		readAsDataURL(blob: Blob): void {
			blob.arrayBuffer().then((buffer) => {
				let binary = '';
				for (const byte of Array.from(new Uint8Array(buffer)))
					binary += String.fromCharCode(byte);
				this.result = 'data:' + blob.type + ';base64,' + btoa(binary);
				if (this.onload) this.onload();
			});
		}
	}

	// Loads an image some time later, as the browser does under the Content-Security-Policy
	// of the editor page: an image comes only from a data: URL, and only PNG data decodes.
	class ImageStandIn {
		onload: (() => void) | null = null;
		onerror: (() => void) | null = null;
		complete = false;
		private _src = '';

		get src(): string {
			return this._src;
		}

		set src(url: string) {
			this._src = url;
			const decodes = /^data:image\/png;base64,iVBORw0KGgo/.test(url);
			Promise.resolve().then(() => {
				this.complete = true;
				if (decodes) {
					if (this.onload) this.onload();
				} else if (this.onerror) this.onerror();
			});
		}
	}

	let savedFileReader: any;
	let savedImage: any;
	beforeEach(function () {
		savedFileReader = (globalThis as any).FileReader;
		savedImage = (globalThis as any).Image;
		(globalThis as any).FileReader = FileReaderStandIn;
		(globalThis as any).Image = ImageStandIn;
	});
	afterEach(function () {
		(globalThis as any).FileReader = savedFileReader;
		(globalThis as any).Image = savedImage;
	});

	// A loader whose page S0 shows the image with checksum 7. The promise settles once the
	// image has decoded or turned out to be unavailable, and holds the owners told of the load.
	function loaderWithImage(): {
		loader: VectorResourceLoader;
		settled: Promise<Set<string>[]>;
	} {
		const told: Set<string>[] = [];
		let settle: (told: Set<string>[]) => void = () => {};
		const settled = new Promise<Set<string>[]>((resolve) => {
			settle = resolve;
		});
		const loader = new VectorResourceLoader(
			(owners) => {
				if (owners) told.push(owners);
			},
			() => settle(told),
		);
		SocketRecorder.during(() =>
			loader.add('S0', (walker) =>
				walker.walkPrimitives([
					{ type: 'bitmap', checksum: 7, matrix: [10, 0, 0, 10, 0, 0] } as any,
				]),
			),
		);
		return { loader: loader, settled: settled };
	}

	it('shows an image the engine sends as the bytes of its file', function () {
		const { loader, settled } = loaderWithImage();
		const bytes = new Uint8Array(pngSignature.concat([0, 0, 0, 13]));

		loader.handleImage({ checksum: 7, mime: 'image/png', bytes: bytes } as any);

		return settled.then((told) => {
			const image = loader.image(7);
			nodeassert.ok(image, 'the image is held once its bytes decode');
			nodeassert.strictEqual(
				image.src,
				'data:image/png;base64,' +
					btoa(String.fromCharCode(...Array.from(bytes))),
			);
			nodeassert.deepStrictEqual(
				told.map((owners) => Array.from(owners)),
				[['S0']],
				'the page that shows the image draws again',
			);
		});
	});

	it('gives up an image whose bytes do not decode, and does not ask for it again', function () {
		const { loader, settled } = loaderWithImage();

		loader.handleImage({
			checksum: 7,
			mime: 'image/png',
			bytes: new Uint8Array([1, 2, 3]),
		} as any);

		return settled.then((told) => {
			nodeassert.deepStrictEqual(told, [], 'no page is told of a load');
			const sent = SocketRecorder.during(() =>
				nodeassert.strictEqual(loader.image(7), undefined),
			);
			nodeassert.deepStrictEqual(sent, []);
		});
	});
});
