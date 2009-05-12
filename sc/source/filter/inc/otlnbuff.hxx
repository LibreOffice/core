/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: otlnbuff.hxx,v $
 * $Revision: 1.5.32.2 $
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
        BYTE*           pLevel;
        BOOL*           pOuted;
        BOOL*           pHidden;
        SCSIZE          nSize;
        SCSIZE          nLast;
        BYTE            nMaxLevel;
        ScOutlineArray* pOutlineArray;
        BOOL            bButtonNormal;  // TRUE -> right / under
    public:
                        XclImpOutlineBuffer( SCSIZE nNewSize );
                        ~XclImpOutlineBuffer();
        void            SetLevel( SCSIZE nIndex, BYTE nVal, BOOL bOuted, BOOL bHidden );
        void            SetOutlineArray( ScOutlineArray* pOArray );
        void            Reset( void );
        void            MakeScOutline( void );
        void            SetLevelRange( SCSIZE nF, SCSIZE nL, BYTE nVal,
                            BOOL bOuted, BOOL bHidden );

        inline BOOL     HasOutline( void ) const;

        inline void     SetButtonMode( const BOOL bRightOrUnder );
};




inline BOOL XclImpOutlineBuffer::HasOutline( void ) const
{
    return nMaxLevel > 0;
}


inline void XclImpOutlineBuffer::SetButtonMode( const BOOL b )
{
    bButtonNormal = b;
}


#endif

