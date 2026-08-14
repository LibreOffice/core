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

enum NodeState {
	Invalid = 0,
	Unresolved = 1,
	Resolved = 2,
	Active = 4,
	Frozen = 8,
	Ended = 16,
}

enum FillModes {
	Default = 0,
	Inherit = 0,
	Remove,
	Freeze,
	Hold,
	Transition,
	Auto,
}

enum RestartMode {
	Default = 0,
	Inherit = 0,
	Always,
	WhenNotActive,
	Never,
}

enum ImpressNodeType {
	Default,
	OnClick,
	WithPrevious,
	AfterPrevious,
	MainSequence,
	TimingRoot,
	InteractiveSequence,
}

enum TransitionMode {
	out,
	in,
}

enum AdditiveMode {
	Unknown,
	Base,
	Replace,
	Multiply,
	None,
}

enum AccumulateMode {
	None,
	Sum,
}

enum CalcMode {
	Discrete = 1,
	Linear,
	Paced,
	Spline,
}

function createStateTransitionTable() {
	const aSTT: Map<
		RestartMode,
		Map<FillModes, Map<NodeState, number>>
	> = new Map();

	aSTT.set(RestartMode.Never, new Map());
	aSTT.set(RestartMode.WhenNotActive, new Map());
	aSTT.set(RestartMode.Always, new Map());

	// transition table for restart=NEVER, fill=REMOVE
	const aTable0 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[NodeState.Active, NodeState.Ended],
		[NodeState.Frozen, NodeState.Invalid], // this state is unreachable here
		[NodeState.Ended, NodeState.Ended], // this state is a sink here (cannot restart)
	]);
	aSTT.get(RestartMode.Never).set(FillModes.Remove, aTable0);

	// transition table for restart=NEVER, fill=FREEZE, HOLD, TRANSITION
	const aTable1 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[NodeState.Active, NodeState.Frozen | NodeState.Ended],
		[NodeState.Frozen, NodeState.Ended],
		[NodeState.Ended, NodeState.Ended], // this state is a sink here (cannot restart)
	]);

	aSTT.get(RestartMode.Never).set(FillModes.Freeze, aTable1);
	aSTT.get(RestartMode.Never).set(FillModes.Hold, aTable1);
	aSTT.get(RestartMode.Never).set(FillModes.Transition, aTable1);

	// transition table for restart=WHEN_NOT_ACTIVE, fill=REMOVE
	const aTable2 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[NodeState.Active, NodeState.Ended],
		[NodeState.Frozen, NodeState.Invalid], // this state is unreachable here
		[NodeState.Ended, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
	]);
	aSTT.get(RestartMode.WhenNotActive).set(FillModes.Remove, aTable2);

	// transition table for restart=WHEN_NOT_ACTIVE, fill=FREEZE, HOLD, TRANSITION
	const aTable3 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[NodeState.Active, NodeState.Frozen | NodeState.Ended],
		[NodeState.Frozen, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
		[NodeState.Ended, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
	]);
	aSTT.get(RestartMode.WhenNotActive).set(FillModes.Freeze, aTable3);
	aSTT.get(RestartMode.WhenNotActive).set(FillModes.Hold, aTable3);
	aSTT.get(RestartMode.WhenNotActive).set(FillModes.Transition, aTable3);

	// transition table for restart=ALWAYS, fill=REMOVE
	const aTable4 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[NodeState.Active, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
		[NodeState.Frozen, NodeState.Invalid], // this state is unreachable here
		[NodeState.Ended, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
	]);
	aSTT.get(RestartMode.Always).set(FillModes.Remove, aTable4);

	// transition table for restart=ALWAYS, fill=FREEZE, HOLD, TRANSITION
	const aTable5 = new Map([
		[NodeState.Invalid, NodeState.Invalid],
		[NodeState.Unresolved, NodeState.Resolved | NodeState.Ended],
		[NodeState.Resolved, NodeState.Active | NodeState.Ended],
		[
			NodeState.Active,
			NodeState.Resolved |
				NodeState.Active |
				NodeState.Frozen |
				NodeState.Ended,
		],
		[NodeState.Frozen, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
		[NodeState.Ended, NodeState.Resolved | NodeState.Active | NodeState.Ended], // restart is possible
	]);
	aSTT.get(RestartMode.Always).set(FillModes.Freeze, aTable5);
	aSTT.get(RestartMode.Always).set(FillModes.Hold, aTable5);
	aSTT.get(RestartMode.Always).set(FillModes.Transition, aTable5);

	return aSTT;
}

const aStateTransitionTable = createStateTransitionTable();

function getTransitionTable(eRestartMode: RestartMode, eFillMode: FillModes) {
	// If restart mode has not been resolved we use 'never'.
	// Note: RestartMode.Default == RestartMode.Inherit.
	if (eRestartMode == RestartMode.Default) {
		window.app.console.log(
			'getTransitionTable: unexpected restart mode: ' +
				eRestartMode +
				'. Used NEVER instead.',
		);
		eRestartMode = RestartMode.Never;
	}

	// If fill mode has not been resolved we use 'remove'.
	// Note: FillModes.Default == FillModes.Inherit.
	if (eFillMode == FillModes.Default || eFillMode == FillModes.Auto) {
		eFillMode = FillModes.Remove;
	}

	return aStateTransitionTable.get(eRestartMode).get(eFillMode);
}

class StateTransition {
	private aNode: BaseNode;
	private eToState: NodeState;

	constructor(aBaseNode: BaseNode) {
		this.aNode = aBaseNode;
		this.eToState = NodeState.Invalid;
	}

	public enter(eNodeState: NodeState, bForce?: boolean) {
		if (!bForce) bForce = false;

		if (this.eToState != NodeState.Invalid) {
			window.app.console.log(
				'StateTransition.enter: commit() before enter()ing again!',
			);
			return false;
		}
		if (!bForce && !this.aNode.isTransition(this.aNode.getState(), eNodeState))
			return false;

		// recursion detection:
		if ((this.aNode.nCurrentStateTransition & eNodeState) != 0) return false; // already in wanted transition

		// mark transition:
		this.aNode.nCurrentStateTransition |= eNodeState;
		this.eToState = eNodeState;
		return true;
	}

	public commit() {
		if (this.eToState != NodeState.Invalid) {
			this.aNode.setState(this.eToState);
			this.clear();
		}
	}

	public clear() {
		if (this.eToState != NodeState.Invalid) {
			this.aNode.nCurrentStateTransition &= ~this.eToState;
			this.eToState = NodeState.Invalid;
		}
	}
}

abstract class BaseNode {
	private static CURR_UNIQUE_ID = 0;
	private readonly nId: number;

	protected sClassName = 'BaseNode';
	public readonly eAnimationNodeType: AnimationNodeType;
	protected bIsContainer: boolean;
	protected bIsFirstAutoEffect: boolean = false;
	protected readonly aNodeInfo: AnimationNodeInfo;
	protected readonly aParentNode: BaseContainerNode;
	protected readonly aNodeContext: NodeContext;
	protected _context: SlideShowContext;
	private nStartDelay: number;
	protected eCurrentState: NodeState;
	public nCurrentStateTransition: number;
	private aDeactivatingListenerArray: BaseNode[];
	private aActivationEvent: DelayEvent;
	private aDeactivationEvent: DelayEvent;

	private aBegin: Timing;
	private aEnd: Timing;
	private aDuration: Duration;
	protected bIsMainSequenceRootNode: boolean = false;
	protected bIsInteractiveSequenceRootNode: boolean = false;
	private eFillMode: FillModes;
	private eRestartMode: RestartMode;
	private nRepeatCount: number;
	private nAccelerate: number;
	private nDecelerate: number;
	private bAutoReverse: boolean;

	private aAnimationNodeMap: AnimationNodeMap;
	private aStateTransTable: Map<NodeState, number>;

	protected constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		this.nId = BaseNode.getUniqueId();

		if (!aNodeInfo)
			window.app.console.log(
				'BaseNode(id:' + this.nId + ') constructor: aNodeInfo is not valid',
			);

		if (!aNodeContext)
			window.app.console.log(
				'BaseNode(id:' + this.nId + ') constructor: aNodeContext is not valid',
			);

		if (!aNodeContext._context)
			window.app.console.log(
				'BaseNode(id:' +
					this.nId +
					') constructor: aNodeContext.aContext is not valid',
			);

		this.eAnimationNodeType = getAnimationNodeType(aNodeInfo);

		this.bIsContainer = false;
		this.aNodeInfo = aNodeInfo;
		this.aParentNode = aParentNode;
		this.aNodeContext = aNodeContext;
		this._context = aNodeContext._context;
		this.nStartDelay = aNodeContext.nStartDelay;
		this.eCurrentState = NodeState.Unresolved;
		this.nCurrentStateTransition = 0;
		this.aDeactivatingListenerArray = [];
		this.aActivationEvent = null;
		this.aDeactivationEvent = null;

		this.aBegin = null;
		this.aDuration = null;
		this.aEnd = null;
		this.eFillMode = FillModes.Freeze;
		this.eRestartMode = RestartMode.Never;
		this.nRepeatCount = undefined;
		this.nAccelerate = 0.0;
		this.nDecelerate = 0.0;
		this.bAutoReverse = false;
	}

	private static getUniqueId() {
		++BaseNode.CURR_UNIQUE_ID;
		return BaseNode.CURR_UNIQUE_ID;
	}

	getId() {
		return this.nId;
	}

	public parseNodeInfo() {
		if (this.aNodeInfo.id)
			this.aNodeContext.aAnimationNodeMap.set(this.aNodeInfo.id, this);

		this.aBegin = new Timing(this, this.aNodeInfo.begin);
		this.aBegin.parse();

		this.aEnd = null;
		if (this.aNodeInfo.end) {
			this.aEnd = new Timing(this, this.aNodeInfo.end);
			this.aEnd.parse();
		}

		this.aDuration = new Duration(this.aNodeInfo.dur);
		if (!this.aDuration.isSet()) {
			if (this.isContainer()) this.aDuration = null;
			else this.aDuration = new Duration('indefinite');
		}

		if (this.aNodeInfo.fill && this.aNodeInfo.fill in FillModes) {
			const sFill = this.aNodeInfo.fill as keyof typeof FillModes;
			this.eFillMode = FillModes[sFill];
		} else {
			this.eFillMode = FillModes.Default;
		}

		if (this.aNodeInfo.restart && this.aNodeInfo.restart in RestartMode) {
			const sRestart = this.aNodeInfo.restart as keyof typeof RestartMode;
			this.eRestartMode = RestartMode[sRestart];
		} else {
			this.eRestartMode = RestartMode.Default;
		}

		this.nRepeatCount = 1;
		if (this.aNodeInfo.repeatCount)
			this.nRepeatCount = parseFloat(this.aNodeInfo.repeatCount);
		if (isNaN(this.nRepeatCount) && this.aNodeInfo.repeatCount !== 'indefinite')
			this.nRepeatCount = 1;

		this.nAccelerate = 0.0;
		const sAccelerate = this.aNodeInfo.accelerate;
		if (sAccelerate) this.nAccelerate = parseFloat(sAccelerate);
		if (isNaN(this.nAccelerate)) this.nAccelerate = 0.0;

		this.nDecelerate = 0.0;
		const sDecelerate = this.aNodeInfo.decelerate;
		if (sDecelerate) this.nDecelerate = parseFloat(sDecelerate);
		if (isNaN(this.nDecelerate)) this.nDecelerate = 0.0;

		this.bAutoReverse = false;
		const sAutoReverse = this.aNodeInfo.autoreverse;
		if (sAutoReverse == 'true') this.bAutoReverse = true;

		// resolve fill value
		if (this.eFillMode === FillModes.Default) {
			if (this.getParentNode())
				this.eFillMode = this.getParentNode().getFillMode();
			else this.eFillMode = FillModes.Auto;
		}

		if (this.eFillMode === FillModes.Auto) {
			// see SMIL recommendation document
			this.eFillMode =
				this.aEnd ||
				this.nRepeatCount != 1 ||
				(this.aDuration && !this.aDuration.isIndefinite())
					? FillModes.Remove
					: FillModes.Freeze;
		}

		// resolve restart value
		if (this.eRestartMode === RestartMode.Default) {
			if (this.getParentNode())
				this.eRestartMode = this.getParentNode().getRestartMode();
			// SMIL recommendation document says to set it to 'always'
			else this.eRestartMode = RestartMode.Always;
		}

		// resolve accelerate and decelerate attributes
		// from the SMIL recommendation document: if the individual values of the accelerate
		// and decelerate attributes are between 0 and 1 and the sum is greater than 1,
		// then both the accelerate and decelerate attributes will be ignored and the timed
		// element will behave as if neither attribute was specified.
		if (this.nAccelerate + this.nDecelerate > 1.0) {
			this.nAccelerate = 0.0;
			this.nDecelerate = 0.0;
		}

		this.aStateTransTable = getTransitionTable(
			this.getRestartMode(),
			this.getFillMode(),
		);
	}

	public getParentNode(): BaseContainerNode {
		return this.aParentNode;
	}

	public init(): boolean {
		this.DBG(this.callInfo('init'));
		if (!this.checkValidNode()) return false;
		if (this.aActivationEvent) this.aActivationEvent.dispose();
		if (this.aDeactivationEvent) this.aDeactivationEvent.dispose();

		this.eCurrentState = NodeState.Unresolved;

		return this.init_st();
	}

	public resolve() {
		if (this.aNodeContext.bIsInvalid || !this.checkValidNode()) return false;

		this.DBG(this.callInfo('resolve'));

		if (this.eCurrentState == NodeState.Resolved)
			window.app.console.log('BaseNode.resolve: already in RESOLVED state');

		const aStateTrans = new StateTransition(this);

		if (
			aStateTrans.enter(NodeState.Resolved) &&
			this.isTransition(NodeState.Resolved, NodeState.Active) &&
			this.resolve_st()
		) {
			aStateTrans.commit();

			if (this.aActivationEvent) {
				this.aActivationEvent.charge();
			} else {
				this.aActivationEvent = makeDelay(
					this.activate.bind(this),
					this.getBegin().getOffset() + this.nStartDelay,
				);
			}
			registerEvent(
				this,
				this.aNodeContext._context.aSlideShowHandler,
				this.getBegin(),
				this.aActivationEvent,
				this.aNodeContext,
			);

			return true;
		}

		return false;
	}

	public activate() {
		if (!this.checkValidNode()) return false;

		if (this.eCurrentState === NodeState.Active)
			window.app.console.log('BaseNode.activate: already in ACTIVE state');

		this.DBG(this.callInfo('activate'), getCurrentSystemTime());

		const aStateTrans = new StateTransition(this);

		if (aStateTrans.enter(NodeState.Active)) {
			this.activate_st();
			aStateTrans.commit();

			if (this.bIsFirstAutoEffect)
				this.aNodeContext._context.aSlideShowHandler.notifyFirstAutoEffectStarted();
			if (!this._context.aEventMultiplexer)
				window.app.console.log(
					'BaseNode.activate: this.aContext.aEventMultiplexer is not valid',
				);
			this._context.aEventMultiplexer.notifyEvent(
				EventTrigger.BeginEvent,
				this.getId(),
			);
			return true;
		}
		return false;
	}

	public deactivate() {
		if (
			this.inStateOrTransition(NodeState.Ended | NodeState.Frozen) ||
			!this.checkValidNode()
		)
			return;

		if (this.isTransition(this.eCurrentState, NodeState.Frozen)) {
			this.DBG(this.callInfo('deactivate'), getCurrentSystemTime());

			const aStateTrans = new StateTransition(this);
			if (aStateTrans.enter(NodeState.Frozen, true /* FORCE */)) {
				this.deactivate_st(NodeState.Frozen);
				aStateTrans.commit();

				this.notifyEndListeners();
				if (this.bIsFirstAutoEffect)
					this.aNodeContext._context.aSlideShowHandler.notifyFirstAutoEffectEnded();

				if (this.aActivationEvent) this.aActivationEvent.dispose();
				if (this.aDeactivationEvent) this.aDeactivationEvent.dispose();
			}
		} else {
			this.end();
		}
		// state has changed either to FROZEN or ENDED
	}

	public end() {
		const bIsFrozenOrInTransitionToFrozen = this.inStateOrTransition(
			NodeState.Frozen,
		);
		if (this.inStateOrTransition(NodeState.Ended) || !this.checkValidNode())
			return;

		if (!this.isTransition(this.eCurrentState, NodeState.Ended))
			window.app.console.log(
				'BaseNode.end: end state not reachable in transition table',
			);

		this.DBG(this.callInfo('end'), getCurrentSystemTime());

		const aStateTrans = new StateTransition(this);
		if (aStateTrans.enter(NodeState.Ended, true /* FORCE */)) {
			this.deactivate_st(NodeState.Ended);
			aStateTrans.commit();

			// if is FROZEN or is to be FROZEN, then
			// will/already notified deactivating listeners
			if (!bIsFrozenOrInTransitionToFrozen) this.notifyEndListeners();
			if (this.bIsFirstAutoEffect)
				this.aNodeContext._context.aSlideShowHandler.notifyFirstAutoEffectEnded();

			if (this.aActivationEvent) this.aActivationEvent.dispose();
			if (this.aDeactivationEvent) this.aDeactivationEvent.dispose();
		}
	}

	public dispose() {
		if (this.aActivationEvent) this.aActivationEvent.dispose();
		if (this.aDeactivationEvent) this.aDeactivationEvent.dispose();
		this.aDeactivatingListenerArray = [];
	}

	public getState() {
		return this.eCurrentState;
	}

	public setState(nodeState: NodeState) {
		this.eCurrentState = nodeState;
	}

	public registerDeactivatingListener(aNotifiee: BaseNode) {
		if (!this.checkValidNode()) return false;

		if (!aNotifiee) {
			window.app.console.log(
				'BaseNode.registerDeactivatingListener(): invalid notifiee',
			);
			return false;
		}
		this.aDeactivatingListenerArray.push(aNotifiee);

		return true;
	}

	public notifyDeactivating(aNotifier: BaseNode) {
		assert(
			aNotifier.getState() === NodeState.Frozen ||
				aNotifier.getState() === NodeState.Ended,
			'BaseNode.notifyDeactivating: Notifier node is neither in FROZEN nor in ENDED state',
		);
	}

	public isContainer(): boolean {
		return this.bIsContainer;
	}

	public getPresetLabel(): string | undefined {
		return this.aNodeInfo.presetLabel;
	}

	public getTargetAnimatedElement(): AnimatedElement {
		return null;
	}

	public isMainSequenceRootNode(): boolean {
		return this.bIsMainSequenceRootNode;
	}

	public isInteractiveSequenceRootNode(): boolean {
		return this.bIsInteractiveSequenceRootNode;
	}

	public makeDeactivationEvent(nDelay: number): EventBase {
		if (this.aDeactivationEvent) {
			this.aDeactivationEvent.charge();
		} else if (typeof nDelay == typeof 0) {
			this.aDeactivationEvent = makeDelay(this.deactivate.bind(this), nDelay);
		} else {
			this.aDeactivationEvent = null;
		}
		return this.aDeactivationEvent;
	}

	public scheduleDeactivationEvent(aEvent?: EventBase) {
		this.DBG(this.callInfo('scheduleDeactivationEvent'));

		if (!aEvent) {
			if (this.getDuration() && this.getDuration().isValue())
				aEvent = this.makeDeactivationEvent(this.getDuration().getValue());
		}
		if (aEvent) {
			this._context.aTimerEventQueue.addEvent(aEvent);
		}
	}

	public checkValidNode() {
		return this.eCurrentState !== NodeState.Invalid;
	}

	protected init_st() {
		return true;
	}

	protected resolve_st() {
		return true;
	}

	protected activate_st() {
		this.scheduleDeactivationEvent();
	}

	protected deactivate_st(aNodeState?: NodeState) {
		// empty body
	}

	public notifyEndListeners() {
		const nDeactivatingListenerCount = this.aDeactivatingListenerArray.length;

		for (let i = 0; i < nDeactivatingListenerCount; ++i) {
			this.aDeactivatingListenerArray[i].notifyDeactivating(this);
		}

		this._context.aEventMultiplexer.notifyEvent(
			EventTrigger.EndEvent,
			this.getId(),
		);
		if (this.getParentNode() && this.getParentNode().isMainSequenceRootNode())
			this._context.aEventMultiplexer.notifyNextEffectEndEvent();

		if (this.isMainSequenceRootNode())
			this._context.aEventMultiplexer.notifyAnimationsEndEvent();
	}

	public getContext() {
		return this._context;
	}

	public isTransition(eFromState: NodeState, eToState: NodeState) {
		return (this.aStateTransTable.get(eFromState) & eToState) !== 0;
	}

	public inStateOrTransition(nMask: number) {
		return (
			(this.eCurrentState & nMask) != 0 ||
			(this.nCurrentStateTransition & nMask) != 0
		);
	}

	public getBegin(): Timing {
		return this.aBegin;
	}

	public getEnd(): Timing {
		return this.aEnd;
	}

	public getDuration(): Duration {
		return this.aDuration;
	}

	public getFillMode() {
		return this.eFillMode;
	}

	public getRestartMode() {
		return this.eRestartMode;
	}

	public getRepeatCount() {
		return this.nRepeatCount;
	}

	public getAccelerateValue() {
		return this.nAccelerate;
	}

	public getDecelerateValue() {
		return this.nDecelerate;
	}

	public isAutoReverseEnabled() {
		return this.bAutoReverse;
	}

	public info(bVerbose: boolean = false) {
		let sInfo = `${this.sClassName}(${this.aNodeInfo.nodeName}, ${this.getId()}, ${NodeState[this.getState()]})`;

		if (bVerbose) {
			// is container
			sInfo += ';  is container: ' + this.isContainer();

			// begin
			if (this.getBegin()) {
				if (this.getBegin().getEventType() === EventTrigger.OnNext)
					// on click starts the next effect
					sInfo += ';  \x1B[31mbegin: ' + this.getBegin().info() + '\x1B[m';
				else sInfo += ';  begin: ' + this.getBegin().info();
			}

			// duration
			if (this.getDuration()) sInfo += ';  dur: ' + this.getDuration().info();

			// end
			if (this.getEnd()) sInfo += ';  end: ' + this.getEnd().info();

			// fill mode
			if (this.getFillMode())
				sInfo += ';  fill: ' + FillModes[this.getFillMode()];

			// restart mode
			if (this.getRestartMode())
				sInfo += ';  restart: ' + RestartMode[this.getRestartMode()];

			// repeatCount
			if (this.getRepeatCount() && this.getRepeatCount() != 1.0)
				sInfo += ';  repeatCount: ' + this.getRepeatCount();

			// preset id (effect type)
			if (this.aNodeInfo.presetId)
				sInfo += `; \x1B[31mpresetId: ${this.aNodeInfo.presetId}\x1B[m`;

			// preset subType
			if (this.aNodeInfo.presetSubType)
				sInfo += `; presetSubType: ${this.aNodeInfo.presetSubType}`;

			// accelerate
			if (this.getAccelerateValue())
				sInfo += ';  accelerate: ' + this.getAccelerateValue();

			// decelerate
			if (this.getDecelerateValue())
				sInfo += ';  decelerate: ' + this.getDecelerateValue();

			// auto reverse
			if (this.isAutoReverseEnabled()) sInfo += ';  autoReverse: true';
		}

		return sInfo;
	}

	callInfo(sMethodName: string) {
		const sInfo =
			this.sClassName +
			'( ' +
			this.getId() +
			', ' +
			NodeState[this.getState()] +
			' ).' +
			sMethodName;
		return sInfo;
	}

	DBG(sMessage: string, nTime?: number) {
		ANIMDBG.print(sMessage, nTime);
	}

	public abstract hasPendingAnimation(): boolean;
	public abstract removeEffect(): void;
	public abstract saveStateOfAnimatedElement(): void;
}
