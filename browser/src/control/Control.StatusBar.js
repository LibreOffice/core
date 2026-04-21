/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * JSDialog.StatusBar - statusbar component
 */

/* global $ app JSDialog _ _UNO  getPermissionModeElements URLPopUpSection */
class StatusBar extends JSDialog.Toolbar {
	constructor(map) {
		super(map, 'Statusbar', 'toolbar-down');

		map.on('doclayerinit', this.onDocLayerInit, this);
		map.on('languagesupdated', this.onLanguagesUpdated, this);
		map.on('commandstatechanged', this.onCommandStateChanged, this);
		app.events.on('contextchange', this.onContextChange.bind(this));
		app.events.on('updatepermission', this.onPermissionChanged.bind(this));
		map.on('updatestatepagenumber', this.onPageChange, this);
		map.on('search', this.onSearch, this);
		map.on('zoomend', this.onZoomEnd, this);
		map.on('initmodificationindicator', this.onInitModificationIndicator, this);
		map.on('updatemodificationindicator', this.onUpdateModificationIndicator, this);
	}

	isSaveIndicatorActive() {
		return window.useStatusbarSaveIndicator;
	}

	localizeStateTableCell(text) {
		var stateArray = text.split(';');
		var stateArrayLength = stateArray.length;
		var localizedText = '';
		for (var i = 0; i < stateArrayLength; i++) {
			var labelValuePair = stateArray[i].split(':');
			localizedText += _(labelValuePair[0].trim()) + ':' + labelValuePair[1];
			if (stateArrayLength > 1 && i < stateArrayLength - 1) {
				localizedText += '; ';
			}
		}
		return localizedText;
	}

	toLocalePattern(pattern, regex, text, sub1, sub2) {
		var matches = new RegExp(regex, 'g').exec(text);
		if (matches) {
			text = pattern.toLocaleString().replace(sub1, parseInt(matches[1].replace(/,/g,'')).toLocaleString(String.locale)).replace(sub2, parseInt(matches[2].replace(/,/g,'')).toLocaleString(String.locale));
		}
		return text;
	}

	_updateToolbarsVisibility(context) {
		var isReadOnly = this.map.isReadOnlyMode();
		if (isReadOnly) {
			this.enableItem('languagestatus', false);
			this.showItem('insertmode-container', false);
			this.showItem('statusselectionmode-container', false);
		} else {
			this.enableItem('languagestatus', true);
		}
		this.updateVisibilityForToolbar(context);
	}

	onContextChange(event) {
		this._updateToolbarsVisibility(event.detail.context);
	}

	callback(objectType, eventType, object, data, builder) {
		if (object.id === 'search-input' || object.id === 'search') {
			// its handled by widget itself
			return;
		} else if (object.id === 'zoom') {
			var selected = this._generateZoomItems().filter((item) => { return item.id === data; });
			if (selected.length)
				this.map.setZoom(selected[0].scale, null, true /* animate? */);
			return;
		} else if (object.id === 'StateTableCellMenu') {
			var clicked = parseInt(data);
			var current = parseInt(app.map['stateChangeHandler'].getItemValue('.uno:StatusBarFunc')) || 0;

			var value;
			if (clicked === 1) {
				// 'None' was clicked — clear everything
				value = 0;
			} else {
				// Toggle the clicked bit
				value = current ^ clicked;
				// Clear the 'None' bit (1) if any function is now active
				if (value & ~1)
					value = value & ~1;
			}

			var command = {
				'StatusBarFunc': {
					type: 'unsigned short',
					value: value
				}
			};

			this.map.sendUnoCommand('.uno:StatusBarFunc', command);
			return;
		} else if (data === 'menuoverflow' || object.id === 'menuoverflow') {
			const targetBtn = document.getElementById('menuoverflow-button') || document.getElementById('menuoverflow');
			this._openConfigMenu(targetBtn);
			return;
		}

		this.builder._defaultCallbackHandler(objectType, eventType, object, data, builder);
	}

	onSearch(e) {
		var searchInput = window.L.DomUtil.get('search-input');
		if (e.count === 0) {
			this.enableItem('searchprev', false);
			this.enableItem('searchnext', false);
			if (window.mode.isSmallScreenDevice()) {
				this.enableItem('cancelsearch', false);
			} else {
				this.showItem('cancelsearch', false);
			}
			window.L.DomUtil.addClass(searchInput, 'search-not-found');
			$('#findthis').addClass('search-not-found');
			app.searchService.resetSelection();
			setTimeout(function () {
				$('#findthis').removeClass('search-not-found');
				window.L.DomUtil.removeClass(searchInput, 'search-not-found');
			}, 800);
		}
	}

	onZoomEnd() {
		var zoomPercent = this.map.getZoomPercent();
		var zoomSelected = 'zoom' + zoomPercent;

		this.builder.updateWidget(this.parentContainer,
			{
				id: 'zoom',
				type: 'menubutton',
				text: '' + zoomPercent,
				selected: zoomSelected,
				menu: this._generateZoomItems(),
				image: false
			});
	}

	onPageChange(e) {
		var state = e.state;
		state = this.toLocalePattern('Page %1 of %2', 'Page (\\d+) of (\\d+)', state, '%1', '%2');
		this.updateHtmlItem('StatePageNumber', state ? state : ' ');
	}

	onShowCommentsChange(e) {
		var state = e.state;
		var statemsg;
		if (state === 'true')
			statemsg = _UNO('.uno:ShowAnnotations') +': ' + _('On');
		else if (state === 'false')
			statemsg = _UNO('.uno:ShowAnnotations') +': ' + _('Off');
		$('#showcomments-container').attr('default-state', state || null);
		this.updateHtmlItem('ShowComments', state ? statemsg : ' ');
	}

	_generateHtmlItem(id, dataPriority, configPeers) {
		var isReadOnlyMode = app.map ? app.map.isReadOnlyMode() : true;
		var canUserWrite = !app.isReadOnly();

		const item = {
			type: 'container',
			id: id + '-container',
			children: [
				{type: 'htmlcontent', id: id, htmlId: id, text: ' ', isReadOnlyMode: isReadOnlyMode, canUserWrite: canUserWrite},
				{type: 'separator', id: id + 'break', orientation: 'vertical'}
			],
			vertical: false,
			visible: false
		}

		if (dataPriority)
			item.dataPriority = dataPriority;

		const configLabel = JSDialog.getStatusbarItemConfigLabel(id);
		if (configLabel) {
			item.configLabel = configLabel;
			item.configId = id;
		}
		if (configPeers)
			item.configPeers = configPeers;

		return item;
	}

	_generateStateMenuEntry(id, text, selection) {
		return {id: id, text: text, selected: !!(selection & parseInt(id))};
	}

	_generateStateTableCellMenuItem(selection, visible) {
		var submenu = [
			this._generateStateMenuEntry('2', _('Average'), selection),
			this._generateStateMenuEntry('8', _('CountA'), selection),
			this._generateStateMenuEntry('4', _('Count'), selection),
			this._generateStateMenuEntry('16', _('Maximum'), selection),
			this._generateStateMenuEntry('32', _('Minimum'), selection),
			this._generateStateMenuEntry('512', _('Sum'), selection),
			this._generateStateMenuEntry('8192', _('Selection count'), selection),
			this._generateStateMenuEntry('1', _('None'), selection)
		];
		var selected = submenu
			.filter((item) => { return item.selected; })
			.map((item) => { return item.text; });
		var text = selected.length ? selected.join('; ') : _('None');
		return {type: 'menubutton', id: 'StateTableCellMenu', text: text, image: false, menu: submenu, visible: visible, dataPriority: 5};
	}

	_generateZoomItems() {
		return [
			{ id: 'zoom20', text: '20', scale: 1},
			{ id: 'zoom25', text: '25', scale: 2},
			{ id: 'zoom30', text: '30', scale: 3},
			{ id: 'zoom35', text: '35', scale: 4},
			{ id: 'zoom40', text: '40', scale: 5},
			{ id: 'zoom50', text: '50', scale: 6},
			{ id: 'zoom60', text: '60', scale: 7},
			{ id: 'zoom70', text: '70', scale: 8},
			{ id: 'zoom85', text: '85', scale: 9},
			{ id: 'zoom100', text: '100', scale: 10},
			{ id: 'zoom120', text: '120', scale: 11},
			{ id: 'zoom150', text: '150', scale: 12},
			{ id: 'zoom170', text: '170', scale: 13},
			{ id: 'zoom200', text: '200', scale: 14},
			{ id: 'zoom235', text: '235', scale: 15},
			{ id: 'zoom280', text: '280', scale: 16},
			{ id: 'zoom335', text: '335', scale: 17},
			{ id: 'zoom400', text: '400', scale: 18},
		];
	}

	getToolItems() {
		return [
			this._generateHtmlItem('statusdocpos'), 					// spreadsheet
			this._generateHtmlItem('rowcolselcount', 1), 					// spreadsheet
			this._generateHtmlItem('statepagenumber'), 					// text
			this._generateHtmlItem('statewordcount', 1), 					// text
			this._generateHtmlItem('insertmode', 5),						// spreadsheet, text
			this._generateHtmlItem('showcomments', 4),					    // text
			this._generateHtmlItem('statusselectionmode', 6),				// text
			this._generateHtmlItem('slidestatus'),						// presentation
			this._generateHtmlItem('pagestatus'),						// drawing
			{type: 'menubutton', id: 'languagestatus:LanguageStatusMenu', dataPriority: 3,
				configLabel: _('Language'), configId: 'languagestatus', configTargetId: 'languagestatus', configPeers: ['languagestatusbreak']},
			{type: 'separator', id: 'languagestatusbreak', orientation: 'vertical', visible: false, dataPriority: 3}, // spreadsheet
			this._generateHtmlItem('statetablecell', 4, ['StateTableCellMenu', 'statetablebreak']),
			this._generateStateTableCellMenuItem(2, false),			// spreadsheet
			{type: 'separator', id: 'statetablebreak', orientation: 'vertical', visible: false, dataPriority: 7}, // spreadsheet
			this._generateHtmlItem('permissionmode'),					// spreadsheet, text, presentation
			{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: '', text: _UNO('.uno:Signature'), visible: false},
			{type: 'spacer',  id: 'permissionspacer'},
			this._generateHtmlItem('documentstatus', 2),					// spreadsheet, text, presentation, drawing
			{type: 'customtoolitem',  id: 'multi-page-view', command: 'multipageview', text: _('Multi Page View'), dataPriority: 10, visible: false,
				configLabel: _('Multi Page View')},		
			{type: 'customtoolitem',  id: 'prevpage', command: 'prev', text: _UNO('.uno:PageUp', 'text'), pressAndHold: true, dataPriority: 9,
				configLabel: _('Previous Page')},
			{type: 'customtoolitem',  id: 'nextpage', command: 'next', text: _UNO('.uno:PageDown', 'text'), pressAndHold: true, dataPriority: 9,
				configLabel: _('Next Page')},
			{type: 'separator', id: 'prevnextbreak', orientation: 'vertical', dataPriority: 9},
			{type: 'toolitem',  id: 'overview', command: '.uno:InsertCanvasSlide', text: _('Overview'), dataPriority: 9, visible: !app.isReadOnly(),
				configLabel: _('Overview'), configPeers: ['overviewbreak']},
			{type: 'separator', id: 'overviewbreak', orientation: 'vertical', dataPriority: 9, visible: !app.isReadOnly()},
		].concat(window.mode.isTablet() ? [] : [
			{type: 'customtoolitem',  id: 'fitwidthzoom', command: 'fitwidthzoom', text: _('Zoom to Fit Page Width'), icon: 'pagewidth.svg', dataPriority: 8, visible: false},
			{type: 'customtoolitem',  id: 'zoomreset', command: 'zoomreset', text: _('Reset zoom'), icon: 'zoomreset.svg', dataPriority: 8},
			{type: 'customtoolitem',  id: 'zoomout', command: 'zoomout', text: _UNO('.uno:ZoomMinus'), icon: 'minus.svg'},
			{type: 'menubutton', id: 'zoom', text: '100', selected: 'zoom100', menu: this._generateZoomItems(), image: false},
			{type: 'customtoolitem',  id: 'zoomin', command: 'zoomin', text: _UNO('.uno:ZoomPlus'), icon: 'plus.svg'},
			{type: 'toolitem', id: 'menuoverflow', command: 'menuoverflow', text: _('Status Bar Menu'), icon: 'statusbarmenu.svg'}
		]);
	}

	create() {
		if (this.parentContainer.firstChild)
			return;

		this.parentContainer.replaceChildren();

		const items = this.getToolItems();
		this.model.fullUpdate({
			id: this.toolbarElementId,
			dialogid: this.toolbarElementId,
			jsontype: 'toolbar',
			type: 'toolbox',
			children: items,
		});

		this.builder.build(this.parentContainer, this.model.getSnapshot().children);
		this.onLanguagesUpdated();
		JSDialog.MakeStatusPriority(this.parentContainer.querySelector('div'), items);
		JSDialog.RefreshScrollables();

		window.L.DomEvent.on(this.parentContainer, 'contextmenu', this.onContextMenu, this);
	}

	_getAnchorAtCursor(e) {
		if (!this._menuPosEl) {
			this._menuPosEl = document.createElement('div');
			this._menuPosEl.style.position = 'fixed';
			this._menuPosEl.style.width = '0';
			this._menuPosEl.style.height = '0';
			this._menuPosEl.style.zIndex = '-1';
			document.body.appendChild(this._menuPosEl);
		}
		this._menuPosEl.style.left = e.clientX + 'px';
		this._menuPosEl.style.top = e.clientY + 'px';
		return this._menuPosEl;
	}

	onContextMenu(e) {
		window.L.DomEvent.preventDefault(e);
		this._openConfigMenu(this._getAnchorAtCursor(e));
	}

	_openConfigMenu(anchor) {
		const items = this.getToolItems();
		const menuEntries = [];
		const visible = {};

		for (let i = 0; i < items.length; i++) {
			const item = items[i];
			if (!item.configLabel)
				continue;

			const configId = item.configId || item.id;
			if (visible[configId])
				continue;
			visible[configId] = true;

			const targetId = item.configTargetId || item.id;
			const el = document.getElementById(targetId)
				|| this.parentContainer.querySelector('[modelid="' + item.id + '"]');

			const isUserHidden = this.isItemUserHidden(item.id);

			// Skip items that are hidden and not relevant for this doc type,
			// but keep user-hidden items visible in the menu so they can be re-enabled.
			if (!el)
				continue;
			if (el.classList.contains('hidden') && !isUserHidden)
				continue;

			menuEntries.push({
				id: configId,
				itemId: item.id,
				targetId: targetId,
				peers: item.configPeers || [],
				type: 'comboboxentry',
				text: item.configLabel,
				checked: !isUserHidden,
				checktype: 'checkmark'
			});
		}

		const callback = (objectType, eventType, object, data) => {
			if (eventType !== 'selected')
				return;
			const pos = parseInt(data);
			const entry = menuEntries[pos];
			if (!entry)
				return;

			const newHidden = !this.isItemUserHidden(entry.itemId);
			this._applyUserHidden(entry.itemId, entry.targetId, entry.peers, newHidden);
			this.map.uiManager.setDocTypePref('StatusbarUserHidden.' + entry.id, newHidden);
		};

		JSDialog.OpenDropdown('statusbar-context-menu', anchor, menuEntries, callback, 'top', false);
	}

	_applyUserHidden(itemId, targetId, peers, hide) {
		this.userHideItem(itemId, hide);
		if (targetId && targetId !== itemId)
			this.userHideItem(targetId, hide);
		if (peers) {
			for (const peer of peers)
				this.userHideItem(peer, hide);
		}
	}

	_restoreUserHiddenState() {
		const items = this.getToolItems();
		const visible = {};
		for (let i = 0; i < items.length; i++) {
			const item = items[i];
			if (!item.configLabel)
				continue;

			const configId = item.configId || item.id;
			if (visible[configId])
				continue;
			visible[configId] = true;

			const hide = this.map.uiManager.getBooleanDocTypePref(
				'StatusbarUserHidden.' + configId, false);
			if (!hide)
				continue;

			this._applyUserHidden(item.id, item.configTargetId, item.configPeers, true);
		}
	}

	initialize() {
		const showStatusbar = this.map.uiManager.getBooleanDocTypePref('ShowStatusbar', true);

		if (showStatusbar)
			this.map.uiManager.showStatusBar();
		else
			this.map.uiManager.hideStatusBar(true);

		const statusbarState = showStatusbar ? 'true' : 'false';
		app.map['stateChangeHandler'].setItemValue('showstatusbar', statusbarState);
		this.map.fire('commandstatechanged', {commandName : 'showstatusbar', state : statusbarState});
	}

	onDocLayerInit() {
		this.initialize();

		switch (this.map.getDocType()) {
		case 'spreadsheet':
			this.showItem('prev', false);
			this.showItem('next', false);
			this.showItem('prevnextbreak', false);
			this.showItem('overview', false);
			this.showItem('overviewbreak', false);

			if (!window.mode.isSmallScreenDevice()) {
				this.showItem('statusdocpos-container', true);
				this.showItem('rowcolselcount-container', true);
				this.showItem('insertmode-container', true);
				this.showItem('statusselectionmode-container', true);
				this.showItem('languagestatus', !app.map.isReadOnlyMode());
				this.showItem('languagestatusbreak', !app.map.isReadOnlyMode());
				this.showItem('statetablecell-container', true);
				this.showItem('StateTableCellMenu', !app.map.isReadOnlyMode());
				this.showItem('statetablebreak', !app.map.isReadOnlyMode());
				this.showItem('permissionmode-container', true);
				this.showItem('documentstatus-container', true);
			}
			break;

		case 'text':
			this.showItem('overview', false);
			this.showItem('overviewbreak', false);

			if (!window.mode.isSmallScreenDevice()) {
				this.showItem('statepagenumber-container', true);
				this.showItem('statewordcount-container', true);
				this.showItem('insertmode-container', true);
				this.showItem('statusselectionmode-container', true);
				this.showItem('languagestatus', !app.map.isReadOnlyMode());
				this.showItem('languagestatusbreak', !app.map.isReadOnlyMode());
				this.showItem('permissionmode-container', true);
				this.showItem('showcomments-container', true);
				this.showItem('documentstatus-container', true);
				this.showItem('fitwidthzoom', true);
				this.showItem('zoomreset', false);

				this.showItem('multi-page-view', true);
			}
			break;

		case 'presentation':
			if (!window.mode.isSmallScreenDevice()) {
				this.showItem('slidestatus-container', true);
				this.showItem('languagestatus', !app.map.isReadOnlyMode());
				this.showItem('languagestatusbreak', !app.map.isReadOnlyMode());
				this.showItem('permissionmode-container', true);
				this.showItem('documentstatus-container', true);
			}
			break;
		case 'drawing':
			if (!window.mode.isSmallScreenDevice()) {
				this.showItem('pagestatus-container', true);
				this.showItem('languagestatus', !app.map.isReadOnlyMode());
				this.showItem('languagestatusbreak', !app.map.isReadOnlyMode());
				this.showItem('permissionmode-container', true);
				this.showItem('documentstatus-container', true);
			}
			break;
		}

		var language = app.map['stateChangeHandler'].getItemValue('.uno:LanguageStatus');
		if (language)
			this.updateLanguageItem(this.extractLanguageFromStatus(language));

		this._updateToolbarsVisibility();
		this._restoreUserHiddenState();
		JSDialog.RefreshScrollables();
	}

	show() {
		this.parentContainer.style.display = '';
		JSDialog.RefreshScrollables();
	}

	hide() {
		this.parentContainer.style.display = 'none';
	}

	updateHtmlItem(id, text, disabled) {
		this.builder.updateWidget(this.parentContainer, {
			id: id,
			type: 'htmlcontent',
			htmlId: id.toLowerCase(),
			text: text,
			enabled: !disabled
		});

		JSDialog.RefreshScrollables();
	}

	updateLanguageItem(language) {
		if (app.map.isReadOnlyMode())
			return;

		this.builder.updateWidget(this.parentContainer,
			{type: 'menubutton', id: 'languagestatus:LanguageStatusMenu', noLabel: false, text: language});
		JSDialog.RefreshScrollables();
	}

	showSigningItem(icon, text) {
		this.builder.updateWidget(this.parentContainer,
			{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: icon, text: text ? text : _UNO('.uno:Signature')});
		JSDialog.RefreshScrollables();
	}

	onPermissionChanged(event) {
		var isReadOnlyMode = event.detail.perm === 'readonly';
		if (isReadOnlyMode) {
			$('#toolbar-down').addClass('readonly');
		} else {
			$('#toolbar-down').removeClass('readonly');
		}

		var canUserWrite = window.ThisIsAMobileApp ? !app.isReadOnly() : this.map['wopi'].UserCanWrite;
		var NotEditDocMode = false;
		if (app.map['stateChangeHandler'].getItemValue('EditDoc') !== undefined) {
			NotEditDocMode = app.map['stateChangeHandler'].getItemValue('EditDoc') === "false"; // can be true, false or disabled
			if (NotEditDocMode) {
				if (window.mode.isCODesktop()) {
					app.map.uiManager.showSnackbar(_('The document is probably locked and has been opened as view-only'));
					// Don't let the user even try to make the document
					// editable, as that will lead to things Online is not
					// prepared to handle.
					const button = document.querySelector('#mobile-edit-button');
					if (button)
						button.style.setProperty('display', 'none');
				}
				else
					app.map.uiManager.showSnackbar(_('To prevent accidental changes, the author has set this file to open as view-only'));
			}
		}

		canUserWrite = canUserWrite && !NotEditDocMode;

		var permissionContainer = document.getElementById('permissionmode-container');
		if (permissionContainer) {
			while (permissionContainer.firstChild)
				permissionContainer.removeChild(permissionContainer.firstChild);
			permissionContainer.appendChild(getPermissionModeElements(isReadOnlyMode, canUserWrite, this.map));
		}

		this.builder.updateWidget(this.parentContainer, {
			id: 'PermissionMode',
			type: 'htmlcontent',
			htmlId: 'permissionmode',
			isReadOnlyMode: isReadOnlyMode,
			canUserWrite: canUserWrite
		});

		JSDialog.RefreshScrollables();

		if (!window.mode.isSmallScreenDevice()) {
			this.showItem('languagestatus', !isReadOnlyMode);
			this.showItem('languagestatusbreak', !isReadOnlyMode);
			if (this.map.getDocType() === 'spreadsheet') {
				this.showItem('StateTableCellMenu', !isReadOnlyMode);
				this.showItem('statetablebreak', !isReadOnlyMode);
			}

			// updateLanguageItem() is a no-op in read-only mode, so the
			// widget may never have received its text.  Populate it now.
			if (!isReadOnlyMode) {
				var language = app.map['stateChangeHandler'].getItemValue('.uno:LanguageStatus');
				if (language)
					this.updateLanguageItem(this.extractLanguageFromStatus(language));
			}
		}
	}

	extractLanguageFromStatus(state) {
		var code = state;
		var language = _(state);
		var split = code.split(';');
		if (split.length > 1)
			language = _(split[0]);
		return language;
	}

	onCommandStateChanged(e) {
		var commandName = e.commandName;
		var state = e.state;

		if (!commandName)
			return;

		if (commandName === '.uno:StatusDocPos') {
			state = this.toLocalePattern('Sheet %1 of %2', 'Sheet (\\d+) of (\\d+)', state, '%1', '%2');
			this.updateHtmlItem('StatusDocPos', state ? state : ' ');
		}
		else if (commandName === '.uno:LanguageStatus') {
			var language = this.extractLanguageFromStatus(state);
			this.updateLanguageItem(language);
		}
		else if (commandName === '.uno:RowColSelCount') {
			state = this.toLocalePattern('$1 rows, $2 columns selected', '(\\d+) rows, (\\d+) columns selected', state, '$1', '$2');
			state = this.toLocalePattern('$1 of $2 records found', '(\\d+) of (\\d+) records found', state, '$1', '$2');
			this.updateHtmlItem('RowColSelCount', state ? state : _('Select multiple cells'), !state);
		}
		else if (commandName === '.uno:InsertMode') {
			this.updateHtmlItem('InsertMode', state ? window.L.Styles.insertMode[state].toLocaleString() : ' ', !state);

			$('#InsertMode').removeClass();
			$('#InsertMode').addClass('jsdialog ui-badge insert-mode-' + state);
			var isDefaultState = state === 'true' || state === '';
			$('#insertmode-container').attr('default-state', isDefaultState || null);

			if ((state === 'false' || !state) && URLPopUpSection.isOpen()) {
				this.map.hyperlinkUnderCursor = null;
				URLPopUpSection.closeURLPopUp();
			}
		}
		else if (commandName === '.uno:StatusSelectionMode' || commandName === '.uno:SelectionMode') {
			$('#statusselectionmode-container').attr('default-state', state === '0' || null);
			this.updateHtmlItem('StatusSelectionMode', state ? window.L.Styles.selectionMode[state].toLocaleString() : _('Selection mode: inactive'), !state);
		}
		else if (commandName == '.uno:StateTableCell') {
			this.updateHtmlItem('StateTableCell', state ? this.localizeStateTableCell(state) : ' ');
		}
		else if (commandName === '.uno:StatusBarFunc') {
			if (app.map.isReadOnlyMode())
				return;

			// Check 'None' even when state is 0
			if (state === '0')
				state = '1';

			try {
				state = parseInt(state);
			} catch (e) { console.error(e); state = 1; }

			this.builder.updateWidget(this.parentContainer, this._generateStateTableCellMenuItem(state, true));
			JSDialog.RefreshScrollables();
		}
		else if (commandName === '.uno:StatePageNumber') {
			this.onPageChange(e);
			return;
		}
		else if (commandName === 'showannotations') {
			this.onShowCommentsChange(e);
			return;
		}
		else if (commandName === '.uno:StateWordCount') {
			state = this.toLocalePattern('%1 words, %2 characters', '([\\d,]+) words, ([\\d,]+) characters', state, '%1', '%2');
			this.updateHtmlItem('StateWordCount', state ? state : ' ');
		}
		else if (commandName === '.uno:PageStatus') {
			if (this.map.getDocType() === 'presentation') {
				state = this.toLocalePattern('Slide %1 of %2', 'Slide (\\d+) of (\\d+)', state, '%1', '%2');
				this.updateHtmlItem('SlideStatus', state ? state : ' ');
			} else {
				state = this.toLocalePattern('Page %1 of %2', 'Slide (\\d+) of (\\d+)', state, '%1', '%2');
				this.updateHtmlItem('PageStatus', state ? state : ' ');
			}
		}
		else if (commandName === '.uno:EditDoc') {
			state = state !== "false";
			$('#permissionmode-container').attr('default-state', this.map.isEditMode() || null);
			this.onPermissionChanged({detail : {
				perm: state && this.map.isEditMode() ? "edit" : "readonly"
			} });
		}
		else if (commandName === '.uno:Signature') {
			// Use the same handler as the 'signaturestatus:' protocol message, which is
			// sent right after document load.
			this.map.onChangeSignStatus(state);
		}
	}

	onLanguagesUpdated() {
		var menuEntries = [];
		var translated, neutral;
		var constLang = '.uno:LanguageStatus?Language:string=';
		var constDefault = 'Default_RESET_LANGUAGES';
		var constNone = 'Default_LANGUAGE_NONE';
		var resetLang = _('Reset to Default Language');
		var noneLang = _('None (Do not check spelling)');
		var languages = app.languages;

		menuEntries.push({id: 'nonelanguage', uno: constLang + constNone, text: noneLang});

		for (var lang in languages) {
			if (languages.length > 10 && app.favouriteLanguages.indexOf(languages[lang].iso) < 0)
				continue;

			translated = languages[lang].translated;
			neutral = languages[lang].neutral;
			var splitNeutral = neutral.split(';');
			menuEntries.push({id: neutral, text: translated, uno: constLang + encodeURIComponent('Default_' + splitNeutral[0])});
		}

		menuEntries.push({id: 'reset', text: resetLang, uno: constLang + constDefault});
		menuEntries.push({id: 'morelanguages', action: 'morelanguages-all', text: _('Set Language for All text')});

		if (this.map.getDocType() === 'text') {
			menuEntries.push({id: 'langpara', action: 'morelanguages-paragraph', text: _('Set Language for Paragraph')});
			menuEntries.push({id: 'langselection', action: 'morelanguages-selection', text:  _('Set Language for Selection')});
		}

		JSDialog.MenuDefinitions.set('LanguageStatusMenu', menuEntries);
	}

    updateDefaultStateAttribute() {
        const docstatcontainer = document.getElementById('documentstatus-container');
        const documentStatus = document.getElementById('DocumentStatus');
        if (documentStatus && (!documentStatus.textContent || documentStatus.textContent.trim() === '')) {
            docstatcontainer.setAttribute('default-state', 'true');
        } else if (docstatcontainer.hasAttribute('default-state')) {
            docstatcontainer.removeAttribute('default-state');
        }
    }

	onInitModificationIndicator(lastmodtime) {
		if (!this.isSaveIndicatorActive())
			return;

		const docstatcontainer = document.getElementById('documentstatus-container');
		if (lastmodtime == null) {
			if (docstatcontainer !== null && docstatcontainer !== undefined) {
				docstatcontainer.classList.add('hidden');
			}
			return;
		}
		docstatcontainer.classList.remove('hidden');
        this.updateDefaultStateAttribute()

		this.map.fire('modificationindicatorinitialized');
	}

	// status can be '', 'SAVING', 'MODIFIED' or 'SAVED'
	onUpdateModificationIndicator(e) {
		if (!this.isSaveIndicatorActive())
			return;

		if (this._lastModstatus !== e.status) {
			this.updateHtmlItem('DocumentStatus', e.status);
			this._lastModStatus = e.status;

            // Update default-state attribute after updating status
            this.updateDefaultStateAttribute()
		}
		if (e.lastSaved !== null && e.lastSaved !== undefined) {
			const lastSaved = document.getElementById('last-saved');
			if (lastSaved !== null && lastSaved !== undefined) {
				lastSaved.textContent = e.lastSaved;
			}
		}
	}
}

JSDialog.StatusBar = function (map) {
	return new StatusBar(map);
};
