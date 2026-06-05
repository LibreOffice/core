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
 * JSDialog.TopToolbar - component of top toolbar in compact mode
 */

/* global $ JSDialog _ _UNO app */
class TopToolbar extends JSDialog.Toolbar {
	constructor(map) {
		super(map, 'TopToolbar', 'toolbar-up');
		this.stylesSelectValue = null;

		map.on('doclayerinit', this.onDocLayerInit, this);
		this.onUpdatePermission = this.onUpdatePermission.bind(this);
		app.events.on('updatepermission', this.onUpdatePermission);
		map.on('wopiprops', this.onWopiProps, this);
		map.on('commandstatechanged', this.onCommandStateChanged, this);
		app.events.on('contextchange', this.onContextChange.bind(this));

		if (!window.mode.isSmallScreenDevice()) {
			map.on('updatetoolbarcommandvalues', this.updateCommandValues, this);
		}
	}

	onRemove() {
		if (this.parentContainer) {
			this.parentContainer.outerHTML = '';
			this.parentContainer = null;
		}

		this.map.off('doclayerinit', this.onDocLayerInit, this);
		app.events.off('updatepermission', this.onUpdatePermission);
		this.map.off('wopiprops', this.onWopiProps, this);
		this.map.off('commandstatechanged', this.onCommandStateChanged, this);

		if (!window.mode.isSmallScreenDevice()) {
			this.map.off('updatetoolbarcommandvalues', this.updateCommandValues, this);
		}
	}

	reset() {
		this.parentContainer = window.L.DomUtil.get('toolbar-up');

		// In case it contains garbage
		if (this.parentContainer) {
			this.parentContainer.outerHTML = '';
			this.parentContainer = null;
		}

		// Use original template as provided by server
		$('#toolbar-logo').after(this.map.toolbarUpTemplate.cloneNode(true));
		this.parentContainer = window.L.DomUtil.get('toolbar-up');
		window.L.DomUtil.addClass(this.parentContainer, 'ui-toolbar');
	}

	callback(objectType, eventType, object, data, builder) {
		if (object.id === 'fontnamecombobox') {
			if (eventType === 'selected')
				this.map.applyFont(data.substr(data.indexOf(';') + 1));
			else if (eventType === 'change')
				this.map.applyFont(data);
			this.map.focus();
			return 'focusHandled';
		}
		if (object.id === 'fontsizecombobox') {
			if (eventType === 'selected')
				this.map.applyFontSize(data.substr(data.indexOf(';') + 1));
			else if (eventType === 'change')
				this.map.applyFontSize(data);
			this.map.focus();
			return 'focusHandled';
		}
		if (object.id === 'styles') {
			if (eventType === 'selected') {
				const pos = parseInt(data.substr(0, data.indexOf(';')), 10);
				const label = data.substr(data.indexOf(';') + 1);
				const style = (this.styleProgNames && this.styleProgNames[pos] !== undefined)
					? this.styleProgNames[pos]
					: label;
				this.onStyleSelect({target: {value: style}});
				return 'focusHandled';
			} else if (eventType === 'change') {
				this.onStyleSelect({target: {value: data}});
				return 'focusHandled';
			}
			return;
		}

		this.builder._defaultCallbackHandler(objectType, eventType, object, data, builder);
	}

	onStyleSelect(e) {
		var style = e.target.value;
		if (style.startsWith('.uno:')) {
			this.map.sendUnoCommand(style);
		}
		else if (this.map.getDocType() === 'text') {
			this.map.applyStyle(style, 'ParagraphStyles');
		}
		else if (this.map.getDocType() === 'spreadsheet') {
			this.map.applyStyle(style, 'CellStyles');
		}
		else if (this.map.getDocType() === 'presentation' || this.map.getDocType() === 'drawing') {
			var layoutId = style;
			window.L.Styles.impressLayout.forEach(function(layout) {
				if (layout.translatedName === style)
					layoutId = layout.id;
			});
			this.map.applyLayout(layoutId);
		}
		this.map.focus();
	}

	onContextChange(event) {
		this.updateVisibilityForToolbar(event.detail.context);
	}

	// mobile:false means hide it both for normal Online used from a mobile phone browser, and in a mobile app on a mobile phone
	// mobilebrowser:false means hide it for normal Online used from a mobile browser, but don't hide it in a mobile app
	// tablet:true means show it in normal Online from a tablet browser, and in a mobile app on a tablet
	// tablet:false means hide it in normal Online used from a tablet browser, and in a mobile app on a tablet

	// hidden means display:none
	// invisible means visibility:hidden

	getToolItems() {
		var saveGroup = [
			{type: 'customtoolitem',  id: 'closemobile', desktop: false, mobile: false, tablet: true, visible: false},
			{type: 'customtoolitem',  id: 'save', command: 'save', text: _UNO('.uno:Save'), lockUno: '.uno:Save', icon: 'compact_save.svg', accessibility: { focusBack: true, combination: 'SV' }},
		];
		var printGroup = [
			{type: 'customtoolitem',  id: 'print', command: 'print', text: _UNO('.uno:Print'), mobile: false, tablet: false, lockUno: '.uno:Print', icon: 'compact_print.svg', accessibility: { focusBack: true, combination: 'PT' }},
			{type: 'menubutton',  id: 'printoptions',  command: 'printoptions', noLabel: true, text: _UNO('.uno:Print', 'text'), mobile: false, tablet: false, lockUno: '.uno:Print', icon: 'compact_print.svg', accessibility: { focusBack: true, combination: 'PT' },
				menu: [
					{id: 'print-active-sheet', action: 'print-active-sheet', text: _('Active Sheet')},
					{id: 'print-all-sheets', action: 'print-all-sheets', text: _('All Sheets')},
				]
			}
		];
		var undoGroup = [
			{type: 'toolitem',  id: 'undo', text: _UNO('.uno:Undo'), command: '.uno:Undo', mobile: false, icon: 'compact_undo.svg', accessibility: { focusBack: true, combination: 'ZZ' }},
			{type: 'toolitem',  id: 'redo', text: _UNO('.uno:Redo'), command: '.uno:Redo', mobile: false, icon: 'compact_redo.svg', accessibility: { focusBack: true, combination: 'RE' }}
		];
		var fontGroup = [
			{type: 'toolitem',  id: 'formatpaintbrush', text: _UNO('.uno:FormatPaintbrush'), tooltip: _('Clone Formatting (double click to keep active)'), activeTooltip: _('Clone Formatting is active (click again or press Esc to exit)'), command: '.uno:FormatPaintbrush', doubleClickCommand: '.uno:FormatPaintbrush', doubleClickCommandArgs: { PersistentCopy: { type: 'boolean', value: true } }, mobile: false, icon: 'compact_formatpaintbrush.svg', accessibility: { focusBack: true, combination: 'CF' }},
			{type: 'toolitem',  id: 'reset', text: _UNO('.uno:ResetAttributes', 'text'), visible: false, command: '.uno:ResetAttributes', mobile: false, icon: 'compact_setdefault.svg', accessibility: { focusBack: true, combination: 'RF' }},
			{type: 'toolitem',  id: 'resetimpress', class: 'unoResetAttributes', text: _UNO('.uno:SetDefault', 'presentation', 'true'), visible: false, command: '.uno:SetDefault', mobile: false, icon: 'compact_setdefault.svg', accessibility: { focusBack: true, combination: 'RF' }},
			{type: 'separator', orientation: 'vertical', id: 'breakreset', invisible: true, mobile: false, tablet: false,},
			{type: 'combobox', id: 'styles', text: _('Default Style'), desktop: true, mobile: false, tablet: false, changeOnEnterOnly: true, accessibility: { focusBack: true, combination: 'DS' }},
			{type: 'combobox', id: 'fontnamecombobox', text: 'Carlito', command: '.uno:CharFontName', mobile: false, changeOnEnterOnly: true, accessibility: { focusBack: true, combination: 'FN' }},
			{type: 'combobox', id: 'fontsizecombobox', text: '12 pt', command: '.uno:FontHeight', mobile: false, changeOnEnterOnly: true, entries: ['6','7','8','9','10','10.5','11','12','13','14','15','16','18','20','22','24','26','28','32','36','40','44','48','54','60','66','72','80','88','96'], accessibility: { focusBack: true, combination: 'FS' }}
		];
		var formatGroup = [
			{type: 'toolitem',  id: 'bold', text: _UNO('.uno:Bold'), command: '.uno:Bold', accessibility: { focusBack: true, combination: '1' }},
			{type: 'toolitem',  id: 'italic', text: _UNO('.uno:Italic'), command: '.uno:Italic', accessibility: { focusBack: true, combination: '2' }},
			{type: 'toolitem',  id: 'underline', text: _UNO('.uno:Underline'), command: '.uno:Underline', accessibility: { focusBack: true, combination: '3' }},
			{type: 'toolitem',  id: 'strikeout', text: _UNO('.uno:Strikeout'), command: '.uno:Strikeout', accessibility: { focusBack: true, combination: '4' }},
		];
		const fontColorGroup = [
			{type: 'colorlistbox',  id: 'fontcolorwriter:ColorPickerMenu', command: '.uno:FontColor', text: _UNO('.uno:FontColor'), visible: false, lockUno: '.uno:FontColor', icon: 'compact_fontcolor.svg', accessibility: { focusBack: true, combination: 'FC' }},
			{type: 'colorlistbox',  id: 'fontcolor:ColorPickerMenu', command: '.uno:Color', text: _UNO('.uno:FontColor'), lockUno: '.uno:FontColor', icon: 'compact_fontcolor.svg', accessibility: { focusBack: true, combination: 'FC' }},
			{type: 'colorlistbox',  id: 'backcolor:ColorPickerMenu', command: '.uno:CharBackColor', text: _UNO('.uno:CharBackColor', 'text'), visible: false, lockUno: '.uno:CharBackColor', icon: 'compact_backcolor.svg', accessibility: { focusBack: true, combination: 'HC' }},
			{type: 'colorlistbox',  id: 'backgroundcolor:ColorPickerMenu', command: '.uno:BackgroundColor', text: _UNO('.uno:BackgroundColor'), visible: false, lockUno: '.uno:BackgroundColor', icon: 'compact_fillcolor.svg', accessibility: { focusBack: true, combination: 'BC' }},
		];
		const indentGroup = [
			{type: 'toolitem',  id: 'leftpara',  command: '.uno:LeftPara', text: _UNO('.uno:LeftPara', '', true), visible: false, icon: 'compact_leftpara.svg', accessibility: { focusBack: true, combination: 'AL' }},
			{type: 'toolitem',  id: 'centerpara',  command: '.uno:CenterPara', text: _UNO('.uno:CenterPara', '', true), visible: false, icon: 'compact_centerpara.svg', accessibility: { focusBack: true, combination: 'AC' }},
			{type: 'toolitem',  id: 'rightpara',  command: '.uno:RightPara', text: _UNO('.uno:RightPara', '', true), visible: false, icon: 'compact_rightpara.svg', accessibility: { focusBack: true, combination: 'AR' }},
			{type: 'toolitem',  id: 'justifypara',  command: '.uno:JustifyPara', text: _UNO('.uno:JustifyPara', '', true), visible: false, icon: 'compact_justifypara.svg', accessibility: { focusBack: true, combination: 'AJ' }},
			{type: 'separator', orientation: 'vertical', id: 'breakpara', visible: false},
			{type: 'menubutton',  id: 'setborderstyle:BorderStyleMenu', noLabel: true, command: '.uno:SetBorderStyle', text: _('Borders'), visible: false, icon: 'compact_setborderstyle.svg', accessibility: { focusBack: true, combination: 'BS'}},
			{type: 'toolitem',  id: 'togglemergecells', text: _UNO('.uno:ToggleMergeCells', 'spreadsheet', true), visible: false, command: '.uno:ToggleMergeCells', icon: 'compact_togglemergecells.svg', accessibility: { focusBack: true, combination: 'MC' }},
			{type: 'separator', orientation: 'vertical', id: 'breakmergecells', visible: false},
			{type: 'menubutton', id: 'textalign', command: 'justifypara', noLabel: true, text: _UNO('.uno:TextAlign'), visible: false, lockUno: '.uno:TextAlign', icon: 'compact_justifypara.svg', accessibility: { focusBack: true, combination: 'AT' },
				menu: [
					{id: 'alignleft', text: _UNO('.uno:AlignLeft', 'spreadsheet', true), icon: 'alignleft', uno: '.uno:AlignLeft'},
					{id: 'alignhorizontalcenter', text: _UNO('.uno:AlignHorizontalCenter', 'spreadsheet', true), icon: 'alignhorizontal', uno: '.uno:AlignHorizontalCenter'},
					{id: 'alignright', text: _UNO('.uno:AlignRight', 'spreadsheet', true), icon: 'alignright', uno: '.uno:AlignRight'},
					{id: 'alignblock', text: _UNO('.uno:AlignBlock', 'spreadsheet', true), icon: 'alignblock', uno: '.uno:AlignBlock'},
					{type: 'separator'},
					{id: 'aligntop', text: _UNO('.uno:AlignTop', 'spreadsheet', true), icon: 'aligntop', uno: '.uno:AlignTop'},
					{id: 'alignvcenter', text: _UNO('.uno:AlignVCenter', 'spreadsheet', true), icon: 'alignvcenter', uno: '.uno:AlignVCenter'},
					{id: 'alignbottom', text: _UNO('.uno:AlignBottom', 'spreadsheet', true), icon: 'alignbottom', uno: '.uno:AlignBottom'},
				]},
			{type: 'menubutton',  id: 'linespacing',  command: 'linespacing', noLabel: true, text: _UNO('.uno:FormatSpacingMenu'), visible: false, lockUno: '.uno:FormatSpacingMenu', icon: 'compact_linespacing.svg', accessibility: { focusBack: true, combination: 'LS' },
				menu: [
					{id: 'spacepara1', text: _UNO('.uno:SpacePara1'), uno: '.uno:SpacePara1'},
					{id: 'spacepara115', text: _UNO('.uno:SpacePara115'), uno: '.uno:SpacePara115'},
					{id: 'spacepara15', text: _UNO('.uno:SpacePara15'), uno: '.uno:SpacePara15'},
					{id: 'spacepara2', text: _UNO('.uno:SpacePara2'), uno: '.uno:SpacePara2'},
					{type: 'separator'},
					{id: 'paraspaceincrease', text: _UNO('.uno:ParaspaceIncrease'), uno: '.uno:ParaspaceIncrease'},
					{id: 'paraspacedecrease', text: _UNO('.uno:ParaspaceDecrease'), uno: '.uno:ParaspaceDecrease'},
					{type: 'separator'},
					{id: 'paragraphdialog', text: _UNO('.uno:ParagraphDialog'), uno: '.uno:ParagraphDialog'}
				],
			},
			{type: 'toolitem',  id: 'wraptextbutton', text: _UNO('.uno:WrapText', 'spreadsheet', true), visible: false, command: '.uno:WrapText', icon: 'compact_wraptext.svg', accessibility: { focusBack: true, combination: 'WT' }}
		];
		var otherGroup = [
			{type: 'toolitem',  id: 'defaultnumbering', text: _UNO('.uno:DefaultNumbering', '', true), visible: false, command: '.uno:DefaultNumbering', icon: 'compact_defaultnumbering.svg', accessibility: { focusBack: true, combination: 'OL' }},
			{type: 'toolitem',  id: 'defaultbullet', text: _UNO('.uno:DefaultBullet', '', true), visible: false, command: '.uno:DefaultBullet', icon: 'compact_defaultbullet.svg', accessibility: { focusBack: true, combination: 'UL' }},
			{type: 'separator', orientation: 'vertical', id: 'breakbullet', visible: false},
			{type: 'toolitem',  id: 'incrementindent', text: _UNO('.uno:IncrementIndent', '', true), command: '.uno:IncrementIndent', visible: false, icon: 'compact_leftindent.svg', accessibility: { focusBack: true, combination: 'II' }},
			{type: 'toolitem',  id: 'decrementindent', text: _UNO('.uno:DecrementIndent', '', true), command: '.uno:DecrementIndent', visible: false, icon: 'compact_decrementindent.svg', accessibility: { focusBack: true, combination: 'DI' }},
			{type: 'separator', orientation: 'vertical', id: 'breakindent', visible: false},
			{type: 'menubutton', id: 'conditionalformatdialog:ConditionalFormatMenu', noLabel: true, text: _UNO('.uno:ConditionalFormatMenu', 'spreadsheet', true), visible: false, lockUno: '.uno:ConditionalFormatMenu', icon: 'compact_conditionalformatmenu.svg', accessibility: { focusBack: true, combination: 'CO' }},
			{type: 'toolitem',  id: 'sortascending', text: _UNO('.uno:SortAscending', 'spreadsheet', true), command: '.uno:SortAscending', visible: false, icon: 'compact_sortascending.svg', accessibility: { focusBack: true, combination: 'SA' }},
			{type: 'toolitem',  id: 'sortdescending', text: _UNO('.uno:SortDescending', 'spreadsheet', true), command: '.uno:SortDescending', visible: false, icon: 'compact_sortdescending.svg', accessibility: { focusBack: true, combination: 'SD' }},
			{type: 'separator', orientation: 'vertical', id: 'breaksorting', visible: false},
			{type: 'toolitem',  id: 'numberformatcurrency', text: _UNO('.uno:NumberFormatCurrency', 'spreadsheet', true), visible: false, command: '.uno:NumberFormatCurrency', accessibility: { focusBack: true, combination: 'NC' }},
			{type: 'toolitem',  id: 'numberformatpercent', text: _UNO('.uno:NumberFormatPercent', 'spreadsheet', true), visible: false, command: '.uno:NumberFormatPercent', accessibility: { focusBack: true, combination: 'NP' }},
			{type: 'toolitem',  id: 'numberformatdecdecimals', text: _UNO('.uno:NumberFormatDecDecimals', 'spreadsheet', true), visible: false, command: '.uno:NumberFormatDecDecimals', accessibility: { focusBack: true, combination: 'DD' }},
			{type: 'toolitem',  id: 'numberformatincdecimals', text: _UNO('.uno:NumberFormatIncDecimals', 'spreadsheet', true), visible: false, command: '.uno:NumberFormatIncDecimals', accessibility: { focusBack: true, combination: 'ID' }},
			{type: 'separator', orientation: 'vertical',   id: 'break-number', visible: false},
			{type: 'button', id: 'gridvisible', img: 'gridvisible', hint: _UNO('.uno:GridVisible'), uno: 'GridVisible', hidden: true},
			{type: 'button', id: 'griduse', img: 'griduse', hint: _UNO('.uno:GridUse'), uno: 'GridUse', hidden: true},
			{type: 'menubutton',  id: 'inserttable:InsertTableMenu', command: 'inserttable', noLabel: true, text: _('Insert table'), visible: false, lockUno: '.uno:InsertTable', icon: 'compact_inserttable.svg', accessibility: { focusBack: true, combination: 'IT' }},
			{type: 'menubutton', id: 'menugraphic:InsertImageMenu', noLabel: true, command: '.uno:InsertGraphic', text: _UNO('.uno:InsertGraphic', '', true), visible: false, lockUno: '.uno:InsertGraphic', icon: 'compact_insertgraphic.svg', accessibility: { focusBack: true, combination: 'IG' }},
			{type: 'toolitem',  id: 'insertobjectchart', text: _UNO('.uno:InsertObjectChart', '', true), command: '.uno:InsertObjectChart', icon: 'compact_drawchart.svg', accessibility: { focusBack: true, combination: 'IC' }},
			{type: 'menubutton',  id: 'insertshapes:InsertShapesMenu', command: '.uno:BasicShapes', noLabel: true, text: _('Insert shapes'), icon: 'compact_basicshapes.svg', accessibility: { focusBack: true, combination: 'IP' }},
			{type: 'toolitem',  id: 'insertline', text: _UNO('.uno:Line', '', true), command: '.uno:Line', icon: 'compact_line.svg', accessibility: { focusBack: true, combination: 'IL' }},
			{type: 'menubutton',  id: 'insertconnectors:InsertConnectorsMenu', command: 'connector', noLabel: true, text: _('Insert connectors'), visible: false, accessibility: { focusBack: true, combination: 'IR' }},
			{type: 'separator', orientation: 'vertical',   id: 'breakinsert', desktop: true},
			{type: 'customtoolitem',  id: 'inserttextbox', text: _UNO('.uno:Text', '', true), command: 'inserttextbox', visible: false, icon: 'compact_text.svg', accessibility: { focusBack: true, combination: 'IB' }},
			{type: 'customtoolitem',  id: 'insertannotation', text: _UNO('.uno:InsertAnnotation', '', true), visible: false, lockUno: '.uno:InsertAnnotation', icon: 'compact_shownote.svg', accessibility: { focusBack: true, combination: 'IA' }},
			{type: 'customtoolitem',  id: 'inserthyperlink',  command: 'inserthyperlink', text: _UNO('.uno:HyperlinkDialog', '', true), lockUno: '.uno:HyperlinkDialog', icon: 'compact_inserthyperlink.svg', accessibility: { focusBack: true, combination: 'IH' }},
			{type: 'toolitem',  id: 'insertsymbol', text: _UNO('.uno:InsertSymbol', '', true), command: '.uno:InsertSymbol', icon: 'compact_insertsymbol.svg', accessibility: { focusBack: true, combination: 'IS' }},
			];

		var items = [
			{type: 'overflowmanager', id: 'overflow-manager-toptoolbar', children: [
				{type: 'overflowgroup', id: 'save-toptoolbar', children: saveGroup},
				{type: 'separator', orientation: 'vertical', id: 'savebreak', mobile: false},
				{type: 'overflowgroup', id: 'undo-toptoolbar', children: undoGroup},
				{type: 'separator', orientation: 'vertical', id: 'redobreak', mobile: false, tablet: false,},
				{type: 'overflowgroup', id: 'save-toptoolbar', children: printGroup},
				{type: 'separator', orientation: 'vertical', id: 'printbreak', mobile: false},
				{type: 'overflowgroup', id: 'font-toptoolbar', children: fontGroup},
				{type: 'separator', orientation: 'vertical', id: 'breakfontsizes', invisible: true, mobile: false, tablet: false},
				{type: 'overflowgroup', id: 'format-toptoolbar', children: formatGroup},
				{type: 'separator', orientation: 'vertical', id: 'breakformatting'},
				{type: 'overflowgroup', id: 'fontcolor-toptoolbar', children: fontColorGroup},
				{type: 'separator', orientation: 'vertical' , id: 'breakcolor', mobile:false},
				{type: 'overflowgroup', id: 'indent-toptoolbar', children: indentGroup},
				{type: 'separator', orientation: 'vertical', id: 'breakspacing', visible: false},
				{type: 'overflowgroup', id: 'other-toptoolbar', children: otherGroup},
			]},
			{type: 'spacer', id: 'topspacer'},
			{type: 'separator', orientation: 'vertical', id: 'breaksidebar', visible: false},
			{type: 'toolitem',  id: 'sidebar', text: _UNO('.uno:Sidebar', '', true), command: '.uno:SidebarDeck.PropertyDeck', visible: false, icon: 'compact_sidebar.svg', accessibility: { focusBack: true, combination: 'ZB' }},
			{type: 'toolitem',  id: 'modifypage', text: _UNO('.uno:ModifyPage', 'presentation', true), command: '.uno:ModifyPage', visible: false, icon: 'compact_sidebar.svg', accessibility: { focusBack: true, combination: 'ZL' }},
			{type: 'customtoolitem',  id: 'slidechangewindow', text: _UNO('.uno:SlideChangeWindow', 'presentation', true), command: 'transitiondeck', icon: 'compact_slidechangewindow.svg', visible: false, accessibility: { focusBack: true, combination: 'ZT' }},
			{type: 'toolitem',  id: 'customanimation', text: _UNO('.uno:CustomAnimation', 'presentation', true), command: '.uno:CustomAnimation', visible: false, icon: 'compact_customanimation.svg', accessibility: { focusBack: true, combination: 'ZA' }},
			{type: 'toolitem',  id: 'masterslidespanel', text: _UNO('.uno:MasterSlidesPanel', 'presentation', true), command: '.uno:MasterSlidesPanel', visible: false, icon: 'compact_masterslides.svg', accessibility: { focusBack: true, combination: 'ZM' }},
			{type: 'customtoolitem',  id: 'fold', desktop: false, mobile: false, tablet: true, iosapptablet: false, visible: false},
		];

		this.customizeItems(items[0].children);
		return items;
	}

	customizeItems(overflowManagerItem) {
		const matchesItem = (where, toInsert) => {
			return where.id.toLowerCase() === toInsert.beforeId.toLowerCase();
		};
		const insertItem = (group, toInsert) => {
			const subNodes = group && group.children ? group.children : [];
			for (const i in subNodes) {
				const node = subNodes[i];
				if (matchesItem(node, toInsert)) {
					const items = toInsert.items;
					for (const j in items) {
						const position = Number(i) + Number(j);
						subNodes.splice(position, 0, items[j]);
						app.console.debug('Toolbar: inserted ' + items[j].id + ' item at position: ' + position);
					}

					return true;
				}
			}

			return false;
		};

		this.customItems.forEach((customButton) => {
			// main level
			if (insertItem(overflowManagerItem, customButton))
				return;

			// groups
			let inserted = false;
			overflowManagerItem.forEach((item) => {
				if (inserted) return;
				inserted = insertItem(item, customButton);
			});

			// fallback
			if (!inserted)
				overflowManagerItem.push(customButton);
		});
	}

	updateControlsState() {
		if (this.map['stateChangeHandler']) {
			var items = this.map['stateChangeHandler'].getItems();
			if (items) {
				for (var item in items) {
					this.processStateChangedCommand(item, items[item]);
				}
			}
		}
	}

	create() {
		this.reset();

		var items = this.getToolItems();
		this.builder.build(this.parentContainer, items);

		if (window.mode.isSmallScreenDevice()) {
			JSDialog.MakeScrollable(this.parentContainer, this.parentContainer.querySelector('div'));
			JSDialog.RefreshScrollables();
		}

		if (this.map.isRestrictedUser()) {
			for (var i = 0; i < items.length; i++) {
				var it = items[i];
				var item = $('#' + it.id)[0];
				this.map.hideRestrictedItems(it, item, item);
			}
		}

		if (this.map.isLockedUser()) {
			for (var i = 0; i < items.length; i++) {
				var it = items[i];
				var item = $('#' + it.id)[0];
				this.map.disableLockedItem(it, item, item);
			}
		}

		this.map.createFontSelector('fontnamecombobox');

		// on mode switch NB -> Compact
		if (this.map._docLoadedOnce)
			this.onDocLayerInit();

		// if app opens direct in compact mode then we need to set the saveState first
		this.map.saveState = new app.definitions.saveState(this.map);
	}

	onDocLayerInit() {
		var docType = this.map.getDocType();

		switch (docType) {
		case 'spreadsheet':
			if (this.parentContainer) {
				['reset', 'textalign', 'wraptextbutton', 'breakspacing', 'insertannotation', 'conditionalformatdialog',
					'numberformatcurrency', 'numberformatpercent',
					'numberformatincdecimals', 'numberformatdecdecimals', 'break-number', 'togglemergecells', 'breakmergecells',
					'setborderstyle', 'sortascending', 'sortdescending', 'breaksorting', 'backgroundcolor', 'breaksidebar', 'sidebar', 'printoptions'
				].forEach((id) => {
					this.showItem(id, true);
				});

				this.showItem('print', false);
				this.showItem('styles', false);
			}

			$('#toolbar-wrapper').addClass('spreadsheet');
			if (window.mode.isTablet()) {
				$(this.map.options.documentContainer).addClass('tablet');
				$('#toolbar-wrapper').addClass('tablet');
			}

			break;
		case 'text':
			if (this.parentContainer) {
				['fontcolorwriter', 'reset', 'leftpara', 'centerpara', 'rightpara', 'justifypara', 'breakpara', 'linespacing',
					'breakspacing', 'defaultbullet', 'defaultnumbering', 'breakbullet', 'incrementindent', 'decrementindent',
					'breakindent', 'inserttable', 'insertannotation', 'backcolor', 'breaksidebar', 'sidebar'
				].forEach((id) => {
					this.showItem(id, true);
				});

				this.showItem('printoptions', false);
				this.showItem('fontcolor', false);
			}
			break;
		case 'presentation':
			{
				// Fill the style combobox with Impress layouts
				var stylesContainer = document.getElementById('styles');
				if (stylesContainer && stylesContainer.updateEntries) {
					var layoutEntries = [];
					window.L.Styles.impressLayout.forEach(function(layout) {
						layoutEntries.push(layout.translatedName);
					});
					stylesContainer.updateEntries(layoutEntries);
				}
			}

			if (this.parentContainer) {
				['resetimpress', 'breaksidebar', 'modifypage',
					'leftpara', 'centerpara', 'rightpara', 'justifypara', 'breakpara', 'linespacing',
					'gridvisible', 'griduse',
					'breakspacing', 'defaultbullet', 'defaultnumbering', 'breakbullet', 'inserttextbox', 'inserttable',  'insertannotation', 'backcolor',
					'breaksidebar', 'modifypage', 'slidechangewindow', 'customanimation', 'masterslidespanel', 'navigator'
				].forEach((id) => {
					this.showItem(id, true);
				});

				this.showItem('printoptions', false);
			}
			break;
		case 'drawing':
			if (this.parentContainer) {
				['leftpara', 'centerpara', 'rightpara', 'justifypara', 'breakpara', 'linespacing', 'gridvisible', 'griduse',
					'breakspacing', 'defaultbullet', 'defaultnumbering', 'breakbullet', 'inserttextbox', 'inserttable', 'backcolor',
					'breaksidebar', 'sidebar', 'insertconnectors'
				].forEach((id) => {
					this.showItem(id, true);
				});

				this.showItem('printoptions', false);
			}
			break;
		}

		this.updateVisibilityForToolbar();

		this.map.createFontSizeSelector('fontsizecombobox');

		JSDialog.RefreshScrollables();
	}

	onUpdatePermission(e) {
		var ids = ['styles', 'fontnamecombobox', 'fontsizecombobox'];
		ids.forEach(function(id) {
			var el = document.getElementById(id);
			if (el) {
				if (e.detail.perm === 'edit')
					el.removeAttribute('disabled');
				else
					el.setAttribute('disabled', 'true');
			}
		});
	}

	onWopiProps(e) {
		if (e.HideSaveOption) {
			this.showItem('save', false);
		}
		if (e.HidePrintOption) {
			this.showItem('print', false);
		}

		// On desktop we only have Save and Print buttons before the first
		// splitter/break. Hide the splitter if we hid both save and print.
		// TODO: Apply the same logic to mobile/tablet to avoid beginning with a splitter.
		if (window.mode.isDesktop() && e.HideSaveOption && e.HidePrintOption) {
			this.showItem('savebreak', false);
		}

		if (this.parentContainer) {
			if (e.EnableInsertRemoteImage || !e.DisableInsertLocalImage) {
				this.showItem('menugraphic', true);
			} else {
				this.showItem('menugraphic', false);
			}
		}
	}

	updateCommandValues(e) {
		if (e.commandName !== '.uno:StyleApply')
			return;

		var commandValues = this.map.getToolbarCommandValues(e.commandName);
		if (typeof commandValues === 'undefined')
			return;

		if (this.map.getDocType() === 'presentation') {
			// styles are not applied for presentation
			return;
		}

		var entries = [];
		this.styleProgNames = [];
		var commands = commandValues.Commands;
		if (commands && commands.length > 0) {
			commands.forEach(function (command) {
				var translated = command.text;
				if (window.L.Styles.styleMappings[command.text]) {
					translated = window.L.Styles.styleMappings[command.text].toLocaleString();
				}
				entries.push(translated);
				this.styleProgNames.push(command.id);
			}, this);
		}

		var styles = [];
		var topStyles = [];
		if (this.map.getDocType() === 'text') {
			styles = commandValues.ParagraphStyles.slice(7);
			topStyles = commandValues.ParagraphStyles.slice(0, 7);
		}
		else if (this.map.getDocType() === 'spreadsheet') {
			styles = commandValues.CellStyles;
		}

		topStyles.forEach(function (style) {
			entries.push(window.L.Styles.styleMappings[style].toLocaleString());
			this.styleProgNames.push(style);
		}, this);

		if (styles !== undefined && styles.length > 0) {
			styles.forEach(function (style) {
				var localeStyle;
				if (style.startsWith('outline')) {
					var outlineLevel = style.split('outline')[1];
					localeStyle = 'Outline'.toLocaleString() + ' ' + outlineLevel;
				} else {
					localeStyle = window.L.Styles.styleMappings[style];
					localeStyle = localeStyle === undefined ? style : localeStyle.toLocaleString();
				}
				entries.push(localeStyle);
				this.styleProgNames.push(style);
			}, this);
		}

		var container = document.getElementById('styles');
		if (container && container.updateEntries) {
			container.updateEntries(entries);
			if (this.stylesSelectValue)
				container.onSetText(this.stylesSelectValue);
		}
	}

	processStateChangedCommand(commandName, state) {
		if (commandName === '.uno:StyleApply') {
			if (!state)
				return;

			// For impress documents, no styles is supported.
			if (this.map.getDocType() === 'presentation')
				return;

			this.stylesSelectValue = state;
			var container = document.getElementById('styles');
			if (container && container.onSetText)
				container.onSetText(state);
		}

		window.processStateChangedCommand(commandName, state);
	}

	onCommandStateChanged(e) {
		this.processStateChangedCommand(e.commandName, e.state);
	}
};

JSDialog.TopToolbar = function (map) {
	return new TopToolbar(map);
};
