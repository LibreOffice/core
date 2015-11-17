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

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>

#include <basic/sbx.hxx>
#include <basic/sbxfac.hxx>
#include "sbxbase.hxx"

#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>

// AppData-Structure for SBX:


SbxAppData::SbxAppData()
    : eSbxError(ERRCODE_SBX_OK)
    , pBasicFormater(nullptr)
    , eBasicFormaterLangType(LANGUAGE_DONTKNOW)
{
}

SbxAppData::~SbxAppData()
{
    SolarMutexGuard g;

    delete pBasicFormater;
    m_Factories.clear();
}

SbxBase::SbxBase()
{
    nFlags  = SbxFlagBits::ReadWrite;
}

SbxBase::SbxBase( const SbxBase& r )
    : SvRefBase( r )
{
    nFlags  = r.nFlags;
}

SbxBase::~SbxBase()
{
}

SbxBase& SbxBase::operator=( const SbxBase& r )
{
    nFlags = r.nFlags;
    return *this;
}

SbxDataType SbxBase::GetType() const
{
    return SbxEMPTY;
}

SbxClassType SbxBase::GetClass() const
{
    return SbxCLASS_DONTCARE;
}

void SbxBase::Clear()
{
}

bool SbxBase::IsFixed() const
{
    return IsSet( SbxFlagBits::Fixed );
}

void SbxBase::SetModified( bool b )
{
    if( IsSet( SbxFlagBits::NoModify ) )
        return;
    if( b )
        SetFlag( SbxFlagBits::Modified );
    else
        ResetFlag( SbxFlagBits::Modified );
}

SbxError SbxBase::GetError()
{
    return GetSbxData_Impl().eSbxError;
}

void SbxBase::SetError( SbxError e )
{
    SbxAppData& r = GetSbxData_Impl();
    if( e && r.eSbxError == ERRCODE_SBX_OK )
        r.eSbxError = e;
}

bool SbxBase::IsError()
{
    return GetSbxData_Impl().eSbxError != ERRCODE_SBX_OK;
}

void SbxBase::ResetError()
{
    GetSbxData_Impl().eSbxError = ERRCODE_SBX_OK;
}

void SbxBase::AddFactory( SbxFactory* pFac )
{
    SbxAppData& r = GetSbxData_Impl();

    // From 1996-03-06: take the HandleLast-Flag into account
    sal_uInt16 nPos = r.m_Factories.size(); // Insert position
    if( !pFac->IsHandleLast() )         // Only if not self HandleLast
    {
        // Rank new factory in front of factories with HandleLast
        while (nPos > 0 && r.m_Factories[ nPos-1 ]->IsHandleLast())
            nPos--;
    }
    r.m_Factories.insert(r.m_Factories.begin() + nPos, std::unique_ptr<SbxFactory>(pFac));
}

void SbxBase::RemoveFactory( SbxFactory* pFac )
{
    SbxAppData& r = GetSbxData_Impl();
    for (auto it = r.m_Factories.begin(); it != r.m_Factories.end(); ++it)
    {
        if ((*it).get() == pFac)
        {
            std::unique_ptr<SbxFactory> tmp(std::move(*it));
            r.m_Factories.erase( it );
            tmp.release();
            break;
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

    if( nCreator == SBXCR_SBX )
      switch( nSbxId )
    {
        case SBXID_VALUE:       return new SbxValue;
        case SBXID_VARIABLE:    return new SbxVariable;
        case SBXID_ARRAY:       return new SbxArray;
        case SBXID_DIMARRAY:    return new SbxDimArray;
        case SBXID_OBJECT:      return new SbxObject( "" );
        case SBXID_COLLECTION:  return new SbxCollection( "" );
        case SBXID_FIXCOLLECTION:
                                return new SbxStdCollection( "", "" );
        case SBXID_METHOD:      return new SbxMethod( "", SbxEMPTY );
        case SBXID_PROPERTY:    return new SbxProperty( "", SbxEMPTY );
    }
    // Unknown type: go over the factories!
    SbxAppData& r = GetSbxData_Impl();
    SbxBase* pNew = nullptr;
    for (auto const& rpFac : r.m_Factories)
    {
        pNew = rpFac->Create( nSbxId, nCreator );
        if( pNew )
            break;
    }
    SAL_WARN_IF(!pNew, "basic", "No factory for SBX ID " << nSbxId);
    return pNew;
}

SbxObject* SbxBase::CreateObject( const OUString& rClass )
{
    SbxAppData& r = GetSbxData_Impl();
    SbxObject* pNew = nullptr;
    for (auto const& rpFac : r.m_Factories)
    {
        pNew = rpFac->CreateObject( rClass );
        if( pNew )
            break;
    }
    SAL_WARN_IF(!pNew, "basic", "No factory for object class " << rClass);
    return pNew;
}

SbxBase* SbxBase::Load( SvStream& rStrm )
{
    sal_uInt16 nSbxId, nFlagsTmp, nVer;
    sal_uInt32 nCreator, nSize;
    rStrm.ReadUInt32( nCreator ).ReadUInt16( nSbxId ).ReadUInt16( nFlagsTmp ).ReadUInt16( nVer );
    SbxFlagBits nFlags = static_cast<SbxFlagBits>(nFlagsTmp);

    // Correcting a foolishness of mine:
    if( nFlags & SbxFlagBits::Reserved )
        nFlags = ( nFlags & ~SbxFlagBits::Reserved ) | SbxFlagBits::GlobalSearch;

    sal_Size nOldPos = rStrm.Tell();
    rStrm.ReadUInt32( nSize );
    SbxBase* p = Create( nSbxId, nCreator );
    if( p )
    {
        p->nFlags = nFlags;
        if( p->LoadData( rStrm, nVer ) )
        {
            sal_Size nNewPos = rStrm.Tell();
            nOldPos += nSize;
            DBG_ASSERT( nOldPos >= nNewPos, "SBX: Too much data loaded" );
            if( nOldPos != nNewPos )
                rStrm.Seek( nOldPos );
            if( !p->LoadCompleted() )
            {
                // Deleting of the object
                SbxBaseRef aRef( p );
                p = nullptr;
            }
        }
        else
        {
            rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            // Deleting of the object
            SbxBaseRef aRef( p );
            p = nullptr;
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
    rStrm.ReadUInt32( nCreator ).ReadUInt16( nSbxId ).ReadUInt16( nFlags ).ReadUInt16( nVer );

    sal_Size nStartPos = rStrm.Tell();
    rStrm.ReadUInt32( nSize );

    rStrm.Seek( nStartPos + nSize );
}

bool SbxBase::Store( SvStream& rStrm )
{
    if( ( nFlags & SbxFlagBits::DontStore ) == SbxFlagBits::NONE )
    {
        rStrm.WriteUInt32( GetCreator() )
             .WriteUInt16( GetSbxId() )
             .WriteUInt16( static_cast<sal_uInt16>(GetFlags()) )
             .WriteUInt16( GetVersion() );
        sal_Size nOldPos = rStrm.Tell();
        rStrm.WriteUInt32( 0L );
        bool bRes = StoreData( rStrm );
        sal_Size nNewPos = rStrm.Tell();
        rStrm.Seek( nOldPos );
        rStrm.WriteUInt32( nNewPos - nOldPos );
        rStrm.Seek( nNewPos );
        if( rStrm.GetError() != SVSTREAM_OK )
            bRes = false;
        if( bRes )
            bRes = true;
        return bRes;
    }
    else
        return true;
}

bool SbxBase::LoadData( SvStream&, sal_uInt16 )
{
    return false;
}

bool SbxBase::StoreData( SvStream& ) const
{
    return false;
}

bool SbxBase::LoadCompleted()
{
    return true;
}

//////////////////////////////// SbxFactory

SbxFactory::~SbxFactory()
{
}

SbxBase* SbxFactory::Create( sal_uInt16, sal_uInt32 )
{
    return nullptr;
}

SbxObject* SbxFactory::CreateObject( const OUString& )
{
    return nullptr;
}

///////////////////////////////// SbxInfo

SbxInfo::~SbxInfo()
{}

void SbxInfo::AddParam(const OUString& rName, SbxDataType eType, SbxFlagBits nFlags)
{
    m_Params.push_back(std::unique_ptr<SbxParamInfo>(new SbxParamInfo(rName, eType, nFlags)));
}

const SbxParamInfo* SbxInfo::GetParam( sal_uInt16 n ) const
{
    if (n < 1 || n > m_Params.size())
        return nullptr;
    else
        return m_Params[n - 1].get();
}

bool SbxInfo::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    m_Params.clear();
    sal_uInt16 nParam;
    aComment = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    aHelpFile = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
        RTL_TEXTENCODING_ASCII_US);
    rStrm.ReadUInt32( nHelpId ).ReadUInt16( nParam );
    while( nParam-- )
    {
        sal_uInt16 nType, nFlagsTmp;
        sal_uInt32 nUserData = 0;
        OUString aName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm,
            RTL_TEXTENCODING_ASCII_US);
        rStrm.ReadUInt16( nType ).ReadUInt16( nFlagsTmp );
        SbxFlagBits nFlags = static_cast<SbxFlagBits>(nFlagsTmp);
        if( nVer > 1 )
            rStrm.ReadUInt32( nUserData );
        AddParam( aName, (SbxDataType) nType, nFlags );
        SbxParamInfo& p(*m_Params.back());
        p.nUserData = nUserData;
    }
    return true;
}

bool SbxInfo::StoreData( SvStream& rStrm ) const
{
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aComment,
        RTL_TEXTENCODING_ASCII_US );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, aHelpFile,
        RTL_TEXTENCODING_ASCII_US);
    rStrm.WriteUInt32( nHelpId ).WriteUInt16( m_Params.size() );
    for (auto const& i : m_Params)
    {
        write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, i->aName,
            RTL_TEXTENCODING_ASCII_US);
        rStrm.WriteUInt16( i->eType )
             .WriteUInt16( static_cast<sal_uInt16>(i->nFlags) )
             .WriteUInt32( i->nUserData );
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
