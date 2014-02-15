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

#include "swtypes.hxx"
#include "swrect.hxx"

class SwViewShell;
#define VIRTUALHEIGHT 64

class SwLayVout
{
    friend void _FrmFinit();    //deletes Vout
private:
    SwViewShell*      pSh;
    OutputDevice*   pOut;
    VirtualDevice*  pVirDev;
    SwRect          aRect;
    SwRect          aOrgRect;
    Size            aSize;
    sal_uInt16          nCount;

    sal_Bool DoesFit( const Size &rOut );

public:
    SwLayVout() : pSh(0), pOut(0), pVirDev(0), aSize(0, VIRTUALHEIGHT), nCount(0) {}
    ~SwLayVout() { delete pVirDev; }

    /// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
    void Enter( SwViewShell *pShell, SwRect &rRect, sal_Bool bOn );
    void Leave() { --nCount; Flush(); }

    void SetOrgRect( SwRect &rRect ) { aOrgRect = rRect; }
    const SwRect& GetOrgRect() const { return aOrgRect; }

    sal_Bool IsFlushable() const { return 0 != pOut; }
    void _Flush();
    void Flush() { if( pOut ) _Flush(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
