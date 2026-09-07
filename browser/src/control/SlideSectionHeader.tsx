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

interface SlideSectionHeaderCallbacks {
  // The chevron toggle was pressed: collapse or expand the section.
  onToggle: () => void;
  // The header itself was clicked: select the section's slides.
  onSelect: () => void;
}

/*
 * A header row for a slide section: a chevron toggle, the section name and,
 * when one is given, how many slides the section holds. The slide navigator
 * and the slide import pane both build their section headers here, so
 * sections carry the same markup, classes and controls in either panel.
 */
function buildSlideSectionHeader(
  name: string,
  sectionIndex: number,
  collapsed: boolean,
  callbacks: SlideSectionHeaderCallbacks,
  count?: string,
): HTMLElement {
  return (
    <div
      class={'slide-section-header' + (collapsed ? ' collapsed' : '')}
      data-section-index={sectionIndex}
      draggable="false"
      onClick={(e: MouseEvent) => {
        e.stopPropagation();
        e.preventDefault();
        callbacks.onSelect();
      }}
    >
      <button
        type="button"
        class="slide-section-toggle ui-expander-btn"
        aria-expanded={collapsed ? 'false' : 'true'}
        aria-label={_('Toggle section %1').replace('%1', name)}
        onClick={(e: MouseEvent) => {
          e.stopPropagation();
          e.preventDefault();
          callbacks.onToggle();
        }}
      />
      <span class="slide-section-name" title={name}>
        {name}
      </span>
      {count && <span class="slide-section-count">{count}</span>}
    </div>
  );
}
