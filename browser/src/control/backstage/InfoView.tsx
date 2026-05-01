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
 * Document Info view: a column of Type/File Name/Pages/Mode property rows
 * with a button to open the full Document Properties dialog.
 */

namespace BackstageTemplates {
  export interface DocumentProperty {
    label: string;
    value: string;
  }

  export function infoPropertiesColumn(
    properties: DocumentProperty[],
    onOpenProperties: () => void,
  ): HTMLElement {
    return (
      <div class="backstage-info-properties">
        <h3 class="backstage-section-header">{_('Properties')}</h3>
        <div class="backstage-properties-list">
          {properties
            .filter((p) => p.value)
            .map((p) => propertyItem(p.label, p.value))}
        </div>
        {primaryButton(_('More Property Info'), onOpenProperties)}
      </div>
    );
  }

  function propertyItem(label: string, value: string): HTMLElement {
    return (
      <div class="backstage-property-item">
        <div class="property-label">{label}</div>
        <div class="property-value">{value}</div>
      </div>
    );
  }
}
