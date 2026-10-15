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
 *
 * The text is formatted for the document locale, with its own hour format and
 * time separator. The field shows that text as it is, and core parses what the
 * user types.
 */

/* global JSDialog */

JSDialog.timeField = function (
  parentContainer: HTMLElement,
  data: WidgetJSON,
  builder: JSBuilder,
) {
  // The text core knows, which is what the field showed last or what was sent since.
  let sentText = data.text ?? '';

  const sendTime = (timefield: HTMLInputElement) => {
    if (timefield.disabled || timefield.value === sentText) return;
    sentText = timefield.value;
    builder.callback(
      'spinfield',
      'change',
      timefield,
      timefield.value,
      builder,
    );
  };

  const inputTimeField = (
    <input
      type="text"
      id={data.id}
      value={data.text ?? ''}
      className={builder.options.cssClass + ' ui-timefield'}
      disabled={data.enabled === false}
      onChange={(event: Event & { target: HTMLInputElement }) => {
        sendTime(event.target);
      }}
      onKeyDown={(event: KeyboardEvent & { target: HTMLInputElement }) => {
        // Enter sends an edited time and keeps the dialog open, so the user sees the time
        // core understood.
        if (event.key === 'Enter') {
          event.preventDefault();
          sendTime(event.target);
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
