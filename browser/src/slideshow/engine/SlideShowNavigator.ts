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

class SlideShowNavigator {
	private theMetaPres: MetaPresentation = null;
	private slideShowHandler: SlideShowHandler;
	private presenter: SlideShowPresenter;
	private keyHandlerMap: Record<string, () => void>;
	private swipeHandlerMap: Record<string, () => void>;
	private _canvasClickHandler: MouseClickHandler;
	private currentSlide: number;
	private prevSlide: number;
	private isEnabled: boolean;
	private isRewindingToPrevSlide: boolean;
	private lastClickTime: number = 0;
	private readonly RAPID_CLICK_THRESHOLD = 500; // 500ms
	private currentLeaderSlide: number = -1;
	private currentLeaderEffect: number = 0;
	private isSlideLoading: boolean = false;
	private isEffectPending: boolean = false;

	constructor(slideShowHandler: SlideShowHandler) {
		this.slideShowHandler = slideShowHandler;
		this.currentSlide = undefined;
		this.prevSlide = undefined;
		this.isRewindingToPrevSlide = false;
		this.initKeyMap();
		this.addHandlers();
	}

	private initKeyMap() {
		this.keyHandlerMap = {
			ArrowLeft: this.rewindEffect.bind(this),
			ArrowRight: this.dispatchEffect.bind(this),
			ArrowUp: this.rewindEffect.bind(this),
			ArrowDown: this.skipEffect.bind(this),
			PageUp: this.rewindAllEffects.bind(this),
			PageDown: this.skipAllEffects.bind(this),
			Home: this.goToFirstSlide.bind(this),
			End: this.goToLastSlide.bind(this),
			Space: this.dispatchEffect.bind(this),
			Backspace: this.rewindEffect.bind(this),
			Escape: this.quit.bind(this),
		};
		this.swipeHandlerMap = {
			[Hammer.DIRECTION_RIGHT]: this.rewindEffect.bind(this),
			[Hammer.DIRECTION_LEFT]: this.dispatchEffect.bind(this),
			[Hammer.DIRECTION_UP]: this.quit.bind(this),
			[Hammer.DIRECTION_DOWN]: this.quit.bind(this),
		};
	}

	private addHandlers() {
		this._canvasClickHandler = {
			handleClick: this.clickHandler.bind(this),
		};
	}

	private removeHandlers() {
		this._canvasClickHandler.handleClick = null;
	}

	public setMetaPresentation(metaPres: MetaPresentation) {
		this.theMetaPres = metaPres;
	}

	public get currentSlideIndex(): number {
		return this.currentSlide;
	}

	public get canvasClickHandler(): MouseClickHandler {
		return this._canvasClickHandler;
	}

	dispatchEffect(userInitiated: boolean = true) {
		if (userInitiated && !this.canUserAdvanceEffect()) return;
		// Follower: drop dispatcheffect messages that arrive while a
		// slide is loading. The leader's subsequent displayslide /
		// effect messages are authoritative, and the fetchAndRun
		// callback's follower catch-up reconciles state after load.
		if (!userInitiated && this.isSlideLoading) return;
		// Leader: set isEffectPending so the fetchAndRun callback
		// replays the press as switchSlide(+1) after the load.
		// The replay itself starts a new load; presses that arrive
		// during that next load are absorbed by its own callback, so
		// rapid-click-through can still chain multiple advances across
		// successive loads.
		if (this.isSlideLoading) {
			this.isEffectPending = true;
			this.presenter.sendSlideShowFollowMessage('dispatcheffect');
			return;
		}
		this.presenter.sendSlideShowFollowMessage('dispatcheffect');
		const currentTime = Date.now();
		const timeDiff = currentTime - this.lastClickTime;
		NAVDBG.print(
			'SlideShowNavigator.dispatchEffect: current index: ' +
				this.currentSlide +
				', time diff: ' +
				timeDiff +
				'ms',
		);
		this.lastClickTime = currentTime;

		if (timeDiff < this.RAPID_CLICK_THRESHOLD && timeDiff > 0) {
			const bRet = this.slideShowHandler.skipPlayingOrNextEffect();
			if (!bRet) {
				this.switchSlide(1, true);
			}
			return;
		}

		const bRet = this.slideShowHandler.nextEffect();
		if (!bRet) {
			this.switchSlide(1, false);
		}
	}

	skipEffect() {
		if (!this.canUserAdvanceEffect()) return;
		NAVDBG.print(
			'SlideShowNavigator.skipEffect: current index: ' + this.currentSlide,
		);
		const bRet = this.slideShowHandler.skipPlayingOrNextEffect();
		if (!bRet) {
			if (
				this.presenter.isFollower() &&
				this.currentSlide >= this.getLeaderSlide()
			)
				return;
			this.switchSlide(1, true);
		}
	}

	skipAllEffects() {
		if (!this.canUserAdvanceEffect()) return;
		NAVDBG.print(
			'SlideShowNavigator.skipAllEffects: current index: ' + this.currentSlide,
		);
		if (this.presenter.isFollower()) {
			// skip only up to leader's current effect
			const nToSkip =
				this.currentLeaderEffect - this.slideShowHandler.getCurrentEffect();
			if (nToSkip > 0) this.slideShowHandler.skipNEffects(nToSkip);
			if (this.currentSlide < this.getLeaderSlide()) this.switchSlide(1, true);
			return;
		}
		const bRet = this.slideShowHandler.skipAllEffects();
		if (!bRet) {
			this.switchSlide(1, true);
		}
	}

	rewindEffect() {
		this.presenter.sendSlideShowFollowMessage('rewindeffect');
		NAVDBG.print(
			'SlideShowNavigator.rewindEffect: current index: ' + this.currentSlide,
		);
		if (this.backToLastSlide()) return;
		this.slideShowHandler.rewindEffect();
	}

	rewindAllEffects() {
		NAVDBG.print(
			'SlideShowNavigator.rewindAllEffects: current index: ' +
				this.currentSlide,
		);
		if (this.backToLastSlide()) return;
		this.slideShowHandler.rewindAllEffects();
	}

	canUserAdvanceEffect(): boolean {
		if (!this.presenter.isFollower()) return true;
		return (
			this.currentSlide < this.getLeaderSlide() ||
			this.slideShowHandler.getCurrentEffect() < this.currentLeaderEffect
		);
	}

	goToFirstSlide() {
		NAVDBG.print(
			'SlideShowNavigator.goToFirstSlide: current index: ' + this.currentSlide,
		);
		this.startPresentation(0, false);
	}

	goToLastSlide() {
		NAVDBG.print(
			'SlideShowNavigator.goToLastSlide: current index: ' + this.currentSlide,
		);
		const lastSlide = this.presenter.isFollower()
			? this.getLeaderSlide()
			: this.theMetaPres.numberOfSlides - 1;
		if (lastSlide < 0) return;
		this.displaySlide(lastSlide, true);
	}

	goToSlideAtBookmark(bookmark: string) {
		NAVDBG.print(
			'SlideShowNavigator.goToSlideAtBookmark: ' +
				bookmark +
				' current index: ' +
				this.currentSlide,
		);
		for (let i = 0; i < this.theMetaPres.numberOfSlides; i++) {
			const slideInfo = this.theMetaPres.getSlideInfo(
				this.theMetaPres.getSlideHash(i),
			);
			if (slideInfo.name == bookmark) {
				this.displaySlide(i, true);
				break;
			}
		}
	}

	private backToLastSlide(): boolean {
		if (this.currentSlide >= this.theMetaPres.numberOfSlides) {
			this.goToLastSlide();
			return true;
		}
		return false;
	}

	quit() {
		NAVDBG.print(
			'SlideShowNavigator.quit: current index: ' + this.currentSlide,
		);

		this.slideShowHandler.exitSlideShow();
		this.endPresentation(true);
		this.currentSlide = undefined;
		this.prevSlide = undefined;
		this.removeHandlers();
	}

	switchSlide(nOffset: number, bSkipTransition: boolean) {
		NAVDBG.print('SlideShowNavigator.switchSlide: nOffset: ' + nOffset);
		if (this.currentSlide === undefined) {
			NAVDBG.print('SlideShowNavigator.switchSlide: currentSlide undefined');
			return;
		}
		this.displaySlide(this.currentSlide + nOffset, bSkipTransition);
	}

	rewindToPreviousSlide() {
		// play again transition on first slide and any effect that starts
		// automatically after the first slide is displayed
		if (this.currentSlide === 0) this.goToFirstSlide();

		let prevSlide = 0;
		if (this.currentSlide !== undefined && this.currentSlide > 0) {
			prevSlide = this.currentSlide - 1;
		}
		NAVDBG.print(
			'SlideShowNavigator.rewindToPreviousSlide: slide to display: ' +
				prevSlide,
		);
		this.isRewindingToPrevSlide = true;
		this.displaySlide(prevSlide, true);
		this.isRewindingToPrevSlide = false;
		this.presenter.sendSlideShowFollowMessage('skipalleffect');
	}

	setLeaderSlide(info: any) {
		this.currentLeaderSlide = info.currentSlide;
	}

	getLeaderSlide(): number {
		return this.currentLeaderSlide;
	}

	resetLeaderSlide() {
		this.currentLeaderSlide = -1;
	}

	setLeaderEffect(info: any) {
		this.currentLeaderEffect = info.currentEffect;
	}

	getLeaderEffect(): number {
		return this.currentLeaderEffect;
	}

	resetLeaderEffect() {
		this.currentLeaderEffect = 0;
	}

	followLeaderDisplaySlide(nLeaderSlide: number) {
		if (nLeaderSlide === this.currentSlide) return;
		// If the follower is currently loading a slide, the catch-up
		// in the fetchAndRun callback will handle navigating to the
		// leader's slide. Starting a navigation here would overlap
		// with the in-flight fetchAndRun and cause rendering issues.
		if (this.isSlideLoading) return;
		this.displaySlide(nLeaderSlide, true);
	}

	followLeaderSlide() {
		if (this.presenter.isFollowing()) return;
		this.presenter.setFollowing(true);
		if (this.currentLeaderSlide === this.currentSlide) {
			if (this.slideShowHandler.hasAnyEffectStarted())
				this.slideShowHandler.rewindAllEffects();
			this.slideShowHandler.skipNEffects(this.currentLeaderEffect);
		} else {
			this.displaySlide(this.currentLeaderSlide, true);
		}
	}

	displaySlide(
		nNewSlide: number,
		bSkipTransition: boolean,
		nStartEffect: number = undefined,
	) {
		NAVDBG.print(
			'SlideShowNavigator.displaySlide: current index: ' +
				this.currentSlide +
				', nNewSlide: ' +
				nNewSlide +
				', bSkipTransition: ' +
				bSkipTransition,
		);

		if (!this.theMetaPres) return;
		if (this.presenter && !this.presenter._checkAlreadyPresenting()) {
			NAVDBG.print('SlideShowNavigator.displaySlide: no more presenting');
			this.quit();
			return;
		}
		if (nNewSlide === undefined || nNewSlide < 0) {
			NAVDBG.print('SlideShowNavigator.displaySlide: unexpected nNewSlide');
			return;
		}
		if (nNewSlide >= this.theMetaPres.numberOfSlides) {
			this.currentSlide = nNewSlide;
			const force = nNewSlide > this.theMetaPres.numberOfSlides;
			if (force) this.quit();
			else this.endPresentation(this.presenter._isWelcomePresentation);
			return;
		}

		let slideAvailable = true;
		const aNewMetaSlide = this.theMetaPres.getMetaSlideByIndex(nNewSlide);
		if (!aNewMetaSlide) {
			window.app.console.log(
				'SlideShowNavigator.displaySlide: no meta slide for index: ' +
					nNewSlide,
			);
			slideAvailable = false;
		} else if (aNewMetaSlide.hidden) {
			NAVDBG.print(
				'SlideShowNavigator.displaySlide: hidden slide: ' + nNewSlide,
			);
			slideAvailable = false;
		}
		if (!slideAvailable) {
			let offset = 1;
			if (this.currentSlide !== undefined)
				offset = Math.sign(nNewSlide - this.currentSlide);
			if (offset === 0) {
				NAVDBG.print('SlideShowNavigator.displaySlide: offset === 0');
				return;
			}
			this.displaySlide(nNewSlide + offset, bSkipTransition);
			return;
		}

		this.presenter.sendSlideShowFollowMessage(
			'displayslide ' + JSON.stringify({ currentSlide: nNewSlide }),
		);
		this.isSlideLoading = true;
		this.isEffectPending = false;
		this.slideCompositor.fetchAndRun(nNewSlide, () => {
			this.isSlideLoading = false;
			assert(
				this instanceof SlideShowNavigator,
				'SlideShowNavigator.displaySlide: slideCompositor.fetchAndRun: ' +
					'callback: this is not a SlideShowNavigator instance',
			);

			this.prevSlide = this.currentSlide;
			if (this.prevSlide >= this.theMetaPres.numberOfSlides)
				this.prevSlide = undefined;
			this.currentSlide = nNewSlide;
			if (this.presenter.updateControls) {
				this.presenter.updateControls();
			}

			if (this.currentSlide === this.prevSlide) {
				NAVDBG.print(
					'SlideShowNavigator.displaySlide: slideCompositor.fetchAndRun: this.currentSlide === this.prevSlide',
				);
				return;
			}

			// show welcome slideshow once 1st slide is rendered
			if (
				app.map.slideShowPresenter._isWelcomePresentation &&
				this.currentSlide === 0 &&
				window.mode.isCODesktop()
			) {
				const loader = document.getElementById('welcome-loader');
				if (loader) {
					loader.style.opacity = '0';
					// Wait for transition to complete before removing
					loader.addEventListener(
						'transitionend',
						function () {
							loader.parentNode.removeChild(loader);
						},
						{ once: true },
					);
				}
			}

			this.slideShowHandler.displaySlide(
				this.currentSlide,
				this.prevSlide,
				bSkipTransition,
				nStartEffect,
			);
			if (this.isRewindingToPrevSlide) {
				this.slideShowHandler.skipAllEffects();
				this.isRewindingToPrevSlide = false;
			}
			// Replay the leader's pending press (see dispatchEffect).
			// At most one switchSlide(+1) runs per load callback;
			// presses that came in during this load collapse into
			// this single advance. The replay starts a new load,
			// where more presses can queue up independently.
			else if (this.isEffectPending) {
				this.isEffectPending = false;
				this.switchSlide(1, true);
			}
			// Follower catch-up: align to the leader's position now
			// that the load is done. If the leader has moved further
			// ahead while we were loading, navigate to that slide;
			// if they are on the same slide but further into its
			// effects, skip to that effect.
			else if (this.presenter.isFollowing() && nStartEffect === undefined) {
				if (this.currentLeaderSlide > this.currentSlide) {
					this.displaySlide(this.currentLeaderSlide, true);
				} else if (
					this.currentLeaderSlide === this.currentSlide &&
					this.currentLeaderEffect > 0
				) {
					this.slideShowHandler.skipNEffects(this.currentLeaderEffect);
				}
			}
		});
	}

	startPresentation(
		nStartSlide: number,
		bSkipTransition: boolean,
		nStartEffect: number = undefined,
	) {
		NAVDBG.print(
			'SlideShowNavigator.startPresentation: current index: ' +
				this.currentSlide +
				', nStartSlide: ' +
				nStartSlide,
		);
		this.slideShowHandler.isStarting = true;
		this.isRewindingToPrevSlide = false;
		this.currentSlide = undefined;
		this.prevSlide = undefined;
		this.displaySlide(nStartSlide, bSkipTransition, nStartEffect);
	}

	endPresentation(force: boolean = false) {
		this.presenter.endPresentation(force);
	}

	onClick(aEvent: MouseEvent) {
		aEvent.preventDefault();
		aEvent.stopPropagation();

		if (!this.isEnabled) return;

		const metaSlide = this.theMetaPres.getMetaSlideByIndex(this.currentSlide);
		if (!metaSlide)
			window.app.console.log(
				'SlideShowNavigator.onClick: no meta slide available for index: ' +
					this.currentSlide,
			);

		if (metaSlide && metaSlide.animationsHandler) {
			const aEventMultiplexer = metaSlide.animationsHandler.eventMultiplexer;
			if (aEventMultiplexer) {
				if (aEventMultiplexer.hasRegisteredMouseClickHandlers()) {
					aEventMultiplexer.notifyMouseClick(aEvent);
					return;
				}
			}
		}
		this.clickHandler(aEvent);
	}

	private clickHandler(aEvent: MouseEvent) {
		if (this.presenter.isFollower()) this.presenter.setFollowing(false);
		if (aEvent.button === 0) {
			const slideInfo = this.theMetaPres.getSlideInfoByIndex(this.currentSlide);
			const slideHasInteractions =
				slideInfo &&
				slideInfo.interactions &&
				slideInfo.interactions.length > 0;
			const slideHasVideos =
				slideInfo && slideInfo.videos && slideInfo.videos.length > 0;
			if (!slideHasInteractions && !slideHasVideos) {
				this.dispatchEffect();
				return;
			}

			// Get the coordinates of the click
			const canvas = this.presenter.getCanvas();
			const width = canvas.clientWidth;
			const height = canvas.clientHeight;

			const x = (aEvent.offsetX / width) * this.theMetaPres.getDocWidth();
			const y = (aEvent.offsetY / height) * this.theMetaPres.getDocHeight();

			const shape = slideInfo.interactions.find((shape) =>
				hitTest(shape.bounds, x, y),
			);
			const videoInfo = slideInfo.videos.find((videoInfo) =>
				hitTest(videoInfo, x, y),
			);
			if (shape) {
				this._onExecuteInteraction(shape.clickAction);
			} else if (videoInfo) {
				this.presenter.sendSlideShowFollowMessage(
					'followvideo ' +
						JSON.stringify({
							currentSlide: this.currentSlide,
							videoInfoId: videoInfo.id,
						}),
				);

				const video = this.presenter.getVideoRenderer(
					slideInfo.hash,
					videoInfo,
				);
				video.handleClick();
			} else {
				this.dispatchEffect();
			}
		} else if (aEvent.button === 2) {
			this.switchSlide(-1, false);
		}
	}

	followVideo(info: any) {
		const slideInfo = this.theMetaPres.getSlideInfoByIndex(info.currentSlide);
		const videoInfo = slideInfo.videos.find(
			(videoInfo) => info.videoInfoId === videoInfo.id,
		);
		const video = this.presenter.getVideoRenderer(slideInfo.hash, videoInfo);
		video.handleClick();
	}

	onMouseMove(aEvent: MouseEvent) {
		if (!this.isEnabled) return;

		const metaSlide = this.theMetaPres.getMetaSlideByIndex(this.currentSlide);
		if (!metaSlide)
			window.app.console.log(
				'SlideShowNavigator.onMouseMove: no meta slide available for index: ' +
					this.currentSlide,
			);

		if (metaSlide && metaSlide.animationsHandler) {
			const aEventMultiplexer = metaSlide.animationsHandler.eventMultiplexer;
			if (aEventMultiplexer) {
				if (aEventMultiplexer.hasRegisteredMouseClickHandlers()) {
					const canvas = this.presenter.getCanvas();
					const width = canvas.clientWidth;
					const height = canvas.clientHeight;

					const x = (aEvent.offsetX / width) * this.theMetaPres.getDocWidth();
					const y = (aEvent.offsetY / height) * this.theMetaPres.getDocHeight();

					aEventMultiplexer.notifyMouseMove({ x: x, y: y });
					return;
				}
			}
		}
	}

	_onExecuteInteraction(action: ClickAction) {
		if (action) {
			switch (action.action) {
				case 'prevpage':
					this.switchSlide(-1, true);
					break;
				case 'nextpage':
					this.switchSlide(1, true);
					break;
				case 'firstpage':
					this.goToFirstSlide();
					break;
				case 'lastpage':
					this.goToLastSlide();
					break;
				case 'bookmark':
					this.goToSlideAtBookmark(action.bookmark);
					break;
				case 'stoppresentation':
					this.quit();
					break;
			}
		} else {
			this.dispatchEffect();
		}
	}

	onKeyDown(aEvent: KeyboardEvent) {
		if (!this.isEnabled && aEvent.code !== 'Escape') return;
		const handler = this.keyHandlerMap[aEvent.code];
		if (handler) {
			aEvent.preventDefault();
			aEvent.stopPropagation();
			if (this.presenter.isFollower()) this.presenter.setFollowing(false);
			handler();
		}
	}

	onSwipe(event: HammerInput) {
		event.preventDefault();
		if (
			!this.isEnabled &&
			event.direction !== Hammer.DIRECTION_DOWN &&
			event.direction !== Hammer.DIRECTION_UP
		)
			return;
		const handler = this.swipeHandlerMap[event.direction];
		if (handler) handler();
	}

	setPresenter(presenter: SlideShowPresenter) {
		this.presenter = presenter;
	}

	private get slideCompositor(): SlideCompositor {
		return this.presenter._slideCompositor;
	}

	enable() {
		this.isEnabled = true;
	}

	disable() {
		this.isEnabled = false;
	}
}
