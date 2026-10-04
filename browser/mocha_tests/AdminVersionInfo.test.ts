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

describe('AdminVersionInfo', function () {
	const warning = "Warning: the installed packages don't match";
	const mismatch =
		'the office library is [26.04.6.0] but coolwsd is [26.04.7.0]';

	it('shows the build configuration alone when the versions match', function () {
		nodeassert.strictEqual(
			AdminVersionInfo.buildConfigText(
				{ BuildConfig: '--enable-foo' },
				warning,
			),
			'--enable-foo',
		);
	});

	it('shows nothing when there is neither a build configuration nor a difference', function () {
		nodeassert.strictEqual(AdminVersionInfo.buildConfigText({}, warning), '');
	});

	it('shows the difference after the build configuration', function () {
		nodeassert.strictEqual(
			AdminVersionInfo.buildConfigText(
				{ BuildConfig: '--enable-foo', Mismatch: mismatch },
				warning,
			),
			'--enable-foo (' + warning + ': ' + mismatch + ')',
		);
	});

	it('shows the difference alone when the build configuration was not recorded', function () {
		nodeassert.strictEqual(
			AdminVersionInfo.buildConfigText({ Mismatch: mismatch }, warning),
			warning + ': ' + mismatch,
		);
	});
});
