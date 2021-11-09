/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once


namespace pcr
{


    constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";
    constexpr OUStringLiteral PROPERTY_INTROSPECTEDOBJECT = u"IntrospectedObject";
    constexpr OUStringLiteral PROPERTY_CURRENTPAGE = u"CurrentPage";
    constexpr OUStringLiteral PROPERTY_CONTROLCONTEXT = u"ControlContext";

    // properties
    constexpr OUStringLiteral PROPERTY_CLASSID = u"ClassId";
    constexpr OUStringLiteral PROPERTY_CONTROLLABEL = u"LabelControl";
    constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
    constexpr OUStringLiteral PROPERTY_TABINDEX = u"TabIndex";
    constexpr OUStringLiteral PROPERTY_WHEEL_BEHAVIOR = u"MouseWheelBehavior";
    constexpr OUStringLiteral PROPERTY_TAG = u"Tag";
    constexpr OUStringLiteral PROPERTY_NAME = u"Name";
    constexpr OUStringLiteral PROPERTY_GROUP_NAME = u"GroupName";
    constexpr OUStringLiteral PROPERTY_VALUE = u"Value";
    constexpr OUStringLiteral PROPERTY_TEXT = u"Text";
    constexpr OUStringLiteral PROPERTY_NAVIGATION = u"NavigationBarMode";
    constexpr OUStringLiteral PROPERTY_CYCLE = u"Cycle";
    constexpr OUStringLiteral PROPERTY_CONTROLSOURCE = u"DataField";
    constexpr OUStringLiteral PROPERTY_INPUT_REQUIRED = u"InputRequired";
    constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
    constexpr OUStringLiteral PROPERTY_ENABLE_VISIBLE = u"EnableVisible";
    constexpr OUStringLiteral PROPERTY_READONLY = u"ReadOnly";
    #define PROPERTY_ISREADONLY              "IsReadOnly"
    constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
    constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
    constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
    constexpr OUStringLiteral PROPERTY_WORDBREAK = u"WordBreak";
    constexpr OUStringLiteral PROPERTY_TARGET_URL = u"TargetURL";
    constexpr OUStringLiteral PROPERTY_TARGET_FRAME = u"TargetFrame";
    constexpr OUStringLiteral PROPERTY_MAXTEXTLEN = u"MaxTextLen";
    constexpr OUStringLiteral PROPERTY_EDITMASK = u"EditMask";
    constexpr OUStringLiteral PROPERTY_SPIN = u"Spin";
    constexpr OUStringLiteral PROPERTY_TRISTATE = u"TriState";
    constexpr OUStringLiteral PROPERTY_HIDDEN_VALUE = u"HiddenValue";
    constexpr OUStringLiteral PROPERTY_BUTTONTYPE = u"ButtonType";
    constexpr OUStringLiteral PROPERTY_XFORMS_BUTTONTYPE = u"XFormsButtonType";
    constexpr OUStringLiteral PROPERTY_STRINGITEMLIST = u"StringItemList";
    constexpr OUStringLiteral PROPERTY_TYPEDITEMLIST = u"TypedItemList";
    constexpr OUStringLiteral PROPERTY_DEFAULT_TEXT = u"DefaultText";
    constexpr OUStringLiteral PROPERTY_DEFAULT_STATE = u"DefaultState";
    constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
    constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
    constexpr OUStringLiteral PROPERTY_SUBMIT_ACTION = u"SubmitAction";
    constexpr OUStringLiteral PROPERTY_SUBMIT_TARGET = u"SubmitTarget";
    constexpr OUStringLiteral PROPERTY_SUBMIT_METHOD = u"SubmitMethod";
    constexpr OUStringLiteral PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding";
    constexpr OUStringLiteral PROPERTY_IMAGE_URL = u"ImageURL";
    constexpr OUStringLiteral PROPERTY_GRAPHIC = u"Graphic";
    constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
    constexpr OUStringLiteral PROPERTY_LISTSOURCETYPE = u"ListSourceType";
    constexpr OUStringLiteral PROPERTY_LISTSOURCE = u"ListSource";
    constexpr OUStringLiteral PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection";
    constexpr OUStringLiteral PROPERTY_MULTISELECTION = u"MultiSelection";
    constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
    constexpr OUStringLiteral PROPERTY_VERTICAL_ALIGN = u"VerticalAlign";
    constexpr OUStringLiteral PROPERTY_DEFAULT_DATE = u"DefaultDate";
    constexpr OUStringLiteral PROPERTY_DEFAULT_TIME = u"DefaultTime";
    constexpr OUStringLiteral PROPERTY_DEFAULT_VALUE = u"DefaultValue";
    constexpr OUStringLiteral PROPERTY_DECIMAL_ACCURACY = u"DecimalAccuracy";
    constexpr OUStringLiteral PROPERTY_REFVALUE = u"RefValue";
    constexpr OUStringLiteral PROPERTY_UNCHECKEDREFVALUE = u"SecondaryRefValue";
    constexpr OUStringLiteral PROPERTY_VALUEMIN = u"ValueMin";
    constexpr OUStringLiteral PROPERTY_VALUEMAX = u"ValueMax";
    constexpr OUStringLiteral PROPERTY_STRICTFORMAT = u"StrictFormat";
    constexpr OUStringLiteral PROPERTY_ALLOWADDITIONS = u"AllowInserts";
    constexpr OUStringLiteral PROPERTY_ALLOWEDITS = u"AllowUpdates";
    constexpr OUStringLiteral PROPERTY_ALLOWDELETIONS = u"AllowDeletes";
    constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
    constexpr OUStringLiteral PROPERTY_LITERALMASK = u"LiteralMask";
    constexpr OUStringLiteral PROPERTY_VALUESTEP = u"ValueStep";
    constexpr OUStringLiteral PROPERTY_SHOWTHOUSANDSEP = u"ShowThousandsSeparator";
    constexpr OUStringLiteral PROPERTY_CURRENCYSYMBOL = u"CurrencySymbol";
    constexpr OUStringLiteral PROPERTY_DATEFORMAT = u"DateFormat";
    constexpr OUStringLiteral PROPERTY_DATEMIN = u"DateMin";
    constexpr OUStringLiteral PROPERTY_DATEMAX = u"DateMax";
    constexpr OUStringLiteral PROPERTY_TIMEFORMAT = u"TimeFormat";
    constexpr OUStringLiteral PROPERTY_TIMEMIN = u"TimeMin";
    constexpr OUStringLiteral PROPERTY_TIMEMAX = u"TimeMax";
    constexpr OUStringLiteral PROPERTY_LINECOUNT = u"LineCount";
    constexpr OUStringLiteral PROPERTY_BOUNDCOLUMN = u"BoundColumn";
    constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor";
    constexpr OUStringLiteral PROPERTY_FILLCOLOR = u"FillColor";
    constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
    constexpr OUStringLiteral PROPERTY_LINECOLOR = u"LineColor";
    constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
    constexpr OUStringLiteral PROPERTY_ICONSIZE = u"IconSize";
    constexpr OUStringLiteral PROPERTY_DROPDOWN = u"Dropdown";
    constexpr OUStringLiteral PROPERTY_HSCROLL = u"HScroll";
    constexpr OUStringLiteral PROPERTY_VSCROLL = u"VScroll";
    constexpr OUStringLiteral PROPERTY_SHOW_SCROLLBARS = u"ShowScrollbars";
    constexpr OUStringLiteral PROPERTY_TABSTOP = u"Tabstop";
    constexpr OUStringLiteral PROPERTY_AUTOCOMPLETE = u"Autocomplete";
    constexpr OUStringLiteral PROPERTY_PRINTABLE = u"Printable";
    constexpr OUStringLiteral PROPERTY_ECHO_CHAR = u"EchoChar";
    constexpr OUStringLiteral PROPERTY_ROWHEIGHT = u"RowHeight";
    constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
    constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
    constexpr OUStringLiteral PROPERTY_FONT_NAME = u"FontName";
    constexpr OUStringLiteral PROPERTY_FONT_STYLENAME = u"FontStyleName";
    constexpr OUStringLiteral PROPERTY_FONT_FAMILY = u"FontFamily";
    constexpr OUStringLiteral PROPERTY_FONT_CHARSET = u"FontCharset";
    constexpr OUStringLiteral PROPERTY_FONT_HEIGHT = u"FontHeight";
    constexpr OUStringLiteral PROPERTY_FONT_WEIGHT = u"FontWeight";
    constexpr OUStringLiteral PROPERTY_FONT_SLANT = u"FontSlant";
    constexpr OUStringLiteral PROPERTY_FONT_UNDERLINE = u"FontUnderline";
    constexpr OUStringLiteral PROPERTY_FONT_STRIKEOUT = u"FontStrikeout";
    constexpr OUStringLiteral PROPERTY_FONT_RELIEF = u"FontRelief";
    constexpr OUStringLiteral PROPERTY_FONT_EMPHASIS_MARK = u"FontEmphasisMark";
    constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
    constexpr OUStringLiteral PROPERTY_HELPURL = u"HelpURL";
    constexpr OUStringLiteral PROPERTY_RECORDMARKER = u"HasRecordMarker";
    constexpr OUStringLiteral PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault";
    constexpr OUStringLiteral PROPERTY_EFFECTIVE_MIN = u"EffectiveMin";
    constexpr OUStringLiteral PROPERTY_EFFECTIVE_MAX = u"EffectiveMax";
    constexpr OUStringLiteral PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal";
    constexpr OUStringLiteral PROPERTY_CURRSYM_POSITION = u"PrependCurrencySymbol";
    constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
    constexpr OUStringLiteral PROPERTY_COMMANDTYPE = u"CommandType";
    constexpr OUStringLiteral PROPERTY_INSERTONLY = u"IgnoreResult";
    constexpr OUStringLiteral PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing";
    constexpr OUStringLiteral PROPERTY_TITLE = u"Title";
    constexpr OUStringLiteral PROPERTY_SORT = u"Order";
    constexpr OUStringLiteral PROPERTY_DATASOURCE = u"DataSourceName";
    constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
    constexpr OUStringLiteral PROPERTY_DEFAULTBUTTON = u"DefaultButton";
    constexpr OUStringLiteral PROPERTY_LISTINDEX = u"ListIndex";
    constexpr OUStringLiteral PROPERTY_HEIGHT = u"Height";
    constexpr OUStringLiteral PROPERTY_HASNAVIGATION = u"HasNavigationBar";
    constexpr OUStringLiteral PROPERTY_POSITIONX = u"PositionX";
    constexpr OUStringLiteral PROPERTY_POSITIONY = u"PositionY";
    constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
    constexpr OUStringLiteral PROPERTY_STEP = u"Step";
    constexpr OUStringLiteral PROPERTY_WORDLINEMODE = u"FontWordLineMode";
    constexpr OUStringLiteral PROPERTY_PROGRESSVALUE = u"ProgressValue";
    constexpr OUStringLiteral PROPERTY_PROGRESSVALUE_MIN = u"ProgressValueMin";
    constexpr OUStringLiteral PROPERTY_PROGRESSVALUE_MAX = u"ProgressValueMax";
    constexpr OUStringLiteral PROPERTY_SCROLLVALUE = u"ScrollValue";
    constexpr OUStringLiteral PROPERTY_DEFAULT_SCROLLVALUE = u"DefaultScrollValue";
    constexpr OUStringLiteral PROPERTY_SCROLLVALUE_MIN = u"ScrollValueMin";
    constexpr OUStringLiteral PROPERTY_SCROLLVALUE_MAX = u"ScrollValueMax";
    constexpr OUStringLiteral PROPERTY_SCROLL_WIDTH = u"ScrollWidth";
    constexpr OUStringLiteral PROPERTY_SCROLL_HEIGHT = u"ScrollHeight";
    constexpr OUStringLiteral PROPERTY_SCROLL_TOP = u"ScrollTop";
    constexpr OUStringLiteral PROPERTY_SCROLL_LEFT = u"ScrollLeft";
    constexpr OUStringLiteral PROPERTY_LINEINCREMENT = u"LineIncrement";
    constexpr OUStringLiteral PROPERTY_BLOCKINCREMENT = u"BlockIncrement";
    constexpr OUStringLiteral PROPERTY_VISIBLESIZE = u"VisibleSize";
    constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
    constexpr OUStringLiteral PROPERTY_IMAGEPOSITION = u"ImagePosition";
    constexpr OUStringLiteral PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection";
    constexpr OUStringLiteral PROPERTY_ACTIVECOMMAND = u"ActiveCommand";
    constexpr OUStringLiteral PROPERTY_DATE = u"Date";
    constexpr OUStringLiteral PROPERTY_STATE = u"State";
    constexpr OUStringLiteral PROPERTY_TIME = u"Time";
    constexpr OUStringLiteral PROPERTY_SCALEIMAGE = u"ScaleImage";
    constexpr OUStringLiteral PROPERTY_SCALE_MODE = u"ScaleMode";
    constexpr OUStringLiteral PROPERTY_PUSHBUTTONTYPE = u"PushButtonType";
    constexpr OUStringLiteral PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue";
    constexpr OUStringLiteral PROPERTY_SELECTEDITEMS = u"SelectedItems";
    constexpr OUStringLiteral PROPERTY_REPEAT = u"Repeat";
    constexpr OUStringLiteral PROPERTY_REPEAT_DELAY = u"RepeatDelay";
    constexpr OUStringLiteral PROPERTY_SYMBOLCOLOR = u"SymbolColor";
    constexpr OUStringLiteral PROPERTY_SPINVALUE = u"SpinValue";
    constexpr OUStringLiteral PROPERTY_SPINVALUE_MIN = u"SpinValueMin";
    constexpr OUStringLiteral PROPERTY_SPINVALUE_MAX = u"SpinValueMax";
    constexpr OUStringLiteral PROPERTY_DEFAULT_SPINVALUE = u"DefaultSpinValue";
    constexpr OUStringLiteral PROPERTY_SPININCREMENT = u"SpinIncrement";
    constexpr OUStringLiteral PROPERTY_SHOW_POSITION = u"ShowPosition";
    constexpr OUStringLiteral PROPERTY_SHOW_NAVIGATION = u"ShowNavigation";
    constexpr OUStringLiteral PROPERTY_SHOW_RECORDACTIONS = u"ShowRecordActions";
    constexpr OUStringLiteral PROPERTY_SHOW_FILTERSORT = u"ShowFilterSort";
    constexpr OUStringLiteral PROPERTY_LINEEND_FORMAT = u"LineEndFormat";
    constexpr OUStringLiteral PROPERTY_DECORATION = u"Decoration";
    constexpr OUStringLiteral PROPERTY_NOLABEL = u"NoLabel";
    constexpr OUStringLiteral PROPERTY_URL = u"URL";

    constexpr OUStringLiteral PROPERTY_SELECTION_TYPE = u"SelectionType";
    constexpr OUStringLiteral PROPERTY_ROOT_DISPLAYED = u"RootDisplayed";
    constexpr OUStringLiteral PROPERTY_SHOWS_HANDLES = u"ShowsHandles";
    constexpr OUStringLiteral PROPERTY_SHOWS_ROOT_HANDLES = u"ShowsRootHandles";
    constexpr OUStringLiteral PROPERTY_EDITABLE = u"Editable";
    constexpr OUStringLiteral PROPERTY_INVOKES_STOP_NOT_EDITING = u"InvokesStopNodeEditing";

    #define PROPERTY_HARDLINEBREAKS         "HardLineBreaks"

    constexpr OUStringLiteral PROPERTY_TOGGLE = u"Toggle";
    constexpr OUStringLiteral PROPERTY_FOCUSONCLICK = u"FocusOnClick";
    constexpr OUStringLiteral PROPERTY_HIDEINACTIVESELECTION = u"HideInactiveSelection";
    constexpr OUStringLiteral PROPERTY_VISUALEFFECT = u"VisualEffect";
    constexpr OUStringLiteral PROPERTY_BORDERCOLOR = u"BorderColor";

    constexpr OUStringLiteral PROPERTY_ADDRESS = u"Address";
    constexpr OUStringLiteral PROPERTY_REFERENCE_SHEET = u"ReferenceSheet";
    constexpr OUStringLiteral PROPERTY_UI_REPRESENTATION = u"UserInterfaceRepresentation";

    constexpr OUStringLiteral PROPERTY_XML_DATA_MODEL = u"XMLDataModel";
    constexpr OUStringLiteral PROPERTY_BINDING_NAME = u"BindingName";
    constexpr OUStringLiteral PROPERTY_BIND_EXPRESSION = u"BindingExpression";
    constexpr OUStringLiteral PROPERTY_LIST_BINDING = u"ListBinding";
    constexpr OUStringLiteral PROPERTY_XSD_REQUIRED = u"RequiredExpression";
    constexpr OUStringLiteral PROPERTY_XSD_RELEVANT = u"RelevantExpression";
    constexpr OUStringLiteral PROPERTY_XSD_READONLY = u"ReadonlyExpression";
    constexpr OUStringLiteral PROPERTY_XSD_CONSTRAINT = u"ConstraintExpression";
    constexpr OUStringLiteral PROPERTY_XSD_CALCULATION = u"CalculateExpression";
    constexpr OUStringLiteral PROPERTY_XSD_DATA_TYPE = u"Type";
    constexpr OUStringLiteral PROPERTY_XSD_WHITESPACES = u"WhiteSpace";
    constexpr OUStringLiteral PROPERTY_XSD_PATTERN = u"Pattern";
    constexpr OUStringLiteral PROPERTY_XSD_LENGTH = u"Length";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_LENGTH = u"MinLength";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_LENGTH = u"MaxLength";
    constexpr OUStringLiteral PROPERTY_XSD_TOTAL_DIGITS = u"TotalDigits";
    constexpr OUStringLiteral PROPERTY_XSD_FRACTION_DIGITS = u"FractionDigits";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_INT = u"MaxInclusiveInt";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_INT = u"MaxExclusiveInt";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_INT = u"MinInclusiveInt";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_INT = u"MinExclusiveInt";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE = u"MaxInclusiveDouble";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE = u"MaxExclusiveDouble";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE = u"MinInclusiveDouble";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE = u"MinExclusiveDouble";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE = u"MaxInclusiveDate";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE = u"MaxExclusiveDate";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE = u"MinInclusiveDate";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE = u"MinExclusiveDate";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_TIME = u"MaxInclusiveTime";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_TIME = u"MaxExclusiveTime";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_TIME = u"MinInclusiveTime";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_TIME = u"MinExclusiveTime";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME = u"MaxInclusiveDateTime";
    constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME = u"MaxExclusiveDateTime";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME = u"MinInclusiveDateTime";
    constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME = u"MinExclusiveDateTime";
    constexpr OUStringLiteral PROPERTY_SUBMISSION_ID = u"SubmissionID";
    constexpr OUStringLiteral PROPERTY_BINDING_ID = u"BindingID";
    #define PROPERTY_ID                     "ID"
    constexpr OUStringLiteral PROPERTY_WRITING_MODE = u"WritingMode";
    constexpr OUStringLiteral PROPERTY_TEXT_ANCHOR_TYPE = u"TextAnchorType";
    constexpr OUStringLiteral PROPERTY_SHEET_ANCHOR_TYPE = u"SheetAnchorType";
    constexpr OUStringLiteral PROPERTY_ANCHOR_TYPE = u"AnchorType";
    constexpr OUStringLiteral PROPERTY_ANCHOR = u"Anchor";
    constexpr OUStringLiteral PROPERTY_IS_VISIBLE = u"IsVisible";

    constexpr OUStringLiteral PROPERTY_MODEL = u"Model";

    constexpr OUStringLiteral PROPERTY_CELL_EXCHANGE_TYPE = u"ExchangeSelectionIndex";
    constexpr OUStringLiteral PROPERTY_BOUND_CELL = u"BoundCell";
    constexpr OUStringLiteral PROPERTY_LIST_CELL_RANGE = u"CellRange";
    #define PROPERTY_AUTOLINEBREAK          "AutomaticLineBreak"
    constexpr OUStringLiteral PROPERTY_TEXTTYPE = u"TextType";
    constexpr OUStringLiteral PROPERTY_RICHTEXT = u"RichText";
    constexpr OUStringLiteral PROPERTY_ROWSET = u"RowSet";
    constexpr OUStringLiteral PROPERTY_SELECTIONMODEL = u"SelectionModel";
    constexpr OUStringLiteral PROPERTY_USEGRIDLINE = u"UseGridLines";
    constexpr OUStringLiteral PROPERTY_GRIDLINECOLOR = u"GridLineColor";
    constexpr OUStringLiteral PROPERTY_SHOWCOLUMNHEADER = u"ShowColumnHeader";
    constexpr OUStringLiteral PROPERTY_SHOWROWHEADER = u"ShowRowHeader";
    constexpr OUStringLiteral PROPERTY_HEADERBACKGROUNDCOLOR = u"HeaderBackgroundColor";
    constexpr OUStringLiteral PROPERTY_HEADERTEXTCOLOR = u"HeaderTextColor";
    constexpr OUStringLiteral PROPERTY_ACTIVESELECTIONBACKGROUNDCOLOR = u"ActiveSelectionBackgroundColor";
    constexpr OUStringLiteral PROPERTY_ACTIVESELECTIONTEXTCOLOR = u"ActiveSelectionTextColor";
    constexpr OUStringLiteral PROPERTY_INACTIVESELECTIONBACKGROUNDCOLOR = u"InactiveSelectionBackgroundColor";
    constexpr OUStringLiteral PROPERTY_INACTIVESELECTIONTEXTCOLOR = u"InactiveSelectionTextColor";

    // services
    #define SERVICE_COMPONENT_GROUPBOX      "com.sun.star.form.component.GroupBox"
    #define SERVICE_COMPONENT_FIXEDTEXT     "com.sun.star.form.component.FixedText"
    constexpr OUStringLiteral SERVICE_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";

    constexpr OUStringLiteral SERVICE_TEXT_DOCUMENT = u"com.sun.star.text.TextDocument";
    constexpr OUStringLiteral SERVICE_WEB_DOCUMENT = u"com.sun.star.text.WebDocument";
    constexpr OUStringLiteral SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument";
    constexpr OUStringLiteral SERVICE_DRAWING_DOCUMENT = u"com.sun.star.drawing.DrawingDocument";
    constexpr OUStringLiteral SERVICE_PRESENTATION_DOCUMENT = u"com.sun.star.presentation.PresentationDocument";

    constexpr OUStringLiteral SERVICE_SHEET_CELL_BINDING = u"com.sun.star.table.CellValueBinding";
    constexpr OUStringLiteral SERVICE_SHEET_CELL_INT_BINDING = u"com.sun.star.table.ListPositionCellBinding";
    constexpr OUStringLiteral SERVICE_SHEET_CELLRANGE_LISTSOURCE = u"com.sun.star.table.CellRangeListSource";
    constexpr OUStringLiteral SERVICE_ADDRESS_CONVERSION = u"com.sun.star.table.CellAddressConversion";
    constexpr OUStringLiteral SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion";


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
