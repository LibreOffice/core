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


#define _SVARRAY_CXX

#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGSSORT
#define _SVSTDARR_USHORTS
#define _SVSTDARR_LONGS
#define _SVSTDARR_LONGSSORT
#define _SVSTDARR_SHORTS
#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSSORT
#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSISORT
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_USHORTSSORT

#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_BYTESTRINGSDTOR
#define _SVSTDARR_BYTESTRINGSSORT
#define _SVSTDARR_BYTESTRINGSSORTDTOR
#define _SVSTDARR_BYTESTRINGSISORT
#define _SVSTDARR_BYTESTRINGSISORTDTOR

#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_XUB_STRLENSORT

#include <bf_svtools/svstdarr.hxx>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

namespace binfilter
{

SV_IMPL_VARARR(SvPtrarr,VoidPtr)

USHORT SvPtrarr::GetPos( const VoidPtr& aElement ) const
{	USHORT n;
    for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
    return ( n >= nA ? USHRT_MAX : n );
}

SV_IMPL_VARARR( SvBools, BOOL )
SV_IMPL_VARARR( SvBytes, BYTE )
SV_IMPL_VARARR( SvULongs, ULONG )
SV_IMPL_VARARR( SvUShorts, USHORT )
SV_IMPL_VARARR( SvLongs, long)
SV_IMPL_VARARR( SvShorts, short )

SV_IMPL_VARARR( SvXub_StrLens, xub_StrLen )

SV_IMPL_PTRARR( SvStrings, StringPtr )
SV_IMPL_PTRARR( SvStringsDtor, StringPtr )
SV_IMPL_OP_PTRARR_SORT( SvStringsSort, StringPtr )
SV_IMPL_OP_PTRARR_SORT( SvStringsSortDtor, StringPtr )

SV_IMPL_PTRARR( SvByteStrings, ByteStringPtr )
SV_IMPL_PTRARR( SvByteStringsDtor, ByteStringPtr )
SV_IMPL_OP_PTRARR_SORT( SvByteStringsSort, ByteStringPtr )
SV_IMPL_OP_PTRARR_SORT( SvByteStringsSortDtor, ByteStringPtr )



// ---------------- strings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvStringsISort, StringPtr )
void SvStringsISort::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( USHORT n=nP; n < nP + nL; n++ )
            delete *((StringPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}
BOOL SvStringsISort::Seek_Entry( const StringPtr aE, USHORT* pP ) const
{
    register USHORT nO  = SvStringsISort_SAR::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = (*((StringPtr*)pData + nM))->
                                        CompareIgnoreCaseToAscii( *(aE) );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_LESS == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

// ---------------- strings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvStringsISortDtor, StringPtr )
void SvStringsISortDtor::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( USHORT n=nP; n < nP + nL; n++ )
            delete *((StringPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}
BOOL SvStringsISortDtor::Seek_Entry( const StringPtr aE, USHORT* pP ) const
{
    register USHORT nO  = SvStringsISortDtor_SAR::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = (*((StringPtr*)pData + nM))->
                                    CompareIgnoreCaseToAscii( *(aE) );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_LESS == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

// ---------------- Ushorts -------------------------------------

/* SortArray fuer UShorts */
BOOL SvUShortsSort::Seek_Entry( const USHORT aE, USHORT* pP ) const
{
    register USHORT nO  = SvUShorts::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( *(pData + nM) == aE )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( *(pData + nM) < aE )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

void SvUShortsSort::Insert( const SvUShortsSort * pI, USHORT nS, USHORT nE )
{
    if( USHRT_MAX == nE )
        nE = pI->Count();
    USHORT nP;
    const USHORT * pIArr = pI->GetData();
    for( ; nS < nE; ++nS )
    {
        if( ! Seek_Entry( *(pIArr+nS), &nP) )
                SvUShorts::Insert( *(pIArr+nS), nP );
        if( ++nP >= Count() )
        {
            SvUShorts::Insert( pI, nP, nS+1, nE );
            nS = nE;
        }
    }
}

BOOL SvUShortsSort::Insert( const USHORT aE )
{
    USHORT nP;
    BOOL bExist = Seek_Entry( aE, &nP );
    if( !bExist )
        SvUShorts::Insert( aE, nP );
    return !bExist;
}

BOOL SvUShortsSort::Insert( const USHORT aE, USHORT& rP )
{
    BOOL bExist = Seek_Entry( aE, &rP );
    if( !bExist )
        SvUShorts::Insert( aE, rP );
    return !bExist;
}

void SvUShortsSort::Insert( const USHORT* pE, USHORT nL)
{
    USHORT nP;
    for( USHORT n = 0; n < nL; ++n )
        if( ! Seek_Entry( *(pE+n), &nP ))
            SvUShorts::Insert( *(pE+n), nP );
}

// remove ab dem Eintrag
void SvUShortsSort::Remove( const USHORT aE, USHORT nL )
{
    USHORT nP;
    if( nL && Seek_Entry( aE, &nP ) )
        SvUShorts::Remove( nP, nL);
}

// ---------------- bytestrings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvByteStringsISort, ByteStringPtr )
void SvByteStringsISort::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( USHORT n=nP; n < nP + nL; n++ )
            delete *((ByteStringPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}
BOOL SvByteStringsISort::Seek_Entry( const ByteStringPtr aE, USHORT* pP ) const
{
    register USHORT nO  = SvByteStringsISort_SAR::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = (*((ByteStringPtr*)pData + nM))->
                        CompareIgnoreCaseToAscii( *(aE) );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_LESS == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}


// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvByteStringsISortDtor, ByteStringPtr )
void SvByteStringsISortDtor::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( USHORT n=nP; n < nP + nL; n++ )
            delete *((ByteStringPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}
BOOL SvByteStringsISortDtor::Seek_Entry( const ByteStringPtr aE, USHORT* pP ) const
{
    register USHORT nO  = SvByteStringsISortDtor_SAR::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = (*((ByteStringPtr*)pData + nM))->
                                    CompareIgnoreCaseToAscii( *(aE) );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return TRUE;
            }
            else if( COMPARE_LESS == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return FALSE;
}

}
