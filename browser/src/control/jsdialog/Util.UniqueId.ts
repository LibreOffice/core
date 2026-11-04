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

/*
 * Util.MakeIdUnique - generates unique id for a DOM node
 */

declare var JSDialog: any;

let counter = 0;

function makeIdUnique(id: string) {
	counter++;
	while (document.getElementById(id + counter)) counter++;
	return id + counter;
}

JSDialog.MakeIdUnique = makeIdUnique;
