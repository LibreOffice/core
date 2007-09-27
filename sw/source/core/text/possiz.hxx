/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: possiz.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:19:10 $
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
#ifndef _POSSIZ_HXX
#define _POSSIZ_HXX


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#include "txttypes.hxx"

// Im Gegensazt zu den SV-Sizes ist die SwPosSize immer positiv
class SwPosSize
{
    KSHORT nWidth;
    KSHORT nHeight;
public:
    inline SwPosSize( const KSHORT nW = 0, const KSHORT nH = 0 )
        : nWidth(nW), nHeight(nH) { }
    inline SwPosSize( const Size &rSize )
        : nWidth(KSHORT(rSize.Width())), nHeight(KSHORT(rSize.Height())){ }
    inline KSHORT Height() const { return nHeight; }
    inline void Height( const KSHORT nNew ) { nHeight = nNew; }
    inline KSHORT Width() const { return nWidth; }
    inline void Width( const KSHORT nNew ) { nWidth = nNew; }

    inline Size SvLSize() const { return Size( nWidth, nHeight ); }
    inline void SvLSize( const Size &rSize );
    inline void SvXSize( const Size &rSize );
    inline SwPosSize &operator=( const SwPosSize &rSize );
    inline SwPosSize &operator=( const Size &rSize );
};

inline SwPosSize &SwPosSize::operator=(const SwPosSize &rSize )
{
    nWidth  = rSize.Width();
    nHeight = rSize.Height();
    return *this;
}

inline void SwPosSize::SvLSize( const Size &rSize )
{
    nWidth  = KSHORT(rSize.Width());
    nHeight = KSHORT(rSize.Height());
}

inline void SwPosSize::SvXSize( const Size &rSize )
{
    nHeight = KSHORT(rSize.Width());
    nWidth = KSHORT(rSize.Height());
}

inline SwPosSize &SwPosSize::operator=( const Size &rSize )
{
    nWidth  = KSHORT(rSize.Width());
    nHeight = KSHORT(rSize.Height());
    return *this;
}


#endif

