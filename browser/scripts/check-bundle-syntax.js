#!/usr/bin/env node
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Parse the assembled JS bundle at a fixed ECMAScript version to
// catch syntax newer than the minifier (terser) can handle. Fails
// with line/column on parse error.

const fs = require('fs');
const acorn = require('acorn');

const ECMA_VERSION = 2022;
const bundlePath = process.argv[2];

if (!bundlePath) {
	console.error('Usage: check-bundle-syntax.js <bundle.js>');
	process.exit(2);
}

try {
	acorn.parse(fs.readFileSync(bundlePath, 'utf8'), {
		ecmaVersion: ECMA_VERSION,
		sourceType: 'script',
	});
} catch (e) {
	console.error(
		`Bundle parse failed at line ${e.loc.line}, column ${e.loc.column}: ${e.message}`,
	);
	console.error(
		`A dep or hand-written JS uses syntax newer than ES${ECMA_VERSION}, which terser cannot parse. Check recent package.json updates.`,
	);
	process.exit(1);
}
