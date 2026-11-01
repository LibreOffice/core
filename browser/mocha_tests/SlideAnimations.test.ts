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

describe('AnimationCommandNode', function () {
	// The commands the videos received, in order.
	let requests: string[];

	// The handler the renderer of each video comes from.
	const slideShowHandler = {
		getVideoRenderer: function (slideHash: string, videoInfo: any) {
			return {
				playFrom: (seconds: number) =>
					requests.push(slideHash + ' play ' + videoInfo.id + ' at ' + seconds),
				handleClick: () => requests.push(slideHash + ' toggle ' + videoInfo.id),
				stop: () => requests.push(slideHash + ' stop ' + videoInfo.id),
			};
		},
	};

	beforeEach(function () {
		requests = [];
	});

	function runCommand(
		command: string,
		target: string,
		mediaTime?: number,
	): void {
		const context: any = {
			_context: {
				aTimerEventQueue: { addEvent: function () {} },
				aSlideShowHandler: slideShowHandler,
			},
			nStartDelay: 0,
			aAnimationNodeMap: new Map(),
			metaSlide: {
				info: {
					hash: 'slide',
					videos: [{ id: 11 }, { id: 12 }],
				},
			},
		};
		const node: any = new AnimationCommandNode(
			{
				nodeName: 'Command',
				dur: '0.001s',
				command,
				targetElement: target,
				mediaTime,
			} as any,
			null,
			context,
		);
		node.parseNodeInfo();
		node.activate_st();
	}

	it('plays, pauses and stops the video that its target names', function () {
		runCommand('Play', '12');
		runCommand('TogglePause', '12');
		runCommand('Stop', '11');

		nodeassert.deepStrictEqual(requests, [
			'slide play 12 at 0',
			'slide toggle 12',
			'slide stop 11',
		]);
	});

	it('starts the video at the time that a play gives', function () {
		runCommand('Play', '11', 2.5);

		nodeassert.deepStrictEqual(requests, ['slide play 11 at 2.5']);
	});

	it('leaves the videos alone when its target is not a video', function () {
		runCommand('Play', '13');

		nodeassert.deepStrictEqual(requests, []);
	});
});
