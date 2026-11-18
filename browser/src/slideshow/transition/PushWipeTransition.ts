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

declare var SlideShow: any;

function PushWipeTransition(transitionParameters: TransitionParameters) {
	const transitionSubType =
		transitionParameters.transitionFilterInfo.transitionSubtype;

	if (
		transitionSubType == TransitionSubType.COMBHORIZONTAL ||
		transitionSubType == TransitionSubType.COMBVERTICAL
	) {
		return new SlideShow.CombTransition(transitionParameters);
	} else {
		return new SlideShow.PushTransition(transitionParameters);
	}
}

SlideShow.PushWipeTransition = PushWipeTransition;
