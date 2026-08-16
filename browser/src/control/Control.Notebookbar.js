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
	_lastContexts: null,
	_lastSelectedTabName: null,
	// Counter for synthesizing unique ids for contributed separators/menus that have
	// no natural id of their own (see _buildContributedItem).
	_nextContributedId: 0,

	// Contexts that are a modal toggle within whatever was already selected,
	// not a navigation to a different part of the document: entering or
	// leaving one of these shouldn't snap the notebookbar to another tab.
	_inPlaceEditingContexts: ['EditCell', 'DrawText'],

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
			app.LOUtil.syncDocumentLogoAriaLabel(docLogo);
			// Only the desktop applications have a backstage view.
			if (window.mode.isCODesktop())
				app.LOUtil.openTemplatesFromDocumentLogo(docLogo, this.map);
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
		this.setInitialized(false);
		$(this.container).remove();
		this.container = null;
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

	// Shared filter used by each doc-type notebookbar's getTabs/getTabsJSON
	// to drop the Extensions entries.  The tab strip (getTabs labels) and the
	// tab pages (getTabsJSON) are zipped by index in NotebookbarBuilder, so the
	// label and its page must be dropped together or every following tab shifts
	// by one.  Drop the Extensions label when extension support is disabled by
	// runtime config, or when no extension has a sidebar panel; the matching
	// page is a null from getExtensionsTab in those cases and is dropped by
	// the !t guard.
	_filterExtensionsTab: function(arr) {
		var exts = app.map._extensions || {};
		var hideExtensionsTab = !window.enableExperimentalFeatures || !Object.keys(exts).some(
			function(id) { return !!exts[id].options.manifest.entry; });
		return arr.filter(function(t) {
			if (!t) return false;
			if (t.name === 'Extensions' && hideExtensionsTab)
				return false;
			return true;
		});
	},

	// Shared entry used by each doc-type notebookbar's getTabsJSON to build
	// the "Extensions" tab: one bigcustomtoolitem toggle per loaded manifest
	// that has a sidebar panel - a commands-only extension (no `entry`) has
	// nothing for this toggle to open, and reaches the notebookbar solely
	// through its own contributed tab(s), see getContributedNotebookbarTabs
	// below.  Click ids start with "extension-toggle-" so
	// docdispatcher.dispatch routes them to ext.toggle().  Call
	// notebookbar.refresh() after loadExtensions resolves to rebuild this
	// tab against the real extension list.
	getExtensionsTab: function() {
		var exts = app.map._extensions || {};
		var ids = Object.keys(exts)
			.filter(function(id) { return !!exts[id].options.manifest.entry; })
			.sort();
		// Drop the Extensions tab entirely when no extension has a sidebar
		// panel: returning null here lets _filterExtensionsTab strip it.
		// refresh() (called once Control.Extension.loadExtensions resolves)
		// rebuilds the notebookbar, so the tab appears as soon as discovery
		// populates app.map._extensions.
		if (ids.length === 0)
			return null;
		var content = [];
		for (var i = 0; i < ids.length; i++) {
			var id = ids[i];
			var manifest = exts[id].options.manifest;
			var baseUrl = exts[id].options.baseUrl;
			content.push({
				'id': 'extension-toggle-' + id,
				'type': 'bigcustomtoolitem',
				'text': manifest.name,
				'icon': manifest.icon
					? baseUrl + manifest.icon
					: app.LOUtil.getURL('images/extension-fallback.svg'),
				'command': 'extension-toggle-' + id,
			});
		}
		//HACK: Control.JSDialogBuilder.build's "hasManyChildren && isContainer" path only
		// emits the <div id="Extensions-container"> wrapper when the inner overflowmanager
		// has more than one child; so pin a trailing dummy spacer so the 1-extension case
		// still produces the wrapper that A11yValidator's checkTabContainerConsistency
		// (and any future selector wanting #Extensions-container) expects:
		content.push({
			'id': 'extensions-tail-pin',
			'type': 'spacer',
		});
		return this.getTabPage('Extensions', content);
	},

	// Builds one notebookbar item from a contributed group's item descriptor -
	// button/separator/menu, the only three kinds an extension may declare (see
	// browser/extensions/README.md).  Returns null (and warns) for an unknown
	// command id or an item type outside that list, so one bad entry is dropped
	// rather than breaking the whole tab.
	_buildContributedItem: function(extId, commands, baseUrl, warnContext, item) {
		function findCommand(commandId) {
			var command = commands.filter(function(c) { return c.id === commandId; })[0];
			if (!command) {
				console.warn(
					'extension ' + extId + ': ' + warnContext +
					' references unknown command "' + commandId + '"');
			}
			return command;
		}
		if (item.type === 'separator') {
			return {
				'id': 'ext:' + extId + ':sep:' + (this._nextContributedId++),
				'type': 'separator',
				'orientation': 'vertical',
			};
		}
		if (item.type === 'button') {
			var command = findCommand(item.command);
			if (!command) return null;
			return {
				// A distinct DOM/model id per occurrence - the same command can be placed
				// as more than one button (different tabs, or both a button and a menu
				// entry), so extId+command alone is not unique.  `command` (below) is what
				// docdispatcher's ext: routing actually reads and stays extId+command.
				'id': 'ext:' + extId + ':btn:' + (this._nextContributedId++),
				'type': item.size === 'large' ? 'bigcustomtoolitem' : 'customtoolitem',
				'text': command.title,
				'icon': command.icon ? baseUrl + command.icon : undefined,
				'command': 'ext:' + extId + ':' + item.command,
			};
		}
		if (item.type === 'menu') {
			var self = this;
			var menu = (item.items || []).map(function(entry) {
				var entryCommand = findCommand(entry.command);
				if (!entryCommand) return null;
				return {
					'id': 'ext:' + extId + ':entry:' + (self._nextContributedId++),
					'text': entryCommand.title,
					'icon': entryCommand.icon ? baseUrl + entryCommand.icon : undefined,
					'action': 'ext:' + extId + ':' + entry.command,
				};
			}).filter(function(entry) { return !!entry; });
			return {
				'id': 'ext:' + extId + ':menu:' + (this._nextContributedId++),
				'type': 'menubutton',
				'text': item.title,
				'icon': item.icon ? baseUrl + item.icon : undefined,
				'menu': menu,
			};
		}
		console.warn(
			'extension ' + extId + ': ' + warnContext +
			' has an item of unknown type "' + item.type + '"');
		return null;
	},

	// Builds one overflowgroup from a contributed group descriptor - a labeled
	// cluster of items, collapsing into a dropdown when the tab is too narrow,
	// the same as every core notebookbar group.
	_buildContributedGroup: function(extId, commands, baseUrl, group) {
		var self = this;
		var warnContext = 'notebookbar group "' + group.id + '"';
		var children = (group.items || []).map(function(item) {
			return self._buildContributedItem(extId, commands, baseUrl, warnContext, item);
		}).filter(function(item) { return !!item; });
		return {
			// group.id (see ExtensionNotebookbarGroup in Control.Extension.ts) is only
			// ever used in a console warning about one of this group's own items - it
			// isn't required to be unique, so it can't be used verbatim as the DOM/model
			// id here.
			'id': 'ext:' + extId + ':group:' + (this._nextContributedId++),
			'type': 'overflowgroup',
			'name': group.label,
			'children': children,
		};
	},

	// Cheap companion to _getContributedNotebookbarTabs below, for getTabs() (the label
	// strip): that build only ever reads a descriptor's name/insertBefore/insertAfter,
	// never `items`, so this skips building any group/button/menu tree - and, since it
	// never touches _nextContributedId, the ids that land in the rendered page (built
	// separately by _getContributedNotebookbarTabs for getTabsJSON) no longer depend on
	// whether the label pass or the page pass happens to run first.
	_getContributedNotebookbarTabNames: function() {
		if (!window.enableExperimentalFeatures) return [];
		var exts = app.map._extensions || {};
		var tabs = [];
		Object.keys(exts).sort().forEach(function(extId) {
			var contributed = exts[extId].options.manifest.contributes
				&& exts[extId].options.manifest.contributes.notebookbar;
			if (!contributed) return;
			contributed.forEach(function(tabSpec) {
				tabs.push({
					extId: extId,
					name: tabSpec.tab,
					insertBefore: tabSpec.insertBefore,
					insertAfter: tabSpec.insertAfter,
				});
			});
		});
		return tabs;
	},

	// Collects every loaded extension's contributes.notebookbar into a flat list of
	// { extId, name, insertBefore, insertAfter, items } descriptors - one per brand-new
	// tab an extension asked for.  `items` is a fully-built content array of
	// overflowgroup/separator entries (a vertical separator is inserted between each
	// pair of groups automatically, matching the visual rhythm of a core notebookbar
	// tab), ready to hand to getTabPage.  Only getTabsJSON() needs this - see
	// _getContributedNotebookbarTabNames above for the cheaper label-only equivalent.
	// Also the one place a duplicate group.id within one tab gets a console warning -
	// called exactly once per rebuild (unlike _insertContributedNotebookbarTabs below,
	// which runs once for the label pass and once for the page pass), so warning here
	// avoids the double-warning that adding the check there would produce.
	_getContributedNotebookbarTabs: function() {
		if (!window.enableExperimentalFeatures) return [];
		var self = this;
		var exts = app.map._extensions || {};
		var tabs = [];
		Object.keys(exts).sort().forEach(function(extId) {
			var manifest = exts[extId].options.manifest;
			var contributed = manifest.contributes && manifest.contributes.notebookbar;
			if (!contributed) return;
			var commands = manifest.contributes.commands || [];
			var baseUrl = exts[extId].options.baseUrl;
			contributed.forEach(function(tabSpec) {
				var items = [];
				var seenGroupIds = {};
				(tabSpec.groups || []).forEach(function(group, i) {
					if (group && Object.prototype.hasOwnProperty.call(seenGroupIds, group.id)) {
						console.warn(
							'extension ' + extId + ': notebookbar tab "' + tabSpec.tab +
							'" has more than one group with id "' + group.id + '"');
					}
					if (group) seenGroupIds[group.id] = true;
					if (i > 0) {
						// Scoped by the shared counter, not by extId+i: the same index i
						// recurs once per tab, so an extension with more than one
						// multi-group tab would otherwise produce this same id twice.
						items.push({
							'id': 'ext:' + extId + ':groupsep:' + (self._nextContributedId++),
							'type': 'separator',
							'orientation': 'vertical',
						});
					}
					items.push(self._buildContributedGroup(extId, commands, baseUrl, group));
				});
				tabs.push({
					extId: extId,
					name: tabSpec.tab,
					insertBefore: tabSpec.insertBefore,
					insertAfter: tabSpec.insertAfter,
					items: items,
				});
			});
		});
		return tabs;
	},

	// Splices `tabs` (either from _getContributedNotebookbarTabNames, for the label
	// array from a doc type's getTabs(), or from _getContributedNotebookbarTabs, for
	// the page array from its getTabsJSON()) into arr, positioned by matching
	// insertBefore/insertAfter against an existing entry's `name` (both label objects
	// and, since getTabPage now tags its return value with `name` too, tabpage objects
	// carry one), defaulting to the end of the ribbon when neither is given or the
	// named anchor isn't found.  buildEntry turns one descriptor into whichever shape
	// this particular array needs.  The caller decides which of the two tab-list
	// getters to call - taking the already-built list as a parameter, rather than this
	// function calling one of them itself, is what lets getTabs() skip building the
	// (for it, unused) item/group/button tree entirely instead of building and
	// discarding it.
	//
	// `validate`, when true, warns about the manifest-authoring mistakes this function
	// can actually detect - a contributed tab name colliding with an existing tab, both
	// insertBefore and insertAfter set on the same tab (insertBefore silently wins), or
	// either one naming a tab that doesn't exist (silently falls back to the end of the
	// ribbon).  Pass true from exactly one of the two calls per doc type (getTabsJSON's)
	// so a misconfigured tab is reported once per rebuild, not twice.
	_insertContributedNotebookbarTabs: function(arr, tabs, buildEntry, validate) {
		var builtInNames = null;
		if (validate) {
			builtInNames = {};
			arr.forEach(function(t) { if (t && t.name) builtInNames[t.name] = true; });
		}
		tabs.forEach(function(tab) {
			if (validate) {
				if (Object.prototype.hasOwnProperty.call(builtInNames, tab.name)) {
					console.warn(
						'extension ' + tab.extId + ': notebookbar tab "' + tab.name +
						'" collides with an existing tab name');
				}
				if (tab.insertBefore && tab.insertAfter) {
					console.warn(
						'extension ' + tab.extId + ': notebookbar tab "' + tab.name +
						'" sets both insertBefore and insertAfter; insertBefore wins');
				}
			}
			var idx = arr.length;
			if (tab.insertBefore) {
				var before = arr.findIndex(function(t) { return t && t.name === tab.insertBefore; });
				if (before >= 0) {
					idx = before;
				} else if (validate) {
					console.warn(
						'extension ' + tab.extId + ': notebookbar tab "' + tab.name +
						'": insertBefore names unknown tab "' + tab.insertBefore + '"');
				}
			} else if (tab.insertAfter) {
				var after = arr.findIndex(function(t) { return t && t.name === tab.insertAfter; });
				if (after >= 0) {
					idx = after + 1;
				} else if (validate) {
					console.warn(
						'extension ' + tab.extId + ': notebookbar tab "' + tab.name +
						'": insertAfter names unknown tab "' + tab.insertAfter + '"');
				}
			}
			arr.splice(idx, 0, buildEntry(tab));
		});
		return arr;
	},

	// Rebuild the notebookbar from a fresh tabsJSON.  Used by
	// ServerConnectionService once extensions have been discovered so the
	// Extensions tab picks them up; preserves whichever tab the user has
	// open by passing _lastSelectedTabName back through getFullJSON.
	refresh: function() {
		var selected = this._lastSelectedTabName
			? this._lastSelectedTabName + '-tab-label'
			: this.HOME_TAB_ID;
		this.model.fullUpdate(this.getFullJSON(selected));
		if (this.container) this.loadTab();
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
		this.updateTabsVisibilityForContext(this._lastContexts);
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

	updateTabsVisibilityForContext: function(requestedContexts) {
		// The first entry is the context of the selection itself, the
		// remaining entries are the contexts of the structures that enclose
		// the selection, for example the table around a selected image. The
		// tabs of all listed contexts become visible, but only the first
		// context's tab is switched to.
		var allContexts = Array.isArray(requestedContexts)
			? requestedContexts
			: (requestedContexts ? [requestedContexts] : []);
		var requestedContext = allContexts.length ? allContexts[0] : undefined;
		var tabs = this.getTabs();
		var contextTab = null;
		var defaultTab = null;
		let alreadySelected = null;
		// Currently selected tab name, part of the element's ID.
		let currentlySelectedTabName = null;

		if (requestedContext)
			if (requestedContext.includes('MasterPage'))
				this._isMasterView = true;
			else if (requestedContext.includes('DrawPage') ||
				requestedContext.includes('NotesPage'))
				this._isMasterView = false;

		for (var tab in tabs) {
			var tabElement = $('#' + tabs[tab].name + '-tab-label');
			if (tabElement.hasClass('selected')) {
				currentlySelectedTabName = tabs[tab].name;
			}
			if (tabs[tab].context) {
				var contexts = tabs[tab].context.split('|');
				var tabMatched = false;

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
						tabMatched = true;
					} else if (allContexts.indexOf(contexts[context]) > 0) {
						// The tab belongs to a structure that encloses the
						// selection: keep it reachable without switching to it.
						tabElement.show();
						tabElement.removeClass('hidden');
						tabMatched = true;
					} else if (this._isMasterView && contexts[context] === 'MasterPage') {
						tabElement.show();
						tabElement.removeClass('hidden');
						tabMatched = true;
					} else if (contexts[context] === 'default') {
						tabElement.show();
						tabElement.removeClass('hidden');
						tabMatched = true;

						if (!tabElement.hasClass('selected'))
							defaultTab = tabElement;
					}
				}
				if (!tabMatched) {
					tabElement.addClass('hidden');
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

		// Skip auto-snap when entering or leaving an in-place edit (for example
		// a formula-bar cell edit, or editing the text inside a shape).
		const inPlaceEditTransition =
			this._inPlaceEditingContexts.includes(requestedContext) ||
			(Array.isArray(this._lastContexts) &&
				this._inPlaceEditingContexts.includes(this._lastContexts[0]));

		if (contextTab) {
			// Switch to the tab of the context, unless we currently show the review tab
			// for text documents, where jumping to the next change would possibly
			// switch to the Home or Table tabs, which is not wanted.
			if ((docType !== 'text' || currentlySelectedTabName !== 'Review') &&
				!inPlaceEditTransition) {
				contextTab.click();
			}
			const tabId = contextTab.attr('id');
			this.updateButtonVisibilityForContext(requestedContext, tabId);
			return;
		}

		if (defaultTab) {
			if (!inPlaceEditTransition) {
				defaultTab.click();
			}
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

		const contexts = detail.contexts || (detail.context ? [detail.context] : []);
		const oldContexts = detail.oldContexts || (detail.oldContext ? [detail.oldContext] : []);
		if (contexts.join(' ') === oldContexts.join(' '))
			return;

		if (this.shouldIgnoreContextChange([detail.context, detail.oldContext], detail.appId))
			return;

		this.updateTabsVisibilityForContext(contexts);
		this._lastContexts = contexts;
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
