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

namespace cool {
	/// Snapshot of a thumbnail render. partId is the page the preview shows,
	/// mode the page list it belongs to, maxWidth and maxHeight the size
	/// requested at the last render in CSS pixels.
	export interface RenderedPreview {
		partId: VectorPartGuid;
		mode: number;
		maxWidth: number;
		maxHeight: number;
	}
}
