/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Runs as its own suite (run-tests.sh --suite versions) with
// CODA_TEST_SEED_VERSIONS=1, so the service plants version-store
// contents before coda-qt starts - the on-disk state a forced exit
// leaves behind.

import { existsSync } from 'fs';
import { join } from 'path';

import * as versions from '../lib/versions.js';

describe('Version store startup pruning', () => {
	const docDir = () => join(versions.versionsDir(), '00c0ffee00c0ffee');

	it('sweeps the stale copy temporary and the stray root file', async () => {
		await browser.webEngine.waitUntil(
			async () =>
				!existsSync(join(docDir(), '20200102-030405.odtAbCdEfGhIjKl')) &&
				!existsSync(join(versions.versionsDir(), 'stray.txt')),
			{
				timeout: 30000,
				timeoutMsg: 'the startup sweep did not run',
			},
		);
	});

	it('keeps the version, its meta.json and the fresh temporary', async () => {
		expect(existsSync(join(docDir(), '20200102-030405.odt'))).toBe(true);
		expect(existsSync(join(docDir(), 'meta.json'))).toBe(true);
		expect(existsSync(join(docDir(), '20260101-000000.odtMnOpQrStUvWx'))).toBe(
			true,
		);
	});
});
