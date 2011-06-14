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
#include "precompiled_basic.hxx"


#include <tools/shl.hxx>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbxbase.hxx>

// AppData-Structure for SBX:

SV_IMPL_PTRARR(SbxParams,SbxParamInfo*);
SV_IMPL_PTRARR(SbxFacs,SbxFactory*);

TYPEINIT0(SbxBase)

// Request SBX-Data or if necessary create them
// we just create the area and waive the release!

SbxAppData* GetSbxData_Impl()
{
    SbxAppData** ppData = (SbxAppData**) ::GetAppData( SHL_SBX );
    SbxAppData* p = *ppData;
    if( !p )
        p = *ppData  = new SbxAppData;
    return p;
}

SbxAppData::~SbxAppData()
{
    if( pBasicFormater )
        delete pBasicFormater;
}


//////////////////////////////// SbxBase /////////////////////////////////

DBG_NAME(SbxBase);

SbxBase::SbxBase()
{
    DBG_CTOR( SbxBase, 0 );
    nFlags  = SBX_READWRITE;
}

SbxBase::SbxBase( const SbxBase& r )
    : SvRefBase( r )
{
    DBG_CTOR( SbxBase, 0 );
    nFlags  = r.nFlags;
}

SbxBase::~SbxBase()
{
    DBG_DTOR(SbxBase,0);
}

SbxBase& SbxBase::operator=( const SbxBase& r )
{
    DBG_CHKTHIS( SbxBase, 0 );
    nFlags = r.nFlags;
    return *this;
}

SbxDataType SbxBase::GetType() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return SbxEMPTY;
}

SbxClassType SbxBase::GetClass() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return SbxCLASS_DONTCARE;
}

void SbxBase::Clear()
{
    DBG_CHKTHIS( SbxBase, 0 );
}

sal_Bool SbxBase::IsFixed() const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return IsSet( SBX_FIXED );
}

void SbxBase::SetModified( sal_Bool b )
{
    DBG_CHKTHIS( SbxBase, 0 );
    if( IsSet( SBX_NO_MODIFY ) )
        return;
    if( b )
        SetFlag( SBX_MODIFIED );
    else
        ResetFlag( SBX_MODIFIED );
}

SbxError SbxBase::GetError()
{
    return GetSbxData_Impl()->eSbxError;
}

void SbxBase::SetError( SbxError e )
{
    SbxAppData* p = GetSbxData_Impl();
    if( e && p->eSbxError == SbxERR_OK )
        p->eSbxError = e;
}

sal_Bool SbxBase::IsError()
{
    return sal_Bool( GetSbxData_Impl()->eSbxError != SbxERR_OK );
}

void SbxBase::ResetError()
{
    GetSbxData_Impl()->eSbxError = SbxERR_OK;
}

void SbxBase::AddFactory( SbxFactory* pFac )
{
    SbxAppData* p = GetSbxData_Impl();
    const SbxFactory* pTemp = pFac;

    // From 1996-03-06: take the HandleLast-Flag into account
    sal_uInt16 nPos = p->aFacs.Count(); // Insert position
    if( !pFac->IsHandleLast() )         // Only if not self HandleLast
    {
        // Rank new factory in front of factories with HandleLast
        while( nPos > 0 &&
                (static_cast<SbxFactory*>(p->aFacs.GetObject( nPos-1 )))->IsHandleLast() )
            nPos--;
    }
    p->aFacs.Insert( pTemp, nPos );
}

void SbxBase::RemoveFactory( SbxFactory* pFac )
{
    SbxAppData* p = GetSbxData_Impl();
    for( sal_uInt16 i = 0; i < p->aFacs.Count(); i++ )
    {
        if( p->aFacs.GetObject( i ) == pFac )
        {
            p->aFacs.Remove( i, 1 ); break;
        }
    }
}


SbxBase* SbxBase::Create( sal_uInt16 nSbxId, sal_uInt32 nCreator )
{
    // #91626: Hack to skip old Basic dialogs
    // Problem: There does not exist a factory any more,
    // so we have to create a dummy SbxVariable instead
    if( nSbxId == 0x65 )    // Dialog Id
        return new SbxVariable;

    XubString aEmptyStr;
    if( nCreator == SBXCR_SBX )
      switch( nSbxId )
    {
        case SBXID_VALUE:       return new SbxValue;
        case SBXID_VARIABLE:    return new SbxVariable;
        case SBXID_ARRAY:       return new SbxArray;
        case SBXID_DIMARRAY:    return new SbxDimArray;
        case SBXID_OBJECT:      return new SbxObject( aEmptyStr );
        case SBXID_COLLECTION:  return new SbxCollection( aEmptyStr );
        case SBXID_FIXCOLLECTION:
                                return new SbxStdCollection( aEmptyStr, aEmptyStr );
        case SBXID_METHOD:      return new SbxMethod( aEmptyStr, SbxEMPTY );
        case SBXID_PROPERTY:    return new SbxProperty( aEmptyStr, SbxEMPTY );
    }
    // Unknown type: go over the factories!
    SbxAppData* p = GetSbxData_Impl();
    SbxBase* pNew = NULL;
    for( sal_uInt16 i = 0; i < p->aFacs.Count(); i++ )
    {
        SbxFactory* pFac = p->aFacs.GetObject( i );
        pNew = pFac->Create( nSbxId, nCreator );
        if( pNew )
            break;
    }
#ifdef DBG_UTIL
    if( !pNew )
    {
        ByteString aMsg( "SBX: Keine Factory fuer SBX-ID " );
        aMsg += ByteString::CreateFromInt32(nSbxId);
        DbgError( aMsg.GetBuffer() );
    }
#endif
    return pNew;
}

SbxObject* SbxBase::CreateObject( const XubString& rClass )
{
    SbxAppData* p = GetSbxData_Impl();
    SbxObject* pNew = NULL;
    for( sal_uInt16 i = 0; i < p->aFacs.Count(); i++ )
    {
        pNew = p->aFacs.GetObject( i )->CreateObject( rClass );
        if( pNew )
            break;
    }
#ifdef DBG_UTIL
    if( !pNew )
    {
        ByteString aMsg( "SBX: Keine Factory fuer Objektklasse " );
        ByteString aClassStr( (const UniString&)rClass, RTL_TEXTENCODING_ASCII_US );
        aMsg += aClassStr;
        DbgError( (const char*)aMsg.GetBuffer() );
    }
#endif
    return pNew;
}

static sal_Bool bStaticEnableBroadcasting = sal_True;

// Sbx-Solution in exchange for SfxBroadcaster::Enable()
void SbxBase::StaticEnableBroadcasting( sal_Bool bEnable )
{
    bStaticEnableBroadcasting = bEnable;
}

sal_Bool SbxBase::StaticIsEnabledBroadcasting( void )
{
    return bStaticEnableBroadcasting;
}


SbxBase* SbxBase::Load( SvStream& rStrm )
{
    sal_uInt16 nSbxId, nFlags, nVer;
    sal_uInt32 nCreator, nSize;
    rStrm >> nCreator >> nSbxId >> nFlags >> nVer;

    // Correcting a foolishness of mine:
    if( nFlags & SBX_RESERVED )
        nFlags = ( nFlags & ~SBX_RESERVED ) | SBX_GBLSEARCH;

    sal_uIntPtr nOldPos = rStrm.Tell();
    rStrm >> nSize;
    SbxBase* p = Create( nSbxId, nCreator );
    if( p )
    {
        p->nFlags = nFlags;
        if( p->LoadData( rStrm, nVer ) )
        {
            sal_uIntPtr nNewPos = rStrm.Tell();
            nOldPos += nSize;
            DBG_ASSERT( nOldPos >= nNewPos, "SBX: Zu viele Daten eingelesen" );
            if( nOldPos != nNewPos )
                rStrm.Seek( nOldPos );
            if( !p->LoadCompleted() )
            {
                // Deleting of the object
                SbxBaseRef aRef( p );
                p = NULL;
            }
        }
        else
        {
            rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            // Deleting of the object
            SbxBaseRef aRef( p );
            p = NULL;
        }
    }
    else
        rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
    return p;
}

// Skip the Sbx-Object inside the stream
void SbxBase::Skip( SvStream& rStrm )
{
    sal_uInt16 nSbxId, nFlags, nVer;
    sal_uInt32 nCreator, nSize;
    rStrm >> nCreator >> nSbxId >> nFlags >> nVer;

    sal_uIntPtr nStartPos = rStrm.Tell();
    rStrm >> nSize;

    rStrm.Seek( nStartPos + nSize );
}

sal_Bool SbxBase::Store( SvStream& rStrm )
{
    DBG_CHKTHIS( SbxBase, 0 );
    if( !( nFlags & SBX_DONTSTORE ) )
    {
        rStrm << (sal_uInt32) GetCreator()
              << (sal_uInt16) GetSbxId()
              << (sal_uInt16) GetFlags()
              << (sal_uInt16) GetVersion();
        sal_uIntPtr nOldPos = rStrm.Tell();
        rStrm << (sal_uInt32) 0L;
        sal_Bool bRes = StoreData( rStrm );
        sal_uIntPtr nNewPos = rStrm.Tell();
        rStrm.Seek( nOldPos );
        rStrm << (sal_uInt32) ( nNewPos - nOldPos );
        rStrm.Seek( nNewPos );
        if( rStrm.GetError() != SVSTREAM_OK )
            bRes = sal_False;
        if( bRes )
            bRes = StoreCompleted();
        return bRes;
    }
    else
        return sal_True;
}

sal_Bool SbxBase::LoadData( SvStream&, sal_uInt16 )
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_False;
}

sal_Bool SbxBase::StoreData( SvStream& ) const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_False;
}

sal_Bool SbxBase::LoadPrivateData( SvStream&, sal_uInt16 )
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_True;
}

sal_Bool SbxBase::StorePrivateData( SvStream& ) const
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_True;
}

sal_Bool SbxBase::LoadCompleted()
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_True;
}

sal_Bool SbxBase::StoreCompleted()
{
    DBG_CHKTHIS( SbxBase, 0 );
    return sal_True;
}

//////////////////////////////// SbxFactory ////////////////////////////////

SbxBase* SbxFactory::Create( sal_uInt16, sal_uInt32 )
{
    return NULL;
}

SbxObject* SbxFactory::CreateObject( const XubString& )
{
    return NULL;
}

///////////////////////////////// SbxInfo //////////////////////////////////

SbxInfo::~SbxInfo()
{}

void SbxInfo::AddParam
        ( const XubString& rName, SbxDataType eType, sal_uInt16 nFlags )
{
    const SbxParamInfo* p = new SbxParamInfo( rName, eType, nFlags );
    aParams.Insert( p, aParams.Count() );
}

void SbxInfo::AddParam( const SbxParamInfo& r )
{
    const SbxParamInfo* p = new SbxParamInfo
        ( r.aName, r.eType, r.nFlags, r.aTypeRef );
    aParams.Insert( p, aParams.Count() );
}

const SbxParamInfo* SbxInfo::GetParam( sal_uInt16 n ) const
{
    if( n < 1 || n > aParams.Count() )
        return NULL;
    else
        return aParams.GetObject( n-1 );
}

sal_Bool SbxInfo::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    aParams.Remove( 0, aParams.Count() );
    sal_uInt16 nParam;
    rStrm.ReadByteString( aComment, RTL_TEXTENCODING_ASCII_US );
    rStrm.ReadByteString( aHelpFile, RTL_TEXTENCODING_ASCII_US );
    rStrm >> nHelpId >> nParam;
    while( nParam-- )
    {
        XubString aName;
        sal_uInt16 nType, nFlags;
        sal_uInt32 nUserData = 0;
        rStrm.ReadByteString( aName, RTL_TEXTENCODING_ASCII_US );
        rStrm >> nType >> nFlags;
        if( nVer > 1 )
            rStrm >> nUserData;
        AddParam( aName, (SbxDataType) nType, nFlags );
        SbxParamInfo* p = aParams.GetObject( aParams.Count() - 1 );
        p->nUserData = nUserData;
    }
    return sal_True;
}

sal_Bool SbxInfo::StoreData( SvStream& rStrm ) const
{
    rStrm.WriteByteString( aComment, RTL_TEXTENCODING_ASCII_US );
    rStrm.WriteByteString( aHelpFile, RTL_TEXTENCODING_ASCII_US );
    rStrm << nHelpId << aParams.Count();
    for( sal_uInt16 i = 0; i < aParams.Count(); i++ )
    {
        SbxParamInfo* p = aParams.GetObject( i );
        rStrm.WriteByteString( p->aName, RTL_TEXTENCODING_ASCII_US );
        rStrm << (sal_uInt16) p->eType
              << (sal_uInt16) p->nFlags
              << (sal_uInt32) p->nUserData;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
