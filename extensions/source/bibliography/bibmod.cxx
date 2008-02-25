/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibmod.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 15:32:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"


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

#include "bibmod.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
static PtrBibModul pBibModul=NULL;
static sal_uInt32 nBibModulCount=0;

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
    ResId( nId, *pBibModul->GetResMgr() )
{
}
BibConfig* BibModul::pBibConfig = 0;
BibModul::BibModul()
{
    pResMgr = ResMgr::CreateResMgr( "bib" );
}

BibModul::~BibModul()
{
    delete pResMgr;
    delete pBibConfig;
    pBibConfig = 0;
}

BibDataManager*  BibModul::createDataManager()
{
    return new BibDataManager();
}
//-----------------------------------------------------------------------------
BibConfig*  BibModul::GetConfig()
{
    if(! pBibConfig)
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
