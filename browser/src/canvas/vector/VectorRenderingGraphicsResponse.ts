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
	/// One bitmap delivered by the engine. The checksum matches the
	/// value carried by bitmap primitives in the primitive tree. data is
	/// a base64 data URL with a native MIME type (image/png,
	/// image/jpeg, image/svg+xml, image/gif or image/webp). When there
	/// is no data, error carries a token naming why: 'notfound' when
	/// the engine has no graphic for the checksum, 'conversionfailed'
	/// when the graphic could not be encoded to an image format.
	export interface VectorRenderingGraphicsResponse {
		checksum: number;
		data?: string;
		error?: string;
	}
}
