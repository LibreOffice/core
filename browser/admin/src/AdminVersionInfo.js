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
/*
 * The text shown in the admin console for the version information of the office library
 */

/* exported AdminVersionInfo */

var AdminVersionInfo = {
	// The build configuration of the office library, followed by the warning about how its
	// version differs from coolwsd. versionInfo is the JSON of the 'lokitversion' message, where
	// BuildConfig and Mismatch are both optional. warning is the translated label of the warning.
	buildConfigText: function (versionInfo, warning) {
		var buildConfig = versionInfo.BuildConfig || '';
		if (!versionInfo.Mismatch) {
			return buildConfig;
		}

		var mismatch = warning + ': ' + versionInfo.Mismatch;
		if (!buildConfig) {
			return mismatch;
		}

		return buildConfig + ' (' + mismatch + ')';
	},
};
