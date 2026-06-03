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

/// Image mock recorder. Install it as a stand-in for the global
/// Image constructor in tests. The src setter captures the data
/// URL, the image reports complete immediately and onload fires
/// synchronously when assigned. naturalWidth and naturalHeight are
/// fixed dummy values.
class ImageRecorder {
	public src: string = '';
	public readonly complete: boolean = true;
	public readonly naturalWidth: number = 100;
	public readonly naturalHeight: number = 100;

	private _onload: (() => void) | null = null;

	set onload(handler: (() => void) | null) {
		this._onload = handler;
		if (handler) handler();
	}

	get onload(): (() => void) | null {
		return this._onload;
	}
}
