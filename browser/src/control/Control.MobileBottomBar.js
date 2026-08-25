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
 * JSDialog.MobileBottomBar - component of bottom bar on mobile
 */

/* global app JSDialog _ _UNO */
class MobileBottomBar extends JSDialog.Toolbar {
	constructor(map) {
		super(map, 'MobileBottomBar', 'toolbar-down')

		map.on('commandstatechanged', window.onCommandStateChanged);
		map.on('updatetoolbarcommandvalues', window.onCommandStateChanged);
		app.events.on('contextchange', this.onContextChange.bind(this));
	}

	getToolItems() {
		if (this.docType == 'text') {
			return [
				{type: 'customtoolitem', id: 'showsearchbar', w2icon: 'search', text: _('Show the search bar')},
				{type: 'separator'},
				// context: ['default', 'Text', 'DrawText', 'Table']
				{type: 'toolitem', id: 'bold', text: _UNO('.uno:Bold'), command: '.uno:Bold', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'toolitem', id: 'italic', text: _UNO('.uno:Italic'), command: '.uno:Italic', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'toolitem', id: 'underline', text: _UNO('.uno:Underline'), command: '.uno:Underline', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'separator', id: 'breakcolor', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'customtoolitem', id: 'fontcolor', text: _UNO('.uno:FontColor'), lockUno: '.uno:FontColor', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'customtoolitem', id: 'backcolor', text: _UNO('.uno:CharBackColor', 'text'), lockUno: '.uno:CharBackColor', context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'menubutton', id: 'setborderstyle:BorderStyleMenu', noLabel: true, command: '.uno:SetBorderStyle', text: _('Borders'), hidden: true, context: ['Table']},
				{type: 'separator', id: 'breakalign-text', context: ['default', 'Text', 'DrawText']},
				{type: 'menubutton', id: 'aligntext', noLabel: true, command: 'alignblock', text: _UNO('.uno:TextAlign'), lockUno: '.uno:TextAlign', context: ['default', 'Text'],
					menu: [
						{type: 'comboboxentry', id: 'alignleft', text: _UNO('.uno:LeftPara', 'text', true), uno: '.uno:LeftPara'},
						{type: 'comboboxentry', id: 'centerpara', text: _UNO('.uno:CenterPara', 'text', true), uno: '.uno:CenterPara'},
						{type: 'comboboxentry', id: 'alignright', text: _UNO('.uno:RightPara', 'text', true), uno: '.uno:RightPara'},
						{type: 'comboboxentry', id: 'alignblock', text: _UNO('.uno:JustifyPara', 'text', true), uno: '.uno:JustifyPara'},
					]},
				{type: 'separator', id: 'breakalign-table', context: ['Table']},
				{type: 'menubutton', id: 'aligntable', noLabel: true, command: 'alignblock', text: _UNO('.uno:TextAlign'), lockUno: '.uno:TextAlign', context: ['Table', 'DrawText'],
					menu: [
						{type: 'comboboxentry', id: 'alignleft', text: _UNO('.uno:LeftPara', 'text', true), uno: '.uno:LeftPara'},
						{type: 'comboboxentry', id: 'centerpara', text: _UNO('.uno:CenterPara', 'text', true), uno: '.uno:CenterPara'},
						{type: 'comboboxentry', id: 'alignright', text: _UNO('.uno:RightPara', 'text', true), uno: '.uno:RightPara'},
						{type: 'comboboxentry', id: 'alignblock', text: _UNO('.uno:JustifyPara', 'text', true), uno: '.uno:JustifyPara'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'cellverttop', text: _UNO('.uno:CellVertTop', 'text', true), uno: '.uno:CellVertTop'},
						{type: 'comboboxentry', id: 'cellvertcenter', text: _UNO('.uno:CellVertCenter', 'text', true), uno: '.uno:CellVertCenter'},
						{type: 'comboboxentry', id: 'cellvertbottom', text: _UNO('.uno:CellVertBottom', 'text', true), uno: '.uno:CellVertBottom'},
					]},
				{type: 'toolitem', id: 'defaultnumbering-text', text: _UNO('.uno:DefaultNumbering', '', true), command: '.uno:DefaultNumbering', disabled: true, context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'toolitem', id: 'defaultbullet-text', text: _UNO('.uno:DefaultBullet', '', true), command: '.uno:DefaultBullet', disabled: true, context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'toolitem', id: 'incrementindent-text', text: _UNO('.uno:IncrementIndent', '', true), command: '.uno:IncrementIndent', disabled: true, context: ['default', 'Text', 'DrawText', 'Table']},
				{type: 'toolitem', id: 'decrementindent-text', text: _UNO('.uno:DecrementIndent', '', true), command: '.uno:DecrementIndent', disabled: true, context: ['default', 'Text', 'DrawText', 'Table']},
				// context: ['Table']
				{type: 'separator', context: ['Table']},
				{type: 'menubutton', id: 'insertrowsbefore', noLabel: true, text: _UNO('.uno:InsertRowsBefore'), lockUno: '.uno:InsertRowsBefore', context: ['Table'],
					menu: [
						{type: 'comboboxentry', id: 'insertrowsbefore', text: _UNO('.uno:InsertRowsBefore', 'text', true), uno: '.uno:InsertRowsBefore'},
						{type: 'comboboxentry', id: 'insertrowsafter', text: _UNO('.uno:InsertRowsAfter', 'text', true), uno: '.uno:InsertRowsAfter'},
						{type: 'comboboxentry', id: 'deleterows', text: _UNO('.uno:DeleteRows', 'text', true), uno: '.uno:DeleteRows'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'entirecell', text: _UNO('.uno:EntireCell', 'text', true), uno: '.uno:EntireCell'},
						{type: 'comboboxentry', id: 'entirerow', text: _UNO('.uno:EntireRow', 'text', true), uno: '.uno:EntireRow'},
						{type: 'comboboxentry', id: 'selecttable', text: _UNO('.uno:SelectTable', 'text', true), uno: '.uno:SelectTable'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'setoptimalrowheight', text: _UNO('.uno:SetOptimalRowHeight', 'text', true), command: '.uno:SetOptimalRowHeight'},
					]},
				{type: 'menubutton', id: 'insertcolumnsbefore', noLabel: true, text: _UNO('.uno:InsertColumnsBefore'), lockUno: '.uno:InsertColumnsBefore', context: ['Table'],
					menu: [
						{type: 'comboboxentry', id: 'insertcolumnsbefore', text: _UNO('.uno:InsertColumnsBefore', 'text', true), uno: '.uno:InsertColumnsBefore'},
						{type: 'comboboxentry', id: 'insertcolumnsafter', text: _UNO('.uno:InsertColumnsAfter', 'text', true), uno: '.uno:InsertColumnsAfter'},
						{type: 'comboboxentry', id: 'deletecolumns', text: _UNO('.uno:DeleteColumns', 'text', true), uno: '.uno:DeleteColumns'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'entirecell', text: _UNO('.uno:EntireCell', 'text', true), uno: '.uno:EntireCell'},
						{type: 'comboboxentry', id: 'entirecolumn', text: _UNO('.uno:EntireColumn', 'text', true), uno: '.uno:EntireColumn'},
						{type: 'comboboxentry', id: 'selecttable', text: _UNO('.uno:SelectTable', 'text', true), uno: '.uno:SelectTable'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'setoptimalcolumnwidth', text: _UNO('.uno:SetOptimalColumnWidth', 'text', true), uno: '.uno:SetOptimalColumnWidth'},
					]},
				{type: 'toolitem', id: 'togglemergecells', text: _UNO('.uno:ToggleMergeCells', 'text', true), command: '.uno:MergeCells', context: ['Table']},
				// context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']
				{type: 'menubutton', id: 'wrapmenu', noLabel: true, text: _UNO('.uno:WrapMenu'), lockUno: '.uno:WrapMenu', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork'],
					menu: [
						{type: 'comboboxentry', id: 'wrapoff', text: _UNO('.uno:WrapOff', 'text', true), uno: '.uno:WrapOff'},
						{type: 'comboboxentry', id: 'wrapon', text: _UNO('.uno:WrapOn', 'text', true), uno: '.uno:WrapOn'},
						{type: 'comboboxentry', id: 'wrapideal', text: _UNO('.uno:WrapIdeal', 'text', true), uno: '.uno:WrapIdeal'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'wrapleft', text: _UNO('.uno:WrapLeft', 'text', true), uno: '.uno:WrapLeft'},
						{type: 'comboboxentry', id: 'wrapright', text: _UNO('.uno:WrapRight', 'text', true), uno: '.uno:WrapRight'},
						{type: 'comboboxentry', id: 'wrapthrough', text: _UNO('.uno:WrapThrough', 'text', true), uno: '.uno:WrapThrough'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'wrapthroughtransparencytoggle', text: _UNO('.uno:WrapThroughTransparencyToggle', 'text', true), uno: '.uno:WrapThroughTransparencyToggle'},
						{type: 'comboboxentry', id: 'wrapcontour', text: _UNO('.uno:WrapContour', 'text', true), uno: '.uno:WrapContour'},
						{type: 'comboboxentry', id: 'wrapanchoronly', text: _UNO('.uno:WrapAnchorOnly', 'text', true), uno: '.uno:WrapAnchorOnly'},
					]},
				{type: 'separator', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']},
				{type: 'menubutton', id: 'aligncenter', noLabel: true, text: _UNO('.uno:AlignCenter'), lockUno: '.uno:AlignCenter', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork'],
					menu: [
						{type: 'comboboxentry', id: 'objectalignleft', text: _UNO('.uno:ObjectAlignLeft', 'text', true), uno: '.uno:ObjectAlignLeft'},
						{type: 'comboboxentry', id: 'aligncenter', text: _UNO('.uno:AlignCenter', 'text', true), uno: '.uno:AlignCenter'},
						{type: 'comboboxentry', id: 'objectalignright', text: _UNO('.uno:ObjectAlignRight', 'text', true), uno: '.uno:ObjectAlignRight'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'alignup', text: _UNO('.uno:AlignUp', 'text', true), uno: '.uno:AlignUp'},
						{type: 'comboboxentry', id: 'alignmiddle', text: _UNO('.uno:AlignMiddle', 'text', true), uno: '.uno:AlignMiddle'},
						{type: 'comboboxentry', id: 'aligndown', text: _UNO('.uno:AlignDown', 'text', true), uno: '.uno:AlignDown'},
					]},
				{type: 'menubutton', id: 'arrangemenu', noLabel: true, text: _UNO('.uno:ArrangeMenu'), lockUno: '.uno:ArrangeMenu', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork'],
					menu: [
						{type: 'comboboxentry', id: 'bringtofront', text: _UNO('.uno:BringToFront', 'text', true), uno: '.uno:BringToFront'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'objectforwardone', text: _UNO('.uno:ObjectForwardOne', 'text', true), uno: '.uno:ObjectForwardOne'},
						{type: 'comboboxentry', id: 'objectbackone', text: _UNO('.uno:ObjectBackOne', 'text', true), uno: '.uno:ObjectBackOne'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'sendtoback', text: _UNO('.uno:SendToBack', 'text', true), uno: '.uno:SendToBack'},
					]},
				{type: 'toolitem', id: 'flipvertical', text: _UNO('.uno:FlipVertical', 'text', true), command: '.uno:FlipVertical', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']},
				{type: 'toolitem', id: 'fliphorizontal', text: _UNO('.uno:FlipHorizontal', 'text', true), command: '.uno:FlipHorizontal', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']},
				{type: 'toolitem', id: 'crop', text: _UNO('.uno:Crop'), command: '.uno:Crop', context: ['Graphic']},
			];
		} else if (this.docType == 'spreadsheet') {
			return [
				{type: 'customtoolitem', id: 'showsearchbar', w2icon: 'search', text: _('Show the search bar')},
				{type: 'separator'},
				{type: 'toolitem', id: 'bold', text: _UNO('.uno:Bold'), command: '.uno:Bold'},
				{type: 'toolitem', id: 'italic', text: _UNO('.uno:Italic'), command: '.uno:Italic'},
				{type: 'toolitem', id: 'underline', text: _UNO('.uno:Underline'), command: '.uno:Underline'},
				{type: 'separator'},
				{type: 'customtoolitem', id: 'fontcolor', text: _UNO('.uno:FontColor'), lockUno: '.uno:FontColor'},
				{type: 'customtoolitem', id: 'backcolor', text: _UNO('.uno:BackgroundColor'), lockUno: '.uno:BackgroundColor'},
				{type: 'separator'},
				{type: 'menubutton', id: 'textalign', noLabel: true, command: 'alignblock', text: _UNO('.uno:TextAlign'), lockUno: '.uno:TextAlign',
					menu: [
						{type: 'comboboxentry', id: 'alignleft', text: _UNO('.uno:AlignLeft', 'spreadsheet', true), uno: '.uno:AlignLeft'},
						{type: 'comboboxentry', id: 'alignhorizontalcenter', text: _UNO('.uno:AlignHorizontalCenter', 'spreadsheet', true), uno: '.uno:AlignHorizontalCenter'},
						{type: 'comboboxentry', id: 'alignright', text: _UNO('.uno:AlignRight', 'spreadsheet', true), uno: '.uno:AlignRight'},
						{type: 'comboboxentry', id: 'alignblock', text: _UNO('.uno:AlignBlock', 'spreadsheet', true), uno: '.uno:AlignBlock'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'aligntop', text: _UNO('.uno:AlignTop', 'spreadsheet', true), uno: '.uno:AlignTop'},
						{type: 'comboboxentry', id: 'alignvcenter', text: _UNO('.uno:AlignVCenter', 'spreadsheet', true), uno: '.uno:AlignVCenter'},
						{type: 'comboboxentry', id: 'alignbottom', text: _UNO('.uno:AlignBottom', 'spreadsheet', true), uno: '.uno:AlignBottom'},
					]},
				{type: 'toolitem', id: 'wraptext', text: _UNO('.uno:WrapText', 'spreadsheet', true), command: '.uno:WrapText', disabled: true},
				{type: 'separator'},
				{type: 'menubutton', id: 'insertrowsbefore', noLabel: true, text: _UNO('.uno:InsertRowsBefore'), lockUno: '.uno:InsertRowsBefore',
					menu: [
						{type: 'comboboxentry', id: 'insertrowsbefore', text: _UNO('.uno:InsertRowsBefore', 'spreadsheet', true), uno: '.uno:InsertRowsBefore'},
						{type: 'comboboxentry', id: 'insertrowsafter', text: _UNO('.uno:InsertRowsAfter', 'spreadsheet', true), uno: '.uno:InsertRowsAfter'},
						{type: 'comboboxentry', id: 'deleterows', text: _UNO('.uno:DeleteRows', 'spreadsheet', true), uno: '.uno:DeleteRows'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'hiderow', text: _UNO('.uno:HideRow', 'spreadsheet', true), uno: '.uno:HideRow'},
						{type: 'comboboxentry', id: 'showrow', text: _UNO('.uno:ShowRow', 'spreadsheet', true), uno: '.uno:ShowRow'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'freezepanesrow', text: _UNO('.uno:FreezePanesRow', 'spreadsheet', true), uno: '.uno:FreezePanesRow'},
					]},
				{type: 'menubutton', id: 'insertcolumnsbefore', noLabel: true, text: _UNO('.uno:InsertColumnsBefore'), lockUno: '.uno:InsertColumnsBefore',
					menu: [
						{type: 'comboboxentry', id: 'insertcolumnsbefore', text: _UNO('.uno:InsertColumnsBefore', 'spreadsheet', true), uno: '.uno:InsertColumnsBefore'},
						{type: 'comboboxentry', id: 'insertcolumnsafter', text: _UNO('.uno:InsertColumnsAfter', 'spreadsheet', true), uno: '.uno:InsertColumnsAfter'},
						{type: 'comboboxentry', id: 'deletecolumns', text: _UNO('.uno:DeleteColumns', 'spreadsheet', true), uno: '.uno:DeleteColumns'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'hidecolumn', text: _UNO('.uno:HideColumn', 'spreadsheet', true), uno: '.uno:HideColumn'},
						{type: 'comboboxentry', id: 'showcolumn', text: _UNO('.uno:ShowColumn', 'spreadsheet', true), uno: '.uno:ShowColumn'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'freezepanescolumn', text: _UNO('.uno:FreezePanesColumn', 'spreadsheet', true), uno: '.uno:FreezePanesColumn'},
					]},
				{type: 'toolitem', id: 'togglemergecells', text: _UNO('.uno:ToggleMergeCells', 'spreadsheet', true), command: '.uno:ToggleMergeCells', disabled: true},
				{type: 'separator'},
				{type: 'menubutton', id: 'conditionalformatdialog:ConditionalFormatMenu', noLabel: true, text: _UNO('.uno:ConditionalFormatMenu', 'spreadsheet', true), command: '.uno:ConditionalFormatMenu'},
				{type: 'toolitem', id: 'sortascending', text: _UNO('.uno:SortAscending', 'spreadsheet', true), command: '.uno:SortAscending'},
				{type: 'toolitem', id: 'sortdescending', text: _UNO('.uno:SortDescending', 'spreadsheet', true), command: '.uno:SortDescending'},
				{type: 'separator'},
				{type: 'menubutton', id: 'numberformatstandard', noLabel: true, text: _UNO('.uno:NumberFormatStandard'), lockUno: '.uno:NumberFormatStandard',
					menu: [
						{type: 'comboboxentry', id: 'numberformatstandard', text: _UNO('.uno:NumberFormatStandard', 'spreadsheet', true), uno: '.uno:NumberFormatStandard'},
						{type: 'comboboxentry', id: 'numberformatdecimal', text: _UNO('.uno:NumberFormatDecimal', 'spreadsheet', true), uno: '.uno:NumberFormatDecimal'},
						{type: 'comboboxentry', id: 'numberformatpercent', text: _UNO('.uno:NumberFormatPercent', 'spreadsheet', true), uno: '.uno:NumberFormatPercent'},
						{type: 'comboboxentry', id: 'numberformatcurrency', text: _UNO('.uno:NumberFormatCurrency', 'spreadsheet', true), uno: '.uno:NumberFormatCurrency'},
						{type: 'comboboxentry', id: 'numberformatdate', text: _UNO('.uno:NumberFormatDate', 'spreadsheet', true), uno: '.uno:NumberFormatDate'},
						{type: 'comboboxentry', id: 'numberformattime', text: _UNO('.uno:NumberFormatTime', 'spreadsheet', true), uno: '.uno:NumberFormatTime'},
						{type: 'comboboxentry', id: 'numberformatscientific', text: _UNO('.uno:NumberFormatScientific', 'spreadsheet', true), uno: '.uno:NumberFormatScientific'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'numberformatthousands', text: _UNO('.uno:NumberFormatThousands', 'spreadsheet', true), uno: '.uno:NumberFormatThousands'},
					]},
				{type: 'toolitem', id: 'numberformatincdecimals', text: _UNO('.uno:NumberFormatIncDecimals', 'spreadsheet', true), command: '.uno:NumberFormatIncDecimals', disabled: true},
				{type: 'toolitem', id: 'numberformatdecdecimals', text: _UNO('.uno:NumberFormatDecDecimals', 'spreadsheet', true), command: '.uno:NumberFormatDecDecimals', disabled: true},
				{type: 'toolitem', id: 'crop', text: _UNO('.uno:Crop'), command: '.uno:Crop', context: ['Graphic']},
			];
		} else if ((this.docType == 'presentation') || (this.docType == 'drawing')) {
			return [
				{type: 'customtoolitem', id: 'showsearchbar', w2icon: 'search', text: _('Show the search bar')},
				{type: 'separator'},
				// context: ['default', 'DrawPage']
				{type: 'toolitem', id: 'insertpage', text: _UNO('.uno:InsertPage', 'presentation', true), command: '.uno:InsertPage', context: ['default', 'DrawPage']},
				{type: 'toolitem', id: 'duplicatepage', text: _UNO('.uno:DuplicatePage', 'presentation', true), command: '.uno:DuplicatePage', context: ['default', 'DrawPage']},
				{type: 'toolitem', id: 'deletepage', text: _UNO('.uno:DeletePage', 'presentation', true), command: '.uno:DeletePage', context: ['default', 'DrawPage']},
				{type: 'separator', context: ['default', 'DrawPage']},
				{type: 'customtoolitem', id: 'fullscreen', command: 'fullscreen-' + this.docType, text: _UNO('.uno:FullScreen', this.docType), context: ['default', 'DrawPage']},
				// context: ['Text', 'DrawText', 'TextObject', 'Table']
				{type: 'toolitem', id: 'bold', text: _UNO('.uno:Bold'), command: '.uno:Bold', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'toolitem', id: 'italic', text: _UNO('.uno:Italic'), command: 'Italic', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'toolitem', id: 'underline', text: _UNO('.uno:Underline'), command: '.uno:Underline', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'separator', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'customtoolitem', id: 'fontcolor', text: _UNO('.uno:FontColor'), lockUno: '.uno:FontColor', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'customtoolitem', id: 'backcolor', text: _UNO('.uno:BackgroundColor'), lockUno: '.uno:BackgroundColor', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'separator', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'menubutton', id: 'textalign', noLabel: true, command: 'alignblock', text: _UNO('.uno:TextAlign'), lockUno: '.uno:TextAlign', context: ['Text', 'DrawText', 'TextObject', 'Table'],
					menu: [
						{type: 'comboboxentry', id: 'alignleft', text: _UNO('.uno:LeftPara', '', true), uno: '.uno:LeftPara'},
						{type: 'comboboxentry', id: 'centerpara', text: _UNO('.uno:CenterPara', '', true), uno: '.uno:CenterPara'},
						{type: 'comboboxentry', id: 'alignright', text: _UNO('.uno:RightPara', '', true), uno: '.uno:RightPara'},
						{type: 'comboboxentry', id: 'alignblock', text: _UNO('.uno:JustifyPara', '', true), uno: '.uno:JustifyPara'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'cellverttop', text: _UNO('.uno:CellVertTop', '', true), uno: '.uno:CellVertTop'},
						{type: 'comboboxentry', id: 'cellvertcenter', text: _UNO('.uno:CellVertCenter', '', true), uno: '.uno:CellVertCenter'},
						{type: 'comboboxentry', id: 'cellvertbottom', text: _UNO('.uno:CellVertBottom', '', true), uno: '.uno:CellVertBottom'},
					]},
				{type: 'separator', context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'toolitem', id: 'defaultnumbering-text', text: _UNO('.uno:DefaultNumbering', '', true), command: '.uno:DefaultNumbering', disabled: true, context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'toolitem', id: 'defaultbullet', text: _UNO('.uno:DefaultBullet', '', true), command: '.uno:DefaultBullet', disabled: true, context: ['Text', 'DrawText', 'TextObject', 'Table']},
				{type: 'separator', context: ['Table']},
				{type: 'menubutton', id: 'insertrowsbefore', noLabel: true, text: _UNO('.uno:InsertRowsBefore'), lockUno: '.uno:InsertRowsBefore', context: ['Table'],
					menu: [
						{type: 'comboboxentry', id: 'insertrowsbefore', text: _UNO('.uno:InsertRowsBefore', 'presentation', true), uno: '.uno:InsertRowsBefore'},
						{type: 'comboboxentry', id: 'insertrowsafter', text: _UNO('.uno:InsertRowsAfter', 'presentation', true), uno: '.uno:InsertRowsAfter'},
						{type: 'comboboxentry', id: 'deleterows', text: _UNO('.uno:DeleteRows', 'presentation', true), uno: '.uno:DeleteRows'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'entirerow', text: _UNO('.uno:EntireRow', 'presentation', true), uno: '.uno:EntireRow'},
						{type: 'comboboxentry', id: 'selecttable', text: _UNO('.uno:SelectTable', 'presentation', true), uno: '.uno:SelectTable'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'setoptimalrowheight', text: _UNO('.uno:SetOptimalRowHeight', 'presentation', true), uno: '.uno:SetOptimalRowHeight'},
					]},
				{type: 'menubutton', id: 'insertcolumnsbefore', noLabel: true, text: _UNO('.uno:InsertColumnsBefore'), lockUno: '.uno:InsertColumnsBefore', context: ['Table'],
					menu: [
						{type: 'comboboxentry', id: 'insertcolumnsbefore', text: _UNO('.uno:InsertColumnsBefore', 'presentation', true), uno: '.uno:InsertColumnsBefore'},
						{type: 'comboboxentry', id: 'insertcolumnsafter', text: _UNO('.uno:InsertColumnsAfter', 'presentation', true), uno: '.uno:InsertColumnsAfter'},
						{type: 'comboboxentry', id: 'deletecolumns', text: _UNO('.uno:DeleteColumns', 'presentation', true), uno: '.uno:DeleteColumns'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'entirecolumn', text: _UNO('.uno:EntireColumn', 'presentation', true), uno: '.uno:EntireColumn'},
						{type: 'comboboxentry', id: 'selecttable', text: _UNO('.uno:SelectTable', 'presentation', true), uno: '.uno:SelectTable'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'setoptimalcolumnwidth', text: _UNO('.uno:SetOptimalColumnWidth', 'presentation', true), uno: '.uno:SetOptimalColumnWidth'},
					]},
				{type: 'toolitem', id: 'togglemergecells', text: _UNO('.uno:ToggleMergeCells', 'presentation', true), command: '.uno:ToggleMergeCells', context: ['Table']},
				// context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']
				{type: 'menubutton', id: 'aligncenter', noLabel: true, text: _UNO('.uno:AlignCenter'), lockUno: '.uno:AlignCenter', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork'],
					menu: [
						{type: 'comboboxentry', id: 'objectalignleft', text: _UNO('.uno:ObjectAlignLeft', '', true), uno: '.uno:ObjectAlignLeft'},
						{type: 'comboboxentry', id: 'aligncenter', text: _UNO('.uno:AlignCenter', '', true), uno: '.uno:AlignCenter'},
						{type: 'comboboxentry', id: 'objectalignright', text: _UNO('.uno:ObjectAlignRight', '', true), uno: '.uno:ObjectAlignRight'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'alignup', text: _UNO('.uno:AlignUp', '', true), uno: '.uno:AlignUp'},
						{type: 'comboboxentry', id: 'alignmiddle', text: _UNO('.uno:AlignMiddle', '', true), uno: '.uno:AlignMiddle'},
						{type: 'comboboxentry', id: 'aligndown', text: _UNO('.uno:AlignDown', '', true), uno: '.uno:AlignDown'},
					]},
				{type: 'menubutton', id: 'arrangemenu', noLabel: true, text: _UNO('.uno:ArrangeMenu'), lockUno: '.uno:ArrangeMenu', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork'],
					menu: [
						{type: 'comboboxentry', id: 'bringtofront', text: _UNO('.uno:BringToFront', '', true), uno: '.uno:BringToFront'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'objectforwardone', text: _UNO('.uno:ObjectForwardOne', '', true), uno: '.uno:ObjectForwardOne'},
						{type: 'comboboxentry', id: 'objectbackone', text: _UNO('.uno:ObjectBackOne', '', true), uno: '.uno:ObjectBackOne'},
						{type: 'separator'},
						{type: 'comboboxentry', id: 'sendtoback', text: _UNO('.uno:SendToBack', '', true), uno: '.uno:SendToBack'},
					]},
				{type: 'toolitem', id: 'flipvertical', text: _UNO('.uno:FlipVertical', '', true), command: '.uno:FlipVertical', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']},
				{type: 'toolitem', id: 'fliphorizontal', text: _UNO('.uno:FlipHorizontal', '', true), command: '.uno:FlipHorizontal', context: ['Draw', 'DrawLine', '3DObject', 'MultiObject', 'Graphic', 'DrawFontwork']},
				{type: 'toolitem', id: 'crop', text: _UNO('.uno:Crop'), command: '.uno:Crop', context: ['Graphic']},
			];
		}
	}

	onContextChange(event) {
		this.updateVisibilityForToolbar(event.detail.context);
	}
}

JSDialog.MobileBottomBar = function (map) {
	return new MobileBottomBar(map);
};
