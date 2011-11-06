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



#ifndef _EDIMP_HXX
#define _EDIMP_HXX

#include <tools/solar.h>
#include <svl/svarray.hxx>

class SwPaM;
class SwNodeIndex;

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
