/*************************************************************************
 *
 *  $RCSfile: strings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-19 15:41:32 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#define _XMLOFF_FORMS_STRINGS_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//============================================================
//= a helper for static ascii pseudo-unicode strings
//============================================================
struct _ConstAsciiString_
{
    sal_Int32 length;
    sal_Char  const* str;

    operator ::rtl::OUString () const { return ::rtl::OUString(str, length, RTL_TEXTENCODING_ASCII_US); }
    operator const sal_Char* () const { return str; }
};

#ifndef XMLFORM_IMPLEMENT_STRINGS
#define XMLFORM_CONSTASCII_STRING(ident, string) extern const _ConstAsciiString_ ident
#else
#define XMLFORM_CONSTASCII_STRING(ident, string) extern const _ConstAsciiString_ ident = { sizeof(string)-1, string }
#endif

//============================================================
//= string constants
//============================================================

//.........................................................................
namespace xmloff
{
//.........................................................................

    // properties
    XMLFORM_CONSTASCII_STRING( PROPERTY_CLASSID,            "ClassId" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ECHOCHAR,           "EchoChar" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_MULTILINE,          "MultiLine" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_NAME,               "Name" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_IMAGEURL,           "ImageURL" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_LABEL,              "Label" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TARGETFRAME,        "TargetFrame" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TARGETURL,          "TargetURL" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TITLE,              "Tag" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DROPDOWN,           "Dropdown" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_PRINTABLE,          "Printable" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_READONLY,           "ReadOnly" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_STATE,      "DefaultState" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TABSTOP,            "Tabstop" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_STATE,              "State" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ENABLED,            "Enabled" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_MAXTEXTLENGTH,      "MaxTextLen" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_LINECOUNT,          "LineCount" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_TABINDEX,           "TabIndex" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_COMMAND,            "Command" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATASOURCENAME,     "DataSourceName" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_FILTER,             "Filter" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ORDER,              "Order" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWDELETES,       "AllowDeletes" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWINSERTS,       "AllowInserts" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ALLOWUPDATES,       "AllowUpdates" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_APPLYFILTER,        "ApplyFilter" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_ESCAPEPROCESSING,   "EscapeProcessing" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_IGNORERESULT,       "IgnoreResult" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_SUBMIT_ENCODING,    "SubmitEncoding" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_SUBMIT_METHOD,      "SubmitMethod" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_COMMAND_TYPE,       "CommandType" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_NAVIGATION,         "NavigationBarMode" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_CYCLE,              "Cycle" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_BUTTONTYPE,         "ButtonType" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATAFIELD,          "DataField" );
    XMLFORM_CONSTASCII_STRING( PROPERTY_BOUNDCOLUMN,        "BoundColumn");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EMPTY_IS_NULL,      "ConvertEmptyToNull");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LISTSOURCE,         "ListSource");
    XMLFORM_CONSTASCII_STRING( PROPERTY_LISTSOURCETYPE,     "ListSourceType");
    XMLFORM_CONSTASCII_STRING( PROPERTY_ECHO_CHAR,          "EchoChar");
    XMLFORM_CONSTASCII_STRING( PROPERTY_STRICTFORMAT,       "StrictFormat");
    XMLFORM_CONSTASCII_STRING( PROPERTY_AUTOCOMPLETE,       "Autocomplete");
    XMLFORM_CONSTASCII_STRING( PROPERTY_MULTISELECTION,     "MultiSelection");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULTBUTTON,      "DefaultButton");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TRISTATE,           "TriState");
    XMLFORM_CONSTASCII_STRING( PROPERTY_CONTROLLABEL,       "LabelControl");
    XMLFORM_CONSTASCII_STRING( PROPERTY_STRING_ITEM_LIST,   "StringItemList");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_SEQ,          "ValueItemList");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_SELECT_SEQ, "DefaultSelection");
    XMLFORM_CONSTASCII_STRING( PROPERTY_SELECT_SEQ,         "SelectedItems");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE_MIN,           "DateMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE_MAX,           "DateMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME_MIN,           "TimeMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME_MAX,           "TimeMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_MIN,          "ValueMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE_MAX,          "ValueMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MIN,      "EffectiveMin");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MAX,      "EffectiveMax");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_DATE,       "DefaultDate");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DATE,               "Date");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_TIME,       "DefaultTime");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TIME,               "Time");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_VALUE,      "DefaultValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_VALUE,              "Value");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DEFAULT_TEXT,       "DefaultText");
    XMLFORM_CONSTASCII_STRING( PROPERTY_TEXT,               "Text");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_VALUE,    "EffectiveValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_EFFECTIVE_DEFAULT,  "EffectiveDefault");
    XMLFORM_CONSTASCII_STRING( PROPERTY_REFVALUE,           "RefValue");
    XMLFORM_CONSTASCII_STRING( PROPERTY_URL,                "URL");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FONT,               "FontDescriptor");
    XMLFORM_CONSTASCII_STRING( PROPERTY_BACKGROUNDCOLOR,    "BackgroundColor");
    XMLFORM_CONSTASCII_STRING( PROPERTY_MASTERFIELDS,       "MasterFields");
    XMLFORM_CONSTASCII_STRING( PROPERTY_DETAILFIELDS,       "DetailFields");
    XMLFORM_CONSTASCII_STRING( PROPERTY_COLUMNSERVICENAME,  "ColumnServiceName");
    XMLFORM_CONSTASCII_STRING( PROPERTY_FORMATKEY,          "FormatKey");

    // services
    XMLFORM_CONSTASCII_STRING( SERVICE_FORMSCOLLECTION, "com.sun.star.form.Forms" );

    // old service names (compatibility)
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FORM,           "stardiv.one.form.component.Form");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_EDIT,           "stardiv.one.form.component.Edit");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_LISTBOX,        "stardiv.one.form.component.ListBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_COMBOBOX,       "stardiv.one.form.component.ComboBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_RADIOBUTTON,    "stardiv.one.form.component.RadioButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_GROUPBOX,       "stardiv.one.form.component.GroupBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FIXEDTEXT,      "stardiv.one.form.component.FixedText");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_COMMANDBUTTON,  "stardiv.one.form.component.CommandButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_CHECKBOX,       "stardiv.one.form.component.CheckBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_GRID,           "stardiv.one.form.component.Grid");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_IMAGEBUTTON,    "stardiv.one.form.component.ImageButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FILECONTROL,    "stardiv.one.form.component.FileControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_TIMEFIELD,      "stardiv.one.form.component.TimeField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_DATEFIELD,      "stardiv.one.form.component.DateField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_NUMERICFIELD,   "stardiv.one.form.component.NumericField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_CURRENCYFIELD,  "stardiv.one.form.component.CurrencyField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_PATTERNFIELD,   "stardiv.one.form.component.PatternField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_HIDDENCONTROL,  "stardiv.one.form.component.Hidden");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_IMAGECONTROL,   "stardiv.one.form.component.ImageControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_PERSISTENT_COMPONENT_FORMATTEDFIELD, "stardiv.one.form.component.FormattedField");

    // new service names, the old ones are translated into this new ones
    XMLFORM_CONSTASCII_STRING( SERVICE_FORM,            "com.sun.star.form.component.Form");
    XMLFORM_CONSTASCII_STRING( SERVICE_EDIT,            "com.sun.star.form.component.TextField");
    XMLFORM_CONSTASCII_STRING( SERVICE_LISTBOX,         "com.sun.star.form.component.ListBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_COMBOBOX,        "com.sun.star.form.component.ComboBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_RADIOBUTTON,     "com.sun.star.form.component.RadioButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_GROUPBOX,        "com.sun.star.form.component.GroupBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_FIXEDTEXT,       "com.sun.star.form.component.FixedText");
    XMLFORM_CONSTASCII_STRING( SERVICE_COMMANDBUTTON,   "com.sun.star.form.component.CommandButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_CHECKBOX,        "com.sun.star.form.component.CheckBox");
    XMLFORM_CONSTASCII_STRING( SERVICE_GRID,            "com.sun.star.form.component.GridControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_IMAGEBUTTON,     "com.sun.star.form.component.ImageButton");
    XMLFORM_CONSTASCII_STRING( SERVICE_FILECONTROL,     "com.sun.star.form.component.FileControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_TIMEFIELD,       "com.sun.star.form.component.TimeField");
    XMLFORM_CONSTASCII_STRING( SERVICE_DATEFIELD,       "com.sun.star.form.component.DateField");
    XMLFORM_CONSTASCII_STRING( SERVICE_NUMERICFIELD,    "com.sun.star.form.component.NumericField");
    XMLFORM_CONSTASCII_STRING( SERVICE_CURRENCYFIELD,   "com.sun.star.form.component.CurrencyField");
    XMLFORM_CONSTASCII_STRING( SERVICE_PATTERNFIELD,    "com.sun.star.form.component.PatternField");
    XMLFORM_CONSTASCII_STRING( SERVICE_HIDDENCONTROL,   "com.sun.star.form.component.HiddenControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_IMAGECONTROL,    "com.sun.star.form.component.DatabaseImageControl");
    XMLFORM_CONSTASCII_STRING( SERVICE_FORMATTEDFIELD,  "com.sun.star.form.component.FormattedField" );

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_STRINGS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/11/17 19:03:33  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 13.11.00 17:00:01  fs
 ************************************************************************/

