/*************************************************************************
 *
 *  $RCSfile: impgrf.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:42:23 $
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

// include ------------------------------------------------------------------

#include <limits.h>             // USHRT_MAX

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#pragma hdrstop

#include <ucbhelper/content.hxx>
#include <osl/file.hxx>

#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

using namespace ::ucb;
using namespace com::sun::star::uno;

#include "dialogs.hrc"
#include "impgrf.hrc"

#define _SVX_IMPGRF_CXX
#include "impgrf.hxx"

#include "dialmgr.hxx"
#include "svxerr.hxx"
#include "helpid.hrc"

// defines ---------------------------------------------------------------

#define IMPGRF_INIKEY_ASLINK        "ImportGraphicAsLink"
#define IMPGRF_INIKEY_PREVIEW       "ImportGraphicPreview"
#define IMPGRF_CONFIGNAME           String(DEFINE_CONST_UNICODE("ImportGraphicDialog"))


// -----------------------------------------------------------------------

GraphicFilter* DialogsResMgr::GetGrfFilter_Impl()
{
    if( !pGrapicFilter )
    {
#ifndef SVX_LIGHT
        pGrapicFilter = new GraphicFilter;
#else
        pGrapicFilter = new GraphicFilter(sal_False);
#endif
        ::FillFilter( *pGrapicFilter );
    }
    const Link aLink;
    pGrapicFilter->SetStartFilterHdl( aLink );
    pGrapicFilter->SetEndFilterHdl( aLink );
    pGrapicFilter->SetUpdatePercentHdl( aLink );
    return pGrapicFilter;
}

// -----------------------------------------------------------------------

GraphicFilter* GetGrfFilter()
{
    return (*(DialogsResMgr**)GetAppData(SHL_SVX))->GetGrfFilter_Impl();
}

// -----------------------------------------------------------------------

USHORT FillFilter( GraphicFilter& rFilter )
{
    ResMgr* pMgr = DIALOG_MGR();
    return rFilter.GetImportFormatCount();
}

// -----------------------------------------------------------------------

#pragma optimize( "", off )

int LoadGraphic( const String &rPath, const String &rFilterName,
                 Graphic& rGraphic, GraphicFilter* pFilter,
                 USHORT* pDeterminedFormat )
{
    if ( !pFilter )
        pFilter = ::GetGrfFilter();

    const int nFilter = rFilterName.Len() && pFilter->GetImportFormatCount()
                    ? pFilter->GetImportFormatNumber( rFilterName )
                    : GRFILTER_FORMAT_DONTKNOW;

    SfxMedium* pMed = 0;

    // dann teste mal auf File-Protokoll:
    SvStream* pStream = NULL;
    INetURLObject aURL( rPath );

    if ( aURL.HasError() || INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rPath );
    }
    else if ( INET_PROT_FILE != aURL.GetProtocol() )
    {
        // z.Z. nur auf die aktuelle DocShell
        pMed = new SfxMedium( rPath, STREAM_READ, TRUE );
        pMed->SetTransferPriority( SFX_TFPRIO_SYNCHRON );
        pMed->DownLoad();
        pStream = pMed->GetInStream();
    }
    int nRes = GRFILTER_OK;

    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream,
                                       nFilter, pDeterminedFormat );

#ifndef PRODUCT
    if( nRes )
    {
        if( pMed )
        {
            DBG_WARNING3( "GrafikFehler [%d] - [%s] URL[%s]",
                            nRes,
                            pMed->GetPhysicalName().GetBuffer(),
                            rPath.GetBuffer() );
        }
        else
        {
            DBG_WARNING2( "GrafikFehler [%d] - [%s]", nRes, rPath.GetBuffer() );
        }
    }
#endif

    if ( pMed )
        delete pMed;
    return nRes;
}

#pragma optimize( "", on )

