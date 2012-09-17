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

#include <tools/shl.hxx>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbxbase.hxx>

#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include <boost/foreach.hpp>

// AppData-Structure for SBX:

TYPEINIT0(SbxBase)

namespace
{
    class theSbxAppData : public rtl::Static<SbxAppData, theSbxAppData> {};
}

SbxAppData& GetSbxData_Impl()
{
    return theSbxAppData::get();
}

SbxAppData::~SbxAppData()
{
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
    return GetSbxData_Impl().eSbxError;
}

void SbxBase::SetError( SbxError e )
{
    SbxAppData& r = GetSbxData_Impl();
    if( e && r.eSbxError == SbxERR_OK )
        r.eSbxError = e;
}

sal_Bool SbxBase::IsError()
{
    return sal_Bool( GetSbxData_Impl().eSbxError != SbxERR_OK );
}

void SbxBase::ResetError()
{
    GetSbxData_Impl().eSbxError = SbxERR_OK;
}

void SbxBase::AddFactory( SbxFactory* pFac )
{
    SbxAppData& r = GetSbxData_Impl();

    // From 1996-03-06: take the HandleLast-Flag into account
    sal_uInt16 nPos = r.aFacs.size(); // Insert position
    if( !pFac->IsHandleLast() )         // Only if not self HandleLast
    {
        // Rank new factory in front of factories with HandleLast
        while( nPos > 0 &&
                r.aFacs[ nPos-1 ].IsHandleLast() )
            nPos--;
    }
    r.aFacs.insert( r.aFacs.begin() + nPos, pFac );
}

void SbxBase::RemoveFactory( SbxFactory* pFac )
{
    SbxAppData& r = GetSbxData_Impl();
    for(SbxFacs::iterator it = r.aFacs.begin(); it != r.aFacs.end(); ++it)
    {
        if( &(*it) == pFac )
        {
            r.aFacs.release( it ).release(); break;
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

    rtl::OUString aEmptyStr;
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
    SbxAppData& r = GetSbxData_Impl();
    SbxBase* pNew = NULL;
    BOOST_FOREACH(SbxFactory& rFac, r.aFacs)
    {
        pNew = rFac.Create( nSbxId, nCreator );
        if( pNew )
            break;
    }
    SAL_WARN_IF(!pNew, "basic", "No factory for SBX ID " << nSbxId);
    return pNew;
}

SbxObject* SbxBase::CreateObject( const rtl::OUString& rClass )
{
    SbxAppData& r = GetSbxData_Impl();
    SbxObject* pNew = NULL;
    BOOST_FOREACH(SbxFactory& rFac, r.aFacs)
    {
        pNew = rFac.CreateObject( rClass );
        if( pNew )
            break;
    }
    SAL_WARN_IF(!pNew, "basic", "No factory for object class " << rClass);
    return pNew;
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

SbxFactory::~SbxFactory()
{
}

SbxBase* SbxFactory::Create( sal_uInt16, sal_uInt32 )
{
    return NULL;
}

SbxObject* SbxFactory::CreateObject( const rtl::OUString& )
{
    return NULL;
}

///////////////////////////////// SbxInfo //////////////////////////////////

SbxInfo::~SbxInfo()
{}

void SbxInfo::AddParam(const rtl::OUString& rName, SbxDataType eType, sal_uInt16 nFlags)
{
    aParams.push_back(new SbxParamInfo(rName, eType, nFlags));
}

const SbxParamInfo* SbxInfo::GetParam( sal_uInt16 n ) const
{
    if( n < 1 || n > aParams.size() )
        return NULL;
    else
        return &(aParams[n - 1]);
}

sal_Bool SbxInfo::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    aParams.clear();
    sal_uInt16 nParam;
    aComment = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    aHelpFile = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    rStrm >> nHelpId >> nParam;
    while( nParam-- )
    {
        sal_uInt16 nType, nFlags;
        sal_uInt32 nUserData = 0;
        rtl::OUString aName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rStrm,
            RTL_TEXTENCODING_ASCII_US);
        rStrm >> nType >> nFlags;
        if( nVer > 1 )
            rStrm >> nUserData;
        AddParam( aName, (SbxDataType) nType, nFlags );
        SbxParamInfo& p(aParams.back());
        p.nUserData = nUserData;
    }
    return sal_True;
}

sal_Bool SbxInfo::StoreData( SvStream& rStrm ) const
{
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aComment,
        RTL_TEXTENCODING_ASCII_US );
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, aHelpFile,
        RTL_TEXTENCODING_ASCII_US);
    rStrm << nHelpId << static_cast<sal_uInt16>(aParams.size());
    for(SbxParams::const_iterator i = aParams.begin(); i != aParams.end(); ++i)
    {
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rStrm, i->aName,
            RTL_TEXTENCODING_ASCII_US);
        rStrm << (sal_uInt16) i->eType
              << (sal_uInt16) i->nFlags
              << (sal_uInt32) i->nUserData;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
