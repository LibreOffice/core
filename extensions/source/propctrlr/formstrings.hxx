/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#define _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_

#include "stringdefine.hxx"

//............................................................................
namespace pcr
{
//............................................................................

    PCR_CONSTASCII_STRING( PROPERTY_DEFAULTCONTROL,         "DefaultControl" );
    PCR_CONSTASCII_STRING( PROPERTY_INTROSPECTEDOBJECT,     "IntrospectedObject" );
    PCR_CONSTASCII_STRING( PROPERTY_CURRENTPAGE,            "CurrentPage" );
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLCONTEXT,         "ControlContext" );

    // properties
    PCR_CONSTASCII_STRING( PROPERTY_CLASSID,                "ClassId" );
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLLABEL,           "LabelControl");
    PCR_CONSTASCII_STRING( PROPERTY_LABEL,                  "Label");
    PCR_CONSTASCII_STRING( PROPERTY_TABINDEX,               "TabIndex");
    PCR_CONSTASCII_STRING( PROPERTY_WHEEL_BEHAVIOR,         "MouseWheelBehavior");
    PCR_CONSTASCII_STRING( PROPERTY_TAG,                    "Tag");
    PCR_CONSTASCII_STRING( PROPERTY_NAME,                   "Name");
    PCR_CONSTASCII_STRING( PROPERTY_GROUP_NAME,             "GroupName");
    PCR_CONSTASCII_STRING( PROPERTY_VALUE,                  "Value");
    PCR_CONSTASCII_STRING( PROPERTY_TEXT,                   "Text");
    PCR_CONSTASCII_STRING( PROPERTY_NAVIGATION,             "NavigationBarMode");
    PCR_CONSTASCII_STRING( PROPERTY_CYCLE,                  "Cycle");
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLSOURCE,          "DataField");
    PCR_CONSTASCII_STRING( PROPERTY_INPUT_REQUIRED,         "InputRequired");
    PCR_CONSTASCII_STRING( PROPERTY_ENABLED,                "Enabled");
    PCR_CONSTASCII_STRING( PROPERTY_ENABLE_VISIBLE,         "EnableVisible" );
    PCR_CONSTASCII_STRING( PROPERTY_READONLY,               "ReadOnly");
    PCR_CONSTASCII_STRING( PROPERTY_ISREADONLY,             "IsReadOnly");
    PCR_CONSTASCII_STRING( PROPERTY_FILTER,                 "Filter");
    PCR_CONSTASCII_STRING( PROPERTY_WIDTH,                  "Width");
    PCR_CONSTASCII_STRING( PROPERTY_MULTILINE,              "MultiLine");
    PCR_CONSTASCII_STRING( PROPERTY_WORDBREAK,              "WordBreak");
    PCR_CONSTASCII_STRING( PROPERTY_TARGET_URL,             "TargetURL");
    PCR_CONSTASCII_STRING( PROPERTY_TARGET_FRAME,           "TargetFrame");
    PCR_CONSTASCII_STRING( PROPERTY_MAXTEXTLEN,             "MaxTextLen");
    PCR_CONSTASCII_STRING( PROPERTY_EDITMASK,               "EditMask");
    PCR_CONSTASCII_STRING( PROPERTY_SPIN,                   "Spin");
    PCR_CONSTASCII_STRING( PROPERTY_TRISTATE,               "TriState");
    PCR_CONSTASCII_STRING( PROPERTY_HIDDEN_VALUE,           "HiddenValue");
    PCR_CONSTASCII_STRING( PROPERTY_BUTTONTYPE,             "ButtonType");
    PCR_CONSTASCII_STRING( PROPERTY_XFORMS_BUTTONTYPE,      "XFormsButtonType");
    PCR_CONSTASCII_STRING( PROPERTY_STRINGITEMLIST,         "StringItemList");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_TEXT,           "DefaultText");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_STATE,          "DefaultState");
    PCR_CONSTASCII_STRING( PROPERTY_FORMATKEY,              "FormatKey");
    PCR_CONSTASCII_STRING( PROPERTY_FORMATSSUPPLIER,        "FormatsSupplier");
    PCR_CONSTASCII_STRING( PROPERTY_SUBMIT_ACTION,          "SubmitAction");
    PCR_CONSTASCII_STRING( PROPERTY_SUBMIT_TARGET,          "SubmitTarget");
    PCR_CONSTASCII_STRING( PROPERTY_SUBMIT_METHOD,          "SubmitMethod");
    PCR_CONSTASCII_STRING( PROPERTY_SUBMIT_ENCODING,        "SubmitEncoding");
    PCR_CONSTASCII_STRING( PROPERTY_IMAGE_URL,              "ImageURL");
    PCR_CONSTASCII_STRING( PROPERTY_EMPTY_IS_NULL,          "ConvertEmptyToNull");
    PCR_CONSTASCII_STRING( PROPERTY_LISTSOURCETYPE,         "ListSourceType");
    PCR_CONSTASCII_STRING( PROPERTY_LISTSOURCE,             "ListSource");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_SELECT_SEQ,     "DefaultSelection");
    PCR_CONSTASCII_STRING( PROPERTY_MULTISELECTION,         "MultiSelection");
    PCR_CONSTASCII_STRING( PROPERTY_ALIGN,                  "Align");
    PCR_CONSTASCII_STRING( PROPERTY_VERTICAL_ALIGN,         "VerticalAlign");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_DATE,           "DefaultDate");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_TIME,           "DefaultTime");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_VALUE,          "DefaultValue");
    PCR_CONSTASCII_STRING( PROPERTY_DECIMAL_ACCURACY,       "DecimalAccuracy");
    PCR_CONSTASCII_STRING( PROPERTY_REFVALUE,               "RefValue");
    PCR_CONSTASCII_STRING( PROPERTY_UNCHECKEDREFVALUE,      "SecondaryRefValue");
    PCR_CONSTASCII_STRING( PROPERTY_VALUEMIN,               "ValueMin");
    PCR_CONSTASCII_STRING( PROPERTY_VALUEMAX,               "ValueMax");
    PCR_CONSTASCII_STRING( PROPERTY_STRICTFORMAT,           "StrictFormat");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWADDITIONS,         "AllowInserts");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWEDITS,             "AllowUpdates");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWDELETIONS,         "AllowDeletes");
    PCR_CONSTASCII_STRING( PROPERTY_MASTERFIELDS,           "MasterFields");
    PCR_CONSTASCII_STRING( PROPERTY_LITERALMASK,            "LiteralMask");
    PCR_CONSTASCII_STRING( PROPERTY_VALUESTEP,              "ValueStep");
    PCR_CONSTASCII_STRING( PROPERTY_SHOWTHOUSANDSEP,        "ShowThousandsSeparator");
    PCR_CONSTASCII_STRING( PROPERTY_CURRENCYSYMBOL,         "CurrencySymbol");
    PCR_CONSTASCII_STRING( PROPERTY_DATEFORMAT,             "DateFormat");
    PCR_CONSTASCII_STRING( PROPERTY_DATEMIN,                "DateMin");
    PCR_CONSTASCII_STRING( PROPERTY_DATEMAX,                "DateMax");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEFORMAT,             "TimeFormat");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEMIN,                "TimeMin");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEMAX,                "TimeMax");
    PCR_CONSTASCII_STRING( PROPERTY_LINECOUNT,              "LineCount");
    PCR_CONSTASCII_STRING( PROPERTY_BOUNDCOLUMN,            "BoundColumn");
    PCR_CONSTASCII_STRING( PROPERTY_BACKGROUNDCOLOR,        "BackgroundColor");
    PCR_CONSTASCII_STRING( PROPERTY_FILLCOLOR,              "FillColor");
    PCR_CONSTASCII_STRING( PROPERTY_TEXTCOLOR,              "TextColor");
    PCR_CONSTASCII_STRING( PROPERTY_LINECOLOR,              "LineColor");
    PCR_CONSTASCII_STRING( PROPERTY_BORDER,                 "Border");
    PCR_CONSTASCII_STRING( PROPERTY_ICONSIZE,               "IconSize");
    PCR_CONSTASCII_STRING( PROPERTY_DROPDOWN,               "Dropdown");
    PCR_CONSTASCII_STRING( PROPERTY_HSCROLL,                "HScroll");
    PCR_CONSTASCII_STRING( PROPERTY_VSCROLL,                "VScroll");
    PCR_CONSTASCII_STRING( PROPERTY_SHOW_SCROLLBARS,        "ShowScrollbars");
    PCR_CONSTASCII_STRING( PROPERTY_TABSTOP,                "Tabstop");
    PCR_CONSTASCII_STRING( PROPERTY_AUTOCOMPLETE,           "Autocomplete");
    PCR_CONSTASCII_STRING( PROPERTY_PRINTABLE,              "Printable");
    PCR_CONSTASCII_STRING( PROPERTY_ECHO_CHAR,              "EchoChar");
    PCR_CONSTASCII_STRING( PROPERTY_ROWHEIGHT,              "RowHeight");
    PCR_CONSTASCII_STRING( PROPERTY_HELPTEXT,               "HelpText");
    PCR_CONSTASCII_STRING( PROPERTY_FONT,                   "FontDescriptor");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_NAME,              "FontName");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_STYLENAME,         "FontStyleName");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_FAMILY,            "FontFamily");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_CHARSET,           "FontCharset");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_HEIGHT,            "FontHeight");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_WEIGHT,            "FontWeight");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_SLANT,             "FontSlant");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_UNDERLINE,         "FontUnderline");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_STRIKEOUT,         "FontStrikeout");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_RELIEF,            "FontRelief");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_EMPHASIS_MARK,     "FontEmphasisMark");
    PCR_CONSTASCII_STRING( PROPERTY_TEXTLINECOLOR,          "TextLineColor");
    PCR_CONSTASCII_STRING( PROPERTY_HELPURL,                "HelpURL");
    PCR_CONSTASCII_STRING( PROPERTY_RECORDMARKER,           "HasRecordMarker");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_DEFAULT,      "EffectiveDefault");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MIN,          "EffectiveMin");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MAX,          "EffectiveMax");
    PCR_CONSTASCII_STRING( PROPERTY_FILTERPROPOSAL,         "UseFilterValueProposal");
    PCR_CONSTASCII_STRING( PROPERTY_CURRSYM_POSITION,       "PrependCurrencySymbol");
    PCR_CONSTASCII_STRING( PROPERTY_COMMAND,                "Command");
    PCR_CONSTASCII_STRING( PROPERTY_COMMANDTYPE,            "CommandType");
    PCR_CONSTASCII_STRING( PROPERTY_INSERTONLY,             "IgnoreResult");
    PCR_CONSTASCII_STRING( PROPERTY_ESCAPE_PROCESSING,      "EscapeProcessing");
    PCR_CONSTASCII_STRING( PROPERTY_TITLE,                  "Title");
    PCR_CONSTASCII_STRING( PROPERTY_SORT,                   "Order");
    PCR_CONSTASCII_STRING( PROPERTY_DATASOURCE,             "DataSourceName");
    PCR_CONSTASCII_STRING( PROPERTY_DETAILFIELDS,           "DetailFields");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULTBUTTON,          "DefaultButton");
    PCR_CONSTASCII_STRING( PROPERTY_LISTINDEX,              "ListIndex");
    PCR_CONSTASCII_STRING( PROPERTY_HEIGHT,                 "Height");
    PCR_CONSTASCII_STRING( PROPERTY_HASNAVIGATION,          "HasNavigationBar");
    PCR_CONSTASCII_STRING( PROPERTY_POSITIONX,              "PositionX");
    PCR_CONSTASCII_STRING( PROPERTY_POSITIONY,              "PositionY");
    PCR_CONSTASCII_STRING( PROPERTY_STEP,                   "Step");
    PCR_CONSTASCII_STRING( PROPERTY_WORDLINEMODE,           "FontWordLineMode");
    PCR_CONSTASCII_STRING( PROPERTY_PROGRESSVALUE,          "ProgressValue");
    PCR_CONSTASCII_STRING( PROPERTY_PROGRESSVALUE_MIN,      "ProgressValueMin");
    PCR_CONSTASCII_STRING( PROPERTY_PROGRESSVALUE_MAX,      "ProgressValueMax");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLLVALUE,            "ScrollValue");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_SCROLLVALUE,    "DefaultScrollValue");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLLVALUE_MIN,        "ScrollValueMin");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLLVALUE_MAX,        "ScrollValueMax");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLL_WIDTH,        "ScrollWidth");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLL_HEIGHT,        "ScrollHeight");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLL_TOP,        "ScrollTop");
    PCR_CONSTASCII_STRING( PROPERTY_SCROLL_LEFT,        "ScrollLeft");
    PCR_CONSTASCII_STRING( PROPERTY_LINEINCREMENT,          "LineIncrement");
    PCR_CONSTASCII_STRING( PROPERTY_BLOCKINCREMENT,         "BlockIncrement");
    PCR_CONSTASCII_STRING( PROPERTY_VISIBLESIZE,            "VisibleSize");
    PCR_CONSTASCII_STRING( PROPERTY_ORIENTATION,            "Orientation");
    PCR_CONSTASCII_STRING( PROPERTY_IMAGEPOSITION,          "ImagePosition");
    PCR_CONSTASCII_STRING( PROPERTY_ACTIVE_CONNECTION,      "ActiveConnection");
    PCR_CONSTASCII_STRING( PROPERTY_ACTIVECOMMAND,          "ActiveCommand");
    PCR_CONSTASCII_STRING( PROPERTY_DATE,                   "Date");
    PCR_CONSTASCII_STRING( PROPERTY_STATE,                  "State");
    PCR_CONSTASCII_STRING( PROPERTY_TIME,                   "Time");
    PCR_CONSTASCII_STRING( PROPERTY_SCALEIMAGE,             "ScaleImage");
    PCR_CONSTASCII_STRING( PROPERTY_SCALE_MODE,             "ScaleMode");
    PCR_CONSTASCII_STRING( PROPERTY_PUSHBUTTONTYPE,         "PushButtonType");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_VALUE,        "EffectiveValue");
    PCR_CONSTASCII_STRING( PROPERTY_SELECTEDITEMS,          "SelectedItems");
    PCR_CONSTASCII_STRING( PROPERTY_REPEAT,                 "Repeat");
    PCR_CONSTASCII_STRING( PROPERTY_REPEAT_DELAY,           "RepeatDelay");
    PCR_CONSTASCII_STRING( PROPERTY_SYMBOLCOLOR,            "SymbolColor");
    PCR_CONSTASCII_STRING( PROPERTY_SPINVALUE,              "SpinValue");
    PCR_CONSTASCII_STRING( PROPERTY_SPINVALUE_MIN,          "SpinValueMin");
    PCR_CONSTASCII_STRING( PROPERTY_SPINVALUE_MAX,          "SpinValueMax");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_SPINVALUE,      "DefaultSpinValue");
    PCR_CONSTASCII_STRING( PROPERTY_SPININCREMENT,          "SpinIncrement");
    PCR_CONSTASCII_STRING( PROPERTY_SHOW_POSITION,          "ShowPosition" );
    PCR_CONSTASCII_STRING( PROPERTY_SHOW_NAVIGATION,        "ShowNavigation" );
    PCR_CONSTASCII_STRING( PROPERTY_SHOW_RECORDACTIONS,     "ShowRecordActions" );
    PCR_CONSTASCII_STRING( PROPERTY_SHOW_FILTERSORT,        "ShowFilterSort" );
    PCR_CONSTASCII_STRING( PROPERTY_LINEEND_FORMAT,         "LineEndFormat");
    PCR_CONSTASCII_STRING( PROPERTY_DECORATION,             "Decoration" );
    PCR_CONSTASCII_STRING( PROPERTY_NOLABEL,                "NoLabel" );

    PCR_CONSTASCII_STRING( PROPERTY_SELECTION_TYPE,         "SelectionType");
    PCR_CONSTASCII_STRING( PROPERTY_ROOT_DISPLAYED,         "RootDisplayed");
    PCR_CONSTASCII_STRING( PROPERTY_SHOWS_HANDLES,          "ShowsHandles");
    PCR_CONSTASCII_STRING( PROPERTY_SHOWS_ROOT_HANDLES,     "ShowsRootHandles");
    PCR_CONSTASCII_STRING( PROPERTY_EDITABLE,               "Editable");
    PCR_CONSTASCII_STRING( PROPERTY_INVOKES_STOP_NOT_EDITING, "InvokesStopNodeEditing");

    PCR_CONSTASCII_STRING( PROPERTY_HARDLINEBREAKS,         "HardLineBreaks");

    PCR_CONSTASCII_STRING( PROPERTY_TOGGLE,                 "Toggle" );
    PCR_CONSTASCII_STRING( PROPERTY_FOCUSONCLICK,           "FocusOnClick" );
    PCR_CONSTASCII_STRING( PROPERTY_HIDEINACTIVESELECTION,  "HideInactiveSelection" );
    PCR_CONSTASCII_STRING( PROPERTY_VISUALEFFECT,           "VisualEffect" );
    PCR_CONSTASCII_STRING( PROPERTY_BORDERCOLOR,            "BorderColor" );

    PCR_CONSTASCII_STRING( PROPERTY_ADDRESS,                "Address");
    PCR_CONSTASCII_STRING( PROPERTY_REFERENCE_SHEET,        "ReferenceSheet");
    PCR_CONSTASCII_STRING( PROPERTY_UI_REPRESENTATION,      "UserInterfaceRepresentation");

    PCR_CONSTASCII_STRING( PROPERTY_XML_DATA_MODEL,         "XMLDataModel" );
    PCR_CONSTASCII_STRING( PROPERTY_BINDING_NAME,           "BindingName" );
    PCR_CONSTASCII_STRING( PROPERTY_BIND_EXPRESSION,        "BindingExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_LIST_BINDING,           "ListBinding" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_REQUIRED,           "RequiredExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_RELEVANT,           "RelevantExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_READONLY,           "ReadonlyExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_CONSTRAINT,         "ConstraintExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_CALCULATION,        "CalculateExpression" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_DATA_TYPE,          "Type" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_WHITESPACES,        "WhiteSpace" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_PATTERN,            "Pattern" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_LENGTH,             "Length" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_LENGTH,         "MinLength" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_LENGTH,         "MaxLength" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_TOTAL_DIGITS,       "TotalDigits" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_FRACTION_DIGITS,    "FractionDigits" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_INCLUSIVE_INT,  "MaxInclusiveInt" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_EXCLUSIVE_INT,  "MaxExclusiveInt" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_INCLUSIVE_INT,  "MinInclusiveInt" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_EXCLUSIVE_INT,  "MinExclusiveInt" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE, "MaxInclusiveDouble" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE, "MaxExclusiveDouble" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE, "MinInclusiveDouble" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE, "MinExclusiveDouble" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_INCLUSIVE_DATE, "MaxInclusiveDate" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_EXCLUSIVE_DATE, "MaxExclusiveDate" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_INCLUSIVE_DATE, "MinInclusiveDate" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_EXCLUSIVE_DATE, "MinExclusiveDate" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_INCLUSIVE_TIME, "MaxInclusiveTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_EXCLUSIVE_TIME, "MaxExclusiveTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_INCLUSIVE_TIME, "MinInclusiveTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_EXCLUSIVE_TIME, "MinExclusiveTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME, "MaxInclusiveDateTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME, "MaxExclusiveDateTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME, "MinInclusiveDateTime" );
    PCR_CONSTASCII_STRING( PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME, "MinExclusiveDateTime" );
    PCR_CONSTASCII_STRING( PROPERTY_SUBMISSION_ID,          "SubmissionID" );
    PCR_CONSTASCII_STRING( PROPERTY_BINDING_ID,             "BindingID" );
    PCR_CONSTASCII_STRING( PROPERTY_ID,                     "ID" );
    PCR_CONSTASCII_STRING( PROPERTY_WRITING_MODE,           "WritingMode" );
    PCR_CONSTASCII_STRING( PROPERTY_TEXT_ANCHOR_TYPE,       "TextAnchorType" );
    PCR_CONSTASCII_STRING( PROPERTY_SHEET_ANCHOR_TYPE,      "SheetAnchorType" );
    PCR_CONSTASCII_STRING( PROPERTY_ANCHOR_TYPE,            "AnchorType" );
    PCR_CONSTASCII_STRING( PROPERTY_ANCHOR,                 "Anchor" );
    PCR_CONSTASCII_STRING( PROPERTY_IS_VISIBLE,             "IsVisible" );

    PCR_CONSTASCII_STRING( PROPERTY_MODEL,                  "Model" );

    PCR_CONSTASCII_STRING( PROPERTY_CELL_EXCHANGE_TYPE,     "ExchangeSelectionIndex");
    PCR_CONSTASCII_STRING( PROPERTY_BOUND_CELL,             "BoundCell");
    PCR_CONSTASCII_STRING( PROPERTY_LIST_CELL_RANGE,        "CellRange");
    PCR_CONSTASCII_STRING( PROPERTY_AUTOLINEBREAK,          "AutomaticLineBreak");
    PCR_CONSTASCII_STRING( PROPERTY_TEXTTYPE,               "TextType");
    PCR_CONSTASCII_STRING( PROPERTY_RICHTEXT,               "RichText");
    PCR_CONSTASCII_STRING( PROPERTY_ROWSET,                 "RowSet");

    // services
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_GROUPBOX,      "com.sun.star.form.component.GroupBox" );
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_FIXEDTEXT,     "com.sun.star.form.component.FixedText" );
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_FORMATTEDFIELD,"com.sun.star.form.component.FormattedField" );
    PCR_CONSTASCII_STRING( SERVICE_DATABASE_CONTEXT,        "com.sun.star.sdb.DatabaseContext" );

    PCR_CONSTASCII_STRING( SERVICE_TEXT_DOCUMENT,           "com.sun.star.text.TextDocument" );
    PCR_CONSTASCII_STRING( SERVICE_WEB_DOCUMENT,            "com.sun.star.text.WebDocument" );
    PCR_CONSTASCII_STRING( SERVICE_SPREADSHEET_DOCUMENT,    "com.sun.star.sheet.SpreadsheetDocument" );
    PCR_CONSTASCII_STRING( SERVICE_DRAWING_DOCUMENT,        "com.sun.star.drawing.DrawingDocument" );
    PCR_CONSTASCII_STRING( SERVICE_PRESENTATION_DOCUMENT,   "com.sun.star.presentation.PresentationDocument" );

    PCR_CONSTASCII_STRING( SERVICE_SHEET_CELL_BINDING,      "com.sun.star.table.CellValueBinding" );
    PCR_CONSTASCII_STRING( SERVICE_SHEET_CELL_INT_BINDING,  "com.sun.star.table.ListPositionCellBinding" );
    PCR_CONSTASCII_STRING( SERVICE_SHEET_CELLRANGE_LISTSOURCE, "com.sun.star.table.CellRangeListSource" );
    PCR_CONSTASCII_STRING( SERVICE_ADDRESS_CONVERSION,      "com.sun.star.table.CellAddressConversion" );
    PCR_CONSTASCII_STRING( SERVICE_RANGEADDRESS_CONVERSION, "com.sun.star.table.CellRangeAddressConversion" );

    PCR_CONSTASCII_STRING( SERVICE_DESKTOP,                 "com.sun.star.frame.Desktop" );

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
