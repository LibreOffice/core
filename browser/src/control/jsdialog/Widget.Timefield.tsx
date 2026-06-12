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
 * JSDialog.Timefield - input field for time data
 *
 * Example JSON:
 * {
 *     id: 'time',
 *     type: 'time',
 *     text: '01:01:01'
 * }
 */

/* global JSDialog */

JSDialog.timeField = function (
  parentContainer: HTMLElement,
  data: WidgetJSON,
  builder: JSBuilder,
) {
  const inputTimeField = (
    <input
      type="time"
      step={1} // forces the display of seconds
      id={data.id}
      value={data.text}
      className={builder.options.cssClass + ' ui-timefield'}
      disabled={data.enabled === false}
      onChange={(event: Event & { target: HTMLInputElement }) => {
        const timefield = event.target;
        if (!timefield.disabled) {
          builder.callback(
            'spinfield',
            'change',
            timefield,
            timefield.value,
            builder,
          );
        }
      }}
    />
  ) as HTMLInputElement;

  parentContainer.appendChild(inputTimeField);
  JSDialog.SetupA11yLabelForLabelableElement(
    parentContainer,
    inputTimeField,
    data,
    builder,
  );

  return false;
};
