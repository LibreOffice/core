/*************************************************************************
 *
 *  $RCSfile: svarray.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-20 18:31:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#include <svstdarr.hxx>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifdef MAC
#pragma segment NEVER_PURGE
#endif

SV_IMPL_VARARR(SvPtrarr,VoidPtr)
SV_IMPL_VARARR_PLAIN(SvPtrarrPlain,VoidPtr)

USHORT SvPtrarr::GetPos( const VoidPtr& aElement ) const
{   USHORT n;
    for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
    return ( n >= nA ? USHRT_MAX : n );
}

USHORT SvPtrarrPlain::GetPos( const VoidPtr aElement ) const
{   USHORT n;
    for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
    return ( n >= nA ? USHRT_MAX : n );
}


SV_IMPL_VARARR( SvBools, BOOL )
SV_IMPL_VARARR( SvBytes, BYTE )
SV_IMPL_VARARR( SvULongs, ULONG )
SV_IMPL_VARARR( SvUShorts, USHORT )
SV_IMPL_VARARR( SvLongs, long)
SV_IMPL_VARARR( SvShorts, short )

SV_IMPL_VARARR_SORT( SvULongsSort, ULONG )
SV_IMPL_VARARR_SORT( SvLongsSort, long )
SV_IMPL_VARARR_SORT( SvXub_StrLensSort, xub_StrLen )

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
    BOOL bExist;
    if( ! ( bExist = Seek_Entry( aE, &nP ) ) )
        SvUShorts::Insert( aE, nP );
    return !bExist;
}

BOOL SvUShortsSort::Insert( const USHORT aE, USHORT& rP )
{
    BOOL bExist;
    if( ! ( bExist = Seek_Entry( aE, &rP ) ) )
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

// remove ab Pos
void SvUShortsSort::RemoveAt( const USHORT nP, USHORT nL )
{
    if( nL )
        SvUShorts::Remove( nP, nL);
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

