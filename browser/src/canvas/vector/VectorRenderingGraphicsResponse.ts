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
	/// value carried by bitmap primitives in a vector tile. data is
	/// a base64 data URL with a native MIME type (image/png,
	/// image/jpeg, image/svg+xml, image/gif or image/webp).
	export interface VectorRenderingGraphicsResponse {
		checksum: number;
		data: string;
	}
}
