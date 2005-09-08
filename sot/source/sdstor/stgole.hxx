/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stgole.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:43:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    ULONG nFlags;
public:
    StgOleStream( BaseStorage&, BOOL );
    ULONG& GetFlags() { return nFlags; }
    BOOL Load();
    BOOL Store();
};

#endif
