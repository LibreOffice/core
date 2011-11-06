/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
