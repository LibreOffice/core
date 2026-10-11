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
 * JSDialog.MobileTabControl - widgets handling tabs on mobile
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'tabcontrol',
 *     children: [...]
 * }
 */

/* global JSDialog $ */


// pages: the tab control JSON and the page position of each tab, undefined for panels
function _panelTabsHandler(parentContainer, data, builder, tabTooltip, pages) {
	const isTabControl = pages !== undefined;
	if (!builder.options.useSetTabs)
		console.warn('mobile panelTabsHandler: setTabs will be used ignoring useSetTabs property');

	var tabsContainer = window.L.DomUtil.create('div', 'ui-tabs ' + builder.options.cssClass + ' ui-widget');
	var contentsContainer = window.L.DomUtil.create('div', 'ui-tabs-content ' + builder.options.cssClass + ' ui-widget', parentContainer);

	var tabIdx,item;
	if (!isTabControl) {
		for (var tabIdx = data.length - 1; tabIdx >= 0; tabIdx--) {
			var item = data[tabIdx];
			if (item.hidden === true)
				data.splice(tabIdx, 1);
		}
	}

	var tabs = [];
	var contentDivs = [];
	var labels = [];
	for (tabIdx = 0; tabIdx < data.length; tabIdx++) {
		item = data[tabIdx];

		var title = builder._cleanText(item.text);

		var tab = window.L.DomUtil.create('div', 'ui-tab ' + builder.options.cssClass, tabsContainer);
		tab.id = title;
		tabs[tabIdx] = tab;

		var label = window.L.DomUtil.create('span', 'ui-tab-content ' + builder.options.cssClass + ' unolabel', tab);
		label.textContent = title;
		labels[tabIdx] = title;

		var contentDiv = window.L.DomUtil.create('div', 'ui-content level-' + builder._currentDepth + ' ' + builder.options.cssClass, contentsContainer);
		contentDiv.title = title;

		builder._currentDepth++;
		if (item.children)
		{
			for (var i = 0; i < item.children.length; i++) {
				builder.build(contentDiv, [item.children[i]]);
			}
		}
		else // build ourself inside there
		{
			builder.build(contentDiv, [item]);
		}
		builder._currentDepth--;

		$(contentDiv).addClass('hidden');
		contentDivs[tabIdx] = contentDiv;
	}

	// The engine shows its selected page, so that tab opens first.
	let openTab = isTabControl ? pages.positions.indexOf(pages.selected) : -1;
	if (openTab === -1)
		openTab = 0;

	if (builder.wizard) {
		builder.wizard.setTabs(tabsContainer, builder);

		for (var t = 0; t < tabs.length; t++) {
			// to get capture of 't' right has to be a sub fn.
			var fn = builder._createTabClick(
				builder, t, tabs, contentDivs, labels);
			$(tabs[t]).click(fn);
		}

		if (isTabControl && !pages.control.noCoreEvents) {
			for (let tab = 0; tab < tabs.length; tab++) {
				$(tabs[tab]).click(function() {
					// A tap on the open tab leaves the engine on the page it already shows.
					if (tab === openTab)
						return;
					openTab = tab;
					builder.callback('tabcontrol', 'selecttab', pages.control, pages.positions[tab], builder);
				});
			}
		}
	} else {
		window.app.console.debug('Builder used outside of mobile wizard: please implement the click handler');
	}
	$(tabs[openTab]).click();
	builder.wizard.goLevelDown(contentsContainer);

	return false;
}

// use unified tabs handling in mobile wizard so Tab Control is show exactly like Panels
function _tabsToPanelConverter(parentContainer, data, builder, tabTooltip) {
	var tabs = 0;
	var tabObjects = [];
	for (var tabIdx = 0; data.children && tabIdx < data.children.length; tabIdx++) {
		if (data.children[tabIdx].type === 'tabpage' || data.vertical) {
			tabs++;
			tabObjects.push(data.children[tabIdx]);
		}
	}
	var isMultiTabJSON = tabs > 1;

	const pagePositions = [];
	let selectedPage;

	if (data.tabs) {
		for (tabIdx = 0; data.tabs && tabIdx < data.tabs.length; tabIdx++) {
			var isSelectedTab = data.selected == data.tabs[tabIdx].id;
			if (isSelectedTab) {
				selectedPage = tabIdx;
			}
		}
	}

	if (isMultiTabJSON) {
		var tabId = 0;
		for (tabIdx = 0; tabIdx < data.children.length; tabIdx++) {
			var tab = data.children[tabIdx];

			if (tab.type !== 'tabpage' && !data.vertical)
				continue;

			tabObjects[tabId].text = data.tabs[tabId].text;
			pagePositions.push(tabId);
			tabId++;
		}
	} else {
		for (tabIdx = 0; tabIdx < data.children.length; tabIdx++) {
			tab = data.children[tabIdx];

			if (tab.type !== 'tabpage' && !data.vertical)
				continue;

			tabObjects[0].text = data.tabs[selectedPage].text;
			pagePositions.push(selectedPage);
			break;
		}
	}

	_panelTabsHandler(parentContainer, tabObjects, builder, tabTooltip,
		{ control: data, positions: pagePositions, selected: selectedPage });

	return false;
}

// if the tabs list is too long, it ends up being pretty messy. We should switch to submenus in that case
function _tabsToSubmenuConverter(parentContainer, data, builder) {
    const submenuChildren = [];

    for (const [index, tab] of Object.entries(data.children)) {
        if (tab.type !== 'tabpage') {
            continue
        }

        submenuChildren.push({
            children: tab.children,
            command: tab.command,
            enabled: true,
            executionType: 'menu',
            id: tab.id,
            parent: parentContainer,
            text: data.tabs[index].text,
            type: 'submenu',
        });
    }

    data.children = submenuChildren;

    return JSDialog.container(parentContainer, data, builder);
}


JSDialog.mobileTabControl = function (parentContainer, data, builder) {
	var buildInnerData = _tabsToPanelConverter(parentContainer, data, builder);
	return buildInnerData;
};

JSDialog.mobilePanelControl = function (parentContainer, data, builder) {
	var buildInnerData = _panelTabsHandler(parentContainer, data, builder);
	return buildInnerData;
};

JSDialog.mobileSubmenuTabControl = function (parentContainer, data, builder) {
	var buildInnerData = _tabsToSubmenuConverter(parentContainer, data, builder);
	return buildInnerData;
};
