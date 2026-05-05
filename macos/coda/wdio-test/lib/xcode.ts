/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { execFileSync } from 'child_process';
import { existsSync } from 'fs';
import { join } from 'path';

/**
 * Find the Xcode Debug build of Collabora Office.app for the given
 * coda.xcodeproj.
 *
 * Asks Xcode itself via `xcodebuild -showBuildSettings -json`.  This is
 * the same source Xcode's "Product -> Copy Build Folder Path" uses, so
 * it correctly resolves the per-workspace DerivedData hash.
 */
export function findCodaApp(projectPath: string): string {
	let json: string;
	try {
		json = execFileSync('xcodebuild', [
			'-project', projectPath,
			'-scheme', 'coda',
			'-configuration', 'Debug',
			'-showBuildSettings',
			'-json',
		], { encoding: 'utf8', stdio: ['ignore', 'pipe', 'ignore'] });
	} catch (e) {
		throw new Error(
			`xcodebuild failed for ${projectPath}.\n` +
			`Build the project once in Xcode (Cmd+B) before running the tests, ` +
			`or set CODA_APP explicitly.`,
		);
	}

	const settings = JSON.parse(json);
	const target = settings.find((s: any) => s.target === 'coda')?.buildSettings;
	if (!target?.BUILT_PRODUCTS_DIR || !target?.FULL_PRODUCT_NAME) {
		throw new Error(
			`xcodebuild did not return BUILT_PRODUCTS_DIR / FULL_PRODUCT_NAME ` +
			`for project ${projectPath}.`,
		);
	}

	const app = join(target.BUILT_PRODUCTS_DIR, target.FULL_PRODUCT_NAME);
	if (!existsSync(app)) {
		throw new Error(
			`Xcode reports the app should be at ${app}, but the bundle does ` +
			`not exist there.  Build the Xcode project (Cmd+B) first.`,
		);
	}
	return app;
}
