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

