// @ts-strict-ignore
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

type AnimationNodeMap = Map<string, BaseNode>;

enum AnimationNodeType {
	Custom,
	Par,
	Seq,
	Iterate,
	Animate,
	Set,
	AnimateMotion,
	AnimateColor,
	AnimateTransform,
	TransitionFilter,
	Audio,
	Command,
}

function getAnimationNodeType(aNodeInfo: AnimationNodeInfo) {
	const typeStr = aNodeInfo.nodeName as keyof typeof AnimationNodeType;
	return AnimationNodeType[typeStr];
}

function getNodeChildren(aNode: ContainerNodeInfo): Array<AnimationNodeInfo> {
	return aNode.children || [];
}

function isAttributeSupported(aNodeInfo: AnimateNodeInfo) {
	if (!aNodeInfo.attributeName) {
		console.error('slideshow: missing attributeName');
		return false;
	}
	// return true;
	return AnimatedElement.SupportedProperties.has(
		aNodeInfo.attributeName.toLowerCase(),
	);
}

function isTransformSupported(aNodeInfo: AnimateTransformNodeInfo) {
	if (!aNodeInfo.transformType) {
		console.error('slideshow: missing transformType');
		return false;
	}
	// return true;
	return AnimatedElement.SupportedTransformations.has(
		aNodeInfo.transformType.toLowerCase(),
	);
}

function createAnimationTree(
	aAnimationRoot: AnimationNodeInfo,
	aNodeContext: NodeContext,
): BaseNode {
	return createAnimationNode(aAnimationRoot, null, aNodeContext);
}

function createAnimationNode(
	aNodeInfo: AnimationNodeInfo,
	aParentNode: BaseContainerNode,
	aNodeContext: NodeContext,
): BaseNode {
	assert(aNodeInfo, 'createAnimationNode: invalid animation node');

	const eAnimationNodeType = getAnimationNodeType(aNodeInfo);
	let aCreatedNode = null;
	let aCreatedContainer = null;

	switch (eAnimationNodeType) {
		case AnimationNodeType.Par:
			aCreatedNode = aCreatedContainer = new ParallelTimeContainer(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			break;
		case AnimationNodeType.Iterate:
			window.app.console.log('createAnimationNode: Iterate not implemented');
			return null;
		case AnimationNodeType.Seq:
			aCreatedNode = aCreatedContainer = new SequentialTimeContainer(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			break;
		case AnimationNodeType.Animate:
			if (isAttributeSupported(aNodeInfo)) {
				aCreatedNode = new PropertyAnimationNode(
					aNodeInfo,
					aParentNode,
					aNodeContext,
				);
				break;
			} else {
				const attrName = (aNodeInfo as AnimateNodeInfo).attributeName;
				window.app.console.log(
					`createAnimationNode: PropertyAnimationNode: attribute '${attrName}' not supported`,
				);
				return null;
			}
		case AnimationNodeType.Set:
			if (isAttributeSupported(aNodeInfo)) {
				aCreatedNode = new AnimationSetNode(
					aNodeInfo,
					aParentNode,
					aNodeContext,
				);
				break;
			} else {
				const attrName = (aNodeInfo as AnimateNodeInfo).attributeName;
				window.app.console.log(
					`createAnimationNode: AnimationSetNode: attribute '${attrName}' not supported`,
				);
				return null;
			}
		case AnimationNodeType.AnimateMotion:
			aCreatedNode = new AnimationPathMotionNode(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			// window.app.console.log(
			// 	'createAnimationNode: AnimateMotion not implemented',
			// );
			//return null;
			break;
		case AnimationNodeType.AnimateColor:
			aCreatedNode = new AnimationColorNode(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			break;
		case AnimationNodeType.AnimateTransform:
			if (isTransformSupported(aNodeInfo as AnimateTransformNodeInfo)) {
				aCreatedNode = new AnimationTransformNode(
					aNodeInfo,
					aParentNode,
					aNodeContext,
				);
				break;
			} else {
				const transfType = (aNodeInfo as AnimateTransformNodeInfo)
					.transformType;
				window.app.console.log(
					`createAnimationNode: AnimationTransformNode: transform '${transfType}' not supported`,
				);
				return null;
			}
		case AnimationNodeType.TransitionFilter: {
			aCreatedNode = new AnimationTransitionFilterNode(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			break;
		}
		case AnimationNodeType.Audio:
			window.app.console.log('createAnimationNode: Audio not implemented');
			return null;
		case AnimationNodeType.Command:
			aCreatedNode = new AnimationCommandNode(
				aNodeInfo,
				aParentNode,
				aNodeContext,
			);
			break;
		default:
			window.app.console.log(
				'createAnimationNode: invalid Animation Node Type: ' +
					eAnimationNodeType,
			);
			return null;
	}

	if (aCreatedContainer) {
		const aChildrenArray = getNodeChildren(aNodeInfo);
		for (let i = 0; i < aChildrenArray.length; ++i) {
			if (
				!createChildNode(aChildrenArray[i], aCreatedContainer, aNodeContext)
			) {
				// Discard the whole effect except animation node is a Set node and
				// it's not the only node included into the effect.
				if (
					!(
						aChildrenArray.length !== 1 &&
						getAnimationNodeType(aChildrenArray[i]) === AnimationNodeType.Set
					)
				) {
					aCreatedContainer.removeAllChildrenNodes();
					break;
				}
			}
		}
	}

	return aCreatedNode;
}

function createChildNode(
	aNodeInfo: AnimationNodeInfo,
	aParentNode: BaseContainerNode,
	aNodeContext: NodeContext,
): boolean {
	const aChildNode = createAnimationNode(aNodeInfo, aParentNode, aNodeContext);

	if (!aChildNode) {
		// The slideshow plays no audio node, so one is left out and its siblings still run.
		if (getAnimationNodeType(aNodeInfo) === AnimationNodeType.Audio) {
			window.app.console.log('createChildNode: audio node skipped');
			return true;
		}
		window.app.console.log('createChildNode: child node creation failed');
		return false;
	} else {
		if (aChildNode.isContainer() && !aChildNode.isMainSequenceRootNode()) {
			const aContainer = aChildNode as BaseContainerNode;
			// an empty effect should be removed, but sibling effects shouldn't, so don't report failure
			if (aContainer.isEmpty()) {
				window.app.console.log(
					`createChildNode: child-node(${aContainer.getId()}) is empty`,
				);
				return true; // neither append, nor fail
			}
		}
		aParentNode.appendChildNode(aChildNode);
		return true;
	}
}

class SlideAnimations {
	private aRootNode: BaseNode;
	private aContext: NodeContext;
	private aSlideShowHandler: SlideShowHandler;
	private aAnimationNodeMap: AnimationNodeMap;
	private aAnimatedElementMap: Map<string, AnimatedElement>;
	private aSourceEventElementMap: Map<string, SourceEventElement>;
	private aNextEffectEventArray: NextEffectEventArray;
	private aInteractiveAnimationSequenceMap: InteractiveAnimationSequenceMap;
	private aEventMultiplexer: EventMultiplexer;
	private bElementsParsed: boolean;

	constructor(aSlideShowContext: SlideShowContext, metaSlide: MetaSlide) {
		this.aContext = new NodeContext(aSlideShowContext);
		this.aSlideShowHandler = aSlideShowContext.aSlideShowHandler;
		this.aAnimationNodeMap = new Map();
		this.aAnimatedElementMap = new Map();
		this.aSourceEventElementMap = new Map();
		this.aNextEffectEventArray = new NextEffectEventArray();
		this.aInteractiveAnimationSequenceMap = new Map();
		this.aEventMultiplexer = new EventMultiplexer(
			aSlideShowContext.aTimerEventQueue,
		);
		this.aRootNode = null;
		this.bElementsParsed = false;

		this.aContext.metaSlide = metaSlide;
		this.aContext.aAnimationNodeMap = this.aAnimationNodeMap;
		this.aContext.aAnimatedElementMap = this.aAnimatedElementMap;
		this.aContext.aSourceEventElementMap = this.aSourceEventElementMap;
	}

	public importAnimations(aAnimationRoot: AnimationNodeInfo): boolean {
		if (!aAnimationRoot) return false;

		this.aRootNode = createAnimationTree(aAnimationRoot, this.aContext);
		return !!this.aRootNode;
	}

	public parseInfo() {
		if (!this.aRootNode) return false;

		this.aRootNode.parseNodeInfo();
		this.bElementsParsed = true;
	}

	public elementsParsed() {
		return this.bElementsParsed;
	}

	public info(verbose: boolean) {
		if (this.aRootNode) return this.aRootNode.info(verbose);
	}

	isFirstRun() {
		return this.aContext.bFirstRun;
	}

	isAnimated() {
		if (!this.bElementsParsed) return false;

		return this.aRootNode.hasPendingAnimation();
	}

	start() {
		if (!this.bElementsParsed) return false;

		this.chargeSourceEvents();
		this.chargeInterAnimEvents();

		this.aSlideShowHandler.setSlideEvents(
			this.aNextEffectEventArray,
			this.aInteractiveAnimationSequenceMap,
			this.aEventMultiplexer,
		);

		if (this.aContext.bFirstRun === undefined) this.aContext.bFirstRun = true;
		else if (this.aContext.bFirstRun) this.aContext.bFirstRun = false;

		// init all nodes
		this.aContext.bIsInvalid = !this.aRootNode.init();
		if (this.aContext.bIsInvalid) {
			ANIMDBG.print('SlideAnimationsthis.start: aContext.bIsInvalid');
			return false;
		}

		// resolve root node
		return this.aRootNode.resolve();
	}

	end(bLeftEffectsSkipped: boolean) {
		if (!this.bElementsParsed) return; // no animations there

		// end root node
		this.aRootNode.deactivate();
		this.aRootNode.end();

		if (bLeftEffectsSkipped && this.isFirstRun()) {
			// in case this is the first run and left events have been skipped
			// some next effect events for the slide could not be collected
			// so the next time we should behave as it was the first run again
			this.aContext.bFirstRun = undefined;
		} else if (this.isFirstRun()) {
			this.aContext.bFirstRun = false;
		}

		this.aContext.bIsInvalid = false;
	}

	dispose() {
		if (this.aRootNode) {
			this.aRootNode.dispose();
		}
		if (this.aEventMultiplexer) {
			this.aEventMultiplexer.clear();
		}
	}

	clearNextEffectEvents() {
		ANIMDBG.print('SlideAnimations.clearNextEffectEvents');
		this.aNextEffectEventArray.clear();
		this.aContext.bFirstRun = undefined;
	}

	chargeSourceEvents() {
		this.aSourceEventElementMap.forEach((aSourceEventElement) => {
			aSourceEventElement.charge();
		});
	}

	chargeInterAnimEvents() {
		this.aInteractiveAnimationSequenceMap.forEach(
			(aInteractiveAnimationSequence) => {
				aInteractiveAnimationSequence.chargeEvents();
			},
		);
	}

	public getAnimationsTree() {
		return this.aRootNode;
	}

	public getAnimatedElementMap(): Map<string, AnimatedElement> {
		return this.aAnimatedElementMap;
	}

	public get eventMultiplexer(): EventMultiplexer {
		return this.aEventMultiplexer;
	}
}
