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
#include <sal/types.h>

// Compared to the SV sizes SwPosSize is always positive
class SwPosSize
{
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
public:
    SwPosSize( const sal_uInt16 nW = 0, const sal_uInt16 nH = 0 )
        : nWidth(nW)
        , nHeight(nH)
    {
    }
    explicit SwPosSize( const Size &rSize )
        : nWidth(sal_uInt16(rSize.Width()))
        ,nHeight(sal_uInt16(rSize.Height()))
    {
    }
#if defined(__COVERITY__)
    virtual ~SwPosSize() COVERITY_NOEXCEPT_FALSE {}
#else
    virtual ~SwPosSize() {}
#endif
    SwPosSize(SwPosSize const &) = default;
    SwPosSize(SwPosSize &&) = default;
    SwPosSize & operator =(SwPosSize const &) = default;
    SwPosSize & operator =(SwPosSize &&) = default;
    sal_uInt16 Height() const { return nHeight; }
    virtual void Height(const sal_uInt16 nNew, const bool = true) { nHeight = nNew; }
    sal_uInt16 Width() const { return nWidth; }
    void Width( const sal_uInt16 nNew ) { nWidth = nNew; }
    Size SvLSize() const { return Size( nWidth, nHeight ); }
    void SvLSize( const Size &rSize )
    {
        nWidth  = sal_uInt16(rSize.Width());
        nHeight = sal_uInt16(rSize.Height());
    }
    void SvXSize( const Size &rSize )
    {
        nHeight = sal_uInt16(rSize.Width());
        nWidth = sal_uInt16(rSize.Height());
    }
    SwPosSize& operator=( const Size &rSize )
    {
        nWidth  = sal_uInt16(rSize.Width());
        nHeight = sal_uInt16(rSize.Height());
        return *this;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
