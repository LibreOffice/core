/*************************************************************************
 *
 *  $RCSfile: bibmod.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 11:41:26 $
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


#include <tools/resmgr.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XLOCALIZEDALIASES_HPP_
#include <com/sun/star/util/XLocalizedAliases.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#include "bibmod.hxx"
#include "bibresid.hxx"
#include "registry.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
static PtrBibModul pBibModul=NULL;
static nBibModulCount=0;

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

HdlBibModul OpenBibModul()
{
    if(pBibModul==NULL)
    {
        pBibModul=new BibModul();
    }
    nBibModulCount++;
    return &pBibModul;
}

void CloseBibModul(HdlBibModul ppBibModul)
{
    nBibModulCount--;
    if(nBibModulCount==0 && ppBibModul!=NULL)
    {
        delete pBibModul;
        pBibModul=NULL;
    }
}

BibResId::BibResId( sal_uInt16 nId ) :
    ResId( nId, pBibModul->GetResMgr() )
{
}
BibConfig* BibModul::pBibConfig = 0;
BibModul::BibModul()
{
    pResMgr = ResMgr::CreateResMgr( "bib" MAKE_NUMSTR(SUPD) );
    pRegistry = new BibRegistry();
    xRegistry = pRegistry;
}

BibModul::~BibModul()
{
    delete pResMgr;
    delete pBibConfig;
    xRegistry = 0;
    //delete pRegistry;
}

BibDataManager*  BibModul::createDataManager()
{
    return new BibDataManager(pRegistry);
}
//-----------------------------------------------------------------------------
BibConfig*  BibModul::GetConfig()
{
    if(!pBibConfig)
        pBibConfig = new BibConfig;
    return pBibConfig;
}


// PropertyNames
#define STATIC_USTRING(a,b) rtl::OUString a(b)
STATIC_USTRING(FM_PROP_LABEL,C2U("Label"));
STATIC_USTRING(FM_PROP_CONTROLSOURCE,C2U("DataField"));
STATIC_USTRING(FM_PROP_NAME,C2U("Name"));
STATIC_USTRING(FM_PROP_FORMATKEY,C2U("FormatKey"));
#ifdef TF_SDBAPI
#else // !TF_SDBAPI
STATIC_USTRING(FM_PROP_EDITMODE,C2U("RecordMode"));
STATIC_USTRING(FM_PROP_CURSORSOURCETYPE,C2U("DataSelectionType"));
STATIC_USTRING(FM_PROP_CURSORSOURCE,C2U("DataSelection"));
STATIC_USTRING(FM_PROP_DATASOURCE, C2U("DataSource"));
#endif // !TF_SDBAPI
STATIC_USTRING(FM_PROP_VALUE,C2U("Value"));
STATIC_USTRING(FM_PROP_TEXT,C2U("Text"));
/*

STATIC_USTRING(FM_PROP_TABINDEX,C2U("TabIndex"));
STATIC_USTRING(FM_PROP_TAG,C2U("Tag"));
STATIC_USTRING(FM_PROP_CLASSID,C2U("ClassId"));
STATIC_USTRING(FM_PROP_RECORDCOUNT,C2U("RecordCount"));
STATIC_USTRING(FM_PROP_RECORDSTATUS,C2U("RecordStatus"));
STATIC_USTRING(FM_PROP_RECORDCOUNTFINAL,C2U("IsRecordCountFinal"));
STATIC_USTRING(FM_PROP_CANINSERT,C2U("CanInsert"));
STATIC_USTRING(FM_PROP_CANUPDATE,C2U("CanUpdate"));
STATIC_USTRING(FM_PROP_CANDELETE,C2U("CanDelete"));
STATIC_USTRING(FM_PROP_NAVIGATION,C2U("NavigationBarMode"));
STATIC_USTRING(FM_PROP_HASNAVIGATION,C2U("HasNavigationBar"));
STATIC_USTRING(FM_PROP_CYCLE,C2U("Cycle"));
STATIC_USTRING(FM_PROP_ENABLED,C2U("Enabled"));
STATIC_USTRING(FM_PROP_READONLY,C2U("ReadOnly"));
STATIC_USTRING(FM_PROP_ISREADONLY,C2U("IsReadOnly"));
STATIC_USTRING(FM_PROP_FILTER_CRITERIA,C2U("Filter"));
STATIC_USTRING(FM_PROP_SORT_CRITERIA,C2U("SortOrder"));
STATIC_USTRING(FM_PROP_ISREQUIRED,C2U("IsRequired"));
STATIC_USTRING(FM_PROP_AUTOINCREMENT,C2U("IsAutoIncrement"));
STATIC_USTRING(FM_PROP_CACHESIZE,C2U("CacheSize"));
STATIC_USTRING(FM_PROP_DATAENTRY,C2U("DataEntry"));
STATIC_USTRING(FM_PROP_LASTDIRTY,C2U("LastUpdated"));
STATIC_USTRING(FM_PROP_STATEMENT,C2U("Statement"));
STATIC_USTRING(FM_PROP_WIDTH,C2U("Width"));
STATIC_USTRING(FM_PROP_SEARCHINFO,C2U("SearchInfo"));
STATIC_USTRING(FM_PROP_MULTILINE,C2U("MultiLine"));
STATIC_USTRING(FM_PROP_TARGET_URL,C2U("TargetURL"));
STATIC_USTRING(FM_PROP_TARGET_FRAME,C2U("TargetFrame"));
STATIC_USTRING(FM_PROP_DEFAULTCONTROL,C2U("DefaultControl"));
STATIC_USTRING(FM_PROP_MAXTEXTLEN,C2U("MaxTextLen"));
STATIC_USTRING(FM_PROP_EDITMASK,C2U("EditMask"));
STATIC_USTRING(FM_PROP_SIZE,C2U("Size"));
STATIC_USTRING(FM_PROP_SPIN,C2U("Spin"));
STATIC_USTRING(FM_PROP_DATE,C2U("Date"));
STATIC_USTRING(FM_PROP_TIME,C2U("Time"));
STATIC_USTRING(FM_PROP_STATE,C2U("State"));
STATIC_USTRING(FM_PROP_TRISTATE,C2U("TriState"));
STATIC_USTRING(FM_PROP_HIDDEN_VALUE, C2U("HiddenValue"));
STATIC_USTRING(FM_PROP_BUTTONTYPE,C2U("ButtonType"));
STATIC_USTRING(FM_PROP_STRINGITEMLIST,C2U("StringItemList"));
STATIC_USTRING(FM_PROP_DEFAULT_TEXT,C2U("DefaultText"));
STATIC_USTRING(FM_PROP_DEFAULTCHECKED,C2U("DefaultState"));
STATIC_USTRING(FM_PROP_FORMATSSUPPLIER,C2U("FormatsSupplier"));
STATIC_USTRING(FM_PROP_SUBMIT_ACTION,C2U("SubmitAction"));
STATIC_USTRING(FM_PROP_SUBMIT_TARGET,C2U("SubmitTarget"));
STATIC_USTRING(FM_PROP_SUBMIT_METHOD,C2U("SubmitMethod"));
STATIC_USTRING(FM_PROP_SUBMIT_ENCODING,C2U("SubmitEncoding"));
STATIC_USTRING(FM_PROP_IMAGE_URL,C2U("ImageURL"));
STATIC_USTRING(FM_PROP_EMPTY_IS_NULL,C2U("ConvertEmptyToNull"));
STATIC_USTRING(FM_PROP_LISTSOURCETYPE,C2U("ListSourceType"));
STATIC_USTRING(FM_PROP_LISTSOURCE,C2U("ListSource"));
STATIC_USTRING(FM_PROP_SELECT_SEQ,C2U("SelectedItems"));
STATIC_USTRING(FM_PROP_VALUE_SEQ,C2U("SelectedValues"));
STATIC_USTRING(FM_PROP_DEFAULT_SELECT_SEQ,C2U("DefaultSelection"));
STATIC_USTRING(FM_PROP_MULTISELECTION,C2U("MultiSelection"));
STATIC_USTRING(FM_PROP_ALIGN,C2U("Align"));
STATIC_USTRING(FM_PROP_DEFAULT_DATE,C2U("DefaultDate"));
STATIC_USTRING(FM_PROP_DEFAULT_TIME,C2U("DefaultTime"));
STATIC_USTRING(FM_PROP_DEFAULT_VALUE,C2U("DefaultValue"));
STATIC_USTRING(FM_PROP_DECIMAL_ACCURACY,C2U("DecimalAccuracy"));
STATIC_USTRING(FM_PROP_FIELDTYPE,C2U("Type"));
STATIC_USTRING(FM_PROP_DECIMALS,C2U("Decimals"));
STATIC_USTRING(FM_PROP_REFVALUE,C2U("RefValue"));
STATIC_USTRING(FM_PROP_VALUEMIN,C2U("ValueMin"));
STATIC_USTRING(FM_PROP_VALUEMAX,C2U("ValueMax"));
STATIC_USTRING(FM_PROP_STRICTFORMAT,C2U("StrictFormat"));
STATIC_USTRING(FM_PROP_ALLOWADDITIONS,C2U("AllowInserts"));
STATIC_USTRING(FM_PROP_ALLOWEDITS,C2U("AllowUpdates"));
STATIC_USTRING(FM_PROP_ALLOWDELETIONS,C2U("AllowDeletes"));
STATIC_USTRING(FM_PROP_MASTERFIELDS,C2U("MasterFields"));
STATIC_USTRING(FM_PROP_SLAVEFIELDS,C2U("SlaveFields"));
STATIC_USTRING(FM_PROP_ISPASSTHROUGH,C2U("IsPassThrough"));
STATIC_USTRING(FM_PROP_QUERY,C2U("Query"));
STATIC_USTRING(FM_PROP_LITERALMASK,C2U("LiteralMask"));
STATIC_USTRING(FM_PROP_VALUESTEP,C2U("ValueStep"));
STATIC_USTRING(FM_PROP_SHOWTHOUSANDSEP,C2U("ShowThousandsSeparator"));
STATIC_USTRING(FM_PROP_CURRENCYSYMBOL,C2U("CurrencySymbol"));
STATIC_USTRING(FM_PROP_DATEFORMAT,C2U("DateFormat"));
STATIC_USTRING(FM_PROP_DATEMIN,C2U("DateMin"));
STATIC_USTRING(FM_PROP_DATEMAX,C2U("DateMax"));
STATIC_USTRING(FM_PROP_DATE_SHOW_CENTURY,C2U("DateShowCentury"));
STATIC_USTRING(FM_PROP_TIMEFORMAT, C2U("TimeFormat"));
STATIC_USTRING(FM_PROP_TIMEMIN, C2U("TimeMin"));
STATIC_USTRING(FM_PROP_TIMEMAX,C2U("TimeMax"));
STATIC_USTRING(FM_PROP_LINECOUNT,C2U("LineCount"));
STATIC_USTRING(FM_PROP_BOUNDCOLUMN, C2U("BoundColumn"));
STATIC_USTRING(FM_PROP_FONT,C2U("FontDescriptor"));
STATIC_USTRING(FM_PROP_BACKGROUNDCOLOR,C2U("BackgroundColor"));
STATIC_USTRING(FM_PROP_FILLCOLOR,C2U("FillColor"));
STATIC_USTRING(FM_PROP_TEXTCOLOR,C2U("TextColor"));
STATIC_USTRING(FM_PROP_LINECOLOR,C2U("LineColor"));
STATIC_USTRING(FM_PROP_BORDER,C2U("Border"));
STATIC_USTRING(FM_PROP_DROPDOWN,C2U("Dropdown"));
STATIC_USTRING(FM_PROP_MULTI,C2U("Multi"));
STATIC_USTRING(FM_PROP_HSCROLL,C2U("HScroll"));
STATIC_USTRING(FM_PROP_VSCROLL,C2U("VScroll"));
STATIC_USTRING(FM_PROP_TABSTOP,C2U("Tabstop"));
STATIC_USTRING(FM_PROP_AUTOCOMPLETE,C2U("Autocomplete"));
STATIC_USTRING(FM_PROP_HARDLINEBREAKS,C2U("HardLineBreaks"));
STATIC_USTRING(FM_PROP_PRINTABLE,C2U("Printable"));
STATIC_USTRING(FM_PROP_ECHO_CHAR,C2U("EchoChar"));
STATIC_USTRING(FM_PROP_ROWHEIGHT,C2U("RowHeight"));
STATIC_USTRING(FM_PROP_HELPTEXT, C2U("HelpText"));
STATIC_USTRING(FM_PROP_FONT_NAME, C2U("FontName"));
STATIC_USTRING(FM_PROP_FONT_STYLENAME, C2U("FontStyleName"));
STATIC_USTRING(FM_PROP_FONT_FAMILY, C2U("FontFamily"));
STATIC_USTRING(FM_PROP_FONT_CHARSET, C2U("FontCharset"));
STATIC_USTRING(FM_PROP_FONT_HEIGHT, C2U("FontHeight"));
STATIC_USTRING(FM_PROP_FONT_WEIGHT, C2U("FontWeight"));
STATIC_USTRING(FM_PROP_FONT_SLANT, C2U("FontSlant"));
STATIC_USTRING(FM_PROP_FONT_UNDERLINE, C2U("FontUnderline"));
STATIC_USTRING(FM_PROP_FONT_STRIKEOUT, C2U("FontStrikeout"));
STATIC_USTRING(FM_PROP_HELPURL, C2U("HelpURL"));
STATIC_USTRING(FM_PROP_RECORDMARKER, C2U("HasRecordMarker"));
STATIC_USTRING(FM_PROP_BOUNDFIELD, C2U("BoundField"));
STATIC_USTRING(FM_PROP_TREATASNUMERIC, C2U("TreatAsNumber"));
STATIC_USTRING(FM_PROP_EFFECTIVE_VALUE, C2U("EffectiveValue"));
STATIC_USTRING(FM_PROP_EFFECTIVE_DEFAULT, C2U("EffectiveDefault"));
STATIC_USTRING(FM_PROP_EFFECTIVE_MIN, C2U("EffectiveMin"));
STATIC_USTRING(FM_PROP_EFFECTIVE_MAX, C2U("EffectiveMax"));
STATIC_USTRING(FM_PROP_HIDDEN, C2U("Hidden"));
STATIC_USTRING(FM_PROP_FILTERPROPOSAL, C2U("useFilterValueProposal"));
STATIC_USTRING(FM_PROP_FIELDSOURCE, C2U("FieldSource"));
STATIC_USTRING(FM_PROP_TABLENAME, C2U("TableName"));
STATIC_USTRING(FM_PROP_FILTERSUPPLIER, C2U("FilterSupplier"));
STATIC_USTRING(FM_PROP_CURRENTFILTER, C2U("CurrentFilter"));
STATIC_USTRING(FM_PROP_SELECTED_FIELDS, C2U("SelectedFields"));
STATIC_USTRING(FM_PROP_SELECTED_TABLES, C2U("SelectedTables"));
STATIC_USTRING(FM_PROP_THREADSAFE, C2U("ThreadSafe"));
*/

