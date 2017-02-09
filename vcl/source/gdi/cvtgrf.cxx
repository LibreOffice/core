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

#include <vcl/metaact.hxx>
#include <vcl/cvtgrf.hxx>

#include <salinst.hxx>
#include <svdata.hxx>

// Callback
GraphicConverter::GraphicConverter()
{
}

GraphicConverter::~GraphicConverter()
{
}

ErrCode GraphicConverter::Import( SvStream& rIStm, Graphic& rGraphic, ConvertDataFormat nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    ErrCode             nRet = ERRCODE_IO_GENERAL;

    if( pCvt && pCvt->GetFilterHdl().IsSet() )
    {
        ConvertData aData( rGraphic, rIStm, nFormat );

        if( pCvt->GetFilterHdl().Call( aData ) )
        {
            rGraphic = aData.maGraphic;
            nRet = ERRCODE_NONE;
        }
        else if( rIStm.GetError() )
            nRet = rIStm.GetError();
    }

    return nRet;
}

ErrCode GraphicConverter::Export( SvStream& rOStm, const Graphic& rGraphic, ConvertDataFormat nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    ErrCode             nRet = ERRCODE_IO_GENERAL;

    if( pCvt && pCvt->GetFilterHdl().IsSet() )
    {
        ConvertData aData( rGraphic, rOStm, nFormat );

        if( pCvt->GetFilterHdl().Call( aData ) )
            nRet = ERRCODE_NONE;
        else if( rOStm.GetError() )
            nRet = rOStm.GetError();
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
