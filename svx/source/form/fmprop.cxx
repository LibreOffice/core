/*************************************************************************
 *
 *  $RCSfile: fmprop.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:50:47 $
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
#pragma hdrstop

#define USE_PROPERTY_HELPER

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif

#include <set>

namespace svxform
{
    static ::std::set< ::rtl::OUString >& getComposableProperties()
    {
        static ::std::set< ::rtl::OUString > aComposableProperties;
        if ( aComposableProperties.empty() )
        {
            aComposableProperties.insert( FM_PROP_NAME );
            aComposableProperties.insert( FM_PROP_LABEL );
            aComposableProperties.insert( FM_PROP_CONTROLLABEL );
            aComposableProperties.insert( FM_PROP_TEXT );
            aComposableProperties.insert( FM_PROP_MAXTEXTLEN );
            aComposableProperties.insert( FM_PROP_EDITMASK );
            aComposableProperties.insert( FM_PROP_LITERALMASK );
            aComposableProperties.insert( FM_PROP_STRICTFORMAT );
            aComposableProperties.insert( FM_PROP_ENABLED );
            aComposableProperties.insert( FM_PROP_READONLY );
            aComposableProperties.insert( FM_PROP_CONTROLSOURCE );
            aComposableProperties.insert( FM_PROP_DATASOURCE );
            aComposableProperties.insert( FM_PROP_COMMAND );
            aComposableProperties.insert( FM_PROP_COMMANDTYPE );
            aComposableProperties.insert( FM_PROP_ESCAPE_PROCESSING );
            aComposableProperties.insert( FM_PROP_FILTER_CRITERIA );
            aComposableProperties.insert( FM_PROP_SORT );
            aComposableProperties.insert( FM_PROP_INSERTONLY );
            aComposableProperties.insert( FM_PROP_INSERTONLY );
            aComposableProperties.insert( FM_PROP_NAVIGATION );
            aComposableProperties.insert( FM_PROP_CYCLE );
            aComposableProperties.insert( FM_PROP_VALUE );
            aComposableProperties.insert( FM_PROP_VALUEMIN );
            aComposableProperties.insert( FM_PROP_VALUEMAX );
            aComposableProperties.insert( FM_PROP_VALUESTEP );
            aComposableProperties.insert( FM_PROP_DEFAULT_VALUE );
            aComposableProperties.insert( FM_PROP_DECIMAL_ACCURACY );
            aComposableProperties.insert( FM_PROP_SHOWTHOUSANDSEP );
            aComposableProperties.insert( FM_PROP_REFVALUE );
            aComposableProperties.insert( FM_PROP_CURRENCYSYMBOL );
            aComposableProperties.insert( FM_PROP_DATE );
            aComposableProperties.insert( FM_PROP_DATEMIN );
            aComposableProperties.insert( FM_PROP_DATEMAX );
            aComposableProperties.insert( FM_PROP_DATEFORMAT );
            aComposableProperties.insert( FM_PROP_DEFAULT_DATE );
            aComposableProperties.insert( FM_PROP_TIME );
            aComposableProperties.insert( FM_PROP_TIMEMIN );
            aComposableProperties.insert( FM_PROP_TIMEMAX );
            aComposableProperties.insert( FM_PROP_TIMEFORMAT );
            aComposableProperties.insert( FM_PROP_DEFAULT_TIME );
            aComposableProperties.insert( FM_PROP_EFFECTIVE_MIN );
            aComposableProperties.insert( FM_PROP_EFFECTIVE_MAX );
            aComposableProperties.insert( FM_PROP_EFFECTIVE_DEFAULT );
            aComposableProperties.insert( FM_PROP_FORMATKEY );
            aComposableProperties.insert( FM_PROP_FORMATSSUPPLIER );
            aComposableProperties.insert( FM_PROP_CLASSID );
            aComposableProperties.insert( FM_PROP_WIDTH );
            aComposableProperties.insert( FM_PROP_DEFAULTCONTROL );
            aComposableProperties.insert( FM_PROP_BOUNDCOLUMN );
            aComposableProperties.insert( FM_PROP_LISTSOURCETYPE );
            aComposableProperties.insert( FM_PROP_LISTSOURCE );
            aComposableProperties.insert( FM_PROP_STRINGITEMLIST );
            aComposableProperties.insert( FM_PROP_DEFAULT_TEXT );
            aComposableProperties.insert( FM_PROP_ALIGN );
            aComposableProperties.insert( FM_PROP_ROWHEIGHT );
            aComposableProperties.insert( FM_PROP_BACKGROUNDCOLOR );
            aComposableProperties.insert( FM_PROP_TEXTCOLOR );
            aComposableProperties.insert( FM_PROP_BORDER );
            aComposableProperties.insert( FM_PROP_DROPDOWN );
            aComposableProperties.insert( FM_PROP_LINECOUNT );
            aComposableProperties.insert( FM_PROP_MULTILINE );
            aComposableProperties.insert( FM_PROP_SPIN );
            aComposableProperties.insert( FM_PROP_STATE );
            aComposableProperties.insert( FM_PROP_TARGET_URL );
            aComposableProperties.insert( FM_PROP_DEFAULTCHECKED );
            aComposableProperties.insert( ::rtl::OUString::createFromAscii("DefaultButton") );
            aComposableProperties.insert( FM_PROP_VALUE_SEQ );
            aComposableProperties.insert( FM_PROP_SELECT_SEQ );
            aComposableProperties.insert( FM_PROP_DEFAULT_SELECT_SEQ );
            aComposableProperties.insert( FM_PROP_TRISTATE );
            aComposableProperties.insert( FM_PROP_HASNAVIGATION );
            aComposableProperties.insert( FM_PROP_RECORDMARKER );
            aComposableProperties.insert( FM_PROP_FILTERPROPOSAL );
            aComposableProperties.insert( FM_PROP_HELPTEXT );
            aComposableProperties.insert( FM_PROP_HELPURL );
            aComposableProperties.insert( FM_PROP_CURSORCOLOR );
            aComposableProperties.insert( FM_PROP_ALWAYSSHOWCURSOR );
            aComposableProperties.insert( FM_PROP_DISPLAYSYNCHRON );
            aComposableProperties.insert( FM_PROP_SCROLLVALUE_MIN );
            aComposableProperties.insert( FM_PROP_SCROLLVALUE_MAX );
            aComposableProperties.insert( FM_PROP_DEFAULT_SCROLLVALUE );
            aComposableProperties.insert( FM_PROP_LINEINCREMENT );
            aComposableProperties.insert( FM_PROP_BLOCKINCREMENT );
            aComposableProperties.insert( FM_PROP_ORIENTATION );
            aComposableProperties.insert( FM_PROP_REPEAT );
            aComposableProperties.insert( FM_PROP_REPEATDELAY );
            aComposableProperties.insert( FM_PROP_SYMBOLCOLOR );
            aComposableProperties.insert( FM_PROP_VISIBILESIZE );
            aComposableProperties.insert( FM_PROP_SPINVALUE_MIN );
            aComposableProperties.insert( FM_PROP_SPINVALUE_MAX );
            aComposableProperties.insert( FM_PROP_DEFAULT_SPINVALUE );
            aComposableProperties.insert( FM_PROP_SPININCREMENT );
            aComposableProperties.insert( FM_PROP_LINEENDFORMAT );
            aComposableProperties.insert( FM_PROP_TOGGLE );
            aComposableProperties.insert( FM_PROP_FOCUSONCLICK );
            aComposableProperties.insert( FM_PROP_HIDEINACTIVESELECTION );
            aComposableProperties.insert( FM_PROP_VISUALEFFECT );
            aComposableProperties.insert( FM_PROP_BORDERCOLOR );
            aComposableProperties.insert( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImagePosition" ) ) );
        }
        return aComposableProperties;
    }
}

namespace svxform
{
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_NAME,"Name");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CLASSID,"ClassId");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ROWCOUNT,"RowCount");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ROWCOUNTFINAL,"IsRowCountFinal");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FETCHSIZE,"FetchSize");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUE,"Value");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TEXT,"Text");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LABEL,"Label");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CANINSERT,"CanInsert");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CANUPDATE,"CanUpdate");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CANDELETE,"CanDelete");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_NAVIGATION,"NavigationBarMode");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HASNAVIGATION,"HasNavigationBar");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CYCLE,"Cycle");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CONTROLSOURCE,"DataField");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ENABLED,"Enabled");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_READONLY,"ReadOnly");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISREADONLY,"IsReadOnly");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FILTER_CRITERIA,"Filter");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_AUTOINCREMENT,"IsAutoIncrement");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATAENTRY,"DataEntry");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_WIDTH,"Width");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SEARCHABLE,"IsSearchable");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MULTILINE,"MultiLine");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TARGET_URL,"TargetURL");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULTCONTROL,"DefaultControl");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MAXTEXTLEN,"MaxTextLen");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EDITMASK,"EditMask");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SPIN,"Spin");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATE,"Date");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TIME,"Time");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STATE,"State");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TRISTATE,"TriState");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STRINGITEMLIST,"StringItemList");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_TEXT,"DefaultText");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULTCHECKED,"DefaultState");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FORMATKEY,"FormatKey");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FORMATSSUPPLIER,"FormatsSupplier");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LISTSOURCETYPE,"ListSourceType");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LISTSOURCE,"ListSource");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SELECT_SEQ,"SelectedItems");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUE_SEQ,"ValueItemList");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_SELECT_SEQ,"DefaultSelection");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALIGN,"Align");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_DATE,"DefaultDate");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_TIME,"DefaultTime");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_VALUE,"DefaultValue");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DECIMAL_ACCURACY,"DecimalAccuracy");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORSOURCE,"DataSelection");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORSOURCETYPE,"DataSelectionType");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FIELDTYPE,"Type");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_REFVALUE,"RefValue");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUEMIN,"ValueMin");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUEMAX,"ValueMax");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STRICTFORMAT,"StrictFormat");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LITERALMASK,"LiteralMask");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUESTEP,"ValueStep");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SHOWTHOUSANDSEP,"ShowThousandsSeparator");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURRENCYSYMBOL,"CurrencySymbol");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATEFORMAT,"DateFormat");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATEMIN,"DateMin");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATEMAX,"DateMax");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATE_SHOW_CENTURY,"DateShowCentury");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TIMEFORMAT, "TimeFormat");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TIMEMIN, "TimeMin");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TIMEMAX,"TimeMax");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LINECOUNT,"LineCount");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BOUNDCOLUMN, "BoundColumn");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT,"FontDescriptor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BACKGROUNDCOLOR,"BackgroundColor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TEXTCOLOR,"TextColor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BORDER,"Border");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DROPDOWN,"Dropdown");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ROWHEIGHT,"RowHeight");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HELPTEXT, "HelpText");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HELPURL, "HelpURL");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RECORDMARKER, "HasRecordMarker");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BOUNDFIELD, "BoundField");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EFFECTIVE_VALUE, "EffectiveValue");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EFFECTIVE_DEFAULT, "EffectiveDefault");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EFFECTIVE_MIN, "EffectiveMin");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EFFECTIVE_MAX, "EffectiveMax");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HIDDEN, "Hidden");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FILTERPROPOSAL, "UseFilterValueProposal");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FIELDSOURCE, "FieldSource");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TABLENAME, "TableName");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FILTERSUPPLIER, "FilterSupplier");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURRENTFILTER, "CurrentFilter");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SELECTED_FIELDS, "SelectedFields");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SELECTED_TABLES, "SelectedTables");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_THREADSAFE, "ThreadSafe");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISFILTERAPPLIED, "IsFilterApplied");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CONTROLLABEL, "LabelControl");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORCOLOR, "CursorColor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALWAYSSHOWCURSOR, "AlwaysShowCursor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DISPLAYSYNCHRON, "DisplayIsSynchron");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISMODIFIED, "IsModified");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISNEW, "IsNew");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_PRIVILEGES, "Privileges");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_COMMAND,"Command");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_COMMANDTYPE,"CommandType");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RESULTSET_CONCURRENCY, "ResultSetConcurrency");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_INSERTONLY, "IgnoreResult");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RESULTSET_TYPE, "ResultSetType");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ESCAPE_PROCESSING, "EscapeProcessing");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_APPLYFILTER, "ApplyFilter");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISNULLABLE, "IsNullable");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ACTIVECOMMAND, "ActiveCommand");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISCURRENCY, "IsCurrency");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_URL, "URL");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ACTIVE_CONNECTION, "ActiveConnection");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SORT, "Order");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATASOURCE, "DataSourceName");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_REALNAME,"RealName");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CONTROLSOURCEPROPERTY,"DataFieldProperty");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TEXTLINECOLOR, "TextLineColor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONTEMPHASISMARK, "FontEmphasisMark");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONTRELIEF, "FontRelief");

    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SCROLLVALUE_MIN,             "ScrollValueMin"            );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SCROLLVALUE_MAX,             "ScrollValueMax"            );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_DEFAULT_SCROLLVALUE,         "DefaultScrollValue"        );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_LINEINCREMENT,               "LineIncrement"             );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_BLOCKINCREMENT,              "BlockIncrement"            );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_ORIENTATION,                 "Orientation"               );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_REPEAT,                      "Repeat"                    );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_REPEATDELAY,                 "RepeatDelay"               );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SYMBOLCOLOR,                 "SymbolColor"               );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_VISIBILESIZE,                "VisibleSize"               );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SPINVALUE_MIN,               "SpinValueMin"              );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SPINVALUE_MAX,               "SpinValueMax"              );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_DEFAULT_SPINVALUE,           "DefaultSpinValue"          );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_SPININCREMENT,               "SpinIncrement"             );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_LINEENDFORMAT,               "LineEndFormat"             );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_TOGGLE,                      "Toggle"                    );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_FOCUSONCLICK,                "FocusOnClick"              );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_HIDEINACTIVESELECTION,       "HideInactiveSelection"     );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_VISUALEFFECT,                "VisualEffect"              );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_BORDERCOLOR,                 "BorderColor"               );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_DYNAMIC_CONTROL_BORDER,      "DynamicControlBorder"      );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_CONTROL_BORDER_COLOR_FOCUS,  "ControlBorderColorOnFocus" );
    IMPLEMENT_CONSTASCII_USTRING( FM_PROP_CONTROL_BORDER_COLOR_MOUSE,  "ControlBorderColorOnHover" );

}   // namespace svxform

//------------------------------------------------------------------------
bool FmPropertyInfoService::isComposable( const ::rtl::OUString _rName )
{
    ::std::set< ::rtl::OUString >& rComposable = ::svxform::getComposableProperties();
    return ( rComposable.find( _rName ) != rComposable.end() );
}

