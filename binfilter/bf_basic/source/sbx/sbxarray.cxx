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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#include "sbx.hxx"
#include <vector>
using namespace std;

namespace binfilter {

struct SbxDim {                 // eine Array-Dimension:
    SbxDim* pNext;              // Link
    INT32 nLbound, nUbound;     // Begrenzungen
    INT32 nSize;                // Anzahl Elemente
};

class SbxVarEntry : public SbxVariableRef {
public:
    XubString* pAlias;
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

//////////////////////////////////////////////////////////////////////////
//
//  SbxArray
//
//////////////////////////////////////////////////////////////////////////

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
        for( UINT32 i = 0; i < pSrc->size(); i++ )
        {
            SbxVarEntryPtr pSrcRef = (*pSrc)[i];
            SbxVarEntryPtr pDstRef = new SbxVarEntry;
            *((SbxVariableRef*) pDstRef) = *((SbxVariableRef*) pSrcRef);
            if( pSrcRef->pAlias )
                pDstRef->pAlias = new XubString( *pSrcRef->pAlias );
            const SbxVariable* pSrc_ = *pSrcRef;
            if( pSrc_ )
            {
                if( eType != SbxVARIANT )
                    // Keine Objekte konvertieren
                    if( eType != SbxOBJECT || pSrc_->GetClass() != SbxCLASS_OBJECT )
                        ((SbxVariable*) pSrc_)->Convert( eType );
                pData->push_back( pDstRef );
            }
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
    UINT32 nSize = pData->size();
    for( UINT32 i = 0 ; i < nSize ; i++ )
    {
        SbxVarEntry* pEntry = (*pData)[i];
        delete pEntry;
    }
    pData->clear();
}

UINT32 SbxArray::Count32() const
{
    return pData->size();
}

USHORT SbxArray::Count() const
{
    UINT32 nCount = pData->size();
    DBG_ASSERT( nCount <= SBX_MAXINDEX, "SBX: Array-Index > SBX_MAXINDEX" );
    return (USHORT)nCount;
}

SbxVariableRef& SbxArray::GetRef32( UINT32 nIdx )
{
    // Array ggf. vergroessern
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

SbxVariableRef& SbxArray::GetRef( USHORT nIdx )
{
    // Array ggf. vergroessern
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

SbxVariable* SbxArray::Get32( UINT32 nIdx )
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

SbxVariable* SbxArray::Get( USHORT nIdx )
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

void SbxArray::Put32( SbxVariable* pVar, UINT32 nIdx )
{
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Keine Objekte konvertieren
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

void SbxArray::Put( SbxVariable* pVar, USHORT nIdx )
{
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else
    {
        if( pVar )
            if( eType != SbxVARIANT )
                // Keine Objekte konvertieren
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

void SbxArray::Insert32( SbxVariable* pVar, UINT32 nIdx )
{
    DBG_ASSERT( pData->size() <= SBX_MAXINDEX32, "SBX: Array wird zu gross" );
    if( pData->size() > SBX_MAXINDEX32 )
            return;
    SbxVarEntryPtr p = new SbxVarEntry;
    *((SbxVariableRef*) p) = pVar;
    SbxVarEntryPtrVector::size_type nSize = pData->size();
    if( nIdx > nSize )
        nIdx = nSize;
    if( eType != SbxVARIANT && pVar )
        (*p)->Convert( eType );
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

void SbxArray::Insert( SbxVariable* pVar, USHORT nIdx )
{
    DBG_ASSERT( pData->size() <= 0x3FF0, "SBX: Array wird zu gross" );
    if( pData->size() > 0x3FF0 )
            return;
    Insert32( pVar, nIdx );
}

void SbxArray::Remove32( UINT32 nIdx )
{
    if( nIdx < pData->size() )
    {
        SbxVariableRef* pRef = (*pData)[nIdx];
        pData->erase( pData->begin() + nIdx );
        delete pRef;
        SetFlag( SBX_MODIFIED );
    }
}

void SbxArray::Remove( USHORT nIdx )
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
        for( UINT32 i = 0; i < pData->size(); i++ )
        {
            SbxVariableRef* pRef = (*pData)[i];
            // SbxVariableRef* pRef = pData->GetObject( i );
            if( *pRef == pVar )
            {
                Remove32( i ); break;
            }
        }
    }
}

// Uebernahme der Daten aus dem uebergebenen Array, wobei
// gleichnamige Variable ueberschrieben werden.

void SbxArray::Merge( SbxArray* p )
{
    if( p )
    {
        UINT32 nSize = p->Count();
        for( UINT32 i = 0; i < nSize; i++ )
        {
            SbxVarEntryPtr pRef1 = (*(p->pData))[i];
            // Ist das Element by name schon drin?
            // Dann ueberschreiben!
            SbxVariable* pVar = *pRef1;
            if( pVar )
            {
                XubString aName = pVar->GetName();
                USHORT nHash = pVar->GetHashCode();
                for( UINT32 j = 0; j < pData->size(); j++ )
                {
                    SbxVariableRef* pRef2 = (*pData)[j];
                    if( (*pRef2)->GetHashCode() == nHash
                     && (*pRef2)->GetName().EqualsIgnoreCaseAscii( aName ) )
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
                        pRef->pAlias = new XubString( *pRef1->pAlias );
                }
            }
        }
    }
}

// Suchen eines Elements ueber die Userdaten. Falls ein Element
// ein Objekt ist, wird dieses ebenfalls durchsucht.

SbxVariable* SbxArray::FindUserData( UINT32 nData )
{
    SbxVariable* p = NULL;
    for( UINT32 i = 0; i < pData->size(); i++ )
    {
        SbxVariableRef* pRef = (*pData)[i];
        SbxVariable* pVar = *pRef;
        if( pVar )
        {
            if( pVar->IsVisible() && pVar->GetUserData() == nData )
            {
                p = pVar;
                p->ResetFlag( SBX_EXTFOUND );
                break;	// JSM 06.10.95
            }
            // Haben wir ein Array/Objekt mit Extended Search?
            else if( pVar->IsSet( SBX_EXTSEARCH ) )
            {
                switch( pVar->GetClass() )
                {
                    case SbxCLASS_OBJECT:
                    {
                        // Objekte duerfen ihren Parent nicht durchsuchen
                        USHORT nOld = pVar->GetFlags();
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

// Suchen eines Elements ueber den Namen und den Typ. Falls ein Element
// ein Objekt ist, wird dieses ebenfalls durchsucht.

SbxVariable* SbxArray::Find( const XubString& rName, SbxClassType t )
{
    SbxVariable* p = NULL;
    UINT32 nCount = pData->size();
    if( !nCount )
        return NULL;
    BOOL bExtSearch = IsSet( SBX_EXTSEARCH );
    USHORT nHash = SbxVariable::MakeHashCode( rName );
    for( UINT32 i = 0; i < nCount; i++ )
    {
        SbxVariableRef* pRef = (*pData)[i];
        SbxVariable* pVar = *pRef;
        if( pVar && pVar->IsVisible() )
        {
            // Die ganz sichere Suche klappt auch, wenn es
            // keinen Hascode gibt!
            USHORT nVarHash = pVar->GetHashCode();
            if( ( !nVarHash || nVarHash == nHash )
                && ( t == SbxCLASS_DONTCARE || pVar->GetClass() == t )
                && ( pVar->GetName().EqualsIgnoreCaseAscii( rName ) ) )
            {
                p = pVar;
                p->ResetFlag( SBX_EXTFOUND );
                break;
            }
            // Haben wir ein Array/Objekt mit Extended Search?
            else if( bExtSearch && pVar->IsSet( SBX_EXTSEARCH ) )
            {
                switch( pVar->GetClass() )
                {
                    case SbxCLASS_OBJECT:
                    {
                        // Objekte duerfen ihren Parent nicht durchsuchen
                        USHORT nOld = pVar->GetFlags();
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

BOOL SbxArray::LoadData( SvStream& rStrm, USHORT nVer )
{
    UINT16 nElem;
    Clear();
    BOOL bRes = TRUE;
    USHORT f = nFlags;
    nFlags |= SBX_WRITE;
    rStrm >> nElem;
    nElem &= 0x7FFF;
    for( UINT32 n = 0; n < nElem; n++ )
    {
        UINT16 nIdx;
        rStrm >> nIdx;
        SbxVariable* pVar = (SbxVariable*) Load( rStrm );
        if( pVar )
        {
            SbxVariableRef& rRef = GetRef( nIdx );
            rRef = pVar;
        }
        else
        {
            bRes = FALSE; break;
        }
    }
    if( bRes )
        bRes = LoadPrivateData( rStrm, nVer );
    nFlags = f;
    return bRes;
}

BOOL SbxArray::StoreData( SvStream& rStrm ) const
{
    UINT32 nElem = 0;
    UINT32 n;
    // Welche Elemente sind ueberhaupt definiert?
    for( n = 0; n < pData->size(); n++ )
    {
        SbxVariableRef* pRef = (*pData)[n];
        SbxVariable* p = *pRef;
        if( p && !( p->GetFlags() & SBX_DONTSTORE ) )
            nElem++;
    }
    rStrm << (UINT16) nElem;
    for( n = 0; n < pData->size(); n++ )
    {
        SbxVariableRef* pRef = (*pData)[n];
        SbxVariable* p = *pRef;
        if( p && !( p->GetFlags() & SBX_DONTSTORE ) )
        {
            rStrm << (UINT16) n;
            if( !p->Store( rStrm ) )
                return FALSE;
        }
    }
    return StorePrivateData( rStrm );
}

// #100883 Method to set method directly to parameter array
void SbxArray::PutDirect( SbxVariable* pVar, UINT32 nIdx )
{
    SbxVariableRef& rRef = GetRef32( nIdx );
    rRef = pVar;
}


//////////////////////////////////////////////////////////////////////////
//
//  SbxArray
//
//////////////////////////////////////////////////////////////////////////

SbxDimArray::SbxDimArray( SbxDataType t ) : SbxArray( t )
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

// Dimension hinzufuegen

void SbxDimArray::AddDimImpl32( INT32 lb, INT32 ub, BOOL bAllowSize0 )
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

void SbxDimArray::AddDim( short lb, short ub )
{
    AddDimImpl32( lb, ub, FALSE );
}

void SbxDimArray::unoAddDim( short lb, short ub )
{
    AddDimImpl32( lb, ub, TRUE );
}

void SbxDimArray::AddDim32( INT32 lb, INT32 ub )
{
    AddDimImpl32( lb, ub, FALSE );
}

void SbxDimArray::unoAddDim32( INT32 lb, INT32 ub )
{
    AddDimImpl32( lb, ub, TRUE );
}


// Dimensionsdaten auslesen

BOOL SbxDimArray::GetDim32( INT32 n, INT32& rlb, INT32& rub ) const
{
    if( n < 1 || n > nDim )
    {
        SetError( SbxERR_BOUNDS ); rub = rlb = 0; return FALSE;
    }
    SbxDim* p = pFirst;
    while( --n )
        p = p->pNext;
    rub = p->nUbound;
    rlb = p->nLbound;
    return TRUE;
}

BOOL SbxDimArray::GetDim( short n, short& rlb, short& rub ) const
{
    INT32 rlb32, rub32;
    BOOL bRet = GetDim32( n, rlb32, rub32 );
    if( bRet )
    {
        if( rlb32 < -SBX_MAXINDEX || rub32 > SBX_MAXINDEX )
        {
            SetError( SbxERR_BOUNDS );
            return FALSE;
        }
        rub = (short)rub32;
        rlb = (short)rlb32;
    }
    return bRet;
}

// Element-Ptr anhand einer Index-Liste

UINT32 SbxDimArray::Offset32( const INT32* pIdx )
{
    UINT32 nPos = 0;
    for( SbxDim* p = pFirst; p; p = p->pNext )
    {
        INT32 nIdx = *pIdx++;
        if( nIdx < p->nLbound || nIdx > p->nUbound )
        {
            nPos = (UINT32)SBX_MAXINDEX32 + 1; break;
        }
        nPos = nPos * p->nSize + nIdx - p->nLbound;
    }
    if( nDim == 0 || nPos > SBX_MAXINDEX32 )
    {
        SetError( SbxERR_BOUNDS ); nPos = 0;
    }
    return nPos;
}

USHORT SbxDimArray::Offset( const short* pIdx )
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
    return (USHORT) nPos;
}

SbxVariable* SbxDimArray::Get( const short* pIdx )
{
    return SbxArray::Get( Offset( pIdx ) );
}

SbxVariable* SbxDimArray::Get32( const INT32* pIdx )
{
    return SbxArray::Get32( Offset32( pIdx ) );
}

void SbxDimArray::Put32( SbxVariable* p, const INT32* pIdx  )
{
    SbxArray::Put32( p, Offset32( pIdx ) );
}


// Element-Nr anhand eines Parameter-Arrays

UINT32 SbxDimArray::Offset32( SbxArray* pPar )
{
    if( nDim == 0 || !pPar )
    {
        SetError( SbxERR_BOUNDS ); return 0;
    }
    UINT32 nPos = 0;
    USHORT nOff = 1;	// Nicht Element 0!
    for( SbxDim* p = pFirst; p && !IsError(); p = p->pNext )
    {
        INT32 nIdx = pPar->Get( nOff++ )->GetLong();
        if( nIdx < p->nLbound || nIdx > p->nUbound )
        {
            nPos = (UINT32) SBX_MAXINDEX32+1; break;
        }
        nPos = nPos * p->nSize + nIdx - p->nLbound;
    }
    if( nPos > (UINT32) SBX_MAXINDEX32 )
    {
        SetError( SbxERR_BOUNDS ); nPos = 0;
    }
    return nPos;
}

SbxVariable* SbxDimArray::Get( SbxArray* pPar )
{
    return SbxArray::Get32( Offset32( pPar ) );
}

BOOL SbxDimArray::LoadData( SvStream& rStrm, USHORT nVer )
{
    short nDimension;
    rStrm >> nDimension;
    for( short i = 0; i < nDimension && rStrm.GetError() == SVSTREAM_OK; i++ )
    {
        INT16 lb, ub;
        rStrm >> lb >> ub;
        AddDim( lb, ub );
    }
    return SbxArray::LoadData( rStrm, nVer );
}

BOOL SbxDimArray::StoreData( SvStream& rStrm ) const
{
    rStrm << (INT16) nDim;
    for( short i = 0; i < nDim; i++ )
    {
        short lb, ub;
        GetDim( i, lb, ub );
        rStrm << (INT16) lb << (INT16) ub;
    }
    return SbxArray::StoreData( rStrm );
}

}
