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


#ifndef _SWREGION_HXX
#define _SWREGION_HXX

#include <svl/svarray.hxx>

#include "swrect.hxx"

SV_DECL_VARARR( SwRects, SwRect, 20, 8 )

class SwRegionRects : public SwRects
{
    SwRect aOrigin; // die Kopie des StartRects

    inline void InsertRect( const SwRect &rRect, const sal_uInt16 nPos, sal_Bool &rDel);

public:
    SwRegionRects( const SwRect& rStartRect, sal_uInt16 nInit = 20,
                                             sal_uInt16 nGrow = 8 );
    // Zum Ausstanzen aus aOrigin.
    void operator-=( const SwRect& rRect );

    // Aus Loechern werden Flaechen, aus Flaechen werden Loecher.
    void Invert();
    // Benachbarte Rechtecke zusammenfassen.
    void Compress( sal_Bool bFuzzy = sal_True );

    inline const SwRect &GetOrigin() const { return aOrigin; }
    inline void ChangeOrigin( const SwRect &rRect ) { aOrigin = rRect; }
};

#endif  //_SWREGION_HXX
