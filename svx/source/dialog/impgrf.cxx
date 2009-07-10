/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impgrf.cxx,v $
 * $Revision: 1.27 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
// include ------------------------------------------------------------------

#include <limits.h>             // USHRT_MAX
#include <tools/urlobj.hxx>
#include <tools/bigint.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/viewoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <osl/file.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>

using namespace ::ucbhelper;
using namespace com::sun::star::uno;

#include <svx/dialogs.hrc>
#include "impgrf.hrc"

#define _SVX_IMPGRF_CXX
#include "impgrf.hxx"

#include <svx/dialmgr.hxx>
#include "svxerr.hxx"
#include "helpid.hrc"

// defines ---------------------------------------------------------------

#define IMPGRF_INIKEY_ASLINK        "ImportGraphicAsLink"
#define IMPGRF_INIKEY_PREVIEW       "ImportGraphicPreview"
#define IMPGRF_CONFIGNAME           String(DEFINE_CONST_UNICODE("ImportGraphicDialog"))

GraphicFilter* GetGrfFilter()
{
    return GraphicFilter::GetGraphicFilter();
}

// -----------------------------------------------------------------------

int LoadGraphic( const String &rPath, const String &rFilterName,
                 Graphic& rGraphic, GraphicFilter* pFilter,
                 USHORT* pDeterminedFormat )
{
    if ( !pFilter )
        pFilter = ::GetGrfFilter();

    const USHORT nFilter = rFilterName.Len() && pFilter->GetImportFormatCount()
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
        pMed->DownLoad();
        pStream = pMed->GetInStream();
    }
    int nRes = GRFILTER_OK;

    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream,
                                       nFilter, pDeterminedFormat );

#ifdef DBG_UTIL
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

