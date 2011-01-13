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

#if 0
***********************************************************************
*
*   Die vordefinierte Arrays werden ueber POSITIV-Defines aktiviert:
*   (die defines setzen sich aus "_SVSTDARR_" und dem Namen des Array
*    ohne "Sv" zusammen)
*
*   VarArr:     SvBools, SvULongs, SvUShorts, SvLongs, SvShorts
*   PtrArr:     SvStrings, SvStringsDtor
*   SortArr:    SvStringsSort, SvStringsSortDtor,
*               SvStringsISort, SvStringsISortDtor,
*               SvUShortsSort
***********************************************************************
#endif

#include "svl/svldllapi.h"
#include <svl/svarray.hxx>
#include <deque>

typedef std::deque< sal_Bool > SvBools;

typedef std::deque< sal_uInt8 > SvBytes;

//#ifdef _SVSTDARR_ULONGS
#ifndef _SVSTDARR_ULONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvULongs, sal_uLong, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_ULONGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_ULONGSSORT
#ifndef _SVSTDARR_ULONGSSORT_DECL
SV_DECL_VARARR_SORT_VISIBILITY( SvULongsSort, sal_uLong, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_ULONGSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_USHORTS
#ifndef _SVSTDARR_USHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvUShorts, sal_uInt16, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_USHORTS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_USHORTSSORT
#ifndef _SVSTDARR_USHORTSSORT_DECL

typedef sal_Bool (*FnForEach_SvUShortsSort)( const sal_uInt16&, void* );
class SVL_DLLPUBLIC SvUShortsSort : __MWERKS__PRIVATE SvUShorts
{
public:
    SvUShortsSort(sal_uInt8 nSize = 1, sal_uInt8 nG = 1)
        : SvUShorts(nSize,nG) {}

    void Insert( const SvUShortsSort *pI, sal_uInt16 nS=0, sal_uInt16 nE=USHRT_MAX );
    sal_Bool Insert( const sal_uInt16 aE );
    sal_Bool Insert( const sal_uInt16 aE, sal_uInt16& rP );
    void Insert( const sal_uInt16 *pE, sal_uInt16 nL );
    // remove ab Pos
    void RemoveAt( const sal_uInt16 nP, sal_uInt16 nL = 1 );
    // remove ab dem Eintrag
    void Remove( const sal_uInt16 nP, sal_uInt16 nL = 1 );
    sal_Bool Seek_Entry( const sal_uInt16 aE, sal_uInt16* pP = 0 ) const;

    sal_uInt16 Count() const  {   return SvUShorts::Count();    }
    const sal_uInt16* GetData() const { return (const sal_uInt16*)pData; }

    const sal_uInt16& operator[](sal_uInt16 nP) const {
        return SvUShorts::operator[]( nP );
    }
    const sal_uInt16& GetObject(sal_uInt16 nP) const {
        return SvUShorts::GetObject( nP );
    }
    void ForEach( FnForEach_SvUShortsSort fnForEach, void* pArgs = 0 )
    {
        _ForEach( 0, nA, (FnForEach_SvUShorts)fnForEach, pArgs );
    }
    void ForEach( sal_uInt16 nS, sal_uInt16 nE,
                    FnForEach_SvUShortsSort fnForEach, void* pArgs = 0 )
    {
        _ForEach( nS, nE, (FnForEach_SvUShorts)fnForEach, pArgs );
    }
private:
    SvUShortsSort( const SvUShortsSort& );
    SvUShortsSort& operator=( const SvUShortsSort& );
};

#define _SVSTDARR_USHORTSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_LONGS
#ifndef _SVSTDARR_LONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvLongs, long, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_LONGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_LONGSSORT
#ifndef _SVSTDARR_LONGSSORT_DECL
SV_DECL_VARARR_SORT_VISIBILITY( SvLongsSort, long, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_LONGSSORT_DECL
#endif
//#endif

typedef std::deque< short > SvShorts;

/*
 form here all Arrays for Strings, ByteString and then
 len of a string (xub_StrLen)
#if (defined(_SVSTDARR_STRINGS) && !defined(_SVSTDARR_STRINGS_DECL)) || \
    (defined(_SVSTDARR_STRINGSDTOR) && !defined(_SVSTDARR_STRINGSDTOR_DECL)) || \
    (defined(_SVSTDARR_STRINGSSORT) && !defined(_SVSTDARR_STRINGSSORT_DECL)) || \
    (defined(_SVSTDARR_STRINGSSORTDTOR) && !defined(_SVSTDARR_STRINGSSORTDTOR_DECL)) || \
    (defined(_SVSTDARR_STRINGSISORT) && !defined(_SVSTDARR_STRINGSISORT_DECL)) || \
    (defined(_SVSTDARR_STRINGSISORTDTOR) && !defined(_SVSTDARR_STRINGSISORTDTOR_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGS) && !defined(_SVSTDARR_BYTESTRINGS_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGSDTOR) && !defined(_SVSTDARR_BYTESTRINGSDTOR_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGSSORT) && !defined(_SVSTDARR_BYTESTRINGSSORT_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGSSORTDTOR) && !defined(_SVSTDARR_BYTESTRINGSSORTDTOR_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGSISORT) && !defined(_SVSTDARR_BYTESTRINGSISORT_DECL)) || \
    (defined(_SVSTDARR_BYTESTRINGSISORTDTOR) && !defined(_SVSTDARR_BYTESTRINGSISORTDTOR_DECL)) || \
    (defined(_SVSTDARR_XUB_STRLEN) && !defined(_SVSTDARR_XUB_STRLEN_DECL)) ||\
    (defined(_SVSTDARR_XUB_STRLENSORT) && !defined(_SVSTDARR_XUB_STRLENSORT_DECL))
*/
#include <tools/string.hxx>

typedef String* StringPtr;
typedef ByteString* ByteStringPtr;

//#endif

//#ifdef _SVSTDARR_STRINGS
#ifndef _SVSTDARR_STRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvStrings, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSDTOR
#ifndef _SVSTDARR_STRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvStringsDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSSORT
#ifndef _SVSTDARR_STRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsSort, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSSORTDTOR
#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsSortDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSSORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSISORT
#ifndef _SVSTDARR_STRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsISort, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSISORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSISORTDTOR
#ifndef _SVSTDARR_STRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsISortDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSISORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGS
#ifndef _SVSTDARR_BYTESTRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvByteStrings, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSDTOR
#ifndef _SVSTDARR_BYTESTRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvByteStringsDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSSORT
#ifndef _SVSTDARR_BYTESTRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsSort, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSSORTDTOR
#ifndef _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsSortDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSISORT
#ifndef _SVSTDARR_BYTESTRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsISort, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSISORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSISORTDTOR
#ifndef _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsISortDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
#endif
//#endif

typedef std::deque< xub_StrLen > SvXub_StrLens;

