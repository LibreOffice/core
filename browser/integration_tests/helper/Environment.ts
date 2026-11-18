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

import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';

/// The WebSocket URL of the coolwsd the tests talk to.
export const BASE_URL = process.env.COOLWSD_URL || 'ws://localhost:9980';

/// The folder of the sample documents. The default is test/samples of a tree built in place,
/// from the workdir the tests are compiled into.
const SAMPLES_DIR =
	process.env.COOL_TEST_SAMPLES ||
	path.resolve(__dirname, '..', '..', '..', '..', 'test', 'samples');

/// The folder the documents are copied to before they load. The default is a new folder in the
/// temporary folder of the system.
function documentsDir(): string {
	const dir = process.env.COOL_TEST_WORKDIR;
	if (!dir) return fs.mkdtempSync(path.join(os.tmpdir(), 'cool-integration-'));
	fs.mkdirSync(dir, { recursive: true });
	return dir;
}

/// The file:// URL of a fresh copy of a sample document. coolwsd saves a modified document when
/// its last session closes, so every load gets its own copy and the sample stays as it is.
/// COOL_TEST_DOC, when set, names the document every test copies instead.
export function sampleUrl(name: string): string {
	const source = process.env.COOL_TEST_DOC || path.join(SAMPLES_DIR, name);
	const copy = path.join(
		fs.mkdtempSync(path.join(documentsDir(), 'document-')),
		path.basename(source),
	);
	fs.copyFileSync(source, copy);
	return 'file://' + copy;
}
