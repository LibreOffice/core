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


#include <runtime.hxx>
#include <rtlproto.hxx>
#include <errobject.hxx>


// Properties and methods lay the return value down at Get (bWrite = sal_False)
// at the element 0 of the Argv; at Put (bWrite = sal_True) the value from
// element 0 is stored.

void SbRtl_Erl(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutLong(StarBASIC::GetErl()); }

void SbRtl_Err(StarBASIC *, SbxArray & rPar, bool bWrite)
{
    if( SbiRuntime::isVBAEnabled() )
    {
        rPar.Get(0)->PutObject(SbxErrObject::getErrObject().get());
    }
    else
    {
        if( bWrite )
        {
            sal_Int32 nVal = rPar.Get(0)->GetLong();
            if( nVal <= 65535 )
                StarBASIC::Error( StarBASIC::GetSfxFromVBError( static_cast<sal_uInt16>(nVal) ) );
        }
        else
            rPar.Get(0)->PutLong(StarBASIC::GetVBErrorCode(StarBASIC::GetErrBasic()));
    }
}

void SbRtl_Empty(StarBASIC *, SbxArray &, bool) {}

void SbRtl_Nothing(StarBASIC *, SbxArray & rPar, bool)
{
    // return an empty object
    rPar.Get(0)->PutObject(nullptr);
}

void SbRtl_Null(StarBASIC *, SbxArray & rPar, bool)
{
    // returns an empty object-variable
    rPar.Get(0)->PutNull();
}

void SbRtl_PI(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutDouble(M_PI); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
