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

#include "lotfilter.hxx"
#include <lotimpop.hxx>

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include <scerrors.hxx>
#include <root.hxx>
#include <filtopt.hxx>
#include <ftools.hxx>

ErrCode ScFormatFilterPluginImpl::ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, rtl_TextEncoding eSrc )
{
    SvStream* pStream = rMedium.GetInStream();
    if (!pStream)
        return SCERR_IMPORT_OPEN;

    pStream->Seek( 0 );

    pStream->SetBufferSize( 32768 );

    LotusContext aContext;

    ImportLotus aLotusImport(aContext, *pStream, pDocument, eSrc);

    ErrCode eRet;
    if (ScFilterOptions().GetWK3Flag())
        eRet = aLotusImport.Read();
    else
        eRet = ErrCode(0xFFFFFFFF);  // force WK1 /WKS

    // WARNING: QUICK-HACK for WK1 / WKS  <->  WK3 / WK4
    if( eRet == ErrCode(0xFFFFFFFF) )
    {
        pStream->Seek( 0 );
        pStream->SetBufferSize( 32768 );
        eRet = ScImportLotus123old(aContext, *pStream, pDocument, eSrc);
        pStream->SetBufferSize( 0 );
        return eRet;
    }

    if( eRet != ERRCODE_NONE )
        return eRet;

    if (aContext.pLotusRoot->eFirstType == Lotus123Typ::WK3)
    {
        // try to load *.FM3 file
        INetURLObject aURL( rMedium.GetURLObject() );
        aURL.setExtension( "FM3" );
        SfxMedium aMedium( aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::STD_READ );
        pStream = aMedium.GetInStream();
        if ( pStream )
        {
            if( aLotusImport.Read( *pStream ) != ERRCODE_NONE )
                eRet = SCWARN_IMPORT_WRONG_FM3;
        }
        else
            eRet = SCWARN_IMPORT_OPEN_FM3;
    }

    return eRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
