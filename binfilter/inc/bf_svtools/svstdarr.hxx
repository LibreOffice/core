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

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace binfilter
{

//#ifdef _SVSTDARR_BOOLS
#ifndef _SVSTDARR_BOOLS_DECL
SV_DECL_VARARR_VISIBILITY( SvBools, BOOL, 1, 1,  )
#define _SVSTDARR_BOOLS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTES
#ifndef _SVSTDARR_BYTES_DECL
SV_DECL_VARARR_VISIBILITY( SvBytes, BYTE, 1, 1,  )
#define _SVSTDARR_BYTES_DECL
#endif
//#endif

//#ifdef _SVSTDARR_ULONGS
#ifndef _SVSTDARR_ULONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvULongs, ULONG, 1, 1,  )
#define _SVSTDARR_ULONGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_USHORTS
#ifndef _SVSTDARR_USHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvUShorts, USHORT, 1, 1,  )
#define _SVSTDARR_USHORTS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_USHORTSSORT
#ifndef _SVSTDARR_USHORTSSORT_DECL

typedef BOOL (*FnForEach_SvUShortsSort)( const USHORT&, void* );
class  SvUShortsSort : __MWERKS__PRIVATE SvUShorts
{
public:
    SvUShortsSort(BYTE nSize = 1, BYTE nG = 1)
        : SvUShorts(nSize,nG) {}

    void Insert( const SvUShortsSort *pI, USHORT nS=0, USHORT nE=USHRT_MAX );
    BOOL Insert( const USHORT aE );
    BOOL Insert( const USHORT aE, USHORT& rP );
    void Insert( const USHORT *pE, USHORT nL );
    // remove ab dem Eintrag
    void Remove( const USHORT nP, USHORT nL = 1 );
    BOOL Seek_Entry( const USHORT aE, USHORT* pP = 0 ) const;

    USHORT Count() const  {   return SvUShorts::Count();	}
    const USHORT* GetData() const { return (const USHORT*)pData; }

    const USHORT& operator[](USHORT nP) const {
        return SvUShorts::operator[]( nP );
    }
    const USHORT& GetObject(USHORT nP) const {
        return SvUShorts::GetObject( nP );
    }
    void ForEach( FnForEach_SvUShortsSort fnForEach, void* pArgs = 0 )
    {
        _ForEach( 0, nA, (FnForEach_SvUShorts)fnForEach, pArgs );
    }
    void ForEach( USHORT nS, USHORT nE,
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
SV_DECL_VARARR_VISIBILITY( SvLongs, long, 1, 1,  )
#define _SVSTDARR_LONGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_SHORTS
#ifndef _SVSTDARR_SHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvShorts, short, 1, 1,  )
#define _SVSTDARR_SHORTS_DECL
#endif
//#endif

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
    (defined(_SVSTDARR_XUB_STRLEN) && !defined(_SVSTDARR_XUB_STRLEN_DECL))
*/


typedef String* StringPtr;
typedef ByteString* ByteStringPtr;

//#endif

//#ifdef _SVSTDARR_STRINGS
#ifndef _SVSTDARR_STRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvStrings, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSDTOR
#ifndef _SVSTDARR_STRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvStringsDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSSORT
#ifndef _SVSTDARR_STRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsSort, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSSORTDTOR
#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsSortDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSSORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSISORT
#ifndef _SVSTDARR_STRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsISort, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSISORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_STRINGSISORTDTOR
#ifndef _SVSTDARR_STRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsISortDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSISORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGS
#ifndef _SVSTDARR_BYTESTRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvByteStrings, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGS_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSDTOR
#ifndef _SVSTDARR_BYTESTRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvByteStringsDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSSORT
#ifndef _SVSTDARR_BYTESTRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsSort, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSSORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSSORTDTOR
#ifndef _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsSortDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSISORT
#ifndef _SVSTDARR_BYTESTRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsISort, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSISORT_DECL
#endif
//#endif

//#ifdef _SVSTDARR_BYTESTRINGSISORTDTOR
#ifndef _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsISortDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
#endif
//#endif

//#ifdef _SVSTDARR_XUB_STRLEN
#ifndef _SVSTDARR_XUB_STRLEN_DECL
SV_DECL_VARARR_VISIBILITY( SvXub_StrLens, xub_StrLen, 1, 1,  )
#define _SVSTDARR_XUB_STRLEN_DECL
#endif
//#endif

}
