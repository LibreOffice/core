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

describe('SlideAnimations', function () {
	it('plays the nodes under an iterate container on its target', function () {
		const iterate: any = {
			nodeName: 'Iterate',
			targetElement: 'shape',
			subItem: 'OnlyText',
			children: [
				{ nodeName: 'Set', attributeName: 'Visibility' },
				{
					nodeName: 'Par',
					children: [{ nodeName: 'Animate', attributeName: 'Width' }],
				},
			],
		};

		inheritIterateTarget(iterate, iterate.targetElement, iterate.subItem);

		nodeassert.strictEqual(iterate.children[0].targetElement, 'shape');
		nodeassert.strictEqual(iterate.children[0].subItem, 'OnlyText');
		const animate = iterate.children[1].children[0];
		nodeassert.strictEqual(animate.targetElement, 'shape');
		nodeassert.strictEqual(animate.subItem, 'OnlyText');
	});

	it('keeps the target that a node under an iterate container names itself', function () {
		const iterate: any = {
			nodeName: 'Iterate',
			targetElement: 'shape',
			children: [
				{
					nodeName: 'Set',
					attributeName: 'Visibility',
					targetElement: 'other',
				},
			],
		};

		inheritIterateTarget(iterate, iterate.targetElement, iterate.subItem);

		nodeassert.strictEqual(iterate.children[0].targetElement, 'other');
	});
});
