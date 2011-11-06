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



#ifndef SC_OTLNBUFF_HXX
#define SC_OTLNBUFF_HXX

#include <tools/solar.h>

class ScOutlineArray;

class XclImpOutlineBuffer
{
    // -> exctools.cxx
    private:
        sal_uInt8*          pLevel;
        sal_Bool*           pOuted;
        sal_Bool*           pHidden;
        SCSIZE          nSize;
        SCSIZE          nLast;
        sal_uInt8           nMaxLevel;
        ScOutlineArray* pOutlineArray;
        sal_Bool            bButtonNormal;  // sal_True -> right / under
    public:
                        XclImpOutlineBuffer( SCSIZE nNewSize );
                        ~XclImpOutlineBuffer();
        void            SetLevel( SCSIZE nIndex, sal_uInt8 nVal, sal_Bool bOuted, sal_Bool bHidden );
        void            SetOutlineArray( ScOutlineArray* pOArray );
        void            Reset( void );
        void            MakeScOutline( void );
        void            SetLevelRange( SCSIZE nF, SCSIZE nL, sal_uInt8 nVal,
                            sal_Bool bOuted, sal_Bool bHidden );

        inline sal_Bool     HasOutline( void ) const;

        inline void     SetButtonMode( const sal_Bool bRightOrUnder );
};




inline sal_Bool XclImpOutlineBuffer::HasOutline( void ) const
{
    return nMaxLevel > 0;
}


inline void XclImpOutlineBuffer::SetButtonMode( const sal_Bool b )
{
    bButtonNormal = b;
}


#endif

