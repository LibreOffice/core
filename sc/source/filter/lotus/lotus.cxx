/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "lotimpop.hxx"

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "scerrors.hxx"
#include "root.hxx"
#include "filtopt.hxx"
#include "ftools.hxx"

extern FltError ScImportLotus123old( SvStream&, ScDocument*, rtl_TextEncoding eSrc );
        // alter Krempel in filter.cxx!

FltError ScFormatFilterPluginImpl::ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, rtl_TextEncoding eSrc )
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
        aURL.setExtension( "FM3" );
        SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_STD_READ );
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
