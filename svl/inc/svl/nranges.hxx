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
#ifdef MACOSX
// We need an empty block in here. Otherwise, if the #ifndef _SFXNRANGES_HXX
// line is the first line, the Mac OS X version of the gcc preprocessor will
// incorrectly optimize the inclusion process and will never include this file
// a second time
#endif

#ifndef _SFXNRANGES_HXX

#ifndef NUMTYPE

#define NUMTYPE sal_uInt16
#define SfxNumRanges SfxUShortRanges
#include <svl/nranges.hxx>

#undef NUMTYPE
#define NUMTYPE sal_uLong
#define SfxNumRanges SfxULongRanges
#include <svl/nranges.hxx>

#define _SFXNRANGES_HXX

#else
#include <tools/solar.h>

//========================================================================

#define NUMTYPE_ARG int

class SfxNumRanges
{
    NUMTYPE*                    _pRanges; // 0-terminated array of NUMTYPE-pairs

public:
                                SfxNumRanges() : _pRanges( 0 ) {}
                                SfxNumRanges( const SfxNumRanges &rOrig );
                                SfxNumRanges( NUMTYPE nWhich1, NUMTYPE nWhich2 );
                                SfxNumRanges( NUMTYPE_ARG nWh1, NUMTYPE_ARG nWh2, NUMTYPE_ARG nNull, ... );
                                SfxNumRanges( const NUMTYPE* nNumTable );
                                ~SfxNumRanges()
                                { delete [] _pRanges; }

    sal_Bool                        operator == ( const SfxNumRanges & ) const;
    sal_Bool                        operator != ( const SfxNumRanges & rRanges ) const
                                { return !( *this == rRanges ); }

    SfxNumRanges&               operator = ( const SfxNumRanges & );

    SfxNumRanges&               operator += ( const SfxNumRanges & );
    SfxNumRanges&               operator -= ( const SfxNumRanges & );
    SfxNumRanges&               operator /= ( const SfxNumRanges & );

    NUMTYPE                     Count() const;
    sal_Bool                        IsEmpty() const
                                { return !_pRanges || 0 == *_pRanges; }
    sal_Bool                        Contains( NUMTYPE n ) const;
    sal_Bool                        Intersects( const SfxNumRanges & ) const;

                                operator const NUMTYPE* () const
                                { return _pRanges; }
};

#undef NUMTYPE
#undef SfxNumRanges

#endif

#endif
