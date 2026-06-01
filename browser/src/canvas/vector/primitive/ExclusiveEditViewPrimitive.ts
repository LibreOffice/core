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
	/// Wrapper for content that exists only in an exclusive edit
	/// view, for example inline editing handles.
	export interface ExclusiveEditViewPrimitive extends Primitive {
		type: typeof ExclusiveEditViewPrimitive.type;
	}

	export namespace ExclusiveEditViewPrimitive {
		export const type = 'exclusiveEditView';
	}
}
