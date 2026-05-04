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
 * window.L.Control.NotebookbarBuilder - builder of native HTML widgets for tabbed menu
 */

/* global $ _ JSDialog app GraphicSelection Menubar */

window.L.Control.NotebookbarBuilder = window.L.Control.JSDialogBuilder.extend({

	_customizeOptions: function() {
		this.options.noLabelsForUnoButtons = true;
		this.options.useInLineLabelsForUnoButtons = false;
		this.options.cssClass = 'notebookbar';
	},

	_overrideHandlers: function() {
		var builder = this;
		const comboboxesFocusingDocument = ['fontnamecombobox', 'fontsizecombobox', 'styles'];
		const originalCallback = builder.callback;
		this.callback = function(objectType, eventType, object, data, builderArg) {
			if (eventType === 'selected'
				&& comboboxesFocusingDocument.indexOf(object.id) >= 0) {
				builder._defaultCallbackHandler(objectType, eventType, object, data, builderArg);
				builder.map.focus();
				return 'focusHandled';
			}
			// allow for customization
			return originalCallback(objectType, eventType, object, data, builderArg);
		};

		this._controlHandlers['bigtoolitem'] = this._bigtoolitemHandler;
		this._controlHandlers['combobox'] = this._comboboxControl;
		this._controlHandlers['exportmenubutton'] = this._exportMenuButton;
		this._controlHandlers['tabcontrol'] = this._overriddenTabsControlHandler;
		this._controlHandlers['iconviewlist'] = JSDialog.notebookbarIconViewList;
		this._controlHandlers['tabpage'] = this._overriddenTabPageHandler;

		this._toolitemHandlers['.uno:XLineColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FontColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:CharBackColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:BackgroundColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:TableCellBackgroundColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FrameLineColor'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:Color'] = JSDialog.colorPickerButton;
		this._toolitemHandlers['.uno:FillColor'] = JSDialog.colorPickerButton;

		this._toolitemHandlers['.uno:SelectBackground'] = this._selectBackgroundControl;
		this._toolitemHandlers['.uno:InsertAnnotation'] = this._insertAnnotationControl;
		this._toolitemHandlers['.uno:SetDefault'] = this._formattingControl;
		this._toolitemHandlers['.uno:Save'] = this._saveControl;
		this._toolitemHandlers['.uno:SaveAs'] = this._saveAsControl;
		this._toolitemHandlers['.uno:Print'] = this._printControl;
		this._toolitemHandlers['.uno:Settings'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:InsertPageHeader'] = this._headerFooterControl;
		this._toolitemHandlers['.uno:InsertPageFooter'] = this._headerFooterControl;
		this._toolitemHandlers['.uno:Text'] = this._insertTextBoxControl;
		this._toolitemHandlers['.uno:DrawText'] = this._insertTextBoxControl;
		this._toolitemHandlers['.uno:VerticalText'] = this._insertTextBoxControl;
		this._toolitemHandlers['.uno:OnlineHelp'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:ForumHelp'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:KeyboardShortcuts'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:ReportIssue'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:LatestUpdates'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:Feedback'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:About'] = this._onlineHelpControl;
		this._toolitemHandlers['.uno:FullScreen'] = this._onlineHelpControl;

		this._toolitemHandlers['.uno:Zoom'] = function() {};
		this._toolitemHandlers['.uno:InsertObject'] = function() {};
		this._toolitemHandlers['.uno:Gallery'] = function() {};
		this._toolitemHandlers['.uno:BasicShapes.ellipse'] = function() {};
		this._toolitemHandlers['.uno:BasicShapes.rectangle'] = function() {};
		this._toolitemHandlers['.uno:SymbolShapes'] = function() {};
		this._toolitemHandlers['.uno:ArrowShapes'] = function() {};
		this._toolitemHandlers['.uno:StarShapes'] = function() {};
		this._toolitemHandlers['.uno:CalloutShapes'] = function() {};
		this._toolitemHandlers['.uno:FlowChartShapes'] = function() {};
		this._toolitemHandlers['.uno:EmojiControl'] = function() {};
		this._toolitemHandlers['.uno:InsertDraw'] = function() {};
		this._toolitemHandlers['.uno:PageMargin'] = function() {};
		this._toolitemHandlers['.uno:Orientation'] = function() {};
		this._toolitemHandlers['.uno:SelectObject'] = function() {};
		this._toolitemHandlers['.uno:LineStyle'] = function() {};
		this._toolitemHandlers['.uno:AutoSum'] = function() {};
		this._toolitemHandlers['.uno:ReplyComment'] = function() {};
		this._toolitemHandlers['.uno:DeleteComment'] = function() {};
		this._toolitemHandlers['.uno:CompareDocuments'] = this._compareDocumentsControl;
		this._toolitemHandlers['.uno:MergeDocuments'] = function() {};
		this._toolitemHandlers['.uno:FunctionBox'] = function() {};
		this._toolitemHandlers['.uno:EditAnnotation'] = function() {};
		this._toolitemHandlers['.uno:ShowAllNotes'] = function() {};
		this._toolitemHandlers['.uno:HideAllNotes'] = function() {};
		this._toolitemHandlers['.uno:ShareDocument'] = function() {};
		this._toolitemHandlers['.uno:EditDoc'] = function() {};
		this._toolitemHandlers['.uno:PresentationCurrentSlide'] = function() {};
		this._toolitemHandlers['.uno:PresentationLayout'] = function() {};
		this._toolitemHandlers['.uno:CapturePoint'] = function() {};
		this._toolitemHandlers['.uno:Objects3DToolbox'] = function() {};
		this._toolitemHandlers['.uno:InsertMath'] = function() {};
		this._toolitemHandlers['.uno:ShowAnnotations'] = function() {};
		this._toolitemHandlers['.uno:DeleteAnnotation'] = function() {};
		this._toolitemHandlers['.uno:NextAnnotation'] = function() {};
		this._toolitemHandlers['.uno:PreviousAnnotation'] = function() {};
		this._toolitemHandlers['.uno:OptimizeTable'] = function() {};
		this._toolitemHandlers['.uno:TableDesign'] = function() {};
		this._toolitemHandlers['.uno:ContourDialog'] = function() {};
		this._toolitemHandlers['.uno:TextWrap'] = function() {};
		this._toolitemHandlers['.uno:RedactDoc'] = function() {};
		this._toolitemHandlers['.uno:FrameLineColor'] = function() {};
		this._toolitemHandlers['.uno:ProtectTraceChangeMode'] = function() {};
		this._toolitemHandlers['.uno:RowOperations'] = function() {};
		this._toolitemHandlers['.uno:ColumnOperations'] = function() {};
		this._toolitemHandlers['.uno:Insert'] = function() {};
		this._toolitemHandlers['.uno:InsertCell'] = function() {};
		this._toolitemHandlers['.uno:AutoFormat'] = function() {};
		this._toolitemHandlers['.uno:Spacing'] = function() {};
		this._toolitemHandlers['.uno:RotateRight'] = function() {};
		this._toolitemHandlers['.uno:AnchorMenu'] = function() {};
		this._toolitemHandlers['.uno:FontworkShapeType'] = function() {};
		this._toolitemHandlers['.uno:FillShadow'] = function() {};

		/*Draw Home Tab*/
		this._toolitemHandlers['.uno:ObjectAlign'] = function() {};

		/*Graphic Tab*/
		this._toolitemHandlers['.uno:GraphicFilterToolbox'] = function() {};
		this._toolitemHandlers['.uno:SaveGraphic'] = function() {};
		this._toolitemHandlers['.uno:InsertCaptionDialog'] = function() {};
		this._toolitemHandlers['.uno:CompressGraphic'] = function() {};
		this._toolitemHandlers['.uno:GraphicDialog'] = function() {};
		this._toolitemHandlers['.uno:BorderDialog'] = function() {};
		this._toolitemHandlers['.uno:FormatArea'] = function() {};

		this._toolitemHandlers['vnd.sun.star.findbar:FocusToFindbar'] = function() {};
	},

	_bigtoolitemHandler: function(parentContainer, data, builder) {
		var noLabels = builder.options.noLabelsForUnoButtons;
		builder.options.noLabelsForUnoButtons = false;

		builder._toolitemHandler(parentContainer, data, builder);

		builder.options.noLabelsForUnoButtons = noLabels;

		return false;
	},

	onCommandStateChanged: function(e) {
		var commandName = e.commandName;
		var state = e.state;

		if (commandName === '.uno:CharFontName') {
			if (window.ThisIsTheiOSApp) {
				if (state === '')
					$('#fontnamecomboboxios').html(_('Font Name'));
				else
					$('#fontnamecomboboxios').html(state);
				window.LastSetiOSFontNameButtonFont = state;
			}
		} else if (commandName === '.uno:StyleApply') {
			$('#applystyle').val(state).trigger('change');
		}
		else if (commandName === '.uno:ModifiedStatus') {
			const saveEle = document.querySelector('[id^="save"].unotoolbutton');
			if (saveEle) {
				if (state === 'true' &&  this.map.saveState) {
					this.map.saveState.showModifiedStatus();
					const button = document.querySelector('[id^="file-save"]');
					if (button) button.classList.add('savemodified');
				} else {
					const button = document.querySelector('[id^="save"]');
					if (button) button.classList.remove('savemodified');
					const fileButton = document.querySelector('[id^="file-save"]');
					if (fileButton) fileButton.classList.remove('savemodified');
				}
			}
		}
	},

	_comboboxControl: function(parentContainer, data, builder) {
		if (!data.entries || data.entries.length === 0)
			return false;

		// Fix exception due to undefined _createiOsFontButton function
		// Starting with commit 4082e1e570258f5032dfd460cf4d34ff6ae0d575,
		// this._createiOsFontButton() throws and exception because "this"
		// is a Window object. So fix this by moving the _createiOsFontButton
		// function inline.
		if (window.ThisIsTheiOSApp && data.id === 'fontnamecombobox') {
			// Fix issue #5838 Use unique IDs for font name combobox elements
			var table = window.L.DomUtil.createWithId('div', data.id, parentContainer);
			var row = window.L.DomUtil.create('div', 'notebookbar row', table);
			var button = window.L.DomUtil.createWithId('button', data.id + 'ios', row);

			$(table).addClass('ui-combobox jsdialog');
			$(row).addClass('ui-combobox-content');
			$(button).addClass('ui-combobox-content');

			if (data.selectedEntries.length && data.entries[data.selectedEntries[0]])
				button.innerText = data.entries[data.selectedEntries[0]];
			else if (window.LastSetiOSFontNameButtonFont)
				button.innerText = window.LastSetiOSFontNameButtonFont;
			else if (data.text)
				button.innerText = data.text;
			var map = builder.map;
			window.MagicFontNameCallback = function(font) {
				button.innerText = font;
				map.applyFont(font);
				map.focus();
			};
			button.onclick = function() {

				// There doesn't seem to be a way to pre-select an entry in the
				// UIFontPickerViewController so no need to pass the
				// current font here.
				window.postMobileMessage('FONTPICKER');
			};
			return false;
		}

		return JSDialog.combobox(parentContainer, data, builder);
	},

	// overriden
	_createTabClick: function(builder, t, tabs, contentDivs, tabIds)
	{
		const isDesktop = window.mode.isDesktop();
		const tooltipCollapsed = isDesktop ? _('Click to expand') : _('Tap to expand');
		const tooltipExpanded = isDesktop ? _('Click to collapse') : _('Tap to collapse');


		var isFileTab = tabIds[t] === 'File-tab-label' || tabIds[t] === 'File';
		var isFileTabForCoda = isFileTab && window.mode.isCODesktop();
		if (!isFileTabForCoda) {
			if ($(tabs[t]).hasClass('selected'))
				tabs[t].setAttribute('data-cooltip', tooltipExpanded);
			window.L.control.attachTooltipEventListener(tabs[t], builder.map);
		} else {
			tabs[t].removeAttribute('data-cooltip');
		}
		return function(event) {
			if (isFileTabForCoda) {
				if (builder.map.backstageView) {
					console.log('NotebookbarBuilder: Calling backstageView.toggle()');
					builder.map.backstageView.toggle();
				} else {
					console.error('NotebookbarBuilder: backstageView is NOT initialized!');
				}
				event.preventDefault();
				return;
			}

			var tabIsSelected = $(tabs[t]).hasClass('selected');
			var notebookbarIsCollapsed = builder.wizard.isCollapsed();

			var accessibilityInputElementHasFocus = app.UI.notebookbarAccessibility && app.UI.notebookbarAccessibility.accessibilityInputElement === document.activeElement ? true: false;

			for (var i = 0; i < tabs.length; i++) {
				if (i !== t) {
					tabs[i].setAttribute('data-cooltip', '');
				}
			}

			if (tabIsSelected && !notebookbarIsCollapsed && !accessibilityInputElementHasFocus) {
				builder.wizard.collapse();
				for (i = 0; i < tabs.length; i++)
					tabs[i].setAttribute('data-cooltip', tooltipCollapsed);
			} else {
				builder.wizard.extend();
				tabs[t].setAttribute('data-cooltip', tooltipExpanded);
			}

			$(tabs[t]).addClass('selected');
			tabs[t].setAttribute('aria-selected', 'true');
			tabs[t].removeAttribute('tabindex');
			for (i = 0; i < tabs.length; i++) {
				if (i !== t) {
					$(tabs[i]).removeClass('selected');
					tabs[i].setAttribute('aria-selected', 'false');
					tabs[i].tabIndex = -1;
					$(contentDivs[i]).addClass('hidden');
				}
			}
			$(contentDivs[t]).removeClass('hidden');
			$(window).resize();
			builder.map.fire('refreshoverflows',{force: true});
			builder.wizard.selectedTab(tabIds[t]);

			// Keep focus if user is navigating via keyboard.
			if (!tabs[t].enterPressed) {
				// don't lose focus on tab change
				event.preventDefault();
				if (!JSDialog.IsAnyInputFocused())
					builder.map.focus();
				t.enterPressed = false;
			}
		};
	},

	_overriddenTabsControlHandler: function(parentContainer, data, builder) {
		data.isNotebookbar = true;
		data.tabs = builder.wizard.getTabs();
		return builder._tabsControlHandler(parentContainer, data, builder, _('Tap to collapse'));
	},

	_overriddenTabPageHandler: function(parentContainer, data, builder) {
		var result = builder._tabPageHandler(parentContainer, data, builder);

		var tabPage = parentContainer.lastChild;
		JSDialog.MakeFocusCycle(tabPage);

		return result;
	},

	_exportMenuButton: function(parentContainer, data, builder) {
		if (data.id && data.id.startsWith('downloadas-')) {
			var format = data.id.substring('downloadas-'.length);
			app.registerExportFormat(data.text, format);

			if (builder.map['wopi'].HideExportOption)
				return false;
		}

		var separatorPos = data.id.indexOf(':');
		var menuId = data.id.substr(separatorPos + 1);
		var submenu = builder._getSubmenuOpts(builder.options.map._docLayer._docType, menuId, builder);

		JSDialog.MenuDefinitions.set(menuId, submenu);
		JSDialog.menubuttonControl(parentContainer, data, builder);

		for (var i in submenu) {
			var action = submenu[i].action;
			var text = submenu[i].action;

			if (action.startsWith('export')) {
				var format = action.substring('export'.length);
				app.registerExportFormat(text, format);
			}
			else if (action.startsWith('downloadas-')) {
				var format = action.substring('downloadas-'.length);
				app.registerExportFormat(text, format);
			}
		}

		return false;
	},

	_getSubmenuOpts: function(docType, id, builder) {
		switch (id) {
		case 'DownloadAsMenu':
			return builder._getDownloadAsSubmenuOpts(docType);
		case 'SaveAsMenu':
			return builder._getSaveAsSubmenuOpts(docType);
		case 'ExportAsMenu':
			return builder._getExportAsSubmenuOpts(docType);
		case 'PrintOptions':
			return builder._getPrintSubmenuOpts(docType);
		}
		return [];
	},

	_getDownloadAsSubmenuOpts: function(docType) {
		var submenuOpts = [];

		if (docType === 'text') {
			submenuOpts = [
				{
					'action': 'downloadas-odt',
					'text': _('ODF text document (.odt)')
				},
				{
					'action': 'downloadas-rtf',
					'text': _('Rich Text (.rtf)')
				},
				{
					'action': 'downloadas-docx',
					'text': _('Word Document (.docx)')
				},
				{
					'action': 'downloadas-doc',
					'text': _('Word 2003 Document (.doc)')
				},
				{
					'action': !window.ThisIsAMobileApp ? 'exportepub' : 'downloadas-epub',
					'text': _('EPUB (.epub)'),
					'command': !window.ThisIsAMobileApp ? 'exportepub' : 'downloadas-epub'
				},
				{
					'action': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
					'text': _('PDF Document (.pdf)'),
					'command': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf'
				},
				{
					'action': 'exportpdf' ,
					'text': _('PDF Document (.pdf) with options'),
					'command': 'exportpdf'
				}
			];
			submenuOpts.push({
				'action': 'downloadas-html',
				'text': _('HTML File (.html)')
			});
		} else if (docType === 'spreadsheet') {
			submenuOpts = [
				{
					'action': 'downloadas-ods',
					'text': _('ODF spreadsheet (.ods)')
				},
				{
					'action': 'downloadas-xlsx',
					'text': _('Excel Spreadsheet (.xlsx)')
				},
				{
					'action': 'downloadas-xls',
					'text': _('Excel 2003 Spreadsheet (.xls)')
				},
				{
					'action': 'downloadas-csv',
					'text': _('CSV File (.csv)')
				},
				{
					'action': 'downloadas-html',
					'text': _('HTML File (.html)')
				},
				{
					'action': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
					'text': _('PDF Document (.pdf)'),
					'command': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf'
				},
				{
					'action': 'exportpdf' ,
					'text': _('PDF Document (.pdf) with options'),
					'command': 'exportpdf'
				}
			];
		} else if (docType === 'presentation') {
			submenuOpts = [
				{
					'action': 'downloadas-odp',
					'text': _('ODF presentation (.odp)')
				},
				{
					'action': 'downloadas-odg',
					'text': _('ODF Drawing (.odg)')
				},
				{
					'action': 'downloadas-pptx',
					'text': _('PowerPoint Presentation (.pptx)')
				},
				{
					'action': 'downloadas-ppt',
					'text': _('PowerPoint 2003 Presentation (.ppt)')
				},
				{
					'action': 'downloadas-html',
					'text': _('HTML Document (.html)')
				},
				{
					'action': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
					'text': _('PDF Document (.pdf)'),
					'command': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
				},
				{
					'action': 'exportpdf',
					'text': _('PDF Document (.pdf) with options'),
					'command': 'exportpdf'
				}
			];
			if (window.extraExportFormats.includes('impress_svg'))
				submenuOpts.push({
					'action': 'downloadas-svg',
					'text': _('Scalable Vector Graphics (.svg)')
				});
			if (window.extraExportFormats.includes('impress_bmp'))
				submenuOpts.push({
					'action': 'downloadas-bmp',
					'text': _('Current slide as Bitmap (.bmp)')
				});
			if (window.extraExportFormats.includes('impress_gif'))
				submenuOpts.push({
					'action': 'downloadas-gif',
					'text': _('Current slide as Graphics Interchange Format (.gif)')
				});
			if (window.extraExportFormats.includes('impress_png'))
				submenuOpts.push({
					'action': 'downloadas-png',
					'text': _('Current slide as Portable Network Graphics (.png)')
				});
			if (window.extraExportFormats.includes('impress_tiff'))
				submenuOpts.push({
					'action': 'downloadas-tiff',
					'text': _('Current slide as Tag Image File Format (.tiff)')
				});
		} else if (docType === 'drawing') {
			submenuOpts = [
				{
					'action': 'downloadas-odg',
					'text': _('ODF Drawing (.odg)')
				},
				{
					'action': 'downloadas-png',
					'text': _('Image (.png)')
				}
			];
		}

		submenuOpts.forEach(function mapIconToItem(menuItem) {
			menuItem.icon = menuItem.action + '-submenu-icon';
		});

		return submenuOpts;
	},

	_getSaveAsSubmenuOpts: function(docType) {
		var submenuOpts = [];

		if (docType === 'text') {
			submenuOpts = [
				{
					'action': 'saveas-odt',
					'text': _('ODF text document (.odt)')
				},
				{
					'action': 'saveas-rtf',
					'text': _('Rich Text (.rtf)')
				},
				{
					'action': 'saveas-docx',
					'text': _('Word Document (.docx)')
				},
				{
					'action': 'saveas-doc',
					'text': _('Word 2003 Document (.doc)')
				}
			];
		} else if (docType === 'spreadsheet') {
			submenuOpts = [
				{
					'action': 'saveas-ods',
					'text': _('ODF spreadsheet (.ods)')
				},
				{
					'action': 'saveas-xlsx',
					'text': _('Excel Spreadsheet (.xlsx)')
				},
				{
					'action': 'saveas-xls',
					'text': _('Excel 2003 Spreadsheet (.xls)')
				}
			];
		} else if (docType === 'presentation') {
			submenuOpts = [
				{
					'action': 'saveas-odp',
					'text': _('ODF presentation (.odp)')
				},
				{
					'action': 'saveas-pptx',
					'text': _('PowerPoint Presentation (.pptx)')
				},
				{
					'action': 'saveas-ppt',
					'text': _('PowerPoint 2003 Presentation (.ppt)')
				}
			];
		}

		submenuOpts.forEach(function mapIconToItem(menuItem) {
			menuItem.icon = menuItem.action + '-submenu-icon';
		});

		return submenuOpts;
	},

	_getExportAsSubmenuOpts: function(docType) {
		var submenuOpts = [];

		if (docType === 'text') {
			submenuOpts = [
				{
					'action': 'exportas-pdf',
					'text': _('PDF Document (.pdf)')
				},
				{
					'action': 'exportas-epub',
					'text': _('EPUB (.epub)')
				}
			];
		} else if (docType === 'spreadsheet') {
			submenuOpts = [
				{
					'action': 'exportas-pdf',
					'text': _('PDF Document (.pdf)')
				}
			];
		} else if (docType === 'presentation') {
			submenuOpts = [
				{
					'action': 'exportas-pdf',
					'text': _('PDF Document (.pdf)')
				}
			];
		} else if (docType === 'drawing') {
			submenuOpts = [
				{
					'action': 'exportas-pdf',
					'text': _('PDF Document (.pdf)')
				}
			];
		}

		submenuOpts.forEach(function mapIconToItem(menuItem) {
			menuItem.icon = menuItem.action + '-submenu-icon';
		});

		return submenuOpts;
	},

	_getPrintSubmenuOpts: function(docType) {
		var submenuOpts = [];

		 if (docType === 'presentation') {
			submenuOpts = [
				{
					'action': 'print',
					'text':  _('Full Page Slides'),
				},
				{
					'action': 'print-notespages',
					'text':  _('Notes Pages'),
				}
			];
		}

		return submenuOpts;
	},

	_headerFooterControl: function(parentContainer, data, builder) {
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			if (!$(control.container).hasClass('disabled')) {
				builder.refreshSidebar = true;
				var command = data.command + '?On:bool=true';
				builder.callback('toolbutton', 'click', control.button, command, builder);
			}
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_insertTextBoxControl: function(parentContainer, data, builder) {
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			builder.map.sendUnoCommand(data.command + '?CreateDirectly:bool=true');
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_onlineHelpControl: function(parentContainer, data, builder) {
		var originalDataId = data.id; // builder can change this
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			(new Menubar())._executeAction.bind({_map: builder.options.map})(undefined, {id: originalDataId});
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_selectBackgroundControl: function(parentContainer, data, builder) {
		var options = {hasDropdownArrow: false};
		var control = builder._unoToolButton(parentContainer, data, builder, options);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			window.L.DomUtil.get('selectbackground').click();
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_compareDocumentsControl: function(parentContainer, data, builder) {
		const options = {hasDropdownArrow: false};
		const control = builder._unoToolButton(parentContainer, data, builder, options);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			window.L.DomUtil.get('comparedocuments').click();
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_insertAnnotationControl: function(parentContainer, data, builder) {
		var control = builder._unoToolButton(parentContainer, data, builder);
		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function (e) {
			e.preventDefault();
			var docLayer = builder.map._docLayer;
			if (!(docLayer._docType === 'spreadsheet' && GraphicSelection.hasActiveSelection())) {
				builder.map.insertComment();
			}
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_saveControl: function(parentContainer, data, builder) {
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');

		if (!control.label) {
			const tooltip = document.createElement('span');
			tooltip.id = 'save-status';
			tooltip.className = 'tooltip-label visuallyhidden';
			tooltip.setAttribute('role', 'tooltip');
			control.label = tooltip;

			control.button.parentElement.appendChild(tooltip);

			builder.map.on('updatemodificationindicator', function(e) {
				if (e.lastSaved) {
					tooltip.textContent = e.lastSaved;
				}
			});

			control.button.setAttribute('aria-describedby', tooltip.id);
			control.label = tooltip;
		}

		$(control.label).unbind('click');
		$(control.container).click(function () {
			// Save only when not read-only.
			if (!builder.map.isReadOnlyMode()) {
				builder.map.fire('postMessage', {msgId: 'UI_Save', args: { source: 'notebookbar' }});
				if (!builder.map._disableDefaultAction['UI_Save']) {
					builder.map.save(false, false);
				}
			}
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_saveAsControl: function(parentContainer, data, builder) {
		data.text = data.text.replace('...', '');
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			builder.map.openSaveAs();
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	_printControl: function(parentContainer, data, builder) {
		data.text = data.text.replace('...', '');
		var control = builder._unoToolButton(parentContainer, data, builder);

		$(control.button).unbind('click');
		$(control.label).unbind('click');
		$(control.container).click(function () {
			builder.map.print();
		});
		builder._preventDocumentLosingFocusOnClick(control.container);
	},

	buildControl: function(parent, data) {
		if (!data.length)
			return;

		const inlineLabels = this.options.useInLineLabelsForUnoButtons;
		this.options.useInLineLabelsForUnoButtons = false;

		data = data[0];

		var type = data.type;
		var handler = this._controlHandlers[type];

		var isVertical = (data.vertical === 'true' || data.vertical === true);
		var hasManyChildren = data.children && data.children.length > 1;

		if (handler)
			var processChildren = handler(parent, data, this);
		else
			window.app.console.warn('NotebookbarBuilder: Unsupported control type: "' + type + '"');

		if (processChildren && data.children != undefined)
			this.build(parent, data.children, isVertical, hasManyChildren);
		else if (data.visible && (data.visible === false || data.visible === 'false')) {
			$('#' + data.id).addClass('hidden-from-event');
		}

		this.options.useInLineLabelsForUnoButtons = inlineLabels;
	},

	// replaces widget in-place with new instance with updated data
	updateWidget: function (container, data) {
		this._updateWidgetImpl(container, data, this.buildControl);
	},

	build: function(parent, data, hasVerticalParent) {
		if (hasVerticalParent === undefined) {
			parent = window.L.DomUtil.create('div', 'root-container ' + this.options.cssClass, parent);
			parent = window.L.DomUtil.create('div', 'vertical ' + this.options.cssClass, parent);
		}

		for (var childIndex in data) {
			var childData = data[childIndex];
			if (!childData)
				continue;

			var childType = childData.type;
			var isVertical = (childData.vertical === 'true' || childData.vertical === true) ? true : false;

			var processChildren = true;

			if ((childData.id === undefined || childData.id === '' || childData.id === null)
				&& (childType == 'checkbox' || childType == 'radiobutton')) {
				continue;
			}

			var hasManyChildren = childData.children && childData.children.length > 1;
			var isContainer = this.isContainerType(childData.type);
			if (hasManyChildren && isContainer) {
				if (childData.id && childData.id.indexOf(' ') >= 0)
					console.error('notebookbar: space in the id: "' + childData.id + '"');
				var tableId = childData.id ? childData.id.replace(' ', '') : '';
				var table = window.L.DomUtil.createWithId('div', tableId, parent);
				window.L.DomUtil.addClass(table, this.options.cssClass);
				if (isVertical)
					window.L.DomUtil.addClass(table, 'vertical');
				else
					window.L.DomUtil.addClass(table, 'horizontal');
				var childObject = table;
			} else {
				childObject = parent;
			}

			// allow to detect single toolbuttons stacked on each other
			if (childType === 'toolbox')
				childData.hasVerticalParent = hasVerticalParent;

			var handler = this._controlHandlers[childType];
			var twoPanelsAsChildren =
			    childData.children && childData.children.length == 2
			    && childData.children[0] && childData.children[0].type == 'panel'
			    && childData.children[1] && childData.children[1].type == 'panel';

			try {
				if (twoPanelsAsChildren) {
					handler = this._controlHandlers['paneltabs'];
					handler(childObject, childData.children, this);
				} else {
					if (handler) {
						processChildren = handler(childObject, childData, this);
						this.postProcess(childObject, childData);
					} else
						window.app.console.warn('NotebookbarBuilder: Unsupported control type: "' + childType + '"');

					if (processChildren && childData.children != undefined)
						this.build(childObject, childData.children, isVertical);
					else if (childData.visible && (childData.visible === false || childData.visible === 'false')) {
						$('#' + childData.id).addClass('hidden-from-event');
					}
				}
			} catch (ex) {
				window.app.console.error('NotebookbarBuilder: exception while building "' + childData.id + '" : ' + ex);
			}
		}
	}

});

window.L.control.notebookbarBuilder = function (options) {
	var builder = new window.L.Control.NotebookbarBuilder(options);
	builder._setup(options);
	builder._overrideHandlers();
	builder._customizeOptions();
	return builder;
};
