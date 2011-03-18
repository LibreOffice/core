/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sc.hxx"
#include "lotimpop.hxx"

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "scerrors.hxx"
#include "root.hxx"
#include "filtopt.hxx"
#include "ftools.hxx"

//------------------------------------------------------------------------

extern FltError ScImportLotus123old( SvStream&, ScDocument*, CharSet eSrc );
        // alter Krempel in filter.cxx!

FltError ScFormatFilterPluginImpl::ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, CharSet eSrc )
{
        ScFilterOptions aFilterOpt;
    sal_Bool bWithWK3 = aFilterOpt.GetWK3Flag();

    SvStream*           pStream = rMedium.GetInStream();

    if( !pStream )
        return eERR_OPEN;

    FltError            eRet;

    pStream->Seek( 0UL );

    pStream->SetBufferSize( 32768 );

    ImportLotus         aLotusImport( *pStream, pDocument, eSrc );

    if( bWithWK3 )
        eRet = aLotusImport.Read();
    else
        eRet = 0xFFFFFFFF;  // WK1 /WKS erzwingen

    // ACHTUNG: QUICK-HACK fuer WK1 / WKS  <->  WK3 / WK4
    if( eRet == 0xFFFFFFFF )
    {
        pStream->Seek( 0UL );

        pStream->SetBufferSize( 32768 );

        eRet = ScImportLotus123old( *pStream, pDocument, eSrc );

        pStream->SetBufferSize( 0 );

        return eRet;
    }

    if( eRet != eERR_OK )
        return eRet;

    if( pLotusRoot->eFirstType == Lotus_WK3 )
    {// versuchen *.FM3-File zu laden
        INetURLObject aURL( rMedium.GetURLObject() );
        aURL.setExtension( CREATE_STRING( "FM3" ) );
        SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_STD_READ, sal_True );
        pStream = aMedium.GetInStream();
        if ( pStream )
        {
            if( aLotusImport.Read( *pStream ) != eERR_OK )
                eRet = SCWARN_IMPORT_WRONG_FM3;
        }
        else
            eRet = SCWARN_IMPORT_OPEN_FM3;
    }

    return eRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
