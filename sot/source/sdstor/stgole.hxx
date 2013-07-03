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

#ifndef _SDSTOR_STGOLE_HXX
#define _SDSTOR_STGOLE_HXX

#include <string.h> // memset()

#include "sot/stg.hxx"
#include "stgelem.hxx"

class StgInternalStream : public SvStream
{
    BaseStorageStream* pStrm;
    virtual sal_uLong GetData( void* pData, sal_uLong nSize );
    virtual sal_uLong PutData( const void* pData, sal_uLong nSize );
    virtual sal_uLong SeekPos( sal_uLong nPos );
    virtual void      FlushData();
public:
    StgInternalStream( BaseStorage&, const OUString&, bool );
   ~StgInternalStream();
    void Commit();
};

// standard stream "\1CompObj"

class StgCompObjStream : public StgInternalStream
{
    ClsId      aClsId;
    OUString   aUserName;
    sal_uLong  nCbFormat;
public:
    StgCompObjStream( BaseStorage&, bool );
    ClsId&     GetClsId()    { return aClsId;    }
    OUString&  GetUserName() { return aUserName; }
    sal_uLong& GetCbFormat() { return nCbFormat; }
    bool       Load();
    bool       Store();
};

// standard stream "\1Ole"

class StgOleStream : public StgInternalStream
{
    sal_uInt32 nFlags;
public:
    StgOleStream( BaseStorage&, bool );
    sal_uInt32& GetFlags() { return nFlags; }
    bool Load();
    bool Store();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
