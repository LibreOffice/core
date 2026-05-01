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

/*
 * Export view: a grid of cards, one per available download format. The
 * card label shows the file extension; clicking dispatches the format's
 * action to the backstage view, which routes it through the dispatcher
 * or a fallback download path.
 */

namespace BackstageTemplates {
  export function exportGrid(
    options: ExportOptionItem[],
    onExport: (action: string, command?: string) => void,
  ): HTMLElement {
    return (
      <div class="backstage-formats-grid">
        {options.map((option) => exportCard(option, onExport))}
      </div>
    );
  }

  function exportCard(
    option: ExportOptionItem,
    onExport: (action: string, command?: string) => void,
  ): HTMLElement {
    const format = option.action.startsWith('downloadas-')
      ? option.action.substring('downloadas-'.length)
      : option.action.startsWith('export')
        ? option.action.substring('export'.length)
        : '';
    const extensionLabel = format ? format.toUpperCase() : '';

    return (
      <div
        class="backstage-format-card"
        onClick={() => onExport(option.action, option.command)}
      >
        <div class="format-icon">{extensionLabel}</div>
        <div class="format-description">{option.text}</div>
      </div>
    );
  }
}
