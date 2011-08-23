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

#define NUMTYPE	USHORT
#define SfxNumRanges SfxUShortRanges
#include <bf_svtools/nranges.hxx>

#undef NUMTYPE
#define NUMTYPE	ULONG
#define SfxNumRanges SfxULongRanges
#include <bf_svtools/nranges.hxx>

#define _SFXNRANGES_HXX

#else

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

namespace binfilter
{

//========================================================================

#define NUMTYPE_ARG	int

class SfxNumRanges
{
    NUMTYPE*                 	_pRanges; // 0-terminated array of NUMTYPE-pairs

public:
                                SfxNumRanges() : _pRanges( 0 ) {}
                                SfxNumRanges( const SfxNumRanges &rOrig );
                                ~SfxNumRanges()
                                { delete [] _pRanges; }

    BOOL						operator == ( const SfxNumRanges & ) const;
    BOOL						operator != ( const SfxNumRanges & rRanges ) const
                                { return !( *this == rRanges ); }

    SfxNumRanges&				operator = ( const SfxNumRanges & );

    SfxNumRanges&				operator += ( const SfxNumRanges & );
    SfxNumRanges&				operator -= ( const SfxNumRanges & );
    SfxNumRanges&				operator /= ( const SfxNumRanges & );

    NUMTYPE 					Count() const;
    BOOL						IsEmpty() const
                                { return !_pRanges || 0 == *_pRanges; }

                                operator const NUMTYPE* () const
                                { return _pRanges; }
};

}

#undef NUMTYPE
#undef SfxNumRanges

#endif

#endif
