/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { openFixture } from '../lib/file-dialog.js';

// Cross-window slide copy/paste between two in-process Impress documents (one
// kit). Cases drive copy/paste via the bridge messages and the slide-sorter
// context-menu callbacks, so they run headless under weston. Oracle: a paste must
// increment the target's slide count (app.map._docLayer._parts), so every wait
// polls that rather than sleeping a fixed time.

type Mechanism = 'contextmenu' | 'bridge';

const we = (): WebdriverIO.Browser & {
	waitForCondition(
		predicate: () => boolean,
		opts?: { timeout?: number; interval?: number; timeoutMsg?: string },
	): Promise<boolean>;
} => browser.webEngine;

// Source: slide_navigation.odp (14 slides). Target: empty-presentation.odp
// (2 slides; a deterministic "new document" stand-in). The target accumulates
// slides across cases - every assertion uses a freshly captured baseline.
let handleA: string;
let handleB: string;

async function switchTo(handle: string): Promise<void> {
	await we().switchToWindow(handle);
}

async function partsCount(): Promise<number> {
	return we().execute(() => app.map._docLayer._parts);
}

// Exactly one img.preview-img per slide (the leading #first-drop-site frame
// carries no preview img), so this equals the slide count once previews sync.
async function previewImgCount(): Promise<number> {
	return we().execute(
		() => document.querySelectorAll('#slide-sorter img.preview-img').length,
	);
}

async function selectedSlidesCount(): Promise<number> {
	return we().execute(() => app.impress.getSelectedSlidesCount());
}

// Leave coda-qt's read-only view mode so editing commands are enabled.
async function enterEditMode(): Promise<void> {
	await we().execute(() => {
		if (!app.map.isEditMode()) app.map._enterEditMode('edit');
	});
	await we().waitUntil(async () => we().execute(() => app.map.isEditMode()), {
		timeout: 15000,
		interval: 200,
		timeoutMsg: 'document did not enter edit mode',
	});
}

// Let the active window settle before driving a copy or paste. Driving slide
// operations while preview/layout tasks are still in flight races the kit's
// view bookkeeping and degrades .uno:CopySlide from a slide copy into an
// object copy. A momentary idle is not enough (preview generation drains and
// refills), so wait for the layouting service to stay idle for several
// consecutive polls.
async function waitForIdle(): Promise<void> {
	let stableIdlePolls = 0;
	try {
		await we().waitUntil(
			async () => {
				const idle = await we().execute(
					() => !app.layoutingService.hasTasksPending(),
				);
				stableIdlePolls = idle ? stableIdlePolls + 1 : 0;
				return stableIdlePolls >= 5;
			},
			{ timeout: 30000, interval: 200 },
		);
	} catch {
		// Best effort: proceed even if tasks never fully drain.
		console.warn(
			'waitForIdle: layouting tasks did not drain within 30s; proceeding anyway',
		);
	}
}

// Select slides 0..n-1 in the active window's slide sorter. selectPart with
// external=false notifies the server (selectclientpart), so the server-side
// slide-sorter selection that .uno:CopySlide acts on is established too.
async function selectFirstNSlides(n: number): Promise<void> {
	await we().execute((count: number) => {
		app.map.deselectAll();
		app.map.setPart(0);
		for (let i = 0; i < count; i++) app.map.selectPart(i, 1, false);
	}, n);
	await we().waitUntil(async () => (await selectedSlidesCount()) === n, {
		timeout: 10000,
		interval: 200,
		timeoutMsg: `source did not reach ${n} selected slides`,
	});
}

// Put the target's slide sorter into a slide-paste context: select its last
// slide (so inserted slides append at the end) and mark the clip selection type
// 'slide' so .uno:Paste inserts slides rather than text into a slide body.
async function prepareTargetSelection(): Promise<void> {
	await we().execute(() => {
		const last = app.map._docLayer._parts - 1;
		app.map.deselectAll();
		app.map.setPart(last);
		app.map.selectPart(last, 1, false);
		app.map._clip.setTextSelectionType('slide');
	});
}

// Proxy for the copy's COMMANDRESULT: the bridge shows an infinite progress
// snackbar on COPYSLIDE and closes it the moment the copy command completes -
// copy is lazy, so this is near-instant and serialises nothing here. Wait for it
// to appear (best effort - a fast copy may close it before we observe it), then
// wait for it to close. Deliberately does NOT also wait for the unsynchronised
// CLIPBOARDMIMETYPES (installing the lazy clipboard) - racing that is part of
// what we want to catch.
async function waitCopyAcknowledged(): Promise<void> {
	const snackbarPresent = () =>
		!!document.querySelector(
			'#snackbar-container-progress, #snackbar-container, .snackbar',
		);
	try {
		await we().waitUntil(async () => we().execute(snackbarPresent), {
			timeout: 5000,
			interval: 100,
		});
	} catch {
		// Copy completed before the snackbar was observable; fall through.
	}
	await we().waitUntil(async () => !(await we().execute(snackbarPresent)), {
		timeout: 30000,
		interval: 200,
		timeoutMsg: 'copy progress snackbar did not close (no COMMANDRESULT)',
	});
}

async function copySlides(mechanism: Mechanism): Promise<void> {
	switch (mechanism) {
		case 'contextmenu':
			// The exact slide-sorter context-menu "Copy" callback.
			await we().execute(() => {
				app.map._clip.setTextSelectionType('slide');
				app.map._clip._execCopyCutPaste('copy', '.uno:CopySlide');
			});
			break;
		case 'bridge':
			await we().execute(() => {
				window.postMobileMessage('COPYSLIDE');
			});
			break;
	}
}

async function pasteSlides(
	mechanism: Mechanism,
	anchorParts: number,
): Promise<void> {
	switch (mechanism) {
		case 'contextmenu':
			// The exact slide-sorter context-menu "Paste" callback. _pasteSlide
			// is async (it probes navigator.clipboard first, which rejects in an
			// unfocused window and falls back to the native PASTE bridge path);
			// fire it and let the parts-delta oracle wait for the result.
			await we().execute((nPos: number) => {
				void app.map._docLayer._preview._pasteSlide(nPos);
			}, anchorParts);
			break;
		case 'bridge':
			await we().execute(() => {
				window.postMobileMessage('PASTE');
			});
			break;
	}
}

// Wait for the target to gain exactly k slides, then assert. The wait cannot
// mask the bug (a never-incrementing _parts just times out); the assert guards
// against overshoot.
async function expectSlidesAdded(
	beforeParts: number,
	beforeImgs: number,
	k: number,
): Promise<void> {
	await we().waitUntil(
		async () => (await partsCount()) === beforeParts + k,
		{
			timeout: 30000,
			interval: 500,
			timeoutMsg: `paste did not add ${k} slide(s): _parts never reached ${beforeParts + k} (was ${beforeParts})`,
		},
	);
	expect(await partsCount()).toBe(beforeParts + k);

	// Previews sync a moment after _parts; assert the sorter rendered them too.
	await we().waitUntil(
		async () => (await previewImgCount()) === beforeImgs + k,
		{
			timeout: 10000,
			interval: 250,
			timeoutMsg: `slide sorter previews did not grow by ${k}`,
		},
	);
	expect(await previewImgCount()).toBe(beforeImgs + k);
}

interface CrossDocOpts {
	source: string;
	target: string;
	n: number;
	mechanism: Mechanism;
	waitAck: boolean;
	expectAdded?: number;
}

// Copy n slides from source, paste into target, assert the target grew by
// expectAdded (default n). The target is prepared and its baseline snapshotted
// up front, before the source copy, so only a single window switch separates
// the copy from the paste.
async function crossDocCopyPaste(opts: CrossDocOpts): Promise<void> {
	const k = opts.expectAdded ?? opts.n;

	await switchTo(opts.target);
	await waitForIdle();
	await prepareTargetSelection();
	const beforeParts = await partsCount();
	const beforeImgs = await previewImgCount();

	await switchTo(opts.source);
	await waitForIdle();
	await selectFirstNSlides(opts.n);
	await copySlides(opts.mechanism);
	if (opts.waitAck) await waitCopyAcknowledged();

	await switchTo(opts.target);
	await pasteSlides(opts.mechanism, beforeParts);
	await expectSlidesAdded(beforeParts, beforeImgs, k);
}

describe('Cross-window slide copy/paste', () => {
	before(async function () {
		this.timeout(180000);
		handleA = await openFixture(
			browser.webEngine,
			browser.native,
			'slide_navigation.odp',
		);
		handleB = await openFixture(
			browser.webEngine,
			browser.native,
			'empty-presentation.odp',
		);

		// Sanity: both opened as presentations with the expected start counts.
		await switchTo(handleA);
		expect(await partsCount()).toBe(14);
		await switchTo(handleB);
		expect(await partsCount()).toBe(2);

		// Documents open read-only; switch both to edit mode, then let their
		// initial preview/layout generation settle before the first copy.
		await switchTo(handleB);
		await enterEditMode();
		await waitForIdle();
		await switchTo(handleA);
		await enterEditMode();
		await waitForIdle();
	});

	// Each A->B case copies a different slide count, so a degraded copy that
	// pasted the previous case's stale clipboard would land the wrong number of
	// slides and fail the oracle rather than masquerade as a pass.

	// The focus-immune native baseline: postMobileMessage COPYSLIDE / PASTE.
	it('copies 3 slides A->B via the raw bridge messages', async function () {
		await crossDocCopyPaste({
			source: handleA,
			target: handleB,
			n: 3,
			mechanism: 'bridge',
			waitAck: true,
		});
	});

	it('copies 2 slides A->B via the slide-sorter context menu', async function () {
		await crossDocCopyPaste({
			source: handleA,
			target: handleB,
			n: 2,
			mechanism: 'contextmenu',
			waitAck: true,
		});
	});

	// Reverse direction: copy from B (which the A->B cases above left as a paste
	// target) into A. The source (B) is read on the kit thread at paste time, and a
	// paste never switches the kit to a non-active view from the GUI thread, so B's
	// view stays intact and .uno:CopySlide in B copies slides, not an object.
	it('copies 3 slides B->A (reverse direction)', async function () {
		await crossDocCopyPaste({
			source: handleB,
			target: handleA,
			n: 3,
			mechanism: 'bridge',
			waitAck: true,
		});
	});

	// Does the cached snapshot survive a second paste? Copy once in A, paste into
	// B twice; the first paste serialises A once (on the kit thread) and caches
	// it, the second replays the cache. B must grow by k then by 2*k. k differs
	// from the count of the case just above so a stale-clipboard paste would land
	// the wrong number.
	it('pastes twice into B from a single copy in A', async function () {
		const k = 3;

		await switchTo(handleB);
		await waitForIdle();
		await prepareTargetSelection();
		const beforeParts = await partsCount();
		const beforeImgs = await previewImgCount();

		await switchTo(handleA);
		await waitForIdle();
		await selectFirstNSlides(k);
		await copySlides('bridge');
		await waitCopyAcknowledged();

		await switchTo(handleB);
		await pasteSlides('bridge', beforeParts);
		await expectSlidesAdded(beforeParts, beforeImgs, k);

		// Second paste from the same clipboard (source unchanged).
		await prepareTargetSelection();
		await pasteSlides('bridge', beforeParts + k);
		await expectSlidesAdded(beforeParts, beforeImgs, 2 * k);
	});

	// Copying slides must work from a read-only document: copy out of a
	// read-only deck into one you are editing.
	it('copies slides from a read-only document into an edit-mode document', async function () {
		this.timeout(120000);

		// A separate fixture, so opening it gives a new window.
		const readOnlySource = await openFixture(
			browser.webEngine,
			browser.native,
			'readonly-source.odp',
		);
		await switchTo(readOnlySource);
		await waitForIdle();
		expect(await we().execute(() => app.map.isEditMode())).toBe(false);

		await crossDocCopyPaste({
			source: readOnlySource,
			target: handleB,
			n: 2,
			mechanism: 'bridge',
			waitAck: true,
		});
	});
});
