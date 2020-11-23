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
#ifndef INCLUDED_SW_SOURCE_CORE_LAYOUT_VIRTOUTP_HXX
#define INCLUDED_SW_SOURCE_CORE_LAYOUT_VIRTOUTP_HXX

#include <vcl/virdev.hxx>

#include <swrect.hxx>

class SwViewShell;
#define VIRTUALHEIGHT 64

class SwLayVout
{
    friend void FrameFinit();    //deletes Vout
private:
    SwViewShell*      m_pShell;
    VclPtr<OutputDevice>  m_pOut;
    VclPtr<VirtualDevice> m_pVirDev;
    SwRect          m_aRect;
    SwRect          m_aOrgRect;
    Size            m_aSize;
    sal_uInt16          m_nCount;

    bool DoesFit( const Size &rOut );

public:
    SwLayVout() : m_pShell(nullptr), m_pOut(nullptr), m_pVirDev(nullptr), m_aSize(0, VIRTUALHEIGHT), m_nCount(0) {}
    ~SwLayVout() { m_pVirDev.disposeAndClear(); }

    /// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
    void Enter( SwViewShell *pShell, SwRect &rRect, bool bOn );
    void Leave() { --m_nCount; Flush(); }

    void SetOrgRect( SwRect const &rRect ) { m_aOrgRect = rRect; }
    const SwRect& GetOrgRect() const { return m_aOrgRect; }

    bool IsFlushable() const { return bool(m_pOut); }
    void Flush_();
    void Flush() { if( m_pOut ) Flush_(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
