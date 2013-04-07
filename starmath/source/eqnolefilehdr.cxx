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

#include "eqnolefilehdr.hxx"
#include <sot/storage.hxx>

//////////////////////////////////////////////////////////////////////



sal_Bool GetMathTypeVersion( SotStorage* pStor, sal_uInt8 &nVersion )
{
    sal_uInt8 nVer = 0;
    sal_Bool bSuccess = sal_False;

    //
    // code snippet copied from MathType::Parse
    //
    SvStorageStreamRef xSrc = pStor->OpenSotStream(
        OUString("Equation Native"),
        STREAM_STD_READ | STREAM_NOCREATE);
    if ( (!xSrc.Is()) || (SVSTREAM_OK != xSrc->GetError()))
        return bSuccess;
    SvStorageStream *pS = &xSrc;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    //
    EQNOLEFILEHDR aHdr;
    aHdr.Read(pS);
    *pS >> nVer;

    if (!pS->GetError())
    {
        nVersion = nVer;
        bSuccess = sal_True;
    }
    return bSuccess;
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
