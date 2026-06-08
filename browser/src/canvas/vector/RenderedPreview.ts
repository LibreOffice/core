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

namespace cool {
	/// Snapshot of a thumbnail render. part is the slide the preview
	/// points to, maxWidth and maxHeight are the size the consumer
	/// asked for at the last render in CSS pixels.
	export interface RenderedPreview {
		part: number;
		maxWidth: number;
		maxHeight: number;
	}
}
