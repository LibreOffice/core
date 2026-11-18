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

function SlideWipeTransition(transitionParameters: TransitionParameters) {
	if (transitionParameters.transitionFilterInfo.isDirectionForward) {
		return new SlideShow.CoverTransition(transitionParameters);
	} else {
		return new SlideShow.UncoverTransition(transitionParameters);
	}
}

SlideShow.SlideWipeTransition = SlideWipeTransition;
