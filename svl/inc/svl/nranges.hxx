/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

                                operator const NUMTYPE* () const
                                { return _pRanges; }
};

#undef NUMTYPE
#undef SfxNumRanges

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
