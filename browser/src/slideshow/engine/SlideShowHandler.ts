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

type InteractiveAnimationSequenceMap = Map<
	number,
	InteractiveAnimationSequence
>;

class SlideShowContext {
	public aSlideShowHandler: SlideShowHandler;
	public aTimerEventQueue: TimerEventQueue;
	public aEventMultiplexer: EventMultiplexer;
	public aNextEffectEventArray: NextEffectEventArray;
	public aInteractiveAnimationSequenceMap: InteractiveAnimationSequenceMap;
	public aActivityQueue: ActivityQueue;
	public bIsSkipping: boolean;
	public nSlideWidth: number;
	public nSlideHeight: number;
	public _canvas: HTMLCanvasElement;

	constructor(
		aSlideShowHandler: SlideShowHandler,
		aTimerEventQueue: TimerEventQueue,
		aEventMultiplexer: EventMultiplexer,
		aNextEffectEventArray: NextEffectEventArray,
		aInteractiveAnimationSequenceMap: InteractiveAnimationSequenceMap,
		aActivityQueue: ActivityQueue,
	) {
		this.aSlideShowHandler = aSlideShowHandler;
		this.aTimerEventQueue = aTimerEventQueue;
		this.aEventMultiplexer = aEventMultiplexer;
		this.aNextEffectEventArray = aNextEffectEventArray;
		this.aInteractiveAnimationSequenceMap = aInteractiveAnimationSequenceMap;
		this.aActivityQueue = aActivityQueue;
		this.bIsSkipping = false;
	}
}

class SlideShowHandler {
	public static readonly MAXIMUM_FRAME_COUNT: number = 120;
	public static readonly MINIMUM_TIMEOUT: number =
		1.0 / SlideShowHandler.MAXIMUM_FRAME_COUNT;
	public static readonly MAXIMUM_TIMEOUT: number = 4.0;
	public static readonly MINIMUM_FRAMES_PER_SECONDS: number = 10;
	public static readonly PREFERRED_FRAMES_PER_SECONDS: number = 60;
	public static readonly PREFERRED_FRAME_RATE: number =
		1.0 / SlideShowHandler.PREFERRED_FRAMES_PER_SECONDS;

	private theMetaPres: MetaPresentation;
	private slideShowNavigator: SlideShowNavigator;
	private presenter: SlideShowPresenter;
	private aTimer: ElapsedTime;
	private aFrameSynchronization: FrameSynchronization;
	private aTimerEventQueue: TimerEventQueue;
	private aActivityQueue: ActivityQueue;
	private aNextEffectEventArray: NextEffectEventArray;
	private aInteractiveAnimationSequenceMap: InteractiveAnimationSequenceMap;
	private aEventMultiplexer: EventMultiplexer;
	private _context: SlideShowContext;
	private bIsIdle: boolean;
	private bIsEnabled: boolean;
	private bNoSlideTransition: boolean;
	private bIsTransitionRunning: boolean;
	private nCurrentEffect: number;
	private bIsNextEffectRunning: boolean;
	private bIsRewinding: boolean;
	private bIsSkipping: boolean;
	private bIsSkippingAll: boolean;
	private nTotalInteractivePlayingEffects: number;
	private aStartedEffectList: Effect[];
	private aStartedEffectIndexMap: Map<number, number | undefined>;
	private automaticAdvanceTimeout: number | { rewindedEffect: number };
	// True while the countdown to the next slide is held, across slide changes too.
	private bIsAutomaticAdvancePaused: boolean = false;
	// Milliseconds still to wait before the show moves on by itself.
	private nAutomaticAdvanceRemaining: number = 0;
	private nAutomaticAdvanceStartTime: number = 0;
	private enteringSlideTexture: WebGLTexture | ImageBitmap;
	public isStarting: boolean;
	private bIsFirstAutoEffectRunning: boolean = false;
	private static readonly A11Y_TRANSITION_DELAY: number = 1000;
	private static readonly A11Y_SLIDE_CONTENT_DELAY: number = 2000;
	private aPendingA11yTimeouts: number[] = [];
	private transitionsWithMipMapEnabled = new Set([
		TransitionSubType.CORNERSOUT,
		TransitionSubType.TOPTOBOTTOM,
		TransitionSubType.LEFTTORIGHT,
		TransitionSubType.BOTTOMRIGHT,
		TransitionSubType.BOTTOMLEFT,
		TransitionSubType.TOPCENTER,
		TransitionSubType.CORNERSIN,
		TransitionSubType.TOPLEFT,
		TransitionSubType.CIRCLE,
		TransitionSubType.FANOUTHORIZONTAL,
		TransitionSubType.ACROSS,
		TransitionSubType.DIAMOND,
		TransitionSubType.HEART,
	]);

	private _labelMap: Record<string, string> = {
		rewind: _('Previous'),
		dispatch: _('Next'),
		skip: _('Skip'),
		rewindAll: _('First'),
		skipAll: _('Skip All'),
		firstPage: _('First Slide'),
		nextPage: _('Next Slide'),
		prevPage: _('Previous Slide'),
		lastPage: _('Last Slide'),
		quit: _('End Show'),
		pauseShow: _('Paused'),
		resumeShow: _('Resumed'),
	};

	constructor(presenter: SlideShowPresenter) {
		this.presenter = presenter;

		this.aTimer = new ElapsedTime();
		this.aFrameSynchronization = new FrameSynchronization(
			SlideShowHandler.PREFERRED_FRAME_RATE,
		);
		this.aTimerEventQueue = new TimerEventQueue(this.aTimer);
		this.aActivityQueue = new ActivityQueue(this.aTimer);
		this.aNextEffectEventArray = null;
		this.aInteractiveAnimationSequenceMap = null;
		this.aEventMultiplexer = null;

		this._context = new SlideShowContext(
			this,
			this.aTimerEventQueue,
			this.aEventMultiplexer,
			this.aNextEffectEventArray,
			this.aInteractiveAnimationSequenceMap,
			this.aActivityQueue,
		);

		this.bIsIdle = true;
		this.bIsEnabled = true;
		this.bNoSlideTransition = false;
		this.bIsTransitionRunning = false;

		this.nCurrentEffect = 0;
		this.bIsNextEffectRunning = false;
		this.bIsRewinding = false;
		this.bIsSkipping = false;
		this.bIsSkippingAll = false;
		this.nTotalInteractivePlayingEffects = 0;
		this.aStartedEffectList = [];
		this.aStartedEffectIndexMap = new Map();
		this.aStartedEffectIndexMap.set(-1, undefined);
		this.automaticAdvanceTimeout = null;
	}

	private get automaticAdvanceTimeoutRewindedEffect(): number {
		const automaticAdvanceTimeout = this.automaticAdvanceTimeout as {
			rewindedEffect: number;
		};
		return automaticAdvanceTimeout.rewindedEffect;
	}

	setMetaPresentation(metaPres: MetaPresentation) {
		this.theMetaPres = metaPres;
	}

	setNavigator(slideShowNavigator: SlideShowNavigator) {
		this.slideShowNavigator = slideShowNavigator;
	}

	isGlSupported(): boolean {
		return !this.presenter._slideRenderer._context.is2dGl();
	}

	setSlideEvents(
		aNextEffectEventArray: NextEffectEventArray,
		aInteractiveAnimationSequenceMap: InteractiveAnimationSequenceMap,
		aEventMultiplexer: EventMultiplexer,
	) {
		if (!aNextEffectEventArray)
			window.app.console.log(
				'SlideShow.setSlideEvents: aNextEffectEventArray is not valid',
			);

		if (!aInteractiveAnimationSequenceMap)
			window.app.console.log(
				'SlideShow.setSlideEvents:aInteractiveAnimationSequenceMap  is not valid',
			);

		if (!aEventMultiplexer)
			window.app.console.log(
				'SlideShow.setSlideEvents: aEventMultiplexer is not valid',
			);

		this._context.aNextEffectEventArray = aNextEffectEventArray;
		this.aNextEffectEventArray = aNextEffectEventArray;
		this._context.aInteractiveAnimationSequenceMap =
			aInteractiveAnimationSequenceMap;
		this.aInteractiveAnimationSequenceMap = aInteractiveAnimationSequenceMap;
		this._context.aEventMultiplexer = aEventMultiplexer;
		this.aEventMultiplexer = aEventMultiplexer;
		this.nCurrentEffect = 0;
	}

	createSlideTransition(
		aSlideTransitionHandler: SlideTransition,
		transitionParameters: TransitionParameters,
		aTransitionEndEvent: DelayEvent,
	): SimpleActivity {
		if (this.bNoSlideTransition) return null;

		const aSlideTransition =
			aSlideTransitionHandler.createSlideTransition(transitionParameters);
		if (!aSlideTransition) return null;

		let nDuration = 0.001;
		if (aSlideTransitionHandler.getDuration().isValue()) {
			nDuration = aSlideTransitionHandler.getDuration().getValue();
		} else {
			window.app.console.log(
				'SlideShow.createSlideTransition: duration is not a number',
			);
		}

		const aCommonParameterSet = new ActivityParamSet();
		aCommonParameterSet.aEndEvent = aTransitionEndEvent;
		aCommonParameterSet.aTimerEventQueue = this.aTimerEventQueue;
		aCommonParameterSet.aActivityQueue = this.aActivityQueue;
		aCommonParameterSet.nMinDuration = nDuration;
		aCommonParameterSet.nMinNumberOfFrames =
			aSlideTransitionHandler.getMinFrameCount();
		aCommonParameterSet.nSlideWidth = this.theMetaPres.getDocWidth();
		aCommonParameterSet.nSlideHeight = this.theMetaPres.getDocHeight();

		return new SimpleActivity(
			aCommonParameterSet,
			aSlideTransition,
			DirectionType.Forward,
		);
	}

	addA11yString(a11yString: string) {
		this.clearPendingA11yStrings();
		this.writeA11yString(a11yString);
	}

	addA11yStringDelayed(a11yString: string, nDelay: number) {
		const nTimeout = window.setTimeout(() => {
			this.aPendingA11yTimeouts = this.aPendingA11yTimeouts.filter(
				(nPending: number) => nPending !== nTimeout,
			);
			this.writeA11yString(a11yString);
		}, nDelay);
		this.aPendingA11yTimeouts.push(nTimeout);
	}

	clearPendingA11yStrings() {
		this.aPendingA11yTimeouts.forEach((nTimeout: number) =>
			clearTimeout(nTimeout),
		);
		this.aPendingA11yTimeouts = [];
	}

	private writeA11yString(a11yString: string) {
		if (this.presenter._enableA11y) {
			const canvas = this.getContext()._canvas;
			if (canvas) {
				const a11yContainer = window.L.DomUtil.create('div', '');
				a11yContainer.tabIndex = -1;
				canvas.innerHTML = '';
				a11yContainer.innerHTML = app.LOUtil.sanitize(a11yString);
				canvas.appendChild(a11yContainer);
			}
		}
	}

	isEnabled() {
		return this.bIsEnabled;
	}

	disable() {
		this.bIsEnabled = false;
		this.dispose();
	}

	isRunning() {
		return !this.bIsIdle;
	}

	isTransitionPlaying() {
		return this.bIsTransitionRunning;
	}

	isMainEffectPlaying() {
		return this.bIsNextEffectRunning;
	}

	isInteractiveEffectPlaying() {
		return this.nTotalInteractivePlayingEffects > 0;
	}

	isAnyEffectPlaying() {
		return this.isMainEffectPlaying() || this.isInteractiveEffectPlaying();
	}

	hasAnyEffectStarted() {
		return this.aStartedEffectList.length > 0;
	}

	getCurrentEffect(): number {
		return this.nCurrentEffect;
	}

	notifyNextEffectStart() {
		assert(
			!this.bIsNextEffectRunning,
			'SlideShowHandler.notifyNextEffectStart: an effect is already started.',
		);

		ANIMDBG.print('SlideShowHandler.notifyNextEffectStart invoked.');
		this.bIsNextEffectRunning = true;
		this.aEventMultiplexer.registerNextEffectEndHandler(
			this.notifyNextEffectEnd.bind(this),
		);
		const aEffect = new Effect();
		aEffect.start();
		this.aStartedEffectIndexMap.set(-1, this.aStartedEffectList.length);
		this.aStartedEffectList.push(aEffect);

		const sCurSlideHash = this.theMetaPres.getCurrentSlideHash();
		const curMetaSlide = this.theMetaPres.getMetaSlide(sCurSlideHash);
		if (curMetaSlide?.animationsHandler) {
			const aAnimatedElementMap =
				curMetaSlide.animationsHandler.getAnimatedElementMap();
			const currentEffect = this.nCurrentEffect;
			aAnimatedElementMap.forEach((aAnimatedElement: AnimatedElement) => {
				aAnimatedElement.notifyNextEffectStart(currentEffect);
			});
		}

		this.announceEffect(
			this.aNextEffectEventArray?.nodeAt(this.nCurrentEffect),
		);
	}

	private announceEffect(aEffectNode: BaseNode) {
		const sPresetLabel = aEffectNode?.getPresetLabel();
		const sObjectName = aEffectNode?.getTargetAnimatedElement()?.getTitle();

		let sAnnouncement;
		if (sPresetLabel && sObjectName)
			sAnnouncement = _('Animation: {0}, {1}')
				.replace('{0}', sPresetLabel)
				.replace('{1}', sObjectName);
		else if (sPresetLabel || sObjectName)
			sAnnouncement = _('Animation: {0}').replace(
				'{0}',
				sPresetLabel || sObjectName,
			);
		else sAnnouncement = _('Animation');

		this.addA11yStringDelayed(sAnnouncement, 500);
	}

	notifyNextEffectEnd() {
		assert(
			this.bIsNextEffectRunning,
			'SlideShow.notifyNextEffectEnd: effect already ended.',
		);

		ANIMDBG.print('SlideShowHandler.notifyNextEffectEnd invoked.');
		this.bIsNextEffectRunning = false;

		const effectIndex = this.aStartedEffectIndexMap.get(-1);
		if (effectIndex !== undefined && this.aStartedEffectList[effectIndex]) {
			this.aStartedEffectList[effectIndex].end();
		}
		if (this.automaticAdvanceTimeout !== null) {
			if (this.automaticAdvanceTimeoutRewindedEffect === this.nCurrentEffect) {
				this.automaticAdvanceTimeout = null;
				this.notifyAnimationsEnd();
			}
		}
	}

	notifyAnimationsEnd() {
		ANIMDBG.print(
			'SlideShowHandler.notifyAnimationsEnd: current slide index: ' +
				this.slideShowNavigator.currentSlideIndex,
		);
		const sCurrSlideHash = this.theMetaPres.getCurrentSlideHash();

		if (
			this.theMetaPres.isLastSlide(sCurrSlideHash) &&
			!this.presenter._isWelcomePresentation
		)
			return;

		assert(
			this.automaticAdvanceTimeout === null,
			'SlideShow.notifyAnimationsEnd: Timeout already set.',
		);

		const slideInfo = this.theMetaPres.getSlideInfo(sCurrSlideHash);

		if (slideInfo?.nextSlideDuration && slideInfo.nextSlideDuration > 0) {
			this.startAutomaticAdvanceTimeout(slideInfo.nextSlideDuration);
		}
	}

	/** startAutomaticAdvanceTimeout
	 *  Wait the given number of milliseconds and then move on to the next
	 *  slide. While paused the wait is only recorded, ready to be started
	 *  once the show is resumed.
	 */
	private startAutomaticAdvanceTimeout(nDuration: number) {
		this.nAutomaticAdvanceRemaining = nDuration;
		if (this.bIsAutomaticAdvancePaused) return;

		this.nAutomaticAdvanceStartTime = Date.now();
		this.automaticAdvanceTimeout = window.setTimeout(
			this.slideShowNavigator.switchSlide.bind(
				this.slideShowNavigator,
				1,
				false,
			),
			nDuration,
		);
	}

	/** toggleAutomaticAdvancePause
	 *  Hold or release the countdown that moves the show on to the next slide
	 *  by itself. Releasing waits out only the time that was left.
	 */
	toggleAutomaticAdvancePause() {
		if (this.bIsAutomaticAdvancePaused) {
			this.bIsAutomaticAdvancePaused = false;
			this.addA11yString(this._labelMap['resumeShow']);
			if (this.nAutomaticAdvanceRemaining > 0)
				this.startAutomaticAdvanceTimeout(this.nAutomaticAdvanceRemaining);
			return;
		}

		this.bIsAutomaticAdvancePaused = true;
		this.addA11yString(this._labelMap['pauseShow']);
		if (typeof this.automaticAdvanceTimeout === 'number') {
			clearTimeout(this.automaticAdvanceTimeout);
			this.automaticAdvanceTimeout = null;
			const nWaited = Date.now() - this.nAutomaticAdvanceStartTime;
			this.nAutomaticAdvanceRemaining = Math.max(
				0,
				this.nAutomaticAdvanceRemaining - nWaited,
			);
		}
	}

	notifySlideStart(nNewSlideIndex: number, nOldSlideIndex: number) {
		// Each slide has its own wait, so the one recorded for the slide before is
		// dropped here. The hold itself stays on until the user releases it.
		this.nAutomaticAdvanceRemaining = 0;
		this.nCurrentEffect = 0;
		this.bIsNextEffectRunning = false;
		this.bIsRewinding = false;
		this.bIsSkipping = false;
		this.bIsSkippingAll = false;
		this.nTotalInteractivePlayingEffects = 0;
		this.aStartedEffectList = [];
		this.aStartedEffectIndexMap = new Map();
		this.aStartedEffectIndexMap.set(-1, undefined);

		if (nOldSlideIndex !== undefined) {
			const metaOldSlide = this.theMetaPres.getMetaSlideByIndex(nOldSlideIndex);
			if (metaOldSlide?.animationsHandler) {
				const aAnimatedElementMap =
					metaOldSlide.animationsHandler.getAnimatedElementMap();

				aAnimatedElementMap.forEach((aAnimatedElement: AnimatedElement) => {
					aAnimatedElement.notifySlideEnd();
				});
			}
		}
		const metaNewSlide = this.theMetaPres.getMetaSlideByIndex(nNewSlideIndex);
		if (metaNewSlide?.animationsHandler) {
			const aAnimatedElementMap =
				metaNewSlide.animationsHandler.getAnimatedElementMap();

			aAnimatedElementMap.forEach((aAnimatedElement: AnimatedElement) => {
				aAnimatedElement.notifySlideStart(this._context);
			});
		}
		this.slideCompositor.notifyTransitionStart();
		this.presenter._map.fire('transitionstart', { slide: nNewSlideIndex });

		const slideInfo = this.getSlideInfo(nNewSlideIndex);
		if (slideInfo.transitionLabel) {
			this.addA11yStringDelayed(
				_('Transition: {0}').replace('{0}', slideInfo.transitionLabel),
				SlideShowHandler.A11Y_TRANSITION_DELAY,
			);
		}
	}

	notifyTransitionEnd(nNewSlide: number, nOldSlide: number | undefined) {
		NAVDBG.print(
			'SlideShowHandler.notifyTransitionEnd: nNewSlide: ' +
				nNewSlide +
				', nOldSlide: ' +
				nOldSlide +
				', this.bIsRewinding: ' +
				this.bIsRewinding,
		);

		this.bIsTransitionRunning = false;
		if (!this.presenter._checkAlreadyPresenting()) return;
		if (this.bIsRewinding) {
			this.theMetaPres.getMetaSlideByIndex(nNewSlide).hide();
			this.slideShowNavigator.rewindToPreviousSlide();
			this.bIsRewinding = false;
			return;
		}

		const sCurSlideHash = this.theMetaPres.getCurrentSlideHash();
		this.slideCompositor.notifyTransitionEnd(sCurSlideHash);

		try {
			this.presentSlide(nNewSlide);
		} catch (message) {
			app.console.error('notifyTransitionEnd: ' + message);
		}

		this.presenter._map.fire('transitionend', { slide: nNewSlide });

		this.enteringSlideTexture = null;
		this.isStarting = false;

		if (this.isEnabled()) {
			// clear all queues
			this.dispose();

			const aCurrentSlide = this.theMetaPres.getMetaSlide(sCurSlideHash);
			if (
				aCurrentSlide &&
				aCurrentSlide.animationsHandler &&
				aCurrentSlide.animationsHandler.elementsParsed()
			) {
				aCurrentSlide.animationsHandler.start();
				this.aEventMultiplexer.registerAnimationsEndHandler(
					this.notifyAnimationsEnd.bind(this),
				);
			} else this.notifyAnimationsEnd();

			this.update();
		} else this.notifyAnimationsEnd();
	}

	notifyInteractiveAnimationSequenceStart(nNodeId: number) {
		++this.nTotalInteractivePlayingEffects;
		const aEffect = new Effect(nNodeId);
		aEffect.start();
		this.aStartedEffectIndexMap.set(nNodeId, this.aStartedEffectList.length);
		this.aStartedEffectList.push(aEffect);
	}

	notifyInteractiveAnimationSequenceEnd(nNodeId: number) {
		assert(
			this.isInteractiveEffectPlaying(),
			'SlideShow.notifyInteractiveAnimationSequenceEnd: no interactive effect playing.',
		);

		const effectIndex = this.aStartedEffectIndexMap.get(nNodeId);
		if (effectIndex !== undefined && this.aStartedEffectList[effectIndex]) {
			this.aStartedEffectList[effectIndex].end();
		}
		--this.nTotalInteractivePlayingEffects;
	}

	/** nextEffect
	 *  Start the next effect belonging to the main animation sequence if any.
	 *  If there is an already playing effect belonging to any animation sequence
	 *  it is skipped.
	 *
	 *  @return {Boolean}
	 *      False if there is no more effect to start, true otherwise.
	 */
	nextEffect(): boolean {
		if (!this.isEnabled()) return false;

		this.addA11yString(this._labelMap['dispatch']);

		if (this.isTransitionPlaying()) {
			this.skipTransition();
			return true;
		}

		if (this.isFirstAutoEffectRunning()) {
			this.skipFirstAutoEffect();
			return true;
		}

		ANIMDBG.print(
			`SlideShowHandler.nextEffect: current effect: ${this.nCurrentEffect}`,
		);
		if (this.isAnyEffectPlaying()) {
			this.skipAllPlayingEffects();
			return true;
		}

		if (!this.aNextEffectEventArray) return false;

		if (this.nCurrentEffect >= this.aNextEffectEventArray.size()) return false;

		this.notifyNextEffectStart();

		this.aNextEffectEventArray.at(this.nCurrentEffect).fire();
		++this.nCurrentEffect;
		this.presenter.sendSlideShowFollowMessage(
			'effect ' + JSON.stringify({ currentEffect: this.nCurrentEffect }),
		);
		this.update();
		return true;
	}

	/** skipTransition
	 *  Skip the current playing slide transition.
	 */
	skipTransition() {
		if (this.bIsSkipping || this.bIsRewinding) return;

		this.bIsSkipping = true;

		this.aActivityQueue.endAll();
		this.aTimerEventQueue.forceEmpty();
		this.aActivityQueue.endAll();
		this.update();
		this.bIsSkipping = false;
	}

	/** skipAllPlayingEffects
	 *  Skip all playing effect, independently to which animation sequence they
	 *  belong.
	 *
	 */
	skipAllPlayingEffects() {
		this.addA11yString(this._labelMap['skipAll']);

		if (this.bIsSkipping || this.bIsRewinding) return true;

		this.bIsSkipping = true;
		// TODO: The correct order should be based on the left playing time.
		for (let i = 0; i < this.aStartedEffectList.length; ++i) {
			const aEffect = this.aStartedEffectList[i];
			if (aEffect.isPlaying()) {
				if (aEffect.isMainEffect())
					this.aEventMultiplexer.notifySkipEffectEvent();
				else
					this.aEventMultiplexer.notifySkipInteractiveEffectEvent(
						aEffect.getId(),
					);
			}
		}
		this.update();
		this.bIsSkipping = false;
		return true;
	}

	/** skipNextEffect
	 *  Skip the next effect to be played (if any) that belongs to the main
	 *  animation sequence.
	 *  Require: no effect is playing.
	 *
	 *  @return {Boolean}
	 *      False if there is no more effect to skip, true otherwise.
	 */
	skipNextEffect(): boolean {
		if (this.bIsSkipping || this.bIsRewinding) return true;
		ANIMDBG.print(
			`SlideShowHandler.skipNextEffect: current effect: ${this.nCurrentEffect}`,
		);

		assert(!this.isAnyEffectPlaying(), 'SlideShowHandler.skipNextEffect');

		if (!this.aNextEffectEventArray) return false;

		if (this.nCurrentEffect >= this.aNextEffectEventArray.size()) return false;

		this.notifyNextEffectStart();

		this.bIsSkipping = true;
		this.aNextEffectEventArray.at(this.nCurrentEffect).fire();
		this.aEventMultiplexer.notifySkipEffectEvent();
		++this.nCurrentEffect;
		this.presenter.sendSlideShowFollowMessage(
			'effect ' + JSON.stringify({ currentEffect: this.nCurrentEffect }),
		);
		this.update();
		this.bIsSkipping = false;
		return true;
	}

	skipNEffects(nEffectNumber: number) {
		for (let i = 0; i < nEffectNumber; i++) if (!this.skipNextEffect()) break;
	}

	/** skipPlayingOrNextEffect
	 *  Skip the next effect to be played that belongs to the main animation
	 *  sequence  or all playing effects.
	 *
	 *  @return {Boolean}
	 *      False if there is no more effect to skip, true otherwise.
	 */
	skipPlayingOrNextEffect() {
		this.addA11yStringDelayed(this._labelMap['skip'], 600);

		if (this.isTransitionPlaying()) {
			this.skipTransition();
			return true;
		}

		if (this.isFirstAutoEffectRunning()) {
			this.skipFirstAutoEffect();
			return true;
		}

		if (this.isAnyEffectPlaying()) return this.skipAllPlayingEffects();
		else return this.skipNextEffect();
	}

	/** skipAllEffects
	 *  Skip all left effects that belongs to the main animation sequence and all
	 *  playing effects on the current slide.
	 *
	 *  @return {Boolean}
	 *      True if it is already skipping or when it has ended skipping,
	 *      false if the next slide needs to be displayed.
	 */
	skipAllEffects(): boolean {
		if (this.bIsSkippingAll) return true;

		this.bIsSkippingAll = true;

		if (this.isTransitionPlaying()) {
			this.skipTransition();
		}

		if (this.isFirstAutoEffectRunning()) {
			this.skipFirstAutoEffect();
		}

		if (this.isAnyEffectPlaying()) {
			this.skipAllPlayingEffects();
		} else if (
			!this.aNextEffectEventArray ||
			this.nCurrentEffect >= this.aNextEffectEventArray.size()
		) {
			this.bIsSkippingAll = false;
			return false;
		}

		// Pay attention here: a new next effect event is appended to
		// aNextEffectEventArray only after the related animation node has been
		// resolved, that is only after the animation node related to the previous
		// effect has notified to be deactivated to the main sequence time container.
		// So you should avoid any optimization here because the size of
		// aNextEffectEventArray will going on increasing after every skip action.
		while (this.nCurrentEffect < this.aNextEffectEventArray.size()) {
			this.skipNextEffect();
		}
		this.bIsSkippingAll = false;
		return true;
	}

	/** rewindTransition
	 * Rewind the current playing slide transition.
	 */
	rewindTransition() {
		if (this.bIsSkipping || this.bIsRewinding) return;

		this.bIsRewinding = true;
		this.aActivityQueue.endAll();
		this.update();
		this.bIsRewinding = false;
	}

	/** rewindEffect
	 *  Rewind all the effects started after at least one of the current playing
	 *  effects. If there is no playing effect, it rewinds the last played one,
	 *  both in case it belongs to the main or to an interactive animation sequence.
	 *
	 */
	rewindEffect() {
		if (this.bIsSkipping || this.bIsRewinding) return;

		if (
			this.automaticAdvanceTimeout !== null &&
			!this.automaticAdvanceTimeoutRewindedEffect
		) {
			clearTimeout(this.automaticAdvanceTimeout as number);
			this.automaticAdvanceTimeout = { rewindedEffect: this.nCurrentEffect };
		}

		this.addA11yString(this._labelMap['rewind']);

		if (!this.hasAnyEffectStarted()) {
			this.rewindToPreviousSlide();
			return;
		}

		this.bIsRewinding = true;

		let nFirstPlayingEffectIndex = undefined;

		let i = 0;
		for (; i < this.aStartedEffectList.length; ++i) {
			const aEffect = this.aStartedEffectList[i];
			if (aEffect.isPlaying()) {
				nFirstPlayingEffectIndex = i;
				break;
			}
		}

		// There is at least one playing effect.
		if (nFirstPlayingEffectIndex !== undefined) {
			i = this.aStartedEffectList.length - 1;
			for (; i >= nFirstPlayingEffectIndex; --i) {
				const aEffect = this.aStartedEffectList[i];
				if (aEffect.isPlaying()) {
					if (aEffect.isMainEffect()) {
						this.aEventMultiplexer.notifyRewindCurrentEffectEvent();
						if (this.nCurrentEffect > 0) --this.nCurrentEffect;
					} else {
						this.aEventMultiplexer.notifyRewindRunningInteractiveEffectEvent(
							aEffect.getId(),
						);
					}
				} else if (aEffect.isEnded()) {
					if (aEffect.isMainEffect()) {
						this.aEventMultiplexer.notifyRewindLastEffectEvent();
						if (this.nCurrentEffect > 0) --this.nCurrentEffect;
					} else {
						this.aEventMultiplexer.notifyRewindEndedInteractiveEffectEvent(
							aEffect.getId(),
						);
					}
				}
			}
			this.update();

			// Pay attention here: we need to remove all rewinded effects from
			// the started effect list only after updating.
			i = this.aStartedEffectList.length - 1;
			for (; i >= nFirstPlayingEffectIndex; --i) {
				const aEffect = this.aStartedEffectList.pop();
				if (!aEffect.isMainEffect())
					this.aStartedEffectIndexMap.delete(aEffect.getId());
			}
		} // there is no playing effect
		else {
			const aEffect = this.aStartedEffectList.pop();
			if (!aEffect.isMainEffect())
				this.aStartedEffectIndexMap.delete(aEffect.getId());
			if (aEffect.isEnded()) {
				// Well that is almost an assertion.
				if (aEffect.isMainEffect()) {
					this.aEventMultiplexer.notifyRewindLastEffectEvent();
					if (this.nCurrentEffect > 0) --this.nCurrentEffect;
				} else {
					this.aEventMultiplexer.notifyRewindEndedInteractiveEffectEvent(
						aEffect.getId(),
					);
				}
			}
			this.update();
		}

		this.bIsRewinding = false;
		this.presenter.sendSlideShowFollowMessage(
			'effect ' + JSON.stringify({ currentEffect: this.nCurrentEffect - 1 }),
		);
	}

	/** rewindToPreviousSlide
	 *  Displays the previous slide with all effects, that belong to the main
	 *  animation sequence, played.
	 *
	 */
	rewindToPreviousSlide() {
		NAVDBG.print('SlideShowHandler.rewindToPreviousSlide');
		if (this.isFirstAutoEffectRunning()) {
			this.rewindFirstAutoEffect();
		}

		if (this.isTransitionPlaying()) {
			this.rewindTransition();
			return;
		}
		if (this.isAnyEffectPlaying()) return;

		this.slideShowNavigator.rewindToPreviousSlide();
	}

	/** rewindAllEffects
	 *  Rewind all effects already played on the current slide.
	 *
	 */
	rewindAllEffects() {
		this.addA11yString(this._labelMap['rewindAll']);

		if (!this.hasAnyEffectStarted()) {
			this.rewindToPreviousSlide();
			return;
		}

		while (this.hasAnyEffectStarted()) {
			this.rewindEffect();
		}
	}

	cleanLeavingSlideStatus(nOldSlide: number, bSkipSlideTransition: boolean) {
		const aMetaDoc = this.theMetaPres;
		if (nOldSlide !== undefined) {
			this.slideCompositor.pauseVideos(aMetaDoc.getSlideHash(nOldSlide));

			const oldMetaSlide = aMetaDoc.getMetaSlideByIndex(nOldSlide);
			if (this.isEnabled()) {
				if (
					oldMetaSlide.animationsHandler &&
					oldMetaSlide.animationsHandler.isAnimated()
				) {
					// force end animations
					oldMetaSlide.animationsHandler.end(bSkipSlideTransition);

					// clear all queues
					this.dispose();
				}
			}

			if (this.automaticAdvanceTimeout !== null) {
				clearTimeout(this.automaticAdvanceTimeout as number);
				this.automaticAdvanceTimeout = null;
			}
		}
	}

	// This method must be invoked by SlideShowNavigator.displaySlide only,
	// since we need to update the current slide index.
	displaySlide(
		nNewSlide: number,
		nOldSlide: number | undefined,
		bSkipSlideTransition: boolean,
		nSlideEffect: number = undefined,
	) {
		NAVDBG.print(
			'SlideShowHandler.displaySlide: nNewSlide: ' +
				nNewSlide +
				', nOldSlide: ' +
				nOldSlide,
		);
		const aMetaDoc = this.theMetaPres;
		if (nNewSlide >= aMetaDoc.numberOfSlides) {
			this.exitSlideShow();
		}

		this.clearPendingA11yStrings();
		if (this.theMetaPres.numberOfSlides - 1 == nNewSlide) {
			this.addA11yString(this._labelMap['lastPage']);
		} else if (nNewSlide == 0) {
			this.addA11yString(this._labelMap['firstPage']);
		} else if (nNewSlide == nOldSlide + 1) {
			this.addA11yString(this._labelMap['nextPage']);
		} else if (nNewSlide == nOldSlide - 1) {
			this.addA11yString(this._labelMap['prevPage']);
		}

		if (this.isTransitionPlaying()) {
			this.skipTransition();
		}

		if (this.isFirstAutoEffectRunning()) {
			this.skipFirstAutoEffect();
		}

		// handle current slide
		if (nOldSlide !== undefined) {
			this.cleanLeavingSlideStatus(nOldSlide, bSkipSlideTransition);
		}

		this.notifySlideStart(nNewSlide, nOldSlide);

		if (this.isEnabled() && this.isGlSupported() && !bSkipSlideTransition) {
			// create slide transition and add to activity queue
			if (
				(nOldSlide === undefined && this.isStarting) ||
				(nOldSlide !== undefined && nNewSlide > nOldSlide)
			) {
				const aNewMetaSlide = aMetaDoc.getMetaSlideByIndex(nNewSlide);
				const aSlideTransitionHandler = aNewMetaSlide.transitionHandler;
				if (aSlideTransitionHandler && aSlideTransitionHandler.isValid()) {
					const aTransitionEndEvent = makeEvent(
						this.notifyTransitionEnd.bind(this, nNewSlide, nOldSlide),
					);

					try {
						const transitionParameters: TransitionParameters =
							this.createTransitionParameters(nNewSlide, nOldSlide);
						this.enteringSlideTexture = transitionParameters.next;
						const aTransitionActivity = this.createSlideTransition(
							aSlideTransitionHandler,
							transitionParameters,
							aTransitionEndEvent,
						);

						if (aTransitionActivity) {
							this.bIsTransitionRunning = true;
							this.aActivityQueue.addActivity(aTransitionActivity);
							this.update();
							this.presenter.stopLoader();
							return;
						}
					} catch (message) {
						app.console.error('displaySlide failed: ' + message);
					}
				}
			}
		}

		this.notifyTransitionEnd(nNewSlide, nOldSlide);
		// can jump to specific animation only after everything is loaded
		if (nSlideEffect !== undefined) this.skipNEffects(nSlideEffect);
	}

	exitSlideShow() {
		// TODO: implement it;
		this.addA11yString(this._labelMap['quit']);
		this.automaticAdvanceTimeout = null;
		this.bIsAutomaticAdvancePaused = false;
		this.nAutomaticAdvanceRemaining = 0;
	}

	update() {
		this.aTimer.holdTimer();

		// process queues
		this.aTimerEventQueue.process();
		this.aActivityQueue.process();

		if (!this.bIsTransitionRunning) this.aFrameSynchronization.synchronize();

		this.aActivityQueue.processDequeued();

		this.aTimer.releaseTimer();

		const bActivitiesLeft = !this.aActivityQueue.isEmpty();
		const bTimerEventsLeft = !this.aTimerEventQueue.isEmpty();
		const bEventsLeft = bActivitiesLeft || bTimerEventsLeft;

		if (bEventsLeft) {
			let nNextTimeout;
			if (bActivitiesLeft) {
				nNextTimeout = SlideShowHandler.MINIMUM_TIMEOUT;
				this.aFrameSynchronization.activate();
			} else {
				nNextTimeout = this.aTimerEventQueue.nextTimeout();
				if (nNextTimeout < SlideShowHandler.MINIMUM_TIMEOUT)
					nNextTimeout = SlideShowHandler.MINIMUM_TIMEOUT;
				else if (nNextTimeout > SlideShowHandler.MAXIMUM_TIMEOUT)
					nNextTimeout = SlideShowHandler.MAXIMUM_TIMEOUT;
				this.aFrameSynchronization.deactivate();
			}

			this.bIsIdle = false;
			app.timerRegistry.setTimeout(
				'slideshowupdate',
				this.update.bind(this),
				nNextTimeout * 1000,
			);
		} else {
			this.bIsIdle = true;
		}
	}

	dispose() {
		// clear all queues
		this.aTimerEventQueue.clear();
		this.aActivityQueue.clear();
		this.aNextEffectEventArray = null;
		this.aEventMultiplexer = null;
		this.automaticAdvanceTimeout = null;
	}

	getContext() {
		return this._context;
	}

	private get slideRenderer(): SlideRenderer {
		return this.presenter._slideRenderer;
	}
	private get slideCompositor(): SlideCompositor {
		return this.presenter._slideCompositor;
	}

	getSlideInfo(nSlideIndex: number): SlideInfo {
		return this.theMetaPres.getSlideInfoByIndex(nSlideIndex);
	}

	getAnimatedLayerInfo(
		slideHash: string,
		targetElement: string,
	): AnimatedShapeInfo {
		return this.slideCompositor.getAnimatedLayerInfo(slideHash, targetElement);
	}

	private isMipMapsEnable(transitionFilterInfo: TransitionFilterInfo): boolean {
		return (
			transitionFilterInfo.transitionType === TransitionType.MISCSHAPEWIPE &&
			this.transitionsWithMipMapEnabled.has(
				transitionFilterInfo.transitionSubtype,
			)
		);
	}

	private getTexture(
		nSlideIndex: number,
		transitionFilterInfo?: TransitionFilterInfo,
	): WebGLTexture | ImageBitmap | null {
		const slideImage = this.slideCompositor.getSlide(nSlideIndex);
		if (!slideImage) {
			app.console.error('SlideShowHandler: cannot get texture');
			return null;
		}

		// added check for mipmap texture
		let isMipMapEnable = false;
		if (
			transitionFilterInfo &&
			transitionFilterInfo.transitionType &&
			transitionFilterInfo.transitionSubtype
		) {
			isMipMapEnable = this.isMipMapsEnable(transitionFilterInfo);
		}
		return this.slideRenderer.createTexture(slideImage, isMipMapEnable);
	}

	private presentSlide(nSlideIndex: number) {
		const slideInfo = this.getSlideInfo(nSlideIndex);
		if (slideInfo.a11y) {
			this.addA11yStringDelayed(
				slideInfo.a11y,
				SlideShowHandler.A11Y_SLIDE_CONTENT_DELAY,
			);
		}

		let slideTexture = this.enteringSlideTexture;
		if (!slideTexture) slideTexture = this.getTexture(nSlideIndex);
		this.slideRenderer.renderSlide(
			slideTexture,
			this.getSlideInfo(nSlideIndex),
		);
		this.presenter.stopLoader();
	}

	private createTransitionParameters(
		nNewSlide: number,
		nOldSlide: number,
	): TransitionParameters {
		let leavingSlideTexture = null;
		const transitionFilterInfo = TransitionFilterInfo.fromSlideInfo(
			this.getSlideInfo(nNewSlide),
		);
		if (this.isStarting) {
			leavingSlideTexture = this.slideRenderer.createEmptyTexture();
		} else {
			leavingSlideTexture =
				nOldSlide !== undefined &&
				this.slideRenderer.lastRenderedSlideIndex === nOldSlide
					? this.slideRenderer.getSlideTexture()
					: this.getTexture(nOldSlide, transitionFilterInfo);
		}
		const enteringSlideTexture = this.getTexture(
			nNewSlide,
			transitionFilterInfo,
		);
		const transitionParameters = new TransitionParameters();
		transitionParameters.context = this.slideRenderer._context;
		transitionParameters.current = leavingSlideTexture;
		transitionParameters.next = enteringSlideTexture;
		transitionParameters.transitionFilterInfo = transitionFilterInfo;

		return transitionParameters;
	}

	public notifyFirstAutoEffectStarted() {
		app.console.debug('SlideShowHandler.notifyFirstAutoEffectStarted');
		this.bIsFirstAutoEffectRunning = true;
	}

	public notifyFirstAutoEffectEnded() {
		app.console.debug('SlideShowHandler.notifyFirstAutoEffectEnded');
		this.bIsFirstAutoEffectRunning = false;
	}

	public isFirstAutoEffectRunning() {
		return this.bIsFirstAutoEffectRunning;
	}

	private skipFirstAutoEffect() {
		app.console.debug('SlideShowHandler.skipFirstAutoEffect');
		this.bIsSkipping = true;
		this.aEventMultiplexer.notifySkipEffectEvent();
		this.update();
		this.bIsSkipping = false;
		// empty body
	}

	private rewindFirstAutoEffect() {
		this.bIsRewinding = true;
		this.aEventMultiplexer.notifyRewindCurrentEffectEvent();
		this.update();
		this.bIsRewinding = false;
	}
}
