/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nranges.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:55:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#define NUMTYPE USHORT
#define SfxNumRanges SfxUShortRanges
#include <svtools/nranges.hxx>

#undef NUMTYPE
#define NUMTYPE ULONG
#define SfxNumRanges SfxULongRanges
#include <svtools/nranges.hxx>

#define _SFXNRANGES_HXX

#else

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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

    BOOL                        operator == ( const SfxNumRanges & ) const;
    BOOL                        operator != ( const SfxNumRanges & rRanges ) const
                                { return !( *this == rRanges ); }

    SfxNumRanges&               operator = ( const SfxNumRanges & );

    SfxNumRanges&               operator += ( const SfxNumRanges & );
    SfxNumRanges&               operator -= ( const SfxNumRanges & );
    SfxNumRanges&               operator /= ( const SfxNumRanges & );

    NUMTYPE                     Count() const;
    BOOL                        IsEmpty() const
                                { return !_pRanges || 0 == *_pRanges; }
    BOOL                        Contains( NUMTYPE n ) const;
    BOOL                        Intersects( const SfxNumRanges & ) const;

                                operator const NUMTYPE* () const
                                { return _pRanges; }
};

#undef NUMTYPE
#undef SfxNumRanges

#endif

#endif
