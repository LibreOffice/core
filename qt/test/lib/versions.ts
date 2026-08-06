/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { existsSync, readdirSync, statSync } from 'fs';
import { join } from 'path';

export function versionsDir(): string {
	return process.env.CODA_TEST_VERSIONS_DIR!;
}

/** Every version file in the store (meta.json excluded), as full paths. */
export function allVersionFiles(): string[] {
	const root = versionsDir();
	if (!existsSync(root)) return [];
	const files: string[] = [];
	for (const entry of readdirSync(root)) {
		const dirPath = join(root, entry);
		if (!statSync(dirPath).isDirectory()) continue;
		for (const name of readdirSync(dirPath)) {
			if (name !== 'meta.json') files.push(join(dirPath, name));
		}
	}
	return files;
}
