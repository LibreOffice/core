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
 * window.L.Control.Zotero - bibliography dialogs implementation
 */

/* global _ Promise app Set */
window.L.Control.Zotero = window.L.Control.extend({
	_cachedURL: [],
	citations: {},
	itemsPerPage: 15,

	getCachedOrFetch: function (url, retry=true) {
		var that = this;
		var cachedData = this._cachedURL[url];
		if (cachedData) {
			return new Promise(function(resolve) {
				resolve(cachedData);
			});
		} else {
			return fetch(url).then(function (response) {
				if (!response.ok && retry)
					return that.getCachedOrFetch(url, false);

				that._cachedURL[url] = response.json();
				return that._cachedURL[url];
			});
		}
	},

	settings: {
		hasBibliography: '0',
		bibliographyStyleHasBeenSet: '0',
		style: '',
		locale: 'en-US',
		bib: {
			uncited: [],
			omitted: [],
			custom: []
		}
	},

	onAdd: function (map) {
		this.map = map;
		this.enable = false;
		this.fetchStyle();
	},

	extractItemKeyFromLink: function(link) {
		return link.substring(link.lastIndexOf('/')+1);
	},

	onFieldValue: function(fields) {

		var resetCitations = true;
		for (var index = 0; index < fields.length; index++) {
			var field = fields[index];
			var fieldString;
			if (this.getFieldType() === 'Field')
				fieldString = field.command;
			else if (this.getFieldType() === 'ReferenceMark')
				fieldString = field.name;
			else if (this.getFieldType() === 'Bookmark')
				fieldString = field.bookmark;

			if (fieldString.startsWith('ZOTERO_BIBL') || fieldString.startsWith('ADDIN ZOTERO_BIBL')) {
				this.setBibCustomProperty(fieldString);
				continue;
			}

			// Reset citations only once but avoid resetting it if we only received bib fields
			if (resetCitations) {
				resetCitations = false;
				this.resetCitation();
			}
			var values = this.getJSONfromCitationString(fieldString);
			if (!values) {
				continue;
			}
			var that = this;
			this.citationCluster[values.citationID] = [];
			var citationString = app.util.trim(values.properties.plainCitation, this.settings.layout.prefix, this.settings.layout.suffix);
			var citations = citationString.split(this.settings.layout.delimiter);
			var itemUriList = [];
			values.citationItems.forEach(function(item, i) {
				//zotero desktop versions do not store keys in cslJSON
				//extract key from the item url
				var itemUri = item.uris[0];
				var citationId = that.extractItemKeyFromLink(itemUri);
				that.citationCluster[values.citationID].push(citationId);
				that.citations[citationId] = app.util.trim(citations[i], that.settings.group.prefix, that.settings.group.suffix);
				that.setCitationNumber(that.citations[citationId]);
				itemUriList.push(itemUri);
			});
			this.showUnsupportedItemWarning(itemUriList);
		}

		if (this.pendingCitationUpdate || (this.previousNumberOfFields && this.previousNumberOfFields !== fields.length)) {
			delete this.pendingCitationUpdate;
			this.updateCitations(true);
		}
		this.previousNumberOfFields = fields.length;
	},

	getCitationKeys: function() {
		return Object.keys(this.citations);
	},

	getCitationKeysForBib: function() {
		var uncitedKeys = [];
		var that = this;
		this.settings.bib.uncited.forEach(function(item) {
			uncitedKeys.push(that.extractItemKeyFromLink(item[0]));
		});

		var omittedKeys = new Set();
		this.settings.bib.omitted.forEach(function(item) {
			omittedKeys.add(that.extractItemKeyFromLink(item[0]));
		});

		var allKeys = Object.keys(this.citations).concat(uncitedKeys);

		allKeys = allKeys.filter(function(key) { return !omittedKeys.has(key); });

		return allKeys;

	},

	askForApiKey: function () {
		this.map.fire('postMessage', {msgId: 'UI_ZoteroKeyMissing'});
		// if empty - integrator supports Zotero and will ask user to provide the key
		// if undefined/null - integrator doesn't have feature yet - show the warning
		if (this.apiKey !== '')
			this.map.uiManager.showSnackbar(_('Zotero API key is not configured'));
	},

	// executed in ServerConnectionService
	updateUserID: function () {
		app.console.debug('Zotero: updateUserID');

		if (this.apiKey === '')
			return;

		var that = this;
		fetch('https://api.zotero.org/keys/' + this.apiKey)
			.then(function (response) { return response.json(); })
			.then(function (data) {
				app.console.debug('Zotero: got user data');
				that.userID = data.userID;
				that.enable = !!that.userID;
				that.updateGroupIdList();
				// we need to reload the UI now
			}, function () { that.map.uiManager.showSnackbar(_('Zotero API key is incorrect')); });
	},

	updateGroupIdList: function() {
		this.groupIdList = new Set();
		var that = this;
		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/groups?v=3&key=' + this.apiKey)
			.then(function (data) {
				for (var i = 0; i < data.length; i++) {
					that.groupIdList.add(data[i].data.id.toString());
				}
			}, function () {});
	},

	showUnsupportedItemWarning: function(uriList) {
		if (this.showUnsupportedWarning === false || !this.userID)
			return;

		for (var i in uriList) {
			var uriObject = new URL(uriList[i]);

			var catagory = uriObject.pathname.startsWith('/groups/') ? '/groups/' : '/users/';

			var id = uriObject.pathname.substring(catagory.length);
			id = id.substring(0, id.indexOf('/'));
			if (!this.groupIdList.has(id) && id !== this.userID.toString()) {
				this.showUnsupportedWarning = false;
				this.map.uiManager.showInfoModal('zoterounreachablewarn', _('Zotero Warning'),
					_('The document contains some citations which may be unreachable through web API. It may cause some problems while editing citations or bibliography.'),
					null, _('Close'), function() {});
				return;
			}
		}
	},

	getTopBarJSON: function () {
		return {
			id: 'ZoteroDialog-search-container',
			type: 'container',
			layoutstyle: 'end',
			children: [
				{
					id: 'zoterorefresh-buttonbox',
					type: 'buttonbox',
					leftaligned: 'true',
					children: [
						{
							type: 'pushbutton',
							id: 'zoterorefresh',
							text: _('Refresh')
						}
					],
					vertical: false,
					layoutstyle: 'end'
				},
				{
					type: 'edit',
					id: 'zoterosearch',
					placeholder: _('Search'),
					text: ''
				}
			]
		};
	},

	getMainControlsJSON: function (showCategories) {
		return {
			id: 'ZoteroDialog-content',
			type: 'container',
			children: [
				{
					id: 'ZoteroDialog-main',
					type: 'container',
					children: [
						(showCategories) ? {
							type: 'treelistbox',
							id: 'zoterocategory',
							enabled: false,
							entries: this.getDefaultCategories()
						} : {},
						{
							type: 'treelistbox',
							id: 'zoterolist',
							enabled: false,
							entries: [ { columns: [ { text: _('Loading...') } ] } ]
						}
					]
				},
				{
					id: 'ZoteroDialog-buttonbox',
					type: 'buttonbox',
					children: [
						{
							id: 'previous',
							type: 'pushbutton',
							text: _('Previous'),
							enabled: false,
							visible: false
						},
						{
							id: 'pagelabel',
							type: 'fixedtext',
							text: '1/1',
							visible: false
						},
						{
							id: 'next',
							type: 'pushbutton',
							text: _('Next'),
							visible: false

						}
					],
					vertical: false,
					layoutstyle: 'start'
				}
			],
			vertical: true
		};
	},

	getOptionsJSON: function () {
		var that = this;
		return {
			id: 'ZoteroDialog-locale-container',
			type: 'container',
			children: [
				{
					type: 'fixedtext',
					id: 'zoterolocale-label',
					text: _('Language:')
				},
				{
					id: 'zoterolocale',
					type: 'combobox',
					entries: Array.from(Object.keys(this.availableLanguages), function(langCode) {return that.availableLanguages[langCode][0];}),
					selectedCount: '1',
					selectedEntries: [
						Object.keys(this.availableLanguages).indexOf(this.settings.locale)
					],
					enabled: false
				},
				{
					type: 'fixedtext',
					id: 'zoterotype-label',
					text: _('Store as:')
				},
				{
					id: 'zoterotype',
					type: 'combobox',
					entries: [
						_('Fields'),
						_('Bookmarks')
					],
					selectedCount: '1',
					selectedEntries: [
						this.getFieldType() === 'Bookmark' || this.settings.wrapper === 'Endnote' ? 1 : 0
					],
					enabled: false
				}
			]
		};
	},

	dialogSetup: function (title, showCategories, showOptions) {
		var data = {
			id: 'ZoteroDialog',
			dialogid: 'ZoteroDialog',
			type: 'dialog',
			text: title,
			title: title,
			jsontype: 'dialog',
			responses: [
				{
					id: 'ok',
					response: 1
				},
				{
					id: 'cancel',
					response: 0
				},
			],
			children: [
				{
					id: 'ZoteroDialog-mainbox',
					type: 'container',
					vertical: true,
					children: [
						this.getTopBarJSON(),
						this.getMainControlsJSON(showCategories),
						(showOptions) ? this.getOptionsJSON() : {},
						{
							id: 'ZoteroDialog-buttonbox',
							type: 'buttonbox',
							children: [
								{
									id: 'cancel',
									type: 'pushbutton',
									text: _('Cancel'),
								},
								{
									id: 'ok',
									type: 'pushbutton',
									text: _('OK'),
									enabled: false,
								}
							],
							vertical: false,
							layoutstyle: 'end'
						}
					]
				},
			],
		};

		this.items = []; // zoterolist content
		this.categories = this.getDefaultCategories(); // zoterocategory content

		this.groups = [];
		this.collections = [];

		var dialogBuildEvent = {
			data: data,
			callback: this._onAction.bind(this),
		};


		this.map.fire(window.mode.isSmallScreenDevice() ? 'mobilewizard' : 'jsdialog', dialogBuildEvent);

		return this;
	},

	updateControl: function (controlJSON) {
		this.map.fire('jsdialogupdate', {
			data: {
				jsontype: 'dialog',
				action: 'update',
				id: 'ZoteroDialog',
				control: controlJSON,
			},
			callback: this._onAction.bind(this)
		});
	},

	enableDialogOKButton: function() {
		this.updateControl(
			{
				id: 'ok',
				type: 'pushbutton',
				text: _('OK'),
				'has_default': true
			}
		);
	},

	enableDialogLanguageCombobox: function(locale) {
		var that = this;
		this.updateControl(
			{
					id: 'zoterolocale',
					type: 'combobox',
					entries: Array.from(Object.keys(this.availableLanguages), function(langCode) {return that.availableLanguages[langCode][0];}),
					selectedCount: '1',
					selectedEntries: [
						Object.keys(this.availableLanguages).indexOf(locale)
					],
				});
	},

	enableDialogFieldTypeCombobox: function(citationFormat) {
		var entries = [];
		if (citationFormat === 'note') {
			entries = [_('Footnotes'), _('Endnotes')];
		} else {
			entries = [_('Fields'), _('Bookmarks')];
		}
		this.updateControl({
					id: 'zoterotype',
					type: 'combobox',
					entries: entries,
					selectedCount: '1',
					selectedEntries: [
						this.getFieldType() === 'Bookmark' || this.settings.wrapper === 'Endnote' ? 1 : 0
					],
					enabled: !(this.citations && Object.keys(this.citations).length)
				});
	},

	enableDisableNextPrevious: function(hide) {
		this.updateControl({
			id: 'previous',
			type: 'pushbutton',
			text: _('Previous'),
			enabled: this._currentPage > 0,
			visible: hide ? false : true
		});

		this.updateControl({
			id: 'pagelabel',
			type: 'fixedtext',
			text: (this._currentPage + 1) + '/' + this.totalPages,
			visible: hide ? false : true
		});

		this.updateControl({
			id: 'next',
			type: 'pushbutton',
			text: _('Next'),
			enabled: this._currentPage < this.totalPages - 1,
			visible: hide ? false : true
		});
	},

	updateList: function(headerArray, failText) {
		if (this.items.length !== 0) {
			return {
				data: {
					jsontype: 'dialog',
					action: 'update',
					id: 'ZoteroDialog',
					control: {
						id: 'zoterolist',
						type: 'treelistbox',
						headers: headerArray.map(
							function(item) { return { text: item }; }
						),
						entries: this.items,
						sortLocally: true,
					},
				},
				callback: this._onAction.bind(this)
			};
		} else {
			return {
				data: {
					jsontype: 'dialog',
					action: 'update',
					id: 'ZoteroDialog',
					control: {
						id: 'zoterolist',
						type: 'treelistbox',
						entries: [ { columns: [ { text: failText } ] } ]
					},
				},
			};
		}
	},

	getZoteroItemQuery: function(limit, start) {
		var result = '?v=3&key=' + this.apiKey + '&include=data,citation,csljson&style=' + this.settings.style + '&locale=' + this.settings.locale;

		if (limit)
			result += '&limit=' + limit;
		if (start)
			result += '&start=' + start;
		return result;
	},

	_getDefaultSubCollections: function () {
		return [
			{ columns: [ { text: _('My Publications') } ], row: 'https://api.zotero.org/users/' + this.userID + '/publications/items/top' + this.getZoteroItemQuery() },
		];
	},

	getDefaultCategories: function () {
		return [
			{ columns: [{ text: _('My Library') } ], row: 'https://api.zotero.org/users/' + this.userID + '/items/top' + this.getZoteroItemQuery(this.itemsPerPage, 0), children: this._getDefaultSubCollections() },
			{ columns: [{ text: _('Group Libraries')}] }];
	},

	updateCategories: function() {
		this.updateControl({
			id: 'zoterocategory',
			type: 'treelistbox',
			entries: this.categories,
		});
	},

	resetCitation: function() {
		this.citationCluster = {};
		this.citations = {};
		delete this.settings.citationNumber;
		if (this.getFieldType() === 'Bookmark')
			this.bookmarksOrder = [];
	},

	setCitationNumber: function(number) {
		if (this.settings.citationFormat !== 'numeric')
			return;

		if (!this.settings.citationNumber)
			this.settings.citationNumber = 1;

		this.settings.citationNumber = this.settings.citationNumber <= parseInt(number) ? parseInt(number) + 1 : this.settings.citationNumber;

	},

	getCitationText: function(citationId, text) {
		if (this.citations && this.citations[citationId])
			return this.citations[citationId];

		if (this.settings.citationFormat === 'numeric') {
			if (!this.settings.citationNumber)
				this.settings.citationNumber = 1;
			this.citations[citationId] = this.settings.citationNumber++;
		} else {
			this.citations[citationId] = app.util.trim(text, this.settings.layout.prefix, this.settings.layout.suffix);
		}

		return this.settings.group.prefix + this.citations[citationId] + this.settings.group.suffix;
	},

	handleCitationText: function(citationItems) {
		//Some citations have style so trying to change just inner most text without affecting other html
		var citationString = '';
		var citationNode = new DOMParser().parseFromString(citationItems[0].citation, 'text/html').body;

		var that = this;
		citationItems.forEach(function(item) {
			var itemHTML = new DOMParser().parseFromString(item.citation, 'text/html').body;
			// last-child works here only because its a single chain of nodes
			var innerMostNode = itemHTML.querySelector('*:last-child');

			var keys = item['key'] ? [item['key']]
				: Object.keys(item['citationItems']).map(function(citationitem) {return that.extractItemKeyFromLink(item['citationItems'][citationitem].id);});
			keys.forEach(function(key) {
				citationString += innerMostNode ? that.getCitationText(key, innerMostNode.textContent)
					: that.getCitationText(key, item.citation.toString());
				citationString += that.settings.layout.delimiter;
			});
		});

		citationString = this.settings.layout.prefix + app.util.trimEnd(citationString, this.settings.layout.delimiter) + this.settings.layout.suffix;
		// last-child works here only because its a single chain of nodes
		var innerText = citationNode.querySelector('*:last-child');
		if (!innerText)
			citationNode.textContent = citationString;
		else
			innerText.textContent = citationString;
		return citationNode;
	},

	getJSONfromCitationString: function(string) {
		if (!string)
			return {};

		var firstBraceIndex = string.indexOf('{');
		var lastBraceIndex = string.lastIndexOf('}');

		if (firstBraceIndex < 0 || lastBraceIndex > string.length || lastBraceIndex < 0)
			return {};

		var values = JSON.parse(string.substring(firstBraceIndex, lastBraceIndex+1));
		if (values['properties']['formattedCitation'])
			values.citation = values['properties']['formattedCitation']; // comes in handy to avoid multiple property checking latter
		return values;
	},

	getCitationJSONString: function(items) {
		var resultJSON = {};
		resultJSON['citationID'] = app.util.randomString(10);

		var citationNode = this.handleCitationText(items);

		var properties = {
			formattedCitation: citationNode.innerHTML,
			plainCitation: citationNode.textContent,
			noteIndex: '0'
		};

		resultJSON['properties'] = properties;

		resultJSON['citationItems'] = [];

		items.forEach(function(item) {
			if (item['citationItems']) {
				resultJSON['citationItems'] = resultJSON['citationItems'].concat(item['citationItems']);
				return;
			}
			var citationItems = {
				id: item['csljson'].id,
				uris: Object.keys(item['links']).map(function(link) {return item['links'][link]['href'];}),
				itemData: item['csljson']
			};
			resultJSON['citationItems'].push(citationItems);
		});


		resultJSON['schema'] = 'https://raw.githubusercontent.com/citation-style-language/schema/master/schemas/input/csl-citation.json';

		return {jsonString: JSON.stringify(resultJSON),
			citationString: properties.formattedCitation};
	},

	fillCategories: function () {
		this.categories = this.getDefaultCategories().slice();
		this.categories[0].children = this.categories[0].children.concat(this.collections.slice());
		this.categories[1].children = this.groups.slice();
	},

	// columns: Array of details which will be displayed in the dialog
	// entryData: Object containing extra details related to the entry
	createEntry: function (index, columns, entryData, hasIcon) {
		if (hasIcon) {
			var type = entryData.itemType;
			type = type.toUpperCase()[0] + type.substr(1);
			var icon = 'zotero' + type;
			var firstColumn = [ { collapsed: icon, expanded: icon } ];
		} else {
			firstColumn = [];
		}
		this.items.push(Object.assign({ 'columns': firstColumn.concat(columns.map(
			function (item) {
				return { text: item };
			}
		)), row: index, state: (this.dialogType === 'itemlist') ? false : undefined,
		}, entryData));
	},

	fillItems: function (items) {
		var index = 0;
		for (var iterator = 0; iterator < items.length; ++iterator) {
			if (items[iterator].data.itemType === 'note')
				continue;

			var creatorArray = [];
			for (var creator = 0; items[iterator].data.creators && creator < items[iterator].data.creators.length; ++creator) {
				creatorArray.push(items[iterator].data.creators[creator].name ?
					items[iterator].data.creators[creator].name:
					items[iterator].data.creators[creator].firstName + ' ' + items[iterator].data.creators[creator].lastName);
			}
			var creatorString = creatorArray.join(', ');
			this.createEntry(index++,
				[items[iterator].csljson.title, creatorString, items[iterator].data.date ? items[iterator].data.date : items[iterator].data.dateDecided],
				{type: 'item', itemType: items[iterator].data.itemType, item: items[iterator]},
				true
			);
		}
	},

	fillStyles: function (styles) {
		var styleToSelect = this.settings.style;
		if (this.settings.style === '')
			styleToSelect = window.prefs.get('Zotero_LastUsedStyle', '');

		for (var iterator = 0; iterator < styles.length; ++iterator) {
			this.createEntry(iterator, [styles[iterator].title],
				Object.assign({name: styles[iterator].name, type: 'style'},
					styleToSelect === styles[iterator].name ? {selected: true} : null));
		}
	},

	fillNotes: function (items, notesCount) {
		var index = notesCount;
		for (var iterator = 0; iterator < items.length; ++iterator) {
			if (items[iterator].data.itemType !== 'note')
				continue;

			var dummyNode = window.L.DomUtil.create('div');
			dummyNode.innerHTML = items[iterator].data.note;
			var note = dummyNode.innerText.replaceAll('\n', ' ');

			this.createEntry(index++,
				[note],
				{type: 'note', itemType: items[iterator].data.itemType, item: items[iterator]},
				true
			);
		}
		return index;
	},

	showItemList: function () {
		if (!this.enable) {
			this.askForApiKey();
			return;
		}

		if (!this.settings.style) {
			this.pendingAction = this.showItemList;
			this.showStyleList();
			return;
		}
		var that = this;
		this.dialogType = 'itemlist';

		that.dialogSetup(_('My Library'), true);
		that.updateCategories();

		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/groups?v=3&key=' + this.apiKey)
			.then(function (data) {
				for (var i = 0; i < data.length; i++) {
					that.groups.push(
						{
							columns: [ { text: data[i].data.name } ],
							id: data[i].data.id,
							row: data[i].links.self.href + '/items/top' + that.getZoteroItemQuery()
						});
					that.fillCategories();
					that.updateCategories();
				}
			}, function () {
				that.map.uiManager.showSnackbar(_('Failed to load groups'));
			});

		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/collections' + this.getZoteroItemQuery())
			.then(function (data) {
				for (var i = 0; i < data.length; i++) {
					that.collections.push(
						{
							columns: [ { text: data[i].data.name } ],
							id: data[i].data.key,
							row: data[i].links.self.href + '/items/top' + that.getZoteroItemQuery(),
							children: [ { text: '<dummy>' } ],
							ondemand: true
						});
					that.fillCategories();
					that.updateCategories();
				}
			}, function () {
				that.map.uiManager.showSnackbar(_('Failed to load collections'));
			});

			this.totalItems = -1;
			this.totalPages = -1;
			this._currentPage = 0;
			fetch('https://api.zotero.org/users/'+ this.userID +'/items/top?v=3&key=' + this.apiKey + '&limit=1') //find total number of entries
			.then(function (response) {
				that.map.uiManager.showProgressBar(_("Loading references"), undefined, undefined, 10000);
				that.showItemsForUrl('https://api.zotero.org/users/' + that.userID + '/items/top' + that.getZoteroItemQuery(that.itemsPerPage, 0), true);

				that.totalItems = parseInt(response.headers.get('Total-Results'));
				that.totalPages = Math.ceil(that.totalItems/that.itemsPerPage);
				that._currentPage = 0;
				that.enableDisableNextPrevious();
				that.asyncItemLoading();
			});
	},

	asyncItemLoading: function() {
		var that = this;
		var pagesLoaded = 1;
		for (var i = this.itemsPerPage; i < that.totalItems; i += this.itemsPerPage) {
			that.getCachedOrFetch('https://api.zotero.org/users/' + that.userID + '/items/top' + that.getZoteroItemQuery(that.itemsPerPage, i), true)
			.then(function () {
				++pagesLoaded;
				var loadPct = (pagesLoaded/that.totalPages)*100;
				that.map.uiManager.setSnackbarProgress(loadPct);
				if (pagesLoaded === that.totalPages)
					that.map.uiManager.closeSnackbar();
			});
		}
	},

	showStyleList: function() {
		if (!this.enable) {
			this.askForApiKey();
			return;
		}

		var that = this;
		this.dialogType = 'stylelist';
		this.dialogSetup(_('Citation Style'), false, true);
		this.getCachedOrFetch('https://www.zotero.org/styles-files/styles.json')
			.then(function (data) {
				that.fillStyles(data);

				var dialogUpdateEvent = that.updateList([_('Styles')],_('An error occurred while fetching style list'));

				if (window.mode.isSmallScreenDevice()) window.mobileDialogId = dialogUpdateEvent.data.id;
				that.map.fire('jsdialogupdate', dialogUpdateEvent);
				var styleToBeSelected = (that.settings.style && that.settings.style !== '') ? that.settings : {name: window.prefs.get('Zotero_LastUsedStyle', '')};
				if (styleToBeSelected !== '')
					that.checkStyleTypeAndEnableOK(styleToBeSelected);
			}, function () {
				that.map.uiManager.showSnackbar(_('Failed to load styles'));
			});
	},

	fetchStyle: function() {
		this.fetchCustomProperty('ZOTERO_PREF_');
	},

	fetchCustomProperty: function(prefix) {
		app.socket.sendMessage('commandvalues command=.uno:SetDocumentProperties?namePrefix=' + prefix);
	},

	setFetchedCitationFormat: function(style) {
		if (!style)
			style = this.settings.style;

		var that = this;
		fetch('https://www.zotero.org/styles/' + style)
			.then(function (response) { return response.text(); })
			.then(function (html) {
				var csl = new DOMParser().parseFromString(html, 'text/xml');
				var categories = csl.getElementsByTagName('category');
				for (var i = 0; i < categories.length; i++) {
					if (categories[i].getAttribute('citation-format')) {
						that.settings.citationFormat = categories[i].getAttribute('citation-format');
						break;
					}
				}

				var citation = csl.getElementsByTagName('citation')[0];

				if (citation) {
					that.setCitationLayout(citation);
					that.updateCitations(true);
					if (that.settings.bibliographyStyleHasBeenSet === '1') {
						that.updateBibList();
						that.handleInsertBibliography();
					}
				} else {
					var link = csl.getElementsByTagName('link');
					for (var i = 0; i < link.length; i++) {
						if (link[i].getAttribute('rel') === 'independent-parent') {
							that.setFetchedCitationFormat(that.extractItemKeyFromLink(link[i].getAttribute('href')));
							break;
						}
					}
				}
			});
	},

	/// returns found languageNames key
	makeLanguageAvailable: function (locale) {
		var ret = null;
		var languages = Object.keys(this.languageNames).filter(
			function (value) { return value.indexOf(locale) >= 0; });
		for (var l in languages) {
			var language = languages[l];
			ret = language;
			this.availableLanguages[language] = this.languageNames[language];
		}
		return ret;
	},

	checkStyleTypeAndEnableOK: function(selectedStyle) {
		var style = selectedStyle.name ? selectedStyle.name : selectedStyle.style;

		if (!style)
			return;

		var that = this;
		fetch('https://www.zotero.org/styles/' + style)
			.then(function (response) { return response.text(); })
			.then(function (html) {
				var csl = new DOMParser().parseFromString(html, 'text/xml');

				that.availableLanguages = {};

				var locales = csl.getElementsByTagName('locale');
				for (var i = 0; i < locales.length; i++) {
					if (locales[i].getAttribute('xml:lang')) {
						var code = locales[i].getAttribute('xml:lang');
						that.makeLanguageAvailable(code);
					}
				}

				var style = csl.getElementsByTagName('style');
				if (style.length) {
					var defaultLocale = style[0].getAttribute('default-locale');
					if (defaultLocale && defaultLocale !== '')
						defaultLocale = that.makeLanguageAvailable(defaultLocale);
				}

				var availableLocale =
					Object.keys(that.availableLanguages).indexOf(that.settings.locale) >= 0;
				that.enableDialogLanguageCombobox(
					availableLocale ? that.settings.locale : defaultLocale);

				var categories = csl.getElementsByTagName('category');
				for (var i = 0; i < categories.length; i++) {
					if (categories[i].getAttribute('citation-format')) {
						selectedStyle.citationFormat = categories[i].getAttribute('citation-format');
						break;
					}
				}
				that.enableDialogFieldTypeCombobox(selectedStyle.citationFormat);
				that.enableDialogOKButton();
			});
	},

	setCitationLayout: function(ciatationNode) {
		var layout = ciatationNode.getElementsByTagName('layout')[0];
		this.settings.layout = {};
		this.settings.layout['prefix'] = layout && layout.getAttribute('prefix') ? layout.getAttribute('prefix') : '';
		this.settings.layout['suffix'] = layout && layout.getAttribute('suffix') ? layout.getAttribute('suffix') : '';
		this.settings.layout['delimiter'] = layout && layout.getAttribute('delimiter') ? layout.getAttribute('delimiter') : '';

		var group = layout.getElementsByTagName('group')[0];
		this.settings.group = {};
		this.settings.group['prefix'] = group && group.getAttribute('prefix') ? group.getAttribute('prefix') : '';
		this.settings.group['suffix'] = group && group.getAttribute('suffix') ? group.getAttribute('suffix') : '';
		this.settings.group['delimiter'] = group && group.getAttribute('delimiter') ? group.getAttribute('delimiter') : '';
	},

	setFetchedStyle: function(valueString) {

		var value = new DOMParser().parseFromString(valueString, 'text/xml');
		var styleNode = value.getElementsByTagName('style')[0];

		this.settings.style = this.extractItemKeyFromLink(styleNode.id);
		var locale = styleNode.getAttribute('locale');
		if (locale)
			this.settings.locale = locale;
		var fieldTypeNode = value.getElementsByName('fieldType');
		if (fieldTypeNode && fieldTypeNode[0])
			this.settings.fieldType = fieldTypeNode[0].getAttribute('value');
		else
			this.settings.fieldType = this.getFieldType();

		var noteTypeNode = value.getElementsByName('noteType');
		if (noteTypeNode && noteTypeNode[0])
			this.settings.wrapper = noteTypeNode[0].getAttribute('value') === '1' ? 'Footnote' : 'Endnote';

		this.settings.hasBibliography = styleNode.getAttribute('hasBibliography');
		this.settings.bibliographyStyleHasBeenSet = styleNode.getAttribute('bibliographyStyleHasBeenSet');

		this.setFetchedCitationFormat();

		window.prefs.set('Zotero_LastUsedStyle', this.settings.style);
	},

	getStyleXml: function() {
		var xmlDoc = new DOMParser().parseFromString('<data></data>', 'text/xml');
		var dataNode = xmlDoc.getElementsByTagName('data')[0];
		dataNode.setAttribute('data-version', '3');

		var sessionNode = xmlDoc.createElement('session');
		sessionNode.setAttribute('id', app.util.randomString(8));

		dataNode.appendChild(sessionNode);

		var styleNode = xmlDoc.createElement('style');
		styleNode.setAttribute('id', 'http://www.zotero.org/styles/' + this.settings.style);
		if (this.selectedCitationLangCode)
			this.settings.locale = this.selectedCitationLangCode;
		styleNode.setAttribute('locale', this.settings.locale);
		styleNode.setAttribute('hasBibliography', this.settings.hasBibliography);
		styleNode.setAttribute('bibliographyStyleHasBeenSet', this.settings.bibliographyStyleHasBeenSet);

		dataNode.appendChild(styleNode);

		var prefsNode = xmlDoc.createElement('prefs');

		var prefFieldNode = xmlDoc.createElement('pref');
		prefFieldNode.setAttribute('name', 'fieldType');
		prefFieldNode.setAttribute('value', this.getFieldType());

		prefsNode.appendChild(prefFieldNode);

		if (this.settings.wrapper) {
			var prefNotedNode = xmlDoc.createElement('pref');
			prefNotedNode.setAttribute('name', 'noteType');

			if (this.settings.wrapper === 'Footnote')
				prefNotedNode.setAttribute('value', '1');
			else
				prefNotedNode.setAttribute('value', '2');

			prefsNode.appendChild(prefNotedNode);
		}

		dataNode.appendChild(prefsNode);

		return dataNode;
	},

	setStyle: function(style) {
		this.settings.style = style.name;
		this.settings.hasBibliography = '1';

		var dataNode = this.getStyleXml();

		var valueString = dataNode.outerHTML;

		this.setCustomProperty('ZOTERO_PREF_', valueString);
		this.setFetchedCitationFormat();

		window.prefs.set('Zotero_LastUsedStyle', this.settings.style);
	},

	markBibliographyStyleHasBeenSet: function(unset) {
		if (unset)
			this.settings.bibliographyStyleHasBeenSet = '0';
		else
			this.settings.bibliographyStyleHasBeenSet = '1';

		var dataNode = this.getStyleXml();
		var valueString = dataNode.outerHTML;
		this.setCustomProperty('ZOTERO_PREF_', valueString);
	},

	getFieldType: function() {
		if (this.settings.fieldType)
			return this.settings.fieldType;

		var fileExtension = this.map['wopi'].BaseFileName.substring(this.map['wopi'].BaseFileName.lastIndexOf('.') + 1);
		this.settings.fieldType = fileExtension.startsWith('doc') ? 'Field' : 'ReferenceMark';

		//TODO: probably also check for notes?
		return this.settings.fieldType;
	},

	setFieldType: function(field, format) {
		this.settings.wrapper = '';
		if (field === 'Bookmarks')
			this.settings.fieldType = 'Bookmark';
		else {
			var fileExtension = this.map['wopi'].BaseFileName.substring(this.map['wopi'].BaseFileName.lastIndexOf('.') + 1);
			this.settings.fieldType = fileExtension.startsWith('doc') ? 'Field' : 'ReferenceMark';
			if (format === 'note') {
				this.settings.wrapper = 'Footnote';
				if (field === 'Endnotes')
					this.settings.wrapper = 'Endnote';
			}
		}
	},

	_findEntryWithUrlImpl: function(entry, url) {
		if (entry.row === url)
			return entry;

		for (var i in entry.children) {
			var found = this._findEntryWithUrlImpl(entry.children[i], url);
			if (found)
				return found;
		}

		return null;
	},

	_findEntryWithUrl: function(data, url) {
		for (var i in data.entries) {
			var found = this._findEntryWithUrlImpl(data.entries[i], url);
			if (found)
				return found;
		}

		return null;
	},

	_fetchCollectionAndPushTo: function(collectionId, targetCollection) {
		var that = this;
		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/collections/' + collectionId + '/collections/?v=3&key=' + this.apiKey)
			.then(function (data) {
				for (var i = 0; i < data.length; i++) {
					targetCollection.children.push(
						{
							columns: [ { text: data[i].data.name } ],
							id: data[i].data.key,
							row: data[i].links.self.href + '/items/top' + that.getZoteroItemQuery(),
							children: [ { text: '<dummy>' } ],
							ondemand: true
						});
				}

				if (!data.length)
					targetCollection.children = undefined;
				that.fillCategories();
				that.updateCategories();
			}, function () {
				that.map.uiManager.showSnackbar(_('Failed to load collections'));
			});
	},

	_expandCollection: function(treeViewData, row) {
		var entry = this._findEntryWithUrl(treeViewData, row);
		var searchArray = { entries: this.collections };
		var targetEntry = this._findEntryWithUrl(searchArray, row);

		if (entry && targetEntry) {
			if (targetEntry.children && targetEntry.children.length === 1
				&& targetEntry.children[0].text === '<dummy>') {
				targetEntry.children = [];
				targetEntry.ondemand = undefined;
				this.fillCategories();
				this.updateCategories();

				this._fetchCollectionAndPushTo(entry.id, targetEntry);
			}
		}
	},

	showItemsForUrl: function(url, checkState) {
		var that = this;
		that.items = [];
		this.getCachedOrFetch(url)
			.then(function (data) {
				that.fillItems(data);
				var dialogUpdateEvent = that.updateList([_('Title'), _('Creator(s)'), _('Date')], _('Your library is empty'));
				that.map.fire('jsdialogupdate', dialogUpdateEvent);
				if (window.mode.isSmallScreenDevice()) window.mobileDialogId = dialogUpdateEvent.data.id;

				// mark already existing citations
				if (checkState) {
					that.pendingCitationUpdate = false;
					that.pendingItemListing = true;
					that.fetchCitationUnderCursor();
				}
				that.enableDisableNextPrevious(!url.includes('limit='));
			}, function () {
				that.map.uiManager.showSnackbar(_('Failed to load items'));
			});
	},

	_onAction: function(element, action, data, index) {
		if (element === 'dialog' && action === 'close') return;
		if (element === 'treeview') {
			if (action == 'keydown')
				return;
			if (data.id === 'zoterocategory') {
				var url = index;
				if (!url)
					return;

				if (action === 'expand')
					this._expandCollection(data, index);

				this.showItemsForUrl(url);
				return;
			} else {
				if (this.dialogType === 'itemlist') {
					if (action !== 'change')
						return;

					this.selected = data.entries.filter(function (e) { return e.state === true; });
					this.enableDialogOKButton();
				} else {
					this.selected = data.entries[parseInt(index)];

					if (this.dialogType === 'stylelist') {
						this.checkStyleTypeAndEnableOK(this.selected);
					} else if (this.dialogType === 'insertnote') {
						this.enableDialogOKButton();
					}
				}

				return;
			}
		}
		if (element === 'edit' && data.id === 'zoterosearch') {
			const query = data.querySelector('input').value;
			if (query)
				document.getElementById('zoterolist').filterEntries(query);
			return;
		}
		if (data.id == 'ok') {
			// set selected style, style format and field
			if (!this.selected || this.selected.type === 'style') {
				var citationFormat = this.selected ? this.selected.citationFormat : this.settings.citationFormat;
				var parameters = this.selected ? this.selected : {name: window.prefs.get('Zotero_LastUsedStyle', ''), type: 'style'};
				var selectedFieldType = this.selectedFieldType;
				this.closeZoteroDialog();
				this.map.uiManager.showConfirmModal('zoterofieldtypewarn', _('Citation warning'),
					_('Once citations are entered their storage and display type can not be changed.'),
					 _('Confirm'), function() {
						this.setFieldType(selectedFieldType, citationFormat);
						this._onOk(parameters);
						this.dispatchPendingAction();
					 }.bind(this));
				return;
			}

			this._onOk(this.selected);
			this.dispatchPendingAction();
		}
		if (element === 'pushbutton' && data.id === 'zoterorefresh') {
			this._cachedURL = [];
			if (this.dialogType === 'itemlist')
				this.showItemList();
			else if (this.dialogType === 'insertnote')
				this.handleInsertNote();
			else
				this.showStyleList();
			return;
		}
		if (element === 'combobox') {
			if (data.id === 'zoterolocale') {
				this.selectedCitationLangCode = Object.keys(this.availableLanguages)[parseInt(index)];
				if (this.settings.style)
					this.enableDialogOKButton();
			} else if (data.id === 'zoterotype') {
				this.selectedFieldType = index.substring(index.indexOf(';')+1);
			}
			return;
		}
		if (data.id === 'next') {
			this.showItemsForUrl('https://api.zotero.org/users/' + this.userID + '/items/top' + this.getZoteroItemQuery(this.itemsPerPage, (this._currentPage + 1)*this.itemsPerPage), true);
			this._currentPage++;
			this.enableDisableNextPrevious();
			return;
		}
		if (data.id === 'previous') {
			this.showItemsForUrl('https://api.zotero.org/users/' + this.userID + '/items/top' + this.getZoteroItemQuery(this.itemsPerPage, (this._currentPage - 1)*this.itemsPerPage), true);
			this._currentPage--;
			this.enableDisableNextPrevious();
			return;
		}

		this.closeZoteroDialog();
		this._currentPage = 0;
	},

	dispatchPendingAction: function() {
		if (this.pendingAction) {
			this.pendingAction();
			delete this.pendingAction;
		}
	},

	closeZoteroDialog: function() {
		var closeEvent = {
			data: {
				action: 'close',
				id: 'ZoteroDialog',
			}
		};
		this.map.fire(window.mode.isSmallScreenDevice() ? 'closemobilewizard' : 'jsdialog', closeEvent);

		// clear all previous selections
		delete this.selectedCitationLangCode;
		delete this.selected;
		delete this.selectedFieldType;
	},

	_onOk: function (selected) {
		if (this.dialogType === 'itemlist') {
			var citationsToInsert = [];

			for (var i in selected) {
			    citationsToInsert = citationsToInsert.concat([selected[i].item]);
			}

			this.insertCitation(citationsToInsert);

			// update all the citations once citations are inserted and we get updated fields
			this.pendingCitationUpdate = true;
		}
		else if (selected.type === 'style') {
			this.setStyle(selected);
		}
		else if (selected.type === 'note') {
			if (this.map._clip) {
				this.map._clip.dataTransferToDocumentFallback(null, selected.item.data.note);
			}
			else
				console.warn('zotero: cannot paste a note');
		}
	},

	handleItemList: function() {
		this.showItemList();
	},

	handleStyleList: function() {
		this.showStyleList();
	},

	handleInsertNote: function() {
		if (!this.enable) {
			this.askForApiKey();
			return;
		}

		if (!this.settings.style) {
			this.pendingAction = this.handleInsertNote;
			this.showStyleList();
			return;
		}

		var that = this;
		this.dialogType = 'insertnote';
		var notesCount = 0;

		this.dialogSetup(_('Add Note'), false);

		var updateDialog = function () {
			var dialogUpdateEvent = that.updateList([_('Notes')],_('An error occurred while fetching notes'));
			if (window.mode.isSmallScreenDevice()) window.mobileDialogId = dialogUpdateEvent.data.id;
			that.map.fire('jsdialogupdate', dialogUpdateEvent);
		};

		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/items' + this.getZoteroItemQuery())
			.then(function (data) {
				notesCount += that.fillNotes(data, notesCount);
				updateDialog();

				that.getCachedOrFetch('https://api.zotero.org/users/' + that.userID + '/groups?v=3&key=' + that.apiKey)
					.then(function (data) {
						for (var i = 0; i < data.length; i++) {
							var groupUrl = data[i].links.self.href + '/items' + that.getZoteroItemQuery();
							that.getCachedOrFetch(groupUrl)
								.then(function (data) {
									notesCount += that.fillNotes(data, notesCount);
									updateDialog();
								});
						}
					});
			});
	},

	insertBibliography: function() {
		if (!this.enable) {
			this.askForApiKey();
			return;
		}

		this.handleInsertBibliography();
	},

	handleInsertBibliography: function() {
		if (!this.settings.style) {
			this.pendingAction = this.handleInsertBibliography;
			this.showStyleList();
			return;
		}

		if (!(this.citations && Object.keys(this.citations)))
			return;

		if (this.getCitationKeys().length === 0)
			return;

		var that = this;

		if (this.settings.citationFormat === 'numeric') {
			this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/items?include=bib&itemKey=' + this.getCitationKeysForBib().join(',') + '&v=3&key=' + this.apiKey + '&style=' + this.settings.style + '&locale=' + this.settings.locale)
				.then(function(data) {
					var bibList = data.reduce(function(map, item) {
						map[item.key] = item.bib;
						return map;
					}, {});

					var html = '';
					that.getCitationKeysForBib().forEach(function(key) {
						var bib = new DOMParser().parseFromString(bibList[key], 'text/html').body;
						var numberNode = bib.getElementsByClassName('csl-entry')[0].firstElementChild;
						var number = parseInt(numberNode.textContent.substring(numberNode.textContent.search(/[0-9]/))).toString();
						numberNode.textContent = numberNode.textContent.replace(number, that.citations[key]);
						html += bib.innerHTML;
					});

					that.sendInsertBibCommand(html);
					that.markBibliographyStyleHasBeenSet(); // update the document meta data about bib being set

				});
			return;
		}

		fetch('https://api.zotero.org/users/' + this.userID + '/items?format=bib&itemKey=' + this.getCitationKeys().join(',') + '&v=3&key=' + this.apiKey + '&style=' + this.settings.style + '&locale=' + this.settings.locale)
			.then(function (response) { return response.text(); })
			.then(function (html) {
				that.sendInsertBibCommand(html);
				that.markBibliographyStyleHasBeenSet(); // update the document meta data about bib being set
			});
	},

	updateFieldsList: function() {
		if (this.getFieldType() === 'Field')
			app.socket.sendMessage('commandvalues command=.uno:TextFormFields?type=vnd.oasis.opendocument.field.UNHANDLED&commandPrefix=ADDIN%20ZOTERO_ITEM%20CSL_CITATION');
		else if (this.getFieldType() === 'ReferenceMark')
			app.socket.sendMessage('commandvalues command=.uno:Fields?typeName=SetRef&namePrefix=ZOTERO_ITEM%20CSL_CITATION');
		else if (this.getFieldType() === 'Bookmark')
			app.socket.sendMessage('commandvalues command=.uno:Bookmarks?namePrefix=ZOTERO_BREF_');
	},

	updateBibList: function() {
		if (this.getFieldType() === 'Field')
			app.socket.sendMessage('commandvalues command=.uno:TextFormFields?type=vnd.oasis.opendocument.field.UNHANDLED&commandPrefix=ADDIN%20ZOTERO_BIBL');
		else if (this.getFieldType() === 'ReferenceMark')
			app.socket.sendMessage('commandvalues command=.uno:Sections?typeName=SetRef&namePrefix=ZOTERO_BIBL');
		else if (this.getFieldType() === 'Bookmark')
			app.socket.sendMessage('commandvalues command=.uno:SetDocumentProperties?namePrefix=' + this.bibBookmarkName);
	},

	updateCitations: function(showSnackbar) {
		if (!this.citations) {
			this.updateFieldsList();
			return;
		}


		var that = this;
		var citationKeys = this.getCitationKeys();
		var citationCluster = this.citationCluster ? this.citationCluster : [];

		this.getCachedOrFetch('https://api.zotero.org/users/' + this.userID + '/items/top' + this.getZoteroItemQuery() + '&itemKey=' + citationKeys.join(','))
			.then(function (data) {
				that.resetCitation();
				// creating object for faster search
				var dataMap = data.reduce(function(map, item) {
					map[item.key] = item;
					return map;
				}, {});

				var newValues = [];
				Object.keys(citationCluster).forEach(function (clusterKey) {
					var itemList = citationCluster[clusterKey].reduce(function(list, item) {
						list.push(dataMap[item]);
						return list;
					}, []);
					var citationData = that.getCitationJSONString(itemList);
					var updateParameter = that.getFieldParameters(citationData.jsonString, citationData.citationString);
					newValues.push(that.getFieldType() !== 'Bookmark' ? updateParameter : {parameter: updateParameter, cslJSON: citationData.jsonString});
				});

				that.sendUpdateCitationCommand(newValues);
				if (showSnackbar)
					that.map.uiManager.showSnackbar(_('Updated citations'));
			}, function () {
				that.map.uiManager.showSnackbar(_('Citations update failed'));
			});

		if (showSnackbar)
			this.map.uiManager.showSnackbar(_('Updating citations'));
	},

	handleUnlinkCitations: function() {
		var parametes = {};
		var command = '';
		if (this.getFieldType() === 'Field') {
			parametes = {
				'FieldType': {
					'type': 'string',
					'value': 'vnd.oasis.opendocument.field.UNHANDLED'
				},
				'FieldCommandPrefix': {
					'type': 'string',
					'value': 'ADDIN ZOTERO_'
				},
			};
			command = '.uno:DeleteTextFormFields';
		} else if (this.getFieldType() === 'Bookmark') {
			parametes = {
				'BookmarkNamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_BREF_'
				}
			};
			command = '.uno:DeleteBookmarks';
			this.setCustomProperty('ZOTERO_BREF_', ''); // deletes stored CSL data from custom properties
		} else if (this.getFieldType() === 'ReferenceMark') {
			parametes = {
				'TypeName': {
					'type': 'string',
					'value': 'SetRef'
				},
				'NamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_ITEM CSL_CITATION'
				}
			};
			command = '.uno:DeleteFields';

			var sectionUnlinkParameter = {
				'SectionNamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_BIBL'
				}
			};
			this.map.sendUnoCommand('.uno:DeleteSections', sectionUnlinkParameter, true);
		}
		this.map.sendUnoCommand(command, parametes, true);
		this.resetCitation();
		this.map.uiManager.showSnackbar(_('Unlinked citations'));
	},

	unlinkCitations: function() {
		this.map.uiManager.showInfoModal('zoterounlinkwarn', _('Unlink Citations'),
			_('Unlinking citations will prevent Collabora Online from updating citations and bibliography in this document.'),
			null, _('Confirm'), this.handleUnlinkCitations.bind(this));
	},

	handleRefreshCitationsAndBib: function(showSnackbar) {
		//discard the cached url and fetch fresh one
		this._cachedURL = [];
		this.updateCitations(showSnackbar);
		if (this.settings.bibliographyStyleHasBeenSet === '1')
			this.handleInsertBibliography();
	},

	refreshCitationsAndBib: function() {
		if (!this.enable) {
			this.askForApiKey();
			return;
		}

		this.handleRefreshCitationsAndBib(true);
	},

	handleInsertCitation: function() {
		//start by fetching existing citation under the cursor
		if (!this.fetchCitationUnderCursor())
			this.insertCitation({}); //temp fix until exitsting citations can be fetched for every field type
	},

	fetchCitationUnderCursor: function() {
		if (this.getFieldType() === 'Field')
			app.socket.sendMessage('commandvalues command=.uno:TextFormField?type=vnd.oasis.opendocument.field.UNHANDLED&commandPrefix=ADDIN%20ZOTERO_ITEM');
		else if (this.getFieldType() === 'ReferenceMark')
			app.socket.sendMessage('commandvalues command=.uno:Field?typeName=SetRef&namePrefix=ZOTERO_ITEM%20CSL_CITATION');
		else if (this.getFieldType() === 'Bookmark')
			app.socket.sendMessage('commandvalues command=.uno:Bookmark?namePrefix=ZOTERO_BREF_');
		else
			return false;

		return true;
	},

	// happens on _onCommandValuesMsg in WriterTileLayer.js
	handleFieldUnderCursor: function (field) {
		if (!this.pendingItemListing)
			return;

		var cslString;
		if (this.getFieldType() === 'Field') {
			cslString = field.command;
		} else if (this.getFieldType() == 'Bookmark') {
			cslString = field.bookmark;
		} else if (this.getFieldType() === 'ReferenceMark') {
			cslString = field.name;
		}
		var existingCitationUnderCursor = this.getJSONfromCitationString(cslString);
		this.insertNewCitation = !Object.keys(existingCitationUnderCursor).length;

		// we need only to update states of items to show in the dialog
		this.pendingItemListing = false;
		for (var i in existingCitationUnderCursor.citationItems) {
			var existingItem = existingCitationUnderCursor.citationItems[i];
			var itemUri = existingItem.uris[0];
			var key = this.extractItemKeyFromLink(itemUri);

			for (var j in this.items) {
				var listItem = this.items[j];
				if (listItem.item.key === key) {
					listItem.state = true;
					break;
				}
			}
		}

		var dialogUpdateEvent = this.updateList([_('Title'), _('Creator(s)'), _('Date')], _('Your library is empty'));
		this.map.fire('jsdialogupdate', dialogUpdateEvent);
	},

	insertCitation: function(citationsToInsert) {
		if (!(citationsToInsert && citationsToInsert.length))
			return;

		var command = '';
		if (this.getFieldType() === 'Field') {
			command = this.insertNewCitation ? '.uno:TextFormField' : '.uno:UpdateTextFormField';
		} else if (this.getFieldType() == 'Bookmark') {
			command = this.insertNewCitation ? '.uno:InsertBookmark' : '.uno:UpdateBookmark';
		} else if (this.getFieldType() === 'ReferenceMark') {
			command = this.insertNewCitation ? '.uno:InsertField': '.uno:UpdateField';
		}

		var citationData = this.getCitationJSONString(citationsToInsert);
		var parameters = this.insertNewCitation ? this.getFieldParameters(citationData.jsonString, citationData.citationString) : this.getFieldUpdateParameters(citationData.jsonString, citationData.citationString);

		if (this.getFieldType() == 'Bookmark') {
			var prefix = this.insertNewCitation ? parameters['Bookmark'].value : parameters['Bookmark']['value']['Bookmark'].value;
			this.setCustomProperty(prefix + '_', 'ZOTERO_ITEM CSL_CITATION ' + citationData.jsonString);
		}

		delete this.insertNewCitation;
		this.map.sendUnoCommand(command, parameters, true);
		this.updateFieldsList();
	},

	sendUpdateCitationCommand: function(newValueArray) {

		var updatedCitations;
		if (this.getFieldType() === 'Field') {
			updatedCitations = {
				'FieldType': {
					'type': 'string',
					'value': 'vnd.oasis.opendocument.field.UNHANDLED'
				},
				'FieldCommandPrefix': {
					'type': 'string',
					'value': 'ADDIN ZOTERO_ITEM CSL_CITATION'
				},
				'Fields': {
					'type': '[][]com.sun.star.beans.PropertyValue',
					'value': newValueArray
				}
			};
			this.map.sendUnoCommand('.uno:TextFormFields', updatedCitations, true);
		} else if (this.getFieldType() === 'ReferenceMark') {
			updatedCitations = {
				'TypeName': {
					'type': 'string',
					'value': 'SetRef'
				},
				'NamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_ITEM CSL_CITATION'
				},
				'Fields': {
					'type': '[][]com.sun.star.beans.PropertyValue',
					'value': newValueArray
				},
				'NeverExpand': {
					'type': 'boolean',
					'value': true
				}
			};
			this.map.sendUnoCommand('.uno:UpdateFields', updatedCitations, true);
		} else if (this.getFieldType() === 'Bookmark') {
			updatedCitations = {
				'BookmarkNamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_BREF_'
				},
				'Bookmarks': {
					'type': '[][]com.sun.star.beans.PropertyValue',
					'value': newValueArray.map(function(value) {return value.parameter;})
				}
			};
			var that = this;
			newValueArray.forEach(function(value) {
				that.setCustomProperty(value.parameter['Bookmark'].value + '_', 'ZOTERO_ITEM CSL_CITATION ' + value.cslJSON);
			});
			this.map.sendUnoCommand('.uno:UpdateBookmarks', updatedCitations, true);
		}

		this.updateFieldsList();
	},

	getFieldParameters: function(cslJSON, citationString) {
		var field = {};

		if (this.getFieldType() === 'Field') {
			field['FieldType'] = {type: 'string', value: 'vnd.oasis.opendocument.field.UNHANDLED'};
			field['FieldCommand'] = {type: 'string', value: 'ADDIN ZOTERO_ITEM CSL_CITATION ' + cslJSON};
			field['FieldResult'] = {type: 'string', value: citationString};
		} else if (this.getFieldType() === 'ReferenceMark') {
			field['TypeName'] = {type: 'string', value: 'SetRef'};
			field['Name'] = {type: 'string', value: 'ZOTERO_ITEM CSL_CITATION ' + cslJSON + ' RND' + app.util.randomString(10)};
			field['Content'] = {type: 'string', value: citationString};
			field['NeverExpand'] = {type: 'boolean', value: true};
		} else if (this.getFieldType() == 'Bookmark') {
			field['Bookmark'] = {type: 'string', value: 'ZOTERO_BREF_' + app.util.randomString(12)};
			field['BookmarkText'] = {type: 'string', value: citationString};
		}

		if (this.settings.wrapper)
			field['Wrapper'] = {type: 'string', value: this.settings.wrapper};

		return field;
	},

	getFieldUpdateParameters: function(cslJSON, citationString) {
		var field = {};

		if (this.getFieldType() === 'Field') {
			field = {
				'FieldType': {
					'type': 'string',
					'value': 'vnd.oasis.opendocument.field.UNHANDLED'
				},
				'FieldCommandPrefix': {
					'type': 'string',
					'value': 'ADDIN ZOTERO_ITEM'
				},
				'Field': {
					'type': '[]com.sun.star.beans.PropertyValue',
					'value': {
						'FieldType': {
							'type': 'string',
							'value': 'vnd.oasis.opendocument.field.UNHANDLED'
						},
						'FieldCommand': {
							'type': 'string',
							'value': 'ADDIN ZOTERO_ITEM CSL_CITATION ' + cslJSON
						},
						'FieldResult': {
							'type': 'string',
							'value': citationString
						}
					}
				}
			};
		} else if (this.getFieldType() === 'ReferenceMark') {
			field = {
				'TypeName': {
					'type': 'string',
					'value': 'SetRef'
				},
				'NamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_ITEM CSL_CITATION'
				},
				'Field': {
					'type': '[]com.sun.star.beans.PropertyValue',
					'value': {
						'Name': {
							'type': 'string',
							'value': 'ZOTERO_ITEM CSL_CITATION ' + cslJSON
						},
						'Content': {
							'type': 'string',
							'value': citationString
						}
					}
				}
			};
		} else if (this.getFieldType() === 'Bookmark') {
			field = {
				'BookmarkNamePrefix': {
					'type': 'string',
					'value': 'ZOTERO_BREF_'
				},
				'Bookmark': {
					'type': '[]com.sun.star.beans.PropertyValue',
					'value': {
						'Bookmark': {
							'type': 'string',
							'value': 'ZOTERO_BREF_' + app.util.randomString(12)
						},
						'BookmarkText': {
							'type': 'string',
							'value': citationString
						}
					}
				}
			};
		}
		return field;
	},

	getBibParameters: function(html) {
		var field = {};
		// TODO: support uncited omitted (citation) and custom sources in bibliography
		if (this.getFieldType() === 'Field') {
			field['FieldType'] = {type: 'string', value: 'vnd.oasis.opendocument.field.UNHANDLED'};
			field['FieldCommand'] = {type: 'string', value: 'ADDIN ZOTERO_BIBL ' + JSON.stringify(this.settings.bib) + ' CSL_BIBLIOGRAPHY'};
			field['FieldResult'] = {type: 'string', value: html};
		} else if (this.getFieldType() == 'Bookmark') {
			field['Bookmark'] = {type: 'string', value: 'ZOTERO_BREF_' + app.util.randomString(12)};
			field['BookmarkText'] = {type: 'string', value: html};
		} else {
			field['RegionName'] = {type: 'string', value: 'ZOTERO_BIBL ' + JSON.stringify(this.settings.bib) + ' CSL_BIBLIOGRAPHY ' + ' RND' + app.util.randomString(10)};
			field['Content'] = {type: 'string', value: html};
		}

		return field;
	},

	sendInsertBibCommand: function(html) {
		var command = '';
		var parameters = this.getBibParameters(html);
		if (this.getFieldType() === 'Field') {
			command = '.uno:TextFormField';
			if (this.settings.bibliographyStyleHasBeenSet == '1') {
				parameters = {
					'FieldType': {
						'type': 'string',
						'value': 'vnd.oasis.opendocument.field.UNHANDLED'
					},
					'FieldCommandPrefix': {
						'type': 'string',
						'value': 'ADDIN ZOTERO_BIBL'
					},
					'Fields': {
						'type': '[][]com.sun.star.beans.PropertyValue',
						'value': [parameters]
					}
				};
				command = '.uno:TextFormFields';
			}

		} else if (this.getFieldType() == 'Bookmark') {
			command = '.uno:InsertBookmark';
			var newBibBookmarkName = parameters['Bookmark'].value;
			if (this.settings.bibliographyStyleHasBeenSet == '1') {
				command = '.uno:UpdateBookmarks';
				parameters = {
					'BookmarkNamePrefix': {
						'type': 'string',
						'value': this.bibBookmarkName
					},
					'Bookmarks': {
						'type': '[][]com.sun.star.beans.PropertyValue',
						'value': [parameters]
					}
				};
			}
			this.bibBookmarkName = newBibBookmarkName;
			this.setCustomProperty(this.bibBookmarkName + '_', 'ZOTERO_BIBL ' + JSON.stringify(this.settings.bib) + ' CSL_BIBLIOGRAPHY');
		} else {
			command = '.uno:InsertSection';
			if (this.settings.bibliographyStyleHasBeenSet == '1') {
				var command = '.uno:UpdateSections';
				parameters = {
					'SectionNamePrefix': {
						'type': 'string',
						'value': 'ZOTERO_BIBL'
					},
					'Sections': {
						'type': '[][]com.sun.star.beans.PropertyValue',
						'value': [parameters]
					}
				};
			}
		}

		this.map.sendUnoCommand(command, parameters, true);
	},

	setCustomProperty: function(prefix, string) {
		var property =
		{
			'UpdatedProperties': {
				'type': '[]com.sun.star.beans.PropertyValue',
				'value': {
					'NamePrefix': {
						'type': 'string',
						'value': prefix
					},
					'UserDefinedProperties': {
						'type': '[]com.sun.star.beans.PropertyValue',
						'value': {
						}
					}
				}
			}
		};

		// style preference needs to be stored into chunks of max 255 chars
		for (var start = 0, end = 1; (end * 255) < (string.length + 255); start++, end++) {
			property['UpdatedProperties']['value']['UserDefinedProperties']['value'][prefix+end] =
			{
				'type': 'string',
				'value': string.slice(start*255, end*255)
			};

		}
		this.map.sendUnoCommand('.uno:SetDocumentProperties', property, true);
	},

	handleCustomProperty: function(userDefinedProperties) {
		var prefixList = new Set();
		var nameValueMap = {};
		var resultMap = {};
		for (var i = 0; i < userDefinedProperties.length; i++) {
			prefixList.add(userDefinedProperties[i].name.substring(0, userDefinedProperties[i].name.lastIndexOf('_')));
			nameValueMap[userDefinedProperties[i].name] = userDefinedProperties[i].value;
		}

		prefixList.forEach(function(prefix) {
			for (i = 1; nameValueMap[prefix + '_' + i]; i++) {
				if (resultMap[prefix])
					resultMap[prefix] += nameValueMap[prefix + '_' + i];
				else
					resultMap[prefix] = nameValueMap[prefix + '_' + i];
			}
		});

		var resultKeys = Object.keys(resultMap);
		if (resultMap.ZOTERO_PREF) {
			this.setFetchedStyle(resultMap.ZOTERO_PREF);
		}
		// insert new bookmark citation
		else if (!resultKeys.length) {
			this.handleFieldUnderCursor({});
		}
		// update existing bookmark citation
		else if (resultKeys.length && resultKeys[0].startsWith('ZOTERO_BREF')) {
			this.handleFieldUnderCursor({bookmark: resultMap[resultKeys[0]]});

			var fields = [];
			var that = this;
			this.bookmarksOrder.forEach(function(bookmark) {
				if (resultMap[bookmark].startsWith('ZOTERO_BIBL')) {
					that.bibBookmarkName = bookmark;
					that.setBibCustomProperty(resultMap[bookmark]);
					return;
				}
				fields.push({bookmark: resultMap[bookmark]});
			});
			this.onFieldValue(fields);
		}
	},

	setBibCustomProperty: function(bibProperty) {
		if (!bibProperty)
			return {};

		var firstBraceIndex = bibProperty.indexOf('{');
		var lastBraceIndex = bibProperty.lastIndexOf('}');

		if (firstBraceIndex < 0 || lastBraceIndex > bibProperty.length || lastBraceIndex < 0)
			return {};

		var values = JSON.parse(bibProperty.substring(firstBraceIndex, lastBraceIndex+1));

		this.settings.bib.uncited = values.uncited;
		this.settings.bib.omitted = values.omitted;
		this.settings.bib.custom = values.custom;
	},

	handleBookmark: function(bookmarks) {
		this.resetCitation();
		var that = this;
		bookmarks.forEach(function(bookmark) {
			that.bookmarksOrder.push(bookmark.name);
		});
		this.fetchCustomProperty('ZOTERO_BREF_');
	},

	_onMessage: function(message) {
		if (message.startsWith('itemslist: ')) {
			this.handleItemList(message);
		}
	},

	// from https://raw.githubusercontent.com/citation-style-language/locales/master/locales.json
	// saves us from fetching same thing every time for every user
	availableLanguages: {},
	languageNames: {
		'af-ZA': [
			'Afrikaans',
			'Afrikaans'
		],
		'ar': [
			'العربية',
			'Arabic'
		],
		'bg-BG': [
			'Български',
			'Bulgarian'
		],
		'ca-AD': [
			'Català',
			'Catalan'
		],
		'cs-CZ': [
			'Čeština',
			'Czech'
		],
		'cy-GB': [
			'Cymraeg',
			'Welsh'
		],
		'da-DK': [
			'Dansk',
			'Danish'
		],
		'de-AT': [
			'Deutsch (Österreich)',
			'German (Austria)'
		],
		'de-CH': [
			'Deutsch (Schweiz)',
			'German (Switzerland)'
		],
		'de-DE': [
			'Deutsch (Deutschland)',
			'German (Germany)'
		],
		'el-GR': [
			'Ελληνικά',
			'Greek'
		],
		'en-GB': [
			'English (UK)',
			'English (UK)'
		],
		'en-US': [
			'English (US)',
			'English (US)'
		],
		'es-CL': [
			'Español (Chile)',
			'Spanish (Chile)'
		],
		'es-ES': [
			'Español (España)',
			'Spanish (Spain)'
		],
		'es-MX': [
			'Español (México)',
			'Spanish (Mexico)'
		],
		'et-EE': [
			'Eesti keel',
			'Estonian'
		],
		'eu': [
			'Euskara',
			'Basque'
		],
		'fa-IR': [
			'فارسی',
			'Persian'
		],
		'fi-FI': [
			'Suomi',
			'Finnish'
		],
		'fr-CA': [
			'Français (Canada)',
			'French (Canada)'
		],
		'fr-FR': [
			'Français (France)',
			'French (France)'
		],
		'he-IL': [
			'עברית',
			'Hebrew'
		],
		'hi-IN': [
			'हिंदी',
			'Hindi'
		],
		'hr-HR': [
			'Hrvatski',
			'Croatian'
		],
		'hu-HU': [
			'Magyar',
			'Hungarian'
		],
		'id-ID': [
			'Bahasa Indonesia',
			'Indonesian'
		],
		'is-IS': [
			'Íslenska',
			'Icelandic'
		],
		'it-IT': [
			'Italiano',
			'Italian'
		],
		'ja-JP': [
			'日本語',
			'Japanese'
		],
		'km-KH': [
			'ភាសាខ្មែរ',
			'Khmer'
		],
		'ko-KR': [
			'한국어',
			'Korean'
		],
		'la': [
			'Latina',
			'Latin'
		],
		'lt-LT': [
			'Lietuvių kalba',
			'Lithuanian'
		],
		'lv-LV': [
			'Latviešu',
			'Latvian'
		],
		'mn-MN': [
			'Монгол',
			'Mongolian'
		],
		'nb-NO': [
			'Norsk bokmål',
			'Norwegian (Bokmål)'
		],
		'nl-NL': [
			'Nederlands',
			'Dutch'
		],
		'nn-NO': [
			'Norsk nynorsk',
			'Norwegian (Nynorsk)'
		],
		'pl-PL': [
			'Polski',
			'Polish'
		],
		'pt-BR': [
			'Português (Brasil)',
			'Portuguese (Brazil)'
		],
		'pt-PT': [
			'Português (Portugal)',
			'Portuguese (Portugal)'
		],
		'ro-RO': [
			'Română',
			'Romanian'
		],
		'ru-RU': [
			'Русский',
			'Russian'
		],
		'sk-SK': [
			'Slovenčina',
			'Slovak'
		],
		'sl-SI': [
			'Slovenščina',
			'Slovenian'
		],
		'sr-RS': [
			'Српски / Srpski',
			'Serbian'
		],
		'sv-SE': [
			'Svenska',
			'Swedish'
		],
		'th-TH': [
			'ไทย',
			'Thai'
		],
		'tr-TR': [
			'Türkçe',
			'Turkish'
		],
		'uk-UA': [
			'Українська',
			'Ukrainian'
		],
		'vi-VN': [
			'Tiếng Việt',
			'Vietnamese'
		],
		'zh-CN': [
			'中文 (中国大陆)',
			'Chinese (PRC)'
		],
		'zh-TW': [
			'中文 (台灣)',
			'Chinese (Taiwan)'
		]
	}
});

window.L.control.zotero = function (map) {
	return new window.L.Control.Zotero(map);
};
