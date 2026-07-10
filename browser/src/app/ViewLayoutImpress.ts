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

// Impress and Draw edit view. A single centred slide/page laid out with the
// inherited single-window machinery.
class ViewLayoutImpress extends ViewLayoutBase {
	public readonly type: string = 'ViewLayoutImpress';

	protected override usesSingleWindowView(): boolean {
		return true;
	}
}
