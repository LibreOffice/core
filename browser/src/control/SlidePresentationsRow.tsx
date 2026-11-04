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

interface SlidePresentationsRowCallbacks {
  // Every linked presentation is to show its slides and sections.
  onExpandAll: () => void;
  // Every linked presentation is to show as its tile.
  onCollapseAll: () => void;
}

/*
 * The row above the maximized slide grid with the actions that apply to
 * every linked presentation at once: one button expands them all and one
 * collapses them all. The row shows in the grid only.
 */
function buildSlidePresentationsRow(
  callbacks: SlidePresentationsRowCallbacks,
): HTMLElement {
  return (
    <div
      class="slide-presentations-row"
      role="group"
      aria-label={_('Linked presentations')}
    >
      <button
        type="button"
        class="ui-linkbutton slide-presentations-expand-all"
        onClick={(e: MouseEvent) => {
          e.stopPropagation();
          callbacks.onExpandAll();
        }}
      >
        {_('Expand all linked presentations')}
      </button>
      <button
        type="button"
        class="ui-linkbutton slide-presentations-collapse-all"
        onClick={(e: MouseEvent) => {
          e.stopPropagation();
          callbacks.onCollapseAll();
        }}
      >
        {_('Collapse all linked presentations')}
      </button>
    </div>
  ) as HTMLElement;
}
