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

#ifndef _EDIMP_HXX
#define _EDIMP_HXX

#include <tools/solar.h>
#include <svl/svarray.hxx>

class SwPaM;
class SwNodeIndex;

// Macros to iterate over all ranges.
#define PCURCRSR (_pStartCrsr)

#define FOREACHPAM_START(pCURSH) \
    {\
        SwPaM *_pStartCrsr = (pCURSH)->GetCrsr(), *__pStartCrsr = _pStartCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr ); \
    }



struct SwPamRange
{
    sal_uLong nStart, nEnd;

    SwPamRange() : nStart( 0 ), nEnd( 0 )   {}
    SwPamRange( sal_uLong nS, sal_uLong nE ) : nStart( nS ), nEnd( nE ) {}

    sal_Bool operator==( const SwPamRange& rRg )
        { return nStart == rRg.nStart ? sal_True : sal_False; }
    sal_Bool operator<( const SwPamRange& rRg )
        { return nStart < rRg.nStart ? sal_True : sal_False; }
};

SV_DECL_VARARR_SORT( _SwPamRanges, SwPamRange, 0, 1 )

class SwPamRanges : private _SwPamRanges
{
public:
    SwPamRanges( const SwPaM& rRing );

    void Insert( const SwNodeIndex& rIdx1, const SwNodeIndex& rIdx2 );
    SwPaM& SetPam( sal_uInt16 nArrPos, SwPaM& rPam );

    sal_uInt16 Count() const
                {   return _SwPamRanges::Count(); }
    SwPamRange operator[]( sal_uInt16 nPos ) const
                { return _SwPamRanges::operator[](nPos); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
