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

/*
 * JSDialog.progressDots - a row of dots that shows how far along a sequence of steps the user is.
 * There is one dot per step, and the dot of the current step is marked. A screen reader reads
 * the same position as text, for example "Step 2 of 5".
 *
 * Example JSON:
 * {
 *     id: 'my-progress',
 *     type: 'progressdots',
 *     count: 5,     // number of steps
 *     current: 1,   // index of the current step, starting at 0
 * }
 */

JSDialog.progressDots = function (
  parentContainer: Element,
  data: ProgressDotsWidgetJSON,
  builder: JSBuilder,
): boolean {
  const count = data.count > 0 ? Math.floor(data.count) : 0;

  // A current step outside the list marks the nearest dot.
  let current = Math.floor(data.current) || 0;
  if (count > 0 && (current < 0 || current >= count)) {
    app.console.warn(
      'progressdots: current step ' +
        data.current +
        ' is outside 0 to ' +
        (count - 1),
    );
    current = Math.min(Math.max(current, 0), count - 1);
  }

  const dots: HTMLElement[] = [];
  for (let i = 0; i < count; i++)
    dots.push(
      <span
        class={
          'ui-progressdots-dot' +
          (i === current ? ' ui-progressdots-current' : '')
        }
        aria-hidden="true"
      />,
    );

  const label = _('Step %1 of %2')
    .replace('%1', String(current + 1))
    .replace('%2', String(count));

  parentContainer.appendChild(
    <div
      id={data.id || undefined}
      class={'ui-progressdots ' + builder.options.cssClass}
    >
      {dots}
      <span class="visuallyhidden">{label}</span>
    </div>,
  );

  return false;
};
