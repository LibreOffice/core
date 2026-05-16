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
	/// Return true if the vector rendering should be enabled
	export namespace VectorRenderingConfig {
		export function isEnabled(): boolean {
			return window.coolParams.get('vector') === 'true';
		}
	}
}
