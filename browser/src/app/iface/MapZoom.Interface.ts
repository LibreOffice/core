/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Subset of MapInterface containing zoom options.
// Needed in ServerCommand and its unit tests.
interface MapZoomInterface {
	_docLayer: {
		options: {
			tileWidthTwips: number;
		};
	};
	options: {
		zoom: number;
	};
}
