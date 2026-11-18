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

/// <reference path="./types.ts" />

function assertPosSize(actual: mtest.Rectangle, expected: mtest.Rectangle) {
	// Only assert components of expected that are provided.
	if (typeof expected.x === 'number')
		nodeassert.equal(actual.x, expected.x, 'Left mismatch');
	if (typeof expected.y === 'number')
		nodeassert.equal(actual.y, expected.y, 'Top mismatch');
	if (typeof expected.width === 'number')
		nodeassert.equal(actual.width, expected.width, 'Width mismatch');
	if (typeof expected.height === 'number')
		nodeassert.equal(actual.height, expected.height, 'Height mismatch');
}

function getSectionRectangle(section: CanvasSectionObject): mtest.Rectangle {
	return {
		x: section.myTopLeft[0],
		y: section.myTopLeft[1],
		width: section.size[0],
		height: section.size[1],
	};
}
