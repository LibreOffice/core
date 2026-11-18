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
	/// One font file. fontId is the hex string text portions carry.
	/// The file comes either as data, plain base64, or as bytes, the
	/// file itself. Both are absent when the engine does not hold the id.
	export interface VectorRenderingFontResponse {
		fontId: string;
		data?: string;
		bytes?: Uint8Array;
	}
}
