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
 * Widget.TabControl - the tab component used by desktop JSDialogs.
 *
 * Renders a tablist of buttons and one content panel per tab. The
 * notebookbar and the mobile wizard register their own tab handlers, so
 * this component only ever runs for desktop dialog tabcontrols.
 *
 * When a dialog has more than VERTICAL_TABS_THRESHOLD tabs the rail is
 * placed on the inline-start side and the tabs stack vertically. A
 * horizontal row of that many tabs no longer reads as a row, and a
 * vertical rail also leaves more height for the panel content. The
 * threshold is the largest tab count that still reads comfortably as a
 * horizontal row.
 */

declare var JSDialog: any;

const VERTICAL_TABS_THRESHOLD = 4;

interface TabControlItem {
  id: string | number;
  name: string;
  text: string;
  context?: string;
}

interface TabControlData {
  id: string;
  tabs?: TabControlItem[];
  children?: WidgetJSON[];
  selected?: string | number;
  vertical?: boolean;
  noCoreEvents?: boolean;
}

// A tabpage child is the content of one tab. The vertical flag asks us to
// treat every child as tab content even when it is not a tabpage. This is
// unrelated to the rail layout below: it comes from a dialog built with a
// vertical-tabs notebook, and only changes which children count as content.
function isTabContent(child: WidgetJSON, data: TabControlData): boolean {
  return child.type === 'tabpage' || !!data.vertical;
}

JSDialog.tabControl = function (
  parentContainer: HTMLElement,
  data: TabControlData,
  builder: any,
  tabTooltip: string,
): boolean {
  if (tabTooltip === undefined) tabTooltip = '';

  if (!data.tabs) return false;

  let contentTabs = 0;
  for (let i = 0; data.children && i < data.children.length; i++) {
    if (isTabContent(data.children[i], data)) contentTabs++;
  }
  const isMultiTabJSON = contentTabs > 1;

  const useVerticalRail = data.tabs.length > VERTICAL_TABS_THRESHOLD;

  const tabs: HTMLButtonElement[] = [];
  const tabIds: string[] = [];
  const contentDivs: HTMLElement[] = [];
  let selectedTabIdx: number | null = null;

  const cssClass = builder.options.cssClass;
  const depth = builder._currentDepth;

  const tabsContainer = (
    <div
      class={
        'ui-tabs ' +
        cssClass +
        ' ui-widget' +
        (useVerticalRail ? ' vertical' : '')
      }
      role="tablist"
      aria-orientation={useVerticalRail ? 'vertical' : 'horizontal'}
    />
  ) as HTMLElement;

  const contentsContainer = (
    <div class={'ui-tabs-content ' + cssClass} />
  ) as HTMLElement;

  const rootContainer = (
    <div
      class={'ui-tabs-root ' + cssClass + (useVerticalRail ? ' vertical' : '')}
      id={data.id}
    >
      {/* With useSetTabs the tablist is attached separately by setTabs below,
          so it is left out of the root here. Otherwise it sits in the root. */}
      {builder.options.useSetTabs ? false : tabsContainer}
      {contentsContainer}
    </div>
  ) as HTMLElement;

  for (let tabIdx = 0; tabIdx < data.tabs.length; tabIdx++) {
    const item = data.tabs[tabIdx];
    const title = builder._cleanText(item.text);

    // Core sends a plain number id, so prefix it with the control id to keep
    // it distinct from the same number used by other controls.
    const tabId = Number.isInteger(parseInt(item.id as string))
      ? data.id + '-' + item.id
      : (item.id as string);

    const isSelectedTab = data.selected == item.id;

    const contentDiv = (
      <div
        class={'ui-content level-' + depth + ' ' + cssClass}
        id={item.name}
        role="tabpanel"
        aria-labelledby={tabId}
      />
    ) as HTMLElement;

    const tabLabel = (<span class="ui-tab-label">{title}</span>) as HTMLElement;

    const tab = (
      <button
        class={'ui-tab ' + cssClass}
        id={tabId}
        role="tab"
        aria-controls={contentDiv.id}
      >
        {tabLabel}
      </button>
    ) as HTMLButtonElement;

    JSDialog.AddAriaLabel(tab, item, builder);
    builder._setAccessKey(tab, builder._getAccessKeyFromText(item.text));
    builder._stressAccessKey(tabLabel, tab.accessKey);

    if (isSelectedTab) {
      tab.classList.add('selected');
      tab.setAttribute('aria-selected', 'true');
      tab.tabIndex = 0;
      tab.setAttribute('data-cooltip', tabTooltip);
      selectedTabIdx = tabIdx;
    } else {
      tab.setAttribute('aria-selected', 'false');
      tab.tabIndex = -1;
      contentDiv.classList.add('hidden');
    }

    if (item.context) {
      const hasCurrentContext =
        builder.map.context.context !== '' &&
        item.context.indexOf(builder.map.context.context) !== -1;
      const hasDefaultContext = item.context.indexOf('default') !== -1;
      if (!hasCurrentContext && !hasDefaultContext) tab.classList.add('hidden');
    }

    tabsContainer.appendChild(tab);
    contentsContainer.appendChild(contentDiv);

    tabs[tabIdx] = tab;
    tabIds[tabIdx] = item.name;
    contentDivs[tabIdx] = contentDiv;
  }

  parentContainer.appendChild(rootContainer);

  if (builder.wizard) {
    if (builder.options.useSetTabs)
      builder.wizard.setTabs(tabsContainer, builder);

    tabs.forEach(function (tab, index) {
      const selectTab = builder._createTabClick(
        builder,
        index,
        tabs,
        contentDivs,
        tabIds,
      );
      tab.addEventListener('click', function (event: Event) {
        selectTab(event);
        if (!data.noCoreEvents)
          builder.callback(
            'tabcontrol',
            'selecttab',
            rootContainer,
            index,
            builder,
          );
      });
    });

    JSDialog.KeyboardTabNavigation(tabs, contentDivs, useVerticalRail);
  } else {
    window.app.console.debug(
      'Builder used outside of mobile wizard: please implement the click handler',
    );
  }

  if (isMultiTabJSON) {
    let contentIdx = 0;
    for (let i = 0; i < data.children.length; i++) {
      const child = data.children[i];
      if (!isTabContent(child, data)) continue;
      builder.build(contentDivs[contentIdx], [child], false, false);
      contentIdx++;
    }
  } else if (selectedTabIdx != null) {
    for (let i = 0; i < data.children.length; i++) {
      const child = data.children[i];
      if (!isTabContent(child, data)) continue;
      builder.build(contentDivs[selectedTabIdx], [child], false, false);
      break;
    }
  }

  return false;
};
