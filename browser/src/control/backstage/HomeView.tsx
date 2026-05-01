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
 * Home view: a row of starter templates, a "More Templates" link, and a
 * table of recently opened documents. The view passes in already-resolved
 * data plus click handlers; this file owns the structure.
 */

namespace BackstageTemplates {
  export interface HomeTemplatesProps {
    templates: TemplateData[];
    onTemplateClick: (template: TemplateData) => void;
    onMoreTemplatesClick: () => void;
  }

  export interface RecentDocRowData {
    index: number;
    fileName: string;
    filePath: string;
    formattedTime: string;
    uri: string;
    iconClass: string;
  }

  export function homeTemplatesSection(props: HomeTemplatesProps): HTMLElement {
    return (
      <>
        {props.templates.length > 0 ? (
          <div class="backstage-home-templates-row">
            {props.templates.map((t) =>
              templateCard(t, () => props.onTemplateClick(t)),
            )}
          </div>
        ) : null}
        <div class="backstage-home-more-templates">
          <div class="backstage-home-divider" />
          <div
            class="backstage-home-more-button"
            role="button"
            tabindex="0"
            aria-label={_('More Templates')}
            onClick={props.onMoreTemplatesClick}
          >
            {_('More Templates')}
          </div>
        </div>
      </>
    );
  }

  export function recentDocsTable(
    rows: RecentDocRowData[],
    onOpen: (uri: string) => void,
  ): HTMLElement {
    return (
      <table class="backstage-recent-documents-table">
        <thead class="backstage-recent-documents-header">
          <tr class="backstage-recent-documents-header-row">
            <th class="backstage-recent-documents-header-cell">{_('Name')}</th>
            <th class="backstage-recent-documents-header-cell">
              {_('Modified Date')}
            </th>
          </tr>
        </thead>
        <tbody class="backstage-recent-documents-body">
          {rows.map((r) => recentDocRow(r, onOpen))}
        </tbody>
      </table>
    );
  }

  function recentDocRow(
    r: RecentDocRowData,
    onOpen: (uri: string) => void,
  ): HTMLElement {
    return (
      <tr class="backstage-recent-document-row" onClick={() => onOpen(r.uri)}>
        <td
          class="backstage-recent-document-name-cell"
          id={`backstage-recent-document-${r.index}`}
        >
          <div class="backstage-recent-document-content-wrapper">
            <span class={`backstage-recent-document-icon ${r.iconClass}`} />
            <div class="backstage-recent-document-text-wrapper">
              <div class="backstage-recent-document-name-text">
                {r.fileName}
              </div>
              <div class="backstage-recent-document-path-text">
                {r.filePath}
              </div>
            </div>
          </div>
        </td>
        <td class="backstage-recent-document-time-cell">{r.formattedTime}</td>
      </tr>
    );
  }
}
