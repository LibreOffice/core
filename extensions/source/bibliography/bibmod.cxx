/*************************************************************************
 *
 *  $RCSfile: bibmod.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2003-12-11 12:24:51 $
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
    ResId( nId, pBibModul->GetResMgr() )
{
}
BibConfig* BibModul::pBibConfig = 0;
BibModul::BibModul()
{
    pResMgr = ResMgr::CreateResMgr( "bib" MAKE_NUMSTR(SUPD) );
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
