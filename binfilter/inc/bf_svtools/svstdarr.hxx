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

#include <bf_svtools/svarray.hxx>

#include <tools/string.hxx>

namespace binfilter
{

#ifndef _SVSTDARR_BOOLS_DECL
SV_DECL_VARARR_VISIBILITY( SvBools, BOOL, 1, 1,  )
#define _SVSTDARR_BOOLS_DECL
#endif

#ifndef _SVSTDARR_BYTES_DECL
SV_DECL_VARARR_VISIBILITY( SvBytes, BYTE, 1, 1,  )
#define _SVSTDARR_BYTES_DECL
#endif

#ifndef _SVSTDARR_ULONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvULongs, ULONG, 1, 1,  )
#define _SVSTDARR_ULONGS_DECL
#endif

#ifndef _SVSTDARR_USHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvUShorts, USHORT, 1, 1,  )
#define _SVSTDARR_USHORTS_DECL
#endif

#ifndef _SVSTDARR_USHORTSSORT_DECL

typedef BOOL (*FnForEach_SvUShortsSort)( const USHORT&, void* );
class  SvUShortsSort : private SvUShorts
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

#ifndef _SVSTDARR_LONGS_DECL
SV_DECL_VARARR_VISIBILITY( SvLongs, long, 1, 1,  )
#define _SVSTDARR_LONGS_DECL
#endif

#ifndef _SVSTDARR_SHORTS_DECL
SV_DECL_VARARR_VISIBILITY( SvShorts, short, 1, 1,  )
#define _SVSTDARR_SHORTS_DECL
#endif

typedef String* StringPtr;
typedef ByteString* ByteStringPtr;


#ifndef _SVSTDARR_STRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvStrings, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGS_DECL
#endif

#ifndef _SVSTDARR_STRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvStringsDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSDTOR_DECL
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsSort, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSSORT_DECL
#endif

#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsSortDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSSORTDTOR_DECL
#endif

#ifndef _SVSTDARR_STRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvStringsISort, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSISORT_DECL
#endif

#ifndef _SVSTDARR_STRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsISortDtor, StringPtr, 1, 1,  )
#define _SVSTDARR_STRINGSISORTDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGS_DECL
SV_DECL_PTRARR_VISIBILITY( SvByteStrings, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGS_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSDTOR_DECL
SV_DECL_PTRARR_DEL_VISIBILITY( SvByteStringsDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSSORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsSort, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSSORT_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsSortDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSSORTDTOR_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSISORT_DECL
SV_DECL_PTRARR_SORT_VISIBILITY( SvByteStringsISort, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSISORT_DECL
#endif

#ifndef _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvByteStringsISortDtor, ByteStringPtr, 1, 1,  )
#define _SVSTDARR_BYTESTRINGSISORTDTOR_DECL
#endif

#ifndef _SVSTDARR_XUB_STRLEN_DECL
SV_DECL_VARARR_VISIBILITY( SvXub_StrLens, xub_StrLen, 1, 1,  )
#define _SVSTDARR_XUB_STRLEN_DECL
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
