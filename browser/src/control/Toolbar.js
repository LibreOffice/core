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
 * Toolbar handler
 */

/* global app $ window brandProductName DocUtil GraphicSelection _ */

window.L.Map.include({

	// a mapping of uno commands to more readable toolbar items
	unoToolbarCommands: [
		'.uno:StyleApply',
		'.uno:CharFontName'
	],

	_modalDialogOptions: {
		overlayClose:true,
		opacity: 80,
		overlayCss: {
			backgroundColor : '#000'
		},
		containerCss: {
			overflow : 'hidden',
			backgroundColor : '#fff',
			padding : '20px',
			border : '2px solid #000'
		}
	},

	_getCurrentFontName: function() {
		return this['stateChangeHandler'].getItemValue('.uno:CharFontName');
	},

	createFontSelector: function(containerId) {
		var that = this;
		var container = document.getElementById(containerId);
		if (!container)
			return;

		var populateEntries = function() {
			var commandValues = that.getToolbarCommandValues('.uno:CharFontName');
			if (typeof commandValues === 'object' && container.updateEntries) {
				var fontNames = Object.keys(commandValues);
				container.updateEntries(fontNames);
			}
			var currentFont = that._getCurrentFontName();
			if (currentFont && container.onSetText)
				container.onSetText(currentFont);
		};

		populateEntries();

		var onCommandStateChanged = function(e) {
			if (e.commandName !== '.uno:CharFontName')
				return;

			if (container.onSetText)
				container.onSetText(e.state);
			this['stateChangeHandler'].setItemValue('.uno:CharFontName', e.state);
		};

		var onFontListChanged = function(e) {
			if (e.commandName === '.uno:CharFontName')
				populateEntries();
		};

		this.off('commandstatechanged', onCommandStateChanged);
		this.on('commandstatechanged', onCommandStateChanged);
		this.off('updatetoolbarcommandvalues', onFontListChanged);
		this.on('updatetoolbarcommandvalues', onFontListChanged);
	},

	createFontSizeSelector: function(containerId) {
		var container = document.getElementById(containerId);
		if (!container)
			return;

		var onCommandStateChanged = function(e) {
			if (e.commandName !== '.uno:FontHeight')
				return;

			var state = e.state;
			if (state === '0')
				state = '';

			if (container.onSetText)
				container.onSetText(state);
			this['stateChangeHandler'].setItemValue('.uno:FontHeight', state);
		};

		this.off('commandstatechanged', onCommandStateChanged);
		this.on('commandstatechanged', onCommandStateChanged);

		// Initialize with current state value if available
		var currentState = this['stateChangeHandler'].getItemValue('.uno:FontHeight');
		if (currentState) {
			onCommandStateChanged.call(this, {commandName: '.uno:FontHeight', state: currentState});
		}
	},

	applyFont: function (fontName) {
		if (!fontName)
			return;
		if (this.isEditMode()) {
			var msg = 'uno .uno:CharFontName {' +
				'"CharFontName.FamilyName": ' +
					'{"type": "string", "value": "' + fontName + '"}}';
			app.socket.sendMessage(msg);
		}
	},

	applyFontSize: function (fontSize) {
		if (this.isEditMode()) {
			var msg = 'uno .uno:FontHeight {' +
				'"FontHeight.Height": ' +
				'{"type": "float", "value": "' + fontSize + '"}}';
			app.socket.sendMessage(msg);
		}
	},

	getToolbarCommandValues: function (command) {
		if (this._docLayer) {
			return this._docLayer._toolbarCommandValues[command];
		}

		return undefined;
	},

	downloadAs: function (name, format, options, id) {
		if (this._fatal) {
			return;
		}

		id = id || 'export'; // not any special download, simple export

		if ((id === 'print' && this['wopi'].DisablePrint) ||
			(id === 'export' && this['wopi'].DisableExport)) {
			this.hideBusy();
			return;
		}

		if (format === undefined || format === null) {
			format = '';
		}
		if (options === undefined || options === null) {
			options = '';
		}

		// printing: don't export form fields, irrelevant, and can be buggy
		// comments are irrelevant, too
		if (id === 'print' && format === 'pdf' && options === '')
			options = '{\"ExportFormFields\":{\"type\":\"boolean\",\"value\":\"false\"},' +
						'\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}';

		// download: don't export comments into PDF by default
		if (id == 'export' && format === 'pdf' && options === '')
			options = '{\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}';

		if (!window.ThisIsAMobileApp)
			this.showBusy(_('Downloading...'), false);

		app.socket.sendMessage('downloadas ' +
			'name=' + encodeURIComponent(name) + ' ' +
			'id=' + id + ' ' +
			'format=' + format + ' ' +
			'options=' + options);
	},

	print: function (options) {
		if (window.ThisIsTheiOSApp || window.ThisIsTheAndroidApp || window.ThisIsTheMacOSApp || window.ThisIsTheWindowsApp || window.ThisIsTheQtApp) {
			window.postMobileMessage('PRINT');
		} else {
			this.showBusy(_('Downloading...'), false);
			this.downloadAs('print.pdf', 'pdf', options, 'print');
		}
	},

	saveAs: function (url, format, options) {
		if (url === undefined || url == null) {
			return;
		}
		if (format === undefined || format === null) {
			format = '';
		}
		if (options === undefined || options === null) {
			options = '';
		}

		this.showBusy(_('Saving...'), false);
		app.socket.sendMessage('saveas ' +
			'url=wopi:' + encodeURIComponent(url) + ' ' +
			'format=' + format + ' ' +
			'options=' + options);
	},

	exportAs: function (url) {
		if (url === undefined || url == null) {
			return;
		}

		app.socket.sendMessage('exportas url=wopi:' + encodeURIComponent(url));
	},

	renameFile: function (filename) {
		if (!filename) {
			return;
		}
		this.showBusy(_('Renaming...'), false);
		app.socket.sendMessage('renamefile filename=' + encodeURIComponent(filename));
	},

	applyStyle: function (style, familyName) {
		if (!style || !familyName) {
			this.fire('error', {cmd: 'setStyle', kind: 'incorrectparam'});
			return;
		}
		if (this.isEditMode()) {
			var msg = 'uno .uno:StyleApply {' +
					'"Style":{"type":"string", "value": "' + style + '"},' +
					'"FamilyName":{"type":"string", "value":"' + familyName + '"}' +
					'}';
			app.socket.sendMessage(msg);
		}
	},

	applyLayout: function (layout) {
		if (!layout) {
			this.fire('error', {cmd: 'setLayout', kind: 'incorrectparam'});
			return;
		}
		if (this.isEditMode()) {
			var msg = 'uno .uno:AssignLayout {' +
					'"WhatPage":{"type":"unsigned short", "value": "' + this.getCurrentPartNumber() + '"},' +
					'"WhatLayout":{"type":"unsigned short", "value": "' + layout + '"}' +
					'}';
			app.socket.sendMessage(msg);
		}
	},

	save: function(dontTerminateEdit, dontSaveIfUnmodified, extendedData) {
		this.fire('updatemodificationindicator', { status: 'SAVING' });

		var msg = 'save' +
					' dontTerminateEdit=' + (dontTerminateEdit ? 1 : 0) +
					' dontSaveIfUnmodified=' + (dontSaveIfUnmodified ? 1 : 0);

		if (extendedData !== undefined) {
			msg += ' extendedData=' + extendedData;
		}

		app.socket.sendMessage(msg);
	},

	messageNeedsToBeRedirected: function(command) {
		if (command === '.uno:EditHyperlink') {
			this.sendUnoCommand('.uno:HyperlinkDialog');
			return true;
		}
		else {
			return false;
		}
	},

	sendUnoCommand: function (command, json, force) {
		if (command.indexOf('.uno:') < 0 && command.indexOf('vnd.sun.star.script') < 0)
			console.error('Trying to send uno command without prefix: "' + command + '"');

		if ((command.startsWith('.uno:Sidebar') && !command.startsWith('.uno:SidebarShow')) ||
			command.startsWith('.uno:CustomAnimation') || command.startsWith('.uno:ModifyPage') ||
			command.startsWith('.uno:MasterSlidesPanel') || command.startsWith('.uno:SidebarDeck')) {

			// sidebar control is present only in desktop/tablet case
			if (this.sidebar) {
				if (this.sidebar.isVisible()) {
					this.sidebar.setupTargetDeck(command);
				} else {
					// we don't know which deck was active last, show first then switch if needed
					app.socket.sendMessage('uno .uno:SidebarShow');

					this.sidebar.setupTargetDeck(command);
					return;
				}
			}
		}

		var isAllowedInReadOnly = false;
		var allowedCommands = ['.uno:Save', '.uno:SaveAs', '.uno:WordCountDialog',
			'.uno:Signature', '.uno:PrepareSignature', '.uno:DownloadSignature', '.uno:InsertSignatureLine',
			'.uno:ShowResolvedAnnotations', '.uno:Open', '.uno:CloseWin',
			'.uno:ToolbarMode?Mode:string=notebookbar_online.ui', '.uno:ToolbarMode?Mode:string=Default',
			'.uno:ExportToEPUB', '.uno:ExportToPDF', '.uno:ExportDirectToPDF', '.uno:MoveKeepInsertMode', '.uno:ShowRuler',
			'.uno:Navigator', '.uno:GotoPage'];
		if (app.isCommentEditingAllowed()) {
			allowedCommands.push('.uno:InsertAnnotation','.uno:DeleteCommentThread', '.uno:DeleteAnnotation', '.uno:DeleteNote',
				'.uno:DeleteComment', '.uno:ReplyComment', '.uno:ReplyToAnnotation', '.uno:PromoteComment', '.uno:ResolveComment',
				'.uno:ResolveCommentThread', '.uno:ResolveComment', '.uno:EditAnnotation', '.uno:ExportToEPUB', '.uno:ExportToPDF',
				'.uno:ExportDirectToPDF');

			const graphicInfo = GraphicSelection.extraInfo;
			if (graphicInfo && graphicInfo.isSignature)
			{
				// If the just added signature line shape is selected, allow
				// moving/resizing it.
				allowedCommands.push('.uno:TransformDialog', '.uno:MoveShapeHandle');
			}
		}
		if (app.isRedlineManagementAllowed()) {
			allowedCommands.push('.uno:ShowTrackedChanges', '.uno:AcceptTrackedChanges', '.uno:AcceptTrackedChange', '.uno:RejectTrackedChange',
				'.uno:AcceptAllTrackedChanges', '.uno:RejectAllTrackedChanges', '.uno:AcceptTrackedChangeToNext', '.uno:RejectTrackedChangeToNext',
				'.uno:CommentChangeTracking', '.uno:PreviousTrackedChange', '.uno:NextTrackedChange');
		}

		for (var i in allowedCommands) {
			if (allowedCommands[i] === command) {
				isAllowedInReadOnly = true;
				break;
			}
		}

		var map = this;

		if (command.startsWith('.uno:SpellOnline')) {
			var val = map['stateChangeHandler'].getItemValue('.uno:SpellOnline');

			// proceed if the toggle button is pressed
			if (val && (json === undefined || json === null)) {
				 // because it is toggle, state has to be the opposite
				var state = !(val === 'true');
				window.prefs.set('spellOnline', state);
			}
		}

		if (this.uiManager.isUIBlocked())
			return;
		if ((this.dialog.hasOpenedDialog() || (this.jsdialog && this.jsdialog.hasDialogOpened()))
			&& !command.startsWith('.uno:ToolbarMode') && !force) {
			console.debug('Cannot execute: ' + command + ' when dialog is opened.');
			this.dialog.blinkOpenDialog();
		} else if ((this.isEditMode() || isAllowedInReadOnly) && !this.messageNeedsToBeRedirected(command)) {
			app.socket.sendMessage('uno ' + command + (json ? ' ' + JSON.stringify(json) : ''));
			// user interaction turns off the following of other users
			if (map.userList && map._docLayer && map._docLayer._viewId)
				map.userList.followUser(map._docLayer._viewId, /* do instant scroll */ false);
		}
	},

	toggleCommandState: function (unoState) {
		if (this.isEditMode()) {
			if (!unoState.startsWith('.uno:')) {
				unoState = '.uno:' + unoState;
			}
			this.sendUnoCommand(unoState);
		}
	},

	insertGraphic: function (file) {
		this.fire('insertgraphic', {file: file});
	},

	insertMultimedia: function (file) {
		this.fire('insertmultimedia', {file: file});
	},

	insertURL: function (url, urltype) {
		this.fire('inserturl', {url: url, urltype: urltype});
	},

	selectBackground: function (file) {
		this.fire('selectbackground', {file: file});
	},

	compareDocuments: function (file) {
		this.fire('comparedocuments', {file: file});
	},

	onHelpOpen: function(id, map, productName) {
		var i;
		// Display keyboard shortcut or online help
		if (id === 'keyboard-shortcuts-content') {
			document.getElementById('online-help-content').classList.add('hide');
			// Display help according to document opened
			if (map.getDocType() === 'text') {
				document.getElementById('text-shortcuts').classList.add('show');
			}
			else if (map.getDocType() === 'spreadsheet') {
				document.getElementById('spreadsheet-shortcuts').classList.add('show');
			}
			else if (map.getDocType() === 'presentation') {
				document.getElementById('presentation-shortcuts').classList.add('show');
			}
			else if (map.getDocType() === 'drawing') {
				document.getElementById('drawing-shortcuts').classList.add('show');
			}
		} else /* id === 'online-help' */ {
			document.getElementById('keyboard-shortcuts-content').classList.add('hide');
			if (window.socketProxy) {
				var helpdiv = document.getElementById('online-help-content');
				var imgList = helpdiv.querySelectorAll('img');
				for (var p = 0; p < imgList.length; p++) {
					var imgSrc = imgList[p].src;
					imgSrc = imgSrc.substring(imgSrc.indexOf('/images'));
					imgList[p].src = window.makeWsUrl('/browser/dist'+ imgSrc);
				}
			}
			// Display help according to document opened
			if (map.getDocType() === 'text') {
				var x = document.getElementsByClassName('text');
				for (i = 0; i < x.length; i++) {
					x[i].classList.add('show');
				}
			}
			else if (map.getDocType() === 'spreadsheet') {
				x = document.getElementsByClassName('spreadsheet');
				for (i = 0; i < x.length; i++) {
					x[i].classList.add('show');
				}
			}
			else if (map.getDocType() === 'presentation' || map.getDocType() === 'drawing') {
				x = document.getElementsByClassName('presentation');
				for (i = 0; i < x.length; i++) {
					x[i].classList.add('show');
				}
			}
		}

		var contentElement = document.getElementById(id);

		// Let's translate
		var max;
		var translatableContent = contentElement.querySelectorAll('h1');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}
		translatableContent = contentElement.querySelectorAll('h2');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}
		translatableContent = contentElement.querySelectorAll('h3');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}
		translatableContent = contentElement.querySelectorAll('h4');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}
		translatableContent = contentElement.querySelectorAll('td');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			var orig = translatableContent[i].innerHTML;
			var trans = translatableContent[i].innerHTML.toLocaleHelpString();
			// Try harder to get translation of keyboard shortcuts (html2po trims starting <kbd> and ending </kbd>)
			if (orig === trans && orig.indexOf('kbd') != -1) {
				var trimmedOrig = orig.replace(/^(<kbd>)/,'').replace(/(<\/kbd>$)/,'');
				var trimmedTrans = trimmedOrig.toLocaleHelpString();
				if (trimmedOrig !== trimmedTrans) {
					trans = '<kbd>' + trimmedTrans + '</kbd>';
				}
			}
			translatableContent[i].innerHTML = trans;
		}
		translatableContent = contentElement.querySelectorAll('p');
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}
		translatableContent = contentElement.querySelectorAll('button'); // TOC
		for (i = 0, max = translatableContent.length; i < max; i++) {
			translatableContent[i].innerHTML = translatableContent[i].innerHTML.toLocaleHelpString();
		}

		//translatable screenshots
		var supportedLanguage = ['de', 'fr', 'it', 'es', 'pt-BR'];
		var currentLanguage = String.locale;
		if (supportedLanguage.indexOf(currentLanguage) >= 0) {
			translatableContent = $(contentElement.querySelectorAll('.screenshot img'));

			for (i = 0, max = translatableContent.length; i < max; i++) {
				translatableContent[i].src = translatableContent[i].src.replace('/en/', '/'+currentLanguage+'/');
			}
		}

		// Substitute {productname} in Online Help and replace special Mac key names
		if (id === 'online-help-content') {
			var productNameContent = contentElement.querySelectorAll('span.productname');
			for (i = 0, max = productNameContent.length; i < max; i++) {
				productNameContent[i].innerHTML = productNameContent[i].innerHTML.replace('{productname}', productName);
			}
			document.getElementById('online-help-content').innerHTML = app.util.replaceCtrlAltInMac(document.getElementById('online-help-content').innerHTML);
		}
		if (id === 'keyboard-shortcuts-content') {
			document.getElementById('keyboard-shortcuts-content').innerHTML = app.util.replaceCtrlAltInMac(document.getElementById('keyboard-shortcuts-content').innerHTML);
		}
		var searchInput = document.getElementById('online-help-search-input');
		searchInput.setAttribute('placeholder',_('Search'));
		searchInput.setAttribute('aria-label',_('Search'));
		var helpContentParent = document.getElementsByClassName('ui-dialog-content')[0];
		var startFilter = false;
		var isAnyMatchingContent = false;

		const performSearch = function() {
			// Hide all elements within the #online-help-content on first key stroke/at start of filter content
			if (!startFilter || !isAnyMatchingContent) {
				helpContentParent.style.backgroundColor = 'var(--color-background-dark) !important';
				// Hide all <p> tags within .text, .spreadsheet, or .presentation sections
				document.querySelectorAll('#online-help-content > *:not(a), .link-section p, .product-header').forEach(function (element) {
					// Check if the element has class text, spreadsheet, or presentation
					if (!element.classList.contains('text') && !element.classList.contains('spreadsheet') && !element.classList.contains('presentation')) {
						this.hide(element);
					}
				}.bind(this));

				startFilter = true;
			}
			var searchTerm = searchInput.value.trim();
			// Reset highlighting and visibility if search term is empty
			if (searchTerm === '') {
				this.resetFilterResults();
				startFilter = false;
			}
			else {
				this.filterResults(searchTerm, isAnyMatchingContent, id);
				this._focusContainer(id + '-box');
			}
		}.bind(this);

		searchInput.addEventListener('keydown', function (e) {
			if (e.key === 'Enter') {
				performSearch();
			}
		});

		const searchButton = document.getElementById('online-help-search-button');
		searchButton.setAttribute('aria-label', _('Search'));
		searchButton.addEventListener('click', performSearch);
		searchButton.addEventListener('keydown', function (e) {
			if (e.key === 'Enter') {
				performSearch();
			}
		});

		const onlineHelpContent = document.getElementById('online-help-content');
		const buttons = onlineHelpContent.querySelectorAll('.scroll-button');

		buttons.forEach((button) => {
			button.addEventListener('click', () => {
				const targetId = button.dataset.target;
				if (targetId) {
					const targetElement = document.getElementById(`${targetId}`);
					if (targetElement) {
						targetElement.scrollIntoView();
					}
				}
			});
		});

		this._focusContainer(id + '-box');
	},

	_focusContainer: function(id) {
		app.layoutingService.appendLayoutingTask(() => {
			var contentContainer = document.getElementById(id);
			if (contentContainer) {
				contentContainer.setAttribute('tabindex', '-1');
				contentContainer.focus();
			}
		});
	},

	filterResults: function (searchTerm, isAnyMatchingContent, id) {

		var mainDiv = document.getElementById(id);
		// Combine query parameters to select main sections
		var mainSectionsQuery = '.section:not(div.text .section, div.spreadsheet .section, div.presentation .section)';
		var docType = this.getDocType() === 'drawing' ? 'presentation' : this.getDocType();
		mainSectionsQuery += ', div.' + docType + ' .section';

		// Select main sections elements within the mainDiv
		var mainSections = mainDiv.querySelectorAll(mainSectionsQuery);
		isAnyMatchingContent = false;

		// Loop through each main section
		mainSections.forEach(function (mainSection) {
			// check header text matches or not
			var headerText = mainSection.querySelector('.section-header').textContent.toLowerCase();
			var containsTermInHeader = headerText.includes(searchTerm.toLowerCase());
			// check main section text matches or not
			var sectionText = mainSection.textContent.toLowerCase();
			var containsTerm = sectionText.includes(searchTerm.toLowerCase());

			//sub-section text matches or not
			var subSections = mainSection.querySelectorAll('.sub-section');
			var subSectionContainsTerm = false;

			// if text matching with the main header then display full main section
			if (containsTermInHeader) {
				// first need to reset display of subsection
				subSections.forEach(function(subSection) {
					mainSection.style.backgroundColor = '';
					mainSection.style.paddingInline = '';
					mainSection.style.borderRadius = '';
					this.show(subSection);
				}.bind(this));
				mainSection.style.backgroundColor = 'var(--color-background-lighter)';
				mainSection.style.paddingInline = '12px';
				mainSection.style.borderRadius = 'var(--border-radius-large)';
				this.show(mainSection);
			}
			else {
				// else Loop through each sub-section and display subsections with matching text has search term
				subSections.forEach(function (subSection) {
					// Highlight matching sub-sections
					if (subSection.textContent.toLowerCase().includes(searchTerm.toLowerCase())) {
						subSection.style.color = 'var(--color-text-darker)';
						this.show(subSection);
						mainSection.style.backgroundColor = 'var(--color-background-lighter)';
						mainSection.style.paddingInline = '12px';
						mainSection.style.borderRadius = 'var(--border-radius-large)';
						// make sure main section of matched subsection is visible
						this.show(mainSection);
						subSectionContainsTerm = true;
					} else {
						subSection.style.color = ''; // Remove previous highlighting
						this.hide(subSection);
					}
				}.bind(this));
			}

			if (!subSectionContainsTerm && !containsTerm) {
				this.hide(mainSection);
			}
			else {
				isAnyMatchingContent = true;
			}

		}.bind(this));

		if (!isAnyMatchingContent) {
			this.resetFilterResults();
			$('#online-help-search-input').addClass('search-not-found');
			setTimeout(function () {
				$('#online-help-search-input').removeClass('search-not-found');
			}, 800);
		}
	},

	resetFilterResults: function () {
		var helpContentParent = document.getElementsByClassName('ui-dialog-content')[0];
		helpContentParent.style.backgroundColor='';
		// Select main sections and make it visible
		var mainSections = document.querySelectorAll('.section');
		mainSections.forEach(function(mainSection) {
			mainSection.style.backgroundColor = '';
			this.show(mainSection);

			var subSections = mainSection.querySelectorAll('.sub-section');
			subSections.forEach(function(subSection) {
				this.show(subSection);
			}.bind(this));
		}.bind(this));

		// select all event scroll elements, main-header elements, product header elements and make visible to user if search term is empty
		document.querySelectorAll('.m-v-0, .product-header, .help-dialog-header, .help-toc').forEach(function(element) {
			this.show(element);
			element.style.backgroundColor = '';
		}.bind(this));
	},

	show: function(element) {
		element.classList.remove('hide');
		element.classList.add('show');
	},

	hide: function(element) {
		element.classList.remove('show');
		element.classList.add('hide');
	},

	_doOpenHelpFile: function(data, id, map) {
		let productName;
		if (window.ThisIsAMobileApp) {
			productName = window.MobileAppName;
		} else {
			productName = (typeof brandProductName !== 'undefined') ? brandProductName : 'Collabora Online Development Edition (unbranded)';
		}

		map.uiManager.showYesNoButton(id + '-box', productName, '', _('OK'), null, null, null, true);
		app.layoutingService.appendLayoutingTask(() => {
			const box = document.getElementById(id + '-box');
			const innerDiv = window.L.DomUtil.create('div', '', null);
			box.insertBefore(innerDiv, box.firstChild);
			innerDiv.innerHTML = data;

			this.onHelpOpen(id, map, productName);
		});
	},

	showHelp: function(id) {
		var map = this;
		if (window.ThisIsAMobileApp) {
			map._doOpenHelpFile(window.HelpFile, id, map);
			return;
		}
		var helpLocation = 'cool-help.html';
		if (window.socketProxy)
			helpLocation = window.makeWsUrl('/browser/dist/' + helpLocation);
		$.get(helpLocation, function(data) {
			map._doOpenHelpFile(data, id, map);
		});
	},

	showLOAboutDialog: function() {
		if (this.aboutDialog)
			this.aboutDialog.show();
	},

	extractContent: function(html) {
		html = DocUtil.stripHTML(html);
		var parser = new DOMParser;
		return parser.parseFromString(html, 'text/html').documentElement.getElementsByTagName('body')[0].textContent;
	},

	makeURLFromStr: function(str) {
		str = str.trim();
		const lowerStr = str.toLowerCase();

		if (!(lowerStr.startsWith('http://') || lowerStr.startsWith('https://') ||
			  lowerStr.startsWith('ftp://') || lowerStr.startsWith('mailto:'))) {
			// Regular expression to test if the string is an email address
			const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
			if (emailPattern.test(str))
				str = 'mailto:' + str;
			else
				str = 'http://' + str;
		}
		return str;
	},

	getTextForLink: function() {
		var map = this;
		var text = '';
		if (this.hyperlinkUnderCursor && this.hyperlinkUnderCursor.text) {
			text = this.hyperlinkUnderCursor.text;
		} else if (this._clip && this._clip._selectionType == 'text') {
			if (map['stateChangeHandler'].getItemValue('.uno:Copy') === 'enabled') {
				if (window.L.Browser.clipboardApiAvailable) {
					// Async copy, trigger fetching the text selection.
					app.socket.sendMessage('gettextselection mimetype=text/html,text/plain;charset=utf-8');
				} else {
					text = this.extractContent(this._clip._selectionContent);
				}
			}
		} else if (this._docLayer._selectedTextContent) {
			text = this.extractContent(this._docLayer._selectedTextContent);
		}
		return text;
	},

	cancelSearch: function() {
		var toolbar = window.mode.isSmallScreenDevice() ? app.map.mobileSearchBar: app.map.statusBar;
		var searchInput = window.L.DomUtil.get('search-input');
		app.searchService.resetSelection();
		if (toolbar) {
			if (!window.mode.isSmallScreenDevice()) {
				toolbar.showItem('cancelsearch', false);
			}
			toolbar.enableItem('searchprev', false);
			toolbar.enableItem('searchnext', false);
		}
		searchInput.value = '';
		if (window.mode.isSmallScreenDevice()) {
			searchInput.focus();
			toolbar.enableItem('cancelsearch', false);
		}

		this._onGotFocus();
	},

	openRevisionHistory: function () {
		var map = this;
		// if we are being loaded inside an iframe, ask
		// our host to show revision history mode
		map.fire('postMessage', {msgId: 'rev-history', args: {Deprecated: true}});
		map.fire('postMessage', {msgId: 'UI_FileVersions'});
	},
	openShare: function () {
		var map = this;
		map.fire('postMessage', {msgId: 'UI_Share'});
	},
	openSaveAs: function (format) {
		var map = this;
		map.fire('postMessage', {msgId: 'UI_SaveAs', args: {format: format}});
	},

	onFormulaBarFocus: function() {
		if (window.mode.isSmallScreenDevice() === true) {
			var mobileTopBar = this.mobileTopBar;
			mobileTopBar.showItem('undo', false);
			mobileTopBar.showItem('redo', false);
			mobileTopBar.showItem('cancelformula', true);
			mobileTopBar.showItem('acceptformula', true);
		} else {
			var jsdialogFormulabar = this.formulabar;
			jsdialogFormulabar.hide('startformula');
			jsdialogFormulabar.hide('AutoSumMenu');
			jsdialogFormulabar.show('cancelformula');
			jsdialogFormulabar.show('acceptformula');
		}
	},

	onFormulaBarBlur: function() {
		var map = this;

		if (window.mode.isSmallScreenDevice() && this.isEditMode()) {
			var mobileTopBar = map.mobileTopBar;
			mobileTopBar.showItem('cancelformula', false);
			mobileTopBar.showItem('acceptformula', false);
			mobileTopBar.showItem('undo', true);
			mobileTopBar.showItem('redo', true);
		} else {
			var jsdialogFormulabar = map.formulabar;
			jsdialogFormulabar.hide('cancelformula');
			jsdialogFormulabar.hide('acceptformula');
			jsdialogFormulabar.show('startformula');
			jsdialogFormulabar.show('AutoSumMenu');
		}

		$('#AutoSumMenu-button').css('margin-inline', '0');
		$('#AutoSumMenu .unoarrow').css('margin', '0');

		map.formulabar.blurField();
		$('#addressInput input').blur();
	},

	formulabarBlur: function() {
		if (!this.uiManager.isAnyDialogOpen())
			this.focus();
	},

	formulabarFocus: function() {
		this.formulabar.focusField();
	},

	formulabarSetDirty: function() {
		if (this.formulabar)
			this.formulabar.dirty = true;
	},
});
