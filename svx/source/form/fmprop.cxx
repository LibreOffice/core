/*************************************************************************
 *
 *  $RCSfile: fmprop.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:02:30 $
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
    return reinterpret_cast<const FmPropertyInfo*>(pFirst)->aName.compareTo(reinterpret_cast<const FmPropertyInfo*>(pSecond)->aName);
}

namespace svxform
{

    // property names
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

    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_TEXTLINECOLOR, "TextLineColor");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONTEMPHASISMARK, "FontEmphasisMark");
    IMPLEMENT_CONSTASCII_USTRING(FM_PROP_FONTRELIEF, "FontRelief");

}   // namespace svxform

using namespace svxform;


// Zuordnung von Namen auf Properties Id's, das koennten auch WhichId's fuer Itemset sein
//------------------------------------------------------------------
const FmPropertyInfo* FmPropertyInfoService::getPropertyInfo()
{
    // dieser umstaendliche Weg wird gewaehlt um die Reihenfolge der Initialisierung
    // abzusichern

    sal_uInt16 nPos=1;
    if (pPropertyInfos == NULL)
    {
        // the following does not contain all know properties
        // ever property which is _not_ multi-property-able (means cannot be set on multiple controls at once)
        // and which does not have an UI name is omitted by definition
        static FmPropertyInfo __READONLY_DATA aPropertyInfos[] =
        {
            FmPropertyInfo(FM_PROP_NAME,                sal_False, RID_STR_NAME),
            FmPropertyInfo(FM_PROP_LABEL,               sal_True,  RID_STR_LABEL),
            FmPropertyInfo(FM_PROP_CONTROLLABEL,        sal_False, RID_STR_LABELCONTROL),
            FmPropertyInfo(FM_PROP_TEXT,                sal_True,  0),
            FmPropertyInfo(FM_PROP_MAXTEXTLEN,          sal_True,  RID_STR_MAXTEXTLEN),
            FmPropertyInfo(FM_PROP_EDITMASK,            sal_True,  RID_STR_EDITMASK),
            FmPropertyInfo(FM_PROP_LITERALMASK,         sal_True,  RID_STR_LITERALMASK),
            FmPropertyInfo(FM_PROP_STRICTFORMAT,        sal_True,  RID_STR_STRICTFORMAT),
            FmPropertyInfo(FM_PROP_ENABLED,             sal_True,  RID_STR_ENABLED),
            FmPropertyInfo(FM_PROP_READONLY,            sal_True,  RID_STR_READONLY),
            FmPropertyInfo(FM_PROP_PRINTABLE,           sal_True,  RID_STR_PRINTABLE),
            FmPropertyInfo(FM_PROP_CONTROLSOURCE,       sal_True,  RID_STR_CONTROLSOURCE),
            FmPropertyInfo(FM_PROP_TABSTOP,             sal_True,  RID_STR_TABSTOP),
            FmPropertyInfo(FM_PROP_TABINDEX,            sal_True,  RID_STR_TABINDEX),
            FmPropertyInfo(FM_PROP_DATASOURCE,          sal_True,  RID_STR_DATASOURCE),
            FmPropertyInfo(FM_PROP_COMMAND,             sal_False, RID_STR_CURSORSOURCE),
            FmPropertyInfo(FM_PROP_COMMANDTYPE,         sal_False, RID_STR_CURSORSOURCETYPE),
            FmPropertyInfo(FM_PROP_ESCAPE_PROCESSING,   sal_False, RID_STR_ESCAPE_PROCESSING),
            FmPropertyInfo(FM_PROP_FILTER_CRITERIA,     sal_False, RID_STR_FILTER_CRITERIA),
            FmPropertyInfo(FM_PROP_SORT,                sal_False, RID_STR_SORT_CRITERIA),
            FmPropertyInfo(FM_PROP_INSERTONLY,          sal_False, RID_STR_DATAENTRY),
            FmPropertyInfo(FM_PROP_ALLOWADDITIONS,      sal_True,  RID_STR_ALLOW_ADDITIONS),
            FmPropertyInfo(FM_PROP_ALLOWEDITS,          sal_True,  RID_STR_ALLOW_EDITS),
            FmPropertyInfo(FM_PROP_ALLOWDELETIONS,      sal_True,  RID_STR_ALLOW_DELETIONS),
            FmPropertyInfo(FM_PROP_INSERTONLY,          sal_True,  RID_STR_DATAENTRY),
            FmPropertyInfo(FM_PROP_NAVIGATION,          sal_True,  RID_STR_NAVIGATIONBAR),
            FmPropertyInfo(FM_PROP_CYCLE,               sal_True,  RID_STR_CYCLE),
            FmPropertyInfo(FM_PROP_CACHESIZE,           sal_True,  0),
            FmPropertyInfo(FM_PROP_LASTDIRTY,           sal_True,  0),
            FmPropertyInfo(FM_PROP_HIDDEN_VALUE,        sal_True,  RID_STR_VALUE),
            FmPropertyInfo(FM_PROP_VALUE,               sal_True,  0),
            FmPropertyInfo(FM_PROP_VALUEMIN,            sal_True,  RID_STR_VALUEMIN),
            FmPropertyInfo(FM_PROP_VALUEMAX,            sal_True,  RID_STR_VALUEMAX),
            FmPropertyInfo(FM_PROP_VALUESTEP,           sal_True,  RID_STR_VALUESTEP),
            FmPropertyInfo(FM_PROP_DEFAULT_VALUE,       sal_True,  RID_STR_DEFAULTVALUE),
            FmPropertyInfo(FM_PROP_DECIMAL_ACCURACY,    sal_True,  RID_STR_DECIMAL_ACCURACY),
            FmPropertyInfo(FM_PROP_DECIMALS,            sal_True,   0),
            FmPropertyInfo(FM_PROP_SHOWTHOUSANDSEP,     sal_True,  RID_STR_SHOWTHOUSANDSEP),
            FmPropertyInfo(FM_PROP_REFVALUE,            sal_True,  RID_STR_REFVALUE),
            FmPropertyInfo(FM_PROP_CURRENCYSYMBOL,      sal_True,  RID_STR_CURRENCYSYMBOL),
            FmPropertyInfo(FM_PROP_CURRSYM_POSITION,    sal_True,  RID_STR_CURRSYM_POSITION),
            FmPropertyInfo(FM_PROP_DATE,                sal_True,  0),
            FmPropertyInfo(FM_PROP_DATEMIN,             sal_True,  RID_STR_DATEMIN),
            FmPropertyInfo(FM_PROP_DATEMAX,             sal_True,  RID_STR_DATEMAX),
            FmPropertyInfo(FM_PROP_DATEFORMAT,          sal_True,  RID_STR_DATEFORMAT),
            FmPropertyInfo(FM_PROP_DEFAULT_DATE,        sal_True,  RID_STR_DEFAULTVALUE),
            FmPropertyInfo(FM_PROP_TIME,                sal_True,  0),
            FmPropertyInfo(FM_PROP_TIMEMIN,             sal_True,  RID_STR_TIMEMIN),
            FmPropertyInfo(FM_PROP_TIMEMAX,             sal_True,  RID_STR_TIMEMAX),
            FmPropertyInfo(FM_PROP_TIMEFORMAT,          sal_True,  RID_STR_TIMEFORMAT),
            FmPropertyInfo(FM_PROP_DEFAULT_TIME,        sal_True,  RID_STR_DEFAULTVALUE),
            FmPropertyInfo(FM_PROP_EFFECTIVE_MIN,       sal_False, RID_STR_VALUEMIN),
            FmPropertyInfo(FM_PROP_EFFECTIVE_MAX,       sal_False, RID_STR_VALUEMAX),
            FmPropertyInfo(FM_PROP_EFFECTIVE_DEFAULT,   sal_False, RID_STR_DEFAULTVALUE),
            FmPropertyInfo(FM_PROP_FORMATKEY,           sal_True,  RID_STR_FORMATKEY),
            FmPropertyInfo(FM_PROP_FORMATSSUPPLIER,     sal_True,  0),
            FmPropertyInfo(FM_PROP_SIZE,                sal_True,  RID_STR_SIZE),
            FmPropertyInfo(FM_PROP_CLASSID,             sal_False, RID_STR_CLASSID),
            FmPropertyInfo(FM_PROP_WIDTH,               sal_True,  RID_STR_WIDTH),
            FmPropertyInfo(FM_PROP_DEFAULTCONTROL,      sal_True,  0),
            FmPropertyInfo(FM_PROP_BOUNDCOLUMN,         sal_True,  RID_STR_BOUNDCOLUMN),
            FmPropertyInfo(FM_PROP_LISTSOURCETYPE,      sal_True,  RID_STR_LISTSOURCETYPE),
            FmPropertyInfo(FM_PROP_LISTSOURCE,          sal_True,  RID_STR_LISTSOURCE),
            FmPropertyInfo(FM_PROP_STRINGITEMLIST,      sal_True,  RID_STR_STRINGITEMLIST),
            FmPropertyInfo(FM_PROP_DEFAULT_TEXT,        sal_True,  RID_STR_DEFAULTVALUE),
            FmPropertyInfo(FM_PROP_FONT_NAME,           sal_True,  RID_STR_FONT),
            FmPropertyInfo(FM_PROP_FONT_STYLENAME,      sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_FAMILY,         sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_CHARSET,        sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_HEIGHT,         sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_WEIGHT,         sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_SLANT,          sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_UNDERLINE,      sal_True,  0),
            FmPropertyInfo(FM_PROP_FONT_STRIKEOUT,      sal_True,  0),
            FmPropertyInfo(FM_PROP_ALIGN,               sal_True,  RID_STR_ALIGN),
            FmPropertyInfo(FM_PROP_ROWHEIGHT,           sal_True,  RID_STR_ROWHEIGHT),
            FmPropertyInfo(FM_PROP_BACKGROUNDCOLOR,     sal_True,  RID_STR_BACKGROUNDCOLOR),
            FmPropertyInfo(FM_PROP_FILLCOLOR,           sal_True,  RID_STR_FILLCOLOR),
            FmPropertyInfo(FM_PROP_TEXTCOLOR,           sal_True,  RID_STR_TEXTCOLOR),
            FmPropertyInfo(FM_PROP_LINECOLOR,           sal_True,  RID_STR_LINECOLOR),
            FmPropertyInfo(FM_PROP_BORDER,              sal_True,  RID_STR_BORDER),
            FmPropertyInfo(FM_PROP_DROPDOWN,            sal_True,  RID_STR_DROPDOWN),
            FmPropertyInfo(FM_PROP_AUTOCOMPLETE,        sal_True,  RID_STR_AUTOCOMPLETE),
            FmPropertyInfo(FM_PROP_LINECOUNT,           sal_True,  RID_STR_LINECOUNT),
            FmPropertyInfo(FM_PROP_MULTI,               sal_True,  RID_STR_MULTILINE),
            FmPropertyInfo(FM_PROP_MULTILINE,           sal_True,  RID_STR_MULTILINE),
            FmPropertyInfo(FM_PROP_MULTISELECTION,      sal_True,  RID_STR_MULTISELECTION),
            FmPropertyInfo(FM_PROP_HARDLINEBREAKS,      sal_True,  RID_STR_HARDLINEBREAKS),
            FmPropertyInfo(FM_PROP_HSCROLL,             sal_True,  RID_STR_HSCROLL),
            FmPropertyInfo(FM_PROP_VSCROLL,             sal_True,  RID_STR_VSCROLL),
            FmPropertyInfo(FM_PROP_SPIN,                sal_True,  RID_STR_SPIN),
            FmPropertyInfo(FM_PROP_STATE,               sal_True,  0),
            FmPropertyInfo(FM_PROP_BUTTONTYPE,          sal_True,  RID_STR_BUTTONTYPE),
            FmPropertyInfo(FM_PROP_TARGET_URL,          sal_True,  RID_STR_TARGET_URL),
            FmPropertyInfo(FM_PROP_TARGET_FRAME,        sal_True,  RID_STR_TARGET_FRAME),
            FmPropertyInfo(FM_PROP_SUBMIT_ACTION,       sal_True,  RID_STR_SUBMIT_ACTION),
            FmPropertyInfo(FM_PROP_SUBMIT_TARGET,       sal_True,  RID_STR_SUBMIT_TARGET),
            FmPropertyInfo(FM_PROP_SUBMIT_METHOD,       sal_True,  RID_STR_SUBMIT_METHOD),
            FmPropertyInfo(FM_PROP_SUBMIT_ENCODING,     sal_True,  RID_STR_SUBMIT_ENCODING),
            FmPropertyInfo(FM_PROP_DEFAULTCHECKED,      sal_True,  RID_STR_DEFAULT_CHECKED),
            FmPropertyInfo(::rtl::OUString::createFromAscii("DefaultButton"), sal_True,  RID_STR_DEFAULT_BUTTON),
            FmPropertyInfo(FM_PROP_VALUE_SEQ,           sal_True,  0),
            FmPropertyInfo(FM_PROP_IMAGE_URL,           sal_True,  RID_STR_IMAGE_URL),
            FmPropertyInfo(FM_PROP_SELECT_SEQ,          sal_True,  0),
            FmPropertyInfo(FM_PROP_DEFAULT_SELECT_SEQ,  sal_True,  RID_STR_DEFAULT_SELECT_SEQ),
            FmPropertyInfo(FM_PROP_ECHO_CHAR,           sal_True,  RID_STR_ECHO_CHAR),
            FmPropertyInfo(FM_PROP_EMPTY_IS_NULL,       sal_True,  RID_STR_EMPTY_IS_NULL),
            FmPropertyInfo(FM_PROP_TRISTATE,            sal_True,  RID_STR_TRISTATE),
            FmPropertyInfo(FM_PROP_MASTERFIELDS,        sal_True,  RID_STR_MASTERFIELDS),
            FmPropertyInfo(FM_PROP_DETAILFIELDS,        sal_True,  RID_STR_SLAVEFIELDS),
            FmPropertyInfo(FM_PROP_HASNAVIGATION,       sal_True,  RID_STR_NAVIGATIONBAR),
            FmPropertyInfo(FM_PROP_RECORDMARKER,        sal_True,  RID_STR_RECORDMARKER),
            FmPropertyInfo(FM_PROP_FILTERPROPOSAL,      sal_True,  RID_STR_FILTERPROPOSAL),
            FmPropertyInfo(FM_PROP_QUERY,               sal_True,  0),
            FmPropertyInfo(FM_PROP_ISPASSTHROUGH,       sal_True,  0),
            FmPropertyInfo(FM_PROP_TAG,                 sal_True,  RID_STR_TAG),
            FmPropertyInfo(FM_PROP_HELPTEXT,            sal_False, RID_STR_HELPTEXT),
            FmPropertyInfo(FM_PROP_HELPURL,             sal_False, RID_STR_HELPURL),
            FmPropertyInfo(FM_PROP_CURSORCOLOR,         sal_True, 0),
            FmPropertyInfo(FM_PROP_ALWAYSSHOWCURSOR,    sal_True, 0),
            FmPropertyInfo(FM_PROP_DISPLAYSYNCHRON,     sal_True, 0)
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
FmPropertyInfo::FmPropertyInfo(const ::rtl::OUString& _rName, sal_Bool bMSel,
                               sal_uInt16 _nDisplayNameResId)

               :aName(_rName)
               ,bMultiSelectable(bMSel)
               ,nDisplayNameResId( _nDisplayNameResId )
{
}

//------------------------------------------------------------------
::rtl::OUString FmPropertyInfoService::getPropertyTranslation( const ::rtl::OUString _rName )
{
    const FmPropertyInfo* pInfo = getPropertyInfo( _rName );
    return  ( pInfo && ( pInfo->nDisplayNameResId > 0 ) )
        ?   ::rtl::OUString( SVX_RES( pInfo->nDisplayNameResId ) )
        :   ::rtl::OUString();
}

//------------------------------------------------------------------------
sal_Bool FmPropertyInfoService::getPropertyMultiFlag( const ::rtl::OUString _rName )
{
    const FmPropertyInfo* pInfo = getPropertyInfo( _rName );
    return (pInfo) ? pInfo->bMultiSelectable : sal_False;
}

//------------------------------------------------------------------------
const FmPropertyInfo* FmPropertyInfoService::getPropertyInfo(const ::rtl::OUString& _rName)
{
    // intialisierung
    if(!pPropertyInfos)
        getPropertyInfo();
    FmPropertyInfo  aSearch( _rName, sal_False, 0 );

    const FmPropertyInfo* pPropInfo = (FmPropertyInfo*) bsearch(&aSearch,
                                    (void*) pPropertyInfos,
                                     nCount,
                                     sizeof(FmPropertyInfo),
                                     &PropertyInfoCompare);

    return pPropInfo;
}


