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
// We need an empty block in here. Otherwise, if the #ifndef INCLUDED_SVL_NRANGES_HXX
// line is the first line, the Mac OS X version of the gcc preprocessor will
// incorrectly optimize the inclusion process and will never include this file
// a second time
#endif

#ifndef INCLUDED_SVL_NRANGES_HXX
#define INCLUDED_SVL_NRANGES_HXX

class SfxUShortRanges
{
    sal_uInt16*                 _pRanges; // 0-terminated array of sal_uInt16-pairs

public:
                                SfxUShortRanges() : _pRanges( nullptr ) {}
                                SfxUShortRanges( const SfxUShortRanges &rOrig );
                                SfxUShortRanges( sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
                                SfxUShortRanges( const sal_uInt16* nNumTable );
                                ~SfxUShortRanges()
                                { delete [] _pRanges; }

    bool                        operator == ( const SfxUShortRanges & ) const;
    bool                        operator != ( const SfxUShortRanges & rRanges ) const
                                { return !( *this == rRanges ); }

    SfxUShortRanges&            operator = ( const SfxUShortRanges & );

    SfxUShortRanges&            operator += ( const SfxUShortRanges & );
    SfxUShortRanges&            operator -= ( const SfxUShortRanges & );
    SfxUShortRanges&            operator /= ( const SfxUShortRanges & );

    sal_uInt16                  Count() const;
    bool                        IsEmpty() const
                                { return !_pRanges || 0 == *_pRanges; }

                                operator const sal_uInt16* () const
                                { return _pRanges; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
