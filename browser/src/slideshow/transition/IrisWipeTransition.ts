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

function IrisWipeTransition(transitionParameters: TransitionParameters) {
	const transitionSubType =
		transitionParameters.transitionFilterInfo.transitionSubtype;
	if (transitionSubType == TransitionSubType.DIAMOND) {
		return new SlideShow.DiamondTransition(transitionParameters);
	} else {
		return new SlideShow.BoxTransition(transitionParameters);
	}
}

SlideShow.IrisWipeTransition = IrisWipeTransition;
