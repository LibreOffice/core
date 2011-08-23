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

#include "crsrsh.hxx"
namespace binfilter {

/*
 * MACROS um ueber alle Bereiche zu iterieren
 */
#define PCURCRSR (_pStartCrsr)

#define FOREACHPAM_START(pCURSH) \
    {\
        SwPaM *_pStartCrsr = (pCURSH)->GetCrsr(), *__pStartCrsr = _pStartCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr ); \
    }


#define FOREACHCURSOR_START(pCURSH) \
    {\
        SwShellCrsr *_pStartCrsr = *(pCURSH)->GetSwCrsr(), *__pStartCrsr = _pStartCrsr; \
        do {

#define FOREACHCURSOR_END() \
        } while( (_pStartCrsr=*(SwCursor*)_pStartCrsr->GetNext()) != __pStartCrsr ); \
    }


struct SwPamRange
{
    ULONG nStart, nEnd;

    SwPamRange() : nStart( 0 ), nEnd( 0 )	{}
    SwPamRange( ULONG nS, ULONG nE ) : nStart( nS ), nEnd( nE )	{}

    BOOL operator==( const SwPamRange& rRg )
        { return nStart == rRg.nStart ? TRUE : FALSE; }
    BOOL operator<( const SwPamRange& rRg )
        { return nStart < rRg.nStart ? TRUE : FALSE; }
};

SV_DECL_VARARR_SORT( _SwPamRanges, SwPamRange, 0, 1 )

class SwPamRanges : private _SwPamRanges
{
public:
    SwPamRanges( const SwPaM& rRing ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SwPamRanges( const SwPaM& rRing );

    SwPaM& SetPam( USHORT nArrPos, SwPaM& rPam ){DBG_BF_ASSERT(0, "STRIP"); return rPam;} //STRIP001 SwPaM& SetPam( USHORT nArrPos, SwPaM& rPam );

    USHORT Count() const
                {	return _SwPamRanges::Count(); }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
