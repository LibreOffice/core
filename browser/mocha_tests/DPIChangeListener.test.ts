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

describe('DPIChangeListener', function () {
	afterEach(function () {
		restoreMatchMedia();
		setDevicePixelRatio(1);
	});

	it('registers a change listener at the current density on start', function () {
		installFakeMatchMedia();
		setDevicePixelRatio(1.5);

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();

		nodeassert.strictEqual(fakeMediaQueries.length, 1);
		nodeassert.strictEqual(fakeMediaQueries[0].media, '(resolution: 1.5dppx)');
		nodeassert.strictEqual(fakeMediaQueries[0].listenerCount, 1);
		// The callback only runs on a density change, not on registration.
		nodeassert.strictEqual(fired, 0);
	});

	it('does nothing and does not throw when matchMedia is unavailable', function () {
		(window as any).matchMedia = undefined;

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});

		nodeassert.doesNotThrow(() => listener.start());
		nodeassert.doesNotThrow(() => listener.stop());
		nodeassert.strictEqual(fired, 0);
	});

	it('runs the callback when the density changes', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();
		fakeMediaQueries[0].fireChange();

		nodeassert.strictEqual(fired, 1);
	});

	it('re-registers at the new density after a change', function () {
		installFakeMatchMedia();
		setDevicePixelRatio(1);

		const listener = new DPIChangeListener(() => {});
		listener.start();
		nodeassert.strictEqual(fakeMediaQueries[0].media, '(resolution: 1dppx)');

		setDevicePixelRatio(2);
		fakeMediaQueries[0].fireChange();

		nodeassert.strictEqual(fakeMediaQueries.length, 2);
		nodeassert.strictEqual(fakeMediaQueries[1].media, '(resolution: 2dppx)');
		nodeassert.strictEqual(fakeMediaQueries[1].listenerCount, 1);
		// The stale query no longer holds a listener.
		nodeassert.strictEqual(fakeMediaQueries[0].listenerCount, 0);
	});

	it('fires the callback only once per density change', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();

		const stale = fakeMediaQueries[0];
		stale.fireChange();
		// The one-shot listener has moved to the re-registered query, so firing
		// the stale query again delivers nothing.
		stale.fireChange();

		nodeassert.strictEqual(fired, 1);
	});

	it('keeps tracking across several successive density changes', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();

		for (let i = 0; i < 3; i++) {
			fakeMediaQueries[fakeMediaQueries.length - 1].fireChange();
		}

		nodeassert.strictEqual(fired, 3);
		// One initial registration plus one per change.
		nodeassert.strictEqual(fakeMediaQueries.length, 4);
	});

	it('stops delivering the callback after stop()', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();
		const query = fakeMediaQueries[0];

		listener.stop();
		nodeassert.strictEqual(query.listenerCount, 0);

		query.fireChange();
		nodeassert.strictEqual(fired, 0);
	});

	it('tolerates stop() before start() and repeated stop()', function () {
		installFakeMatchMedia();

		const listener = new DPIChangeListener(() => {});
		nodeassert.doesNotThrow(() => listener.stop());

		listener.start();
		nodeassert.doesNotThrow(() => {
			listener.stop();
			listener.stop();
		});
	});

	it('does not leak the previous listener when start() is called twice', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();
		listener.start();

		nodeassert.strictEqual(fakeMediaQueries.length, 2);
		nodeassert.strictEqual(fakeMediaQueries[0].listenerCount, 0);
		nodeassert.strictEqual(fakeMediaQueries[1].listenerCount, 1);

		// The stale registration is inert, the current one is live.
		fakeMediaQueries[0].fireChange();
		nodeassert.strictEqual(fired, 0);
		fakeMediaQueries[1].fireChange();
		nodeassert.strictEqual(fired, 1);
	});

	it('can be restarted after being stopped', function () {
		installFakeMatchMedia();

		let fired = 0;
		const listener = new DPIChangeListener(() => {
			fired++;
		});
		listener.start();
		listener.stop();
		listener.start();

		fakeMediaQueries[fakeMediaQueries.length - 1].fireChange();
		nodeassert.strictEqual(fired, 1);
	});
});
