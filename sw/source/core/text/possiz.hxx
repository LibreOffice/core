/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_POSSIZ_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_POSSIZ_HXX

#include <tools/gen.hxx>
#include "txttypes.hxx"

// Compared to the SV sizes SwPosSize is always positive
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
