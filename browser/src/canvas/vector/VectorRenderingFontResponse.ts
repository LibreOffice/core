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
	/// One font file. fontId is the hex string text portions carry.
	/// data is the whole font file as plain base64, absent when the
	/// engine does not hold the id.
	export interface VectorRenderingFontResponse {
		fontId: string;
		data?: string;
	}
}
