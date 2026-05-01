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
 * Small reusable backstage building blocks: section headers, the primary
 * action button used in dialogs, and the templates loading placeholder.
 */

namespace BackstageTemplates {
  export function sectionHeader(
    title: string,
    description?: string,
  ): HTMLElement {
    return (
      <>
        <h2 class="backstage-content-title">{title}</h2>
        {description ? (
          <p class="backstage-content-description">{description}</p>
        ) : null}
      </>
    );
  }

  export function primaryButton(
    label: string,
    onClick: () => void,
  ): HTMLElement {
    return (
      <button class="backstage-property-button" onClick={onClick}>
        {label}
      </button>
    );
  }

  export function templatesLoading(): HTMLElement {
    return (
      <div class="backstage-templates-empty">
        <p class="backstage-content-description">{_('Loading templates…')}</p>
      </div>
    );
  }

  export function emptyRecentDocs(): HTMLElement {
    return (
      <p class="backstage-content-description">
        {_('Recently opened documents will appear here')}
      </p>
    );
  }
}
