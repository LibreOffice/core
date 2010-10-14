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

#ifndef _SDSTOR_STGOLE_HXX
#define _SDSTOR_STGOLE_HXX

#include <string.h> // memset()

#include "stg.hxx"
#include "stgelem.hxx"

class StgInternalStream : public SvStream
{
    BaseStorageStream* pStrm;
    virtual ULONG GetData( void* pData, ULONG nSize );
    virtual ULONG PutData( const void* pData, ULONG nSize );
    virtual ULONG SeekPos( ULONG nPos );
    virtual void  FlushData();
public:
    StgInternalStream( BaseStorage&, const String&, BOOL );
   ~StgInternalStream();
    void Commit();
};

// standard stream "\1CompObj"

class StgCompObjStream : public StgInternalStream
{
    ClsId  aClsId;
    String aUserName;
    ULONG  nCbFormat;
public:
    StgCompObjStream( BaseStorage&, BOOL );
    ClsId&  GetClsId()    { return aClsId;    }
    String& GetUserName() { return aUserName; }
    ULONG&  GetCbFormat() { return nCbFormat; }
    BOOL    Load();
    BOOL    Store();
};

// standard stream "\1Ole"

class StgOleStream : public StgInternalStream
{
    sal_uInt32 nFlags;
public:
    StgOleStream( BaseStorage&, BOOL );
    sal_uInt32& GetFlags() { return nFlags; }
    BOOL Load();
    BOOL Store();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
