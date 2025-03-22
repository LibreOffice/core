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

#include <rtl/ustring.hxx>

inline constexpr OUString PROPERTY_DEFAULTCONTROL = u"DefaultControl"_ustr;
inline constexpr OUString PROPERTY_INTROSPECTEDOBJECT = u"IntrospectedObject"_ustr;
inline constexpr OUString PROPERTY_CURRENTPAGE = u"CurrentPage"_ustr;
inline constexpr OUString PROPERTY_CONTROLCONTEXT = u"ControlContext"_ustr;

// properties
inline constexpr OUString PROPERTY_CLASSID = u"ClassId"_ustr;
inline constexpr OUString PROPERTY_CONTROLLABEL = u"LabelControl"_ustr;
inline constexpr OUString PROPERTY_LABEL = u"Label"_ustr;
inline constexpr OUString PROPERTY_TABINDEX = u"TabIndex"_ustr;
inline constexpr OUString PROPERTY_WHEEL_BEHAVIOR = u"MouseWheelBehavior"_ustr;
inline constexpr OUString PROPERTY_TAG = u"Tag"_ustr;
inline constexpr OUString PROPERTY_NAME = u"Name"_ustr;
inline constexpr OUString PROPERTY_GROUP_NAME = u"GroupName"_ustr;
inline constexpr OUString PROPERTY_VALUE = u"Value"_ustr;
inline constexpr OUString PROPERTY_TEXT = u"Text"_ustr;
inline constexpr OUString PROPERTY_NAVIGATION = u"NavigationBarMode"_ustr;
inline constexpr OUString PROPERTY_CYCLE = u"Cycle"_ustr;
inline constexpr OUString PROPERTY_CONTROLSOURCE = u"DataField"_ustr;
inline constexpr OUString PROPERTY_INPUT_REQUIRED = u"InputRequired"_ustr;
inline constexpr OUString PROPERTY_ENABLED = u"Enabled"_ustr;
inline constexpr OUString PROPERTY_ENABLE_VISIBLE = u"EnableVisible"_ustr;
inline constexpr OUString PROPERTY_READONLY = u"ReadOnly"_ustr;
inline constexpr OUString PROPERTY_FILTER = u"Filter"_ustr;
inline constexpr OUString PROPERTY_WIDTH = u"Width"_ustr;
inline constexpr OUString PROPERTY_MULTILINE = u"MultiLine"_ustr;
inline constexpr OUString PROPERTY_WORDBREAK = u"WordBreak"_ustr;
inline constexpr OUString PROPERTY_TARGET_URL = u"TargetURL"_ustr;
inline constexpr OUString PROPERTY_TARGET_FRAME = u"TargetFrame"_ustr;
inline constexpr OUString PROPERTY_MAXTEXTLEN = u"MaxTextLen"_ustr;
inline constexpr OUString PROPERTY_EDITMASK = u"EditMask"_ustr;
inline constexpr OUString PROPERTY_SPIN = u"Spin"_ustr;
inline constexpr OUString PROPERTY_TRISTATE = u"TriState"_ustr;
inline constexpr OUString PROPERTY_HIDDEN_VALUE = u"HiddenValue"_ustr;
inline constexpr OUString PROPERTY_BUTTONTYPE = u"ButtonType"_ustr;
inline constexpr OUString PROPERTY_XFORMS_BUTTONTYPE = u"XFormsButtonType"_ustr;
inline constexpr OUString PROPERTY_STRINGITEMLIST = u"StringItemList"_ustr;
inline constexpr OUString PROPERTY_TYPEDITEMLIST = u"TypedItemList"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_TEXT = u"DefaultText"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_STATE = u"DefaultState"_ustr;
inline constexpr OUString PROPERTY_FORMATKEY = u"FormatKey"_ustr;
inline constexpr OUString PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_ACTION = u"SubmitAction"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_TARGET = u"SubmitTarget"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_METHOD = u"SubmitMethod"_ustr;
inline constexpr OUString PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding"_ustr;
inline constexpr OUString PROPERTY_IMAGE_URL = u"ImageURL"_ustr;
inline constexpr OUString PROPERTY_GRAPHIC = u"Graphic"_ustr;
inline constexpr OUString PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull"_ustr;
inline constexpr OUString PROPERTY_LISTSOURCETYPE = u"ListSourceType"_ustr;
inline constexpr OUString PROPERTY_LISTSOURCE = u"ListSource"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection"_ustr;
inline constexpr OUString PROPERTY_MULTISELECTION = u"MultiSelection"_ustr;
inline constexpr OUString PROPERTY_ALIGN = u"Align"_ustr;
inline constexpr OUString PROPERTY_VERTICAL_ALIGN = u"VerticalAlign"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_DATE = u"DefaultDate"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_TIME = u"DefaultTime"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_VALUE = u"DefaultValue"_ustr;
inline constexpr OUString PROPERTY_DECIMAL_ACCURACY = u"DecimalAccuracy"_ustr;
inline constexpr OUString PROPERTY_REFVALUE = u"RefValue"_ustr;
inline constexpr OUString PROPERTY_UNCHECKEDREFVALUE = u"SecondaryRefValue"_ustr;
inline constexpr OUString PROPERTY_VALUEMIN = u"ValueMin"_ustr;
inline constexpr OUString PROPERTY_VALUEMAX = u"ValueMax"_ustr;
inline constexpr OUString PROPERTY_STRICTFORMAT = u"StrictFormat"_ustr;
inline constexpr OUString PROPERTY_ALLOWADDITIONS = u"AllowInserts"_ustr;
inline constexpr OUString PROPERTY_ALLOWEDITS = u"AllowUpdates"_ustr;
inline constexpr OUString PROPERTY_ALLOWDELETIONS = u"AllowDeletes"_ustr;
inline constexpr OUString PROPERTY_MASTERFIELDS = u"MasterFields"_ustr;
inline constexpr OUString PROPERTY_LITERALMASK = u"LiteralMask"_ustr;
inline constexpr OUString PROPERTY_VALUESTEP = u"ValueStep"_ustr;
inline constexpr OUString PROPERTY_SHOWTHOUSANDSEP = u"ShowThousandsSeparator"_ustr;
inline constexpr OUString PROPERTY_CURRENCYSYMBOL = u"CurrencySymbol"_ustr;
inline constexpr OUString PROPERTY_DATEFORMAT = u"DateFormat"_ustr;
inline constexpr OUString PROPERTY_DATEMIN = u"DateMin"_ustr;
inline constexpr OUString PROPERTY_DATEMAX = u"DateMax"_ustr;
inline constexpr OUString PROPERTY_TIMEFORMAT = u"TimeFormat"_ustr;
inline constexpr OUString PROPERTY_TIMEMIN = u"TimeMin"_ustr;
inline constexpr OUString PROPERTY_TIMEMAX = u"TimeMax"_ustr;
inline constexpr OUString PROPERTY_LINECOUNT = u"LineCount"_ustr;
inline constexpr OUString PROPERTY_BOUNDCOLUMN = u"BoundColumn"_ustr;
inline constexpr OUString PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor"_ustr;
inline constexpr OUString PROPERTY_FILLCOLOR = u"FillColor"_ustr;
inline constexpr OUString PROPERTY_TEXTCOLOR = u"TextColor"_ustr;
inline constexpr OUString PROPERTY_LINECOLOR = u"LineColor"_ustr;
inline constexpr OUString PROPERTY_BORDER = u"Border"_ustr;
inline constexpr OUString PROPERTY_ICONSIZE = u"IconSize"_ustr;
inline constexpr OUString PROPERTY_DROPDOWN = u"Dropdown"_ustr;
inline constexpr OUString PROPERTY_HSCROLL = u"HScroll"_ustr;
inline constexpr OUString PROPERTY_VSCROLL = u"VScroll"_ustr;
inline constexpr OUString PROPERTY_SHOW_SCROLLBARS = u"ShowScrollbars"_ustr;
inline constexpr OUString PROPERTY_TABSTOP = u"Tabstop"_ustr;
inline constexpr OUString PROPERTY_AUTOCOMPLETE = u"Autocomplete"_ustr;
inline constexpr OUString PROPERTY_PRINTABLE = u"Printable"_ustr;
inline constexpr OUString PROPERTY_PRIVILEGES = u"Privileges"_ustr;
inline constexpr OUString PROPERTY_ECHO_CHAR = u"EchoChar"_ustr;
inline constexpr OUString PROPERTY_ROWHEIGHT = u"RowHeight"_ustr;
inline constexpr OUString PROPERTY_HELPTEXT = u"HelpText"_ustr;
inline constexpr OUString PROPERTY_FONT = u"FontDescriptor"_ustr;
inline constexpr OUString PROPERTY_FONT_NAME = u"FontName"_ustr;
inline constexpr OUString PROPERTY_FONT_STYLENAME = u"FontStyleName"_ustr;
inline constexpr OUString PROPERTY_FONT_FAMILY = u"FontFamily"_ustr;
inline constexpr OUString PROPERTY_FONT_CHARSET = u"FontCharset"_ustr;
inline constexpr OUString PROPERTY_FONT_HEIGHT = u"FontHeight"_ustr;
inline constexpr OUString PROPERTY_FONT_WEIGHT = u"FontWeight"_ustr;
inline constexpr OUString PROPERTY_FONT_SLANT = u"FontSlant"_ustr;
inline constexpr OUString PROPERTY_FONT_UNDERLINE = u"FontUnderline"_ustr;
inline constexpr OUString PROPERTY_FONT_STRIKEOUT = u"FontStrikeout"_ustr;
inline constexpr OUString PROPERTY_FONT_RELIEF = u"FontRelief"_ustr;
inline constexpr OUString PROPERTY_FONT_EMPHASIS_MARK = u"FontEmphasisMark"_ustr;
inline constexpr OUString PROPERTY_TEXTLINECOLOR = u"TextLineColor"_ustr;
inline constexpr OUString PROPERTY_HELPURL = u"HelpURL"_ustr;
inline constexpr OUString PROPERTY_RECORDMARKER = u"HasRecordMarker"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_MIN = u"EffectiveMin"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_MAX = u"EffectiveMax"_ustr;
inline constexpr OUString PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal"_ustr;
inline constexpr OUString PROPERTY_CURRSYM_POSITION = u"PrependCurrencySymbol"_ustr;
inline constexpr OUString PROPERTY_COMMAND = u"Command"_ustr;
inline constexpr OUString PROPERTY_COMMANDTYPE = u"CommandType"_ustr;
inline constexpr OUString PROPERTY_INSERTONLY = u"IgnoreResult"_ustr;
inline constexpr OUString PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing"_ustr;
inline constexpr OUString PROPERTY_TITLE = u"Title"_ustr;
inline constexpr OUString PROPERTY_SORT = u"Order"_ustr;
inline constexpr OUString PROPERTY_DATASOURCE = u"DataSourceName"_ustr;
inline constexpr OUString PROPERTY_DETAILFIELDS = u"DetailFields"_ustr;
inline constexpr OUString PROPERTY_DEFAULTBUTTON = u"DefaultButton"_ustr;
inline constexpr OUString PROPERTY_LISTINDEX = u"ListIndex"_ustr;
inline constexpr OUString PROPERTY_HEIGHT = u"Height"_ustr;
inline constexpr OUString PROPERTY_HASNAVIGATION = u"HasNavigationBar"_ustr;
inline constexpr OUString PROPERTY_POSITIONX = u"PositionX"_ustr;
inline constexpr OUString PROPERTY_POSITIONY = u"PositionY"_ustr;
inline constexpr OUString PROPERTY_AUTOGROW = u"AutoGrow"_ustr;
inline constexpr OUString PROPERTY_STEP = u"Step"_ustr;
inline constexpr OUString PROPERTY_WORDLINEMODE = u"FontWordLineMode"_ustr;
inline constexpr OUString PROPERTY_PROGRESSVALUE = u"ProgressValue"_ustr;
inline constexpr OUString PROPERTY_PROGRESSVALUE_MIN = u"ProgressValueMin"_ustr;
inline constexpr OUString PROPERTY_PROGRESSVALUE_MAX = u"ProgressValueMax"_ustr;
inline constexpr OUString PROPERTY_SCROLLVALUE = u"ScrollValue"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SCROLLVALUE = u"DefaultScrollValue"_ustr;
inline constexpr OUString PROPERTY_SCROLLVALUE_MIN = u"ScrollValueMin"_ustr;
inline constexpr OUString PROPERTY_SCROLLVALUE_MAX = u"ScrollValueMax"_ustr;
inline constexpr OUString PROPERTY_SCROLL_WIDTH = u"ScrollWidth"_ustr;
inline constexpr OUString PROPERTY_SCROLL_HEIGHT = u"ScrollHeight"_ustr;
inline constexpr OUString PROPERTY_SCROLL_TOP = u"ScrollTop"_ustr;
inline constexpr OUString PROPERTY_SCROLL_LEFT = u"ScrollLeft"_ustr;
inline constexpr OUString PROPERTY_LINEINCREMENT = u"LineIncrement"_ustr;
inline constexpr OUString PROPERTY_BLOCKINCREMENT = u"BlockIncrement"_ustr;
inline constexpr OUString PROPERTY_VISIBLESIZE = u"VisibleSize"_ustr;
inline constexpr OUString PROPERTY_ORIENTATION = u"Orientation"_ustr;
inline constexpr OUString PROPERTY_IMAGEPOSITION = u"ImagePosition"_ustr;
inline constexpr OUString PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection"_ustr;
inline constexpr OUString PROPERTY_ACTIVECOMMAND = u"ActiveCommand"_ustr;
inline constexpr OUString PROPERTY_DATE = u"Date"_ustr;
inline constexpr OUString PROPERTY_STATE = u"State"_ustr;
inline constexpr OUString PROPERTY_TIME = u"Time"_ustr;
inline constexpr OUString PROPERTY_SCALEIMAGE = u"ScaleImage"_ustr;
inline constexpr OUString PROPERTY_SCALE_MODE = u"ScaleMode"_ustr;
inline constexpr OUString PROPERTY_PUSHBUTTONTYPE = u"PushButtonType"_ustr;
inline constexpr OUString PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue"_ustr;
inline constexpr OUString PROPERTY_SELECTEDITEMS = u"SelectedItems"_ustr;
inline constexpr OUString PROPERTY_REPEAT = u"Repeat"_ustr;
inline constexpr OUString PROPERTY_REPEAT_DELAY = u"RepeatDelay"_ustr;
inline constexpr OUString PROPERTY_SYMBOLCOLOR = u"SymbolColor"_ustr;
inline constexpr OUString PROPERTY_SPINVALUE = u"SpinValue"_ustr;
inline constexpr OUString PROPERTY_SPINVALUE_MIN = u"SpinValueMin"_ustr;
inline constexpr OUString PROPERTY_SPINVALUE_MAX = u"SpinValueMax"_ustr;
inline constexpr OUString PROPERTY_DEFAULT_SPINVALUE = u"DefaultSpinValue"_ustr;
inline constexpr OUString PROPERTY_SPININCREMENT = u"SpinIncrement"_ustr;
inline constexpr OUString PROPERTY_SHOW_POSITION = u"ShowPosition"_ustr;
inline constexpr OUString PROPERTY_SHOW_NAVIGATION = u"ShowNavigation"_ustr;
inline constexpr OUString PROPERTY_SHOW_RECORDACTIONS = u"ShowRecordActions"_ustr;
inline constexpr OUString PROPERTY_SHOW_FILTERSORT = u"ShowFilterSort"_ustr;
inline constexpr OUString PROPERTY_LINEEND_FORMAT = u"LineEndFormat"_ustr;
inline constexpr OUString PROPERTY_DECORATION = u"Decoration"_ustr;
inline constexpr OUString PROPERTY_NOLABEL = u"NoLabel"_ustr;
inline constexpr OUString PROPERTY_URL = u"URL"_ustr;

inline constexpr OUString PROPERTY_SELECTION_TYPE = u"SelectionType"_ustr;
inline constexpr OUString PROPERTY_STANDARD_THEME = u"StandardTheme"_ustr;
inline constexpr OUString PROPERTY_ROOT_DISPLAYED = u"RootDisplayed"_ustr;
inline constexpr OUString PROPERTY_SHOWS_HANDLES = u"ShowsHandles"_ustr;
inline constexpr OUString PROPERTY_SHOWS_ROOT_HANDLES = u"ShowsRootHandles"_ustr;
inline constexpr OUString PROPERTY_EDITABLE = u"Editable"_ustr;
inline constexpr OUString PROPERTY_INVOKES_STOP_NOT_EDITING = u"InvokesStopNodeEditing"_ustr;

inline constexpr OUString PROPERTY_TOGGLE = u"Toggle"_ustr;
inline constexpr OUString PROPERTY_FOCUSONCLICK = u"FocusOnClick"_ustr;
inline constexpr OUString PROPERTY_HIDEINACTIVESELECTION = u"HideInactiveSelection"_ustr;
inline constexpr OUString PROPERTY_VISUALEFFECT = u"VisualEffect"_ustr;
inline constexpr OUString PROPERTY_BORDERCOLOR = u"BorderColor"_ustr;

inline constexpr OUString PROPERTY_ADDRESS = u"Address"_ustr;
inline constexpr OUString PROPERTY_REFERENCE_SHEET = u"ReferenceSheet"_ustr;
inline constexpr OUString PROPERTY_UI_REPRESENTATION = u"UserInterfaceRepresentation"_ustr;

inline constexpr OUString PROPERTY_XML_DATA_MODEL = u"XMLDataModel"_ustr;
inline constexpr OUString PROPERTY_BINDING_NAME = u"BindingName"_ustr;
inline constexpr OUString PROPERTY_BIND_EXPRESSION = u"BindingExpression"_ustr;
inline constexpr OUString PROPERTY_LIST_BINDING = u"ListBinding"_ustr;
inline constexpr OUString PROPERTY_XSD_REQUIRED = u"RequiredExpression"_ustr;
inline constexpr OUString PROPERTY_XSD_RELEVANT = u"RelevantExpression"_ustr;
inline constexpr OUString PROPERTY_XSD_READONLY = u"ReadonlyExpression"_ustr;
inline constexpr OUString PROPERTY_XSD_CONSTRAINT = u"ConstraintExpression"_ustr;
inline constexpr OUString PROPERTY_XSD_CALCULATION = u"CalculateExpression"_ustr;
inline constexpr OUString PROPERTY_XSD_DATA_TYPE = u"Type"_ustr;
inline constexpr OUString PROPERTY_XSD_WHITESPACES = u"WhiteSpace"_ustr;
inline constexpr OUString PROPERTY_XSD_PATTERN = u"Pattern"_ustr;
inline constexpr OUString PROPERTY_XSD_LENGTH = u"Length"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_LENGTH = u"MinLength"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_LENGTH = u"MaxLength"_ustr;
inline constexpr OUString PROPERTY_XSD_TOTAL_DIGITS = u"TotalDigits"_ustr;
inline constexpr OUString PROPERTY_XSD_FRACTION_DIGITS = u"FractionDigits"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_INT = u"MaxInclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_INT = u"MaxExclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_INT = u"MinInclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_INT = u"MinExclusiveInt"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE = u"MaxInclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE = u"MaxExclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE = u"MinInclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE = u"MinExclusiveDouble"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DATE = u"MaxInclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DATE = u"MaxExclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DATE = u"MinInclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DATE = u"MinExclusiveDate"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_TIME = u"MaxInclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_TIME = u"MaxExclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_TIME = u"MinInclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_TIME = u"MinExclusiveTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME = u"MaxInclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME = u"MaxExclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME = u"MinInclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME = u"MinExclusiveDateTime"_ustr;
inline constexpr OUString PROPERTY_SUBMISSION_ID = u"SubmissionID"_ustr;
inline constexpr OUString PROPERTY_BINDING_ID = u"BindingID"_ustr;
inline constexpr OUString PROPERTY_WRITING_MODE = u"WritingMode"_ustr;
inline constexpr OUString PROPERTY_TEXT_ANCHOR_TYPE = u"TextAnchorType"_ustr;
inline constexpr OUString PROPERTY_SHEET_ANCHOR_TYPE = u"SheetAnchorType"_ustr;
inline constexpr OUString PROPERTY_ANCHOR_TYPE = u"AnchorType"_ustr;
inline constexpr OUString PROPERTY_ANCHOR = u"Anchor"_ustr;
inline constexpr OUString PROPERTY_IS_VISIBLE = u"IsVisible"_ustr;

inline constexpr OUString PROPERTY_MODEL = u"Model"_ustr;

inline constexpr OUString PROPERTY_CELL_EXCHANGE_TYPE = u"ExchangeSelectionIndex"_ustr;
inline constexpr OUString PROPERTY_BOUND_CELL = u"BoundCell"_ustr;
inline constexpr OUString PROPERTY_LIST_CELL_RANGE = u"CellRange"_ustr;
inline constexpr OUString PROPERTY_TEXTTYPE = u"TextType"_ustr;
inline constexpr OUString PROPERTY_RICHTEXT = u"RichText"_ustr;
inline constexpr OUString PROPERTY_ROWSET = u"RowSet"_ustr;
inline constexpr OUString PROPERTY_SELECTIONMODEL = u"SelectionModel"_ustr;
inline constexpr OUString PROPERTY_USEGRIDLINE = u"UseGridLines"_ustr;
inline constexpr OUString PROPERTY_GRIDLINECOLOR = u"GridLineColor"_ustr;
inline constexpr OUString PROPERTY_SHOWCOLUMNHEADER = u"ShowColumnHeader"_ustr;
inline constexpr OUString PROPERTY_SHOWROWHEADER = u"ShowRowHeader"_ustr;
inline constexpr OUString PROPERTY_HEADERBACKGROUNDCOLOR = u"HeaderBackgroundColor"_ustr;
inline constexpr OUString PROPERTY_HEADERTEXTCOLOR = u"HeaderTextColor"_ustr;
inline constexpr OUString PROPERTY_ACTIVESELECTIONBACKGROUNDCOLOR = u"ActiveSelectionBackgroundColor"_ustr;
inline constexpr OUString PROPERTY_ACTIVESELECTIONTEXTCOLOR = u"ActiveSelectionTextColor"_ustr;
inline constexpr OUString PROPERTY_INACTIVESELECTIONBACKGROUNDCOLOR = u"InactiveSelectionBackgroundColor"_ustr;
inline constexpr OUString PROPERTY_INACTIVESELECTIONTEXTCOLOR = u"InactiveSelectionTextColor"_ustr;

// services
inline constexpr OUString SERVICE_COMPONENT_GROUPBOX = u"com.sun.star.form.component.GroupBox"_ustr;
inline constexpr OUString SERVICE_COMPONENT_FIXEDTEXT = u"com.sun.star.form.component.FixedText"_ustr;
inline constexpr OUString SERVICE_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField"_ustr;

inline constexpr OUString SERVICE_TEXT_DOCUMENT = u"com.sun.star.text.TextDocument"_ustr;
inline constexpr OUString SERVICE_WEB_DOCUMENT = u"com.sun.star.text.WebDocument"_ustr;
inline constexpr OUString SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument"_ustr;
inline constexpr OUString SERVICE_DRAWING_DOCUMENT = u"com.sun.star.drawing.DrawingDocument"_ustr;
inline constexpr OUString SERVICE_PRESENTATION_DOCUMENT = u"com.sun.star.presentation.PresentationDocument"_ustr;

inline constexpr OUString SERVICE_SHEET_CELL_BINDING = u"com.sun.star.table.CellValueBinding"_ustr;
inline constexpr OUString SERVICE_SHEET_CELL_INT_BINDING = u"com.sun.star.table.ListPositionCellBinding"_ustr;
inline constexpr OUString SERVICE_SHEET_CELLRANGE_LISTSOURCE = u"com.sun.star.table.CellRangeListSource"_ustr;
inline constexpr OUString SERVICE_ADDRESS_CONVERSION = u"com.sun.star.table.CellAddressConversion"_ustr;
inline constexpr OUString SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion"_ustr;



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
