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

#include <tools/stream.hxx>
#include <basic/sbx.hxx>
#include "runtime.hxx"
#include <vector>
using namespace std;

struct SbxDim {                 // an array-dimension:
    SbxDim* pNext;              // Link
    sal_Int32 nLbound, nUbound; // Limitations
    sal_Int32 nSize;            // Number of elements
};

class SbxVarEntry : public SbxVariableRef {
public:
    OUString* pAlias;
    SbxVarEntry() : SbxVariableRef(), pAlias( NULL ) {}
   ~SbxVarEntry() { delete pAlias; }
};

typedef SbxVarEntry* SbxVarEntryPtr;
typedef vector< SbxVarEntryPtr > SbxVarEntryPtrVector;
class SbxVarRefs : public SbxVarEntryPtrVector
{
public:
    SbxVarRefs( void ) {}
};


TYPEINIT1(SbxArray,SbxBase)
TYPEINIT1(SbxDimArray,SbxArray)

//  SbxArray

SbxArray::SbxArray( SbxDataType t ) : SbxBase()
{
    pData = new SbxVarRefs;
    eType = t;
    if( t != SbxVARIANT )
        SetFlag( SBX_FIXED );
}

SbxArray::SbxArray( const SbxArray& rArray ) :
    SvRefBase( rArray ), SbxBase()
{
    pData = new SbxVarRefs;
    if( rArray.eType != SbxVARIANT )
        SetFlag( SBX_FIXED );
    *this = rArray;
}

SbxArray& SbxArray::operator=( const SbxArray& rArray )
{
    if( &rArray != this )
    {
        eType = rArray.eType;
        Clear();
        SbxVarRefs* pSrc = rArray.pData;
        for( sal_uInt32 i = 0; i < pSrc->size(); i++ )
        {
            SbxVarEntryPtr pSrcRef = (*pSrc)[i];
            const SbxVariable* pSrc_ = *pSrcRef;
            if( !pSrc_ )
                continue;
            SbxVarEntryPtr pDstRef = new SbxVarEntry;
            *((SbxVariableRef*) pDstRef) = *((SbxVariableRef*) pSrcRef);
            if( pSrcRef->pAlias )
            {
                pDstRef->pAlias = new OUString( *pSrcRef->pAlias );
            }
            if( eType != SbxVARIANT )
            {
                // Convert no objects
                if( eType != SbxOBJECT || pSrc_->GetClass() != SbxCLASS_OBJECT )
                {
                    ((SbxVariable*) pSrc_)->Convert( eType );
                }
            }
            pData->push_back( pDstRef );
        }
    }
    return *this;
}

SbxArray::~SbxArray()
{
    Clear();
    delete pData;
}

SbxDataType SbxArray::GetType() const
{
    return (SbxDataType) ( eType | SbxARRAY );
}

SbxClassType SbxArray::GetClass() const
{
    return SbxCLASS_ARRAY;
}

void SbxArray::Clear()
{
    sal_uInt32 nSize = pData->size();
    for( sal_uInt32 i = 0 ; i < nSize ; i++ )
    {
        SbxVarEntry* pEntry = (*pData)[i];
        delete pEntry;
    }
    pData->clear();
}

sal_uInt32 SbxArray::Count32() const
{
    return pData->size();
}

sal_uInt16 SbxArray::Count() const
{
    sal_uInt32 nCount = pData->size();
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
        SetError( SbxERR_BOUNDS );
        nIdx = 0;
    }
    while( pData->size() <= nIdx )
    {
        const SbxVarEntryPtr p = new SbxVarEntry;
        pData->push_back( p );
    }
    return *((*pData)[nIdx]);
}

SbxVariableRef& SbxArray::GetRef( sal_uInt16 nIdx )
{
    // If necessary extend the array
    DBG_ASSERT( nIdx <= SBX_MAXINDEX, "SBX: Array-Index > SBX_MAXINDEX" );
    // Very Hot Fix
    if( nIdx > SBX_MAXINDEX )
    {
        SetError( SbxERR_BOUNDS );
        nIdx = 0;
    }
    while( pData->size() <= nIdx )
    {
        const SbxVarEntryPtr p = new SbxVarEntry;
        pData->push_back( p );
    }
    return *((*pData)[nIdx]);
}

SbxVariable* SbxArray::Get32( sal_uInt32 nIdx )
{
    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY );
        return NULL;
    }
    SbxVariableRef& rRef = GetRef32( nIdx );

    if ( !rRef.Is() )
        rRef = new SbxVariable( eType );

    return rRef;
}

SbxVariable* SbxArray::Get( sal_uInt16 nIdx )
{
    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY );
        return NULL;
    }
    SbxVariableRef& rRef = GetRef( nIdx );

    if ( !rRef.Is() )
        rRef = new SbxVariable( eType );

    return rRef;
}

void SbxArray::Put32( SbxVariable* pVar, sal_uInt32 nIdx )
{
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Convert no objects
                if( eType != SbxOBJECT || pVar->GetClass() != SbxCLASS_OBJECT )
                    pVar->Convert( eType );
        SbxVariableRef& rRef = GetRef32( nIdx );
        if( (SbxVariable*) rRef != pVar )
        {
            rRef = pVar;
            SetFlag( SBX_MODIFIED );
        }
    }
}

void SbxArray::Put( SbxVariable* pVar, sal_uInt16 nIdx )
{
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Convert no objects
                if( eType != SbxOBJECT || pVar->GetClass() != SbxCLASS_OBJECT )
                    pVar->Convert( eType );
        SbxVariableRef& rRef = GetRef( nIdx );
        if( (SbxVariable*) rRef != pVar )
        {
            rRef = pVar;
            SetFlag( SBX_MODIFIED );
        }
    }
}

const OUString& SbxArray::GetAlias( sal_uInt16 nIdx )
{
static const OUString sEmpty("");

    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY );
        return sEmpty;
    }
    SbxVarEntry& rRef = (SbxVarEntry&) GetRef( nIdx );

    if ( !rRef.pAlias )
    {
        return sEmpty;
    }

    return *rRef.pAlias;
}

void SbxArray::PutAlias( const OUString& rAlias, sal_uInt16 nIdx )
{
    if( !CanWrite() )
    {
        SetError( SbxERR_PROP_READONLY );
    }
    else
    {
        SbxVarEntry& rRef = (SbxVarEntry&) GetRef( nIdx );
        if( !rRef.pAlias )
        {
            rRef.pAlias = new OUString( rAlias );
        }
        else
        {
            *rRef.pAlias = rAlias;
        }
    }
}

void SbxArray::Insert32( SbxVariable* pVar, sal_uInt32 nIdx )
{
    DBG_ASSERT( pData->size() <= SBX_MAXINDEX32, "SBX: Array gets too big" );
    if( pData->size() > SBX_MAXINDEX32 )
    {
            return;
    }
    SbxVarEntryPtr p = new SbxVarEntry;
    *((SbxVariableRef*) p) = pVar;
    SbxVarEntryPtrVector::size_type nSize = pData->size();
    if( nIdx > nSize )
    {
        nIdx = nSize;
    }
    if( eType != SbxVARIANT && pVar )
    {
        (*p)->Convert( eType );
    }
    if( nIdx == nSize )
    {
        pData->push_back( p );
    }
    else
    {
        pData->insert( pData->begin() + nIdx, p );
    }
    SetFlag( SBX_MODIFIED );
}

void SbxArray::Insert( SbxVariable* pVar, sal_uInt16 nIdx )
{
    DBG_ASSERT( pData->size() <= 0x3FF0, "SBX: Array gets too big" );
    if( pData->size() > 0x3FF0 )
    {
        return;
    }
    Insert32( pVar, nIdx );
}

void SbxArray::Remove32( sal_uInt32 nIdx )
{
    if( nIdx < pData->size() )
    {
        SbxVariableRef* pRef = (*pData)[nIdx];
        pData->erase( pData->begin() + nIdx );
        delete pRef;
        SetFlag( SBX_MODIFIED );
    }
}

void SbxArray::Remove( sal_uInt16 nIdx )
{
    if( nIdx < pData->size() )
    {
        SbxVariableRef* pRef = (*pData)[nIdx];
        pData->erase( pData->begin() + nIdx );
        delete pRef;
        SetFlag( SBX_MODIFIED );
    }
}

void SbxArray::Remove( SbxVariable* pVar )
{
    if( pVar )
    {
        for( sal_uInt32 i = 0; i < pData->size(); i++ )
        {
            SbxVariableRef* pRef = (*pData)[i];
            if( *pRef == pVar )
            {
                Remove32( i ); break;
            }
        }
    }
}

// Taking over of the data from the passed array, at which
// the variable of the same name will be overwritten.

void SbxArray::Merge( SbxArray* p )
{
    if( p )
    {
        sal_uInt32 nSize = p->Count();
        for( sal_uInt32 i = 0; i < nSize; i++ )
        {
            SbxVarEntryPtr pRef1 = (*(p->pData))[i];
            // Is the element by name already inside?
            // Then overwrite!
            SbxVariable* pVar = *pRef1;
            if( pVar )
            {
                OUString aName = pVar->GetName();
                sal_uInt16 nHash = pVar->GetHashCode();
                for( sal_uInt32 j = 0; j < pData->size(); j++ )
                {
                    SbxVariableRef* pRef2 = (*pData)[j];
                    if( (*pRef2)->GetHashCode() == nHash
                        && (*pRef2)->GetName().equalsIgnoreAsciiCase( aName ) )
                    {
                        *pRef2 = pVar; pRef1 = NULL;
                        break;
                    }
                }
                if( pRef1 )
                {
                    SbxVarEntryPtr pRef = new SbxVarEntry;
                    const SbxVarEntryPtr pTemp = pRef;
                    pData->push_back( pTemp );
                    *((SbxVariableRef*) pRef) = *((SbxVariableRef*) pRef1);
                    if( pRef1->pAlias )
                    {
                        pRef->pAlias = new OUString( *pRef1->pAlias );
                    }
                }
            }
        }
    }
}

// Search of an element via the user data. If the element is
// object, it will also be scanned.

SbxVariable* SbxArray::FindUserData( sal_uInt32 nData )
{
    SbxVariable* p = NULL;
    for( sal_uInt32 i = 0; i < pData->size(); i++ )
    {
        SbxVariableRef* pRef = (*pData)[i];
        SbxVariable* pVar = *pRef;
        if( pVar )
        {
            if( pVar->IsVisible() && pVar->GetUserData() == nData )
            {
                p = pVar;
                p->ResetFlag( SBX_EXTFOUND );
                break;  // JSM 1995-10-06
            }
            // Did we have an array/object with extended search?
            else if( pVar->IsSet( SBX_EXTSEARCH ) )
            {
                switch( pVar->GetClass() )
                {
                    case SbxCLASS_OBJECT:
                    {
                        // Objects are not allowed to scan their parent.
                        sal_uInt16 nOld = pVar->GetFlags();
                        pVar->ResetFlag( SBX_GBLSEARCH );
                        p = ((SbxObject*) pVar)->FindUserData( nData );
                        pVar->SetFlags( nOld );
                        break;
                    }
                    case SbxCLASS_ARRAY:
                        p = ((SbxArray*) pVar)->FindUserData( nData );
                        break;
                    default: break;
                }
                if( p )
                {
                    p->SetFlag( SBX_EXTFOUND );
                    break;
                }
            }
        }
    }
    return p;
}

// Search of an element by his name and type. If an element is an object,
// it will also be scanned..

SbxVariable* SbxArray::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* p = NULL;
    sal_uInt32 nCount = pData->size();
    if( !nCount )
        return NULL;
    bool bExtSearch = IsSet( SBX_EXTSEARCH );
    sal_uInt16 nHash = SbxVariable::MakeHashCode( rName );
    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SbxVariableRef* pRef = (*pData)[i];
        SbxVariable* pVar = *pRef;
        if( pVar && pVar->IsVisible() )
        {
            // The very secure search works as well, if there is no hashcode!
            sal_uInt16 nVarHash = pVar->GetHashCode();
            if( ( !nVarHash || nVarHash == nHash )
                && ( t == SbxCLASS_DONTCARE || pVar->GetClass() == t )
                && ( pVar->GetName().equalsIgnoreAsciiCase( rName ) ) )
            {
                p = pVar;
                p->ResetFlag( SBX_EXTFOUND );
                break;
            }
            // Did we have an array/object with extended search?
            else if( bExtSearch && pVar->IsSet( SBX_EXTSEARCH ) )
            {
                switch( pVar->GetClass() )
                {
                    case SbxCLASS_OBJECT:
                    {
                        // Objects are not allowed to scan their parent.
                        sal_uInt16 nOld = pVar->GetFlags();
                        pVar->ResetFlag( SBX_GBLSEARCH );
                        p = ((SbxObject*) pVar)->Find( rName, t );
                        pVar->SetFlags( nOld );
                        break;
                    }
                    case SbxCLASS_ARRAY:
                        p = ((SbxArray*) pVar)->Find( rName, t );
                        break;
                    default: break;
                }
                if( p )
                {
                    p->SetFlag( SBX_EXTFOUND );
                    break;
                }
            }
        }
    }
    return p;
}

bool SbxArray::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
    sal_uInt16 nElem;
    Clear();
    bool bRes = true;
    sal_uInt16 f = nFlags;
    nFlags |= SBX_WRITE;
    rStrm.ReadUInt16( nElem );
    nElem &= 0x7FFF;
    for( sal_uInt32 n = 0; n < nElem; n++ )
    {
        sal_uInt16 nIdx;
        rStrm.ReadUInt16( nIdx );
        SbxVariable* pVar = (SbxVariable*) Load( rStrm );
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
    if( bRes )
        bRes = LoadPrivateData( rStrm, nVer );
    nFlags = f;
    return bRes;
}

bool SbxArray::StoreData( SvStream& rStrm ) const
{
    sal_uInt32 nElem = 0;
    sal_uInt32 n;
    // Which elements are even defined?
    for( n = 0; n < pData->size(); n++ )
    {
        SbxVariableRef* pRef = (*pData)[n];
        SbxVariable* p = *pRef;
        if( p && !( p->GetFlags() & SBX_DONTSTORE ) )
            nElem++;
    }
    rStrm.WriteUInt16( (sal_uInt16) nElem );
    for( n = 0; n < pData->size(); n++ )
    {
        SbxVariableRef* pRef = (*pData)[n];
        SbxVariable* p = *pRef;
        if( p && !( p->GetFlags() & SBX_DONTSTORE ) )
        {
            rStrm.WriteUInt16( (sal_uInt16) n );
            if( !p->Store( rStrm ) )
                return false;
        }
    }
    return StorePrivateData( rStrm );
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
    pFirst = pLast = NULL;
    nDim = 0;
}

SbxDimArray::SbxDimArray( const SbxDimArray& rArray )
    : SvRefBase( rArray ), SbxArray( rArray.eType )
{
    pFirst = pLast = NULL;
    nDim = 0;
    *this = rArray;
}

SbxDimArray& SbxDimArray::operator=( const SbxDimArray& rArray )
{
    if( &rArray != this )
    {
        SbxArray::operator=( (const SbxArray&) rArray );
        SbxDim* p = rArray.pFirst;
        while( p )
        {
            AddDim32( p->nLbound, p->nUbound );
            p = p->pNext;
        }
        this->mbHasFixedSize = rArray.mbHasFixedSize;
    }
    return *this;
}

SbxDimArray::~SbxDimArray()
{
    Clear();
}

void SbxDimArray::Clear()
{
    SbxDim* p = pFirst;
    while( p )
    {
        SbxDim* q = p->pNext;
        delete p;
        p = q;
    }
    pFirst = pLast = NULL;
    nDim   = 0;
}

// Add a dimension

void SbxDimArray::AddDimImpl32( sal_Int32 lb, sal_Int32 ub, bool bAllowSize0 )
{
    SbxError eRes = SbxERR_OK;
    if( ub < lb && !bAllowSize0 )
    {
        eRes = SbxERR_BOUNDS;
        ub = lb;
    }
    SbxDim* p = new SbxDim;
    p->nLbound = lb;
    p->nUbound = ub;
    p->nSize   = ub - lb + 1;
    p->pNext   = NULL;
    if( !pFirst )
        pFirst = pLast = p;
    else
        pLast->pNext = p, pLast = p;
    nDim++;
    if( eRes )
        SetError( eRes );
}

short SbxDimArray::GetDims() const
{
    return nDim;
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
    if( n < 1 || n > nDim )
    {
        SetError( SbxERR_BOUNDS );
        rub = rlb = 0;
        return false;
    }
    SbxDim* p = pFirst;
    while( --n )
        p = p->pNext;
    rub = p->nUbound;
    rlb = p->nLbound;
    return true;
}

bool SbxDimArray::GetDim( short n, short& rlb, short& rub ) const
{
    sal_Int32 rlb32, rub32;
    bool bRet = GetDim32( n, rlb32, rub32 );
    if( bRet )
    {
        if( rlb32 < -SBX_MAXINDEX || rub32 > SBX_MAXINDEX )
        {
            SetError( SbxERR_BOUNDS );
            return false;
        }
        rub = (short)rub32;
        rlb = (short)rlb32;
    }
    return bRet;
}

// Element-Ptr with the help of an index list

sal_uInt32 SbxDimArray::Offset32( const sal_Int32* pIdx )
{
    sal_uInt32 nPos = 0;
    for( SbxDim* p = pFirst; p; p = p->pNext )
    {
        sal_Int32 nIdx = *pIdx++;
        if( nIdx < p->nLbound || nIdx > p->nUbound )
        {
            nPos = (sal_uInt32)SBX_MAXINDEX32 + 1; break;
        }
        nPos = nPos * p->nSize + nIdx - p->nLbound;
    }
    if( nDim == 0 || nPos > SBX_MAXINDEX32 )
    {
        SetError( SbxERR_BOUNDS ); nPos = 0;
    }
    return nPos;
}

sal_uInt16 SbxDimArray::Offset( const short* pIdx )
{
    long nPos = 0;
    for( SbxDim* p = pFirst; p; p = p->pNext )
    {
        short nIdx = *pIdx++;
        if( nIdx < p->nLbound || nIdx > p->nUbound )
        {
            nPos = SBX_MAXINDEX + 1; break;
        }
        nPos = nPos * p->nSize + nIdx - p->nLbound;
    }
    if( nDim == 0 || nPos > SBX_MAXINDEX )
    {
        SetError( SbxERR_BOUNDS ); nPos = 0;
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
#ifndef DISABLE_SCRIPTING
    if( nDim == 0 || !pPar || ( ( nDim != ( pPar->Count() - 1 ) ) && SbiRuntime::isVBAEnabled() ) )
    {
        SetError( SbxERR_BOUNDS ); return 0;
    }
#endif
    sal_uInt32 nPos = 0;
    sal_uInt16 nOff = 1;    // Non element 0!
    for( SbxDim* p = pFirst; p && !IsError(); p = p->pNext )
    {
        sal_Int32 nIdx = pPar->Get( nOff++ )->GetLong();
        if( nIdx < p->nLbound || nIdx > p->nUbound )
        {
            nPos = (sal_uInt32) SBX_MAXINDEX32+1; break;
        }
        nPos = nPos * p->nSize + nIdx - p->nLbound;
    }
    if( nPos > (sal_uInt32) SBX_MAXINDEX32 )
    {
        SetError( SbxERR_BOUNDS ); nPos = 0;
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
        sal_Int16 lb, ub;
        rStrm.ReadInt16( lb ).ReadInt16( ub );
        AddDim( lb, ub );
    }
    return SbxArray::LoadData( rStrm, nVer );
}

bool SbxDimArray::StoreData( SvStream& rStrm ) const
{
    rStrm.WriteInt16( (sal_Int16) nDim );
    for( short i = 0; i < nDim; i++ )
    {
        short lb, ub;
        GetDim( i, lb, ub );
        rStrm.WriteInt16( (sal_Int16) lb ).WriteInt16( (sal_Int16) ub );
    }
    return SbxArray::StoreData( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
