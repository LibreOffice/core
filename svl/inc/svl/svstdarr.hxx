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

/***********************************************************************
*
*   Die vordefinierte Arrays werden ueber POSITIV-Defines aktiviert:
*   (die defines setzen sich aus "_SVSTDARR_" und dem Namen des Array
*    ohne "Sv" zusammen)
*
*   VarArr:     SvULongs, SvUShorts
*   PtrArr:     SvStrings, SvStringsDtor
*   SortArr:    SvStringsSort, SvStringsSortDtor,
*               SvStringsISort, SvStringsISortDtor,
*               SvUShortsSort
***********************************************************************/

#include "svl/svldllapi.h"
#include <svl/svarray.hxx>
#include <deque>

#ifndef _SVSTDARR_ULONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvULongs, sal_uLong, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_ULONGS_DECL
#endif

#ifndef _SVSTDARR_ULONGSSORT_DECL
SV_DECL_VARARR_SORT_VISIBILITY( SvULongsSort, sal_uLong, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_ULONGSSORT_DECL
#endif

#ifndef _SVSTDARR_USHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvUShorts, sal_uInt16, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_USHORTS_DECL
#endif

#ifndef _SVSTDARR_USHORTSSORT_DECL

typedef sal_Bool (*FnForEach_SvUShortsSort)( const sal_uInt16&, void* );
class SVL_DLLPUBLIC SvUShortsSort : private SvUShorts
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

#ifndef _SVSTDARR_LONGSSORT_DECL
SV_DECL_VARARR_SORT_VISIBILITY( SvLongsSort, long, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_LONGSSORT_DECL
#endif

#include <tools/string.hxx>

typedef String* StringPtr;
typedef ByteString* ByteStringPtr;


#ifndef _SVSTDARR_STRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvStrings, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGS_DECL
#endif

#ifndef _SVSTDARR_STRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvStringsDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSDTOR_DECL
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsSort, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSSORT_DECL
#endif

#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsSortDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSSORTDTOR_DECL
#endif

#ifndef _SVSTDARR_STRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsISort, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSISORT_DECL
#endif

#ifndef _SVSTDARR_STRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsISortDtor, StringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSISORTDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvByteStrings, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGS_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvByteStringsDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsSort, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSSORT_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsSortDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsISort, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSISORT_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsISortDtor, ByteStringPtr, 1, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
#endif

typedef std::deque< xub_StrLen > SvXub_StrLens;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
