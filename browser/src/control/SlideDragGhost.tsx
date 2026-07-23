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

// How far each stacked picture sits behind the one in front of it, in CSS
// pixels, and how many pictures the stack shows at most. Past the cap the
// badge alone tells the full count.
const SLIDE_DRAG_GHOST_STACK_OFFSET = 8;
const SLIDE_DRAG_GHOST_MAX_LAYERS = 3;

// Each ghost picture draws a 3px border on every side, so its box is 6px
// larger than the picture it shows.
const SLIDE_DRAG_GHOST_BORDER = 6;

/*
 * The floating picture under the pointer while slides are dragged. The
 * grabbed slide's picture sits on top at its on-screen size; when several
 * slides travel in one drag, the following pictures stack behind it with a
 * small offset and a badge over the top picture's corner shows how many
 * slides the drag carries. The slide navigator's reorder drag and the
 * slide import pane's insert drag both build their ghost here.
 *
 * sources lists the pictures front to back, the grabbed slide first; blank
 * entries (a slide whose preview has not rendered yet) are skipped. count
 * is the full number of slides in the drag, which can exceed the pictures
 * shown. The caller appends the ghost to the document, hands it to
 * setDragImage and removes it once the browser has taken its snapshot.
 */
function buildSlideDragGhost(
  sources: string[],
  width: number,
  height: number,
  count: number,
): HTMLElement {
  const layers = sources
    .filter((source) => !!source)
    .slice(0, SLIDE_DRAG_GHOST_MAX_LAYERS);
  const spread = (layers.length - 1) * SLIDE_DRAG_GHOST_STACK_OFFSET;
  const ghost = (
    <div
      class="slide-drag-ghost"
      aria-hidden="true"
      style={{
        width: width + SLIDE_DRAG_GHOST_BORDER + spread + 'px',
        height: height + SLIDE_DRAG_GHOST_BORDER + spread + 'px',
      }}
    />
  );
  // Painter's order: the deepest picture first, the grabbed slide last so
  // it paints on top at the ghost's origin, where the caller anchors the
  // grab point.
  for (let i = layers.length - 1; i >= 0; i--) {
    const offset = i * SLIDE_DRAG_GHOST_STACK_OFFSET;
    ghost.appendChild(
      <img
        src={layers[i]}
        alt=""
        style={{
          left: offset + 'px',
          top: offset + 'px',
          width: width + 'px',
          height: height + 'px',
        }}
      />,
    );
  }
  if (count > 1) {
    ghost.appendChild(
      <span
        class="slide-drag-ghost-count"
        style={{ top: '4px', right: spread + 4 + 'px' }}
      >
        {String(count)}
      </span>,
    );
  }
  return ghost;
}
