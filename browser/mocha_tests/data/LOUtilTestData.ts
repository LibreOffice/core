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

class LOUtilTestData {
	public static tree = {
		id: 'level0',
		children: [
			{
				id: 'level1_1',
				children: [
					{
						id: 'level2',
						children: [
							{
								id: 'level3_0',
								data: 4321,
							},
							{
								id: 'level3',
								children: [
									{
										id: 'level4',
										data: 1234,
									},
								],
							},
						],
					},
				],
			},
			{
				id: 'level1_2',
			},
		],
	};

	public static shortTree = {
		id: 'level0',
		children: [
			{ class: 'abc', name: 'ABC' },
			{ class: 'abc', name: 'ABC1' },
			{ class: 'bac', name: 'BAC' },
			{ class: 'bac', name: 'BAC1' },
			{ class: 'cab', name: 'CAB' },
		],
	};
}
