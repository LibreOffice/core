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

/// Load a wire-format JSON reference written by the engine test
/// CppunitTest_sd_vector_rendering into the engine workdir.
function loadVectorRenderingReference(name: string): any {
	const fs = require('fs');
	const path = require('path');
	const engineWorkdir =
		process.env.ENGINE_WORKDIR ||
		path.join(__dirname, '..', '..', '..', 'engine', 'workdir');
	const referencePath = path.join(
		engineWorkdir,
		'VectorRenderingReference',
		name + '.json',
	);
	let json: string;
	try {
		json = fs.readFileSync(referencePath, 'utf8');
	} catch (error) {
		if (error.code !== 'ENOENT') throw error;
		throw new Error(
			'Missing vector rendering reference ' +
				referencePath +
				'. The engine test that writes these files has not run yet. ' +
				'Run "make CppunitTest_sd_vector_rendering" in the engine ' +
				'directory, or point ENGINE_WORKDIR at an engine workdir ' +
				'that already has them, then re-run the mocha tests.',
		);
	}
	return JSON.parse(json);
}
