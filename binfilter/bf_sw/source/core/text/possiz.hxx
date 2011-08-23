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
#ifndef _POSSIZ_HXX
#define _POSSIZ_HXX


#include <tools/gen.hxx>
#include "txttypes.hxx"
namespace binfilter {

// Im Gegensazt zu den SV-Sizes ist die SwPosSize immer positiv
class SwPosSize
{
    KSHORT nWidth;
    KSHORT nHeight;
public:
    inline SwPosSize( const KSHORT nWidth = 0, const KSHORT nHeight = 0 )
        : nWidth(nWidth), nHeight(nHeight) { }
    inline SwPosSize( const Size &rSize )
        : nWidth(KSHORT(rSize.Width())), nHeight(KSHORT(rSize.Height())){ }
    inline KSHORT Height() const { return nHeight; }
    inline void Height( const KSHORT nNew ) { nHeight = nNew; }
    inline KSHORT Width() const { return nWidth; }
    inline void Width( const KSHORT nNew ) { nWidth = nNew; }

    inline Size SvLSize() const { return Size( nWidth, nHeight ); }
    inline void SvLSize( const Size &rSize );
    inline void SvXSize( const Size &rSize );
    inline sal_Bool operator==( const SwPosSize &rSize ) const;
    inline SwPosSize &operator=( const SwPosSize &rSize );
    inline SwPosSize &operator=( const Size &rSize );
};

inline sal_Bool SwPosSize::operator==(const SwPosSize &rSize ) const
{
    return( Height() == rSize.Height() &&
            Width() == rSize.Width() );
}

inline SwPosSize &SwPosSize::operator=(const SwPosSize &rSize )
{
    nWidth	= rSize.Width();
    nHeight = rSize.Height();
    return *this;
}

inline void SwPosSize::SvLSize( const Size &rSize )
{
    nWidth	= KSHORT(rSize.Width());
    nHeight = KSHORT(rSize.Height());
}

inline void SwPosSize::SvXSize( const Size &rSize )
{
    nHeight	= KSHORT(rSize.Width());
    nWidth = KSHORT(rSize.Height());
}

inline SwPosSize &SwPosSize::operator=( const Size &rSize )
{
    nWidth	= KSHORT(rSize.Width());
    nHeight = KSHORT(rSize.Height());
    return *this;
}


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
