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

interface SlideSectionHeaderOptions {
  // The row toggles the section, the way the source row above it does,
  // rather than selecting the section's slides. The slide import pane wants
  // this; the navigator selects on the row and renames on a second press.
  rowToggles?: boolean;
  // The name a screen reader gives the chevron toggle, in place of "Toggle
  // section <name>", for a header that is no section's.
  toggleLabel?: string;
  // A checkbox at the end of the row that takes the section's slides into
  // the selection or out of it. Partial means some but not all of them.
  pick?: {
    checked: boolean;
    partial: boolean;
    label: string;
    onPick: () => void;
  };
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
  options?: SlideSectionHeaderOptions,
): HTMLElement {
  const pick = options && options.pick ? options.pick : null;
  const header = (
    <div
      class={'slide-section-header' + (collapsed ? ' collapsed' : '')}
      data-section-index={sectionIndex}
      draggable="false"
      onClick={(e: MouseEvent) => {
        e.stopPropagation();
        e.preventDefault();
        if (options && options.rowToggles) callbacks.onToggle();
        else callbacks.onSelect();
      }}
    >
      <button
        type="button"
        class="slide-section-toggle ui-expander-btn"
        aria-expanded={collapsed ? 'false' : 'true'}
        aria-label={
          options && options.toggleLabel
            ? options.toggleLabel
            : _('Toggle section %1').replace('%1', name)
        }
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
      {pick && (
        <input
          type="checkbox"
          class="slide-section-pick"
          checked={pick.checked}
          aria-label={pick.label}
          onClick={(e: MouseEvent) => e.stopPropagation()}
          onChange={() => pick.onPick()}
        />
      )}
    </div>
  ) as HTMLElement;

  // indeterminate is a property and not an attribute, so it is set here
  // rather than in the markup above.
  if (pick && pick.partial) {
    const box = header.querySelector(
      '.slide-section-pick',
    ) as HTMLInputElement | null;
    if (box) box.indeterminate = true;
  }

  return header;
}
