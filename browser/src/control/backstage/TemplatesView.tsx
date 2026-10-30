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
 * "New" view: a row of blank documents, then the templates under a search box
 * and type filter chips. Also exposes templateCard, reused by HomeView.
 */

namespace BackstageTemplates {
  export interface TemplateExplorerProps {
    blankTemplates: TemplateData[];
    searchQuery: string;
    filter: TemplateFilter;
    // The part below the search field and the filter chips.
    sections: HTMLElement;
    onTemplateClick: (template: TemplateData) => void;
    onSearchInput: (value: string) => void;
    onFilterChange: (filter: TemplateFilter) => void;
  }

  export interface TemplateSectionsProps {
    // Null where the app keeps no templates of the user.
    userTemplates: TemplateData[] | null;
    builtInTemplates: TemplateData[];
    searchQuery: string;
    onClick: (template: TemplateData) => void;
    onAddTemplate: () => void;
    onManageTemplates: () => void;
    favoriteState: (template: TemplateData) => TemplateFavoriteState;
  }

  export interface TemplateCardOptions {
    isBlank?: boolean;
    // Absent for a card that cannot be a favorite.
    favorite?: TemplateFavoriteState;
  }

  export interface TemplateFavoriteState {
    isFavorite: boolean;
    onToggle: () => void;
  }

  export function templateCard(
    template: TemplateData,
    onClick: () => void,
    options: TemplateCardOptions = {},
  ): HTMLElement {
    const cls = [
      'backstage-template-card',
      options.isBlank ? 'is-blank' : '',
      options.favorite?.isFavorite ? 'is-favorite' : '',
    ]
      .filter(Boolean)
      .join(' ');

    // a localized preview that does not exist steps on to the next one
    const fallbacks = (template.previewFallbacks || []).slice();
    const nextPreview = (ev: Event) => {
      const next = fallbacks.shift();
      if (next) (ev.target as HTMLImageElement).src = next;
    };

    return (
      <div class={cls} id={template.id} onClick={onClick}>
        <div class="template-thumbnail">
          <img
            class={template.preview ? null : 'is-type-icon'}
            alt={template.name}
            src={template.preview || defaultPreview(template.type)}
            onError={nextPreview}
          />
        </div>
        <div class="template-name">{template.name}</div>
        {options.favorite ? favoriteButton(options.favorite) : null}
      </div>
    );
  }

  function favoriteButton(favorite: TemplateFavoriteState): HTMLElement {
    const label = favorite.isFavorite
      ? _('Remove from favorites')
      : _('Add to favorites');
    return (
      <button
        type="button"
        class="backstage-template-favorite"
        aria-pressed={String(favorite.isFavorite)}
        aria-label={label}
        title={label}
        onClick={(event: Event) => {
          // The star sits on the card, and the card opens the template.
          event.stopPropagation();
          favorite.onToggle();
        }}
      />
    );
  }

  function defaultPreview(type: TemplateType): string {
    const previews: Record<TemplateType, string> = {
      writer: 'images/x-office-document.svg',
      calc: 'images/x-office-spreadsheet.svg',
      impress: 'images/x-office-presentation.svg',
      draw: 'images/x-office-drawing.svg',
    };
    return previews[type] || 'images/x-office-document.svg';
  }

  export function templateExplorer(props: TemplateExplorerProps): HTMLElement {
    return (
      <div class="backstage-template-explorer">
        {blankTemplatesRow(props.blankTemplates, props.onTemplateClick)}
        <h3 class="backstage-templates-heading">{_('Templates')}</h3>
        <div class="template-search-bar">
          {searchInput(props.searchQuery, props.onSearchInput)}
          {filterChips(props.filter, props.onFilterChange)}
        </div>
        {props.sections}
      </div>
    );
  }

  function blankTemplatesRow(
    templates: TemplateData[],
    onClick: (t: TemplateData) => void,
  ): HTMLElement {
    return (
      <div class="template-blank-row">
        {templates.map((t) =>
          templateCard(t, () => onClick(t), { isBlank: true }),
        )}
      </div>
    );
  }

  export function typeLabel(type: TemplateType): string {
    const labels: Record<TemplateType, string> = {
      writer: _('Document'),
      calc: _('Spreadsheet'),
      impress: _('Presentation'),
      draw: _('Drawing'),
    };
    return labels[type];
  }

  function filterChips(
    active: TemplateFilter,
    onChange: (filter: TemplateFilter) => void,
  ): HTMLElement {
    const chips: [TemplateFilter, string][] = [
      ['all', _('All')],
      ...(['writer', 'calc', 'impress', 'draw'] as TemplateType[]).map(
        (type): [TemplateFilter, string] => [type, typeLabel(type)],
      ),
    ];

    const select = (event: Event, filter: TemplateFilter) => {
      const chosen = event.currentTarget as HTMLElement;
      chosen.parentElement
        .querySelectorAll('.template-type-button')
        .forEach((chip) => {
          chip.classList.toggle('active', chip === chosen);
          chip.setAttribute('aria-pressed', String(chip === chosen));
        });
      onChange(filter);
    };

    return (
      <div class="template-filter-bar" role="group" aria-label={_('Type')}>
        {chips.map(([filter, label]) => (
          <button
            type="button"
            class={
              filter === active
                ? 'template-type-button active'
                : 'template-type-button'
            }
            aria-pressed={String(filter === active)}
            onClick={(event: Event) => select(event, filter)}
          >
            {label}
          </button>
        ))}
      </div>
    );
  }

  export function templateSections(props: TemplateSectionsProps): HTMLElement {
    return (
      <div class="backstage-template-sections">
        {props.userTemplates
          ? myTemplatesSection({
              templates: props.userTemplates,
              onTemplateClick: props.onClick,
              onAddClick: props.onAddTemplate,
              onManageClick: props.onManageTemplates,
              favoriteState: props.favoriteState,
            })
          : null}
        <div class="backstage-template-section-header">
          <h4 class="backstage-template-section-title">
            {_('Built-in templates')}
          </h4>
        </div>
        {templateGrid(
          props.builtInTemplates,
          props.searchQuery,
          props.onClick,
          props.favoriteState,
        )}
      </div>
    );
  }

  function templateGrid(
    templates: TemplateData[],
    searchQuery: string,
    onClick: (t: TemplateData) => void,
    favoriteState: (template: TemplateData) => TemplateFavoriteState,
  ): HTMLElement {
    let emptyText: string | null = null;
    if (!templates.length)
      emptyText = searchQuery.trim()
        ? _('No templates match your search.')
        : _('No templates of this type.');
    return (
      <div class="backstage-templates-grid">
        {emptyText ? <div class="template-grid-empty">{emptyText}</div> : null}
        {templates.map((t) =>
          templateCard(t, () => onClick(t), { favorite: favoriteState(t) }),
        )}
      </div>
    );
  }

  function searchInput(
    query: string,
    onInput: (value: string) => void,
  ): HTMLElement {
    return (
      <div class="template-search">
        <span class="template-search-icon" aria-hidden="true" />
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
