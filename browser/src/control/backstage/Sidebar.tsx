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
 * Backstage chrome: the top header bar (title + close button) and the
 * left sidebar (back-to-document button + tab list). The view passes in
 * tab configs and click handlers; this file owns only the structure.
 */

namespace BackstageTemplates {
  interface HeaderProps {
    isStarterMode: boolean;
    onClose: () => void;
  }

  interface SidebarProps {
    isStarterMode: boolean;
    tabs: BackstageTabConfig[];
    onTabClick: (config: BackstageTabConfig) => void;
    onBackClick: () => void;
    saveTabRef: (el: HTMLElement) => void;
  }

  export interface ShellProps {
    isStarterMode: boolean;
    tabs: BackstageTabConfig[];
    onTabClick: (config: BackstageTabConfig) => void;
    onClose: () => void;
    onBackClick: () => void;
    saveTabRef: (el: HTMLElement) => void;
    contentAreaRef: (el: HTMLElement) => void;
  }

  export function shell(props: ShellProps): HTMLElement {
    const containerClass = props.isStarterMode
      ? 'backstage-view hidden is-starter-mode'
      : 'backstage-view hidden';

    return (
      <div class={containerClass} id="backstage-view">
        {header({
          isStarterMode: props.isStarterMode,
          onClose: props.onClose,
        })}
        <div class="backstage-main-wrapper">
          {sidebar({
            isStarterMode: props.isStarterMode,
            tabs: props.tabs,
            onTabClick: props.onTabClick,
            onBackClick: props.onBackClick,
            saveTabRef: props.saveTabRef,
          })}
          <div class="backstage-content" ref={props.contentAreaRef} />
        </div>
      </div>
    );
  }

  export function header(props: HeaderProps): HTMLElement {
    return (
      <div class="backstage-header">
        <span class="backstage-header-title">Collabora Office</span>
        {props.isStarterMode ? null : closeButton(props.onClose)}
      </div>
    );
  }

  function closeButton(onClose: () => void): HTMLElement {
    return (
      <div
        class="backstage-header-close"
        aria-label={_('Close backstage')}
        title={_('Close backstage')}
        onClick={onClose}
      >
        <span class="backstage-header-close-icon" aria-hidden="true" />
      </div>
    );
  }

  export function sidebar(props: SidebarProps): HTMLElement {
    return (
      <div class="backstage-sidebar">
        {props.isStarterMode ? null : backButton(props.onBackClick)}
        <div class="backstage-sidebar-tabs">
          {props.tabs
            .filter((c) => c.visible !== false)
            .map((c) => tabElement(c, props))}
        </div>
      </div>
    );
  }

  function backButton(onClick: () => void): HTMLElement {
    return (
      <div
        class="backstage-sidebar-back"
        aria-label={_('Back to document')}
        title={_('Back to document')}
        onClick={onClick}
      >
        <span class="backstage-sidebar-back-icon" aria-hidden="true" />
      </div>
    );
  }

  function tabElement(
    config: BackstageTabConfig,
    props: SidebarProps,
  ): HTMLElement {
    const svgContent = config.icon ? BackstageSVGIcons[config.icon] : undefined;

    return (
      <div
        class="backstage-sidebar-item"
        id={`backstage-${config.id}`}
        ref={(el) => {
          if (config.id === 'save') props.saveTabRef(el);
        }}
        onClick={() => props.onTabClick(config)}
      >
        {config.icon ? (
          <span
            class="backstage-sidebar-icon"
            aria-hidden="true"
            dangerouslySetInnerHTML={
              svgContent ? { __html: svgContent } : undefined
            }
          />
        ) : null}
        <span>{config.label}</span>
      </div>
    );
  }
}
