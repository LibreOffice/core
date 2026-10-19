/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global _ app JSDialog */

/*
 * The onboarding tour: a modal JSDialog popup that walks through the steps of OnboardingSteps.ts
 * one at a time. A step with an element on screen anchors the popup to that element and gives the
 * element the onboarding-highlight class. A step without one shows the popup in the middle of the
 * window over a dimmed page. The tour runs once per document type on the first visit, and the
 * user can start it again at any time.
 */

class OnboardingTourController {
	// The id the tour passes to the modal helpers. The popup window id is built from it.
	private static readonly modalId = 'onboarding-tour';

	// The steps of the running tour, chosen when it starts. Empty when no tour is running.
	private steps: OnboardingStep[] = [];
	private stepIndex = 0;

	// The element that carries the highlight of the current step, or null.
	private highlighted: HTMLElement | null = null;

	// True once the document is on screen and sidebar initialization has finished. The tour
	// starts on its own only once the parts of the screen it describes are there.
	private uiReady = false;

	// The layouting task that starts the tour on its own, while it waits to run.
	private startTask: TaskId | null = null;

	// The layouting task that adds the arrow keys to the popup of the current step, while it
	// waits to run.
	private arrowKeysTask: TaskId | null = null;

	isRunning(): boolean {
		return this.steps.length > 0;
	}

	// The id of the step on screen, or null when no tour is running.
	currentStepId(): string | null {
		return this.isRunning() ? this.steps[this.stepIndex].id : null;
	}

	// The document is on screen and sidebar initialization has finished, whether or not the
	// sidebar was requested.
	onSidebarSettled(): void {
		this.uiReady = true;
		this.maybeAutoStart();
	}

	// Opens the tour whatever the stored preference says, so that the user can ask for it again
	// once it has been seen. The tour exists on the desktop layout only.
	start(): void {
		if (!window.mode.isDesktop()) return;

		// A popup that was closed without its close callback, as closing every dialog does,
		// leaves a tour that is no longer on screen. That tour is cleared and a new one
		// starts.
		if (this.isRunning() && !this.isPopupOpen()) this.reset();

		if (this.isRunning()) return;

		const steps = this.collectSteps();
		if (!steps.length) return;

		// The preference is stored before the tour opens, so that a view which goes away
		// mid-tour still counts as one that has seen it.
		app.map.uiManager.setDocTypePref('onboardingTourSeen', true);

		this.steps = steps;
		this.showStep(0);
	}

	// Closes the tour from any step and takes the highlight away. The focus goes back to the
	// document.
	end(): void {
		if (this.startTask !== null) {
			app.layoutingService.cancelLayoutingTask(this.startTask);
			this.startTask = null;
		}

		this.cancelArrowKeysTask();

		if (!this.isRunning()) return;

		this.reset();
		app.map.uiManager.closeModal(OnboardingTourController.popupId());
	}

	next(): void {
		if (!this.isRunning()) return;
		if (this.stepIndex + 1 >= this.steps.length) this.end();
		else this.showStep(this.stepIndex + 1);
	}

	back(): void {
		if (!this.isRunning() || this.stepIndex === 0) return;
		this.showStep(this.stepIndex - 1);
	}

	private static popupId(): string {
		return JSDialog.generateModalId(OnboardingTourController.modalId);
	}

	private isPopupOpen(): boolean {
		const jsdialog = app.map.jsdialog;
		return !!jsdialog && !!jsdialog.dialogs[OnboardingTourController.popupId()];
	}

	private reset(): void {
		this.steps = [];
		this.stepIndex = 0;
		this.setHighlight(null);
	}

	private maybeAutoStart(): void {
		// The steps are chosen from what is on screen when the tour starts.
		if (!this.uiReady || this.isRunning() || this.startTask !== null) return;

		if (!window.mode.isDesktop()) return;

		// A read-only view has neither the toolbar nor the notebookbar the tour walks
		// through.
		if (app.isReadOnly()) return;

		if (app.map.uiManager.getBooleanDocTypePref('onboardingTourSeen', false))
			return;

		// The sidebar attaches its content in a layouting task of its own. This task runs
		// after it, so the tour sees the sidebar as it will stay.
		this.startTask = app.layoutingService.appendLayoutingTask(() => {
			this.startTask = null;
			this.startUnlessWelcomeIsOpen();
		});
	}

	// The welcome dialog explains the release and the tour explains the screen, so the tour
	// waits until the welcome has been closed.
	private startUnlessWelcomeIsOpen(): void {
		const welcome = app.map.welcome;
		if (welcome && welcome.isOpen()) {
			app.map.once('welcomeclosed', () => this.maybeAutoStart());
			return;
		}

		this.start();
	}

	private showStep(index: number): void {
		this.stepIndex = index;
		const step = this.steps[index];

		if (step.beforeShow) step.beforeShow();

		const target = OnboardingTourController.resolveTarget(step);
		this.setHighlight(target);

		// The popup of the previous step closes and the popup of this step opens in its
		// place. The new popup takes the focus, so the focus stays in the tour.
		const uiManager = app.map.uiManager;
		uiManager.closeModal(OnboardingTourController.popupId(), true);

		const json = uiManager._modalDialogJSON(
			OnboardingTourController.modalId,
			step.title,
			false,
			this.buildWidgets(index),
			'onboarding-next-button',
		);
		if (target) {
			json.popupParent = target;
			json.popupAnchor = step.anchor || '';
			json.fitContent = true;
		}

		uiManager.showModal(json, [
			{ id: 'onboarding-skip', type: 'click', func: () => this.end() },
			{ id: 'onboarding-back', type: 'click', func: () => this.back() },
			{ id: 'onboarding-next', type: 'click', func: () => this.next() },
			// The popup closed by Escape or by the close button of its title bar.
			{ id: '__POPOVER__', type: 'close', func: () => this.end() },
		]);

		// The popup is attached to the page in a layouting task of the platform. This task
		// runs after it.
		this.cancelArrowKeysTask();
		this.arrowKeysTask = app.layoutingService.appendLayoutingTask(() => {
			this.arrowKeysTask = null;
			this.addArrowKeys();
		});
	}

	private cancelArrowKeysTask(): void {
		if (this.arrowKeysTask === null) return;
		app.layoutingService.cancelLayoutingTask(this.arrowKeysTask);
		this.arrowKeysTask = null;
	}

	// The arrow keys move between the steps while the card has the focus. The listener is on the
	// popup window, so it goes away with the popup.
	private addArrowKeys(): void {
		const popup = document.getElementById(OnboardingTourController.popupId());
		if (!popup) return;

		popup.addEventListener('keydown', (event: KeyboardEvent) => {
			if (event.key === 'ArrowRight') {
				event.preventDefault();
				this.next();
			} else if (event.key === 'ArrowLeft') {
				event.preventDefault();
				this.back();
			}
		});
	}

	// The text, the progress and the buttons of the card for the step at index.
	private buildWidgets(index: number): WidgetJSON[] {
		const step = this.steps[index];
		const isLast = index + 1 === this.steps.length;

		const text: WidgetJSON = {
			id: 'onboarding-text',
			type: 'fixedtext',
			text: step.text,
		};

		const progress: ProgressDotsWidgetJSON = {
			id: 'onboarding-progress',
			type: 'progressdots',
			count: this.steps.length,
			current: index,
		};

		const skip: PushButtonWidget = {
			id: 'onboarding-skip',
			type: 'pushbutton',
			text: _('Skip'),
		};

		const back: PushButtonWidget = {
			id: 'onboarding-back',
			type: 'pushbutton',
			text: _('Back'),
			enabled: index > 0,
		};

		const next: PushButtonWidget = {
			id: 'onboarding-next',
			type: 'pushbutton',
			text: isLast ? _('Done') : _('Next'),
			has_default: true,
		};

		const buttons: ButtonBoxWidget = {
			id: 'onboarding-buttons',
			type: 'buttonbox',
			layoutstyle: 'end',
			children: [skip, back, next],
		};

		// The progress and the buttons share one row.
		const footer: ContainerWidgetJSON = {
			id: 'onboarding-footer',
			type: 'container',
			vertical: false,
			children: [progress, buttons],
		};

		return [text, footer];
	}

	// Moves the highlight to the given element, or removes it when the element is null.
	private setHighlight(element: HTMLElement | null): void {
		if (this.highlighted)
			this.highlighted.classList.remove('onboarding-highlight');

		this.highlighted = element;
		if (element) element.classList.add('onboarding-highlight');
	}

	// The steps that suit this document type and user interface mode and whose element is on
	// screen. A step whose element is missing is left out. The choice is made once, when the
	// tour starts.
	private collectSteps(): OnboardingStep[] {
		const docType = app.map.getDocType() as OnboardingDocType;
		const uiMode = app.map.uiManager.getCurrentMode() as OnboardingUiMode;

		return getOnboardingSteps().filter((step) => {
			if (step.docTypes && step.docTypes.indexOf(docType) === -1) return false;
			if (step.uiModes && step.uiModes.indexOf(uiMode) === -1) return false;
			if (step.isAvailable && !step.isAvailable()) return false;
			if (!step.target) return true;
			return OnboardingTourController.resolveTarget(step) !== null;
		});
	}

	// Returns the element a step points at, or null when the step speaks about the document as
	// a whole or the element is not on screen in this view.
	static resolveTarget(step: OnboardingStep): HTMLElement | null {
		if (!step.target) return null;

		const element =
			typeof step.target === 'string'
				? document.querySelector(step.target)
				: step.target();

		if (!(element instanceof HTMLElement)) return null;
		if (!OnboardingTourController.isOnScreen(element)) return null;

		return element;
	}

	// Whether the element takes up room on screen. The size comes from the layout box, which an
	// element that is sliding into place already has in full.
	static isOnScreen(element: HTMLElement): boolean {
		return element.offsetWidth > 0 && element.offsetHeight > 0;
	}

	// The first of these selectors that finds an element on screen. It lets a step name several
	// places the same thing can live in.
	static firstOnScreen(selectors: string[]): HTMLElement | null {
		for (let i = 0; i < selectors.length; i++) {
			const element = document.querySelector(selectors[i]);
			if (
				element instanceof HTMLElement &&
				OnboardingTourController.isOnScreen(element)
			)
				return element;
		}

		return null;
	}
}
