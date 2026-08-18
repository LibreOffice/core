// @ts-strict-ignore
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

/*
This file defines the SaveState class, which handles the logic for managing the document's save
state. It controls the display of the saving status, saved status, and triggers the associated
animations and icon changes. A save the user asked for additionally shows a timed "Saved"
tooltip under the save button once it finishes; a save the server starts on its own only runs
the icon animation.
*/

// How long the "Saved" tooltip stays on screen.
const SAVED_TOOLTIP_DURATION_MS = 3000;

// How long a save the user asked for may take and still count as that user's save when it
// finishes. A completion arriving later than this is treated as a save the server started on
// its own, so a request whose round trip failed long ago cannot claim a later save.
const USER_SAVE_MATCH_WINDOW_MS = 30000;

class SaveState {
  map: any;
  saveEle: HTMLElement;
  // True from the moment a save of modified content starts until the "Saved"
  // state has been shown for it.
  saveRunning: boolean = false;
  // Time of the last save the user asked for, or 0 once that save has finished.
  userSaveTime: number = 0;

  // eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
  constructor(map: any) {
    this.map = map;
    this.hideSavedTooltip = this.hideSavedTooltip.bind(this);
    this.initialize();
  }

  initialize() {
    this.saveEle = document.querySelector('[id^="save"].unotoolbutton');
    if (this.saveEle) {
      // Reloading the shortcuts bar on user join can rebuild save icon
      if (app.file.modified) this.saveEle.classList.add('savemodified');
      this.insertSpinner();
    } else {
      app.console.debug('SaveState: no save icon - might be hidden');
    }
  }

  private insertSpinner(): void {
    if (this.saveEle.querySelector('.save-spinner')) return;
    const reducedMotion = window.matchMedia(
      '(prefers-reduced-motion: reduce)',
    ).matches;
    this.saveEle.appendChild(
      <svg class="save-spinner" viewBox="0 0 24 24" aria-hidden="true">
        <circle class="save-spinner-track" cx="12" cy="12" r="10.5" />
        <g>
          <circle
            class="save-spinner-arc"
            cx="12"
            cy="12"
            r="10.5"
            pathLength="100"
            stroke-dasharray="25 75"
          >
            {!reducedMotion && (
              <animate
                attributeName="stroke-dasharray"
                values="5 95;60 40;5 95"
                keyTimes="0;0.5;1"
                dur="1.5s"
                repeatCount="indefinite"
                calcMode="spline"
                keySplines="0.42 0 0.58 1;0.42 0 0.58 1"
              />
            )}
            {!reducedMotion && (
              <animate
                attributeName="stroke-dashoffset"
                values="0;-12;-100"
                keyTimes="0;0.5;1"
                dur="1.5s"
                repeatCount="indefinite"
                calcMode="spline"
                keySplines="0.42 0 0.58 1;0.42 0 0.58 1"
              />
            )}
          </circle>
          <animateTransform
            attributeName="transform"
            attributeType="XML"
            type="rotate"
            from="0 12 12"
            to="360 12 12"
            dur="2s"
            repeatCount="indefinite"
          />
        </g>
      </svg>,
    );
  }

  // Records that the user asked for the save that is about to run.
  markUserSave(): void {
    this.userSaveTime = Date.now();
  }

  // Function to show the saving status
  showSavingStatus(): void {
    if (window.mode.isSmallScreenDevice()) return;

    if (!this.saveEle) this.initialize();

    // Only do saving animation if any content is modified in document
    if (this.saveEle && this.saveEle.classList.contains('savemodified')) {
      this.saveRunning = true;
      this.hideSavedTooltip();
      this.saveEle.classList.remove('savemodified');
      this.saveEle.classList.add('saving');
      this.saveEle.setAttribute('disabled', 'true'); // Disable the button
    }
  }

  // Function to show the saved status
  showSavedStatus(): void {
    if (window.mode.isSmallScreenDevice()) return;

    if (!this.saveEle) this.initialize();

    if (
      this.saveEle &&
      this.saveRunning &&
      !this.saveEle.classList.contains('savemodified')
    ) {
      this.saveRunning = false;
      this.saveEle.classList.remove('saving');
      this.saveEle.classList.add('saved');
      if (
        this.userSaveTime &&
        Date.now() - this.userSaveTime < USER_SAVE_MATCH_WINDOW_MS
      )
        this.showSavedTooltip();
      this.userSaveTime = 0;
      // Add some delay to show "saved" status, then hide this info
      app.timerRegistry.setTimeout(
        'savedstatushide',
        () => {
          if (!this.saveEle) return;
          this.saveEle.classList.remove('saved');
          this.saveEle.removeAttribute('disabled'); // Enable the button
        },
        2000,
      );
    }
  }

  showModifiedStatus(): void {
    if (!this.saveEle) this.initialize();
    if (this.saveEle) {
      this.clearSaveInProgress();
      this.saveEle.classList.add('savemodified');
    }
  }

  showSaveFailedStatus(): void {
    if (!this.saveEle) this.initialize();
    if (this.saveEle) {
      this.clearSaveInProgress();
      if (app.file.modified) this.saveEle.classList.add('savemodified');
    }
  }

  private clearSaveInProgress(): void {
    this.saveRunning = false;
    this.userSaveTime = 0;
    this.hideSavedTooltip();
    this.saveEle.classList.remove('saving');
    this.saveEle.classList.remove('saved');
    this.saveEle.removeAttribute('disabled');
  }

  // Shows the standard tooltip with a "Saved" message under the save button. It goes away on
  // its own after a few seconds, and the first click anywhere dismisses it at once, so the
  // toolbar under it stays free to use.
  showSavedTooltip(): void {
    if (!this.map.uiManager || !this.map.tooltip) return;
    this.map.uiManager.showTimedTooltip(
      this.saveEle,
      _('Saved'),
      SAVED_TOOLTIP_DURATION_MS,
    );
    document.addEventListener('mousedown', this.hideSavedTooltip, true);
  }

  hideSavedTooltip(): void {
    document.removeEventListener('mousedown', this.hideSavedTooltip, true);
    if (this.map.tooltip && this.map.tooltip._current === this.saveEle)
      this.map.tooltip.mouseLeave();
  }
}

app.definitions.saveState = SaveState;
