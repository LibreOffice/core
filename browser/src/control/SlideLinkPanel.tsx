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
 * The panel that shows a linked presentation beside the slide navigator: a
 * header in the shape of the navigation header, with the name of the
 * presentation as its title and a close button, and a deck under it for the
 * section headers and the slide frames of the presentation. The deck starts
 * empty. The navigator moves its own headers and frames into it while the
 * panel is open and takes them back when the panel closes, so the panel holds
 * no copy of them. The name takes the same markup as on a section header, so
 * the mark of the link goes next to it the same way.
 */
function buildSlideLinkPanel(name: string, onClose: () => void): HTMLElement {
  const closeLabel = _('Close the linked presentation');
  return (
    <div
      class="slide-link-panel"
      role="region"
      aria-label={_('Linked presentation %1').replace('%1', name)}
    >
      <div class="slide-link-panel-title navigation-header">
        <h2 class="navigation-title slide-section-name" title={name}>
          {name}
        </h2>
        <div class="close-navigation-wrapper">
          <button
            type="button"
            class="close-navigation-button slide-link-panel-close"
            aria-label={closeLabel}
            data-cooltip={closeLabel}
            onClick={(e: MouseEvent) => {
              e.stopPropagation();
              onClose();
            }}
          />
        </div>
      </div>
      <div
        class="slide-link-panel-deck"
        role="listbox"
        aria-multiselectable="true"
        aria-label={_('Slides of %1').replace('%1', name)}
      />
    </div>
  ) as HTMLElement;
}
