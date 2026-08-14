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

abstract class BaseContainerNode extends BaseNode {
	protected aChildrenArray: Array<BaseNode>;
	protected nFinishedChildren: number;
	private bDurationIndefinite: boolean;
	private nLeftIterations: number;
	private eImpressNodeType: ImpressNodeType;
	protected bIsEffect: boolean = false;

	protected constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		super(aNodeInfo, aParentNode, aNodeContext);

		this.sClassName = 'BaseContainerNode';
		this.bIsContainer = true;

		this.aChildrenArray = [];
		this.nFinishedChildren = 0;
		this.bDurationIndefinite = false;
		this.nLeftIterations = 1;

		this.eImpressNodeType = undefined;
	}

	public parseNodeInfo() {
		super.parseNodeInfo();

		const aNodeInfo: ContainerNodeInfo = this.aNodeInfo;

		if (this.getParentNode() && this.getParentNode().isMainSequenceRootNode()) {
			if (this.getBegin().getEventType() !== EventTrigger.OnNext) {
				this.bIsFirstAutoEffect = true;
			}
		}

		// nodeType property
		this.eImpressNodeType = ImpressNodeType.Default;
		if (aNodeInfo.nodeType && aNodeInfo.nodeType in ImpressNodeType) {
			const sImpressNodeType =
				aNodeInfo.nodeType as keyof typeof ImpressNodeType;
			this.eImpressNodeType = ImpressNodeType[sImpressNodeType];
		}
		this.bIsMainSequenceRootNode =
			this.eImpressNodeType === ImpressNodeType.MainSequence;
		this.bIsInteractiveSequenceRootNode =
			this.eImpressNodeType === ImpressNodeType.InteractiveSequence;

		this.bIsEffect = !!this.aNodeInfo.presetId;

		for (const childNode of this.aChildrenArray) {
			childNode.parseNodeInfo();
		}

		// resolve duration
		this.bDurationIndefinite =
			(!this.getDuration() || this.getDuration().isIndefinite()) &&
			(!this.getEnd() || this.getEnd().getType() != TimingType.Offset);
	}

	// the preset is on the effect node
	public getPresetLabel(): string | undefined {
		const sPresetLabel = super.getPresetLabel();
		if (sPresetLabel) return sPresetLabel;

		for (const aChildNode of this.aChildrenArray) {
			const sChildPresetLabel = aChildNode.getPresetLabel();
			if (sChildPresetLabel) return sChildPresetLabel;
		}
		return undefined;
	}

	public getTargetAnimatedElement(): AnimatedElement {
		for (const aChildNode of this.aChildrenArray) {
			const aAnimatedElement = aChildNode.getTargetAnimatedElement();
			if (aAnimatedElement) return aAnimatedElement;
		}
		return null;
	}

	public appendChildNode(aAnimationNode: BaseNode) {
		if (!this.checkValidNode()) return;

		if (aAnimationNode.registerDeactivatingListener(this))
			this.aChildrenArray.push(aAnimationNode);
	}

	public removeAllChildrenNodes() {
		this.aChildrenArray = [];
	}

	protected init_st() {
		this.nLeftIterations = this.getRepeatCount();

		return this.init_children();
	}

	protected init_children() {
		this.nFinishedChildren = 0;
		const nChildrenCount = this.aChildrenArray.length;
		let nInitChildren = 0;
		for (let i = 0; i < nChildrenCount; ++i) {
			if (this.aChildrenArray[i].init()) {
				++nInitChildren;
			}
		}
		return nChildrenCount === nInitChildren;
	}

	protected deactivate_st(eDestState: NodeState) {
		this.nLeftIterations = 0;
		if (eDestState === NodeState.Frozen) {
			// deactivate all children that are not FROZEN or ENDED:
			this.forEachChildNode(
				BaseNode.prototype.deactivate,
				~(NodeState.Frozen | NodeState.Ended),
			);
		} else {
			// end all children that are not ENDED:
			this.forEachChildNode(BaseNode.prototype.end, ~NodeState.Ended);
			if (this.getFillMode() === FillModes.Remove) this.removeEffect();
		}
	}

	public hasPendingAnimation() {
		const nChildrenCount = this.aChildrenArray.length;
		for (let i = 0; i < nChildrenCount; ++i) {
			if (this.aChildrenArray[i].hasPendingAnimation()) return true;
		}
		return false;
	}

	protected abstract activate_st(): void;

	public abstract notifyDeactivating(aAnimationNode: BaseNode): void;

	public isDurationIndefinite() {
		return this.bDurationIndefinite;
	}

	public isChildNode(aAnimationNode: BaseNode) {
		const nChildrenCount = this.aChildrenArray.length;
		for (let i = 0; i < nChildrenCount; ++i) {
			if (this.aChildrenArray[i].getId() == aAnimationNode.getId()) return true;
		}
		return false;
	}

	public notifyDeactivatedChild(aChildNode: BaseNode) {
		assert(
			aChildNode.getState() == NodeState.Frozen ||
				aChildNode.getState() == NodeState.Ended,
			'BaseContainerNode.notifyDeactivatedChild: passed child node is neither in FROZEN nor in ENDED state',
		);

		assert(
			this.getState() !== NodeState.Invalid,
			'BaseContainerNode.notifyDeactivatedChild: this node is invalid',
		);

		if (!this.isChildNode(aChildNode)) {
			window.app.console.log(
				'BaseContainerNode.notifyDeactivatedChild: unknown child notifier!',
			);
			return false;
		}

		const nChildrenCount = this.aChildrenArray.length;

		assert(
			this.nFinishedChildren < nChildrenCount,
			'BaseContainerNode.notifyDeactivatedChild: assert(this.nFinishedChildren < nChildrenCount) failed',
		);

		++this.nFinishedChildren;
		let bFinished = this.nFinishedChildren >= nChildrenCount;

		if (bFinished && this.isDurationIndefinite()) {
			if (this.nLeftIterations >= 1.0) {
				this.nLeftIterations -= 1.0;
			}
			if (this.nLeftIterations >= 1.0) {
				bFinished = false;
				const aRepetitionEvent = makeDelay(this.repeat.bind(this), 0.0);
				this._context.aTimerEventQueue.addEvent(aRepetitionEvent);
			} else {
				this.deactivate();
			}
		}

		return bFinished;
	}

	public repeat() {
		// end all children that are not ENDED:
		this.forEachChildNode(BaseNode.prototype.end, ~NodeState.Ended);
		this.removeEffect();
		const bInitialized = this.init_children();
		if (bInitialized) this.activate_st();
		return bInitialized;
	}

	public removeEffect() {
		const nChildrenCount = this.aChildrenArray.length;
		if (nChildrenCount == 0) return;
		// We remove effect in reverse order.
		for (let i = nChildrenCount - 1; i >= 0; --i) {
			if (
				(this.aChildrenArray[i].getState() &
					(NodeState.Frozen | NodeState.Ended)) ==
				0
			) {
				window.app.console.log(
					'BaseContainerNode.removeEffect: child(id:' +
						this.aChildrenArray[i].getId() +
						') is neither frozen nor ended;' +
						' state: ' +
						NodeState[this.aChildrenArray[i].getState()],
				);
				continue;
			}
			this.aChildrenArray[i].removeEffect();
		}
	}

	public saveStateOfAnimatedElement() {
		const nChildrenCount = this.aChildrenArray.length;
		for (let i = 0; i < nChildrenCount; ++i) {
			this.aChildrenArray[i].saveStateOfAnimatedElement();
		}
	}

	public forEachChildNode(
		aFunction: (t: BaseNode) => void,
		eNodeStateMask?: number,
	) {
		if (!eNodeStateMask) eNodeStateMask = -1;

		for (const childNode of this.aChildrenArray) {
			if (eNodeStateMask != -1 && (childNode.getState() & eNodeStateMask) == 0)
				continue;
			aFunction.call(childNode);
		}
	}

	public getImpressNodeType(): ImpressNodeType {
		return this.eImpressNodeType;
	}

	public dispose() {
		const nChildrenCount = this.aChildrenArray.length;
		for (let i = 0; i < nChildrenCount; ++i) {
			this.aChildrenArray[i].dispose();
		}

		super.dispose.call(this);
	}

	public info(verbose: boolean): string {
		let sInfo = super.info(verbose);

		if (verbose) {
			if (this.getImpressNodeType())
				sInfo += `; \x1B[31mnodeType: ${ImpressNodeType[this.getImpressNodeType()]}\x1B[m`;
		}

		for (const child of this.aChildrenArray) {
			sInfo += '\n';
			sInfo += child.info(verbose);
		}
		return sInfo;
	}

	isEmpty() {
		return this.aChildrenArray.length === 0;
	}
}

class ParallelTimeContainer extends BaseContainerNode {
	constructor(aNodeInfo: any, aParentNode: any, aNodeContext: any) {
		super(aNodeInfo, aParentNode, aNodeContext);
		this.sClassName = 'ParallelTimeContainer';
	}

	protected activate_st() {
		const nChildrenCount = this.aChildrenArray.length;
		let nResolvedChildren = 0;
		for (let i = 0; i < nChildrenCount; ++i) {
			if (this.aChildrenArray[i].resolve()) {
				++nResolvedChildren;
			}
		}

		if (nChildrenCount != nResolvedChildren) {
			window.app.console.log(
				'ParallelTimeContainer.activate_st: resolving all children failed',
			);
			return;
		}

		if (this.isDurationIndefinite() && nChildrenCount == 0) {
			this.scheduleDeactivationEvent(this.makeDeactivationEvent(0.0));
		} else {
			this.scheduleDeactivationEvent();
		}
	}

	public notifyDeactivating(aAnimationNode: BaseNode) {
		this.notifyDeactivatedChild(aAnimationNode);
	}
}

class SequentialTimeContainer extends BaseContainerNode {
	private bIsRewinding: boolean;
	private aCurrentSkipEvent: DelayEvent;
	private aRewindCurrentEffectEvent: DelayEvent;
	private aRewindLastEffectEvent: DelayEvent;

	constructor(aNodeInfo: any, aParentNode: any, aNodeContext: any) {
		super(aNodeInfo, aParentNode, aNodeContext);

		this.sClassName = 'SequentialTimeContainer';
		this.bIsRewinding = false;
		this.aCurrentSkipEvent = null;
		this.aRewindCurrentEffectEvent = null;
		this.aRewindLastEffectEvent = null;
	}

	protected activate_st() {
		const nChildrenCount = this.aChildrenArray.length;
		for (; this.nFinishedChildren < nChildrenCount; ++this.nFinishedChildren) {
			if (this.resolveChild(this.aChildrenArray[this.nFinishedChildren])) break;
			else
				window.app.console.log(
					`SequentialTimeContainer(${this.getId()}).activate_st: resolving child(${this.nFinishedChildren}) failed!`,
				);
		}

		if (
			this.isDurationIndefinite() &&
			(nChildrenCount == 0 || this.nFinishedChildren >= nChildrenCount)
		) {
			// deactivate ASAP:
			this.scheduleDeactivationEvent(this.makeDeactivationEvent(0.0));
		} else {
			this.scheduleDeactivationEvent();
		}
	}

	public notifyDeactivating(aNotifier: BaseNode) {
		// If we are rewinding we have not to resolve the next child.
		if (this.bIsRewinding) return;

		if (this.notifyDeactivatedChild(aNotifier)) return;

		assert(
			this.nFinishedChildren < this.aChildrenArray.length,
			'SequentialTimeContainer.notifyDeactivating: assertion (this.nFinishedChildren < this.aChildrenArray.length) failed',
		);

		const aNextChild = this.aChildrenArray[this.nFinishedChildren];

		assert(
			aNextChild.getState() === NodeState.Unresolved,
			'SequentialTimeContainer.notifyDeactivating: assertion (aNextChild.getState == UNRESOLVED_NODE) failed',
		);

		if (!this.resolveChild(aNextChild)) {
			// could not resolve child - since we risk to
			// stall the chain of events here, play it safe
			// and deactivate this node (only if we have
			// indefinite duration - otherwise, we'll get a
			// deactivation event, anyways).
			this.deactivate();
		}
	}

	/** skipEffect
	 *  Skip the current playing shape effect.
	 *  Requires: the current node is the main sequence root node.
	 *
	 *  @param aChildNode
	 *      An animation node representing the root node of the shape effect being
	 *      played.
	 */
	public skipEffect(aChildNode: BaseNode) {
		if (this.isChildNode(aChildNode)) {
			// First off we end all queued activities.
			this.getContext().aActivityQueue.endAll();
			// We signal that we are going to skip all subsequent animations by
			// setting the bIsSkipping flag to 'true', then all queued events are
			// fired immediately. In such a way the correct order of the various
			// events that belong to the animation time-line is preserved.
			this.getContext().bIsSkipping = true;
			this.getContext().aTimerEventQueue.forceEmpty();
			this.getContext().bIsSkipping = false;
			const aEvent = makeEvent(aChildNode.deactivate.bind(aChildNode));
			this.getContext().aTimerEventQueue.addEvent(aEvent);
		} else {
			window.app.console.log(
				'SequentialTimeContainer.skipEffect: unknown child: ' +
					aChildNode.getId(),
			);
		}
	}

	/** rewindCurrentEffect
	 *  Rewind a playing shape effect.
	 *  Requires: the current node is the main sequence root node.
	 *
	 *  @param aChildNode
	 *      An animation node representing the root node of the shape effect being
	 *      played
	 */
	public rewindCurrentEffect(aChildNode: BaseNode) {
		if (this.isChildNode(aChildNode)) {
			assert(
				!this.bIsRewinding,
				'SequentialTimeContainer.rewindCurrentEffect: is already rewinding.',
			);

			// We signal we are rewinding so the notifyDeactivating method returns
			// immediately without increment the finished children counter and
			// resolve the next child.
			this.bIsRewinding = true;
			// First off we end all queued activities.
			this.getContext().aActivityQueue.endAll();
			// We signal that we are going to skip all subsequent animations by
			// setting the bIsSkipping flag to 'true', then all queued events are
			// fired immediately. In such a way the correct order of the various
			// events that belong to the animation time-line is preserved.
			this.getContext().bIsSkipping = true;
			this.getContext().aTimerEventQueue.forceEmpty();
			this.getContext().bIsSkipping = false;
			// We end all new activities appended to the activity queue by
			// the fired events.
			this.getContext().aActivityQueue.endAll();

			// Now we perform a final 'end' and restore the animated shape to
			// the state it was before the current effect was applied.
			aChildNode.end();
			aChildNode.removeEffect();
			// Finally we place the child node to the 'unresolved' state and
			// resolve it again.
			aChildNode.init();
			this.resolveChild(aChildNode);
			this.notifyRewindedEvent(aChildNode);
			this.bIsRewinding = false;
		} else {
			window.app.console.log(
				'SequentialTimeContainer.rewindCurrentEffect: unknown child: ' +
					aChildNode.getId(),
			);
		}
	}

	/** rewindLastEffect
	 *  Rewind the last ended effect.
	 *  Requires: the current node is the main sequence root node.
	 *
	 *  @param aChildNode
	 *      An animation node representing the root node of the next shape effect
	 *      to be played.
	 */
	public rewindLastEffect(aChildNode: BaseNode) {
		if (this.isChildNode(aChildNode)) {
			assert(
				!this.bIsRewinding,
				'SequentialTimeContainer.rewindLastEffect: is already rewinding.',
			);

			// We signal we are rewinding so the notifyDeactivating method returns
			// immediately without increment the finished children counter and
			// resolve the next child.
			this.bIsRewinding = true;
			// We end the current effect.
			this.getContext().aTimerEventQueue.forceEmpty();
			this.getContext().aActivityQueue.clear();
			aChildNode.end();
			// Invoking the end method on the current child node that has not yet
			// been activated should not lead to any change on the animated shape.
			// However for safety we used to call the removeEffect method but
			// lately we noticed that when interactive animation sequences are
			// involved into the shape effect invoking such a method causes
			// some issue.
			//aChildNode.removeEffect();

			// As we rewind the previous effect we need to decrease the finished
			// children counter.
			--this.nFinishedChildren;
			const aPreviousChildNode = this.aChildrenArray[this.nFinishedChildren];
			// No need to invoke the end method for the previous child as it is
			// already in the ENDED state.

			aPreviousChildNode.removeEffect();
			// We place the child node to the 'unresolved' state.
			aPreviousChildNode.init();
			// We need to re-initialize the old current child too, because it is
			// in ENDED state now, On the contrary it cannot be resolved again later.
			aChildNode.init();
			this.resolveChild(aPreviousChildNode);
			this.notifyRewindedEvent(aChildNode);
			this.bIsRewinding = false;
		} else {
			window.app.console.log(
				'SequentialTimeContainer.rewindLastEffect: unknown child: ' +
					aChildNode.getId(),
			);
		}
	}

	/** resolveChild
	 *  Resolve the passed child.
	 *  In case this node is a main sequence root node events for skipping and
	 *  rewinding the effect related to the passed child node are created and
	 *  registered.
	 *
	 *  @param aChildNode
	 *      An animation node representing the root node of the next shape effect
	 *      to be played.
	 *  @return
	 *      It returns true if the passed child has been resolved successfully,
	 *      false otherwise.
	 */
	public resolveChild(aChildNode: BaseNode) {
		const bResolved = aChildNode.resolve();

		if (
			bResolved &&
			(this.isMainSequenceRootNode() || this.isInteractiveSequenceRootNode())
		) {
			if (this.aCurrentSkipEvent) this.aCurrentSkipEvent.dispose();
			this.aCurrentSkipEvent = makeEvent(
				this.skipEffect.bind(this, aChildNode),
			);

			if (this.aRewindCurrentEffectEvent)
				this.aRewindCurrentEffectEvent.dispose();
			this.aRewindCurrentEffectEvent = makeEvent(
				this.rewindCurrentEffect.bind(this, aChildNode),
			);

			if (this.aRewindLastEffectEvent) this.aRewindLastEffectEvent.dispose();
			this.aRewindLastEffectEvent = makeEvent(
				this.rewindLastEffect.bind(this, aChildNode),
			);

			if (this.isMainSequenceRootNode()) {
				this._context.aEventMultiplexer.registerSkipEffectEvent(
					this.aCurrentSkipEvent,
				);
				this._context.aEventMultiplexer.registerRewindCurrentEffectEvent(
					this.aRewindCurrentEffectEvent,
				);
				this._context.aEventMultiplexer.registerRewindLastEffectEvent(
					this.aRewindLastEffectEvent,
				);
			} else if (this.isInteractiveSequenceRootNode()) {
				this._context.aEventMultiplexer.registerSkipInteractiveEffectEvent(
					aChildNode.getId(),
					this.aCurrentSkipEvent,
				);
				this._context.aEventMultiplexer.registerRewindRunningInteractiveEffectEvent(
					aChildNode.getId(),
					this.aRewindCurrentEffectEvent,
				);
				this._context.aEventMultiplexer.registerRewindEndedInteractiveEffectEvent(
					aChildNode.getId(),
					this.aRewindLastEffectEvent,
				);
			}
		}
		return bResolved;
	}

	public notifyRewindedEvent(aChildNode: BaseNode) {
		if (this.isInteractiveSequenceRootNode()) {
			this._context.aEventMultiplexer.notifyRewindedEffectEvent(
				aChildNode.getId(),
			);

			const sId = aChildNode.getBegin().getEventBaseElementId();
			if (sId) {
				this._context.aEventMultiplexer.notifyRewindedEffectEvent(sId);
			}
		}
	}

	public dispose() {
		if (this.aCurrentSkipEvent) this.aCurrentSkipEvent.dispose();

		super.dispose();
	}
}
