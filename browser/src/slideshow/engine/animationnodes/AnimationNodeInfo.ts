/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface AnimationsInfo {
	root: AnimationNodeInfo;
}

interface BaseNodeInfo {
	id: string;
	nodeName: string;
	begin?: string;
	dur?: string;
	end?: string;
	fill?: string;
	fillDefault?: string;
	restart: string;
	restartDefault?: string;
	accelerate?: string;
	decelerate?: string;
	autoreverse?: string;
	repeatCount?: string;
	repeatDur?: string;
	endSync?: string;
	nodeType?: string;
	presetId?: string;
	presetLabel?: string;
	presetSubType?: string;
	presetClass?: string;
	masterElement?: string;
	groupId?: number;
}

interface ContainerNodeInfo extends BaseNodeInfo {
	children?: Array<AnimationNodeInfo>;
}

interface IterateNodeInfo extends ContainerNodeInfo {
	targetElement?: string;
	subItem?: string;
	iterateType?: string;
	iterateInterval?: string;
}

interface AnimateNodeInfo extends BaseNodeInfo {
	targetElement?: string;
	title?: string;
	objectName?: string;
	subItem?: string;
	attributeName?: string;
	values?: string;
	from?: string;
	by?: string;
	to?: string;
	keyTimes?: string;
	formula?: string;
	calcMode?: string;
	accumulate?: string;
	additive?: string;
	keySplines?: string;
}

interface AnimateMotionNodeInfo extends AnimateNodeInfo {
	path?: string;
}

interface AnimateColorNodeInfo extends AnimateNodeInfo {
	colorInterpolation: string;
	colorInterpolationDirection: string;
}

interface AnimateTransformNodeInfo extends AnimateNodeInfo {
	transformType: string;
}

interface TransitionFilterNodeInfo extends AnimateNodeInfo {
	transitionType: string;
	transitionSubType?: string;
	transitionMode?: string;
	transitionDirection?: string;
	transitionFadeColor?: string;
}

type AnimationNodeInfo =
	| ContainerNodeInfo
	| IterateNodeInfo
	| AnimateNodeInfo
	| AnimateMotionNodeInfo
	| AnimateColorNodeInfo
	| AnimateTransformNodeInfo
	| TransitionFilterNodeInfo;
