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

inline constexpr OUStringLiteral PROPERTY_DEFAULTCONTROL = u"DefaultControl";
inline constexpr OUStringLiteral PROPERTY_INTROSPECTEDOBJECT = u"IntrospectedObject";
inline constexpr OUStringLiteral PROPERTY_CURRENTPAGE = u"CurrentPage";
inline constexpr OUStringLiteral PROPERTY_CONTROLCONTEXT = u"ControlContext";

// properties
inline constexpr OUStringLiteral PROPERTY_CLASSID = u"ClassId";
inline constexpr OUStringLiteral PROPERTY_CONTROLLABEL = u"LabelControl";
inline constexpr OUStringLiteral PROPERTY_LABEL = u"Label";
inline constexpr OUStringLiteral PROPERTY_TABINDEX = u"TabIndex";
inline constexpr OUStringLiteral PROPERTY_WHEEL_BEHAVIOR = u"MouseWheelBehavior";
inline constexpr OUStringLiteral PROPERTY_TAG = u"Tag";
inline constexpr OUStringLiteral PROPERTY_NAME = u"Name";
inline constexpr OUStringLiteral PROPERTY_GROUP_NAME = u"GroupName";
inline constexpr OUStringLiteral PROPERTY_VALUE = u"Value";
inline constexpr OUStringLiteral PROPERTY_TEXT = u"Text";
inline constexpr OUStringLiteral PROPERTY_NAVIGATION = u"NavigationBarMode";
inline constexpr OUStringLiteral PROPERTY_CYCLE = u"Cycle";
inline constexpr OUStringLiteral PROPERTY_CONTROLSOURCE = u"DataField";
inline constexpr OUStringLiteral PROPERTY_INPUT_REQUIRED = u"InputRequired";
inline constexpr OUStringLiteral PROPERTY_ENABLED = u"Enabled";
inline constexpr OUStringLiteral PROPERTY_ENABLE_VISIBLE = u"EnableVisible";
inline constexpr OUStringLiteral PROPERTY_READONLY = u"ReadOnly";
#define PROPERTY_ISREADONLY              "IsReadOnly"
inline constexpr OUStringLiteral PROPERTY_FILTER = u"Filter";
inline constexpr OUStringLiteral PROPERTY_WIDTH = u"Width";
inline constexpr OUStringLiteral PROPERTY_MULTILINE = u"MultiLine";
inline constexpr OUStringLiteral PROPERTY_WORDBREAK = u"WordBreak";
inline constexpr OUStringLiteral PROPERTY_TARGET_URL = u"TargetURL";
inline constexpr OUStringLiteral PROPERTY_TARGET_FRAME = u"TargetFrame";
inline constexpr OUStringLiteral PROPERTY_MAXTEXTLEN = u"MaxTextLen";
inline constexpr OUStringLiteral PROPERTY_EDITMASK = u"EditMask";
inline constexpr OUStringLiteral PROPERTY_SPIN = u"Spin";
inline constexpr OUStringLiteral PROPERTY_TRISTATE = u"TriState";
inline constexpr OUStringLiteral PROPERTY_HIDDEN_VALUE = u"HiddenValue";
inline constexpr OUStringLiteral PROPERTY_BUTTONTYPE = u"ButtonType";
inline constexpr OUStringLiteral PROPERTY_XFORMS_BUTTONTYPE = u"XFormsButtonType";
inline constexpr OUStringLiteral PROPERTY_STRINGITEMLIST = u"StringItemList";
inline constexpr OUStringLiteral PROPERTY_TYPEDITEMLIST = u"TypedItemList";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_TEXT = u"DefaultText";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_STATE = u"DefaultState";
inline constexpr OUStringLiteral PROPERTY_FORMATKEY = u"FormatKey";
inline constexpr OUStringLiteral PROPERTY_FORMATSSUPPLIER = u"FormatsSupplier";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_ACTION = u"SubmitAction";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_TARGET = u"SubmitTarget";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_METHOD = u"SubmitMethod";
inline constexpr OUStringLiteral PROPERTY_SUBMIT_ENCODING = u"SubmitEncoding";
inline constexpr OUStringLiteral PROPERTY_IMAGE_URL = u"ImageURL";
inline constexpr OUStringLiteral PROPERTY_GRAPHIC = u"Graphic";
inline constexpr OUStringLiteral PROPERTY_EMPTY_IS_NULL = u"ConvertEmptyToNull";
inline constexpr OUStringLiteral PROPERTY_LISTSOURCETYPE = u"ListSourceType";
inline constexpr OUStringLiteral PROPERTY_LISTSOURCE = u"ListSource";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SELECT_SEQ = u"DefaultSelection";
inline constexpr OUStringLiteral PROPERTY_MULTISELECTION = u"MultiSelection";
inline constexpr OUStringLiteral PROPERTY_ALIGN = u"Align";
inline constexpr OUStringLiteral PROPERTY_VERTICAL_ALIGN = u"VerticalAlign";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_DATE = u"DefaultDate";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_TIME = u"DefaultTime";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_VALUE = u"DefaultValue";
inline constexpr OUStringLiteral PROPERTY_DECIMAL_ACCURACY = u"DecimalAccuracy";
inline constexpr OUStringLiteral PROPERTY_REFVALUE = u"RefValue";
inline constexpr OUStringLiteral PROPERTY_UNCHECKEDREFVALUE = u"SecondaryRefValue";
inline constexpr OUStringLiteral PROPERTY_VALUEMIN = u"ValueMin";
inline constexpr OUStringLiteral PROPERTY_VALUEMAX = u"ValueMax";
inline constexpr OUStringLiteral PROPERTY_STRICTFORMAT = u"StrictFormat";
inline constexpr OUStringLiteral PROPERTY_ALLOWADDITIONS = u"AllowInserts";
inline constexpr OUStringLiteral PROPERTY_ALLOWEDITS = u"AllowUpdates";
inline constexpr OUStringLiteral PROPERTY_ALLOWDELETIONS = u"AllowDeletes";
inline constexpr OUStringLiteral PROPERTY_MASTERFIELDS = u"MasterFields";
inline constexpr OUStringLiteral PROPERTY_LITERALMASK = u"LiteralMask";
inline constexpr OUStringLiteral PROPERTY_VALUESTEP = u"ValueStep";
inline constexpr OUStringLiteral PROPERTY_SHOWTHOUSANDSEP = u"ShowThousandsSeparator";
inline constexpr OUStringLiteral PROPERTY_CURRENCYSYMBOL = u"CurrencySymbol";
inline constexpr OUStringLiteral PROPERTY_DATEFORMAT = u"DateFormat";
inline constexpr OUStringLiteral PROPERTY_DATEMIN = u"DateMin";
inline constexpr OUStringLiteral PROPERTY_DATEMAX = u"DateMax";
inline constexpr OUStringLiteral PROPERTY_TIMEFORMAT = u"TimeFormat";
inline constexpr OUStringLiteral PROPERTY_TIMEMIN = u"TimeMin";
inline constexpr OUStringLiteral PROPERTY_TIMEMAX = u"TimeMax";
inline constexpr OUStringLiteral PROPERTY_LINECOUNT = u"LineCount";
inline constexpr OUStringLiteral PROPERTY_BOUNDCOLUMN = u"BoundColumn";
inline constexpr OUStringLiteral PROPERTY_BACKGROUNDCOLOR = u"BackgroundColor";
inline constexpr OUStringLiteral PROPERTY_FILLCOLOR = u"FillColor";
inline constexpr OUStringLiteral PROPERTY_TEXTCOLOR = u"TextColor";
inline constexpr OUStringLiteral PROPERTY_LINECOLOR = u"LineColor";
inline constexpr OUStringLiteral PROPERTY_BORDER = u"Border";
inline constexpr OUStringLiteral PROPERTY_ICONSIZE = u"IconSize";
inline constexpr OUStringLiteral PROPERTY_DROPDOWN = u"Dropdown";
inline constexpr OUStringLiteral PROPERTY_HSCROLL = u"HScroll";
inline constexpr OUStringLiteral PROPERTY_VSCROLL = u"VScroll";
inline constexpr OUStringLiteral PROPERTY_SHOW_SCROLLBARS = u"ShowScrollbars";
inline constexpr OUStringLiteral PROPERTY_TABSTOP = u"Tabstop";
inline constexpr OUStringLiteral PROPERTY_AUTOCOMPLETE = u"Autocomplete";
inline constexpr OUStringLiteral PROPERTY_PRINTABLE = u"Printable";
inline constexpr OUStringLiteral PROPERTY_ECHO_CHAR = u"EchoChar";
inline constexpr OUStringLiteral PROPERTY_ROWHEIGHT = u"RowHeight";
inline constexpr OUStringLiteral PROPERTY_HELPTEXT = u"HelpText";
inline constexpr OUStringLiteral PROPERTY_FONT = u"FontDescriptor";
inline constexpr OUStringLiteral PROPERTY_FONT_NAME = u"FontName";
inline constexpr OUStringLiteral PROPERTY_FONT_STYLENAME = u"FontStyleName";
inline constexpr OUStringLiteral PROPERTY_FONT_FAMILY = u"FontFamily";
inline constexpr OUStringLiteral PROPERTY_FONT_CHARSET = u"FontCharset";
inline constexpr OUStringLiteral PROPERTY_FONT_HEIGHT = u"FontHeight";
inline constexpr OUStringLiteral PROPERTY_FONT_WEIGHT = u"FontWeight";
inline constexpr OUStringLiteral PROPERTY_FONT_SLANT = u"FontSlant";
inline constexpr OUStringLiteral PROPERTY_FONT_UNDERLINE = u"FontUnderline";
inline constexpr OUStringLiteral PROPERTY_FONT_STRIKEOUT = u"FontStrikeout";
inline constexpr OUStringLiteral PROPERTY_FONT_RELIEF = u"FontRelief";
inline constexpr OUStringLiteral PROPERTY_FONT_EMPHASIS_MARK = u"FontEmphasisMark";
inline constexpr OUStringLiteral PROPERTY_TEXTLINECOLOR = u"TextLineColor";
inline constexpr OUStringLiteral PROPERTY_HELPURL = u"HelpURL";
inline constexpr OUStringLiteral PROPERTY_RECORDMARKER = u"HasRecordMarker";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_DEFAULT = u"EffectiveDefault";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_MIN = u"EffectiveMin";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_MAX = u"EffectiveMax";
inline constexpr OUStringLiteral PROPERTY_FILTERPROPOSAL = u"UseFilterValueProposal";
inline constexpr OUStringLiteral PROPERTY_CURRSYM_POSITION = u"PrependCurrencySymbol";
inline constexpr OUStringLiteral PROPERTY_COMMAND = u"Command";
inline constexpr OUStringLiteral PROPERTY_COMMANDTYPE = u"CommandType";
inline constexpr OUStringLiteral PROPERTY_INSERTONLY = u"IgnoreResult";
inline constexpr OUStringLiteral PROPERTY_ESCAPE_PROCESSING = u"EscapeProcessing";
inline constexpr OUStringLiteral PROPERTY_TITLE = u"Title";
inline constexpr OUStringLiteral PROPERTY_SORT = u"Order";
inline constexpr OUStringLiteral PROPERTY_DATASOURCE = u"DataSourceName";
inline constexpr OUStringLiteral PROPERTY_DETAILFIELDS = u"DetailFields";
inline constexpr OUStringLiteral PROPERTY_DEFAULTBUTTON = u"DefaultButton";
inline constexpr OUStringLiteral PROPERTY_LISTINDEX = u"ListIndex";
inline constexpr OUStringLiteral PROPERTY_HEIGHT = u"Height";
inline constexpr OUStringLiteral PROPERTY_HASNAVIGATION = u"HasNavigationBar";
inline constexpr OUStringLiteral PROPERTY_POSITIONX = u"PositionX";
inline constexpr OUStringLiteral PROPERTY_POSITIONY = u"PositionY";
inline constexpr OUStringLiteral PROPERTY_AUTOGROW = u"AutoGrow";
inline constexpr OUStringLiteral PROPERTY_STEP = u"Step";
inline constexpr OUStringLiteral PROPERTY_WORDLINEMODE = u"FontWordLineMode";
inline constexpr OUStringLiteral PROPERTY_PROGRESSVALUE = u"ProgressValue";
inline constexpr OUStringLiteral PROPERTY_PROGRESSVALUE_MIN = u"ProgressValueMin";
inline constexpr OUStringLiteral PROPERTY_PROGRESSVALUE_MAX = u"ProgressValueMax";
inline constexpr OUStringLiteral PROPERTY_SCROLLVALUE = u"ScrollValue";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SCROLLVALUE = u"DefaultScrollValue";
inline constexpr OUStringLiteral PROPERTY_SCROLLVALUE_MIN = u"ScrollValueMin";
inline constexpr OUStringLiteral PROPERTY_SCROLLVALUE_MAX = u"ScrollValueMax";
inline constexpr OUStringLiteral PROPERTY_SCROLL_WIDTH = u"ScrollWidth";
inline constexpr OUStringLiteral PROPERTY_SCROLL_HEIGHT = u"ScrollHeight";
inline constexpr OUStringLiteral PROPERTY_SCROLL_TOP = u"ScrollTop";
inline constexpr OUStringLiteral PROPERTY_SCROLL_LEFT = u"ScrollLeft";
inline constexpr OUStringLiteral PROPERTY_LINEINCREMENT = u"LineIncrement";
inline constexpr OUStringLiteral PROPERTY_BLOCKINCREMENT = u"BlockIncrement";
inline constexpr OUStringLiteral PROPERTY_VISIBLESIZE = u"VisibleSize";
inline constexpr OUStringLiteral PROPERTY_ORIENTATION = u"Orientation";
inline constexpr OUStringLiteral PROPERTY_IMAGEPOSITION = u"ImagePosition";
inline constexpr OUStringLiteral PROPERTY_ACTIVE_CONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral PROPERTY_ACTIVECOMMAND = u"ActiveCommand";
inline constexpr OUStringLiteral PROPERTY_DATE = u"Date";
inline constexpr OUStringLiteral PROPERTY_STATE = u"State";
inline constexpr OUStringLiteral PROPERTY_TIME = u"Time";
inline constexpr OUStringLiteral PROPERTY_SCALEIMAGE = u"ScaleImage";
inline constexpr OUStringLiteral PROPERTY_SCALE_MODE = u"ScaleMode";
inline constexpr OUStringLiteral PROPERTY_PUSHBUTTONTYPE = u"PushButtonType";
inline constexpr OUStringLiteral PROPERTY_EFFECTIVE_VALUE = u"EffectiveValue";
inline constexpr OUStringLiteral PROPERTY_SELECTEDITEMS = u"SelectedItems";
inline constexpr OUStringLiteral PROPERTY_REPEAT = u"Repeat";
inline constexpr OUStringLiteral PROPERTY_REPEAT_DELAY = u"RepeatDelay";
inline constexpr OUStringLiteral PROPERTY_SYMBOLCOLOR = u"SymbolColor";
inline constexpr OUStringLiteral PROPERTY_SPINVALUE = u"SpinValue";
inline constexpr OUStringLiteral PROPERTY_SPINVALUE_MIN = u"SpinValueMin";
inline constexpr OUStringLiteral PROPERTY_SPINVALUE_MAX = u"SpinValueMax";
inline constexpr OUStringLiteral PROPERTY_DEFAULT_SPINVALUE = u"DefaultSpinValue";
inline constexpr OUStringLiteral PROPERTY_SPININCREMENT = u"SpinIncrement";
inline constexpr OUStringLiteral PROPERTY_SHOW_POSITION = u"ShowPosition";
inline constexpr OUStringLiteral PROPERTY_SHOW_NAVIGATION = u"ShowNavigation";
inline constexpr OUStringLiteral PROPERTY_SHOW_RECORDACTIONS = u"ShowRecordActions";
inline constexpr OUStringLiteral PROPERTY_SHOW_FILTERSORT = u"ShowFilterSort";
inline constexpr OUStringLiteral PROPERTY_LINEEND_FORMAT = u"LineEndFormat";
inline constexpr OUStringLiteral PROPERTY_DECORATION = u"Decoration";
inline constexpr OUStringLiteral PROPERTY_NOLABEL = u"NoLabel";
inline constexpr OUStringLiteral PROPERTY_URL = u"URL";

inline constexpr OUStringLiteral PROPERTY_SELECTION_TYPE = u"SelectionType";
inline constexpr OUStringLiteral PROPERTY_ROOT_DISPLAYED = u"RootDisplayed";
inline constexpr OUStringLiteral PROPERTY_SHOWS_HANDLES = u"ShowsHandles";
inline constexpr OUStringLiteral PROPERTY_SHOWS_ROOT_HANDLES = u"ShowsRootHandles";
inline constexpr OUStringLiteral PROPERTY_EDITABLE = u"Editable";
inline constexpr OUStringLiteral PROPERTY_INVOKES_STOP_NOT_EDITING = u"InvokesStopNodeEditing";

#define PROPERTY_HARDLINEBREAKS         "HardLineBreaks"

inline constexpr OUStringLiteral PROPERTY_TOGGLE = u"Toggle";
inline constexpr OUStringLiteral PROPERTY_FOCUSONCLICK = u"FocusOnClick";
inline constexpr OUStringLiteral PROPERTY_HIDEINACTIVESELECTION = u"HideInactiveSelection";
inline constexpr OUStringLiteral PROPERTY_VISUALEFFECT = u"VisualEffect";
inline constexpr OUStringLiteral PROPERTY_BORDERCOLOR = u"BorderColor";

inline constexpr OUStringLiteral PROPERTY_ADDRESS = u"Address";
inline constexpr OUStringLiteral PROPERTY_REFERENCE_SHEET = u"ReferenceSheet";
inline constexpr OUStringLiteral PROPERTY_UI_REPRESENTATION = u"UserInterfaceRepresentation";

inline constexpr OUStringLiteral PROPERTY_XML_DATA_MODEL = u"XMLDataModel";
inline constexpr OUStringLiteral PROPERTY_BINDING_NAME = u"BindingName";
inline constexpr OUStringLiteral PROPERTY_BIND_EXPRESSION = u"BindingExpression";
inline constexpr OUStringLiteral PROPERTY_LIST_BINDING = u"ListBinding";
inline constexpr OUStringLiteral PROPERTY_XSD_REQUIRED = u"RequiredExpression";
inline constexpr OUStringLiteral PROPERTY_XSD_RELEVANT = u"RelevantExpression";
inline constexpr OUStringLiteral PROPERTY_XSD_READONLY = u"ReadonlyExpression";
inline constexpr OUStringLiteral PROPERTY_XSD_CONSTRAINT = u"ConstraintExpression";
inline constexpr OUStringLiteral PROPERTY_XSD_CALCULATION = u"CalculateExpression";
inline constexpr OUStringLiteral PROPERTY_XSD_DATA_TYPE = u"Type";
inline constexpr OUStringLiteral PROPERTY_XSD_WHITESPACES = u"WhiteSpace";
inline constexpr OUStringLiteral PROPERTY_XSD_PATTERN = u"Pattern";
inline constexpr OUStringLiteral PROPERTY_XSD_LENGTH = u"Length";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_LENGTH = u"MinLength";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_LENGTH = u"MaxLength";
inline constexpr OUStringLiteral PROPERTY_XSD_TOTAL_DIGITS = u"TotalDigits";
inline constexpr OUStringLiteral PROPERTY_XSD_FRACTION_DIGITS = u"FractionDigits";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_INT = u"MaxInclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_INT = u"MaxExclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_INT = u"MinInclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_INT = u"MinExclusiveInt";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE = u"MaxInclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE = u"MaxExclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE = u"MinInclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE = u"MinExclusiveDouble";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE = u"MaxInclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE = u"MaxExclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE = u"MinInclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE = u"MinExclusiveDate";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_TIME = u"MaxInclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_TIME = u"MaxExclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_TIME = u"MinInclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_TIME = u"MinExclusiveTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME = u"MaxInclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME = u"MaxExclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME = u"MinInclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME = u"MinExclusiveDateTime";
inline constexpr OUStringLiteral PROPERTY_SUBMISSION_ID = u"SubmissionID";
inline constexpr OUStringLiteral PROPERTY_BINDING_ID = u"BindingID";
#define PROPERTY_ID                     "ID"
inline constexpr OUStringLiteral PROPERTY_WRITING_MODE = u"WritingMode";
inline constexpr OUStringLiteral PROPERTY_TEXT_ANCHOR_TYPE = u"TextAnchorType";
inline constexpr OUStringLiteral PROPERTY_SHEET_ANCHOR_TYPE = u"SheetAnchorType";
inline constexpr OUStringLiteral PROPERTY_ANCHOR_TYPE = u"AnchorType";
inline constexpr OUStringLiteral PROPERTY_ANCHOR = u"Anchor";
inline constexpr OUStringLiteral PROPERTY_IS_VISIBLE = u"IsVisible";

inline constexpr OUStringLiteral PROPERTY_MODEL = u"Model";

inline constexpr OUStringLiteral PROPERTY_CELL_EXCHANGE_TYPE = u"ExchangeSelectionIndex";
inline constexpr OUStringLiteral PROPERTY_BOUND_CELL = u"BoundCell";
inline constexpr OUStringLiteral PROPERTY_LIST_CELL_RANGE = u"CellRange";
#define PROPERTY_AUTOLINEBREAK          "AutomaticLineBreak"
inline constexpr OUStringLiteral PROPERTY_TEXTTYPE = u"TextType";
inline constexpr OUStringLiteral PROPERTY_RICHTEXT = u"RichText";
inline constexpr OUStringLiteral PROPERTY_ROWSET = u"RowSet";
inline constexpr OUStringLiteral PROPERTY_SELECTIONMODEL = u"SelectionModel";
inline constexpr OUStringLiteral PROPERTY_USEGRIDLINE = u"UseGridLines";
inline constexpr OUStringLiteral PROPERTY_GRIDLINECOLOR = u"GridLineColor";
inline constexpr OUStringLiteral PROPERTY_SHOWCOLUMNHEADER = u"ShowColumnHeader";
inline constexpr OUStringLiteral PROPERTY_SHOWROWHEADER = u"ShowRowHeader";
inline constexpr OUStringLiteral PROPERTY_HEADERBACKGROUNDCOLOR = u"HeaderBackgroundColor";
inline constexpr OUStringLiteral PROPERTY_HEADERTEXTCOLOR = u"HeaderTextColor";
inline constexpr OUStringLiteral PROPERTY_ACTIVESELECTIONBACKGROUNDCOLOR = u"ActiveSelectionBackgroundColor";
inline constexpr OUStringLiteral PROPERTY_ACTIVESELECTIONTEXTCOLOR = u"ActiveSelectionTextColor";
inline constexpr OUStringLiteral PROPERTY_INACTIVESELECTIONBACKGROUNDCOLOR = u"InactiveSelectionBackgroundColor";
inline constexpr OUStringLiteral PROPERTY_INACTIVESELECTIONTEXTCOLOR = u"InactiveSelectionTextColor";

// services
inline constexpr OUStringLiteral SERVICE_COMPONENT_GROUPBOX = u"com.sun.star.form.component.GroupBox";
inline constexpr OUStringLiteral SERVICE_COMPONENT_FIXEDTEXT = u"com.sun.star.form.component.FixedText";
inline constexpr OUStringLiteral SERVICE_COMPONENT_FORMATTEDFIELD = u"com.sun.star.form.component.FormattedField";

inline constexpr OUStringLiteral SERVICE_TEXT_DOCUMENT = u"com.sun.star.text.TextDocument";
inline constexpr OUStringLiteral SERVICE_WEB_DOCUMENT = u"com.sun.star.text.WebDocument";
inline constexpr OUStringLiteral SERVICE_SPREADSHEET_DOCUMENT = u"com.sun.star.sheet.SpreadsheetDocument";
inline constexpr OUStringLiteral SERVICE_DRAWING_DOCUMENT = u"com.sun.star.drawing.DrawingDocument";
inline constexpr OUStringLiteral SERVICE_PRESENTATION_DOCUMENT = u"com.sun.star.presentation.PresentationDocument";

inline constexpr OUStringLiteral SERVICE_SHEET_CELL_BINDING = u"com.sun.star.table.CellValueBinding";
inline constexpr OUStringLiteral SERVICE_SHEET_CELL_INT_BINDING = u"com.sun.star.table.ListPositionCellBinding";
inline constexpr OUStringLiteral SERVICE_SHEET_CELLRANGE_LISTSOURCE = u"com.sun.star.table.CellRangeListSource";
inline constexpr OUStringLiteral SERVICE_ADDRESS_CONVERSION = u"com.sun.star.table.CellAddressConversion";
inline constexpr OUStringLiteral SERVICE_RANGEADDRESS_CONVERSION = u"com.sun.star.table.CellRangeAddressConversion";



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
