/*************************************************************************
 *
 *  $RCSfile: frm_strings.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:18:31 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef FRM_STRINGS_HXX
#define FRM_STRINGS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//..............................................................................
namespace frm
{
//..............................................................................

    struct ConstAsciiString
    {
        const sal_Char* ascii;
        sal_Int32       length;

        inline  operator const ::rtl::OUString () const;
        inline  operator const sal_Char* () const { return ascii; }

        inline ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength);
        inline ~ConstAsciiString();

    private:
        mutable ::rtl::OUString*    ustring;
    };

    //------------------------------------------------------------
    inline ConstAsciiString::ConstAsciiString(const sal_Char* _pAsciiZeroTerminated, const sal_Int32 _nLength)
        :ascii(_pAsciiZeroTerminated)
        ,length(_nLength)
        ,ustring(NULL)
    {
    }

    //------------------------------------------------------------
    inline ConstAsciiString::~ConstAsciiString()
    {
        delete ustring;
        ustring = NULL;
    }

    //------------------------------------------------------------
    inline ConstAsciiString::operator const ::rtl::OUString () const
    {
        if ( !ustring )
            ustring = new ::rtl::OUString( ascii, length, RTL_TEXTENCODING_ASCII_US );
        return *ustring;
    }

    #ifndef FORMS_IMPLEMENT_STRINGS
        #define FORMS_CONSTASCII_STRING( ident, string )    \
            extern const ConstAsciiString ident
    #else
        #define FORMS_CONSTASCII_STRING( ident, string )    \
            extern const ConstAsciiString ident( string, sizeof( string )-1 )
    #endif

//..............................................................................
}   // namespace frm
//..............................................................................

//..............................................................................
namespace frm
{
//..............................................................................

    //--------------------------------------------------------------------------
    //- properties
    //--------------------------------------------------------------------------
    FORMS_CONSTASCII_STRING( PROPERTY_TABINDEX,                 "TabIndex" );
    FORMS_CONSTASCII_STRING( PROPERTY_TAG,                      "Tag" );
    FORMS_CONSTASCII_STRING( PROPERTY_NAME,                     "Name" );
    FORMS_CONSTASCII_STRING( PROPERTY_CLASSID,                  "ClassId" );
    FORMS_CONSTASCII_STRING( PROPERTY_FETCHSIZE,                "FetchSize" );
    FORMS_CONSTASCII_STRING( PROPERTY_VALUE,                    "Value" );
    FORMS_CONSTASCII_STRING( PROPERTY_TEXT,                     "Text" );
    FORMS_CONSTASCII_STRING( PROPERTY_LABEL,                    "Label" );
    FORMS_CONSTASCII_STRING( PROPERTY_CANINSERT,                "CanInsert" );
    FORMS_CONSTASCII_STRING( PROPERTY_CANUPDATE,                "CanUpdate" );
    FORMS_CONSTASCII_STRING( PROPERTY_CANDELETE,                "CanDelete" );
    FORMS_CONSTASCII_STRING( PROPERTY_NAVIGATION,               "NavigationBarMode" );
    FORMS_CONSTASCII_STRING( PROPERTY_HASNAVIGATION,            "HasNavigationBar" );
    FORMS_CONSTASCII_STRING( PROPERTY_CYCLE,                    "Cycle" );
    FORMS_CONSTASCII_STRING( PROPERTY_CONTROLSOURCE,            "DataField" );
    FORMS_CONSTASCII_STRING( PROPERTY_ENABLED,                  "Enabled" );
    FORMS_CONSTASCII_STRING( PROPERTY_READONLY,                 "ReadOnly" );
    FORMS_CONSTASCII_STRING( PROPERTY_ISREADONLY,               "IsReadOnly" );
    FORMS_CONSTASCII_STRING( PROPERTY_FILTER,                   "Filter" );
    FORMS_CONSTASCII_STRING( PROPERTY_WIDTH,                    "Width" );
    FORMS_CONSTASCII_STRING( PROPERTY_SEARCHABLE,               "IsSearchable" );
    FORMS_CONSTASCII_STRING( PROPERTY_MULTILINE,                "MultiLine" );
    FORMS_CONSTASCII_STRING( PROPERTY_TARGET_URL,               "TargetURL" );
    FORMS_CONSTASCII_STRING( PROPERTY_TARGET_FRAME,             "TargetFrame" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULTCONTROL,           "DefaultControl" );
    FORMS_CONSTASCII_STRING( PROPERTY_MAXTEXTLEN,               "MaxTextLen" );
    FORMS_CONSTASCII_STRING( PROPERTY_EDITMASK,                 "EditMask" );
    FORMS_CONSTASCII_STRING( PROPERTY_SIZE,                     "Size" );
    FORMS_CONSTASCII_STRING( PROPERTY_SPIN,                     "Spin" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATE,                     "Date" );
    FORMS_CONSTASCII_STRING( PROPERTY_TIME,                     "Time" );
    FORMS_CONSTASCII_STRING( PROPERTY_STATE,                    "State" );
    FORMS_CONSTASCII_STRING( PROPERTY_TRISTATE,                 "TriState" );
    FORMS_CONSTASCII_STRING( PROPERTY_HIDDEN_VALUE,             "HiddenValue" );
    FORMS_CONSTASCII_STRING( PROPERTY_BUTTONTYPE,               "ButtonType" );
    FORMS_CONSTASCII_STRING( PROPERTY_STRINGITEMLIST,           "StringItemList" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_TEXT,             "DefaultText" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULTCHECKED,           "DefaultState" );
    FORMS_CONSTASCII_STRING( PROPERTY_FORMATKEY,                "FormatKey" );
    FORMS_CONSTASCII_STRING( PROPERTY_FORMATSSUPPLIER,          "FormatsSupplier" );
    FORMS_CONSTASCII_STRING( PROPERTY_SUBMIT_ACTION,            "SubmitAction" );
    FORMS_CONSTASCII_STRING( PROPERTY_SUBMIT_TARGET,            "SubmitTarget" );
    FORMS_CONSTASCII_STRING( PROPERTY_SUBMIT_METHOD,            "SubmitMethod" );
    FORMS_CONSTASCII_STRING( PROPERTY_SUBMIT_ENCODING,          "SubmitEncoding" );
    FORMS_CONSTASCII_STRING( PROPERTY_IMAGE_URL,                "ImageURL" );
    FORMS_CONSTASCII_STRING( PROPERTY_EMPTY_IS_NULL,            "ConvertEmptyToNull" );
    FORMS_CONSTASCII_STRING( PROPERTY_LISTSOURCETYPE,           "ListSourceType" );
    FORMS_CONSTASCII_STRING( PROPERTY_LISTSOURCE,               "ListSource" );
    FORMS_CONSTASCII_STRING( PROPERTY_SELECT_SEQ,               "SelectedItems" );
    FORMS_CONSTASCII_STRING( PROPERTY_VALUE_SEQ,                "ValueItemList" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_SELECT_SEQ,       "DefaultSelection" );
    FORMS_CONSTASCII_STRING( PROPERTY_MULTISELECTION,           "MultiSelection" );
    FORMS_CONSTASCII_STRING( PROPERTY_ALIGN,                    "Align" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_DATE,             "DefaultDate" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_TIME,             "DefaultTime" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_VALUE,            "DefaultValue" );
    FORMS_CONSTASCII_STRING( PROPERTY_DECIMAL_ACCURACY,         "DecimalAccuracy" );
    FORMS_CONSTASCII_STRING( PROPERTY_CURSORSOURCE,             "DataSelection" );
    FORMS_CONSTASCII_STRING( PROPERTY_CURSORSOURCETYPE,         "DataSelectionType" );
    FORMS_CONSTASCII_STRING( PROPERTY_FIELDTYPE,                "Type" );
    FORMS_CONSTASCII_STRING( PROPERTY_DECIMALS,                 "Decimals" );
    FORMS_CONSTASCII_STRING( PROPERTY_REFVALUE,                 "RefValue" );
    FORMS_CONSTASCII_STRING( PROPERTY_VALUEMIN,                 "ValueMin" );
    FORMS_CONSTASCII_STRING( PROPERTY_VALUEMAX,                 "ValueMax" );
    FORMS_CONSTASCII_STRING( PROPERTY_STRICTFORMAT,             "StrictFormat" );
    FORMS_CONSTASCII_STRING( PROPERTY_ALLOWADDITIONS,           "AllowInserts" );
    FORMS_CONSTASCII_STRING( PROPERTY_ALLOWEDITS,               "AllowUpdates" );
    FORMS_CONSTASCII_STRING( PROPERTY_ALLOWDELETIONS,           "AllowDeletes" );
    FORMS_CONSTASCII_STRING( PROPERTY_MASTERFIELDS,             "MasterFields" );
    FORMS_CONSTASCII_STRING( PROPERTY_ISPASSTHROUGH,            "IsPassThrough" );
    FORMS_CONSTASCII_STRING( PROPERTY_QUERY,                    "Query" );
    FORMS_CONSTASCII_STRING( PROPERTY_LITERALMASK,              "LiteralMask" );
    FORMS_CONSTASCII_STRING( PROPERTY_VALUESTEP,                "ValueStep" );
    FORMS_CONSTASCII_STRING( PROPERTY_SHOWTHOUSANDSEP,          "ShowThousandsSeparator" );
    FORMS_CONSTASCII_STRING( PROPERTY_CURRENCYSYMBOL,           "CurrencySymbol" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATEFORMAT,               "DateFormat" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATEMIN,                  "DateMin" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATEMAX,                  "DateMax" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATE_SHOW_CENTURY,        "DateShowCentury" );
    FORMS_CONSTASCII_STRING( PROPERTY_TIMEFORMAT,               "TimeFormat" );
    FORMS_CONSTASCII_STRING( PROPERTY_TIMEMIN,                  "TimeMin" );
    FORMS_CONSTASCII_STRING( PROPERTY_TIMEMAX,                  "TimeMax" );
    FORMS_CONSTASCII_STRING( PROPERTY_LINECOUNT,                "LineCount" );
    FORMS_CONSTASCII_STRING( PROPERTY_BOUNDCOLUMN,              "BoundColumn" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT,                     "FontDescriptor" );
    FORMS_CONSTASCII_STRING( PROPERTY_FILLCOLOR,                "FillColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_LINECOLOR,                "LineColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_DROPDOWN,                 "Dropdown" );
    FORMS_CONSTASCII_STRING( PROPERTY_HSCROLL,                  "HScroll" );
    FORMS_CONSTASCII_STRING( PROPERTY_VSCROLL,                  "VScroll" );
    FORMS_CONSTASCII_STRING( PROPERTY_TABSTOP,                  "Tabstop" );
    FORMS_CONSTASCII_STRING( PROPERTY_AUTOCOMPLETE,             "Autocomplete" );
    FORMS_CONSTASCII_STRING( PROPERTY_HARDLINEBREAKS,           "HardLineBreaks" );
    FORMS_CONSTASCII_STRING( PROPERTY_PRINTABLE,                "Printable" );
    FORMS_CONSTASCII_STRING( PROPERTY_ECHO_CHAR,                "EchoChar" );
    FORMS_CONSTASCII_STRING( PROPERTY_ROWHEIGHT,                "RowHeight" );
    FORMS_CONSTASCII_STRING( PROPERTY_HELPTEXT,                 "HelpText" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_NAME,                "FontName" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_STYLENAME,           "FontStyleName" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_FAMILY,              "FontFamily" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_CHARSET,             "FontCharset" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_HEIGHT,              "FontHeight" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_WEIGHT,              "FontWeight" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_SLANT,               "FontSlant" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_UNDERLINE,           "FontUnderline" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_WORDLINEMODE,        "FontWordLineMode" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_STRIKEOUT,           "FontStrikeout" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONTEMPHASISMARK,         "FontEmphasisMark" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONTRELIEF,               "FontRelief" );
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_CHARWIDTH,           "FontCharWidth");
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_KERNING,             "FontKerning");
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_ORIENTATION,         "FontOrientation");
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_PITCH,               "FontPitch");
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_TYPE,                "FontType");
    FORMS_CONSTASCII_STRING( PROPERTY_FONT_WIDTH,               "FontWidth");
    FORMS_CONSTASCII_STRING( PROPERTY_HELPURL,                  "HelpURL" );
    FORMS_CONSTASCII_STRING( PROPERTY_RECORDMARKER,             "HasRecordMarker" );
    FORMS_CONSTASCII_STRING( PROPERTY_BOUNDFIELD,               "BoundField" );
    FORMS_CONSTASCII_STRING( PROPERTY_TREATASNUMERIC,           "TreatAsNumber" );
    FORMS_CONSTASCII_STRING( PROPERTY_EFFECTIVE_VALUE,          "EffectiveValue" );
    FORMS_CONSTASCII_STRING( PROPERTY_EFFECTIVE_DEFAULT,        "EffectiveDefault" );
    FORMS_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MIN,            "EffectiveMin" );
    FORMS_CONSTASCII_STRING( PROPERTY_EFFECTIVE_MAX,            "EffectiveMax" );
    FORMS_CONSTASCII_STRING( PROPERTY_HIDDEN,                   "Hidden" );
    FORMS_CONSTASCII_STRING( PROPERTY_FILTERPROPOSAL,           "UseFilterValueProposal" );
    FORMS_CONSTASCII_STRING( PROPERTY_FIELDSOURCE,              "FieldSource" );
    FORMS_CONSTASCII_STRING( PROPERTY_TABLENAME,                "TableName" );
    FORMS_CONSTASCII_STRING( PROPERTY_ISFILTERAPPLIED,          "IsFilterApplied" );
    FORMS_CONSTASCII_STRING( PROPERTY_CONTROLLABEL,             "LabelControl" );
    FORMS_CONSTASCII_STRING( PROPERTY_CURRSYM_POSITION,         "PrependCurrencySymbol" );
    FORMS_CONSTASCII_STRING( PROPERTY_CURSORCOLOR,              "CursorColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_ALWAYSSHOWCURSOR,         "AlwaysShowCursor" );
    FORMS_CONSTASCII_STRING( PROPERTY_DISPLAYSYNCHRON,          "DisplayIsSynchron" );
    FORMS_CONSTASCII_STRING( PROPERTY_TEXTCOLOR,                "TextColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_DELAY,                    "RepeatDelay" );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_SCROLL_VALUE,     "DefaultScrollValue" );
    FORMS_CONSTASCII_STRING( PROPERTY_SCROLL_VALUE,             "ScrollValue"        );
    FORMS_CONSTASCII_STRING( PROPERTY_DEFAULT_SPIN_VALUE,       "DefaultSpinValue"   );
    FORMS_CONSTASCII_STRING( PROPERTY_SPIN_VALUE,               "SpinValue"          );
    FORMS_CONSTASCII_STRING( PROPERTY_REFERENCE_DEVICE,         "ReferenceDevice"    );
    FORMS_CONSTASCII_STRING( PROPERTY_ISMODIFIED,               "IsModified" );
    FORMS_CONSTASCII_STRING( PROPERTY_ISNEW,                    "IsNew" );
    FORMS_CONSTASCII_STRING( PROPERTY_PRIVILEGES,               "Privileges" );
    FORMS_CONSTASCII_STRING( PROPERTY_COMMAND,                  "Command" );
    FORMS_CONSTASCII_STRING( PROPERTY_COMMANDTYPE,              "CommandType" );
    FORMS_CONSTASCII_STRING( PROPERTY_RESULTSET_CONCURRENCY,    "ResultSetConcurrency" );
    FORMS_CONSTASCII_STRING( PROPERTY_INSERTONLY,               "IgnoreResult" );
    FORMS_CONSTASCII_STRING( PROPERTY_RESULTSET_TYPE,           "ResultSetType" );
    FORMS_CONSTASCII_STRING( PROPERTY_ESCAPE_PROCESSING,        "EscapeProcessing" );
    FORMS_CONSTASCII_STRING( PROPERTY_APPLYFILTER,              "ApplyFilter" );

    FORMS_CONSTASCII_STRING( PROPERTY_ISNULLABLE,               "IsNullable" );
    FORMS_CONSTASCII_STRING( PROPERTY_ACTIVECOMMAND,            "ActiveCommand" );
    FORMS_CONSTASCII_STRING( PROPERTY_ISCURRENCY,               "IsCurrency" );
    FORMS_CONSTASCII_STRING( PROPERTY_URL,                      "URL" );
    FORMS_CONSTASCII_STRING( PROPERTY_TITLE,                    "Title" );
    FORMS_CONSTASCII_STRING( PROPERTY_ACTIVE_CONNECTION,        "ActiveConnection" );
    FORMS_CONSTASCII_STRING( PROPERTY_SCALE,                    "Scale" );
    FORMS_CONSTASCII_STRING( PROPERTY_SORT,                     "Order" );
    FORMS_CONSTASCII_STRING( PROPERTY_DATASOURCE,               "DataSourceName" );
    FORMS_CONSTASCII_STRING( PROPERTY_DETAILFIELDS,             "DetailFields" );

    FORMS_CONSTASCII_STRING( PROPERTY_COLUMNSERVICENAME,        "ColumnServiceName" );
    FORMS_CONSTASCII_STRING( PROPERTY_REALNAME,                 "RealName" );
    FORMS_CONSTASCII_STRING( PROPERTY_CONTROLSOURCEPROPERTY,    "DataFieldProperty" );
    FORMS_CONSTASCII_STRING( PROPERTY_USER,                     "User" );
    FORMS_CONSTASCII_STRING( PROPERTY_PASSWORD,                 "Password" );
    FORMS_CONSTASCII_STRING( PROPERTY_DISPATCHURLINTERNAL,      "DispatchURLInternal" );
    FORMS_CONSTASCII_STRING( PROPERTY_PERSISTENCE_MAXTEXTLENGTH,"PersistenceMaxTextLength" );
    FORMS_CONSTASCII_STRING( PROPERTY_RICH_TEXT,                "RichText" );
    FORMS_CONSTASCII_STRING( PROPERTY_ENFORCE_FORMAT,           "EnforceFormat" );

    FORMS_CONSTASCII_STRING( PROPERTY_BORDER,                   "Border" );
    FORMS_CONSTASCII_STRING( PROPERTY_BORDERCOLOR,              "BorderColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_BACKGROUNDCOLOR,          "BackgroundColor"   );
    FORMS_CONSTASCII_STRING( PROPERTY_ICONSIZE,                 "IconSize"        );
    FORMS_CONSTASCII_STRING( PROPERTY_TEXTLINECOLOR,            "TextLineColor" );
    FORMS_CONSTASCII_STRING( PROPERTY_HIDEINACTIVESELECTION,    "HideInactiveSelection" );

    FORMS_CONSTASCII_STRING( PROPERTY_SHOW_POSITION,            "ShowPosition" );
    FORMS_CONSTASCII_STRING( PROPERTY_SHOW_NAVIGATION,          "ShowNavigation" );
    FORMS_CONSTASCII_STRING( PROPERTY_SHOW_RECORDACTIONS,       "ShowRecordActions" );
    FORMS_CONSTASCII_STRING( PROPERTY_SHOW_FILTERSORT,          "ShowFilterSort" );

    FORMS_CONSTASCII_STRING( PROPERTY_DYNAMIC_CONTROL_BORDER,   "DynamicControlBorder" );
    FORMS_CONSTASCII_STRING( PROPERTY_CONTROL_BORDER_COLOR_FOCUS, "ControlBorderColorOnFocus" );
    FORMS_CONSTASCII_STRING( PROPERTY_CONTROL_BORDER_COLOR_MOUSE, "ControlBorderColorOnHover" );

    //--------------------------------------------------------------------------
    //- URLs
    //--------------------------------------------------------------------------
    FORMS_CONSTASCII_STRING( URL_CONTROLLER_PREFIX, ".uno:FormController/" );

    FORMS_CONSTASCII_STRING( URL_FORM_POSITION,     ".uno:FormController/positionForm"  );
    FORMS_CONSTASCII_STRING( URL_FORM_RECORDCOUNT,  ".uno:FormController/RecordCount"  );
    FORMS_CONSTASCII_STRING( URL_RECORD_FIRST,      ".uno:FormController/moveToFirst"   );
    FORMS_CONSTASCII_STRING( URL_RECORD_PREV,       ".uno:FormController/moveToPrev"    );
    FORMS_CONSTASCII_STRING( URL_RECORD_NEXT,       ".uno:FormController/moveToNext"    );
    FORMS_CONSTASCII_STRING( URL_RECORD_LAST,       ".uno:FormController/moveToLast"    );
    FORMS_CONSTASCII_STRING( URL_RECORD_SAVE,       ".uno:FormController/saveRecord"    );
    FORMS_CONSTASCII_STRING( URL_RECORD_UNDO,       ".uno:FormController/undoRecord"    );
    FORMS_CONSTASCII_STRING( URL_RECORD_NEW,        ".uno:FormController/moveToNew"     );
    FORMS_CONSTASCII_STRING( URL_RECORD_DELETE,     ".uno:FormController/deleteRecord"  );
    FORMS_CONSTASCII_STRING( URL_FORM_REFRESH,      ".uno:FormController/refreshForm"   );

    FORMS_CONSTASCII_STRING( URL_FORM_SORT_UP,      ".uno:FormController/sortUp"   );
    FORMS_CONSTASCII_STRING( URL_FORM_SORT_DOWN,    ".uno:FormController/sortDown"   );
    FORMS_CONSTASCII_STRING( URL_FORM_SORT,         ".uno:FormController/sort"   );
    FORMS_CONSTASCII_STRING( URL_FORM_AUTO_FILTER,  ".uno:FormController/autoFilter"   );
    FORMS_CONSTASCII_STRING( URL_FORM_FILTER,       ".uno:FormController/filter"   );
    FORMS_CONSTASCII_STRING( URL_FORM_APPLY_FILTER, ".uno:FormController/applyFilter"   );
    FORMS_CONSTASCII_STRING( URL_FORM_REMOVE_FILTER,".uno:FormController/removeFilterOrder"   );

//..............................................................................
}   // namespace frm
//..............................................................................


#endif // FRM_STRINGS_HXX
