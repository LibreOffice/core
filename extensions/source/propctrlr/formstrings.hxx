/*************************************************************************
 *
 *  $RCSfile: formstrings.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-22 09:32:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#define _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_

#ifndef _EXTENSIONS_FORMSCTRLR_STRINGDEFINE_HXX_
#include "stringdefine.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    // properties
    PCR_CONSTASCII_STRING( PROPERTY_CLASSID,                "ClassId" );
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLLABEL,           "LabelControl");
    PCR_CONSTASCII_STRING( PROPERTY_LABEL,                  "Label");
    PCR_CONSTASCII_STRING( PROPERTY_TABINDEX,               "TabIndex");
    PCR_CONSTASCII_STRING( PROPERTY_TAG,                    "Tag");
    PCR_CONSTASCII_STRING( PROPERTY_NAME,                   "Name");
    PCR_CONSTASCII_STRING( PROPERTY_ROWCOUNT,               "RowCount");
    PCR_CONSTASCII_STRING( PROPERTY_ROWCOUNTFINAL,          "IsRowCountFinal");
    PCR_CONSTASCII_STRING( PROPERTY_FETCHSIZE,              "FetchSize");
    PCR_CONSTASCII_STRING( PROPERTY_VALUE,                  "Value");
    PCR_CONSTASCII_STRING( PROPERTY_TEXT,                   "Text");
    PCR_CONSTASCII_STRING( PROPERTY_CANINSERT,              "CanInsert");
    PCR_CONSTASCII_STRING( PROPERTY_CANUPDATE,              "CanUpdate");
    PCR_CONSTASCII_STRING( PROPERTY_CANDELETE,              "CanDelete");
    PCR_CONSTASCII_STRING( PROPERTY_NAVIGATION,             "NavigationBarMode");
    PCR_CONSTASCII_STRING( PROPERTY_CYCLE,                  "Cycle");
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLSOURCE,          "DataField");
    PCR_CONSTASCII_STRING( PROPERTY_ENABLED,                "Enabled");
    PCR_CONSTASCII_STRING( PROPERTY_READONLY,               "ReadOnly");
    PCR_CONSTASCII_STRING( PROPERTY_ISREADONLY,             "IsReadOnly");
    PCR_CONSTASCII_STRING( PROPERTY_FILTER_CRITERIA,        "Filter");
    PCR_CONSTASCII_STRING( PROPERTY_ISREQUIRED,             "IsRequired");
    PCR_CONSTASCII_STRING( PROPERTY_AUTOINCREMENT,          "IsAutoIncrement");
    PCR_CONSTASCII_STRING( PROPERTY_CACHESIZE,              "CacheSize");
    PCR_CONSTASCII_STRING( PROPERTY_DATAENTRY,              "DataEntry");
    PCR_CONSTASCII_STRING( PROPERTY_LASTDIRTY,              "LastUpdated");
    PCR_CONSTASCII_STRING( PROPERTY_STATEMENT,              "Statement");
    PCR_CONSTASCII_STRING( PROPERTY_WIDTH,                  "Width");
    PCR_CONSTASCII_STRING( PROPERTY_SEARCHABLE,             "IsSearchable");
    PCR_CONSTASCII_STRING( PROPERTY_MULTILINE,              "MultiLine");
    PCR_CONSTASCII_STRING( PROPERTY_TARGET_URL,             "TargetURL");
    PCR_CONSTASCII_STRING( PROPERTY_TARGET_FRAME,           "TargetFrame");
    PCR_CONSTASCII_STRING( PROPERTY_MAXTEXTLEN,             "MaxTextLen");
    PCR_CONSTASCII_STRING( PROPERTY_EDITMASK,               "EditMask");
    PCR_CONSTASCII_STRING( PROPERTY_SPIN,                   "Spin");
    PCR_CONSTASCII_STRING( PROPERTY_DATE,                   "Date");
    PCR_CONSTASCII_STRING( PROPERTY_TIME,                   "Time");
    PCR_CONSTASCII_STRING( PROPERTY_STATE,                  "State");
    PCR_CONSTASCII_STRING( PROPERTY_TRISTATE,               "TriState");
    PCR_CONSTASCII_STRING( PROPERTY_HIDDEN_VALUE,           "HiddenValue");
    PCR_CONSTASCII_STRING( PROPERTY_BUTTONTYPE,             "ButtonType");
    PCR_CONSTASCII_STRING( PROPERTY_STRINGITEMLIST,         "StringItemList");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_TEXT,           "DefaultText");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULTCHECKED,         "DefaultState");
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
    PCR_CONSTASCII_STRING( PROPERTY_SELECT_SEQ,             "SelectedItems");
    PCR_CONSTASCII_STRING( PROPERTY_VALUE_SEQ,              "ValueItemList");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_SELECT_SEQ,     "DefaultSelection");
    PCR_CONSTASCII_STRING( PROPERTY_MULTISELECTION,         "MultiSelection");
    PCR_CONSTASCII_STRING( PROPERTY_ALIGN,                  "Align");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_DATE,           "DefaultDate");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_TIME,           "DefaultTime");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULT_VALUE,          "DefaultValue");
    PCR_CONSTASCII_STRING( PROPERTY_DECIMAL_ACCURACY,       "DecimalAccuracy");
    PCR_CONSTASCII_STRING( PROPERTY_CURSORSOURCE,           "DataSelection");
    PCR_CONSTASCII_STRING( PROPERTY_CURSORSOURCETYPE,       "DataSelectionType");
    PCR_CONSTASCII_STRING( PROPERTY_FIELDTYPE,              "Type");
    PCR_CONSTASCII_STRING( PROPERTY_DECIMALS,               "Decimals");
    PCR_CONSTASCII_STRING( PROPERTY_REFVALUE,               "RefValue");
    PCR_CONSTASCII_STRING( PROPERTY_VALUEMIN,               "ValueMin");
    PCR_CONSTASCII_STRING( PROPERTY_VALUEMAX,               "ValueMax");
    PCR_CONSTASCII_STRING( PROPERTY_STRICTFORMAT,           "StrictFormat");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWADDITIONS,         "AllowInserts");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWEDITS,             "AllowUpdates");
    PCR_CONSTASCII_STRING( PROPERTY_ALLOWDELETIONS,         "AllowDeletes");
    PCR_CONSTASCII_STRING( PROPERTY_MASTERFIELDS,           "MasterFields");
    PCR_CONSTASCII_STRING( PROPERTY_ISPASSTHROUGH,          "IsPassThrough");
    PCR_CONSTASCII_STRING( PROPERTY_QUERY,                  "Query");
    PCR_CONSTASCII_STRING( PROPERTY_LITERALMASK,            "LiteralMask");
    PCR_CONSTASCII_STRING( PROPERTY_VALUESTEP,              "ValueStep");
    PCR_CONSTASCII_STRING( PROPERTY_SHOWTHOUSANDSEP,        "ShowThousandsSeparator");
    PCR_CONSTASCII_STRING( PROPERTY_CURRENCYSYMBOL,         "CurrencySymbol");
    PCR_CONSTASCII_STRING( PROPERTY_DATEFORMAT,             "DateFormat");
    PCR_CONSTASCII_STRING( PROPERTY_DATEMIN,                "DateMin");
    PCR_CONSTASCII_STRING( PROPERTY_DATEMAX,                "DateMax");
    PCR_CONSTASCII_STRING( PROPERTY_DATE_SHOW_CENTURY,      "DateShowCentury");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEFORMAT,             "TimeFormat");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEMIN,                "TimeMin");
    PCR_CONSTASCII_STRING( PROPERTY_TIMEMAX,                "TimeMax");
    PCR_CONSTASCII_STRING( PROPERTY_LINECOUNT,              "LineCount");
    PCR_CONSTASCII_STRING( PROPERTY_BOUNDCOLUMN,            "BoundColumn");
    PCR_CONSTASCII_STRING( PROPERTY_FONT,                   "FontDescriptor");
    PCR_CONSTASCII_STRING( PROPERTY_BACKGROUNDCOLOR,        "BackgroundColor");
    PCR_CONSTASCII_STRING( PROPERTY_FILLCOLOR,              "FillColor");
    PCR_CONSTASCII_STRING( PROPERTY_TEXTCOLOR,              "TextColor");
    PCR_CONSTASCII_STRING( PROPERTY_LINECOLOR,              "LineColor");
    PCR_CONSTASCII_STRING( PROPERTY_BORDER,                 "Border");
    PCR_CONSTASCII_STRING( PROPERTY_DROPDOWN,               "Dropdown");
    PCR_CONSTASCII_STRING( PROPERTY_MULTI,                  "Multi");
    PCR_CONSTASCII_STRING( PROPERTY_HSCROLL,                "HScroll");
    PCR_CONSTASCII_STRING( PROPERTY_VSCROLL,                "VScroll");
    PCR_CONSTASCII_STRING( PROPERTY_TABSTOP,                "Tabstop");
    PCR_CONSTASCII_STRING( PROPERTY_AUTOCOMPLETE,           "Autocomplete");
    PCR_CONSTASCII_STRING( PROPERTY_HARDLINEBREAKS,         "HardLineBreaks");
    PCR_CONSTASCII_STRING( PROPERTY_PRINTABLE,              "Printable");
    PCR_CONSTASCII_STRING( PROPERTY_ECHO_CHAR,              "EchoChar");
    PCR_CONSTASCII_STRING( PROPERTY_ROWHEIGHT,              "RowHeight");
    PCR_CONSTASCII_STRING( PROPERTY_HELPTEXT,               "HelpText");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_NAME,              "FontName");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_STYLENAME,         "FontStyleName");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_FAMILY,            "FontFamily");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_CHARSET,           "FontCharset");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_HEIGHT,            "FontHeight");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_WEIGHT,            "FontWeight");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_SLANT,             "FontSlant");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_UNDERLINE,         "FontUnderline");
    PCR_CONSTASCII_STRING( PROPERTY_FONT_STRIKEOUT,         "FontStrikeout");
    PCR_CONSTASCII_STRING( PROPERTY_HELPURL,                "HelpURL");
    PCR_CONSTASCII_STRING( PROPERTY_RECORDMARKER,           "HasRecordMarker");
    PCR_CONSTASCII_STRING( PROPERTY_BOUNDFIELD,             "BoundField");
    PCR_CONSTASCII_STRING( PROPERTY_TREATASNUMERIC,         "TreatAsNumber");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_VALUE,        "EffectiveValue");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_DEFAULT,      "EffectiveDefault");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MIN,          "EffectiveMin");
    PCR_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MAX,          "EffectiveMax");
    PCR_CONSTASCII_STRING( PROPERTY_HIDDEN,                 "Hidden");
    PCR_CONSTASCII_STRING( PROPERTY_FILTERPROPOSAL,         "UseFilterValueProposal");
    PCR_CONSTASCII_STRING( PROPERTY_FIELDSOURCE,            "FieldSource");
    PCR_CONSTASCII_STRING( PROPERTY_TABLENAME,              "TableName");
    PCR_CONSTASCII_STRING( PROPERTY_FILTERSUPPLIER,         "FilterSupplier");
    PCR_CONSTASCII_STRING( PROPERTY_CURRENTFILTER,          "CurrentFilter");
    PCR_CONSTASCII_STRING( PROPERTY_SELECTED_FIELDS,        "SelectedFields");
    PCR_CONSTASCII_STRING( PROPERTY_SELECTED_TABLES,        "SelectedTables");
    PCR_CONSTASCII_STRING( PROPERTY_THREADSAFE,             "ThreadSafe");
    PCR_CONSTASCII_STRING( PROPERTY_ISFILTERAPPLIED,        "IsFilterApplied");
    PCR_CONSTASCII_STRING( PROPERTY_CURRSYM_POSITION,       "PrependCurrencySymbol");
    PCR_CONSTASCII_STRING( PROPERTY_SOURCE,                 "Source");
    PCR_CONSTASCII_STRING( PROPERTY_CURSORCOLOR,            "CursorColor");
    PCR_CONSTASCII_STRING( PROPERTY_ALWAYSSHOWCURSOR,       "AlwaysShowCursor");
    PCR_CONSTASCII_STRING( PROPERTY_DISPLAYSYNCHRON,        "DisplayIsSynchron");
    PCR_CONSTASCII_STRING( PROPERTY_ISMODIFIED,             "IsModified");
    PCR_CONSTASCII_STRING( PROPERTY_ISNEW,                  "IsNew");
    PCR_CONSTASCII_STRING( PROPERTY_PRIVILEGES,             "Privileges");
    PCR_CONSTASCII_STRING( PROPERTY_COMMAND,                "Command");
    PCR_CONSTASCII_STRING( PROPERTY_COMMANDTYPE,            "CommandType");
    PCR_CONSTASCII_STRING( PROPERTY_RESULTSET_CONCURRENCY,  "ResultSetConcurrency");
    PCR_CONSTASCII_STRING( PROPERTY_INSERTONLY,             "IgnoreResult");
    PCR_CONSTASCII_STRING( PROPERTY_RESULTSET_TYPE,         "ResultSetType");
    PCR_CONSTASCII_STRING( PROPERTY_ESCAPE_PROCESSING,      "EscapeProcessing");
    PCR_CONSTASCII_STRING( PROPERTY_APPLYFILTER,            "ApplyFilter");
    PCR_CONSTASCII_STRING( PROPERTY_ISNULLABLE,             "IsNullable");
    PCR_CONSTASCII_STRING( PROPERTY_ACTIVECOMMAND,          "ActiveCommand");
    PCR_CONSTASCII_STRING( PROPERTY_ISCURRENCY,             "IsCurrency");
    PCR_CONSTASCII_STRING( PROPERTY_NUMBER_FORMATS_SUPPLIER,"NumberFormatsSupplier");
    PCR_CONSTASCII_STRING( PROPERTY_URL,                    "URL");
    PCR_CONSTASCII_STRING( PROPERTY_TITLE,                  "Title");
    PCR_CONSTASCII_STRING( PROPERTY_ACTIVE_CONNECTION,      "ActiveConnection");
    PCR_CONSTASCII_STRING( PROPERTY_SCALE,                  "Scale");
    PCR_CONSTASCII_STRING( PROPERTY_SORT,                   "Order");
    PCR_CONSTASCII_STRING( PROPERTY_DATASOURCE,             "DataSourceName");
    PCR_CONSTASCII_STRING( PROPERTY_DETAILFIELDS,           "DetailFields");
    PCR_CONSTASCII_STRING( PROPERTY_REALNAME,               "RealName");
    PCR_CONSTASCII_STRING( PROPERTY_CONTROLSOURCEPROPERTY,  "DataFieldProperty");
    PCR_CONSTASCII_STRING( PROPERTY_DEFAULTBUTTON,          "DefaultButton");
    PCR_CONSTASCII_STRING( PROPERTY_LISTINDEX,              "ListIndex");
    PCR_CONSTASCII_STRING( PROPERTY_HEIGHT,                 "Height");
    PCR_CONSTASCII_STRING( PROPERTY_HASNAVIGATION,          "HasNavigationBar");
    PCR_CONSTASCII_STRING( PROPERTY_POSITIONX,              "PositionX");
    PCR_CONSTASCII_STRING( PROPERTY_POSITIONY,              "PositionY");

    // services
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_GROUPBOX,      "com.sun.star.form.component.GroupBox");
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_FIXEDTEXT,     "com.sun.star.form.component.FixedText");
    PCR_CONSTASCII_STRING( SERVICE_COMPONENT_FORMATTEDFIELD,"com.sun.star.form.component.FormattedField");
    PCR_CONSTASCII_STRING( SERVICE_DATABASE_CONTEXT,        "com.sun.star.sdb.DatabaseContext");

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/02/06 10:18:10  fs
 *  #83479# reintroduced the HasNavigationBar property
 *
 *  Revision 1.2  2001/01/17 08:44:07  fs
 *  #82726# removed an obsolete string
 *
 *  Revision 1.1  2001/01/12 11:29:10  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 14:17:16  fs
 ************************************************************************/

