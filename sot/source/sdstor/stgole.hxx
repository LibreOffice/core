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

#include "sot/stg.hxx"
#include "stgelem.hxx"

class StgInternalStream : public SvStream
{
    BaseStorageStream* pStrm;
    virtual sal_uLong GetData( void* pData, sal_uLong nSize );
    virtual sal_uLong PutData( const void* pData, sal_uLong nSize );
    virtual sal_uLong SeekPos( sal_uLong nPos );
    virtual void  FlushData();
public:
    StgInternalStream( BaseStorage&, const String&, sal_Bool );
   ~StgInternalStream();
    void Commit();
};

// standard stream "\1CompObj"

class StgCompObjStream : public StgInternalStream
{
    ClsId  aClsId;
    String aUserName;
    sal_uLong  nCbFormat;
public:
    StgCompObjStream( BaseStorage&, sal_Bool );
    ClsId&  GetClsId()    { return aClsId;    }
    String& GetUserName() { return aUserName; }
    sal_uLong&  GetCbFormat() { return nCbFormat; }
    sal_Bool    Load();
    sal_Bool    Store();
};

// standard stream "\1Ole"

class StgOleStream : public StgInternalStream
{
    sal_uInt32 nFlags;
public:
    StgOleStream( BaseStorage&, sal_Bool );
    sal_uInt32& GetFlags() { return nFlags; }
    sal_Bool Load();
    sal_Bool Store();
};

#endif
