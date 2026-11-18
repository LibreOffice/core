// @ts-strict-ignore
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

class PaneBorder {
	_border: any;
	_xFixed: boolean;
	_yFixed: boolean;
	_index: number;

	constructor(paneBorder: any, paneXFixed: boolean, paneYFixed: boolean) {
		this._border = paneBorder;
		this._xFixed = paneXFixed;
		this._yFixed = paneYFixed;
		this._index = 0;
	}

	getBorderIndex() {
		return this._index;
	}

	incBorderIndex() {
		this._index += 1;
	}

	getBorderBounds() {
		return this._border;
	}

	isXFixed() {
		return this._xFixed;
	}

	isYFixed() {
		return this._yFixed;
	}
}
