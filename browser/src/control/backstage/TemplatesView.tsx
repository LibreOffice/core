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
 * "New" view: featured row of blank templates, search box, and a grid of
 * filtered templates. Also exposes templateCard, reused by HomeView.
 */

namespace BackstageTemplates {
  export interface TemplateExplorerProps {
    featuredTemplates: TemplateData[];
    gridTemplates: TemplateData[];
    searchQuery: string;
    showSearch: boolean;
    onTemplateClick: (template: TemplateData) => void;
    onSearchInput: (value: string) => void;
    featuredRowRef: (el: HTMLElement | null) => void;
    searchContainerRef: (el: HTMLElement | null) => void;
    gridContainerRef: (el: HTMLElement) => void;
  }

  export interface TemplateCardOptions {
    variant?: 'featured';
    isBlank?: boolean;
  }

  export function templateCard(
    template: TemplateData,
    onClick: () => void,
    options: TemplateCardOptions = {},
  ): HTMLElement {
    const cls = [
      'backstage-template-card',
      options.variant === 'featured' ? 'is-featured' : '',
      options.isBlank ? 'is-blank' : '',
    ]
      .filter(Boolean)
      .join(' ');

    return (
      <div class={cls} id={template.id} onClick={onClick}>
        <div class="template-thumbnail">
          <img
            alt={template.name}
            src={template.preview || defaultPreview(template.type)}
          />
        </div>
        <div class="template-name">{template.name}</div>
      </div>
    );
  }

  function defaultPreview(type: TemplateType): string {
    const previews: Record<TemplateType, string> = {
      writer: 'images/filetype/writer.svg',
      calc: 'images/filetype/calc.svg',
      impress: 'images/filetype/impress.svg',
    };
    return previews[type] || 'images/filetype/document.svg';
  }

  export function templateExplorer(props: TemplateExplorerProps): HTMLElement {
    const featuredRow =
      props.featuredTemplates.length > 0
        ? featuredTemplatesRow(props.featuredTemplates, props.onTemplateClick)
        : null;
    props.featuredRowRef(featuredRow);

    const searchEl = props.showSearch
      ? searchInput(props.searchQuery, props.onSearchInput)
      : null;
    props.searchContainerRef(searchEl);

    const gridEl = templateGrid(
      props.gridTemplates,
      props.searchQuery,
      props.onTemplateClick,
    );
    props.gridContainerRef(gridEl);

    return (
      <div class="backstage-template-explorer">
        {featuredRow}
        {searchEl}
        {gridEl}
      </div>
    );
  }

  export function featuredTemplatesRow(
    templates: TemplateData[],
    onClick: (t: TemplateData) => void,
  ): HTMLElement {
    return (
      <div class="template-featured-row">
        {templates.map((t) =>
          templateCard(t, () => onClick(t), {
            variant: 'featured',
            isBlank: true,
          }),
        )}
      </div>
    );
  }

  export function templateGrid(
    templates: TemplateData[],
    searchQuery: string,
    onClick: (t: TemplateData) => void,
  ): HTMLElement {
    const noMatches = !templates.length && searchQuery.trim();
    return (
      <div class="backstage-templates-grid">
        {noMatches ? (
          <div class="template-grid-empty">
            {_('No templates match your search.')}
          </div>
        ) : null}
        {templates.map((t) => templateCard(t, () => onClick(t)))}
      </div>
    );
  }

  function searchInput(
    query: string,
    onInput: (value: string) => void,
  ): HTMLElement {
    return (
      <div class="template-search">
        <input
          type="search"
          class="template-search-input"
          placeholder={_('Search templates')}
          value={query}
          onInput={(e: Event) =>
            onInput((e.target as HTMLInputElement).value || '')
          }
        />
      </div>
    );
  }
}
