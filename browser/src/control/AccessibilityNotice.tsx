// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global _ */

/*
 * A bar along the bottom of a small screen saying that this view is not fully
 * accessible, with a link to the accessibility statement of the deployment.
 * A screen reader speaks the message on its own, so the bar never takes the
 * focus away from the document.
 */

namespace AccessibilityNotice {
  let notice: HTMLElement | null = null;

  export function show(statementUrl: string): void {
    if (notice) return;

    const announcement = (
      <div class="accessibility-notice-announcement" role="alert" />
    );

    notice = (
      <div class="accessibility-notice">
        <p class="accessibility-notice-message">
          {_('This mobile view is not fully accessible.')}
        </p>
        {announcement}
        <a
          class="accessibility-notice-link"
          href={window.sanitizeUrl(statementUrl)}
          target="_blank"
          rel="noopener noreferrer"
        >
          {_('Accessibility statement')}
        </a>
        <button
          class="accessibility-notice-dismiss"
          type="button"
          onClick={hide}
        >
          {_('Dismiss')}
        </button>
      </div>
    );

    document.body.appendChild(notice);

    // A screen reader speaks an alert when its text changes while it is
    // already in the page. The text arrives a few seconds late so that the
    // reader has finished announcing the page itself and this is not lost in
    // that.
    setTimeout(() => {
      if (notice)
        announcement.textContent = _(
          'This mobile view is not fully accessible.',
        );
    }, 3000);
  }

  export function hide(): void {
    if (!notice) return;
    notice.remove();
    notice = null;
  }
}
