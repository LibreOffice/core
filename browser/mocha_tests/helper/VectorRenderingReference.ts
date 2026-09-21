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

/// The folder holding the wire-format JSON references written by the engine
/// test CppunitTest_sd_vector_rendering. Nothing in the online build writes
/// them, so a tree that has never run that test has no folder at all.
function vectorRenderingReferenceDir(): string {
	const path = require('path');
	const engineWorkdir =
		process.env.ENGINE_WORKDIR ||
		path.join(__dirname, '..', '..', '..', 'engine', 'workdir');
	return path.join(engineWorkdir, 'VectorRenderingReference');
}

/// The test on its way. Mocha names it to a hook alone, while the loader
/// below is called from the body of the test, so the root hook keeps it here
/// for the loader to mark pending.
let currentVectorReferenceTest: Mocha.Test | undefined;

beforeEach(function () {
	currentVectorReferenceTest = this.currentTest;
});

/// Said once, however many tests go pending for the same missing folder.
let saidReferencesAreMissing = false;

/// Mark the running test pending. A tree that has never run the engine test
/// cannot answer these tests at all, and failing them would report a
/// regression where there is only a prerequisite nobody has built yet.
function skipWithoutVectorRenderingReferences(directory: string): never {
	if (!saidReferencesAreMissing) {
		saidReferencesAreMissing = true;
		console.log(
			'\nNo vector rendering references under ' +
				directory +
				', so the tests\nthat read them are pending. Run "make ' +
				'CppunitTest_sd_vector_rendering" in the\nengine directory, or ' +
				'point ENGINE_WORKDIR at an engine workdir that already\nhas ' +
				'them, to run these tests.\n',
		);
	}
	if (!currentVectorReferenceTest)
		throw new Error(
			'loadVectorRenderingReference called outside a test: ' +
				'there is nothing to mark pending',
		);
	return currentVectorReferenceTest.skip();
}

/// Load a wire-format JSON reference written by the engine test
/// CppunitTest_sd_vector_rendering into the engine workdir. With no such
/// reference anywhere the test goes pending; with the others in place but
/// this one absent the engine test ran and left it out, which is a fault.
function loadVectorRenderingReference(name: string): any {
	const fs = require('fs');
	const path = require('path');
	const directory = vectorRenderingReferenceDir();
	const referencePath = path.join(directory, name + '.json');
	let json: string;
	try {
		json = fs.readFileSync(referencePath, 'utf8');
	} catch (error) {
		if (error.code !== 'ENOENT') throw error;
		if (!fs.existsSync(directory))
			skipWithoutVectorRenderingReferences(directory);
		throw new Error(
			'Missing vector rendering reference ' +
				referencePath +
				'. The engine test that writes these files has run, since ' +
				'the folder is there, but it wrote no reference under this ' +
				'name. Re-run "make CppunitTest_sd_vector_rendering" in the ' +
				'engine directory and check that it passes.',
		);
	}
	return JSON.parse(json);
}
