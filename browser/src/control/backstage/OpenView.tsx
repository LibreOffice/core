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
 * "Open" view: a row of large tiles for opening a document - one for the
 * local file picker, then a vertical separator, then one per registered
 * cloud provider. Below that, a divider with an "Add Cloud +" action on
 * the right. The view passes resolved tile data and click handlers; this
 * file owns only the structure.
 */

namespace BackstageTemplates {
  export interface CloudTileData {
    id: string;
    typeName: string;
    iconName: string;
    userName: string;
    onClick: () => void;
    onEdit: () => void;
  }

  export interface OpenViewProps {
    onOpenLocal: () => void;
    cloudTiles: CloudTileData[];
    onAddCloudClick: () => void;
  }

  export function openView(props: OpenViewProps): HTMLElement {
    return (
      <>
        {sectionHeader(
          _('Open'),
          _('Open a document from your computer or a cloud location.'),
        )}
        <div class="backstage-open-tiles-row">
          {openTile({
            label: _('Open File from Computer'),
            iconName: 'lc_open.svg',
            onClick: props.onOpenLocal,
          })}
          <div
            class="backstage-open-tiles-separator"
            role="separator"
            aria-orientation="vertical"
          />
          {props.cloudTiles.map(cloudProviderTile)}
        </div>
        <div class="backstage-open-actions-row">
          <div class="backstage-open-actions-divider" />
          <button
            type="button"
            class="backstage-open-add-cloud-button"
            onClick={props.onAddCloudClick}
          >
            {_('Add Cloud +')}
          </button>
        </div>
      </>
    );
  }

  export interface OpenTileProps {
    label: string;
    iconName: string;
    subtitle?: string;
    onClick?: () => void;
    extraClass?: string;
    inert?: boolean;
  }

  export function openTile(props: OpenTileProps): HTMLElement {
    const svg = BackstageSVGIcons[props.iconName];
    const ariaLabel = props.subtitle
      ? `${props.label} (${props.subtitle})`
      : props.label;
    const cls = ['backstage-open-tile', props.extraClass]
      .filter(Boolean)
      .join(' ');
    return (
      <div
        class={cls}
        role="button"
        tabindex={props.inert ? -1 : 0}
        aria-label={ariaLabel}
        aria-hidden={props.inert ? 'true' : undefined}
        onClick={props.inert ? undefined : props.onClick}
      >
        <span
          class="backstage-open-tile-icon"
          aria-hidden="true"
          dangerouslySetInnerHTML={svg ? { __html: svg } : undefined}
        />
        <span class="backstage-open-tile-label">{props.label}</span>
        {props.subtitle ? (
          <span class="backstage-open-tile-sublabel">{props.subtitle}</span>
        ) : null}
      </div>
    );
  }

  function cloudProviderTile(t: CloudTileData): HTMLElement {
    const svg = BackstageSVGIcons[t.iconName];
    const editIcon = BackstageSVGIcons['lc_edit.svg'];
    return (
      <div
        class="backstage-open-tile is-cloud-provider"
        role="button"
        tabindex="0"
        aria-label={`${t.typeName} (${t.userName})`}
        onClick={t.onClick}
      >
        <span
          class="backstage-open-tile-icon"
          aria-hidden="true"
          dangerouslySetInnerHTML={svg ? { __html: svg } : undefined}
        />
        <span class="backstage-open-tile-label">{t.typeName}</span>
        <span class="backstage-open-tile-sublabel">{t.userName}</span>
        <button
          type="button"
          class="backstage-open-tile-edit"
          aria-label={_('Edit account')}
          title={_('Edit account')}
          onClick={(e: MouseEvent) => {
            e.stopPropagation();
            t.onEdit();
          }}
          dangerouslySetInnerHTML={editIcon ? { __html: editIcon } : undefined}
        />
      </div>
    );
  }
}
