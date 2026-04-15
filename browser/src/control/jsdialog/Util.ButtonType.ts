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
 * JSDialog.ButtonType - button type utilities for Uno tool buttons.
 */

declare var JSDialog: any;

function isOverFlowButtonId(id: string) {
	return id.startsWith('overflow-button');
}

function getToggleButtons() {
	return [
		'.uno:AccessibilityCheck',
		'.uno:AlignCenter',
		'.uno:AlignDown',
		'.uno:AlignHorizontalCenter',
		'.uno:AlignLeft',
		'.uno:AlignMiddle',
		'.uno:AlignRight',
		'.uno:AlignUp',
		'.uno:Bold',
		'.uno:CenterPara',
		'.uno:CharBackgroundExt',
		'.uno:ControlCodes',
		'.uno:DefaultBullet',
		'.uno:DefaultNumbering',
		'.uno:DocumentRepair',
		'.uno:EditDoc',
		'.uno:FormatPaintbrush',
		'.uno:invertbackground',
		'.uno:Italic',
		'.uno:JustifyPara',
		'.uno:LeftPara',
		'.uno:ModifiedStatus',
		'.uno:ObjectAlignLeft',
		'.uno:ObjectAlignRight',
		'.uno:OnlineAutoFormat',
		'.uno:OutlineFont',
		'.uno:ParaLeftToRight',
		'.uno:ParaRightToLeft',
		'.uno:RightPara',
		'.uno:Shadowed',
		'.uno:showannotations',
		'.uno:ShowResolvedAnnotations',
		'.uno:showruler',
		'.uno:showstatusbar',
		'.uno:ShowTrackedChanges',
		'.uno:Sidebar',
		'.uno:SidebarDeck.PropertyDeck',
		'.uno:SidebarDeck.StyleListDeck',
		'.uno:SpacePara1',
		'.uno:SpacePara15',
		'.uno:SpacePara2',
		'.uno:SpellOnline',
		'.uno:Strikeout',
		'.uno:SubScript',
		'.uno:SuperScript',
		'.uno:TrackChanges',
		'.uno:TrackChangesInAllViews',
		'.uno:TrackChangesInThisView',
		'.uno:Underline',
		'comparechanges',
		'showannotations',
		'toggledarktheme',
	];
}

function getDialogButtons() {
	return [
		'.uno:About',
		'.uno:AcceptTrackedChanges',
		'.uno:ChangeAlignment',
		'.uno:ChangeDistance',
		'.uno:ChangeFont',
		'.uno:ChangeFontSize',
		'.uno:ChapterNumberingDialog',
		'.uno:ContentControlProperties',
		'.uno:EditRegion',
		'.uno:FontDialog',
		'.uno:FootnoteDialog',
		'.uno:FormatColumns',
		'.uno:FormatLine',
		'.uno:ForumHelp',
		'.uno:InsertAnnotation',
		'.uno:InsertBookmark',
		'.uno:InsertBreak',
		'.uno:InsertFieldCtrl',
		'.uno:InsertFrame',
		'.uno:InsertIndexesEntry',
		'.uno:InsertMultiIndex',
		'.uno:InsertQrCode',
		'.uno:InsertReferenceField',
		'.uno:InsertSection',
		'.uno:KeyboardShortcuts',
		'.uno:LineNumberingDialog',
		'.uno:NameGroup',
		'.uno:ObjectTitleDescription',
		'.uno:OnlineHelp',
		'.uno:OutlineBullet',
		'.uno:PageDialog',
		'.uno:PageNumberWizard',
		'.uno:ParagraphDialog',
		'.uno:Print',
		'.uno:ReportIssue',
		'.uno:SearchDialog?InitialFocusReplace:bool=true',
		'.uno:SetDocumentProperties',
		'.uno:SetOutline',
		'.uno:Settings',
		'.uno:Signature',
		'.uno:Spacing',
		'.uno:SpellingAndGrammarDialog',
		'.uno:SplitCell',
		'.uno:SplitTable',
		'.uno:TableDialog',
		'.uno:TableNumberFormatDialog',
		'.uno:TableSort',
		'.uno:ThemeDialog',
		'.uno:ThesaurusDialog',
		'.uno:TitlePageDialog',
		'.uno:TransformDialog',
		'.uno:UpdateCurIndex',
		'.uno:Watermark',
		'.uno:WordCountDialog',
		'charmapcontrol',
		'hyperlinkdialog',
		'remotelink',
		'renamedocument',
		'serveraudit',
	];
}

function getDropdownButtons() {
	return [
		'.uno:AutoSumMenu',
		'.uno:BasicShapes',
		'.uno:CharSpacing',
		'.uno:CompareDocuments',
		'.uno:ConditionalFormatMenu',
		'.uno:FormatMenu',
		'.uno:FormattingMarkMenu',
		'.uno:GrafContrast',
		'.uno:GrafLuminance',
		'.uno:GrafMode',
		'.uno:GrafTransparence',
		'.uno:InsertGraphic',
		'.uno:InsertTable',
		'.uno:LanguageMenu',
		'.uno:LineSpacing',
		'.uno:Paste',
		'.uno:SetBorderStyle',
		'.uno:XLineColor',
		'DateAndTimeFunctionsMenu',
		'downloadas',
		'exportas',
		'FinancialFunctionsMenu',
		'home-search',
		'LanguageStatusMenu',
		'LogicalFunctionsMenu',
		'LookupAndRefFunctionsMenu',
		'MathAndTrigFunctionsMenu',
		'MenuColumnWidth',
		'MenuMargins',
		'MenuOrientation',
		'MenuPageSizesCalc',
		'MenuPageSizesWriter',
		'MenuPrintRanges',
		'MenuRowHeight',
		'Menu Statistic',
		'MoreFunctionsMenu',
		'saveas',
		'StateTableCellMenu',
		'TextFunctionsMenu',
		'viewModeDropdownButton',
		'zoom',
	];
}

function getOverflowGroupDropdownButtons() {
	return [
		'.uno:AutoSumMenu',
		'.uno:ConditionalFormatMenu',
		'.uno:DataDataPilotRun',
		'.uno:DataFilterAutoFilter',
		'.uno:DefineDBName',
		'.uno:DrawText',
		'.uno:EntireCell',
		'.uno:FontDialog',
		'.uno:FormatBulletsMenu',
		'.uno:FormatGroup',
		'.uno:FreezePanes',
		'.uno:FullScreen',
		'.uno:GrafLuminance',
		'.uno:Group',
		'.uno:InsertAnnotation',
		'.uno:InsertCalcTable',
		'.uno:InsertFieldCtrl',
		'.uno:InsertFootnote',
		'.uno:InsertGraphic',
		'.uno:InsertMultiIndex',
		'.uno:InsertObjectChart',
		'.uno:InsertPagebreak',
		'.uno:InsertTable',
		'.uno:Paste',
		'.uno:ToggleSheetGrid',
		'charmapcontrol',
		'home-search',
		'MenuMargins',
		'zoomreset',
	];
}

function getOverflowGroupToggleButtons() {
	return ['.uno:DefaultBullet', '.uno:ObjectAlignLeft', '.uno:TrackChanges'];
}

function getOverflowGroupDialogButtons() {
	return ['.uno:InsertSection', '.uno:SplitCell'];
}

JSDialog.IsToggleButton = function (
	id: string,
	commandName: string,
	builder: any,
) {
	const toggleBtnCommands = isOverFlowButtonId(id)
		? getOverflowGroupToggleButtons()
		: getToggleButtons();

	// fallback: if any command is not included in above list
	const stateValue = builder.map['stateChangeHandler']
		? builder.map['stateChangeHandler'].getItemValue(commandName)
		: 'false';

	const isToggle = stateValue === 'false' || stateValue === 'true';

	return toggleBtnCommands.includes(commandName) || isToggle;
};

JSDialog.IsDialogButton = function (id: string, commandName: string) {
	const dialogBtnCommands = isOverFlowButtonId(id)
		? getOverflowGroupDialogButtons()
		: getDialogButtons();

	return dialogBtnCommands.includes(commandName);
};

JSDialog.IsDropdownButton = function (id: string, commandName: string) {
	const dropdownBtnCommands = isOverFlowButtonId(id)
		? getOverflowGroupDropdownButtons()
		: getDropdownButtons();

	return dropdownBtnCommands.includes(commandName);
};
