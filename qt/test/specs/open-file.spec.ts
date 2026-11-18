/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { openFixture } from '../lib/file-dialog.js';

describe('Open existing file', () => {
	it('should open a .odt file via the native file dialog', async function () {
		await openFixture(browser.webEngine, browser.native, 'simple.odt');

		const state = await browser.webEngine.execute(() => ({
			docLoaded: app.map._docLoaded,
			docType: app.map.getDocType(),
		}));

		expect(state.docLoaded).toBe(true);
		expect(state.docType).toBe('text');
	});
});
