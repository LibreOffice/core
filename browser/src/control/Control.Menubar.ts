/* -*- js-indent-level: 8 -*- */
/* global app */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface MenuItem {
	id?: string;
	name?: string;
	uno?: string;
	// Tag a menu item as being for the uno command, but don't do anything.  If `uno` is set
	// this shouldn't be. It is used by `_getItemsForCommand()`.
	unoid?: string;
	type?: string;
	menu?: MenuItem[];
	disabled?: boolean;
	hidden?: boolean;
	mobileapp?: boolean;
	iosapp?: boolean;
	tablet?: boolean;
	tag?: string;
}

interface MenuStructure {
	id?: string;
	type: string;
	enabled: boolean;
	text?: string;
	command?: string;
	executionType?: string;
	data?: MenuItem;
	children: MenuStructure[];
	checked?: boolean;
}

interface SmartMenusOptions {
	hideOnClick?: boolean;
	showOnClick?: boolean;
	hideTimeout?: number;
	hideDuration?: number;
	hideFunction?: (() => void) | null;
	showDuration?: number;
	showFunction?: (() => void) | null;
	showTimeout?: number;
	collapsibleHideDuration?: number;
	collapsibleHideFunction?: (() => void) | null;
	subIndicatorsPos?: string;
	subIndicatorsText?: string;
}

interface JQuery {
	smartmenus(options?: SmartMenusOptions): JQuery;
	smartmenus(instanceMethod: string): void;
}

/**
 * Menubar control class.
 * This control initializes and manages the application’s menubar,
 * including building menus, binding events, and updating UI states.
 */
class Menubar extends window.L.Control {
	// TODO: Some mechanism to stop the need to copy duplicate menus (eg. Help, eg: mobiledrawing)
	options: {
        initial: MenuItem[];
        text: MenuItem[];
        presentation: MenuItem[];
        drawing: MenuItem[];
        spreadsheet: MenuItem[];
        math: string[];
        mobiletext: MenuItem[];
        mobilepresentation: MenuItem[];
        mobiledrawing: MenuItem[];
        mobilespreadsheet: MenuItem[];
        mobileInsertMenu: Record<string, MenuItem>;
        commandStates: Record<string, any>;
        allowedReadonlyMenus: string[];
        allowedViewModeCommands: string[];
        allowedViewModeActions: (string | (() => string | undefined))[];
        allowedRedlineManagementMenus: string[];
        allowedRedlineManagementModeCommands: string[];
        allowedRedlineManagementModeActions: string[];
    } = {
		initial: [
			{name: _UNO('.uno:PickList')},
			{name: _UNO('.uno:EditMenu')},
			{name: _UNO('.uno:ViewMenu')},
			{name: _UNO('.uno:InsertMenu')},
			{name: _UNO('.uno:ToolsMenu')}
		],
		text:  [
			{name: _UNO('.uno:PickList', 'text'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'text'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'text'), id: 'saveas', type: window.prefs.get('saveAsMode') === 'group' ? 'menu' : 'action', menu: [
					{name: _('ODF text document (.odt)'), id: 'saveas-odt', type: 'action'},
					{name: _('Word 2003 Document (.doc)'), id: 'saveas-doc', type: 'action'},
					{name: _('Word Document (.docx)'), id: 'saveas-docx', type: 'action'},
					{name: _('Rich Text (.rtf)'), id: 'saveas-rtf', type: 'action'},
				]},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'},
					{name: _('EPUB (.epub)'), id: 'exportas-epub', type: 'action'}
				]},
				{name: _('Rename Document'), id: 'renamedocument', type: 'action'},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id: 'downloadas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
					{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
					{name: _('ODF text document (.odt)'), id: 'downloadas-odt', type: 'action'},
					{name: _('Word 2003 Document (.doc)'), id: 'downloadas-doc', type: 'action'},
					{name: _('Word Document (.docx)'), id: 'downloadas-docx', type: 'action'},
					{name: _('Rich Text (.rtf)'), id: 'downloadas-rtf', type: 'action'},
					{name: _('EPUB (.epub)'), id: !window.ThisIsAMobileApp ? 'exportepub' : 'downloadas-epub', type: 'action'},
					{name: _('HTML file (.html)'), id: 'downloadas-html', type: 'action'}]},
				{name: _UNO('.uno:SetDocumentProperties', 'text'), uno: '.uno:SetDocumentProperties', id: 'properties'},
				{name: _UNO('.uno:Signature', 'text'), uno: '.uno:Signature', id: 'signature'},
				{name: _('Options'), id: 'settings-dialog', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'text'), unoid: '.uno:Print', id: 'print', type: 'action'},
				{name: _('Close document'), id: 'closedocument', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'text'), id: 'editmenu', type: 'menu', menu: [
				{name: _UNO('.uno:Undo', 'text'), uno: '.uno:Undo'},
				{name: _UNO('.uno:Redo', 'text'), uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Cut', 'text'), uno: '.uno:Cut'},
				{name: _UNO('.uno:Copy', 'text'), uno: '.uno:Copy'},
				{name: _UNO('.uno:Paste', 'text'), uno: '.uno:Paste'},
				{name: _UNO('.uno:PasteSpecial', 'text'), uno: '.uno:PasteSpecial'},
				{name: _UNO('.uno:SelectAll', 'text'), uno: '.uno:SelectAll'},
				{type: 'separator'},
				{uno: '.uno:SearchDialog', id: 'searchreplace'},
				{type: 'separator'},
				{name: _UNO('.uno:ChangesMenu', 'text'), id: 'changesmenu', type: 'menu', menu: [
					{uno: '.uno:TrackChanges'},
					{name: _('Compare Changes'), id: 'comparechanges', type: 'action'},
					{uno: '.uno:ShowTrackedChanges'},
					{type: 'separator'},
					{uno: '.uno:AcceptTrackedChanges'},
					{uno: '.uno:AcceptTrackedChange'},
					{uno: '.uno:AcceptTrackedChangeToNext'},
					{type: 'action', id: 'acceptalltrackedchanges', uno: '.uno:AcceptAllTrackedChanges'},
					{uno: '.uno:RejectTrackedChange'},
					{uno: '.uno:RejectTrackedChangeToNext'},
					{type: 'action', id: 'rejectalltrackedchanges', uno: '.uno:RejectAllTrackedChanges'},
					{uno: '.uno:ReinstateTrackedChange'},
					{uno: '.uno:PreviousTrackedChange'},
					{uno: '.uno:NextTrackedChange'}
				]},
				{type: 'separator'},
				{name: _UNO('.uno:GotoPage', 'text'), uno: '.uno:GotoPage'}
			]},
			{name: _UNO('.uno:ViewMenu', 'text'), id: 'view', type: 'menu',
			 menu: (window.mode.isTablet() ? [
					{name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[] : [
					{name: _UNO('.uno:FullScreen', 'text'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action'},
					{type: 'separator'},
					{name: _UNO('.uno:ZoomPlus', 'text'), unoid: '.uno:ZoomPlus', id: 'zoomin', type: 'action'},
					{name: _UNO('.uno:ZoomMinus', 'text'), unoid: '.uno:ZoomMinus', id: 'zoomout', type: 'action',},
					{name: _('Fit to Screen'), id: 'fitwidthzoom', type: 'action'},
				] as MenuItem[]).concat([
					{type: 'separator'},
					{name: _('Toggle UI Mode'), id: 'toggleuimode', type: 'action'},
					{name: _('Multi Page View'), id: 'multipageview', type: 'action'},
					{name: _('Show Ruler'), id: 'showruler', type: 'action'},
					{name: _('Show Status Bar'), id: 'showstatusbar', type: 'action'},
					{name: _('Hide Menu Bar'), id: 'togglemenubar', type: 'action'},
					{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
					{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
					{uno: '.uno:SidebarDeck.PropertyDeck', id: 'view-sidebar-property-deck', name: _UNO('.uno:Sidebar')},
					{uno: '.uno:SidebarDeck.StyleListDeck', name: _('Style list')},
					{uno: '.uno:Navigator', id: 'navigator'},
					{type: 'separator'},
					{name: _UNO('.uno:ShowAnnotations', 'text'), unoid: '.uno:ShowAnnotations', id: 'showannotations', type: 'action'},
					{name: _UNO('.uno:ShowResolvedAnnotations', 'text'), unoid: '.uno:ShowResolvedAnnotations', id: 'showresolved', type: 'action'},
					{uno: '.uno:ControlCodes'},
				])},
			{name: _UNO('.uno:InsertMenu', 'text'), id: 'insert', type: 'menu', menu: [
				{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
				{name: _UNO('.uno:InsertGraphic', 'text'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
				{name: _UNO('.uno:InsertAnnotation', 'text'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
				{uno: '.uno:InsertObjectChart'},
				{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
				{name: _UNO('.uno:DrawText'), uno: '.uno:DrawText'},
				{name: _UNO('.uno:InsertFrame', 'text'), uno: '.uno:InsertFrame'},
				{name: _UNO('.uno:VerticalText'), uno: '.uno:VerticalText'},
				{type: 'separator'},
				{uno: '.uno:InsertSection', id: 'insertsection'},
				{uno: '.uno:PageNumberWizard', id: 'pagenumberwizard'},
				{name: _UNO('.uno:InsertFieldCtrl', 'text'), type: 'menu', menu: [
					{uno: '.uno:InsertPageNumberField'},
					{uno: '.uno:InsertPageCountField'},
					{uno: '.uno:InsertDateField'},
					{uno: '.uno:InsertTimeField'},
					{uno: '.uno:InsertTitleField'},
					{uno: '.uno:InsertAuthorField'},
					{uno: '.uno:InsertTopicField'},
					{type: 'separator'},
					{uno: '.uno:InsertField'},
				]},
				{name: _UNO('.uno:InsertHeaderFooterMenu', 'text'), type: 'menu', menu: [
					{name: _UNO('.uno:InsertPageHeader', 'text'), type: 'menu', menu: [
						{name: _('All'), disabled: true, id: 'insertheader', tag: '_ALL_', uno: '.uno:InsertPageHeader?'}]},
					{name: _UNO('.uno:InsertPageFooter', 'text'), type: 'menu', menu: [
						{name: _('All'), disabled: true, id: 'insertfooter', tag: '_ALL_', uno: '.uno:InsertPageFooter?'}]}
				]},
				{name: _UNO('.uno:InsertFootnote', 'text'), uno: '.uno:InsertFootnote'},
				{name: _UNO('.uno:InsertEndnote', 'text'), uno: '.uno:InsertEndnote'},
				{type: 'separator'},
				{uno: '.uno:InsertPagebreak'},
				{uno: '.uno:InsertBreak'},
				{name: _UNO('.uno:InsertColumnBreak', 'spreadsheet'), uno: '.uno:InsertColumnBreak'},
				{type: 'separator'},
				{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
				{name: _('Smart Picker'), id: 'remotelink', type: 'action'},
				{name: _('AI Assistant'), id: 'remoteaicontent', type: 'action'},
				{type: 'separator'},
				{uno: '.uno:InsertQrCode'},
				{uno: '.uno:InsertSymbol'},
				{uno: '.uno:InsertObjectStarMath'},
				{name: _UNO('.uno:FormattingMarkMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:InsertNonBreakingSpace'},
					{uno: '.uno:InsertHardHyphen'},
					{uno: '.uno:InsertSoftHyphen'},
					{uno: '.uno:InsertZWSP'},
					{uno: '.uno:InsertWJ'},
					{uno: '.uno:InsertLRM'},
					{uno: '.uno:InsertRLM'}]},
			]},
			{name: _UNO('.uno:FormatMenu', 'text'), id: 'format', type: 'menu', menu: [
				{name: _UNO('.uno:FormatTextMenu', 'text'), type: 'menu', menu: [
					{name: _UNO('.uno:Bold', 'text'), uno: '.uno:Bold'},
					{name: _UNO('.uno:Italic', 'text'), uno: '.uno:Italic'},
					{name: _UNO('.uno:Underline', 'text'), uno: '.uno:Underline'},
					{name: _UNO('.uno:UnderlineDouble', 'text'), uno: '.uno:UnderlineDouble'},
					{name: _UNO('.uno:Strikeout', 'text'), uno: '.uno:Strikeout'},
					{uno: '.uno:Overline'},
					{type: 'separator'},
					{name: _UNO('.uno:SuperScript', 'text'), uno: '.uno:SuperScript'},
					{name: _UNO('.uno:SubScript', 'text'), uno: '.uno:SubScript'},
					{type: 'separator'},
					{uno: '.uno:Shadowed'},
					{uno: '.uno:OutlineFont'},
					{type: 'separator'},
					{uno: '.uno:Grow'},
					{uno: '.uno:Shrink'},
					{type: 'separator'},
					{uno: '.uno:ChangeCaseToUpper'},
					{uno: '.uno:ChangeCaseToLower'},
					{uno: '.uno:ChangeCaseRotateCase'},
					{type: 'separator'},
					{uno: '.uno:ChangeCaseToSentenceCase'},
					{uno: '.uno:ChangeCaseToTitleCase'},
					{uno: '.uno:ChangeCaseToToggleCase'},
					{type: 'separator'},
					{uno: '.uno:SmallCaps'}]},
				{name: _('Text orientation'), type: 'menu', menu: [
					{uno: '.uno:ParaLeftToRight'},
					{uno: '.uno:ParaRightToLeft'}]},
				{name: _UNO('.uno:FormatSpacingMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:SpacePara1'},
					{uno: '.uno:SpacePara115'},
					{uno: '.uno:SpacePara15'},
					{uno: '.uno:SpacePara2'},
					{type: 'separator'},
					{uno: '.uno:ParaspaceIncrease'},
					{uno: '.uno:ParaspaceDecrease'},
					{type: 'separator'},
					{uno: '.uno:IncrementIndent'},
					{uno: '.uno:DecrementIndent'}]},
				{name: _UNO('.uno:TextAlign', 'text'), type: 'menu', menu: [
					{name: _UNO('.uno:CommonAlignLeft', 'text'), uno: '.uno:CommonAlignLeft'},
					{name: _UNO('.uno:CommonAlignHorizontalCenter', 'text'), uno: '.uno:CommonAlignHorizontalCenter'},
					{name: _UNO('.uno:CommonAlignRight', 'text'), uno: '.uno:CommonAlignRight'},
					{name: _UNO('.uno:CommonAlignJustified', 'text'), uno: '.uno:CommonAlignJustified'},
					{type: 'separator'},
					{uno: '.uno:CommonAlignTop'},
					{uno: '.uno:CommonAlignVerticalCenter'},
					{uno: '.uno:CommonAlignBottom'}]},
				{name: _UNO('.uno:FormatBulletsMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:DefaultBullet'},
					{uno: '.uno:DefaultNumbering'},
					{type: 'separator'},
					{uno: '.uno:DecrementLevel'},
					{uno: '.uno:IncrementLevel'},
					{uno: '.uno:DecrementSubLevels'},
					{uno: '.uno:IncrementSubLevels'},
					{type: 'separator'},
					{uno: '.uno:MoveDown'},
					{uno: '.uno:MoveUp'},
					{uno: '.uno:MoveDownSubItems'},
					{uno: '.uno:MoveUpSubItems'},
					{type: 'separator'},
					{uno: '.uno:InsertNeutralParagraph'},
					{uno: '.uno:NumberingStart'},
					{uno: '.uno:RemoveBullets'},
					{type: 'separator'},
					{uno: '.uno:JumpDownThisLevel'},
					{uno: '.uno:JumpUpThisLevel'},
					{uno: '.uno:ContinueNumbering'}]},
				{type: 'separator'},
				{uno: '.uno:FormatPaintbrush'},
				{uno: '.uno:ResetAttributes'},
				{type: 'separator'},
				{uno: '.uno:FontDialog'},
				{uno: '.uno:ParagraphDialog'},
				{uno: '.uno:SidebarDeck.StyleListDeck'},
				{uno: '.uno:OutlineBullet'},
				{uno: '.uno:ThemeDialog'},
				{type: 'separator'},
				{uno: '.uno:PageDialog'},
				{uno: '.uno:TitlePageDialog'},
				{uno: '.uno:FormatColumns'},
				{uno: '.uno:Watermark'},
				{uno: '.uno:EditRegion'},
				{type: 'separator'},
				{uno: '.uno:TransformDialog'},
				{uno: '.uno:FormatLine'},
				{uno: '.uno:FormatArea'},
				{uno: '.uno:NameGroup'},
				{uno: '.uno:ObjectTitleDescription'},
				{type: 'separator'},
				{uno: '.uno:ChangeFont'},
				{uno: '.uno:ChangeFontSize'},
				{uno: '.uno:ChangeDistance'},
				{uno: '.uno:ChangeAlignment'}
			]},
			{name: _('References'), id: 'references', type: 'menu', menu: [
				{name: _UNO('.uno:IndexesMenu', 'text'), uno: '.uno:InsertMultiIndex'},
				{uno: '.uno:InsertIndexesEntry'},
				{name: _('Update Index'), uno: '.uno:UpdateCurIndex'},
				{type: 'separator'},
				{uno: '.uno:InsertFootnote'},
				{uno: '.uno:InsertEndnote'},
				{uno: '.uno:FootnoteDialog'},
				{type: 'separator'},
				{uno: '.uno:InsertBookmark'},
				{uno: '.uno:InsertReferenceField'},
				{id: 'zoteroseparator', type: 'separator', hidden: !window.zoteroEnabled},
				{name: _('Add Citation'), id: 'zoteroaddeditcitation', type: 'action', hidden: !window.zoteroEnabled},
				{name: _('Add Citation Note'), id: 'zoteroaddnote', type: 'action', hidden: !window.zoteroEnabled},
				{name: _('Add Bibliography'), id: 'zoteroaddeditbibliography', type: 'action', hidden: !window.zoteroEnabled},
				{id: 'zoteroseparator2', type: 'separator', hidden: !window.zoteroEnabled},
				{name: _('Refresh Citations'), id: 'zoterorefresh', type: 'action', hidden: !window.zoteroEnabled},
				{name: _('Unlink Citations'), id: 'zoterounlink', type: 'action', hidden: !window.zoteroEnabled},
				{name: _('Citation Preferences'), id: 'zoterosetdocprefs', type: 'action', iosapp: false, hidden: !window.zoteroEnabled}]
			},
			{name: _UNO('.uno:TableMenu', 'text'), type: 'menu', id: 'table', menu: [
				{uno: '.uno:InsertTable'},
				{name: _UNO('.uno:TableInsertMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:InsertRowsBefore'},
					{uno: '.uno:InsertRowsAfter'},
					{type: 'separator'},
					{uno: '.uno:InsertColumnsBefore'},
					{uno: '.uno:InsertColumnsAfter'}]},
				{name: _UNO('.uno:TableDeleteMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:DeleteRows'},
					{uno: '.uno:DeleteColumns'},
					{uno: '.uno:DeleteTable'}]},
				{name: _UNO('.uno:TableSelectMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:SelectTable'},
					{uno: '.uno:EntireRow'},
					{uno: '.uno:EntireColumn'},
					{uno: '.uno:EntireCell'}]},
				{uno: '.uno:SplitCell'},
				{uno: '.uno:MergeCells'},
				{type: 'separator'},
				{uno: '.uno:Protect'},
				{uno: '.uno:UnsetCellsReadOnly'},
				{type: 'separator'},
				{uno: '.uno:TableDialog'}
			]},
			{name: _UNO('.uno:FormatFormMenu', 'text'), id: 'form', type: 'menu', menu: [
				{name: _('Insert Rich Text'), uno: '.uno:InsertContentControl'},
				{name: _('Insert Checkbox'), uno: '.uno:InsertCheckboxContentControl'},
				{name: _('Insert Dropdown'), uno: '.uno:InsertDropdownContentControl'},
				{name: _('Insert Picture'), uno: '.uno:InsertPictureContentControl'},
				{name: _('Insert Date'), uno: '.uno:InsertDateContentControl'},
				{name: _('Properties'), uno: '.uno:ContentControlProperties'},
			]},
			{name: _UNO('.uno:ToolsMenu', 'text'), id: 'tools', type: 'menu', menu: [
				{uno: '.uno:SpellingAndGrammarDialog'},
				{uno: '.uno:SpellOnline'},
				window.deeplEnabled ?
					{
						uno: '.uno:Translate'
					}: {},
				{uno: '.uno:ThesaurusDialog'},
				{name: _UNO('.uno:LanguageMenu'), type: 'menu', menu: [
					{name: _UNO('.uno:SetLanguageSelectionMenu', 'text'), type: 'menu', menu: [
						{name: _('None (Do not check spelling)'), id: 'noneselection', uno: '.uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE'} ]},
					{name: _UNO('.uno:SetLanguageParagraphMenu', 'text'), type: 'menu', menu: [
						{name: _('None (Do not check spelling)'), id: 'noneparagraph', uno: '.uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE'} ]},
					{name: _UNO('.uno:SetLanguageAllTextMenu', 'text'), type: 'menu', menu: [
						{name: _('None (Do not check spelling)'), id: 'nonelanguage', uno: '.uno:LanguageStatus?Language:string=Default_LANGUAGE_NONE'}]}
				]},
				{uno: '.uno:WordCountDialog'},
				window.enableAccessibility ?
					{name: _('Screen Reading'), id: 'togglea11ystate', type: 'action'} : {},
				{uno: '.uno:SidebarDeck.A11yCheckDeck'},
				{type: 'separator'},
				{name: _UNO('.uno:AutoFormatMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:OnlineAutoFormat'}]},
				{type: 'separator'},
				{uno: '.uno:ChapterNumberingDialog'},
				{uno: '.uno:LineNumberingDialog'},
				{uno: '.uno:FootnoteDialog'},
				{type: 'separator'},
				{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'}
			]},
			{name: _UNO('.uno:HelpMenu', 'text'), id: 'help', type: 'menu', menu: [
				{name: _('Online Help'), id: 'online-help', type: 'action', iosapp: false},
				{type: 'separator', iosapp: false},
				{name: _('Keyboard shortcuts'), id: 'keyboard-shortcuts', type: 'action', iosapp: false},
				{type: 'separator'},
				{name: _('Forum'), id: 'forum', type: 'action'},
				{name: _('Report an issue'), id: 'report-an-issue', type: 'action', iosapp: false},
				{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
				{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('Server audit'), id: 'serveraudit', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('About'), id: 'about', type: 'action'}]
			},
			{name: _('Last modification'), id: 'last-mod', type: 'action', tablet: false}
		],

		presentation: [
			{name: _UNO('.uno:PickList', 'presentation'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'presentation'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'presentation'), id: 'saveas', type: window.prefs.get('saveAsMode') === 'group' ? 'menu' : 'action', menu: [
					{name: _('ODF presentation (.odp)'), id: 'saveas-odp', type: 'action'},
					{name: _('PowerPoint 2003 Presentation (.ppt)'), id: 'saveas-ppt', type: 'action'},
					{name: _('PowerPoint Presentation (.pptx)'), id: 'saveas-pptx', type: 'action'},
				]},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Save Comments'), id: 'savecomments', type: 'action'},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id: 'downloadas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
					{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
					{name: _('ODF presentation (.odp)'), id: 'downloadas-odp', type: 'action'},
					{name: _('PowerPoint 2003 Presentation (.ppt)'), id: 'downloadas-ppt', type: 'action'},
					{name: _('PowerPoint Presentation (.pptx)'), id: 'downloadas-pptx', type: 'action'},
					{name: _('HTML Document (.html)'), id: 'downloadas-html', type: 'action'},
					{name: _('Scalable Vector Graphics (.svg)'), id: 'downloadas-svg', type: 'action'},
					{name: _('Current slide as Bitmap (.bmp)'), id: 'downloadas-bmp', type: 'action'},
					{name: _('Current slide as Graphics Interchange Format (.gif)'), id: 'downloadas-gif', type: 'action'},
					{name: _('Current slide as Portable Network Graphics (.png)'), id: 'downloadas-png', type: 'action'},
					{name: _('Current slide as Tag Image File Format (.tiff)'), id: 'downloadas-tiff', type: 'action'},
				]},
				{name: _UNO('.uno:SetDocumentProperties', 'presentation'), uno: '.uno:SetDocumentProperties', id: 'properties'},
				{name: _UNO('.uno:Signature', 'presentation'), uno: '.uno:Signature', id: 'signature'},
				{name: _('Options'), id: 'settings-dialog', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'presentation'), id: 'print', type: 'menu', menu: [
					{name: _('Full Page Slides'), id: 'print', type: 'action'},
					{name: _('Notes Pages'), id: 'print-notespages' , type: 'action'},
				]},
				{name: _('Close document'), id: 'closedocument', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'presentation'), id: 'editmenu', type: 'menu', menu: [
				{name: _UNO('.uno:Undo', 'presentation'), uno: '.uno:Undo'},
				{name: _UNO('.uno:Redo', 'presentation'), uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Cut', 'presentation'), uno: '.uno:Cut'},
				{name: _UNO('.uno:Copy', 'presentation'), uno: '.uno:Copy'},
				{name: _UNO('.uno:Paste', 'presentation'), uno: '.uno:Paste'},
				{name: _UNO('.uno:PasteSpecial', 'presentation'), uno: '.uno:PasteSpecial'},
				{name: _UNO('.uno:SelectAll', 'presentation'), uno: '.uno:SelectAll'},
				{type: 'separator'},
				{uno: '.uno:SearchDialog', id: 'searchreplace'}
			]},
			{name: _UNO('.uno:ViewMenu', 'presentation'), id: 'view', type: 'menu',
			 menu: (window.mode.isTablet() ? [
					{name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[] : [
					{name: _UNO('.uno:FullScreen', 'presentation'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action'},
				   {type: 'separator'},
					{name: _UNO('.uno:ZoomPlus', 'presentation'), unoid: '.uno:ZoomPlus', id: 'zoomin', type: 'action'},
					{name: _UNO('.uno:ZoomMinus', 'presentation'), unoid: '.uno:ZoomMinus', id: 'zoomout', type: 'action'},
				   {name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[]).concat([
				   {type: 'separator'},
				   {uno: '.uno:GridVisible', name: _UNO('.uno:GridVisible')},
				   {uno: '.uno:GridUse', name: _UNO('.uno:GridUse')},
				   {type: 'separator'},
				   {name: _('Toggle UI Mode'), id: 'toggleuimode', type: 'action'},
				   {name: _('Show Ruler'), id: 'showruler', type: 'action'},
				   {name: _('Show Status Bar'), id: 'showstatusbar', type: 'action'},
				   {name: _('Notes View'), id: 'notesmode', type: 'action'},
				   {name: _('Hide Menu Bar'), id: 'togglemenubar', type: 'action'},
				   {name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				   {name: _('Invert Background'), id: 'invertbackground', type: 'action'},
				   {name: _('Master View'), uno: '.uno:SlideMasterPage'},
				   {uno: '.uno:SidebarDeck.PropertyDeck', name: _UNO('.uno:Sidebar')},
				   {uno: '.uno:Navigator', id: 'navigator'},
				   {type: 'separator'},
				   {uno: '.uno:ModifyPage'},
					 {name: _UNO('.uno:SlideChangeWindow', 'presentation', true), unoid: '.uno:SlideChangeWindow', id: 'transitiondeck', type: 'action'},
					 {uno: '.uno:MasterSlidesPanel'},
					 {uno: '.uno:CustomAnimation'}, // core version
				   //{name: _UNO('.uno:CustomAnimation', 'presentation', true), id: 'animationdeck', type: 'action'}, // online version
				])},
			{name: _UNO('.uno:InsertMenu', 'presentation'), id: 'insert', type: 'menu', menu: [
				{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
				{name: _UNO('.uno:InsertGraphic', 'presentation'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
				{name: _('Local Multimedia...'), id: 'insertmultimedia', type: 'action'},
				{name: _UNO('.uno:SelectBackground', 'presentation'), unoid: '.uno:SelectBackground', id: 'selectbackground', type: 'action'},
				{name: _UNO('.uno:InsertAnnotation', 'presentation'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
				{uno: '.uno:InsertObjectChart'},
				{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
				{name: _UNO('.uno:Text', 'presentation'), unoid: '.uno:Text', id: 'inserttextbox', type: 'action'},
				{name: _UNO('.uno:VerticalText'), uno: '.uno:VerticalText'},
				{type: 'separator'},
				{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
				{name: _('Smart Picker'), id: 'remotelink', type: 'action'},
				{name: _('AI Assistant'), id: 'remoteaicontent', type: 'action'},
				{type: 'separator'},
				{uno: '.uno:InsertSymbol'},
				{type: 'separator'},
				{uno: '.uno:HeaderAndFooter'},
				{type: 'separator'},
				{name: _UNO('.uno:InsertField', 'text'), id: 'insertfield', type: 'menu', menu: [
					{uno: '.uno:InsertDateFieldFix'},
					{uno: '.uno:InsertDateFieldVar'},
					{uno: '.uno:InsertTimeFieldFix'},
					{uno: '.uno:InsertTimeFieldVar'},
					{type: 'separator'},
					{name: _UNO('.uno:InsertSlideField', 'presentation'), uno: '.uno:InsertPageField'},
					{name: _UNO('.uno:InsertSlideTitleField', 'presentation'), uno: '.uno:InsertPageTitleField'},
					{name: _UNO('.uno:InsertSlidesField', 'presentation'), uno: '.uno:InsertPagesField'},
				]},
			]},
			{name: _UNO('.uno:FormatMenu', 'presentation'), id: 'format', type: 'menu', menu: [
				{uno: '.uno:FontDialog'},
				{uno: '.uno:ParagraphDialog'},
				{name: _UNO('.uno:SlideSetup', 'presentation'), uno: '.uno:PageSetup'},
				{type: 'separator'},
				{uno: '.uno:TransformDialog'},
				{uno: '.uno:FormatLine'},
				{uno: '.uno:FormatArea'},
				{uno: '.uno:NameGroup'},
				{uno: '.uno:ObjectTitleDescription'},
				{type: 'separator'},
				{uno: '.uno:OutlineBullet'},
				{uno: '.uno:ThemeDialog'}]
			},
			{name: _UNO('.uno:TableMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), id:'table', type: 'menu', menu: [
				{name: _UNO('.uno:InsertTable', 'text'), uno: '.uno:InsertTable'},
				{name: _UNO('.uno:TableInsertMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), type: 'menu', menu: [
					{uno: '.uno:InsertRowsBefore'},
					{uno: '.uno:InsertRowsAfter'},
					{type: 'separator'},
					{uno: '.uno:InsertColumnsBefore'},
					{uno: '.uno:InsertColumnsAfter'}]},
				{name: _UNO('.uno:TableDeleteMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), type: 'menu', menu: [
					{uno: '.uno:DeleteRows'},
					{uno: '.uno:DeleteColumns'},
					{uno: '.uno:DeleteTable'}]},
				{name: _UNO('.uno:TableSelectMenu', 'text'), type: 'menu', menu: [
					{uno: '.uno:SelectTable'},
					{uno: '.uno:EntireRow'},
					{uno: '.uno:EntireColumn'}]},
				{uno: '.uno:MergeCells'},
				{uno: '.uno:TableDialog'}]
			},
			{name: _UNO('.uno:SlideMenu', 'presentation'), id: 'slide', type: 'menu', menu: [
				{name: _UNO('.uno:InsertSlide', 'presentation'), unoid: '.uno:InsertSlide', id: 'insertpage', type: 'action'},
				{name: _UNO('.uno:DuplicateSlide', 'presentation'), unoid: '.uno:DuplicateSlide', id: 'duplicatepage', type: 'action'},
				{name: _UNO('.uno:DeleteSlide', 'presentation'), unoid: '.uno:DeleteSlide', id: 'deletepage', type: 'action'},
				{name: _UNO('.uno:ShowSlide', 'presentation'), unoid: '.uno:ShowSlide', id: 'showslide', type: 'action'},
				{name: _UNO('.uno:HideSlide', 'presentation'), unoid: '.uno:HideSlide', id: 'hideslide', type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:GotoSlide', 'presentation'), uno: '.uno:GotoPage'},
				{type: 'separator', id: 'fullscreen-presentation-separator'},
				{name: _('Fullscreen presentation'), id: 'fullscreen-presentation', type: 'action'},
				{name: _('Present current slide'), id: 'presentation-currentslide', type: 'action'},
				{name: _('Present in new window'), id: 'present-in-window', type: 'action'},
				...((!window.ThisIsAMobileApp || window.mode.isCODesktop()) ? [
					{name: _('Presenter Console'), id: 'presentation-in-console', type: 'action'}
				] : [])
			]
			},
			{name: _UNO('.uno:ToolsMenu', 'presentation'), id: 'tools', type: 'menu', menu: [
				{uno: '.uno:SpellDialog'},
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:LanguageMenu'), type: 'menu', menu: [
					{name: _('None (Do not check spelling)'), id: 'nonelanguage', uno: '.uno:LanguageStatus?Language:string=Default_LANGUAGE_NONE'}]},
				{type: 'separator'},
				{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'}

			]},
			{name: _UNO('.uno:HelpMenu', 'presentation'), id: 'help', type: 'menu', menu: [
				{name: _('Online Help'), id: 'online-help', type: 'action', iosapp: false},
				{type: 'separator', iosapp: false},
				{name: _('Keyboard shortcuts'), id: 'keyboard-shortcuts', type: 'action', iosapp: false},
				{type: 'separator'},
				{name: _('Forum'), id: 'forum', type: 'action'},
				{name: _('Report an issue'), id: 'report-an-issue', type: 'action', iosapp: false},
				{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
				{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('Server audit'), id: 'serveraudit', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('About'), id: 'about', type: 'action'}]
			},
			{name: _('Last modification'), id: 'last-mod', type: 'action', tablet: false}
		],

		drawing: [
			{name: _UNO('.uno:PickList', 'presentation'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'presentation'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'presentation'), unoid: '.uno:SaveAs', id: 'saveas', type: 'action'},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Save Comments'), id: 'savecomments', type: 'action'},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _UNO('.uno:Print', 'presentation'), unoid: '.uno:Print', id: 'print', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id: 'downloadas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
					{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
					{name: _('ODF Drawing (.odg)'), id: 'downloadas-odg', type: 'action'}
				]},
				{name: _UNO('.uno:SetDocumentProperties', 'presentation'), uno: '.uno:SetDocumentProperties', id: 'properties'},
				{name: _UNO('.uno:Signature', 'presentation'), uno: '.uno:Signature', id: 'signature'},
				{name: _('Options'), id: 'settings-dialog', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('Close document'), id: 'closedocument', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'presentation'), id: 'editmenu', type: 'menu', menu: [
				{name: _UNO('.uno:Undo', 'presentation'), uno: '.uno:Undo'},
				{name: _UNO('.uno:Redo', 'presentation'), uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Cut', 'presentation'), uno: '.uno:Cut'},
				{name: _UNO('.uno:Copy', 'presentation'), uno: '.uno:Copy'},
				{name: _UNO('.uno:Paste', 'presentation'), uno: '.uno:Paste'},
				{name: _UNO('.uno:PasteSpecial', 'presentation'), uno: '.uno:PasteSpecial'},
				{name: _UNO('.uno:SelectAll', 'presentation'), uno: '.uno:SelectAll'},
				{type: 'separator'},
				{uno: '.uno:SearchDialog', id: 'searchreplace'}
			]},
			{name: _UNO('.uno:ViewMenu', 'presentation'), id: 'view', type: 'menu',
			 menu: (window.mode.isTablet() ? [
					{name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[] : [
					{name: _UNO('.uno:FullScreen', 'presentation'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action'},
					{type: 'separator'},
					{name: _UNO('.uno:ZoomPlus', 'presentation'), unoid: '.uno:ZoomPlus', id: 'zoomin', type: 'action'},
					{name: _UNO('.uno:ZoomMinus', 'presentation'), unoid: '.uno:ZoomMinus', id: 'zoomout', type: 'action'},
					{name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[]).concat([
					{type: 'separator'},
					{uno: '.uno:GridVisible', name: _UNO('.uno:GridVisible')},
					{uno: '.uno:GridUse', name: _UNO('.uno:GridUse')},
					{type: 'separator'},
					{name: _('Toggle UI Mode'), id: 'toggleuimode', type: 'action'},
					{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
					{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
					{uno: '.uno:SidebarDeck.PropertyDeck', name: _UNO('.uno:Sidebar')},
					{uno: '.uno:Navigator', id: 'navigator'},
					{name: _('Show Status Bar'), id: 'showstatusbar', type: 'action'},
					{name: _('Hide Menu Bar'), id: 'togglemenubar', type: 'action'},
				])},
			{name: _UNO('.uno:InsertMenu', 'presentation'), id: 'insert', type: 'menu', menu: [
				{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
				{name: _UNO('.uno:InsertGraphic', 'presentation'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
				{name: _UNO('.uno:SelectBackground', 'presentation'), unoid: '.uno:SelectBackground', id: 'selectbackground', type: 'action'},
				{name: _UNO('.uno:InsertAnnotation', 'presentation'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
				{uno: '.uno:InsertObjectChart'},
				{type: 'separator'},
				{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
				{name: _('Smart Picker'), id: 'remotelink', type: 'action'},
				{name: _('AI Assistant'), id: 'remoteaicontent', type: 'action'},
				{type: 'separator'},
				{uno: '.uno:InsertSymbol'},
				{type: 'separator'},
				{name: _UNO('.uno:InsertField', 'text'), id: 'insertfield', type: 'menu', menu: [
					{name: _UNO('.uno:InsertDateFieldFix', 'presentation'), uno: '.uno:InsertDateFieldFix'},
					{name: _UNO('.uno:InsertDateFieldVar', 'presentation'), uno: '.uno:InsertDateFieldVar'},
					{name: _UNO('.uno:InsertTimeFieldFix', 'presentation'), uno: '.uno:InsertTimeFieldFix'},
					{name: _UNO('.uno:InsertTimeFieldVar', 'presentation'), uno: '.uno:InsertTimeFieldVar'},
					{type: 'separator'},
					{name: _UNO('.uno:InsertPageField', 'presentation'), uno: '.uno:InsertPageField'},
					{name: _UNO('.uno:InsertPageTitleField', 'presentation'), uno: '.uno:InsertPageTitleField'},
					{name: _UNO('.uno:InsertPagesField', 'presentation'), uno: '.uno:InsertPagesField'},
				]},
				{name: _UNO('.uno:InsertSignatureLine'), unoid: '.uno:InsertSignatureLine', id: 'insert-signatureline', type: 'action'},
			]},
			{name: _UNO('.uno:FormatMenu', 'presentation'), id: 'format', type: 'menu', menu: [
				{uno: '.uno:FontDialog'},
				{uno: '.uno:ParagraphDialog'},
				{name: _UNO('.uno:PageSetup', 'presentation'), uno: '.uno:PageSetup'},
				{type: 'separator'},
				{uno: '.uno:TransformDialog'},
				{uno: '.uno:FormatLine'},
				{uno: '.uno:FormatArea'},
				{uno: '.uno:NameGroup'},
				{uno: '.uno:ObjectTitleDescription'},
				{type: 'separator'},
				{uno: '.uno:OutlineBullet'},
				{uno: '.uno:ThemeDialog'}]
			},
			{name: _UNO('.uno:TableMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), id: 'table', type: 'menu', menu: [
				{name: _UNO('.uno:InsertTable', 'text'), uno: '.uno:InsertTable'},
				{name: _UNO('.uno:TableInsertMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), type: 'menu', menu: [
					{name: _UNO('.uno:InsertRowsBefore', 'presentation'), uno: '.uno:InsertRowsBefore'},
					{name: _UNO('.uno:InsertRowsAfter', 'presentation'), uno: '.uno:InsertRowsAfter'},
					{type: 'separator'},
					{name: _UNO('.uno:InsertColumnsBefore', 'presentation'), uno: '.uno:InsertColumnsBefore'},
					{name: _UNO('.uno:InsertColumnsAfter', 'presentation'), uno: '.uno:InsertColumnsAfter'}]},
				{name: _UNO('.uno:TableDeleteMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), type: 'menu', menu: [
					{name: _UNO('.uno:DeleteRows', 'presentation'), uno: '.uno:DeleteRows'},
					{name: _UNO('.uno:DeleteColumns', 'presentation'), uno: '.uno:DeleteColumns'},
					{name: _UNO('.uno:DeleteTable', 'presentation'), uno: '.uno:DeleteTable'}]},
				{name: _UNO('.uno:TableSelectMenu', 'text'), type: 'menu', menu: [
					{name: _UNO('.uno:SelectTable', 'presentation'), uno: '.uno:SelectTable'},
					{name: _UNO('.uno:EntireRow', 'presentation'), uno: '.uno:EntireRow'},
					{name: _UNO('.uno:EntireColumn', 'presentation'), uno: '.uno:EntireColumn'}]},
				{name: _UNO('.uno:MergeCells', 'presentation'), uno: '.uno:MergeCells'},
				{name: _UNO('.uno:TableDialog', 'presentation'), uno: '.uno:TableDialog'}]
			},
			{name: _UNO('.uno:PageMenu', 'presentation'), type: 'menu', menu: [
				{name: _UNO('.uno:InsertPage', 'presentation'), unoid: '.uno:InsertPage', id: 'insertpage', type: 'action'},
				{name: _UNO('.uno:DuplicatePage', 'presentation'), unoid: '.uno:DuplicatePage', id: 'duplicatepage', type: 'action'},
				{name: _UNO('.uno:DeletePage', 'presentation'), unoid: '.uno:DeletePage', id: 'deletepage', type: 'action'},
				{name: _UNO('.uno:GotoPage', 'presentation'), uno: '.uno:GotoPage'}]
			},
			{name: _UNO('.uno:ToolsMenu', 'presentation'), id: 'tools', type: 'menu', menu: [
				{uno: '.uno:SpellDialog'},
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:LanguageMenu'), type: 'menu', menu: [
					{name: _('None (Do not check spelling)'), id: 'nonelanguage', uno: '.uno:LanguageStatus?Language:string=Default_LANGUAGE_NONE'}]}
			]},
			{name: _UNO('.uno:HelpMenu', 'presentation'), id: 'help', type: 'menu', menu: [
				{name: _('Online Help'), id: 'online-help', type: 'action', iosapp: false},
				{type: 'separator', iosapp: false},
				{name: _('Keyboard shortcuts'), id: 'keyboard-shortcuts', type: 'action', iosapp: false},
				{type: 'separator'},
				{name: _('Forum'), id: 'forum', type: 'action'},
				{name: _('Report an issue'), id: 'report-an-issue', type: 'action', iosapp: false},
				{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
				{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('Server audit'), id: 'serveraudit', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('About'), id: 'about', type: 'action'}]
			},
			{name: _('Last modification'), id: 'last-mod', type: 'action', tablet: false}
		],

		spreadsheet: [
			{name: _UNO('.uno:PickList', 'spreadsheet'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'spreadsheet'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'spreadsheet'), id: 'saveas', type: window.prefs.get('saveAsMode') === 'group' ? 'menu' : 'action', menu: [
					{name: _('ODF spreadsheet (.ods)'), id: 'saveas-ods', type: 'action'},
					{name: _('Excel 2003 Spreadsheet (.xls)'), id: 'saveas-xls', type: 'action'},
					{name: _('Excel Spreadsheet (.xlsx)'), id: 'saveas-xlsx', type: 'action'},
				]},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id:'downloadas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
					{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
					{name: _('ODF spreadsheet (.ods)'), id: 'downloadas-ods', type: 'action'},
					{name: _('Excel 2003 Spreadsheet (.xls)'), id: 'downloadas-xls', type: 'action'},
					{name: _('Excel Spreadsheet (.xlsx)'), id: 'downloadas-xlsx', type: 'action'},
					{name: _('CSV file (.csv)'), id: 'downloadas-csv', type: 'action'},
					{name: _('HTML file (.html)'), id: 'downloadas-html', type: 'action'}]},
				{name: _UNO('.uno:SetDocumentProperties', 'spreadsheet'), uno: '.uno:SetDocumentProperties', id: 'properties'},
				{name: _UNO('.uno:Signature', 'spreadsheet'), uno: '.uno:Signature', id: 'signature'},
				{name: _('Options'), id: 'settings-dialog', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'spreadsheet'), id: 'print', type: 'menu', menu: [
					{name: _('Active sheet'), id: 'print-active-sheet', type: 'action'},
					{name: _('All Sheets'), id: 'print-all-sheets', type: 'action'},
				]},
				{name: _('Close document'), id: 'closedocument', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'spreadsheet'), id: 'editmenu', type: 'menu', menu: [
				{name: _UNO('.uno:Undo', 'text'), uno: '.uno:Undo'},
				{name: _UNO('.uno:Redo', 'text'), uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Cut', 'text'), uno: '.uno:Cut'},
				{name: _UNO('.uno:Copy', 'text'), uno: '.uno:Copy'},
				{name: _UNO('.uno:Paste', 'text'), uno: '.uno:Paste'},
				{name: _UNO('.uno:PasteSpecial', 'text'), uno: '.uno:PasteSpecial'},
				{name: _UNO('.uno:SelectAll', 'text'), uno: '.uno:SelectAll'},
				{type: 'separator'},
				{uno: '.uno:SearchDialog', id: 'searchreplace'}
			]},
			{name: _UNO('.uno:ViewMenu', 'spreadsheet'), id: 'view', type: 'menu',
			 menu: (window.mode.isTablet() ? [
					{name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[] : [
					{name: _UNO('.uno:FullScreen', 'spreadsheet'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action'},
				   {type: 'separator'},
				   {name: _UNO('.uno:ZoomPlus', 'text'), unoid: '.uno:ZoomPlus', id: 'zoomin', type: 'action'},
				   {name: _UNO('.uno:ZoomMinus', 'text'), unoid: '.uno:ZoomMinus', id: 'zoomout', type: 'action',},
				   {name: _('Reset zoom'), id: 'zoomreset', type: 'action'},
				] as MenuItem[]).concat([
				   {type: 'separator'},
				   {name: _('Toggle UI Mode'), id: 'toggleuimode', type: 'action'},
				   {name: _('Show Status Bar'), id: 'showstatusbar', type: 'action'},
				   {name: _('Hide Menu Bar'), id: 'togglemenubar', type: 'action'},
				   {name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				   {name: _('Invert Background'), id: 'invertbackground', type: 'action'},
				   {uno: '.uno:SidebarDeck.PropertyDeck', name: _UNO('.uno:Sidebar')},
				   {uno: '.uno:Navigator', id: 'navigator'},
				   {uno: '.uno:SidebarDeck.StyleListDeck', name: _('Style list')},
				   {type: 'separator'},
				   {name: _UNO('.uno:ToggleSheetGrid', 'spreadsheet', true), uno: '.uno:ToggleSheetGrid', id: 'sheetgrid'},
				   {name: _('Focus Cell'), type:'action', id: 'columnrowhighlight'},
				   {name: _UNO('.uno:FreezePanes', 'spreadsheet', true), id: 'FreezePanes', uno: '.uno:FreezePanes'},
				   {name: _UNO('.uno:FreezeCellsMenu', 'spreadsheet', true), id: 'FreezeCellsMenu', type: 'menu', uno: '.uno:FreezeCellsMenu', menu: [
					   {name: _UNO('.uno:FreezePanesColumn', 'spreadsheet', true), id: 'FreezePanesColumn', uno: '.uno:FreezePanesColumn'},
					   {name: _UNO('.uno:FreezePanesRow', 'spreadsheet', true), id: 'FreezePanesRow', uno: '.uno:FreezePanesRow'}
				   ]},
				])},
			{name: _UNO('.uno:InsertMenu', 'spreadsheet'), id: 'insert', type: 'menu', menu: [
				{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
				{name: _UNO('.uno:InsertGraphic', 'spreadsheet'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
				{name: _UNO('.uno:DataDataPilotRun', 'spreadsheet'), uno: '.uno:DataDataPilotRun'},
				{name: _UNO('.uno:CalculatedFieldRun', 'spreadsheet'), uno: '.uno:CalculatedFieldRun'},
				{name: _UNO('.uno:InsertCalcTable', 'spreadsheet'), uno: '.uno:InsertCalcTable'},
				{name: _UNO('.uno:InsertSparkline', 'spreadsheet'), uno: '.uno:InsertSparkline'},
				{name: _UNO('.uno:InsertAnnotation', 'spreadsheet'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
				{uno: '.uno:InsertObjectChart'},
				{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
				{name: _UNO('.uno:DrawText'), uno: '.uno:DrawText'},
				{name: _UNO('.uno:VerticalText'), uno: '.uno:VerticalText'},
				{uno: '.uno:FunctionDialog'},
				{type: 'separator'},
				{name: _UNO('.uno:HyperlinkDialog'), uno: '.uno:HyperlinkDialog'},
				{name: _('Smart Picker'), id: 'remotelink', type: 'action'},
				{name: _('AI Assistant'), id: 'remoteaicontent', type: 'action'},
				{uno: '.uno:InsertSymbol'},
				{type: 'separator'},
				{name: _UNO('.uno:InsertField', 'text'), type: 'menu', menu: [
					{uno: '.uno:InsertCurrentDate'},
					{uno: '.uno:InsertCurrentTime'}
				]},
				{uno: '.uno:EditHeaderAndFooter'} /*todo: add to Control.Notebookbar.Calc.js (as Insert tab)*/
			]},
			{name: _UNO('.uno:FormatMenu', 'spreadsheet'), id: 'format', type: 'menu', menu: [
				{name: _UNO('.uno:FormatTextMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _UNO('.uno:Bold', 'spreadsheet'), uno: '.uno:Bold'},
					{name: _UNO('.uno:Italic', 'spreadsheet'), uno: '.uno:Italic'},
					{name: _UNO('.uno:Underline', 'spreadsheet'), uno: '.uno:Underline'},
					{name: _UNO('.uno:UnderlineDouble', 'spreadsheet'), uno: '.uno:UnderlineDouble'},
					{name: _UNO('.uno:Strikeout', 'spreadsheet'), uno: '.uno:Strikeout'},
					{uno: '.uno:Overline'},
					{type: 'separator'},
					{name: _UNO('.uno:SuperScript', 'spreadsheet'), uno: '.uno:SuperScript'},
					{name: _UNO('.uno:SubScript', 'spreadsheet'), uno: '.uno:SubScript'},
					{type: 'separator'},
					{uno: '.uno:Shadowed'},
					{uno: '.uno:OutlineFont'},
					{type: 'separator'},
					{uno: '.uno:WrapText'},
					{type: 'separator'},
					{uno: '.uno:ChangeCaseToUpper'},
					{uno: '.uno:ChangeCaseToLower'},
					{uno: '.uno:ChangeCaseRotateCase'},
					{type: 'separator'},
					{uno: '.uno:ChangeCaseToSentenceCase'},
					{uno: '.uno:ChangeCaseToTitleCase'},
					{uno: '.uno:ChangeCaseToToggleCase'}]},
				{name: _UNO('.uno:TextAlign', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:CommonAlignLeft'},
					{uno: '.uno:CommonAlignHorizontalCenter'},
					{uno: '.uno:CommonAlignRight'},
					{uno: '.uno:CommonAlignJustified'},
					{type: 'separator'},
					{uno: '.uno:CommonAlignTop'},
					{uno: '.uno:CommonAlignVerticalCenter'},
					{uno: '.uno:CommonAlignBottom'},
					{type: 'separator'},
					{uno: '.uno:ParaLeftToRight'},
					{uno: '.uno:ParaRightToLeft'}]},
				{name: _UNO('.uno:NumberFormatMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:NumberFormatStandard'},
					{uno: '.uno:NumberFormatDecimal'},
					{uno: '.uno:NumberFormatPercent'},
					{uno: '.uno:NumberFormatCurrency'},
					{uno: '.uno:NumberFormatDate'},
					{uno: '.uno:NumberFormatTime'},
					{uno: '.uno:NumberFormatScientific'},
					{type: 'separator'},
					{uno: '.uno:NumberFormatThousands'}]},
				{type: 'separator'},
				{uno: '.uno:FormatPaintbrush'},
				{uno: '.uno:ResetAttributes'},
				{name: _UNO('.uno:PrintRangesMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:DefinePrintArea'},
					{uno: '.uno:AddPrintArea'},
					{uno: '.uno:EditPrintArea'},
					{uno: '.uno:DeletePrintArea'}]},
				{name: _UNO('.uno:FormatSparklineMenu', 'spreadsheet'), type: 'menu', menu: [
				    {uno: '.uno:InsertSparkline'},
				    {uno: '.uno:DeleteSparkline'},
				    {uno: '.uno:DeleteSparklineGroup'},
				    {uno: '.uno:EditSparklineGroup'},
				    {uno: '.uno:EditSparkline'},
				    {uno: '.uno:GroupSparklines'},
				    {uno: '.uno:UngroupSparklines'}
				]},
				{name: _UNO('.uno:ConditionalFormatMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _('Highlight cells with...'), type: 'menu', menu: [
						{name: _('Values greater than...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=2'},
						{name: _('Values less than...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=1'},
						{name: _('Values equal to...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=0'},
						{name: _('Values between...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=6'},
						{name: _('Values duplicate...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=8'},
						{name: _('Containing text...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=23'},
						{type: 'separator'},
						{name: _('More highlights...'), uno: '.uno:ConditionalFormatDialog'},
					]},
					{name: _('Top/Bottom Rules...'), type: 'menu', menu: [
						{name: _('Top N elements...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=11'},
						{name: _('Top N percent...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=13'},
						{name: _('Bottom N elements...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=12'},
						{name: _('Bottom N percent...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=14'},
						{name: _('Above Average...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=15'},
						{name: _('Below Average...'), uno: '.uno:ConditionalFormatEasy?FormatRule:short=16'},
						{type: 'separator'},
						{name: _('More highlights...'), uno: '.uno:ConditionalFormatDialog'},
					]},
					{uno: '.uno:ColorScaleFormatDialog'},
					{uno: '.uno:DataBarFormatDialog'},
					{uno: '.uno:IconSetFormatDialog'},
					{uno: '.uno:CondDateFormatDialog'},
					{type: 'separator'},
					{uno: '.uno:ConditionalFormatManagerDialog'}]},
				{type: 'separator'},
				{uno: '.uno:FormatCellDialog'},
				{name: _('Rows'), type: 'menu', menu: [
					{uno: '.uno:RowHeight'},
					{uno: '.uno:SetOptimalRowHeight'}]},
				{name: _('Columns'), type: 'menu', menu: [
					{uno: '.uno:ColumnWidth'},
					{uno: '.uno:SetOptimalColumnWidth'}]},
				{uno: '.uno:FontDialog'},
				{uno: '.uno:ParagraphDialog'},
				{uno: '.uno:SidebarDeck.StyleListDeck'},
				{uno: '.uno:PageFormatDialog'},
				{type: 'separator'},
				{uno: '.uno:TransformDialog'},
				{uno: '.uno:FormatLine'},
				{uno: '.uno:FormatArea'},
				{uno: '.uno:ThemeDialog'}
			]},
			{name: _UNO('.uno:SheetMenu', 'spreadsheet'), id: 'sheet', type: 'menu', menu: [
				{uno: '.uno:InsertCell'},
				{name: _UNO('.uno:InsertRowsMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:InsertRowsBefore'},
					{uno: '.uno:InsertRowsAfter'}]},
				{name: _UNO('.uno:InsertColumnsMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:InsertColumnsBefore'},
					{uno: '.uno:InsertColumnsAfter'}]},
				{type: 'separator'},
				{uno: '.uno:DeleteCell'},
				{uno: '.uno:DeleteRows'},
				{uno: '.uno:DeleteColumns'},
				{uno: '.uno:SheetRightToLeft'},
				{type: 'separator'},
				{name:  _UNO('.uno:JumpToTable', 'spreadsheet'), uno: '.uno:JumpToTable'}
			]},
			{name: _UNO('.uno:DataMenu', 'spreadsheet'), id: 'data', type: 'menu', menu: [
				{uno: '.uno:DataSort'},
				{uno: '.uno:SortAscending'},
				{uno: '.uno:SortDescending'},
				{uno: '.uno:Validation'},
				{uno: '.uno:Calculate'},
				{uno: '.uno:ConvertFormulaToValue'},
				{type: 'separator'},
				{uno: '.uno:DataFilterAutoFilter'},
				{name: _UNO('.uno:FilterMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:DataFilterStandardFilter'},
					{uno: '.uno:DataFilterSpecialFilter'},
					{type: 'separator'},
					{uno: '.uno:DataFilterRemoveFilter'},
					{uno: '.uno:DataFilterHideAutoFilter'}]},
				{type: 'separator'},
				{name: _UNO('.uno:DataPilotMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _UNO('.uno:InsertPivotTable', 'spreadsheet'), uno: '.uno:DataDataPilotRun'},
					{name: _UNO('.uno:InsertCalculation', 'spreadsheet'), uno: '.uno:CalculatedFieldRun'},
					{name: _UNO('.uno:RecalcPivotTable', 'spreadsheet'), uno: '.uno:RecalcPivotTable'},
					{name: _UNO('.uno:DeletePivotTable', 'spreadsheet'), uno: '.uno:DeletePivotTable'}]},
				{type: 'separator'},
				{name: _UNO('.uno:DataTableMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _UNO('.uno:RunInsCalcTable', 'spreadsheet'), uno: '.uno:InsertCalcTable'},
					{name: _UNO('.uno:RemoveCalcTable', 'spreadsheet'), uno: '.uno:RemoveCalcTable'}]},
				{type: 'separator'},
				{name: _UNO('.uno:NamesMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _UNO('.uno:AddName', 'spreadsheet'), uno: '.uno:AddName'},
					{name: _UNO('.uno:DefineName', 'spreadsheet'), uno: '.uno:DefineName'}]},
				{name: _UNO('.uno:DefineDBName', 'spreadsheet'), uno: '.uno:DefineDBName'},
				{name: _UNO('.uno:SelectDB', 'spreadsheet'), uno: '.uno:SelectDB'},
				{name: _UNO('.uno:DataAreaRefresh', 'spreadsheet'), uno: '.uno:DataAreaRefresh'},
				{type: 'separator'},
				{name: _UNO('.uno:GroupOutlineMenu', 'spreadsheet'), type: 'menu', menu: [
					{uno: '.uno:Group'},
					{uno: '.uno:Ungroup'},
					{type: 'separator'},
					{uno: '.uno:ClearOutline'},
					{type: 'separator'},
					{uno: '.uno:HideDetail'},
					{uno: '.uno:ShowDetail'}]},
				{type: 'separator'},
				{name: _UNO('.uno:StatisticsMenu', 'spreadsheet'), type: 'menu', menu: [
					{name: _UNO('.uno:SamplingDialog', 'spreadsheet'), uno: '.uno:SamplingDialog'},
					{name: _UNO('.uno:DescriptiveStatisticsDialog', 'spreadsheet'), uno: '.uno:DescriptiveStatisticsDialog'},
					{name: _UNO('.uno:AnalysisOfVarianceDialog', 'spreadsheet'), uno: '.uno:AnalysisOfVarianceDialog'},
					{name: _UNO('.uno:CorrelationDialog', 'spreadsheet'), uno: '.uno:CorrelationDialog'},
					{name: _UNO('.uno:CovarianceDialog', 'spreadsheet'), uno: '.uno:CovarianceDialog'},
					{name: _UNO('.uno:ExponentialSmoothingDialog', 'spreadsheet'), uno: '.uno:ExponentialSmoothingDialog'},
					{name: _UNO('.uno:MovingAverageDialog', 'spreadsheet'), uno: '.uno:MovingAverageDialog'},
					{name: _UNO('.uno:RegressionDialog', 'spreadsheet'), uno: '.uno:RegressionDialog'},
					{name: _UNO('.uno:TTestDialog', 'spreadsheet'), uno: '.uno:TTestDialog'},
					{name: _UNO('.uno:FTestDialog', 'spreadsheet'), uno: '.uno:FTestDialog'},
					{name: _UNO('.uno:ZTestDialog', 'spreadsheet'), uno: '.uno:ZTestDialog'},
					{name: _UNO('.uno:ChiSquareTestDialog', 'spreadsheet'), uno: '.uno:ChiSquareTestDialog'},
					{name: _UNO('.uno:FourierAnalysisDialog', 'spreadsheet'), uno: '.uno:FourierAnalysisDialog'}]},
			]},
			{name: _UNO('.uno:ToolsMenu', 'spreadsheet'), id: 'tools', type: 'menu', menu: [
				{uno: '.uno:SpellDialog'},
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:LanguageMenu'), type: 'menu', menu: [
					{name: _('None (Do not check spelling)'), id: 'nonelanguage', uno: '.uno:LanguageStatus?Language:string=Default_LANGUAGE_NONE'}]},
				{uno: '.uno:GoalSeekDialog'},
				{uno: '.uno:Protect'},
				{type: 'separator'},
				{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'}
			]},
			{name: _UNO('.uno:HelpMenu', 'spreadsheet'), id: 'help', type: 'menu', menu: [
				{name: _('Online Help'), id: 'online-help', type: 'action', iosapp: false},
				{type: 'separator', iosapp: false},
				{name: _('Keyboard shortcuts'), id: 'keyboard-shortcuts', type: 'action', iosapp: false},
				{type: 'separator'},
				{name: _('Forum'), id: 'forum', type: 'action'},
				{name: _('Report an issue'), id: 'report-an-issue', type: 'action', iosapp: false},
				{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
				{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('Server audit'), id: 'serveraudit', type: 'action', mobileapp: false},
				{type: 'separator'},
				{name: _('About'), id: 'about', type: 'action'}]
			},
			{name: _('Last modification'), id: 'last-mod', type: 'action', tablet: false}
		],

		mobiletext:  [
			{name: _('Search'), id: 'recsearch', type: 'action'},
			{name: _UNO('.uno:PickList', 'text'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'text'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'text'), unoid: '.uno:SaveAs', id: 'saveas', type: 'action'},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'},
					{name: _('EPUB (.epub)'), id: 'exportas-epub', type: 'action'}
				]},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'text'), unoid: '.uno:Print', id: 'print', type: 'action'},
			]},
			{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id: 'downloadas', type: 'menu', menu: [
				{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
				{name: _('PDF Document (.pdf) as...'), id: 'exportpdf', type: 'action'},
				{name: _('ODF text document (.odt)'), id: 'downloadas-odt', type: 'action'},
				{name: _('Word 2003 Document (.doc)'), id: 'downloadas-doc', type: 'action'},
				{name: _('Word Document (.docx)'), id: 'downloadas-docx', type: 'action'},
				{name: _('Rich Text (.rtf)'), id: 'downloadas-rtf', type: 'action'},
				{name: _('EPUB (.epub)'), id: !window.ThisIsAMobileApp ? 'exportepub' : 'downloadas-epub', type: 'action'},
			]},
			{name: _UNO('.uno:EditMenu', 'text'), id: 'editmenu', type: 'menu', menu: [
				{uno: '.uno:Undo'},
				{uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{uno: '.uno:Cut'},
				{uno: '.uno:Copy'},
				{uno: '.uno:Paste'},
				{uno: '.uno:SelectAll'}
			]},
			{name: _UNO('.uno:ChangesMenu', 'text'), id: 'changesmenu', type: 'menu', menu: [
				{uno: '.uno:TrackChanges'},
				{uno: '.uno:ShowTrackedChanges'},
				{type: 'separator'},
				{type: 'action', id: 'acceptalltrackedchanges', uno: '.uno:AcceptAllTrackedChanges'},
				{type: 'action', id: 'rejectalltrackedchanges', uno: '.uno:RejectAllTrackedChanges'},
				{uno: '.uno:PreviousTrackedChange'},
				{uno: '.uno:NextTrackedChange'}
			]},
			{name: _UNO('.uno:ViewMenu', 'text'), id: 'view', type: 'menu', menu: [
				{name: _UNO('.uno:FullScreen', 'text'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action', mobileapp: false},
				{uno: '.uno:ControlCodes', id: 'formattingmarks'},
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:ShowResolvedAnnotations', 'text'), id: 'showresolved', uno: '.uno:ShowResolvedAnnotations'},
				{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
			]
			},
			window.enableAccessibility ?
				{name: _('Screen Reading'), id: 'togglea11ystate', type: 'action'} : {},
			{id: 'watermark', uno: '.uno:Watermark'},
			{name: _('Page Setup'), id: 'pagesetup', type: 'action'},
			{uno: '.uno:WordCountDialog'},
			window.deeplEnabled ?
				{
					uno: '.uno:Translate'
				} : {},
			{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'},
			{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
			{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
			{name: _('About'), id: 'about', type: 'action'},
		],

		mobilepresentation: [
			{name: _('Search'), id: 'recsearch', type: 'action'},
			{name: _UNO('.uno:PickList', 'presentation'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'presentation'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'presentation'), unoid: '.uno:SaveAs', id: 'saveas', type: 'action'},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'presentation'), uno: '.uno:Print', id: 'print', type: 'action'},
			]},
			{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id:'downloadas', type: 'menu', menu: [
				{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
				{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
				{name: _('ODF presentation (.odp)'), id: 'downloadas-odp', type: 'action'},
				{name: _('PowerPoint 2003 Presentation (.ppt)'), id: 'downloadas-ppt', type: 'action'},
				{name: _('PowerPoint Presentation (.pptx)'), id: 'downloadas-pptx', type: 'action'},
				{name: _('ODF Drawing (.odg)'), id: 'downloadas-odg', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'presentation'), id: 'editmenu', type: 'menu', menu: [
				{uno: '.uno:Undo'},
				{uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{uno: '.uno:Cut'},
				{uno: '.uno:Copy'},
				{uno: '.uno:Paste'},
				{uno: '.uno:SelectAll'}
			]},
			{name: _UNO('.uno:ViewMenu', 'presentation'), id: 'view', type: 'menu', menu: [
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:FullScreen', 'presentation'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action', mobileapp: false},
				{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
			]
			},
			{name: _UNO('.uno:TableMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), id: 'tablemenu', type: 'menu', menu: [
				{uno: '.uno:InsertRowsBefore'},
				{uno: '.uno:InsertRowsAfter'},
				{type: 'separator'},
				{uno: '.uno:InsertColumnsBefore'},
				{uno: '.uno:InsertColumnsAfter'},
				{uno: '.uno:SelectTable'},
				{uno: '.uno:EntireRow'},
				{uno: '.uno:EntireColumn'},
				{uno: '.uno:MergeCells'},
				{uno: '.uno:DeleteRows'},
				{uno: '.uno:DeleteColumns'},
				{uno: '.uno:DeleteTable'},
			]
			},
			{name: _UNO('.uno:SlideMenu', 'presentation'), id: 'slidemenu', type: 'menu', menu: [
				{name: _UNO('.uno:InsertSlide', 'presentation'), unoid: '.uno:InsertSlide', id: 'insertpage', type: 'action'},
				{name: _UNO('.uno:DuplicateSlide', 'presentation'), unoid: '.uno:DuplicateSlide', id: 'duplicatepage', type: 'action'},
				{name: _UNO('.uno:DeleteSlide', 'presentation'), unoid: '.uno:DeleteSlide', id: 'deletepage', type: 'action'}]
			},
			{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'},
			{name: _('Fullscreen presentation'), id: 'fullscreen-presentation', type: 'action'},
			{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
			{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
			{name: _('About'), id: 'about', type: 'action'},
		],

		mobiledrawing: [
			{name: _('Search'), id: 'recsearch', type: 'action'},
			{name: _UNO('.uno:PickList', 'presentation'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'presentation'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'presentation'), unoid: '.uno:SaveAs', id: 'saveas', type: 'action'},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _UNO('.uno:Print', 'presentation'), unoid: '.uno:Print', id: 'print', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
			]},
			{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id:'downloadas', type: 'menu', menu: [
				{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
				{name: _('PDF Document (.pdf) as...'), id: 'exportpdf', type: 'action'},
				{name: _('ODF Drawing (.odg)'), id: 'downloadas-odg', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'presentation'), id: 'editmenu', type: 'menu', menu: [
				{uno: '.uno:Undo'},
				{uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{uno: '.uno:Cut'},
				{uno: '.uno:Copy'},
				{uno: '.uno:Paste'},
				{uno: '.uno:SelectAll'}
			]},
			{name: _UNO('.uno:ViewMenu', 'presentation'), id: 'view', type: 'menu', menu: [
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:FullScreen', 'presentation'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action', mobileapp: false},
				{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
			]
			},
			{name: _UNO('.uno:TableMenu', 'text'/*HACK should be 'presentation', but not in xcu*/), id: 'tablemenu', type: 'menu', menu: [
				{name: _UNO('.uno:InsertRowsBefore', 'presentation'), uno: '.uno:InsertRowsBefore'},
				{name: _UNO('.uno:InsertRowsAfter', 'presentation'), uno: '.uno:InsertRowsAfter'},
				{type: 'separator'},
				{name: _UNO('.uno:InsertColumnsBefore', 'presentation'), uno: '.uno:InsertColumnsBefore'},
				{name: _UNO('.uno:InsertColumnsAfter', 'presentation'), uno: '.uno:InsertColumnsAfter'},
				{name: _UNO('.uno:DeleteRows', 'presentation'), uno: '.uno:DeleteRows'},
				{name: _UNO('.uno:DeleteColumns', 'presentation'), uno: '.uno:DeleteColumns'},
				{name: _UNO('.uno:MergeCells', 'presentation'), uno: '.uno:MergeCells'}]
			},
			{name: _UNO('.uno:PageMenu', 'presentation'), id: 'pagemenu', type: 'menu', menu: [
				{name: _UNO('.uno:InsertPage', 'presentation'), unoid: '.uno:InsertPage', id: 'insertpage', type: 'action'},
				{name: _UNO('.uno:DuplicatePage', 'presentation'), unoid: '.uno:DuplicatePage', id: 'duplicatepage', type: 'action'},
				{name: _UNO('.uno:DeletePage', 'presentation'), unoid: '.uno:DeletePage', id: 'deletepage', type: 'action'}]
			},
			{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'},
			{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
			{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
			{name: _('About'), id: 'about', type: 'action'},
		],

		mobilespreadsheet: [
			{name: _('Search'), id: 'recsearch', type: 'action'},
			{name: _UNO('.uno:PickList', 'spreadsheet'), id: 'file', type: 'menu', menu: [
				{name: _UNO('.uno:Save', 'spreadsheet'), unoid: '.uno:Save', id: 'save', type: 'action'},
				{name: _UNO('.uno:SaveAs', 'spreadsheet'), unoid: '.uno:SaveAs', id: 'saveas', type: 'action'},
				{name: _('Export as'), id: 'exportas', type: 'menu', menu: [
					{name: _('PDF Document (.pdf)'), id: 'exportas-pdf', type: 'action'}
				]},
				{name: _('Share...'), id:'shareas', type: 'action'},
				{name: _('See revision history'), id: 'rev-history', type: 'action'},
				{type: 'separator'},
				{name: _UNO('.uno:Print', 'spreadsheet'), unoid: '.uno:Print', id: 'print', type: 'action'},
				{name: _('Define print area'), uno: '.uno:DefinePrintArea' },
				{name: _('Remove print area'), uno: '.uno:DeletePrintArea' },
			]},
			{name: !window.ThisIsAMobileApp ? _('Download as') : _('Export as'), id:'downloadas', type: 'menu', menu: [
				{name: _('PDF Document (.pdf)'), id: !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', type: 'action'},
				{name: _('PDF Document (.pdf) as...'), id: 'exportpdf' , type: 'action'},
				{name: _('ODF spreadsheet (.ods)'), id: 'downloadas-ods', type: 'action'},
				{name: _('Excel 2003 Spreadsheet (.xls)'), id: 'downloadas-xls', type: 'action'},
				{name: _('Excel Spreadsheet (.xlsx)'), id: 'downloadas-xlsx', type: 'action'}
			]},
			{name: _UNO('.uno:EditMenu', 'spreadsheet'), id: 'editmenu', type: 'menu', menu: [
				{uno: '.uno:Undo'},
				{uno: '.uno:Redo'},
				{name: _('Repair'), id: 'repair',  type: 'action'},
				{type: 'separator'},
				{uno: '.uno:Cut'},
				{uno: '.uno:Copy'},
				{uno: '.uno:Paste'},
				{uno: '.uno:SelectAll'}
			]},
			{name: _UNO('.uno:ViewMenu', 'spreadsheet'), id: 'view', type: 'menu', menu: [
				{uno: '.uno:SpellOnline'},
				{name: _UNO('.uno:FullScreen', 'presentation'), unoid: '.uno:FullScreen', id: 'fullscreen', type: 'action', mobileapp: false},
				{name: _('Dark Mode'), id: 'toggledarktheme', type: 'action'},
				{name: _('Invert Background'), id: 'invertbackground', type: 'action'},
			]
			},
			{name: _UNO('.uno:SheetMenu', 'spreadsheet'), id: 'sheetmenu', type: 'menu', menu: [
				{name: _UNO('.uno:InsertRowsMenu', 'spreadsheet'), id: 'insertrowsmenu', type: 'menu', menu: [
					{uno: '.uno:InsertRowsBefore'},
					{uno: '.uno:InsertRowsAfter'}]},
				{name: _UNO('.uno:InsertColumnsMenu', 'spreadsheet'), id: 'insertcolumnsmenu', type: 'menu', menu: [
					{uno: '.uno:InsertColumnsBefore'},
					{uno: '.uno:InsertColumnsAfter'}]},
				{type: 'separator'},
				{uno: '.uno:DeleteRows'},
				{uno: '.uno:DeleteColumns'},
				{type: 'separator'},
				{name: _UNO('.uno:FreezePanes', 'spreadsheet'), uno: '.uno:FreezePanes'},
				{name: _UNO('.uno:FreezePanesColumn', 'spreadsheet'), uno: '.uno:FreezePanesColumn'},
				{name: _UNO('.uno:FreezePanesRow', 'spreadsheet'), uno: '.uno:FreezePanesRow'}
			]},
			{name: _UNO('.uno:DataMenu', 'spreadsheet'), id: 'datamenu', type: 'menu', menu: [
				{uno: '.uno:Validation'},
				{type: 'separator'},
				{uno: '.uno:SortAscending'},
				{uno: '.uno:SortDescending'},
				{type: 'separator'},
				{name: _UNO('.uno:GroupOutlineMenu', 'spreadsheet'), id: 'groupoutlinemenu', type: 'menu', menu: [
					{uno: '.uno:Group'},
					{uno: '.uno:Ungroup'},
					{type: 'separator'},
					{uno: '.uno:ClearOutline'},
					{type: 'separator'},
					{uno: '.uno:HideDetail'},
					{uno: '.uno:ShowDetail'}]}
			]},
			{name: _UNO('.uno:RunMacro'), id: 'runmacro', uno: '.uno:RunMacro'},
			{name: _('Latest Updates'), id: 'latestupdates', type: 'action', iosapp: false},
			{name: _('Send Feedback'), id: 'feedback', type: 'action', mobileapp: false},
			{name: _('About'), id: 'about', type: 'action'},
		],

		mobileInsertMenu : {
			'text' : {
				name: _UNO('.uno:InsertMenu', 'text'), id: 'insert', type: 'menu', menu: [
					{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
					{name: _UNO('.uno:InsertGraphic', 'text'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
					{uno: '.uno:InsertObjectChart'},
					{name: _UNO('.uno:InsertAnnotation', 'text'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
					{name: _UNO('.uno:TableMenu'), unoid: '.uno:TableMenu', id: 'inserttable', type: 'action'},
					{type: 'separator'},
					{name: _UNO('.uno:InsertField', 'text'), id: 'insertfield', type: 'menu', menu: [
						{uno: '.uno:InsertPageNumberField'},
						{uno: '.uno:InsertPageCountField'},
						{uno: '.uno:InsertDateField'},
						{uno: '.uno:InsertTimeField'},
						{uno: '.uno:InsertTitleField'},
						{uno: '.uno:InsertAuthorField'},
						{uno: '.uno:InsertTopicField'}
					]},
					{name: _UNO('.uno:InsertHeaderFooterMenu', 'text'), id: 'insertheaderfooter', type: 'menu', menu: [
						{name: _UNO('.uno:InsertPageHeader', 'text'), id: 'insertpageheader', type: 'menu', menu: [
							{name: _('All'), disabled: true, id: 'insertheader', tag: '_ALL_', uno: '.uno:InsertPageHeader?On:bool=true'}]},
						{name: _UNO('.uno:InsertPageFooter', 'text'), id: 'insertpagefooter', type: 'menu', menu: [
							{name: _('All'), disabled: true, id: 'insertfooter', tag: '_ALL_', uno: '.uno:InsertPageFooter?On:bool=true'}]}
					]},
					{uno: '.uno:InsertFootnote'},
					{uno: '.uno:InsertEndnote'},
					{type: 'separator'},
					{uno: '.uno:InsertPagebreak'},
					{name: _UNO('.uno:InsertColumnBreak', 'spreadsheet'), uno: '.uno:InsertColumnBreak'},
					{type: 'separator'},
					{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
					{name: _UNO('.uno:ShapesMenu'), unoid: '.uno:ShapesMenu', id: 'insertshape', type: 'action'},
					{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
					{name: _UNO('.uno:FormattingMarkMenu', 'text'), id: 'formattingmark', type: 'menu', menu: [
						{uno: '.uno:InsertNonBreakingSpace'},
						{uno: '.uno:InsertHardHyphen'},
						{uno: '.uno:InsertSoftHyphen'},
						{uno: '.uno:InsertZWSP'},
						{uno: '.uno:InsertWJ'},
						{uno: '.uno:InsertLRM'},
						{uno: '.uno:InsertRLM'}]},
					{name: _UNO('.uno:FormatFormMenu', 'text'), id: 'formatformmenu', type: 'menu', menu: [
						{name: _('Insert Rich Text'), uno: '.uno:InsertContentControl'},
						{name: _('Insert Checkbox'), uno: '.uno:InsertCheckboxContentControl'},
						{name: _('Insert Dropdown'), uno: '.uno:InsertDropdownContentControl'},
						{name: _('Insert Picture'), uno: '.uno:InsertPictureContentControl'},
						{name: _('Insert Date'), uno: '.uno:InsertDateContentControl'},
						{name: _('Properties'), uno: '.uno:ContentControlProperties'},
					]},
				]
			},
			'spreadsheet' : {
				name: _UNO('.uno:InsertMenu', 'spreadsheet'), id: 'insert', type: 'menu', menu: [
					{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
					{name: _UNO('.uno:InsertGraphic', 'spreadsheet'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
					{uno: '.uno:InsertObjectChart'},
					{name: _UNO('.uno:InsertAnnotation', 'spreadsheet'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
					{type: 'separator'},
					{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
					{name: _UNO('.uno:ShapesMenu'), unoid: '.uno:ShapesMenu', id: 'insertshape', type: 'action'},
					{uno: '.uno:InsertCurrentDate'},
					{uno: '.uno:InsertCurrentTime'},
					// other fields need EditEngine context & can't be disabled in the menu.
				]
			},
			'presentation' : {
				name: _UNO('.uno:InsertMenu', 'presentation'), id: 'insert', type: 'menu', menu: [
					{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
					{name: _UNO('.uno:InsertGraphic', 'presentation'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
					{uno: '.uno:InsertObjectChart'},
					{name: _UNO('.uno:InsertAnnotation', 'presentation'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
					{name: _UNO('.uno:TableMenu'), unoid: '.uno:TableMenu', id: 'inserttable', type: 'action'},
					{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
					{name: _UNO('.uno:ShapesMenu'), unoid: '.uno:ShapesMenu', id: 'insertshape', type: 'action'},
					{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
					{name: _UNO('.uno:Text', 'presentation'), unoid: '.uno:Text', id: 'inserttextbox', type: 'action'},
					{name: _UNO('.uno:InsertField', 'text'), id: 'insertfield', type: 'menu', menu: [
						{uno: '.uno:InsertDateFieldFix'},
						{uno: '.uno:InsertDateFieldVar'},
						{uno: '.uno:InsertTimeFieldFix'},
						{uno: '.uno:InsertTimeFieldVar'},
						{type: 'separator'},
						{name: _UNO('.uno:InsertSlideField', 'presentation'), uno: '.uno:InsertPageField'},
						{name: _UNO('.uno:InsertSlideTitleField', 'presentation'), uno: '.uno:InsertPageTitleField'},
						{name: _UNO('.uno:InsertSlidesField', 'presentation'), uno: '.uno:InsertPagesField'},
					]},
				]
			},
			drawing : {
				name: _UNO('.uno:InsertMenu', 'presentation'), id: 'insert', type: 'menu', menu: [
					{name: _('Local Image...'), id: 'insertgraphic', type: 'action'},
					{name: _UNO('.uno:InsertGraphic', 'presentation'), unoid: '.uno:InsertGraphic', id: 'insertgraphicremote', type: 'action'},
					{uno: '.uno:InsertObjectChart'},
					{name: _UNO('.uno:InsertAnnotation', 'presentation'), unoid: '.uno:InsertAnnotation', id: 'insertcomment', type: 'action'},
					{name: _UNO('.uno:TableMenu'), unoid: '.uno:TableMenu', id: 'inserttable', type: 'action'},
					{name: _UNO('.uno:HyperlinkDialog'), unoid: '.uno:HyperlinkDialog', id: 'inserthyperlink', type: 'action'},
					{name: _UNO('.uno:ShapesMenu'), unoid: '.uno:ShapesMenu', id: 'insertshape', type: 'action'},
					{name: _UNO('.uno:FontworkGalleryFloater'), uno: '.uno:FontworkGalleryFloater', id: 'fontworkgalleryfloater'},
					{name: _UNO('.uno:Text', 'presentation'), unoid: '.uno:Text', id: 'inserttextbox', type: 'action'},
					{name: _UNO('.uno:InsertField', 'text'), id: 'insertfield', type: 'menu', menu: [
						{name: _UNO('.uno:InsertDateFieldFix', 'presentation'), uno: '.uno:InsertDateFieldFix'},
						{name: _UNO('.uno:InsertDateFieldVar', 'presentation'), uno: '.uno:InsertDateFieldVar'},
						{name: _UNO('.uno:InsertTimeFieldFix', 'presentation'), uno: '.uno:InsertTimeFieldFix'},
						{name: _UNO('.uno:InsertTimeFieldVar', 'presentation'), uno: '.uno:InsertTimeFieldVar'},
						{type: 'separator'},
						{name: _UNO('.uno:InsertPageField', 'presentation'), uno: '.uno:InsertPageField'},
						{name: _UNO('.uno:InsertPageTitleField', 'presentation'), uno: '.uno:InsertPageTitleField'},
						{name: _UNO('.uno:InsertPagesField', 'presentation'), uno: '.uno:InsertPagesField'},
					]},
				]
			}
		},

		commandStates: {},

		// Only these menu options will be visible in readonly mode
		allowedReadonlyMenus: window.mode.isCODesktop() ? ['file', 'view', 'slide', 'help'] : ['file', 'downloadas', 'view', 'insert', 'slide', 'help', 'print'],
		allowedRedlineManagementMenus: window.mode.isCODesktop() ? [] : ['editmenu', 'changesmenu', ],

		math: ['.uno:ChangeFont', '.uno:ChangeFontSize', '.uno:ChangeDistance', '.uno:ChangeAlignment'],

		// Only these UNO commands will be enabled in readonly mode
		allowedViewModeCommands: [
			'.uno:Signature',
		],

		allowedViewModeActions: [
			() => app.sectionContainer.getSectionWithName(app.CSections.CommentList.name).hasAnyComments() ? 'savecomments' : undefined,
			'shareas', //file menu
			'print','print-active-sheet', 'print-all-sheets', 'print-notespages', // file menu
			'downloadas-odt', 'downloadas-doc', 'downloadas-docx', 'downloadas-rtf', // file menu
			'downloadas-odp', 'downloadas-ppt', 'downloadas-pptx', 'downloadas-odg', 'exportpdf' , // file menu
			!window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf', !window.ThisIsAMobileApp ? 'exportepub' : 'downloadas-epub', // file menu
			'downloadas-ods', 'downloadas-xls', 'downloadas-xlsx', 'downloadas-csv', 'closedocument', // file menu
			() => !window.L.Browser.edge ? 'fullscreen' : undefined, 'zoomin', 'zoomout', 'zoomreset', 'fitwidthzoom', 'showstatusbar', 'showresolved', 'showannotations', 'toggledarktheme', // view menu
			'insert-signatureline', // insert menu
			'about', 'keyboard-shortcuts', 'latestupdates', 'feedback', 'serveraudit', 'online-help', 'report-an-issue', // help menu
			'insertcomment'
		],

		// Only these UNO commands will be enabled in redline management mode
		allowedRedlineManagementModeCommands: [
			'.uno:ShowTrackedChanges',
			'.uno:AcceptTrackedChanges',
			'.uno:AcceptTrackedChange',
			'.uno:RejectTrackedChange',
			'.uno:AcceptAllTrackedChanges',
			'.uno:RejectAllTrackedChanges',
			'.uno:AcceptTrackedChangeToNext',
			'.uno:RejectTrackedChangeToNext',
			'.uno:CommentChangeTracking',
			'.uno:PreviousTrackedChange',
			'.uno:NextTrackedChange',
		],

		allowedRedlineManagementModeActions: [
			'acceptalltrackedchanges',
			'rejectalltrackedchanges',
		],
	}

	// Private properties
	private _initialized: boolean = false;
	private _hiddenItems: string[] = [];
	private _menubarCont: HTMLElement | null = null;
	private _isFileODF: boolean = true;
	private lastModIndicator: HTMLElement | null = null;
	private _map: any;

	/**
	 * Called when the menubar is added to the map.
	 * Initializes the menubar container, sets up the template, and binds events.
	 * @param map - The Leaflet map instance.
	 * @returns The menubar container HTMLElement.
	 */
	onAdd(map: ReturnType<typeof window.L.Map>) {
		this._initialized = false;
		this._hiddenItems = [];
		this._menubarCont = window.L.DomUtil.get('main-menu');
		this._isFileODF = true;
		this._map = map;
		// In case it contains garbage
		if (this._menubarCont)
			this._menubarCont.remove();
		// Use original template as provided by server
		this._menubarCont = map.mainMenuTemplate.cloneNode(true);
		if (this._menubarCont != null)
			$('#main-menu-state').after(this._menubarCont);

		if (window.mode.isDesktop()) {
			$('#main-menu-state').attr('type', 'hidden');
		}

		if (!this._map['wopi'].DisablePresentation)
			this.options.allowedViewModeActions = this.options.allowedViewModeActions.concat(['fullscreen-presentation', 'presentation-currentslide', 'present-in-window','presentation-in-console']);

		this._initializeMenu(this.options.initial);

		map.on('doclayerinit', this._onDocLayerInit, this);
		this._onRefresh = this._onRefresh.bind(this);
		app.events.on('updatepermission', this._onRefresh);
		map.on('addmenu', this._addMenu, this);
		map.on('languagesupdated', this._onInitLanguagesMenu, this);
		map.on('updatetoolbarcommandvalues', this._onStyleMenu, this);
		map.on('initmodificationindicator', this._onInitModificationIndicator, this);
		map.on('updatemodificationindicator', this._onUpdateModificationIndicator, this);
	}

	/**
	 * Called when the menubar is removed from the map.
	 * Unbinds events and cleans up the menubar container.
	 */
	onRemove(): void {

		this._map.off('doclayerinit', this._onDocLayerInit, this);
		this._map.off('addmenu', this._addMenu, this);
		this._map.off('languagesupdated', this._onInitLanguagesMenu, this);
		this._map.off('updatetoolbarcommandvalues', this._onStyleMenu, this);
		this._map.off('initmodificationindicator', this._onInitModificationIndicator, this);
		this._map.off('updatemodificationindicator', this._onUpdateModificationIndicator, this);
		app.events.off('updatepermission', this._onRefresh);

		this._menubarCont?.remove();
		this._menubarCont = null;
	}

	/**
	 * Adds a new action menu item to the menubar.
	 * @param e - Event data containing the new menu item details.
	 */
	private _addMenu(e: any): void {
		var alreadyExists = window.L.DomUtil.get('menu-' + e.id);
		if (alreadyExists)
			return;

		var liItem = window.L.DomUtil.create('li', '');
		liItem.setAttribute('role', 'menuitem');
		liItem.id = 'menu-' + e.id;
		if (this._map.isReadOnlyMode()) {
			window.L.DomUtil.addClass(liItem, 'readonly');
		}
		var aItem = window.L.DomUtil.create('a', '', liItem);
		$(aItem).text(e.label);
		$(aItem).data('id', e.id);
		$(aItem).data('type', 'action');
		$(aItem).data('postmessage', 'true');
		aItem.tabIndex = 0;
		this._menubarCont?.insertBefore(liItem, this._menubarCont.firstChild);
	}

	/**
       * Creates a menu item element for a UNO command.
       * @param caption - The display text.
       * @param command - The UNO command string.
       * @param tag - Optional additional tag.
       * @returns The created list item element.
       */
	private _createUnoMenuItem(caption: string, command: string, tag?: string): HTMLElement {
		var liItem, aItem;
		liItem = window.L.DomUtil.create('li', '');
		liItem.setAttribute('role', 'menuitem');
		aItem = window.L.DomUtil.create('a', '', liItem);
		$(aItem).text(caption);
		$(aItem).data('type', 'unocommand');
		$(aItem).data('uno', command);
		if (tag !== undefined)
			$(aItem).data('tag', tag);
		aItem.tabIndex = 0;
		return liItem;
	}

	/**
	 * Creates a menu item element for an action.
	 * @param caption - The display text.
	 * @param id - The action identifier.
	 * @returns The created list item element.
	 */
	private _createActionMenuItem(caption: string, id: string): HTMLElement {
		var liItem, aItem;
		liItem = window.L.DomUtil.create('li', '');
		liItem.setAttribute('role', 'menuitem');
		aItem = window.L.DomUtil.create('a', '', liItem);
		$(aItem).text(caption);
		$(aItem).data('type', 'action');
		$(aItem).data('id', id);
		aItem.tabIndex = 0;
		return liItem;
	}

	/**
	 * Initializes the languages menu.
	 */
	private _onInitLanguagesMenu(): void {
		var translated, neutral;
		var constDefa = 'Default_RESET_LANGUAGES';
		var constCurr = 'Current_RESET_LANGUAGES';
		var constPara = 'Paragraph_RESET_LANGUAGES';
		var constLang = '.uno:LanguageStatus?Language:string=';
		var resetLang = _('Reset to Default Language');
		var languages  = app.languages;

		var $menuSelection = $('#menu-noneselection').parent();
		var $menuParagraph = $('#menu-noneparagraph').parent();
		var $menuDefault = $('#menu-nonelanguage').parent();

		var noneselection = $('#menu-noneselection').detach();
		var fontlanguage = $('#menu-fontlanguage').detach();
		var noneparagraph = $('#menu-noneparagraph').detach();
		var paragraphlanguage = $('#menu-paragraphlanguage').detach();
		var nonelanguage = $('#menu-nonelanguage').detach();

		// clear old entries

		$menuSelection.empty();
		$menuParagraph.empty();
		$menuDefault.empty();

		for (var lang in languages) {
			if (languages.length > 10 && app.favouriteLanguages.indexOf(languages[lang].iso) < 0)
				continue;

			translated = languages[lang].translated;
			neutral = languages[lang].neutral;

			$menuSelection.append(this._createUnoMenuItem(translated, constLang + encodeURIComponent('Current_' + neutral)));
			$menuParagraph.append(this._createUnoMenuItem(translated, constLang + encodeURIComponent('Paragraph_' + neutral)));
			$menuDefault.append(this._createUnoMenuItem(translated, constLang + encodeURIComponent('Default_' + neutral)));
		}

		$menuSelection.append(this._createActionMenuItem(_('More...'), 'morelanguages-selection'));
		$menuParagraph.append(this._createActionMenuItem(_('More...'), 'morelanguages-paragraph'));
		$menuDefault.append(this._createActionMenuItem(_('More...'), 'morelanguages-all'));

		$menuSelection.append(this._createMenu([{type: 'separator'}]));
		$menuParagraph.append(this._createMenu([{type: 'separator'}]));
		$menuDefault.append(this._createMenu([{type: 'separator'}]));

		$menuSelection.append(this._createUnoMenuItem(resetLang, constLang + constCurr));
		$menuParagraph.append(this._createUnoMenuItem(resetLang, constLang + constPara));
		$menuDefault.append(this._createUnoMenuItem(resetLang, constLang + constDefa));

		$menuSelection.append(noneselection);
		$menuSelection.append(fontlanguage);
		$menuParagraph.append(noneparagraph);
		$menuParagraph.append(paragraphlanguage);
		$menuDefault.append(nonelanguage);
	}

	/**
       * Adds tabindex properties to main menu items.
       */
	private _addTabIndexPropsToMainMenu(): void {
		var mainMenu = document.getElementById('main-menu');
		if (mainMenu == null)
			return;

		for (var i = 0; i < mainMenu.children.length; i++) {
			if (mainMenu.children[i].children[0].getAttribute('aria-haspopup') === 'true') {
				(mainMenu.children[i].children[0] as HTMLElement).tabIndex = 0;
			}
		}
	}

	/**
	 * Refreshes the menubar.
	 */
	private _onRefresh(): void {
		if (!this._initialized) {
			this._initialized = true;
			this._onDocLayerInit();
		}

		// clear initial menu
		if (this._menubarCont)
			window.L.DomUtil.removeChildNodes(this._menubarCont);

		// Add document specific menu
		var docType = this._map.getDocType();
		if (docType === 'text') {
			this._initializeMenu(this.options.text);
		} else if (docType === 'spreadsheet') {
			this._initializeMenu(this.options.spreadsheet);
		} else if (docType === 'presentation') {
			this._initializeMenu(this.options.presentation);
		} else if (docType === 'drawing') {
			this._initializeMenu(this.options.drawing);
		}

		// initialize menubar plugin
		$('#main-menu').smartmenus({
			hideOnClick: true,
			showOnClick: true,
			hideTimeout: 0,
			hideDuration: 0,
			hideFunction: null,
			showDuration: 0,
			showFunction: null,
			showTimeout: 0,
			collapsibleHideDuration: 0,
			collapsibleHideFunction: null,
			subIndicatorsPos: 'append',
			subIndicatorsText: '&#8250;'
		});
		$('#main-menu').attr('tabindex', 0);

		document.getElementById('main-menu')?.setAttribute('role', 'menubar');
		this._addTabIndexPropsToMainMenu();
		this._createFileIcon();
	}

	/**
	 * Function to bind an event if it's not already bound.
	 * @param element - The target element.
	 * @param eventType - The event type.
	 * @param namespace - The namespace.
	 * @param data - Data to pass to the event handler.
	 * @param handler - The event handler function.
	 */
	private _bindEventIfNotBound(element: any, eventType: string, namespace: string, data: any, handler: any): void {
		const eventName = eventType + (namespace ? '.' + namespace : '');
		$(element).off(eventName).on(eventName, data, (e, menu) => handler.call(this, e, menu));
	}

	/**
	 * Handles style changes for the menu based on UNO command state.
	 * @param e - The event containing command state data.
	 */
	private _onStyleMenu(e: any): void {
		if (e.commandName === '.uno:StyleApply') {
			var style: any;
			var constArg = '&';
			var constHeader = '.uno:InsertPageHeader?PageStyle:string=';
			var constFooter = '.uno:InsertPageFooter?PageStyle:string=';
			var $menuHeader = $('#menu-insertheader').parent();
			var $menuFooter = $('#menu-insertfooter').parent();
			var pageStyles = e.commandValues['HeaderFooter'];
			for (var iterator in pageStyles) {
				style = pageStyles[iterator];
				if (!window.mode.isSmallScreenDevice()) {
					$menuHeader.append(this._createUnoMenuItem(_(style), constHeader + encodeURIComponent(style) + constArg, style));
					$menuFooter.append(this._createUnoMenuItem(_(style), constFooter + encodeURIComponent(style) + constArg, style));
				} else {
					var docType: string = this._map.getDocType();
					var target = this.options.mobileInsertMenu[docType];

					var findFunction = (item: MenuItem) => {
						return item.name === _(style);
					};

					var foundMenu = this._findSubMenuByName(target, _UNO('.uno:InsertPageHeader', 'text'));
					if (foundMenu && foundMenu.menu.find(findFunction) === undefined)
						foundMenu.menu.push({name: _(style), tag: style, uno: constHeader + encodeURIComponent(style) + constArg});

					foundMenu = this._findSubMenuByName(target, _UNO('.uno:InsertPageFooter', 'text'));
					if (foundMenu && foundMenu.menu.find(findFunction) === undefined)
						foundMenu.menu.push({name: _(style), tag: style, uno: constFooter + encodeURIComponent(style) + constArg});
				}
			}
		}
	}

	/**
	 * Creates a new document when the file icon is clicked.
	 * @param e - The event data.
	 */
	private _createDocument(e: any): void {
		var self = e.data.self;
		var docType = self._map.getDocType();
		self._map.fire('postMessage', {msgId: 'UI_CreateFile', args: {DocumentType: docType}});
	}

	/**
       * Initializes the menubar when the document layer is ready.
       */
	private _onDocLayerInit(): void {
		this._onRefresh();

		// Usage
		this._bindEventIfNotBound('#main-menu', 'select', 'smapi', {self: this}, this._onItemSelected);
		this._bindEventIfNotBound('#main-menu', 'beforeshow', 'smapi', {self: this}, this._beforeShow);
		this._bindEventIfNotBound('#main-menu', 'click', 'smapi', {self: this}, this._onClicked);
		this._bindEventIfNotBound('#main-menu', 'keydown', '', {self: this}, this._onKeyDown);

		if (window.mode.isSmallScreenDevice()) {
			$('#main-menu').parent().css('height', '0');
			$('#toolbar-wrapper').addClass('mobile');
		}

		// Also the vertical menu displayed when tapping the hamburger button is produced by SmartMenus
		$(() => {
			const $mainMenuState = $('#main-menu-state');
			if ($mainMenuState.length) {
				// animate mobile menu
				$mainMenuState.change((event) => {
					// This code is invoked when the hamburger menu is opened or closed
					var $menu = $('#main-menu');
					var $nav = $menu.parent();
					if ((event.target as HTMLInputElement).checked) {
						if (!window.mode.isSmallScreenDevice()) {
							// Surely this code, if it really is related only to the hamburger menu,
							// will never be invoked on non-mobile browsers? I might be wrong though.
							// If you notice this logging, please modify this comment to indicate what is
							// going on.
							window.app.console.log('======> Assertion failed!? Not window.mode.isSmallScreenDevice()? Control.Menubar.js #1');
							$nav.css({height: 'initial', bottom: '38px'});
							$menu.hide().slideDown(250, () => { $menu.css('display', ''); });
						} else {
							window.mobileMenuWizard = true;
							const menuData = this._map.menubar.generateFullMenuStructure();
							this._map.fire('mobilewizard', { data: menuData });
							$('#toolbar-hamburger').removeClass('menuwizard-closed').addClass('menuwizard-opened');
							$('#toolbar-mobile-back').css('visibility', 'hidden');
							$('#formulabar').hide();
						}
					} else if (!window.mode.isSmallScreenDevice()) {
						// Ditto.
						window.app.console.log('======> Assertion failed!? Not window.mode.isSmallScreenDevice()? Control.Menubar.js #2');
						$menu.show().slideUp(250, () => { $menu.css('display', ''); });
						$nav.css({height:'', bottom: ''});
					} else {
						window.mobileMenuWizard = false;
						this._map.fire('closemobilewizard');
						$('#toolbar-hamburger').removeClass('menuwizard-opened').addClass('menuwizard-closed');
						$('#toolbar-mobile-back').css('visibility', '');
						if (this._map.getDocType() === 'spreadsheet') {
							$('#formulabar').show();
						}
					}
				});

				// hide mobile menu beforeunload
				$(window).bind('beforeunload unload', () => {
					if (($mainMenuState[0] as HTMLInputElement).checked) {
						$mainMenuState[0].click();
					}
				});
			}
		});

		this._initialized = true;
	}

	/**
       * Handles click events on menu items.
       * @param e - The event data.
       * @param menu - The clicked menu element.
       */
	private _onClicked(e: any, menu: any): void {
		if ($(menu).hasClass('highlighted')) {
			$('#main-menu').smartmenus('menuHideAll');
		}

		var $mainMenuState = $('#main-menu-state');
		if (!$(menu).hasClass('has-submenu') && ($mainMenuState[0] as HTMLInputElement).checked) {
			$mainMenuState[0].click();
		}

		if (menu?.parentElement?.id === 'menu-file' && window.mode.isCODesktop() && app.map.backstageView)
			app.map.backstageView.toggle();
	}

	/**
       * Checks and sets the “checked” visual state for UNO menu items.
       * @param uno - The UNO command.
       * @param item - The menu item element.
       */
	private _checkedMenu(uno: string, item: any): void {
		var constChecked = 'lo-menu-item-checked';
		var state = this._map['stateChangeHandler'].getItemValue(uno);
		var data = $(item).data('tag');
		state = state[data] || false;
		if (state) {
			$(item).addClass(constChecked);
		} else {
			$(item).removeClass(constChecked);
		}
	}

	/**
       * Handler invoked before a submenu is shown.
       * Updates item states (e.g. disabled, checked) based on command values.
       * @param e - Event data.
       * @param menu - The submenu element.
       */
	private _beforeShow(e: any, menu: any): void {
		const items = this._getMenuItems(menu);
		$(items).each((index, aItem) => {
			const type = $(aItem).data('type');
			const id = $(aItem).data('id');
			const uno = $(aItem).data('uno');
			const constChecked = 'lo-menu-item-checked';

			if (this._map.isEditMode()) {
				if (type === 'unocommand') { // enable all depending on stored commandStates
					var data, lang, languageAndCode;
					var constUno = 'uno';
					var constState = 'stateChangeHandler';
					var constLanguage = '.uno:LanguageStatus';
					var constPageHeader = '.uno:InsertPageHeader';
					var constPageFooter = '.uno:InsertPageFooter';
					var unoCommand = $(aItem).data(constUno);
					var itemState = this._map[constState].getItemValue(unoCommand);
					if (itemState === 'disabled') {
						if (unoCommand.startsWith('.uno:Paste')) {
							window.app.console.log('do not disable paste based on server side data');
						} else {
							$(aItem).addClass('disabled');
						}
					} else {
						$(aItem).removeClass('disabled');
					}
					if (unoCommand.startsWith(constLanguage)) {
						unoCommand = constLanguage;
						languageAndCode = this._map[constState].getItemValue(unoCommand);
						lang = languageAndCode.split(';');
						if (lang.length) {
							lang = lang[0];
						} else {
							lang = '';
							app.console.error('Menubar _beforeShow: missing language');
						}
						data = decodeURIComponent($(aItem).data(constUno));
						if (data.indexOf(lang) !== -1) {
							$(aItem).addClass(constChecked);
						} else if (data.indexOf('LANGUAGE_NONE') !== -1 && lang === '[None]') {
							$(aItem).addClass(constChecked);
						} else {
							$(aItem).removeClass(constChecked);
						}
					}
					else if (unoCommand.startsWith(constPageHeader)) {
						unoCommand = constPageHeader;
						this._checkedMenu(unoCommand, this);
					}
					else if (unoCommand.startsWith(constPageFooter)) {
						unoCommand = constPageFooter;
						this._checkedMenu(unoCommand, this);
					}
					else if (itemState === 'true') {
						$(aItem).addClass(constChecked);
					} else {
						$(aItem).removeClass(constChecked);
					}
					if (this.options.math.includes(unoCommand) && app.map.context && app.map.context.context !== 'Math') {
						$(aItem).addClass('disabled');
					}
				} else if (type === 'action') { // enable all except fullscreen on windows
					if (id === 'fullscreen') { // Full screen works weirdly on Edge
						if (window.L.Browser.edge) {
							$(aItem).addClass('disabled');
						} else if (this._map.uiManager.isFullscreen()) {
							$(aItem).addClass(constChecked);
						} else {
							$(aItem).removeClass(constChecked);
						}
					} else if (id === 'showruler') {
						if (this._map.uiManager.isRulerVisible()) {
							$(aItem).addClass(constChecked);
						} else {
							$(aItem).removeClass(constChecked);
						}
					} else if (id == 'toggledarktheme') {
						if (window.prefs.getBoolean('darkTheme')) {
							$(aItem).addClass(constChecked);
							$('#menu-invertbackground').show();
						} else {
							$(aItem).removeClass(constChecked);
							$('#menu-invertbackground').hide();
						}
					} else if (id === 'showstatusbar') {
						if (this._map.uiManager.isStatusBarVisible()) {
							$(aItem).addClass(constChecked);
						} else {
							$(aItem).removeClass(constChecked);
						}
					} else if (id === 'notesmode') {
						if (app.impress.notesMode)
							$(aItem).addClass(constChecked);
						else
							$(aItem).removeClass(constChecked);
					} else if (id === 'toggleuimode') {
						if (this._map.uiManager.shouldUseNotebookbarMode()) {
							$(aItem).text(_('Use Compact view'));
						} else {
							$(aItem).text(_('Use Tabbed view'));
						}
					} else if (id === 'showslide') {
						if (!app.impress.isSlideHidden(this._map.getCurrentPartNumber()))
							$(aItem).hide();
						else
							$(aItem).show();
					} else if (id === 'hideslide') {
						if (app.impress.isSlideHidden(this._map.getCurrentPartNumber()))
							$(aItem).hide();
						else
							$(aItem).show();
					} else if (id === 'togglea11ystate') {
						var enabled = window.prefs.getBoolean('accessibilityState');
						if (enabled) {
							$(aItem).addClass(constChecked);
						} else {
							$(aItem).removeClass(constChecked);
						}
					} else if (this._map.getDocType() === 'presentation' && (id === 'deletepage' || id === 'insertpage' || id === 'duplicatepage')) {
						if (id === 'deletepage') {
							itemState = this._map['stateChangeHandler'].getItemValue('.uno:DeletePage');
						} else if (id === 'insertpage') {
							itemState = this._map['stateChangeHandler'].getItemValue('.uno:InsertPage');
						} else {
							itemState = this._map['stateChangeHandler'].getItemValue('.uno:DuplicatePage');
						}
						if (itemState === 'disabled') {
							$(aItem).addClass('disabled');
						} else {
							$(aItem).removeClass('disabled');
						}
					} else if (id === 'showannotations') {
						var section = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
						if (section) {
							itemState = this._map['stateChangeHandler'].getItemValue('showannotations');
							if (itemState === 'true')
								$(aItem).addClass(constChecked);
							else
								$(aItem).removeClass(constChecked);
						}
					} else if (id === 'showresolved') {
						var section = app.sectionContainer.getSectionWithName(app.CSections.CommentList.name);
						if (section) {
							if (section.sectionProperties.commentList.length === 0 ||
								!section.sectionProperties.show) {
								$(aItem).addClass('disabled');
							} else if (section.sectionProperties.showResolved) {
								$(aItem).removeClass('disabled');
								$(aItem).addClass(constChecked);
							} else {
								$(aItem).removeClass('disabled');
								$(aItem).removeClass(constChecked);
							}
						}
					} else if (id === 'acceptalltrackedchanges' || id === 'rejectalltrackedchanges') {
						itemState = this._map['stateChangeHandler'].getItemValue(uno);
						if (itemState === 'disabled') {
							$(aItem).addClass('disabled');
						} else {
							$(aItem).removeClass('disabled');
						}
					} else if (id === 'serveraudit' && (app.isAdminUser === false || !this._map.serverAuditDialog)) {
						$(aItem).css('display', 'none');
					} else if (id === 'columnrowhighlight') {
						itemState = app.map.uiManager.getHighlightMode();
						if (itemState) $(aItem).addClass(constChecked);
						else $(aItem).removeClass(constChecked);
					} else {
						$(aItem).removeClass('disabled');
					}
				}
				if (id && id.indexOf('zotero') >= 0) {
					if (window.zoteroEnabled && this._map.zotero)
						$(aItem).show();
					else
						$(aItem).hide();
				}
			} else { // eslint-disable-next-line no-lonely-if
				if (type === 'unocommand') { // disable all uno commands
					// Except the ones listed in allowedViewModeCommands:
					var allowed = this.options.allowedViewModeCommands.includes(uno);
					if (!allowed && app.isRedlineManagementAllowed() && !this._map['wopi'].HideChangeTrackingControls) {
						allowed = this.options.allowedRedlineManagementModeCommands.includes(uno);
					}
					if (!allowed) {
						$(aItem).hide();
					} else {
						var itemState = this._map['stateChangeHandler'].getItemValue(uno);
						if (itemState === 'disabled') {
							$(aItem).addClass('disabled');
						} else {
							$(aItem).removeClass('disabled');
							if (itemState === 'true')
								$(aItem).addClass(constChecked);
							else
								$(aItem).removeClass(constChecked);
						}
					}
				} else if (type === 'action') { // disable all except allowedViewModeActions
					var allowed = false;
					for (var i in this.options.allowedViewModeActions) {
						const action = this.options.allowedViewModeActions[i];
						if (typeof action === "string" && action === id) {
							allowed = true;
							break;
						} else if (typeof action === "function" && action() === id) {
							allowed = true;
							break;
						}
					}
					if (!allowed && app.isRedlineManagementAllowed() && !this._map['wopi'].HideChangeTrackingControls)
						allowed = this.options.allowedRedlineManagementModeActions.includes(id);
					if (id === 'insertcomment' && (this._map.getDocType() !== 'drawing' && !app.isCommentEditingAllowed()))
						allowed = false;
					if (id === 'serveraudit' && (app.isAdminUser === false || !this._map.serverAuditDialog))
						allowed = false;
					if (!allowed) {
						$(aItem).hide();
					} else if (uno !== undefined) {
						itemState = this._map['stateChangeHandler'].getItemValue(uno);
						if (itemState === 'disabled') {
							$(aItem).addClass('disabled');
						} else {
							$(aItem).removeClass('disabled');
						}
					}
				}

				const $menuItems = $(menu).children('li');

				$menuItems.each((index, li) => {
					const $aItem = $(li).children('a').first();
					if (!$aItem.hasClass('separator')) return;

					const $prevVisible = $(li).prevAll('li').filter(function() {
						const $a = $(this).children('a').first();
						return !$a.hasClass('separator') &&
							$a.css('display') !== 'none' &&
							$(this).css('display') !== 'none';
					}).first();

					const $nextVisible = $(li).nextAll('li').filter(function() {
						const $a = $(this).children('a').first();
						return !$a.hasClass('separator') &&
							$a.css('display') !== 'none' &&
							$(this).css('display') !== 'none';
					}).first();

					if ($prevVisible.length === 0 || $nextVisible.length === 0) {
						$aItem.hide();
					} else {
						$aItem.show();
					}
				});
			}

			if (id === 'remotelink') {
				if (this._map['wopi'].EnableRemoteLinkPicker)
					$(aItem).show();
				else
					$(aItem).hide();
			}

			if (id === 'remoteaicontent') {
				if (this._map['wopi'].EnableRemoteAIContent)
					$(aItem).show();
				else
					$(aItem).hide();
			}
		});
		// We hide adjacent, leading, and trailing separators that might occur due to hidden items above.
		var visibleItems = this._getMenuItems(menu).filter(function(this: HTMLElement) {
			return $(this).css('display') !== 'none';
		});

		visibleItems.each((index: number, aItem: HTMLElement) => {
			// Always show first, might be hidden by previous pass
			if ($(aItem).hasClass('separator')) {
				$(aItem).show();
			}

			// Hide leading separator
			if (index === 0 && $(aItem).hasClass('separator')) {
				$(aItem).hide();
				return;
			}

			// Hide trailing separator
			if (index === visibleItems.length - 1 && $(aItem).hasClass('separator')) {
				$(aItem).hide();
				return;
			}

			// Hide double/adjacent separators
			// If this is a separator, and the previous visible item was also a separator, hide this one.
			if (index > 0) {
				var prevItem = visibleItems[index - 1];
				if ($(aItem).hasClass('separator') && $(prevItem).hasClass('separator')) {
					$(aItem).hide();
				}
			}
		});
	}

	/**
       * Opens the Insert Shapes wizard.
       */
	private _openInsertShapesWizard(): void {
		var content = window.createShapesPanel('insertshapes');
		var data = {
			id: 'insertshape',
			type: '',
			text: _('Insert Shape'),
			enabled: true,
			children: [] as any[]
		};

		var container = {
			id: '',
			type: 'htmlcontrol',
			content: content,
			enabled: true
		};

		data.children.push(container);
		this._map._docLayer._openMobileWizard(data);
	}

	/**
	 * Executes an action command based on the selected menu item.
	 * @param itNode - The menu item element.
	 * @param itWizard - Optional wizard data.
	 */
	private _executeAction(itNode: any, itWizard?: any): void {
		var id, postmessage, type, command;
		if (itNode === undefined)
		{ // called from JSDialogBuilder
			id = itWizard.id;
			postmessage = false;
			command = itWizard.command;
		}
		else
		{ // called from
			id = $(itNode).data('id');
			type = $(itNode).data('type');
			postmessage = ($(itNode).data('postmessage') === 'true');
			command = $(itNode).data('uno');
		}

		if (id === 'save') {
			// Save only when not read-only.
			if (!this._map.isReadOnlyMode()) {
				this._map.fire('postMessage', {msgId: 'UI_Save', args: { source: 'filemenu' }});

				if (!this._map._disableDefaultAction['UI_Save']) {
					this._map.save(false, false);
				}
			}
		} else if (id === 'saveas' && type !== 'menu') { // jsdialog has no type='action'
			this._map.openSaveAs();
		} else if (id === 'savecomments') {
			app.dispatcher.dispatch('savecomments');
		} else if (id === 'shareas' || id === 'ShareAs') {
			app.dispatcher.dispatch('shareas');
		} else if (id === 'print' || id === 'print-notespages') {
			app.dispatcher.dispatch(id);
		} else if (id.startsWith('downloadas-')
			|| id.startsWith('saveas-')
			|| id.startsWith('export')
			|| id === 'renamedocument'
			|| id.startsWith('zotero')
			|| id === 'deletepage'
			|| id === 'remotelink'
			|| id === 'remoteaicontent'
			|| id === 'toggledarktheme'
			|| id === 'invertbackground'
			|| id === 'home-search'
			|| id === 'print-active-sheet'
			|| id === 'print-all-sheets'
			|| id === 'serveraudit'
			|| id === 'animationdeck'
			|| id === 'transitiondeck') {
			app.dispatcher.dispatch(id);
		} else if (id === ('settings-dialog')) {
			this._map.settings.showSettingsDialog();
		} else if (id === 'insertcomment') {
			this._map.insertComment();
		} else if (id === 'insert-signatureline') {
			if (this._map.eSignature) {
				const args = {
					External: {
						type: 'boolean',
						value: true,
					},
				};
				app.map.sendUnoCommand('.uno:InsertSignatureLine', args);
				const finishMessage = _('The signature line can now be moved or resized as needed.');
				const finishFunc = () => {
					Util.ensureValue(app.map.eSignature);
					app.map.eSignature.insert();
				};
				app.map.uiManager.showSnackbar(finishMessage, _('Finish electronic signing'), finishFunc, -1);
			} else {
				app.map.sendUnoCommand('.uno:InsertSignatureLine');
			}

			// The file based view is primarily to view multi-page PDF files, so
			// it doesn't seem to have precise tracking of invalidations, just
			// request new tiles for now.
			app.map._docLayer.requestNewFiledBasedViewTiles();
		} else if (id === 'insertgraphic') {
			window.L.DomUtil.get('insertgraphic').click();
		} else if (id === 'insertgraphicremote') {
			this._map.fire('postMessage', {msgId: 'UI_InsertGraphic'});
		} else if (id === 'insertmultimedia') {
			window.L.DomUtil.get('insertmultimedia').click();
		} else if (id === 'remotemultimedia') {
			this._map.fire('postMessage', {
				msgId: 'UI_InsertFile', args: {
					callback: 'Action_InsertMultimedia', mimeTypeFilter: app.LOUtil.mediaMimeFilter
				}
			});
		} else if (id === 'selectbackground') {
			app.dispatcher.dispatch('selectbackground');
		} else if (id === 'zoomin' && this._map.getZoom() < this._map.getMaxZoom()) {
			app.dispatcher.dispatch('zoomin');
		} else if (id === 'showannotations') {
			app.dispatcher.dispatch('showannotations');
		} else if (id === 'showresolved') {
			app.dispatcher.dispatch('.uno:ShowResolvedAnnotations');
		} else if (id === 'zoomout' && this._map.getZoom() > this._map.getMinZoom()) {
			app.dispatcher.dispatch('zoomout');
		} else if (id === 'zoomreset') {
			app.dispatcher.dispatch('zoomreset');
		} else if (id === 'fitwidthzoom') {
			app.dispatcher.dispatch('fitwidthzoom');
		} else if (id === 'fullscreen') {
			app.util.toggleFullScreen();
		} else if (id === 'showruler') {
			app.dispatcher.dispatch('showruler');
		} else if (id === 'togglea11ystate') {
			app.dispatcher.dispatch('togglea11ystate');
		} else if (id === 'toggleuimode') {
			app.dispatcher.dispatch('toggleuimode');
		} else if (id === 'showstatusbar') {
			app.dispatcher.dispatch('showstatusbar');
		} else if (id === 'notesmode') {
			app.dispatcher.dispatch('notesmode');
		} else if (id === 'togglemenubar') {
			this._map.uiManager.toggleMenubar();
		} else if (id === 'collapsenotebookbar') {
			app.dispatcher.dispatch('collapsenotebookbar');
		} else if (id === 'fullscreen-presentation' && this._map.getDocType() === 'presentation') {
			app.dispatcher.dispatch('fullscreen-presentation');
		} else if (id === 'presentation-currentslide' && this._map.getDocType() === 'presentation') {
			app.dispatcher.dispatch('presentation-currentslide');
		} else if (id === 'present-in-window' && this._map.getDocType() === 'presentation') {
			app.dispatcher.dispatch('present-in-window');
		} else if (id === 'presentation-in-console') {
			app.dispatcher.dispatch('presenterconsole');
		} else if (id === 'insertpage') {
			this._map.insertPage();
		} else if (id === 'insertshape') {
			this._openInsertShapesWizard();
		} else if (id === 'duplicatepage') {
			this._map.duplicatePage();
		} else if (id === 'about') {
			this._map.showLOAboutDialog();
		} else if (id === 'latestupdates' && this._map.welcome) {
			if (window.mode.isCODesktop()) this._map.welcome.showWelcomeSlideshow();
			else this._map.welcome.showWelcomeDialog();
		} else if (id === 'feedback' && this._map.feedback) {
			this._map.feedback.showFeedbackDialog();
		} else if (id === 'report-an-issue') {
			window.open('https://github.com/CollaboraOnline/online/issues', '_blank', 'noopener');
		} else if (id === 'forum') {
			window.open('https://forum.collaboraonline.com', '_blank', 'noopener');
		} else if (id === 'inserthyperlink') {
			app.dispatcher.dispatch('hyperlinkdialog');
		} else if (id === 'keyboard-shortcuts' || id === 'online-help') {
			this._map.showHelp(id + '-content');
		} else if (window.L.Params.revHistoryEnabled && (id === 'rev-history' || id === 'Rev-History' || id === 'last-mod')) {
			app.dispatcher.dispatch('rev-history');
		} else if (id === 'closedocument') {
			app.dispatcher.dispatch('closeapp');
		} else if (id === 'repair') {
			app.dispatcher.dispatch('repair');
		} else if (id === 'recsearch') {
			app.dispatcher.dispatch('showsearchbar');
		} else if (id === 'inserttextbox') {
			this._map.sendUnoCommand('.uno:Text?CreateDirectly:bool=true');
		} else if (id === 'pagesetup') {
			this._map.sendUnoCommand('.uno:SidebarShow');
			this._map.sendUnoCommand('.uno:LOKSidebarWriterPage');
			this._map.fire('showwizardsidebar', {noRefresh: true});
			window.pageMobileWizard = true;
		} else if (id === 'showslide') {
			app.dispatcher.dispatch('showslide');
		} else if (id === 'hideslide') {
			app.dispatcher.dispatch('hideslide');
		} else if (id.indexOf('morelanguages-') != -1) {
			this._map.fire('morelanguages', { applyto: id.substr('morelanguages-'.length) });
		} else if (id === 'acceptalltrackedchanges' || id === 'rejectalltrackedchanges') {
			app.dispatcher.dispatch(command);
		} else if (id === 'columnrowhighlight') {
			app.dispatcher.dispatch('columnrowhighlight');
		} else if (id === 'comparechanges') {
			app.dispatcher.dispatch('comparechanges');
		} else if (id === 'multipageview') {
			app.dispatcher.dispatch('multipageview');
		} else {
			// not found
			app.console.warn('Menubar: unknown action for id: ' + id);
		}

		// Inform the host if asked
		if (postmessage)
			this._map.fire('postMessage', {msgId: 'Clicked_Button', args: {Id: id} });
	}

	/**
	 * Sends a UNO command extracted from the menu item data.
	 * @param item - The menu item element.
	 */
	private _sendCommand(item: any): void {
		var unoCommand = $(item).data('uno');
		if (unoCommand.startsWith('.uno:InsertPageHeader') || unoCommand.startsWith('.uno:InsertPageFooter')) {
			unoCommand = unoCommand + ($(item).hasClass('lo-menu-item-checked') ? 'On:bool=false' : 'On:bool=true');
		}
		else if (unoCommand.startsWith('.uno:SlideMasterPage')) {
			// Toggle between showing master page and closing it.
			unoCommand = ($(item).hasClass('lo-menu-item-checked') ? '.uno:CloseMasterView' : '.uno:SlideMasterPage');
		}
		else if (this._map._clip && this._map._clip.filterExecCopyPaste(unoCommand)) {
			return;
		}

		this._map.sendUnoCommand(unoCommand);
	}

	/**
	 * Handles the selection of a menu item.
	 * @param e - Event data.
	 * @param item - The selected menu item element.
	 */
	private _onItemSelected(e: any, item: any): void {
		// TODO: Find a way to disable click/select events for locked elements in disableLockedItem
		if ($(item).data('locked') === true)
			return;

		var type = $(item).data('type');
		if (type === 'unocommand') {
			this._sendCommand(item);
		} else if (type === 'action') {
			this._executeAction(item);
		}

		if (!window.mode.isSmallScreenDevice() && $(item).data('id') !== 'insertcomment' && self && this._map)
			this._map.focus();
	}

	/**
	 * Handles keydown events on the menubar.
	 * Specifically, shows help when F1 is pressed.
	 * @param e - The keyboard event.
	 */
	private _onKeyDown(e: KeyboardEvent, self: Menubar): void {
		// handle help - F1
		if (e.type === 'keydown' && !e.shiftKey && !e.ctrlKey && !e.altKey && e.keyCode == 112) {
			self._map.showHelp('online-help-content');
		}
	}

	/**
	 * Creates the file icon in the menubar header.
	 */
	private _createFileIcon(): void {
		if (!(window.logoURL && window.logoURL == "none")) {
			var liItem = window.L.DomUtil.create('li', '');
			liItem.id = 'document-header';
			liItem.setAttribute('role', 'menuitem');
			var aItem = window.L.DomUtil.create('a', 'document-logo', liItem);
			$(aItem).data('id', 'document-logo');
			$(aItem).data('type', 'action');
			aItem.setAttribute('role', 'img');
			aItem.setAttribute('aria-label', _('file type icon'));
			aItem.target = '_blank';

			if (window.logoURL) {
				aItem.style.backgroundImage = "url(" + window.logoURL + ")";
			} else {
				const docType = this._map.getDocType();
				const [iconClass, iconTooltip] = app.LOUtil.getDocumentLogoClass(docType);
				aItem.classList.add(iconClass);
				aItem.setAttribute('data-cooltip', iconTooltip);
			}

			if (this._menubarCont != null)
				this._menubarCont.insertBefore(liItem, this._menubarCont.firstChild);

			const $docLogo = $(aItem);
			$docLogo.bind('click', {self: this}, this._createDocument);
			$docLogo.bind('click', this._createDocument.bind(this));
		}
	}

	/**
       * Checks whether a given menu item (configuration object) should be visible.
       * @param menuItem - The menu item configuration.
       * @returns True if visible; otherwise false.
       */
	private _checkItemVisibility(menuItem: any): boolean {
		if (window.ThisIsAMobileApp && menuItem.mobileapp === false) {
			return false;
		}
		if (window.ThisIsTheiOSApp && menuItem.iosapp === false) {
			return false;
		}
		if (menuItem.id === 'about' && (window.L.DomUtil.get('about-dialog') === null)) {
			return false;
		}
		if (menuItem.id === 'fontworkgalleryfloater' && !this._isFileODF) {
			return false;
		}
		if (this._map.isReadOnlyMode() && menuItem.type === 'menu') {
			var found = this.options.allowedReadonlyMenus.includes(menuItem.id);
			if (!found && app.isRedlineManagementAllowed() && !this._map['wopi'].HideChangeTrackingControls)
				found = this.options.allowedRedlineManagementMenus.includes(menuItem.id);
			if (!found)
				return false;
		}
		if (this._map.isReadOnlyMode()) {
			switch (menuItem.id) {
			case 'last-mod':
			case 'save':
			case 'runmacro':
			case 'pagesetup':
			case 'watermark':
			case 'properties':
			case 'formattingmarks':
				return false;
			case 'insertcomment':
			case 'savecomments':
				if (!app.isCommentEditingAllowed()) {
					return false;
				}
			}
		}

		if (this._map.isEditMode()) {
			switch (menuItem.id) {
			case 'savecomments':
				return false;
			}
		}

		if (menuItem.id === 'runmacro' && !window.enableMacrosExecution)
			return false;

		if (menuItem.type === 'action') {
			if (((menuItem.id === 'rev-history' || menuItem.id === 'Rev-History') && !window.L.Params.revHistoryEnabled) ||
				(menuItem.id === 'closedocument' && !window.L.Params.closeButtonEnabled) ||
				(menuItem.id === 'latestupdates' && !window.enableWelcomeMessage)) {
				return false;
			}
		}

		if (menuItem.id === 'print' && this._map['wopi'].HidePrintOption)
			return false;

		if (menuItem.id === 'save' && this._map['wopi'].HideSaveOption)
			return false;

		if (menuItem.id && (menuItem.id === 'saveas' || menuItem.id.startsWith('saveas-')) && this._map['wopi'].UserCanNotWriteRelative)
			return false;

		if (menuItem.id && (menuItem.id.startsWith('exportas')) && this._map['wopi'].UserCanNotWriteRelative)
			return false;

		if (menuItem.id && menuItem.id === 'exportpdf' && window.ThisIsTheAndroidApp)
			return false;

		if ((menuItem.id === 'shareas' || menuItem.id === 'ShareAs') && !this._map['wopi'].EnableShare)
			return false;

		if (menuItem.id === 'insertgraphicremote' && !this._map['wopi'].EnableInsertRemoteImage)
			return false;

		if (menuItem.id === 'renamedocument' && !(this._map['wopi']._supportsRename() && this._map['wopi'].UserCanRename))
			return false;

		if (menuItem.id === 'insertgraphic' && this._map['wopi'].DisableInsertLocalImage)
			return false;

		if (menuItem.id === 'insertmultimedia' && this._map['wopi'].DisableInsertLocalImage)
			return false;

		if (menuItem.id === 'remotemultimedia' && !this._map['wopi'].EnableInsertRemoteFile)
			return false;

		if (menuItem.id && menuItem.id.startsWith('fullscreen-presentation') && this._map['wopi'].HideExportOption)
			return false;

		if (menuItem.id === 'repair' && this._map['wopi'].HideRepairOption)
			return false;

		if (menuItem.id === 'changesmenu' && this._map['wopi'].HideChangeTrackingControls)
			return false;

		if (menuItem.id === 'invertbackground' && !window.prefs.getBoolean('darkTheme'))
			return false;


		// Keep track of all 'downloadas-' options and register them as
		// export formats with docLayer which can then be publicly accessed unlike
		// this Menubar control for which there doesn't seem to be any easy way
		// to get access to.
		if (menuItem.id && menuItem.id.startsWith('downloadas-')) {
			var format = menuItem.id.substring('downloadas-'.length);

			const docType: string = this._map.getDocType();
			const extraFormats = docType === 'presentation' ? ['svg', 'bmp', 'gif', 'png', 'tiff']
				: [];

			const appNames: Record<string, string> = {
				'presentation': 'impress'
			};

			if (extraFormats.includes(format) && !window.extraExportFormats.includes(`${format}_${appNames[docType]}`)) {
				return false;
			}

			app.registerExportFormat(menuItem.name, format);

			if (this._map['wopi'].HideExportOption)
				return false;
		}

		if (menuItem.id && menuItem.id.startsWith('export')) {
			if (!menuItem.id.startsWith('exportas-')) {
				var format = menuItem.id.substring('export'.length);
				app.registerExportFormat(menuItem.name, format);
			}

			if (this._map['wopi'].HideExportOption)
				return false;
		}

		if (this._hiddenItems &&
		    $.inArray(menuItem.id, this._hiddenItems) !== -1)
			return false;

		if (menuItem.id === 'settings-dialog' && !window.wopiSettingBaseUrl)
			return false;

		return true;
	}

	/**
	 * Recursively creates menu DOM elements from a menu configuration.
	 * @param menu - Array of menu configuration objects.
	 * @returns An array of list item (LI) elements.
	 */
	private _createMenu(menu: any[]): HTMLElement[] {
		var itemList = [];
		var docType = this._map.getDocType();
		var isReadOnly = this._map.isReadOnlyMode();

		if (isReadOnly && !app.file.editComment) {
			this._hiddenItems.push('insert');
		}

		if (!window.documentSigningEnabled) {
			this._hiddenItems.push('signature');
		}

		for (var i in menu) {
			if (this._checkItemVisibility(menu[i]) === false)
				continue;

			var liItem = window.L.DomUtil.create('li', '');
			liItem.setAttribute('role', 'menuitem');
			if (menu[i].type === 'separator') {
				liItem.setAttribute('aria-hidden', 'true');
			}
			if (menu[i].id) {
				liItem.id = 'menu-' + menu[i].id;
				if (menu[i].id === 'closedocument' && isReadOnly) {
					// see corresponding css rule for readonly class usage
					window.L.DomUtil.addClass(liItem, 'readonly');
				}
			}
			var aItem = window.L.DomUtil.create('a', menu[i].disabled ? 'disabled' : '', liItem);
			if (menu[i].name !== undefined) {
				aItem.innerHTML = menu[i].name;
			} else if (menu[i].uno !== undefined) {
				aItem.innerHTML = _UNO(menu[i].uno, docType);
			} else {
				aItem.replaceChildren();
			}
			if (menu[i].uno && (JSDialog.ShortcutsUtil.hasShortcut(menu[i].uno) || JSDialog.ShortcutsUtil.hasShortcut(menu[i].id))) {
				aItem.innerHTML = JSDialog.ShortcutsUtil.getShortcut(aItem.innerHTML, menu[i].uno ? menu[i].uno : menu[i].id);
			}

			if (menu[i].type === 'menu') {
				var ulItem = window.L.DomUtil.create('ul', '', liItem);
				var subitemList = this._createMenu(menu[i].menu);
				if (!subitemList.length) {
					continue;
				}
				for (var idx in subitemList) {
					ulItem.appendChild(subitemList[idx]);
				}
				aItem.tabIndex = 0;
			} else if (menu[i].type === 'unocommand' || (!menu[i].type && menu[i].uno !== undefined)) {
				$(aItem).data('type', 'unocommand');
				$(aItem).data('uno', menu[i].uno);
				$(aItem).data('tag', menu[i].tag);
				aItem.tabIndex = 0;
			} else if (menu[i].type === 'separator') {
				$(aItem).addClass('separator');
				aItem.tabIndex = -1;
			} else if (menu[i].type === 'action') {
				if (menu[i].id == 'feedback' && !this._map.feedback)
					continue;
				$(aItem).data('type', 'action');
				$(aItem).data('id', menu[i].id);
				if (menu[i].uno !== undefined)
					$(aItem).data('uno', menu[i].uno);
				if (menu[i].unoid !== undefined)
					$(aItem).data('unoid', menu[i].unoid);
				aItem.tabIndex = 0;
			}

			if (menu[i].hidden == true)
				$(aItem).css('display', 'none');

			if (menu[i].tablet == false && window.mode.isTablet()) {
				$(aItem).css('display', 'none');
			}

			if (this._hiddenItems && $.inArray(menu[i].id, this._hiddenItems) !== -1) {
				$(aItem).css('display', 'none');
			}

			this._map.hideRestrictedItems(menu[i], aItem, aItem);
			this._map.disableLockedItem(menu[i], aItem, aItem);
			itemList.push(liItem);
		}

		return itemList;
	}

	/**
       * Returns a jQuery collection of all menu item elements.
       * @returns The menu items.
       */
	private _getItems(): JQuery<HTMLElement> | null {
		if (this._menubarCont == null)
			return null;
		const $cont = ($(this._menubarCont) as unknown) as JQuery<HTMLElement>;
		return $cont.children().children('ul').children('li').add($cont.children('li'));
	}

	/**
	 * Retrieves a menu item by its target ID.
	 * @param targetId - The target menu item ID.
	 * @returns The jQuery element if found; otherwise null.
	 */
	private _getItem(targetId: string): JQuery<HTMLElement> | null {
		var items = this._getItems();
		if (items == null)
			return null;
		var found = $(items).filter((_, item) => {
			const id = $(item).attr('id');
			if (id && id == 'menu-' + targetId) {
				return true;
			}
			return false;
		});
		return found.length ? found : null;
	}

	/**
	 * Checks whether a menu item with the given target ID exists.
	 * @param targetId - The target menu item ID.
	 * @returns True if exists; otherwise false.
	 */
	hasItem(targetId: string): boolean {
		return this._getItem(targetId) != null;
	}

	/**
       * Hides a menu item by its target ID.
       * @param targetId - The target menu item ID.
       */
	hideItem(targetId: string): boolean {
		var item = this._getItem(targetId);
		if (item) {
			if ($.inArray(targetId, this._hiddenItems) == -1)
				this._hiddenItems.push(targetId);
			$(item).css('display', 'none');
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Shows a menu item by its target ID.
	 * @param targetId - The target menu item ID.
	 */
	showItem(targetId: string): boolean {
		var item = this._getItem(targetId);
		if (item) {
			if ($.inArray(targetId, this._hiddenItems) !== -1)
				this._hiddenItems.splice(this._hiddenItems.indexOf(targetId), 1);
			$(item).css('display', '');
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Retrieves menu items associated with a specific UNO command.
	 * @param commandId - The UNO command.
	 * @returns The menu items or null.
	 */
	private _getItemsForCommand(commandId: string): JQuery<HTMLElement> | null {
		var items = this._getItems();
		if (items == null)
			return null;
		const isUno = commandId.startsWith('.uno:');
		var found = $(items).filter(function() {
			var item = $(this.children[0]);
			var type = item.data('type');
			var id = null;
			if (type == 'unocommand') {
				id = $(item).data('uno');
			} else if (type == 'action') {
				if (isUno) {
					id = $(item).data('unoid');
					if (!id) {
						id = $(item).data('uno');
					}
				}
				if (!id) {
					id = $(item).data('id');
				}
			}
			if (id && id == commandId) {
				return true;
			}
			return false;
		});
		return found.length ? found : null;
	}

	/**
	 * Hides all menu items corresponding to a given UNO command.
	 * @param targetId - The UNO command.
	 */
	hideUnoItem(targetId: string): boolean {
		var items = this._getItemsForCommand(targetId);
		if (items) {
			$(items).each((_, item) => {
				if ($.inArray(item.id, this._hiddenItems) == -1) {
					this._hiddenItems.push(item.id);
				}
			});
			$(items).css('display', 'none');
			return true;
		}
		return false;
	}

	/**
	 * Shows all menu items corresponding to a given UNO command.
	 * @param targetId - The UNO command.
	 */
	showUnoItem(targetId: string): boolean {
		var items = this._getItemsForCommand(targetId);
		if (items) {
			$(items).each((_, item) => {
				if ($.inArray(item.id, this._hiddenItems) !== -1) {
					this._hiddenItems.splice(this._hiddenItems.indexOf(item.id), 1);
				}
			});
			$(items).css('display', '');
			return true;
		}
		return false;
	}

	/**
	 * Initializes the menubar with a given menu configuration.
	 * @param menu - Array of menu configuration objects.
	 */
	private _initializeMenu(menu: any[]): void {
		this._isFileODF = app.LOUtil.isFileODF(this._map);
		var menuHtml = this._createMenu(menu);
		for (var i in menuHtml) {
			this._menubarCont?.appendChild(menuHtml[i]);
		}
	}

	/**
       * Generates the full menu structure (for mobile use).
       * @returns The full menu structure object.
       */
	generateFullMenuStructure(): any {
		var topMenu = {
			type : 'menubar',
			enabled : true,
			id : 'menubar',
			children : [] as any[]
		};
		var docType = ('mobile' + this._map.getDocType()) as keyof typeof this.options; // FIXME There should be a better structure for 'mobile<something>' in the options
		var items = this.options[docType];
		if (!Array.isArray(items))
			return topMenu;

		for (var i = 0; i < items.length; i++) {
			if (this._checkItemVisibility(items[i]) === true) {
				var item = this._generateMenuStructure(items[i], docType, false);
				if (item)
					topMenu.children.push(item);
			}
		}
		return topMenu;
	}

	/**
	 * Generates the insert menu structure for mobile.
	 * @returns The generated insert menu structure.
	 */
	generateInsertMenuStructure(): any {
		var docType = this._map.getDocType();
		var target = this.options['mobileInsertMenu'][docType];

		var menuStructure = this._generateMenuStructure(target, docType, true);
		return menuStructure;
	}

	/**
       * Recursively generates a menu structure object from a configuration item.
       * @param item - The menu configuration item.
       * @param docType - The document type.
       * @param mainMenu - Flag indicating if it is the main menu.
       * @returns The generated menu structure or undefined.
       */
	private _generateMenuStructure(item: any, docType: string, mainMenu: boolean): any {
		var itemType;
		if (mainMenu) {
			itemType = 'mainmenu';
		} else {
			if (item.mobileapp == true && !window.ThisIsAMobileApp)
				return undefined;
			if (item.mobileapp === false && window.ThisIsAMobileApp)
				return undefined;
			if (!item.menu) {
				itemType = 'menuitem';
			} else {
				itemType = 'submenu';
			}
		}

		if (item.id === 'feedback' && !this._map.feedback)
			return undefined;

		var itemName;
		if (item.name)
			itemName = item.name;
		else if (item.uno)
			itemName = _UNO(item.uno, docType);
		else
			return undefined; // separator

		var menuStructure = {
			id : item.id,
			type : itemType,
			enabled : !item.disabled,
			text : itemName,
			command : item.uno,
			executionType : item.type,
			data : item,
			'checked' : false,
			children : [] as any[]
		};

		// Checked state for insert header / footer
		var insertHeaderString = '.uno:InsertPageHeader?PageStyle:string=';
		var insertFooterString = '.uno:InsertPageFooter?PageStyle:string=';
		if (item.uno && (item.uno.startsWith(insertHeaderString) || item.uno.startsWith(insertFooterString))) {
			var style = decodeURIComponent(item.uno.slice(item.uno.search('=') + 1));
			style = style.slice(0, style.length - 1);
			var shortUno = item.uno.slice(0, item.uno.search('\\?'));
			var state = this._map['stateChangeHandler'].getItemValue(shortUno);
			if (state && state[style]) {
				menuStructure['checked'] = true;
			}
		} else if (item.uno === '.uno:TrackChanges' ||
			item.uno === '.uno:ShowTrackedChanges' ||
			item.uno === '.uno:ControlCodes' ||
			item.uno === '.uno:SpellOnline' ||
			item.uno === '.uno:ShowResolvedAnnotations' ||
			item.uno === '.uno:FreezePanes') {
			if (this._map['stateChangeHandler'].getItemValue(item.uno) === 'true') {
				menuStructure['checked'] = true;
			}
		} else if (item.id === 'togglea11ystate') {
			if (window.prefs.getBoolean('accessibilityState'))
				menuStructure['checked'] = true;
		} else if (item.id === 'toggledarktheme' && window.prefs.getBoolean('darkTheme')) {
			menuStructure['checked'] = true;
		} else if (item.id === 'invertbackground' && window.prefs.getBoolean('darkTheme')) {
			menuStructure['checked'] = !this._map.uiManager.isBackgroundDark();
		}

		if (item.menu)
		{
			for (var i = 0; i < item.menu.length; i++) {
				if (this._checkItemVisibility(item.menu[i]) === true) {
					var element = this._generateMenuStructure(item.menu[i], docType, false);
					if (element)
						menuStructure['children'].push(element);
				}
			}
		}
		return menuStructure;
	}

	/**
	 * Recursively searches for a submenu by name.
	 * @param menuTarget - The menu structure.
	 * @param nameString - The name to search for.
	 * @returns The found submenu or null.
	 */
	private _getMenuItems(menu: any): any {
		return $(menu).children().children('a').not('.has-submenu');
	}

	private _findSubMenuByName(menuTarget: any, nameString: string): any {
		if (menuTarget.name === nameString)
			return menuTarget;

		if (menuTarget.menu)
		{
			for (var i = 0; i < menuTarget.menu.length; i++) {
				var foundItem = this._findSubMenuByName(menuTarget.menu[i], nameString);
				if (foundItem)
					return foundItem;
			}
		}
		return null;
	}

	/**
	 * Initializes the modification indicator in the menubar.
	 * @param lastmodtime - The last modification time.
	 */
	private _onInitModificationIndicator(lastmodtime: any): void {
		var lastModButton = window.L.DomUtil.get('menu-last-mod');
		if (lastModButton !== null && lastModButton !== undefined
			&& lastModButton.firstChild
			&& lastModButton.firstChild.innerHTML !== null
			&& lastModButton.firstChild.childElementCount == 0) {
			if (lastmodtime == null) {
				// No modification time -> hide the indicator
				window.L.DomUtil.setStyle(lastModButton, 'display', 'none');
				return;
			}
			var mainSpan = document.createElement('span');
			this.lastModIndicator = document.createElement('span');
			mainSpan.appendChild(this.lastModIndicator);

			// Replace menu button body with new content
			lastModButton.firstChild.replaceChildren();
			lastModButton.firstChild.appendChild(mainSpan);

			if (window.L.Params.revHistoryEnabled) {
				window.L.DomUtil.setStyle(lastModButton, 'cursor', 'pointer');
			}

			this._map.fire('modificationindicatorinitialized');
		}
	}

	/**
       * Updates the modification indicator.
       * @param e - Event data containing the new modification time.
       */
	private _onUpdateModificationIndicator(e: any): void {
		if (this.lastModIndicator !== null && this.lastModIndicator !== undefined) {
			this.lastModIndicator.textContent = e.lastSaved;
		}
	}
}
