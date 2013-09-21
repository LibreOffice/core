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

#ifndef _EDIMP_HXX
#define _EDIMP_HXX

#include <tools/solar.h>
#include <o3tl/sorted_vector.hxx>

class SwPaM;
class SwNodeIndex;

// Macros to iterate over all ranges.
#define PCURCRSR (_pStartCrsr)

#define FOREACHPAM_START(pCURSH) \
    {\
        SwPaM *_pStartCrsr = (pCURSH), *__pStartCrsr = _pStartCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr ); \
    }



struct SwPamRange
{
    sal_uLong nStart, nEnd;

    SwPamRange() : nStart( 0 ), nEnd( 0 )   {}
    SwPamRange( sal_uLong nS, sal_uLong nE ) : nStart( nS ), nEnd( nE ) {}

    bool operator==( const SwPamRange& rRg ) const
        { return nStart == rRg.nStart ? true : false; }
    bool operator<( const SwPamRange& rRg ) const
        { return nStart < rRg.nStart ? true : false; }
};

class _SwPamRanges : public o3tl::sorted_vector<SwPamRange> {};

class SwPamRanges : private _SwPamRanges
{
public:
    SwPamRanges( const SwPaM& rRing );

    void Insert( const SwNodeIndex& rIdx1, const SwNodeIndex& rIdx2 );
    SwPaM& SetPam( sal_uInt16 nArrPos, SwPaM& rPam );

    sal_uInt16 Count() const
                {   return _SwPamRanges::size(); }
    SwPamRange operator[]( sal_uInt16 nPos ) const
                { return _SwPamRanges::operator[](nPos); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
