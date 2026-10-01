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
	/// Shows another page, such as the slide on a notes page. The
	/// children are the content of the shown page. backdrop, when
	/// present, is the fill the shown page holds behind its objects, and
	/// backdropTransparency the share of the document background that
	/// shows through it, from 0 to 1, absent for a fill that covers it
	/// fully. Without a backdrop the objects of the shown page sit on
	/// the document background.
	export interface PagePreviewPrimitive extends Primitive {
		type: typeof PagePreviewPrimitive.type;
		backdrop?: string;
		backdropTransparency?: number;
	}

	export namespace PagePreviewPrimitive {
		export const type = 'pagePreview';
	}
}
