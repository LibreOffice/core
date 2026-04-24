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
 * window.L.Control.Notebookbar - container for tabbed menu on the top of application
 */

/* global $ _ _UNO JSDialog app */
window.L.Control.Notebookbar = window.L.Control.extend({

	_showNotebookbar: false,
	_RTL: false,
	_lastContext: null,
	_lastSelectedTabName: null,

	container: null,
	builder: null, // see NotebookbarBase
	model: null, // see NotebookbarBase

	HOME_TAB_ID: 'Home-tab-label',
	FORMULAS_TAB_ID: 'Formula-tab-label',

	additionalShortcutButtons: [],

	setBuilder: function(builder, model) {
		this.builder = builder;
		this.model = model;
	},

	getWidgetSnapshot(id) {
		return this.model.getById(id);
	},

	// happens only once
	onAdd: function (map) {
		// log and test window.ThisIsTheiOSApp = true;
		this.map = map;
		this.additionalShortcutButtons = [];

		if (window.L.Browser.cypressTest) {
			window.app.allDialogs = this.getListOfUnoCommandsForDialogs();
		}

		// initialize the model only once, remember updates from core
		if (this.model.getSnapshot() === null)
			this.model.fullUpdate(this.getFullJSON(this.HOME_TAB_ID));

		this.map.on('notebookbar', this.onNotebookbar, this);
	},

	// override in subclasses
	onCallback: function () {
		return false; // consumed
	},

	// on show
	create: function(container) {
		const docType = this._map.getDocType();

		if (document.documentElement.dir === 'rtl')
			this._RTL = true;

		this.container = container;

		this.loadTab();

		this.onContextChange = this.onContextChange.bind(this);
		app.events.on('contextchange', this.onContextChange);
		app.events.on('updatepermission', this.onUpdatePermission.bind(this));
		this.map.on('darkmodechanged', this.onDarkModeToggleChange, this);
		this.map.on('showannotationschanged', this.onShowAnnotationsChange, this);
		this.map.on('a11ystatechanged', this.onAccessibilityToggleChange, this);
		if (docType === 'presentation') {
			this.map.on('updateparts', this.onSlideHideToggle, this);
			this.map.on('toggleslidehide', this.onSlideHideToggle, this);
		}

		$('#toolbar-wrapper').addClass('hasnotebookbar');
		$('.main-nav').addClass('hasnotebookbar');
		this.floatingNavIcon = document.querySelector('.navigator-btn-wrapper');
		if (this.floatingNavIcon)
			this.floatingNavIcon.classList.add('hasnotebookbar');
		document.getElementById('document-container').classList.add('notebookbar-active');

		if (!window.logoURL || window.logoURL != "none") {
			const docLogoHeader = window.L.DomUtil.create('div', '');
			docLogoHeader.id = 'document-header';

			let iconClass = '';
			let iconTooltip;
			if (!window.logoURL) {
				[iconClass, iconTooltip] = app.LOUtil.getDocumentLogoClass(docType);
			}
			const docLogo = window.L.DomUtil.create('a', 'document-logo ' + iconClass, docLogoHeader);

			docLogo.setAttribute('id', 'document-logo');
			docLogo.setAttribute('type', 'action');
			docLogo.setAttribute('target', '_blank');
			docLogo.setAttribute('tabIndex', 0);

			if (iconTooltip) {
				docLogo.setAttribute('data-cooltip', iconTooltip);
			}
			// Mirror data-cooltip onto aria-label so the accessible name
			// matches the visible tooltip, even when branding overrides
			// data-cooltip after load (e.g. to "Collabora Online"). When
			// branding also sets an href, target="_blank" takes effect and
			// the link opens a new tab, so announce that to screen readers.
			const syncAriaLabel = () => {
				const cooltip = docLogo.getAttribute('data-cooltip');
				if (!cooltip) return;
				const label = docLogo.getAttribute('href')
					? _('{0} website, opens in new tab').replace('{0}', cooltip)
					: cooltip;
				docLogo.setAttribute('aria-label', label);
			};
			syncAriaLabel();
			new MutationObserver(syncAriaLabel).observe(docLogo, {
				attributes: true,
				attributeFilter: ['data-cooltip', 'href'],
			});
			window.L.control.attachTooltipEventListener(docLogo, this.map);
			$('.main-nav').prepend(docLogoHeader);

			if (window.logoURL) {
				docLogo.style.backgroundImage = "url(" + window.logoURL + ")";
			}
		}

		const isDarkMode = window.prefs.getBoolean('darkTheme');
		if (!isDarkMode)
			$('#invertbackground').hide();

		if (!this.map.serverAuditDialog) {
			this.hideItem('server-audit');
			this.hideItem('help-serveraudit-break');
		}
	},

	onRemove: function() {
		app.events.off('contextchange', this.onContextChange);
		$('.main-nav #document-header').remove();
		$('.main-nav').removeClass('hasnotebookbar');
		$('#toolbar-wrapper').removeClass('hasnotebookbar');
		if (this.floatingNavIcon)
			this.floatingNavIcon.classList.remove('hasnotebookbar');
		$('.main-nav #document-header').remove();
		this.clearNotebookbar();
		$(this.container).remove();
	},

	onUpdatePermission: function(e) {
		if (e.detail.perm === 'edit') {
			this._showNotebookbar = true;
			this.showTabs();
			$('.main-nav').removeClass('readonly');
		} else {
			this.hideTabs();
		}
	},

	onNotebookbar: function(data) {
		this.setInitialized(true);
		// setup id for events
		this.builder.setWindowId(data.id);
	},

	setInitialized: function(initialized) {
		if (this._isNotebookbarLoadedOnCore === initialized)
			return;

		this._isNotebookbarLoadedOnCore = initialized;
		app.console.debug('Notebookbar: set initialized: ' + initialized);

		if (this.container) {
			if (initialized)
				this.container.classList.add('initialized');
			else
				this.container.classList.remove('initialized');
		}

		if (initialized) {
			app.serverConnectionService.onNotebookbarInCoreInit();
		}
	},

	showTabs: function() {
		$('.ui-tabs.notebookbar').show();
		$('.notebookbar-shortcuts-bar').show();
		this.extend();
		$(window).resize();
	},

	hideTabs: function() {
		$('.ui-tabs.notebookbar').hide();
		$('.notebookbar-shortcuts-bar').hide();
		$('.main-nav').addClass('readonly');
		this.collapse();
	},

	collapse: function() {
		if (this._showNotebookbar !== false) this.map.uiManager.collapseNotebookbar();
	},

	extend: function() {
		if (this._showNotebookbar !== false) this.map.uiManager.extendNotebookbar();
	},

	isCollapsed: function() {
		return this.map.uiManager.isNotebookbarCollapsed();
	},

	clearNotebookbar: function() {
		// viewMode and shareas are injected into the optionstoolbox, which belongs to the notebookbar.
		// When switching to Viewing mode the notebookbar is removed, so we first detach
		// viewMode and shareas to keep both from disappearing.
		this._detachViewModeAndShareAs();

		$('.root-container.notebookbar').remove();
		$('.notebookbar-tabs-container').remove();
		$('.notebookbar-shortcuts-bar').remove();
	},

	loadTab: function() {
		app.console.debug('Notebookbar: loadTab');

		this.clearNotebookbar();

		this.builder.build(this.container, [this.model.getSnapshot()]);

		if (this._showNotebookbar === false)
			this.hideTabs();

		if (window.mode.isDesktop() || window.mode.isTablet())
			this.createOptionsSection();

		JSDialog.RefreshScrollables();
	},

	setTabs: function(tabs) {
		var container = window.L.DomUtil.create('div', 'notebookbar-tabs-container');
		container.appendChild(tabs);
		for (let tab of tabs.children) {
			if (tab.id.endsWith('-tab-label')) {
				let name = tab.id.substring(0, tab.id.length - 10);
				if (!this.map.uiManager.isTabVisible(name)) {
					$(tab).hide();
				}
			}
		}
		$('#document-titlebar').before(container);
		this.createShortcutsBar();
	},

	selectedTab: function(tabName) {
		// implement in child classes
		this._lastSelectedTabName = tabName;
	},

	isTabSelected: function(tabName) {
		return this._lastSelectedTabName === tabName;
	},

	getTabs: function() {
		// implement in child classes
		return [];
	},

	getTabsJSON: function() {
		// implement in child classes
		return [];
	},

	getShortcutsBarData: function() {
		var hasSave = !this._map['wopi'].HideSaveOption;
		return [
			{
				'id': 'shortcutstoolbox',
				'type': 'toolbox',
				'children': [
					hasSave ? {
						'id': 'save',
						'type': 'toolitem',
						'text': _('Save'),
						'command': '.uno:Save',
						'accessKey': '1',
						'isCustomTooltip': true
					} : {}
				]
			}
		];
	},

	createShortcutsBar: function() {
		var shortcutsBar = window.L.DomUtil.create('div', 'notebookbar-shortcuts-bar');
		$('#main-menu-state').after(shortcutsBar);

		if (window.mode.isDesktop()) {
			$('#main-menu-state').attr('type', 'hidden');
		}

		var shortcutsBarData = this.getShortcutsBarData();
		var toolitems = shortcutsBarData[0].children;

		for (var i in this.additionalShortcutButtons) {
			var item = this.additionalShortcutButtons[i];
			toolitems.push(item);
		}

		for (var j in toolitems) {
			item = toolitems[j];
			var hidden = false;
			var commands = this.map._extractCommand(item);
			commands.forEach(function(command) {
				if (!this.map.uiManager.isCommandVisible(command)) {
					toolitems.splice(j, 1);
					hidden = true;
				}
			}.bind(this));
			if (hidden) {
				break;
			}
			if (!this.map.uiManager.isButtonVisible(item.id)) {
				toolitems.splice(j, 1);
				break;
			}
		}

		this.builder.build(shortcutsBar, shortcutsBarData);

		//create SaveState object after addition of shortcut bar in UI
		this.map.saveState = new app.definitions.saveState(this.map);
	},

	reloadShortcutsBar: function() {
		if (!document.querySelector('.notebookbar-shortcuts-bar'))
			return;

		app.layoutingService.appendLayoutingTask(() => {
			$('.notebookbar-shortcuts-bar').remove();
			this.createShortcutsBar();
		});
	},

	insertButtonToShortcuts: function(button) {
		for (var i in this.additionalShortcutButtons) {
			var item = this.additionalShortcutButtons[i];
			if (item.id === button.id)
				return;
		}

		var isUnoCommand = button.unoCommand && button.unoCommand.indexOf('.uno:') >= 0;
		if (button.unoCommand && !isUnoCommand)
			button.unoCommand = '.uno:' + button.unoCommand;

		this.additionalShortcutButtons.push(
			{
				id: button.id,
				type: 'toolitem',
				text: button.label ? button.label : (button.hint ? _(button.hint) : ' '),
				icon: button.imgurl,
				command: button.unoCommand,
				accessKey: button.accessKey ? button.accessKey: null,
				postmessage: button.unoCommand ? undefined : true,
				cssClass: 'integrator-shortcut'
			}
		);

		this.reloadShortcutsBar();
	},

	showNotebookbarCommand: function(commandId, show) {
		var cssClass;
		if (commandId.indexOf('.uno:') == 0) {
			cssClass = 'uno' + commandId.substring(5);
		} else {
			cssClass = commandId;
		}
		var button = $(this.container).find('div.' + cssClass);
		if (button) {
			// TODO: remember state like this.showItem
			if (show) {
				button.show();
			} else {
				button.hide();
			}
			return true;
		}
		return false;
	},

	shouldIgnoreContextChange(contexts, appId) {
		// New -> old context name pairs.
		let ignored = [['NotesPage', 'DrawPage'], ['DrawPage', 'NotesPage'],
			['Graphic', 'DrawPage', 'Animation'], ['DrawPage', 'Graphic', 'Animation']];
		if (appId === 'com.sun.star.text.TextDocument') {
			ignored.push(['Text', '']);
		}

		for (let i = 0; i < ignored.length; i++) {
			if ((ignored[i].length < 3 || this._lastSelectedTabName === ignored[i][2])
				&& contexts[0] === ignored[i][0] && contexts[1] === ignored[i][1])
				return true;
		}

		return false;
	},

	refreshContextTabsVisibility: function() {
		this.updateTabsVisibilityForContext(this._lastContext);
	},

	updateButtonVisibilityForContext: function (context, tabId) {
		const tabsJSON = this.getTabsJSON();
		const splitTabId = tabId.split('-');
		if (splitTabId.length !== 3)
			return;

		const tabName = splitTabId[0];
		const toShow = [];
		const toHide = [];

		tabsJSON.forEach((tabContent) => {
			if (!tabContent || !tabContent.children[0] || !tabContent.children[0].children) return;

			const tabPageId = tabContent.children[0].id;
			const tabPageName = tabPageId.split('-')[0];
			if (tabPageName !== tabName)
				return;

			const children = tabContent.children[0].children;
			const requiredContext = context || 'default';

			children.forEach((item) => {
				if (!item.context) return;

				if (item.context.indexOf(requiredContext) >= 0) {
					toShow.push(item.command.replace('.uno:', ''));
				} else {
					toHide.push(item.command.replace('.uno:', ''));
				}
			});
		});

		toHide.forEach((item) => {
			this.showButton(item, false);
		});
		toShow.forEach((item) => {
			this.showButton(item, true);
		});
	},

	showButton: function (id, show) {
		if (!id) return;

		this.builder.executeAction(this.container, {
			control_id: id,
			action_type: show ? 'show' : 'hide',
		});

		JSDialog.RefreshScrollables();
	},

	updateTabsVisibilityForContext: function(requestedContext) {
		var tabs = this.getTabs();
		var contextTab = null;
		var defaultTab = null;
		let alreadySelected = null;
		// Currently selected tab name, part of the element's ID.
		let currentlySelectedTabName = null;
		for (var tab in tabs) {
			var tabElement = $('#' + tabs[tab].name + '-tab-label');
			if (tabElement.hasClass('selected')) {
				currentlySelectedTabName = tabs[tab].name;
			}
			if (tabs[tab].context) {
				tabElement.hide();
				var contexts = tabs[tab].context.split('|');
				for (var context in contexts) {
					// Check the tab isn't hidden.
					if (!this.map.uiManager.isTabVisible(tabs[tab].name)) {
						continue;
					}
					if (contexts[context] === requestedContext) {
						tabElement.show();
						tabElement.removeClass('hidden');
						if (!tabElement.hasClass('selected'))
							contextTab = tabElement;
						else
							alreadySelected = tabElement;
					} else if (contexts[context] === 'default') {
						tabElement.show();
						if (!tabElement.hasClass('selected'))
							defaultTab = tabElement;
					}
				}
			} else if (!this.map.uiManager.isTabVisible(tabs[tab].name)) {
				// There is no context, but we check if the tab is hidden
				tabElement.hide();
			} else {
				tabElement.show();
			}
		}

		if (alreadySelected) {
			const tabId = alreadySelected.attr('id');
			this.updateButtonVisibilityForContext(requestedContext, tabId);
			return;
		}

		const docType = this._map.getDocType();

		if (docType === 'spreadsheet' && this.isTabSelected('Formulas')) {
			this.updateButtonVisibilityForContext(requestedContext, this.FORMULAS_TAB_ID);
			return;
		}

		if (contextTab) {
			// Switch to the tab of the context, unless we currently show the review tab
			// for text documents, where jumping to the next change would possibly
			// switch to the Home or Table tabs, which is not wanted.
			if (docType !== 'text' || currentlySelectedTabName !== 'Review') {
				contextTab.click();
			}
			const tabId = contextTab.attr('id');
			this.updateButtonVisibilityForContext(requestedContext, tabId);
			return;
		}

		if (defaultTab) {
			defaultTab.click();
			const tabId = defaultTab.attr('id');
			this.updateButtonVisibilityForContext(requestedContext, tabId);
			return;
		}
	},

	onContextChange: function(event) {
		const detail = event.detail;
		if (detail.appId !== detail.oldAppId) {
			var childrenArray = undefined; // Use buttons provided by specific Control.Notebookbar implementation by default
			if (detail.appId === 'com.sun.star.formula.FormulaProperties') {
				childrenArray = [
					{
						'type': 'toolitem',
						'text': _UNO('.uno:SidebarDeck.ElementsDeck', '', true),
						'command': '.uno:SidebarDeck.ElementsDeck'
					}
				];
			}
			this.createOptionsSection(childrenArray);
		}

		if (detail.context === detail.oldContext)
			return;

		if (this.shouldIgnoreContextChange([detail.context, detail.oldContext], detail.appId))
			return;

		this.updateTabsVisibilityForContext(detail.context);
		this._lastContext = detail.context;
	},

	onSlideHideToggle: function() {
		if (!app.impress.isSlideHidden(this.map.getCurrentPartNumber()))
			$('#showslide').hide();
		else
			$('#showslide').show();

		if (app.impress.isSlideHidden(this.map.getCurrentPartNumber()))
			$('#hideslide').hide();
		else
			$('#hideslide').show();
	},

	onDarkModeToggleChange: function() {
		if (window.prefs.getBoolean('darkTheme')) {
			$('#invertbackground').show();
		}
		else {
			$('#invertbackground').hide();
		}
	},

	onShowAnnotationsChange: function(e) {
		if (e.state === 'true')
		{
			$('#review-show-resolved-annotations').removeClass('disabled');
			$('#review-show-resolved-annotations').attr('disabled', false);
			$('#review-show-resolved-annotations-button').attr('disabled', false);
		}
		else
		{
			$('#review-show-resolved-annotations').addClass('disabled');
			$('#review-show-resolved-annotations').attr('disabled', true);
			$('#review-show-resolved-annotations-button').attr('disabled', true);
		}
	},

	onAccessibilityToggleChange: function() {
		if (window.prefs.getBoolean('accessibilityState')) {
			$('#togglea11ystate').addClass('selected');
		} else {
			$('#togglea11ystate').removeClass('selected');
		}
		if (this._map && this._map._lockAccessibilityOn) {
			$('#togglea11ystate').addClass('disabled');
			$('#togglea11ystate').attr('disabled', true);
			$('#togglea11ystate-button').attr('disabled', true);
		}
	},

	buildOptionsSectionData: function(childrenArray) {
		return [
			{
				'id': 'optionscontainer',
				'type': 'container',
				'children': [
					{
						'id': 'optionstoolboxdown',
						'type': 'toolbox',
						'children': childrenArray
					}
				]
			}
		];
	},

	getOptionsSectionData: function() {
		return this.buildOptionsSectionData(this.getDefaultToolItems());
	},

	getDefaultToolItems: function() {
		const optionsToolItems = [
			{
				'type': 'toolitem',
				'text': _UNO('.uno:Sidebar', '', true),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: false, combination: 'ZB', de: null },
				'useInLineLabelsForUnoButtons': false,
			},
		];

		if (this._map && this._map['wopi'].EnableShare) {
			optionsToolItems.push({
				'type': 'customtoolitem',
				'text': _('Share'),
				'command': 'shareas',
				'inlineLabel': true,
				'accessibility': { focusBack: false, combination: 'ZS', de: null },
			});
		}

		return optionsToolItems;
	},

	_detachButtonFromNotebookbar: function (buttonId, targetId) {
		const button = document.getElementById(buttonId);
		if (!button) return;

		const optionsSection = document.querySelector('.notebookbar-options-section');
		if (!optionsSection || !optionsSection.contains(button)) return;

		const target = document.getElementById(targetId);
		if (!target || !target.parentNode) return;

		target.parentNode.insertBefore(button, target);
	},

	_detachViewModeAndShareAs: function () {
		this._detachButtonFromNotebookbar('shareas', 'closebuttonwrapperseparator');

		const viewModeTarget = document.getElementById('shareas') ? 'shareas' : 'closebuttonwrapperseparator';
		this._detachButtonFromNotebookbar('viewMode', viewModeTarget);
	},

	_moveViewModeIntoOptionsToolbox: function () {
		// Check for viewMode which exists in cool.html.m4
		const viewMode = document.getElementById('viewMode');
		if (!viewMode)
			return;

		const optionsSection = document.querySelector('.notebookbar-options-section');
		if (!optionsSection)
			return;

		const toolboxRow = optionsSection.querySelector('.toolbox.level-0#optionstoolboxdown');
		if (!toolboxRow)
			return;

		// Move viewMode before Share (if Share exists), otherwise append to the end
		const share = toolboxRow.querySelector('#shareas');
		if (share)
			toolboxRow.insertBefore(viewMode, share);
		else
			toolboxRow.appendChild(viewMode);
	},

	createOptionsSection: function(childrenArray) {
		// First detach viewMode and shareas to avoid them being removed with the options section
		this._detachViewModeAndShareAs();
		$('.notebookbar-options-section').remove();
		// Remove shareas if it still exists, to avoid duplication in creation
		$('#shareas').remove();

		var optionsSection = window.L.DomUtil.create('div', 'notebookbar-options-section');
		$(optionsSection).insertBefore('#closebuttonwrapperseparator');

		var builderOptions = {
			mobileWizard: this,
			map: this.map,
			cssClass: 'notebookbar',
			suffix: 'notebookbar',
		};

		var builder = new window.L.control.notebookbarBuilder(builderOptions);
		if (childrenArray === undefined)
			childrenArray = this.getOptionsSectionData();
		builder.build(optionsSection, childrenArray);

		this._moveViewModeIntoOptionsToolbox();
	},

	// dynamically show/hide items

	// use getter to hide usage of UIManager's hiddenItems for centralization
	getHiddenItems() {
		if (!this._map || !this._map.uiManager)
			return null;

		return this._map.uiManager.hiddenItems;
	},

	hideItem: function(itemId) {
		app.console.debug('Notebookbar: hide item: ' + itemId);

		this.showItemImpl(itemId, false);

		return true;
	},

	showItem: function(itemId) {
		app.console.debug('Notebookbar: show item: ' + itemId);

		this.showItemImpl(itemId, true);

		return true;
	},

	showItemImpl: function(itemId, show) {
		app.map.fire('jsdialogaction', { data: {
				jsontype: 'notebookbar',
				action: 'action',
				data: {
					control_id: itemId,
					action_type: show ? 'show' : 'hide'
				}
			}
		});
	}
});
