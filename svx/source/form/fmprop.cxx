/*************************************************************************
 *
 *  $RCSfile: fmprop.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:17 $
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

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif

#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _XCEPTION_HXX_ //autogen
#include <vos/xception.hxx>
#endif
#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif

sal_uInt16 FmPropertyInfoService::nCount = 0;
FmPropertyInfo* FmPropertyInfoService::pPropertyInfos = NULL;


// Vergleichen von Properties
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    PropertyCompare( const void* pFirst, const void* pSecond)
{
    return ((::com::sun::star::beans::Property*)pFirst)->Name.compareTo(((::com::sun::star::beans::Property*)pSecond)->Name);
}


// Vergleichen von PropertyInfo
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    PropertyInfoCompare(const void* pFirst, const void* pSecond)
{
    return reinterpret_cast<const FmPropertyInfo*>(pFirst)->aName.CompareTo(reinterpret_cast<const FmPropertyInfo*>(pSecond)->aName);
}

// PropertyNamen
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TABINDEX,"TabIndex");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TAG,"Tag");
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
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISREQUIRED,"IsRequired");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_AUTOINCREMENT,"IsAutoIncrement");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CACHESIZE,"CacheSize");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATAENTRY,"DataEntry");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LASTDIRTY,"LastUpdated");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STATEMENT,"Statement");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_WIDTH,"Width");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SEARCHABLE,"IsSearchable");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MULTILINE,"MultiLine");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TARGET_URL,"TargetURL");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TARGET_FRAME,"TargetFrame");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULTCONTROL,"DefaultControl");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MAXTEXTLEN,"MaxTextLen");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EDITMASK,"EditMask");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SIZE,"Size");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SPIN,"Spin");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATE,"Date");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TIME,"Time");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STATE,"State");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TRISTATE,"TriState");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HIDDEN_VALUE, "HiddenValue");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BUTTONTYPE,"ButtonType");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STRINGITEMLIST,"StringItemList");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_TEXT,"DefaultText");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULTCHECKED,"DefaultState");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FORMATKEY,"FormatKey");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FORMATSSUPPLIER,"FormatsSupplier");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SUBMIT_ACTION,"SubmitAction");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SUBMIT_TARGET,"SubmitTarget");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SUBMIT_METHOD,"SubmitMethod");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SUBMIT_ENCODING,"SubmitEncoding");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_IMAGE_URL,"ImageURL");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_EMPTY_IS_NULL,"ConvertEmptyToNull");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LISTSOURCETYPE,"ListSourceType");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LISTSOURCE,"ListSource");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SELECT_SEQ,"SelectedItems");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUE_SEQ,"ValueItemList");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_SELECT_SEQ,"DefaultSelection");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MULTISELECTION,"MultiSelection");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALIGN,"Align");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_DATE,"DefaultDate");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_TIME,"DefaultTime");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DEFAULT_VALUE,"DefaultValue");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DECIMAL_ACCURACY,"DecimalAccuracy");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORSOURCE,"DataSelection");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORSOURCETYPE,"DataSelectionType");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FIELDTYPE,"Type");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DECIMALS,"Decimals");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_REFVALUE,"RefValue");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUEMIN,"ValueMin");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VALUEMAX,"ValueMax");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_STRICTFORMAT,"StrictFormat");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALLOWADDITIONS,"AllowInserts");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALLOWEDITS,"AllowUpdates");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALLOWDELETIONS,"AllowDeletes");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MASTERFIELDS,"MasterFields");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISPASSTHROUGH,"IsPassThrough");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_QUERY,"Query");
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
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FILLCOLOR,"FillColor");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TEXTCOLOR,"TextColor");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_LINECOLOR,"LineColor");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BORDER,"Border");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DROPDOWN,"Dropdown");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_MULTI,"Multi");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HSCROLL,"HScroll");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_VSCROLL,"VScroll");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TABSTOP,"Tabstop");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_AUTOCOMPLETE,"Autocomplete");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HARDLINEBREAKS,"HardLineBreaks");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_PRINTABLE,"Printable");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ECHO_CHAR,"EchoChar");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ROWHEIGHT,"RowHeight");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HELPTEXT, "HelpText");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_NAME, "FontName");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_STYLENAME, "FontStyleName");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_FAMILY, "FontFamily");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_CHARSET, "FontCharset");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_HEIGHT, "FontHeight");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_WEIGHT, "FontWeight");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_SLANT, "FontSlant");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_UNDERLINE, "FontUnderline");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONT_STRIKEOUT, "FontStrikeout");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_HELPURL, "HelpURL");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RECORDMARKER, "HasRecordMarker");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_BOUNDFIELD, "BoundField");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TREATASNUMERIC, "TreatAsNumber");
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
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURRSYM_POSITION, "PrependCurrencySymbol");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SOURCE, "Source");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CURSORCOLOR, "CursorColor");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ALWAYSSHOWCURSOR, "AlwaysShowCursor");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DISPLAYSYNCHRON, "DisplayIsSynchron");

IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISMODIFIED, "IsModified");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISNEW, "IsNew");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_PRIVILEGES, "Privileges");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_COMMAND,"Command");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_COMMANDTYPE,"CommandType");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RESULTSET_CONCURRENCY, "ResultSetConcurrency");
// renamed
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_INSERTONLY, "IgnoreResult");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_RESULTSET_TYPE, "ResultSetType");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ESCAPE_PROCESSING, "EscapeProcessing");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_APPLYFILTER, "ApplyFilter");

IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISNULLABLE, "IsNullable");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ACTIVECOMMAND, "ActiveCommand");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ISCURRENCY, "IsCurrency");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_NUMBER_FORMATS_SUPPLIER, "NumberFormatsSupplier");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_URL, "URL");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TITLE, "Title");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_ACTIVE_CONNECTION, "ActiveConnection");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SCALE, "Scale");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_SORT, "Order");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DATASOURCE, "DataSourceName");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_DETAILFIELDS,"DetailFields");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_REALNAME,"RealName");
IMPLEMENT_CONSTASCII_USTRING(FM_PROP_CONTROLSOURCEPROPERTY,"DataFieldProperty");



// Zuordnung von Namen auf Properties Id's, das koennten auch WhichId's fuer Itemset sein
//------------------------------------------------------------------
const FmPropertyInfo* FmPropertyInfoService::getPropertyInfo()
{
    // dieser umstaendliche Weg wird gewaehlt um die Reihenfolge der Initialisierung
    // abzusichern

    sal_uInt16 nPos=1;
    if (pPropertyInfos == NULL)
    {
        static FmPropertyInfo __READONLY_DATA aPropertyInfos[] =
        {
            FmPropertyInfo(FM_PROP_NAME,                FM_ATTR_NAME,               sal_False, ::rtl::OUString(SVX_RES(RID_STR_NAME)),              nPos++, HID_PROP_NAME) ,
            FmPropertyInfo(FM_PROP_LABEL,               FM_ATTR_LABEL,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LABEL)),             nPos++, HID_PROP_LABEL),
            FmPropertyInfo(FM_PROP_CONTROLLABEL,        FM_ATTR_CONTROLLABEL,       sal_False, ::rtl::OUString(SVX_RES(RID_STR_LABELCONTROL)),      nPos++, HID_PROP_CONTROLLABEL),
            FmPropertyInfo(FM_PROP_TEXT,                FM_ATTR_TEXT,               sal_True,  ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_MAXTEXTLEN,          FM_ATTR_MAXTEXTLEN,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_MAXTEXTLEN)),            nPos++, HID_PROP_MAXTEXTLEN),
            FmPropertyInfo(FM_PROP_EDITMASK,            FM_ATTR_EDITMASK,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_EDITMASK)),          nPos++, HID_PROP_EDITMASK),
            FmPropertyInfo(FM_PROP_LITERALMASK,         FM_ATTR_LITERALMASK,        sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LITERALMASK)),       nPos++, HID_PROP_LITERALMASK),
            FmPropertyInfo(FM_PROP_STRICTFORMAT,        FM_ATTR_STRICTFORMAT,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_STRICTFORMAT)),      nPos++, HID_PROP_STRICTFORMAT),
            FmPropertyInfo(FM_PROP_ENABLED,             FM_ATTR_ENABLED,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ENABLED)),           nPos++, HID_PROP_ENABLED) ,
            FmPropertyInfo(FM_PROP_READONLY,            FM_ATTR_READONLY,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_READONLY)),          nPos++, HID_PROP_READONLY) ,
            FmPropertyInfo(FM_PROP_PRINTABLE,           FM_ATTR_PRINTABLE,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_PRINTABLE)),         nPos++, HID_PROP_PRINTABLE) ,
            FmPropertyInfo(::rtl::OUString::createFromAscii("GroupBox"),                    FM_ATTR_GROUPBOX,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_GROUPBOX)),          nPos++, HID_PROP_GROUPBOX),
            FmPropertyInfo(FM_PROP_CONTROLSOURCE,       FM_ATTR_CONTROLSOURCE,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_CONTROLSOURCE)),     nPos++, HID_PROP_CONTROLSOURCE),
            FmPropertyInfo(FM_PROP_TABSTOP,             FM_ATTR_TABSTOP,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TABSTOP)),           nPos++, HID_PROP_TABSTOP),
            FmPropertyInfo(FM_PROP_TABINDEX,            FM_ATTR_TABINDEX,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TABINDEX)),          nPos++, HID_PROP_TABINDEX) ,
            FmPropertyInfo(FM_PROP_DATASOURCE,          FM_ATTR_DATASOURCE,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATASOURCE)),            nPos++, HID_PROP_DATASOURCE) ,
            FmPropertyInfo(FM_PROP_COMMAND,             FM_ATTR_COMMAND,            sal_False, ::rtl::OUString(SVX_RES(RID_STR_CURSORSOURCE)),      nPos++, HID_PROP_CURSORSOURCE),
            FmPropertyInfo(FM_PROP_COMMANDTYPE,         FM_ATTR_COMMANDTYPE,        sal_False, ::rtl::OUString(SVX_RES(RID_STR_CURSORSOURCETYPE)),  nPos++, HID_PROP_CURSORSOURCETYPE),
            FmPropertyInfo(FM_PROP_ESCAPE_PROCESSING,   FM_ATTR_ESCAPE_PROCESSING,  sal_False, ::rtl::OUString(SVX_RES(RID_STR_ESCAPE_PROCESSING)), nPos++, 0),
            FmPropertyInfo(FM_PROP_FILTER_CRITERIA,     FM_ATTR_FILTER_CRITERIA,    sal_False,  ::rtl::OUString(SVX_RES(RID_STR_FILTER_CRITERIA)),  nPos++, HID_PROP_FILTER_CRITERIA ),
            FmPropertyInfo(FM_PROP_SORT,                FM_ATTR_SORT,               sal_False,  ::rtl::OUString(SVX_RES(RID_STR_SORT_CRITERIA)),            nPos++, HID_PROP_SORT_CRITERIA) ,
            FmPropertyInfo(FM_PROP_INSERTONLY,          FM_ATTR_INSERTONLY,         sal_False,  ::rtl::OUString(SVX_RES(RID_STR_DATAENTRY)),            nPos++, HID_PROP_DATAENTRY) ,
            FmPropertyInfo(FM_PROP_ALLOWADDITIONS,      FM_ATTR_ALLOWADDITIONS,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ALLOW_ADDITIONS)),   nPos++, HID_PROP_ALLOW_ADDITIONS) ,
            FmPropertyInfo(FM_PROP_ALLOWEDITS,          FM_ATTR_ALLOWEDITS,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ALLOW_EDITS)),       nPos++, HID_PROP_ALLOW_EDITS ) ,
            FmPropertyInfo(FM_PROP_ALLOWDELETIONS,      FM_ATTR_ALLOWDELETIONS,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ALLOW_DELETIONS)),   nPos++, HID_PROP_ALLOW_DELETIONS) ,
            FmPropertyInfo(FM_PROP_INSERTONLY,          FM_ATTR_INSERTONLY,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATAENTRY)),         nPos++, HID_PROP_DATAENTRY) ,
            FmPropertyInfo(FM_PROP_NAVIGATION,          FM_ATTR_NAVIGATION,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_NAVIGATION)),            nPos++, HID_PROP_NAVIGATION) ,
            FmPropertyInfo(FM_PROP_CYCLE,               FM_ATTR_CYCLE,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_CYCLE)),             nPos++, HID_PROP_CYCLE) ,
            FmPropertyInfo(FM_PROP_CACHESIZE,           FM_ATTR_CACHESIZE,          sal_True,  ::rtl::OUString(),                                   nPos++, 0) ,
            FmPropertyInfo(FM_PROP_PRIVILEGES,          FM_ATTR_PRIVILEGES,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            // TODO : UI-Anbindung fuer Privilegien
            FmPropertyInfo(FM_PROP_LASTDIRTY,           FM_ATTR_LASTDIRTY,          sal_True,  ::rtl::OUString(),                                   nPos++, 0) ,
            FmPropertyInfo(FM_PROP_HIDDEN_VALUE,        FM_ATTR_HIDDEN_VALUE,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VALUE)),             nPos++, HID_PROP_HIDDEN_VALUE),
            FmPropertyInfo(FM_PROP_VALUE,               FM_ATTR_VALUE,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VALUE)),             nPos++, HID_PROP_VALUE) ,
            FmPropertyInfo(FM_PROP_VALUEMIN,            FM_ATTR_VALUEMIN,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VALUEMIN)),          nPos++, HID_PROP_VALUEMIN) ,
            FmPropertyInfo(FM_PROP_VALUEMAX,            FM_ATTR_VALUEMAX,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VALUEMAX)),          nPos++, HID_PROP_VALUEMAX) ,
            FmPropertyInfo(FM_PROP_VALUESTEP,           FM_ATTR_VALUESTEP,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VALUESTEP)),         nPos++, HID_PROP_VALUESTEP) ,
            FmPropertyInfo(FM_PROP_DEFAULT_VALUE,       FM_ATTR_DEFAULT_VALUE,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULTVALUE)),      nPos++, HID_PROP_DEFAULT_LONG_VALUE ),
            FmPropertyInfo(FM_PROP_DECIMAL_ACCURACY,    FM_ATTR_DECIMAL_ACCURACY,   sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DECIMAL_ACCURACY)),  nPos++, HID_PROP_DECIMAL_ACCURACY ),
            FmPropertyInfo(FM_PROP_DECIMALS,            FM_ATTR_DECIMALS,           sal_True,   ::rtl::OUString(),                                  nPos++, 0),
            FmPropertyInfo(FM_PROP_SHOWTHOUSANDSEP,     FM_ATTR_SHOWTHOUSANDSEP,    sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SHOWTHOUSANDSEP)),   nPos++, HID_PROP_SHOWTHOUSANDSEP),

            FmPropertyInfo(FM_PROP_REFVALUE,            FM_ATTR_REFVALUE,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_REFVALUE)),          nPos++, HID_PROP_REFVALUE),
            FmPropertyInfo(FM_PROP_CURRENCYSYMBOL,      FM_ATTR_CURRENCYSYMBOL,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_CURRENCYSYMBOL)),        nPos++, HID_PROP_CURRENCYSYMBOL),
            FmPropertyInfo(FM_PROP_CURRSYM_POSITION,    FM_ATTR_CURRSYM_POSITION,   sal_True,  ::rtl::OUString(SVX_RES(RID_STR_CURRSYM_POSITION)),  nPos++, HID_PROP_CURRSYM_POSITION),

            FmPropertyInfo(FM_PROP_DATE,                FM_ATTR_DATE,               sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATE)),              nPos++, HID_PROP_DATE) ,
            FmPropertyInfo(FM_PROP_DATEMIN,             FM_ATTR_DATEMIN,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATEMIN)),           nPos++, HID_PROP_DATEMIN) ,
            FmPropertyInfo(FM_PROP_DATEMAX,             FM_ATTR_DATEMAX,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATEMAX)),           nPos++, HID_PROP_DATEMAX) ,
            FmPropertyInfo(FM_PROP_DATEFORMAT,          FM_ATTR_DATEFORMAT,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATEFORMAT)),            nPos++, HID_PROP_DATEFORMAT) ,
            FmPropertyInfo(FM_PROP_DATE_SHOW_CENTURY,   FM_ATTR_DATE_SHOW_CENTURY,  sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DATE_SHOW_CENTURY)), nPos++, HID_PROP_DATE_SHOW_CENTURY ),
            FmPropertyInfo(FM_PROP_DEFAULT_DATE,        FM_ATTR_DEFAULT_DATE,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULTVALUE)),      nPos++, HID_PROP_DEFAULT_DATE ),

            FmPropertyInfo(FM_PROP_TIME,                FM_ATTR_TIME,               sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TIME)),              nPos++, HID_PROP_TIME) ,
            FmPropertyInfo(FM_PROP_TIMEMIN,             FM_ATTR_TIMEMIN,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TIMEMIN)),           nPos++, HID_PROP_TIMEMIN) ,
            FmPropertyInfo(FM_PROP_TIMEMAX,             FM_ATTR_TIMEMAX,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TIMEMAX)),           nPos++, HID_PROP_TIMEMAX) ,
            FmPropertyInfo(FM_PROP_TIMEFORMAT,          FM_ATTR_TIMEFORMAT,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TIMEFORMAT)),            nPos++, HID_PROP_TIMEFORMAT) ,
            FmPropertyInfo(FM_PROP_DEFAULT_TIME,        FM_ATTR_DEFAULT_TIME,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULTVALUE)),      nPos++, HID_PROP_DEFAULT_TIME ),

            FmPropertyInfo(FM_PROP_SIZE,                FM_ATTR_SIZE,               sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SIZE)),              nPos++, HID_PROP_SIZE),
            FmPropertyInfo(FM_PROP_CLASSID,             FM_ATTR_CLASSID,            sal_False, ::rtl::OUString(SVX_RES(RID_STR_CLASSID)),           nPos++, HID_PROP_CLASSID),
            FmPropertyInfo(::rtl::OUString::createFromAscii("Height"),                  FM_ATTR_HEIGHT,             sal_True,  ::rtl::OUString(SVX_RES(RID_STR_HEIGHT)),                nPos++, HID_PROP_HEIGHT),
            FmPropertyInfo(FM_PROP_WIDTH,               FM_ATTR_WIDTH,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_WIDTH)),             nPos++, HID_PROP_WIDTH),
            FmPropertyInfo(FM_PROP_DEFAULTCONTROL,      FM_ATTR_DEFAULTCONTROL,     sal_True,  ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_BOUNDCOLUMN,         FM_ATTR_BOUNDCOLUMN,        sal_True,  ::rtl::OUString(SVX_RES(RID_STR_BOUNDCOLUMN)),       nPos++, HID_PROP_BOUNDCOLUMN),
            FmPropertyInfo(FM_PROP_LISTSOURCETYPE,      FM_ATTR_LISTSOURCETYPE,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LISTSOURCETYPE)),        nPos++, HID_PROP_LISTSOURCETYPE),
            FmPropertyInfo(FM_PROP_LISTSOURCE,          FM_ATTR_LISTSOURCE,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LISTSOURCE)),            nPos++, HID_PROP_LISTSOURCE),
            FmPropertyInfo(::rtl::OUString::createFromAscii("ListIndex"),               FM_ATTR_LISTINDEX,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LISTINDEX)),         nPos++, HID_PROP_LISTINDEX),
            FmPropertyInfo(FM_PROP_STRINGITEMLIST,      FM_ATTR_STRINGITEMLIST,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_STRINGITEMLIST)),        nPos++, HID_PROP_STRINGITEMLIST),
            FmPropertyInfo(FM_PROP_DEFAULT_TEXT,        FM_ATTR_DEFAULT_TEXT,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULTVALUE)),      nPos++, HID_PROP_DEFAULTVALUE ),
            FmPropertyInfo(FM_PROP_FONT,                FM_ATTR_FONT,               sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_FONT_NAME,           FM_ATTR_FONT_NAME,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_FONT)),              nPos++, HID_PROP_FONT),
            FmPropertyInfo(FM_PROP_FONT_STYLENAME,      FM_ATTR_FONT_STYLENAME,     sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_FAMILY,         FM_ATTR_FONT_FAMILY,        sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_CHARSET,        FM_ATTR_FONT_CHARSET,       sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_HEIGHT,         FM_ATTR_FONT_HEIGHT,        sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_WEIGHT,         FM_ATTR_FONT_WEIGHT,        sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_SLANT,          FM_ATTR_FONT_SLANT,         sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_UNDERLINE,      FM_ATTR_FONT_UNDERLINE,     sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FONT_STRIKEOUT,      FM_ATTR_FONT_STRIKEOUT,     sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_ALIGN,               FM_ATTR_ALIGN,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ALIGN)),             nPos++, HID_PROP_ALIGN),
            FmPropertyInfo(FM_PROP_ROWHEIGHT,           FM_ATTR_ROWHEIGHT,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ROWHEIGHT)),         nPos++, HID_PROP_ROWHEIGHT),
            FmPropertyInfo(FM_PROP_BACKGROUNDCOLOR,     FM_ATTR_BACKGROUNDCOLOR,    sal_True,  ::rtl::OUString(SVX_RES(RID_STR_BACKGROUNDCOLOR)),   nPos++, HID_PROP_BACKGROUNDCOLOR),
            FmPropertyInfo(FM_PROP_FILLCOLOR,           FM_ATTR_FILLCOLOR,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_FILLCOLOR)),         nPos++, HID_PROP_FILLCOLOR),
            FmPropertyInfo(FM_PROP_TEXTCOLOR,           FM_ATTR_TEXTCOLOR,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TEXTCOLOR)),         nPos++, HID_PROP_TEXTCOLOR),
            FmPropertyInfo(FM_PROP_LINECOLOR,           FM_ATTR_LINECOLOR,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LINECOLOR)),         nPos++, HID_PROP_LINECOLOR),
            FmPropertyInfo(FM_PROP_BORDER,              FM_ATTR_BORDER,             sal_True,  ::rtl::OUString(SVX_RES(RID_STR_BORDER)),                nPos++, HID_PROP_BORDER),
            FmPropertyInfo(FM_PROP_DROPDOWN,            FM_ATTR_DROPDOWN,           sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DROPDOWN)),          nPos++, HID_PROP_DROPDOWN),
            FmPropertyInfo(FM_PROP_AUTOCOMPLETE,        FM_ATTR_AUTOCOMPLETE,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_AUTOCOMPLETE)),      nPos++, HID_PROP_AUTOCOMPLETE),
            FmPropertyInfo(FM_PROP_LINECOUNT,           FM_ATTR_LINECOUNT,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_LINECOUNT)),         nPos++, HID_PROP_LINECOUNT),
            FmPropertyInfo(FM_PROP_MULTI,               FM_ATTR_MULTI,              sal_True,  ::rtl::OUString(SVX_RES(RID_STR_MULTILINE)),         nPos++, HID_PROP_MULTILINE),
            FmPropertyInfo(FM_PROP_MULTILINE,           FM_ATTR_MULTILINE,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_MULTILINE)),         nPos++, HID_PROP_MULTILINE),
            FmPropertyInfo(FM_PROP_MULTISELECTION,      FM_ATTR_MULTISELECTION,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_MULTISELECTION)),        nPos++, HID_PROP_MULTISELECTION),
            FmPropertyInfo(FM_PROP_HARDLINEBREAKS,      FM_ATTR_HARDLINEBREAKS,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_HARDLINEBREAKS)),        nPos++, HID_PROP_HARDLINEBREAKS),
            FmPropertyInfo(FM_PROP_HSCROLL,             FM_ATTR_HSCROLL,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_HSCROLL)),           nPos++, HID_PROP_HSCROLL),
            FmPropertyInfo(FM_PROP_VSCROLL,             FM_ATTR_VSCROLL,            sal_True,  ::rtl::OUString(SVX_RES(RID_STR_VSCROLL)),           nPos++, HID_PROP_VSCROLL),
            FmPropertyInfo(FM_PROP_SPIN,                FM_ATTR_SPIN,               sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SPIN)),              nPos++, HID_PROP_SPIN),
            FmPropertyInfo(FM_PROP_STATE,               FM_ATTR_STATE,              sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_BUTTONTYPE,          FM_ATTR_BUTTONTYPE,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_BUTTONTYPE)),            nPos++, HID_PROP_BUTTONTYPE),
            FmPropertyInfo(FM_PROP_TARGET_URL,          FM_ATTR_TARGET_URL,         sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TARGET_URL)),            nPos++, HID_PROP_TARGET_URL ),
            FmPropertyInfo(FM_PROP_TARGET_FRAME,        FM_ATTR_TARGET_FRAME,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TARGET_FRAME)),      nPos++, HID_PROP_TARGET_FRAME ),
            FmPropertyInfo(FM_PROP_SUBMIT_ACTION,       FM_ATTR_SUBMIT_ACTION,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SUBMIT_ACTION)),     nPos++, HID_PROP_SUBMIT_ACTION ),
            FmPropertyInfo(FM_PROP_SUBMIT_TARGET,       FM_ATTR_SUBMIT_TARGET,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SUBMIT_TARGET)),     nPos++, HID_PROP_SUBMIT_TARGET ),
            FmPropertyInfo(FM_PROP_SUBMIT_METHOD,       FM_ATTR_SUBMIT_METHOD,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SUBMIT_METHOD)),     nPos++, HID_PROP_SUBMIT_METHOD ),
            FmPropertyInfo(FM_PROP_SUBMIT_ENCODING,     FM_ATTR_SUBMIT_ENCODING,    sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SUBMIT_ENCODING)),   nPos++, HID_PROP_SUBMIT_ENCODING ),
            FmPropertyInfo(FM_PROP_DEFAULTCHECKED,      FM_ATTR_DEFAULT_CHECKED,    sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULT_CHECKED)),   nPos++, HID_PROP_DEFAULT_CHECKED ),
            FmPropertyInfo(::rtl::OUString::createFromAscii("DefaultButton"),           FM_ATTR_DEFAULT_BUTTON,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULT_BUTTON)),        nPos++, HID_PROP_DEFAULT_BUTTON ),
            FmPropertyInfo(FM_PROP_VALUE_SEQ,           FM_ATTR_VALUE_SEQ,          sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_IMAGE_URL,           FM_ATTR_IMAGE_URL,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_IMAGE_URL)),         nPos++, HID_PROP_IMAGE_URL ),
            FmPropertyInfo(FM_PROP_SELECT_SEQ,          FM_ATTR_SELECT_SEQ,         sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_DEFAULT_SELECT_SEQ,  FM_ATTR_DEFAULT_SELECT_SEQ, sal_True,  ::rtl::OUString(SVX_RES(RID_STR_DEFAULT_SELECT_SEQ)),    nPos++, HID_PROP_DEFAULT_SELECT_SEQ ),
            FmPropertyInfo(FM_PROP_ECHO_CHAR,           FM_ATTR_ECHO_CHAR,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_ECHO_CHAR)),         nPos++, HID_PROP_ECHO_CHAR ),
            FmPropertyInfo(FM_PROP_EMPTY_IS_NULL,       FM_ATTR_EMPTY_IS_NULL,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_EMPTY_IS_NULL)),     nPos++, HID_PROP_EMPTY_IS_NULL ),
            FmPropertyInfo(FM_PROP_TRISTATE,            FM_ATTR_TRISTATE    ,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TRISTATE)),          nPos++, HID_PROP_TRISTATE ),
            FmPropertyInfo(FM_PROP_MASTERFIELDS,        FM_ATTR_MASTERFIELDS,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_MASTERFIELDS)),      nPos++, HID_PROP_MASTERFIELDS) ,
            FmPropertyInfo(FM_PROP_DETAILFIELDS,        FM_ATTR_DETAILFIELDS,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_SLAVEFIELDS)),       nPos++, HID_PROP_SLAVEFIELDS),
            FmPropertyInfo(FM_PROP_HASNAVIGATION,       FM_ATTR_HASNAVIGATION,      sal_True,  ::rtl::OUString(SVX_RES(RID_STR_NAVIGATIONBAR)),     nPos++, HID_PROP_NAVIGATIONBAR) ,
            FmPropertyInfo(FM_PROP_RECORDMARKER,        FM_ATTR_RECORDMARKER,       sal_True,  ::rtl::OUString(SVX_RES(RID_STR_RECORDMARKER)),      nPos++, HID_PROP_RECORDMARKER) ,
            FmPropertyInfo(FM_PROP_FILTERPROPOSAL,      FM_ATTR_FILTERPROPOSAL,     sal_True,  ::rtl::OUString(SVX_RES(RID_STR_FILTERPROPOSAL)),        nPos++, HID_PROP_FILTERPROPOSAL) ,
            FmPropertyInfo(FM_PROP_QUERY,               FM_ATTR_QUERY,              sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_ISPASSTHROUGH,       FM_ATTR_ISPASSTHROUGH,      sal_True,  ::rtl::OUString(),                                   nPos++, 0 ),
            FmPropertyInfo(FM_PROP_FORMATKEY,           FM_ATTR_FORMATKEY,          sal_True,  ::rtl::OUString(SVX_RES(RID_STR_FORMATKEY)),         nPos++, HID_PROP_FORMATKEY),
            FmPropertyInfo(FM_PROP_FORMATSSUPPLIER,     FM_ATTR_FORMATSSUPPLIER,    sal_True,  ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_TREATASNUMERIC,      FM_ATTR_TREATASNUMERIC,     sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_EFFECTIVE_VALUE,     FM_ATTR_EFFECTIVE_VALUE,    sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_EFFECTIVE_MIN,       FM_ATTR_EFFECTIVE_MIN,      sal_False, ::rtl::OUString(SVX_RES(RID_STR_VALUEMIN)),          nPos++, HID_PROP_EFFECTIVEMIN),
            FmPropertyInfo(FM_PROP_EFFECTIVE_MAX,       FM_ATTR_EFFECTIVE_MAX,      sal_False, ::rtl::OUString(SVX_RES(RID_STR_VALUEMAX)),          nPos++, HID_PROP_EFFECTIVEMAX),
            FmPropertyInfo(FM_PROP_EFFECTIVE_DEFAULT,   FM_ATTR_EFFECTIVE_DEFAULT,  sal_False, ::rtl::OUString(SVX_RES(RID_STR_DEFAULTVALUE)),      nPos++, HID_PROP_EFFECTIVEDEFAULT),
            FmPropertyInfo(FM_PROP_TAG,                 FM_ATTR_TAG,                sal_True,  ::rtl::OUString(SVX_RES(RID_STR_TAG)),               nPos++, HID_PROP_TAG ),
            FmPropertyInfo(FM_PROP_HELPTEXT,            FM_ATTR_HELPTEXT,           sal_False, ::rtl::OUString(SVX_RES(RID_STR_HELPTEXT)),          nPos++, HID_PROP_HELPTEXT),
            FmPropertyInfo(FM_PROP_HELPURL,             FM_ATTR_HELPURL,            sal_False, ::rtl::OUString(SVX_RES(RID_STR_HELPURL)),           nPos++, HID_PROP_HELPURL),
            FmPropertyInfo(FM_PROP_BOUNDFIELD,          FM_ATTR_BOUNDFIELD,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_HIDDEN,              FM_ATTR_HIDDEN,             sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_FIELDSOURCE,         FM_ATTR_FIELDSOURCE,        sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_FILTERSUPPLIER,      FM_ATTR_FILTERSUPPLIER,     sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_CURRENTFILTER,       FM_ATTR_CURRENTFILTER,      sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_SELECTED_FIELDS,     FM_ATTR_SELECTED_FIELDS,    sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_SELECTED_TABLES,     FM_ATTR_SELECTED_TABLES,    sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_TABLENAME,           FM_ATTR_TABLENAME,          sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_THREADSAFE,          FM_ATTR_THREADSAFE,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_APPLYFILTER,         FM_ATTR_APPLYFILTER,        sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_SOURCE,              FM_ATTR_SOURCE,             sal_False, ::rtl::OUString(),                                   nPos++, 0),

            FmPropertyInfo(FM_PROP_CURSORCOLOR,         FM_ATTR_CURSORCOLOR,        sal_True, ::rtl::OUString(),                                    nPos++, 0),
            FmPropertyInfo(FM_PROP_ALWAYSSHOWCURSOR,    FM_ATTR_ALWAYSSHOWCURSOR,   sal_True, ::rtl::OUString(),                                    nPos++, 0),
            FmPropertyInfo(FM_PROP_DISPLAYSYNCHRON,     FM_ATTR_DISPLAYSYNCHRON,    sal_True, ::rtl::OUString(),                                    nPos++, 0),

            FmPropertyInfo(FM_PROP_ISMODIFIED,          FM_ATTR_ISMODIFIED,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_ISNEW,               FM_ATTR_ISNEW,              sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_RESULTSET_CONCURRENCY,   FM_ATTR_RESULTSET_CONCURRENCY,  sal_False, ::rtl::OUString(),                           nPos++, 0),
            FmPropertyInfo(FM_PROP_RESULTSET_TYPE,      FM_ATTR_RESULTSET_TYPE,     sal_False, ::rtl::OUString(),                                   nPos++, 0),

            FmPropertyInfo(FM_PROP_ISNULLABLE,          FM_ATTR_ISNULLABLE,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_ACTIVECOMMAND,       FM_ATTR_ACTIVECOMMAND,      sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_ISCURRENCY,          FM_ATTR_ISCURRENCY,         sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_NUMBER_FORMATS_SUPPLIER, FM_ATTR_NUMBER_FORMATS_SUPPLIER,    sal_False, ::rtl::OUString(),                       nPos++, 0),
            FmPropertyInfo(FM_PROP_URL,                 FM_ATTR_URL,                sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_TITLE,               FM_ATTR_TITLE,              sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_ACTIVE_CONNECTION,   FM_ATTR_ACTIVE_CONNECTION,  sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_SCALE,               FM_ATTR_SCALE,              sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_REALNAME,            FM_ATTR_REALNAME,           sal_False, ::rtl::OUString(),                                   nPos++, 0),
            FmPropertyInfo(FM_PROP_CONTROLSOURCEPROPERTY,FM_ATTR_CONTROLSOURCEPROPERTY, sal_False, ::rtl::OUString(),                               nPos++, 0)

        };

        pPropertyInfos = (FmPropertyInfo*)aPropertyInfos;
        nCount = sizeof(aPropertyInfos) / sizeof(FmPropertyInfo);

        // nochmal sortieren, sicher ist sicher
        qsort((void*) aPropertyInfos,
             nCount,
             sizeof(FmPropertyInfo),
             &PropertyInfoCompare);

    }
    return pPropertyInfos;
}

//------------------------------------------------------------------
FmPropertyInfo::FmPropertyInfo(const UniString& _rName, sal_Int32 _nId, sal_Bool bMSel,
                               const UniString& aString, sal_uInt16 nP, sal_uInt32 nHid)

               :aName(_rName),nId(_nId),bMultiSelectable(bMSel),
                aTranslation(aString),nPos(nP),nHelpId(nHid)
{}

//------------------------------------------------------------------
UniString FmPropertyInfoService::getPropertyTranslation(sal_Int32 _nId)
{
    const FmPropertyInfo* pInfo = getPropertyInfo(_nId);
    return (pInfo) ? pInfo->aTranslation : UniString();
}

//------------------------------------------------------------------
sal_uInt32 FmPropertyInfoService::getPropertyHelpId(sal_Int32 _nId)
{
    const FmPropertyInfo* pInfo = getPropertyInfo(_nId);
    return (pInfo) ? pInfo->nHelpId : 0;
}

//------------------------------------------------------------------
sal_Int32 FmPropertyInfoService::getPropertyId(const UniString& _rName)
{
    const FmPropertyInfo* pInfo = getPropertyInfo(_rName);
    return pInfo ? pInfo->nId : -1;
}

//------------------------------------------------------------------
sal_uInt16 FmPropertyInfoService::getPropertyPos(sal_Int32 _nId)
{
    const FmPropertyInfo* pInfo = getPropertyInfo(_nId);
    return (pInfo) ? pInfo->nPos : 0xFFFF;
}

//------------------------------------------------------------------------
sal_Bool FmPropertyInfoService::getPropertyMultiFlag(sal_Int32 _nId)
{
    const FmPropertyInfo* pInfo = getPropertyInfo(_nId);
    return (pInfo) ? pInfo->bMultiSelectable : sal_False;
}

//------------------------------------------------------------------------
const FmPropertyInfo* FmPropertyInfoService::getPropertyInfo(const UniString& _rName)
{
    // intialisierung
    if(!pPropertyInfos)
        getPropertyInfo();
    FmPropertyInfo  aSearch(_rName, 0L, sal_False,UniString(),0);

    const FmPropertyInfo* pPropInfo = (FmPropertyInfo*) bsearch(&aSearch,
                                    (void*) pPropertyInfos,
                                     nCount,
                                     sizeof(FmPropertyInfo),
                                     &PropertyInfoCompare);

    return pPropInfo;
}


//------------------------------------------------------------------------
const FmPropertyInfo* FmPropertyInfoService::getPropertyInfo(sal_Int32 _nId)
{
    // intialisierung
    if(!pPropertyInfos)
        getPropertyInfo();

    for (sal_uInt16 i = 0; i < nCount; i++)
        if (pPropertyInfos[i].nId == _nId)
            return &pPropertyInfos[i];

    return NULL;
}



