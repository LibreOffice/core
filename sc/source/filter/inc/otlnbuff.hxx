/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: otlnbuff.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:23:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OTLNBUFF_HXX
#define _OTLNBUFF_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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
        void            Reset( ScOutlineArray* pOArray );
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

