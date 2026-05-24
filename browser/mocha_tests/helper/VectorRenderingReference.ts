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

/// Load a wire-format JSON reference written by the engine.
/// If the file is missing, the engine test must be run first
/// (make CppunitTest_sd_vector_rendering).
function loadVectorRenderingReference(name: string): any {
	const fs = require('fs');
	const path = require('path');
	const referencePath = path.join(
		__dirname,
		'..',
		'..',
		'..',
		'engine',
		'workdir',
		'VectorRenderingReference',
		name + '.json',
	);
	return JSON.parse(fs.readFileSync(referencePath, 'utf8'));
}
