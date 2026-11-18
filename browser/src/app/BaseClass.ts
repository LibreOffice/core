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

/// Minimal base class for Map, Layer etc.

interface BaseClassOptions {
	[name: string]: any;
}

class BaseClass implements IDAble {
	private baseOptions?: BaseClassOptions;
	public _leaflet_id: number = -1;

	constructor(opts?: BaseClassOptions) {
		this.baseOptions = opts;
	}
}
