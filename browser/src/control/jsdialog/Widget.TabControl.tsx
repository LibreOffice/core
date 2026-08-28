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
 *
 * A dialog holding the empty None fill page gets the rail at any tab count,
 * and tabs inside a tab page stay a row.
 */

declare var JSDialog: any;

const VERTICAL_TABS_THRESHOLD = 4;

// The None fill type, named after its tab label in cui/uiconfig/ui/areatabpage.ui.
const NONE_FILL_TAB = 'lbnone';

function setTabEnabled(
  row: HTMLElement,
  tab: HTMLButtonElement,
  tooltip: string,
  enabled: boolean,
) {
  tab.setAttribute('aria-disabled', enabled ? 'false' : 'true');
  row.setAttribute('data-cooltip', enabled ? '' : tooltip);
}

interface TabControlItem {
  id: string | number;
  name: string;
  text: string;
  context?: string;
}

interface PageFill {
  notebook: TabControlData;
  noneIndex: number;
  selectedIndex: number;
  isNone: boolean;
}

interface TabControlData {
  id: string;
  tabs?: TabControlItem[];
  children?: WidgetJSON[];
  selected?: string | number;
  vertical?: boolean;
  noCoreEvents?: boolean;
  tabControlDepth?: number;
  hasEmptyTabPage?: boolean;
}

// A tabpage child is the content of one tab. The vertical flag asks us to
// treat every child as tab content even when it is not a tabpage. This is
// unrelated to the rail layout below: it comes from a dialog built with a
// vertical-tabs notebook, and only changes which children count as content.
function isTabContent(child: WidgetJSON, data: TabControlData): boolean {
  return child.type === 'tabpage' || !!data.vertical;
}

function isNoneFillTab(tab: TabControlItem): boolean {
  return tab.name === NONE_FILL_TAB;
}

function getTabPages(data: TabControlData): WidgetJSON[] {
  const pages: WidgetJSON[] = [];
  const children = data.children;
  for (let i = 0; children && i < children.length; i++) {
    if (isTabContent(children[i], data)) pages.push(children[i]);
  }

  return pages;
}

// The fill types below this widget: the notebook offering a None type, and where it sits.
function getPageFillTabInfo(widget: WidgetJSON): PageFill | null {
  const notebook = widget ? (widget as TabControlData) : null;
  const tabs = notebook && widget.type === 'tabcontrol' ? notebook.tabs : null;
  const noneIndex = tabs ? tabs.findIndex(isNoneFillTab) : -1;

  if (noneIndex >= 0) {
    const selectedIndex = tabs.findIndex(function (tab) {
      return notebook.selected == tab.id;
    });
    return {
      notebook: notebook,
      noneIndex: noneIndex,
      selectedIndex: selectedIndex,
      isNone: noneIndex === selectedIndex,
    };
  }

  const children = widget ? widget.children : null;
  for (let i = 0; children && i < children.length; i++) {
    const found = getPageFillTabInfo(children[i]);
    if (found) return found;
  }

  return null;
}

// The type each fill notebook showed last, so setting the checkbox comes back to it.
const lastFillIndex = new Map<string, number>();

JSDialog.tabControlHasEmptyPage = function (widget: WidgetJSON): boolean {
  return getPageFillTabInfo(widget) !== null;
};

JSDialog.tabControl = function (
  parentContainer: HTMLElement,
  data: TabControlData,
  builder: any,
  tabTooltip: string,
): boolean {
  if (tabTooltip === undefined) tabTooltip = '';

  if (!data.tabs) return false;

  const pages = getTabPages(data);
  const isMultiTabJSON = pages.length > 1;

  // Pages line up with tabs one for one, unless core sent only the page that is showing.
  const tabbedPages = pages.length === data.tabs.length ? pages : [];

  const ownNoneIndex = data.tabs.findIndex(isNoneFillTab);

  const useVerticalRail =
    !data.tabControlDepth &&
    (data.tabs.length > VERTICAL_TABS_THRESHOLD || !!data.hasEmptyTabPage);

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

  // Core keeps reporting a page as the selected one while its fill is None, and that page
  // opens on nothing, so the rail opens another one instead.
  let openTabIndex = data.tabs.findIndex(function (item) {
    return data.selected == item.id;
  });
  if (
    isMultiTabJSON &&
    data.tabs.length > 1 &&
    getPageFillTabInfo(tabbedPages[openTabIndex])?.isNone
  )
    openTabIndex = openTabIndex === 0 ? 1 : 0;

  function selectPage(control: any, index: number) {
    builder.callback('tabcontrol', 'selecttab', control, index, builder);
  }

  // The rail row of a page that holds fill types: its tab, and a checkbox for None.
  function fillTabRow(
    tab: HTMLButtonElement,
    title: string,
    fill: PageFill,
    tabIndex: number,
  ): HTMLElement {
    const checkbox = (
      <input type="checkbox" class={'ui-checkbox-input ' + cssClass} />
    ) as HTMLInputElement;

    checkbox.id = tab.id + '-fill-input';
    checkbox.checked = !fill.isNone;
    checkbox.setAttribute('aria-label', title);

    const row = (
      <div class={'ui-tab-row ' + cssClass} role="presentation">
        {tab}
        {checkbox}
      </div>
    ) as HTMLElement;

    const DISABLED_TAB_TOOLTIP = _('Add a fill to edit the {0}').replace(
      '{0}',
      title,
    );

    window.L.control.attachTooltipEventListener(row, builder.map);
    setTabEnabled(row, tab, DISABLED_TAB_TOOLTIP, !fill.isNone);

    checkbox.addEventListener('change', function () {
      const hasFill = checkbox.checked;
      const remembered = lastFillIndex.get(fill.notebook.id);
      const firstFill = fill.noneIndex === 0 ? 1 : 0;

      setTabEnabled(row, tab, DISABLED_TAB_TOOLTIP, hasFill);
      selectPage(
        { id: fill.notebook.id },
        hasFill
          ? remembered !== undefined
            ? remembered
            : firstFill
          : fill.noneIndex,
      );

      // The panel changes here the way a tab click changes it, before core answers.
      if (!hasFill && selectedTabIdx !== tabIndex) return;
      const openIndex = hasFill ? tabIndex : tabIndex === 0 ? 1 : 0;
      if (builder.wizard)
        builder._createTabClick(
          builder,
          openIndex,
          tabs,
          contentDivs,
          tabIds,
        )();
      selectedTabIdx = openIndex;
      selectPage(rootContainer, openIndex);
    });

    return row;
  }

  for (let tabIdx = 0; tabIdx < data.tabs.length; tabIdx++) {
    const item = data.tabs[tabIdx];
    const title = builder._cleanText(item.text);

    // Core sends a plain number id, so prefix it with the control id to keep
    // it distinct from the same number used by other controls.
    const tabId = Number.isInteger(parseInt(item.id as string))
      ? data.id + '-' + item.id
      : (item.id as string);

    const isSelectedTab = tabIdx === openTabIndex;

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

    if (tabIdx === ownNoneIndex) {
      tab.classList.add('hidden');
      tab.setAttribute('aria-hidden', 'true');
      tab.tabIndex = -1;
    }

    const foundTabInfo = useVerticalRail
      ? getPageFillTabInfo(tabbedPages[tabIdx])
      : null;
    if (foundTabInfo) {
      if (!foundTabInfo.isNone)
        lastFillIndex.set(foundTabInfo.notebook.id, foundTabInfo.selectedIndex);
      tabsContainer.appendChild(fillTabRow(tab, title, foundTabInfo, tabIdx));
    } else tabsContainer.appendChild(tab);

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
        if (tab.getAttribute('aria-disabled') === 'true') return;
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
    for (let i = 0; i < pages.length && i < contentDivs.length; i++) {
      builder.build(contentDivs[i], [pages[i]], false, false);
    }
  } else if (selectedTabIdx != null && pages.length) {
    builder.build(contentDivs[selectedTabIdx], [pages[0]], false, false);
  }

  return false;
};
