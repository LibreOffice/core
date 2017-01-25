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

#include <config_features.h>

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <basic/sbx.hxx>
#include "runtime.hxx"

#include <boost/optional.hpp>

using namespace std;

struct SbxVarEntry
{
    SbxVariableRef mpVar;
    boost::optional<OUString> maAlias;
};


//  SbxArray

SbxArray::SbxArray( SbxDataType t ) : SbxBase()
{
    eType = t;
    if( t != SbxVARIANT )
        SetFlag( SbxFlagBits::Fixed );
}

SbxArray::SbxArray( const SbxArray& rArray ) :
    SvRefBase( rArray ), SbxBase()
{
    if( rArray.eType != SbxVARIANT )
        SetFlag( SbxFlagBits::Fixed );
    *this = rArray;
}

SbxArray& SbxArray::operator=( const SbxArray& rArray )
{
    if( &rArray != this )
    {
        eType = rArray.eType;
        Clear();
        for( const auto& rpSrcRef : rArray.mVarEntries )
        {
            SbxVariableRef pSrc_ = rpSrcRef.mpVar;
            if( !pSrc_.is() )
                continue;

            if( eType != SbxVARIANT )
            {
                // Convert no objects
                if( eType != SbxOBJECT || pSrc_->GetClass() != SbxClassType::Object )
                {
                    pSrc_->Convert(eType);
                }
            }
            mVarEntries.push_back( rpSrcRef );
        }
    }
    return *this;
}

SbxArray::~SbxArray()
{
}

SbxDataType SbxArray::GetType() const
{
    return (SbxDataType) ( eType | SbxARRAY );
}

SbxClassType SbxArray::GetClass() const
{
    return SbxClassType::Array;
}

void SbxArray::Clear()
{
    mVarEntries.clear();
}

sal_uInt32 SbxArray::Count32() const
{
    return mVarEntries.size();
}

sal_uInt16 SbxArray::Count() const
{
    sal_uInt32 nCount = mVarEntries.size();
    DBG_ASSERT( nCount <= SBX_MAXINDEX, "SBX: Array-Index > SBX_MAXINDEX" );
    return (sal_uInt16)nCount;
}

SbxVariableRef& SbxArray::GetRef32( sal_uInt32 nIdx )
{
    // If necessary extend the array
    DBG_ASSERT( nIdx <= SBX_MAXINDEX32, "SBX: Array-Index > SBX_MAXINDEX32" );
    // Very Hot Fix
    if( nIdx > SBX_MAXINDEX32 )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        nIdx = 0;
    }
    if ( mVarEntries.size() <= nIdx )
        mVarEntries.resize(nIdx+1);

    return mVarEntries[nIdx].mpVar;
}

SbxVariableRef& SbxArray::GetRef( sal_uInt16 nIdx )
{
    // If necessary extend the array
    DBG_ASSERT( nIdx <= SBX_MAXINDEX, "SBX: Array-Index > SBX_MAXINDEX" );
    // Very Hot Fix
    if( nIdx > SBX_MAXINDEX )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        nIdx = 0;
    }
    if ( mVarEntries.size() <= nIdx )
        mVarEntries.resize(nIdx+1);

    return mVarEntries[nIdx].mpVar;
}

SbxVariable* SbxArray::Get32( sal_uInt32 nIdx )
{
    if( !CanRead() )
    {
        SetError( ERRCODE_SBX_PROP_WRITEONLY );
        return nullptr;
    }
    SbxVariableRef& rRef = GetRef32( nIdx );

    if ( !rRef.is() )
        rRef = new SbxVariable( eType );

    return rRef.get();
}

SbxVariable* SbxArray::Get( sal_uInt16 nIdx )
{
    if( !CanRead() )
    {
        SetError( ERRCODE_SBX_PROP_WRITEONLY );
        return nullptr;
    }
    SbxVariableRef& rRef = GetRef( nIdx );

    if ( !rRef.is() )
        rRef = new SbxVariable( eType );

    return rRef.get();
}

void SbxArray::Put32( SbxVariable* pVar, sal_uInt32 nIdx )
{
    if( !CanWrite() )
        SetError( ERRCODE_SBX_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Convert no objects
                if( eType != SbxOBJECT || pVar->GetClass() != SbxClassType::Object )
                    pVar->Convert( eType );
        SbxVariableRef& rRef = GetRef32( nIdx );
        if( rRef.get() != pVar )
        {
            rRef = pVar;
            SetFlag( SbxFlagBits::Modified );
        }
    }
}

void SbxArray::Put( SbxVariable* pVar, sal_uInt16 nIdx )
{
    if( !CanWrite() )
        SetError( ERRCODE_SBX_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Convert no objects
                if( eType != SbxOBJECT || pVar->GetClass() != SbxClassType::Object )
                    pVar->Convert( eType );
        SbxVariableRef& rRef = GetRef( nIdx );
        if(rRef.get() != pVar )
        {
            rRef = pVar;
            SetFlag( SbxFlagBits::Modified );
        }
    }
}

OUString SbxArray::GetAlias( sal_uInt16 nIdx )
{
    if( !CanRead() )
    {
        SetError( ERRCODE_SBX_PROP_WRITEONLY );
        return OUString();
    }
    SbxVarEntry& rRef = reinterpret_cast<SbxVarEntry&>(GetRef( nIdx ));

    if (!rRef.maAlias)
        return OUString();

    return *rRef.maAlias;
}

void SbxArray::PutAlias( const OUString& rAlias, sal_uInt16 nIdx )
{
    if( !CanWrite() )
    {
        SetError( ERRCODE_SBX_PROP_READONLY );
    }
    else
    {
        SbxVarEntry& rRef = reinterpret_cast<SbxVarEntry&>( GetRef( nIdx ) );
        rRef.maAlias.reset(rAlias);
    }
}

void SbxArray::Insert32( SbxVariable* pVar, sal_uInt32 nIdx )
{
    DBG_ASSERT( mVarEntries.size() <= SBX_MAXINDEX32, "SBX: Array gets too big" );
    if( mVarEntries.size() > SBX_MAXINDEX32 )
    {
            return;
    }
    SbxVarEntry p;
    p.mpVar = pVar;
    size_t nSize = mVarEntries.size();
    if( nIdx > nSize )
    {
        nIdx = nSize;
    }
    if( eType != SbxVARIANT && pVar )
    {
        p.mpVar->Convert(eType);
    }
    if( nIdx == nSize )
    {
        mVarEntries.push_back( p );
    }
    else
    {
        mVarEntries.insert( mVarEntries.begin() + nIdx, p );
    }
    SetFlag( SbxFlagBits::Modified );
}

void SbxArray::Insert( SbxVariable* pVar, sal_uInt16 nIdx )
{
    DBG_ASSERT( mVarEntries.size() <= 0x3FF0, "SBX: Array gets too big" );
    if( mVarEntries.size() > 0x3FF0 )
    {
        return;
    }
    Insert32( pVar, nIdx );
}

void SbxArray::Remove( sal_uInt32 nIdx )
{
    if( nIdx < mVarEntries.size() )
    {
        mVarEntries.erase( mVarEntries.begin() + nIdx );
        SetFlag( SbxFlagBits::Modified );
    }
}

void SbxArray::Remove( SbxVariable* pVar )
{
    if( pVar )
    {
        for( size_t i = 0; i < mVarEntries.size(); i++ )
        {
            if (mVarEntries[i].mpVar.get() == pVar)
            {
                Remove( i ); break;
            }
        }
    }
}

// Taking over of the data from the passed array, at which
// the variable of the same name will be overwritten.

void SbxArray::Merge( SbxArray* p )
{
    if (!p)
        return;

    for (auto& rEntry1: p->mVarEntries)
    {
        if (!rEntry1.mpVar.is())
            continue;

        OUString aName = rEntry1.mpVar->GetName();
        sal_uInt16 nHash = rEntry1.mpVar->GetHashCode();

        // Is the element by the same name already inside?
        // Then overwrite!
        for (auto& rEntry2: mVarEntries)
        {
            if (!rEntry2.mpVar.is())
                continue;

            if (rEntry2.mpVar->GetHashCode() == nHash &&
                rEntry2.mpVar->GetName().equalsIgnoreAsciiCase(aName))
            {
                // Take this element and clear the original.
                rEntry2.mpVar = rEntry1.mpVar;
                rEntry1.mpVar.clear();
                break;
            }
        }

        if (rEntry1.mpVar.is())
        {
            // We don't have element with the same name.  Add a new entry.
            SbxVarEntry aNewEntry;
            aNewEntry.mpVar = rEntry1.mpVar;
            if (rEntry1.maAlias)
                aNewEntry.maAlias.reset(*rEntry1.maAlias);
            mVarEntries.push_back(aNewEntry);
        }
    }
}

// Search of an element via the user data. If the element is
// object, it will also be scanned.

SbxVariable* SbxArray::FindUserData( sal_uInt32 nData )
{
    SbxVariable* p = nullptr;
    for (auto& rEntry : mVarEntries)
    {
        if (!rEntry.mpVar.is())
            continue;

        if (rEntry.mpVar->IsVisible() && rEntry.mpVar->GetUserData() == nData)
        {
            p = rEntry.mpVar.get();
            p->ResetFlag( SbxFlagBits::ExtFound );
            break;  // JSM 1995-10-06
        }

        // Did we have an array/object with extended search?
        if (rEntry.mpVar->IsSet(SbxFlagBits::ExtSearch))
        {
            switch (rEntry.mpVar->GetClass())
            {
                case SbxClassType::Object:
                {
                    // Objects are not allowed to scan their parent.
                    SbxFlagBits nOld = rEntry.mpVar->GetFlags();
                    rEntry.mpVar->ResetFlag(SbxFlagBits::GlobalSearch);
                    p = static_cast<SbxObject&>(*rEntry.mpVar).FindUserData(nData);
                    rEntry.mpVar->SetFlags(nOld);
                }
                break;
                case SbxClassType::Array:
                    // Casting SbxVariable to SbxArray?  Really?
                    p = reinterpret_cast<SbxArray&>(*rEntry.mpVar).FindUserData(nData);
                break;
                default:
                    ;
            }

            if (p)
            {
                p->SetFlag(SbxFlagBits::ExtFound);
                break;
            }
        }
    }
    return p;
}

// Search of an element by his name and type. If an element is an object,
// it will also be scanned..

SbxVariable* SbxArray::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* p = nullptr;
    if( mVarEntries.empty() )
        return nullptr;
    bool bExtSearch = IsSet( SbxFlagBits::ExtSearch );
    sal_uInt16 nHash = SbxVariable::MakeHashCode( rName );
    for (auto& rEntry : mVarEntries)
    {
        if (!rEntry.mpVar.is() || !rEntry.mpVar->IsVisible())
            continue;

        // The very secure search works as well, if there is no hashcode!
        sal_uInt16 nVarHash = rEntry.mpVar->GetHashCode();
        if ( (!nVarHash || nVarHash == nHash)
            && (t == SbxClassType::DontCare || rEntry.mpVar->GetClass() == t)
            && (rEntry.mpVar->GetName().equalsIgnoreAsciiCase(rName)))
        {
            p = rEntry.mpVar.get();
            p->ResetFlag(SbxFlagBits::ExtFound);
            break;
        }

        // Did we have an array/object with extended search?
        if (bExtSearch && rEntry.mpVar->IsSet(SbxFlagBits::ExtSearch))
        {
            switch (rEntry.mpVar->GetClass())
            {
                case SbxClassType::Object:
                {
                    // Objects are not allowed to scan their parent.
                    SbxFlagBits nOld = rEntry.mpVar->GetFlags();
                    rEntry.mpVar->ResetFlag(SbxFlagBits::GlobalSearch);
                    p = static_cast<SbxObject&>(*rEntry.mpVar).Find(rName, t);
                    rEntry.mpVar->SetFlags(nOld);
                }
                break;
                case SbxClassType::Array:
                    // Casting SbxVariable to SbxArray?  Really?
                    p = reinterpret_cast<SbxArray&>(*rEntry.mpVar).Find(rName, t);
                break;
                default:
                    ;
            }

            if (p)
            {
                p->SetFlag(SbxFlagBits::ExtFound);
                break;
            }
        }
    }
    return p;
}

bool SbxArray::LoadData( SvStream& rStrm, sal_uInt16 /*nVer*/ )
{
    sal_uInt16 nElem;
    Clear();
    bool bRes = true;
    SbxFlagBits f = nFlags;
    nFlags |= SbxFlagBits::Write;
    rStrm.ReadUInt16( nElem );
    nElem &= 0x7FFF;
    for( sal_uInt32 n = 0; n < nElem; n++ )
    {
        sal_uInt16 nIdx;
        rStrm.ReadUInt16( nIdx );
        SbxVariable* pVar = static_cast<SbxVariable*>(Load( rStrm ));
        if( pVar )
        {
            SbxVariableRef& rRef = GetRef( nIdx );
            rRef = pVar;
        }
        else
        {
            bRes = false;
            break;
        }
    }
    nFlags = f;
    return bRes;
}

bool SbxArray::StoreData( SvStream& rStrm ) const
{
    sal_uInt32 nElem = 0;
    // Which elements are even defined?
    for( auto& rEntry: mVarEntries )
    {
        if (rEntry.mpVar.is() && !(rEntry.mpVar->GetFlags() & SbxFlagBits::DontStore))
            nElem++;
    }
    rStrm.WriteUInt16( nElem );
    for( size_t n = 0; n < mVarEntries.size(); n++ )
    {
        const SbxVarEntry& rEntry = mVarEntries[n];
        if (rEntry.mpVar.is() && !(rEntry.mpVar->GetFlags() & SbxFlagBits::DontStore))
        {
            rStrm.WriteUInt16( n );
            if (!rEntry.mpVar->Store(rStrm))
                return false;
        }
    }
    return true;
}

// #100883 Method to set method directly to parameter array
void SbxArray::PutDirect( SbxVariable* pVar, sal_uInt32 nIdx )
{
    SbxVariableRef& rRef = GetRef32( nIdx );
    rRef = pVar;
}


//  SbxArray

SbxDimArray::SbxDimArray( SbxDataType t ) : SbxArray( t ), mbHasFixedSize( false )
{
}

SbxDimArray::SbxDimArray( const SbxDimArray& rArray )
    : SvRefBase( rArray ), SbxArray( rArray.eType )
{
    *this = rArray;
}

SbxDimArray& SbxDimArray::operator=( const SbxDimArray& rArray )
{
    if( &rArray != this )
    {
        SbxArray::operator=( (const SbxArray&) rArray );
        m_vDimensions = rArray.m_vDimensions;
        this->mbHasFixedSize = rArray.mbHasFixedSize;
    }
    return *this;
}

SbxDimArray::~SbxDimArray()
{
}

void SbxDimArray::Clear()
{
    m_vDimensions.clear();
    SbxArray::Clear();
}

// Add a dimension

void SbxDimArray::AddDimImpl32( sal_Int32 lb, sal_Int32 ub, bool bAllowSize0 )
{
    SbxError eRes = ERRCODE_SBX_OK;
    if( ub < lb && !bAllowSize0 )
    {
        eRes = ERRCODE_SBX_BOUNDS;
        ub = lb;
    }
    SbxDim d;
    d.nLbound = lb;
    d.nUbound = ub;
    d.nSize   = ub - lb + 1;
    m_vDimensions.push_back(d);
    if( eRes )
        SetError( eRes );
}


void SbxDimArray::AddDim( short lb, short ub )
{
    AddDimImpl32( lb, ub, false );
}

void SbxDimArray::unoAddDim( short lb, short ub )
{
    AddDimImpl32( lb, ub, true );
}

void SbxDimArray::AddDim32( sal_Int32 lb, sal_Int32 ub )
{
    AddDimImpl32( lb, ub, false );
}

void SbxDimArray::unoAddDim32( sal_Int32 lb, sal_Int32 ub )
{
    AddDimImpl32( lb, ub, true );
}


// Readout dimension data

bool SbxDimArray::GetDim32( sal_Int32 n, sal_Int32& rlb, sal_Int32& rub ) const
{
    if( n < 1 || n > static_cast<sal_Int32>(m_vDimensions.size()) )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        rub = rlb = 0;
        return false;
    }
    SbxDim d = m_vDimensions[n - 1];
    rub = d.nUbound;
    rlb = d.nLbound;
    return true;
}

bool SbxDimArray::GetDim( short n, short& rlb, short& rub ) const
{
    sal_Int32 rlb32, rub32;
    bool bRet = GetDim32( n, rlb32, rub32 );
    rub = (short)rub32;
    rlb = (short)rlb32;
    if( bRet )
    {
        if( rlb32 < -SBX_MAXINDEX || rub32 > SBX_MAXINDEX )
        {
            SetError( ERRCODE_SBX_BOUNDS );
            return false;
        }
    }
    return bRet;
}

// Element-Ptr with the help of an index list

sal_uInt32 SbxDimArray::Offset32( const sal_Int32* pIdx )
{
    sal_uInt32 nPos = 0;
    for( const auto& rDimension : m_vDimensions )
    {
        sal_Int32 nIdx = *pIdx++;
        if( nIdx < rDimension.nLbound || nIdx > rDimension.nUbound )
        {
            nPos = (sal_uInt32)SBX_MAXINDEX32 + 1; break;
        }
        nPos = nPos * rDimension.nSize + nIdx - rDimension.nLbound;
    }
    if( m_vDimensions.empty() || nPos > SBX_MAXINDEX32 )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        nPos = 0;
    }
    return nPos;
}

sal_uInt16 SbxDimArray::Offset( const short* pIdx )
{
    long nPos = 0;
    for( std::vector<SbxDim>::const_iterator it = m_vDimensions.begin();
         it != m_vDimensions.end(); ++it )
    {
        short nIdx = *pIdx++;
        if( nIdx < it->nLbound || nIdx > it->nUbound )
        {
            nPos = SBX_MAXINDEX + 1;
            break;
        }
        nPos = nPos * it->nSize + nIdx - it->nLbound;
    }
    if( m_vDimensions.empty() || nPos > SBX_MAXINDEX )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        nPos = 0;
    }
    return (sal_uInt16) nPos;
}

SbxVariable* SbxDimArray::Get( const short* pIdx )
{
    return SbxArray::Get( Offset( pIdx ) );
}

void SbxDimArray::Put( SbxVariable* p, const short* pIdx  )
{
    SbxArray::Put( p, Offset( pIdx ) );
}

SbxVariable* SbxDimArray::Get32( const sal_Int32* pIdx )
{
    return SbxArray::Get32( Offset32( pIdx ) );
}

void SbxDimArray::Put32( SbxVariable* p, const sal_Int32* pIdx  )
{
    SbxArray::Put32( p, Offset32( pIdx ) );
}

// Element-Number with the help of Parameter-Array
sal_uInt32 SbxDimArray::Offset32( SbxArray* pPar )
{
#if HAVE_FEATURE_SCRIPTING
    if (m_vDimensions.empty() || !pPar ||
        ((m_vDimensions.size() != sal::static_int_cast<size_t>(pPar->Count() - 1))
            && SbiRuntime::isVBAEnabled()))
    {
        SetError( ERRCODE_SBX_BOUNDS );
        return 0;
    }
#endif
    sal_uInt32 nPos = 0;
    sal_uInt16 nOff = 1;    // Non element 0!
    for( std::vector<SbxDim>::const_iterator it = m_vDimensions.begin();
         it != m_vDimensions.end() && !IsError(); ++it )
    {
        sal_Int32 nIdx = pPar->Get( nOff++ )->GetLong();
        if( nIdx < it->nLbound || nIdx > it->nUbound )
        {
            nPos = (sal_uInt32) SBX_MAXINDEX32+1;
            break;
        }
        nPos = nPos * it->nSize + nIdx - it->nLbound;
    }
    if( nPos > (sal_uInt32) SBX_MAXINDEX32 )
    {
        SetError( ERRCODE_SBX_BOUNDS );
        nPos = 0;
    }
    return nPos;
}

SbxVariable* SbxDimArray::Get( SbxArray* pPar )
{
    return SbxArray::Get32( Offset32( pPar ) );
}

bool SbxDimArray::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    short nDimension;
    rStrm.ReadInt16( nDimension );
    for( short i = 0; i < nDimension && rStrm.GetError() == SVSTREAM_OK; i++ )
    {
        sal_Int16 lb(0), ub(0);
        rStrm.ReadInt16( lb ).ReadInt16( ub );
        AddDim( lb, ub );
    }
    return SbxArray::LoadData( rStrm, nVer );
}

bool SbxDimArray::StoreData( SvStream& rStrm ) const
{
    rStrm.WriteInt16( m_vDimensions.size() );
    for( short i = 0; i < static_cast<short>(m_vDimensions.size()); i++ )
    {
        short lb, ub;
        GetDim( i, lb, ub );
        rStrm.WriteInt16( lb ).WriteInt16( ub );
    }
    return SbxArray::StoreData( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
