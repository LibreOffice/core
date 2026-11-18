/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* global _ app */

/*
 * The part of the user list for the presentations that link to this document: one row per user,
 * naming the user under the mark of a linked slide. The rows take the keyboard focus in turn,
 * numbered on from firstIndex, and nothing happens on a click.
 */
function buildLinkedViewList(
  usernames: string[],
  firstIndex: number,
  map: any,
): HTMLElement {
  const rows = usernames.map((username, i) => {
    // TRANSLATORS: %1 is the name of a user who has a presentation open that links to this one.
    const label = _('%1 - linked presentation').replace('%1', () => username);

    const icon = (
      <img class="avatar-img linked-view-avatar" alt="" aria-hidden="true" />
    ) as HTMLImageElement;
    app.LOUtil.setImage(icon, 'sc_linkedslide.svg', map);

    return (
      <div
        class="user-list-item linked-view"
        role="listitem"
        tabindex="0"
        index={firstIndex + i + ':0'}
      >
        {icon}
        <div class="user-list-item--name-container">
          <div class="user-list-item--name">{label}</div>
        </div>
      </div>
    );
  });

  return (
    <div
      class="linked-view-list"
      role="list"
      aria-label={_('Linked presentations')}
    >
      {rows}
    </div>
  ) as HTMLElement;
}
